#include <string>

#include "PDFExtractor.h"
#include "PDFFont.h"
#include "utils/logger.h"
#include "unicode.h"


static const double EPS = 1e-6;
static const double DEFAULT_DPI = 72.0;

#include <math.h>
//static inline double round(double x) { return (std::abs(x) > EPS) ? x : 0.0; }
static inline bool equal(double x, double y) { return fabs(x-y) <= EPS; }
__attribute__((unused)) static inline bool is_positive(double x) { return x > EPS; }
__attribute__((unused)) static inline bool tm_equal(const double * tm1, const double * tm2, int size = 6) {
    for(int i = 0; i < size; ++i)
        if(!equal(tm1[i], tm2[i]))
            return false;
    return true;
}
static inline long long hash_ref(const Ref * id)
{
    return (((long long)(id->num)) << (sizeof(id->gen)*8)) | (id->gen);
}

GooString g_content;
std::string content;
std::string content0;
bool need_rescale_font = true;
bool process_type3 = false;
bool space_as_offset = false;


#include <UnicodeMap.h>
#include <GlobalParams.h>
#include <PDFDocEncoding.h>
std::string unicode_to_string(GooString *s){
    UnicodeMap *uMap = globalParams->getTextEncoding();

    std::string str;
    char buf[9];
    GBool isUnicode;
    Unicode u;
    int i, n;
    if ( (s->getChar(0) & 0xff ) == 0xfe && 
            (s->getChar(1) & 0xff) == 0xff ){
        isUnicode = gTrue;
        i = 2;
    } else {
        isUnicode = gFalse;
        i = 0;
    }
    while ( i < s->getLength() ){
        if ( isUnicode ){
            u = ((s->getChar(i) & 0xff) << 8) |
                (s->getChar(i+1) & 0xff);
            i += 2;
        } else {
            u = pdfDocEncoding[s->getChar(i) & 0xff];
            i++;
        }
        n = uMap->mapUnicode(u, buf, sizeof(buf));
        buf[n] = '\0';
        str += std::string(buf);
    };
    return str;
}


class PDFExtractor::InnerData {
public:
    InnerData(){
        reset();
    }
    ~InnerData(){};
    void reset();

    PDFDoc *m_pdfDoc;
    double cur_font_size;;
    double string_x;
    double string_y;

    void check_state_change(GfxState *state);
    void prepare_text_line(GfxState *state);

    bool GfxFont2PDFFont(GfxFont &font, PDFFont &pdfFont);

}; //class PDFExtractor::InnerData

void PDFExtractor::InnerData::reset(){
    m_pdfDoc = nullptr;
    cur_font_size = 0.0;
    string_x = 0.0;
    string_y = 0.0;
}

// -------- PDFExtractor::InnerData::check_state_change() --------
void PDFExtractor::InnerData::check_state_change(GfxState * state){

    double new_font_size = state->getFontSize();
    if(!equal(cur_font_size, new_font_size)) {
        need_rescale_font = true;
        cur_font_size = new_font_size;
    }
}

// -------- PDFExtractor::InnerData::prepare_text_line() --------
void PDFExtractor::InnerData::prepare_text_line(GfxState * state){
    double rise_x, rise_y;
    state->textTransformDelta(0, state->getRise(), &rise_x, &rise_y);
    state->transform(state->getCurX() + rise_x, state->getCurY() + rise_y, &string_x, &string_y);
}

// -------- PDFExtractor::InnerData::GfxFont2PDFFont() --------
bool PDFExtractor::InnerData::GfxFont2PDFFont(GfxFont &font, PDFFont &pdfFont){
    pdfFont.ID = hash_ref(font.getID());
    pdfFont.tag =  font.getTag() != nullptr ? font.getTag()->getCString() : "";
    pdfFont.family = font.getFamily() != nullptr ? font.getFamily()->getCString() : "";
    pdfFont.name = font.getName() != nullptr ? font.getName()->getCString() : "";

    GfxFontType fontType = font.getType();
    if ( fontType == fontTrueType ){
        pdfFont.type = PDFFontType::TrueType;
    } else if ( fontType == fontCIDType2 ){
        pdfFont.type = PDFFontType::CIDType2;
    } else if ( fontType == fontType1 ){
        pdfFont.type = PDFFontType::Type1;
    } else if ( fontType == fontType3 ){
        pdfFont.type = PDFFontType::Type3;
    }

    long long embID;
    Ref embRef;
    if ( font.getEmbeddedFontID(&embRef) ){
        embID = hash_ref(&embRef);
    } else {
        embID = hash_ref(&embRef);
        //embID = -1;
    }
    pdfFont.embID = embID;

    pdfFont.encodingName = font.getEncodingName() != nullptr ? font.getEncodingName()->getCString() : "";
    pdfFont.flags = font.getFlags();

    auto *font_loc = font.locateFont(m_pdfDoc->getXRef(), nullptr);
    if ( font_loc != nullptr ){
        switch(font_loc->locType)
        {
            case gfxFontLocEmbedded:
                pdfFont.location = PDFFontLocation::Embedded;
                break;
            case gfxFontLocExternal:
                pdfFont.location = PDFFontLocation::External;
                break;
            case gfxFontLocResident:
                pdfFont.location = PDFFontLocation::Resident;
                break;
        };
    }

    GfxFontLoc *localfontloc = font.locateFont(m_pdfDoc->getXRef(), nullptr);
    pdfFont.filePath = localfontloc != nullptr ?  
        (localfontloc->path != nullptr ? localfontloc->path->getCString() : "") 
        : "";

    pdfFont.embeddedName = font.getEmbeddedFontName() != nullptr ? font.getEmbeddedFontName()->getCString() : "";

    double *ctm = font.getFontMatrix();
    if ( ctm != nullptr ){
        memcpy(pdfFont.ctm, ctm, sizeof(double) * 6);
    }

    double *bbox = font.getFontBBox();
    if ( bbox != nullptr ){
        memcpy(pdfFont.bbox, bbox, sizeof(double) * 4);
    }

    return true;
}
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args){
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// ======== PDFExtractor::PDFExtractor() ========
PDFExtractor::PDFExtractor()
    : OutputDev(), m_innerData(make_unique<PDFExtractor::InnerData>()){
}

// ======== PDFExtractor::~PDFExtractor() ========
PDFExtractor::~PDFExtractor(){
    Object::memCheck(stderr);
    gMemReport(std::stderr);
}

// ======== PDFExtractor::Process() ========
bool PDFExtractor::Process(const std::string &pdfFilename, const std::string &ownerPasswd, const std::string &userPasswd){
    bool ok = false;

    if ( OpenPDFFile(pdfFilename, ownerPasswd, userPasswd) ){

        PDFDoc *pdfDoc = m_innerData->m_pdfDoc;
        if ( pdfDoc != nullptr ){
            auto numPages = m_innerData->m_pdfDoc->getNumPages();
            for(auto i = 0; i < numPages ; ++i) {
                std::cerr << "Preprocessing: " << i + 1 << "/" << numPages << '\r' << std::flush;

                Page *pdfPage = pdfDoc->getPage(i+1);
                if ( pdfPage != nullptr ){
                    PDFRectangle *mediaBox = pdfPage->getMediaBox();
                    LOG(DEBUG) << "mdeiaBox:(" << mediaBox->x1 << ", " << mediaBox->y1
                        << ", " << mediaBox->x2 << ", " << mediaBox->y2 << ")";

                    double pageMediaWidth = pdfPage->getMediaWidth();
                    double pageMediaHeight = pdfPage->getMediaHeight();
                    double pageCropWidth = pdfPage->getCropWidth();
                    double pageCropHeight = pdfPage->getCropHeight();
                    int pageRotate = pdfPage->getRotate();

                    LOG(DEBUG) << "Page " << i << " Media(" << pageMediaWidth << ", " << pageMediaHeight << ") " 
                        << "Crop(" << pageCropWidth << ", " << pageCropHeight << ") "
                        << "Rotate: " << pageRotate;


                    double pageCTM[6];
                    pdfPage->getDefaultCTM(&pageCTM[0], 72, 72, 0, false, false);
                    LOG(DEBUG) << "Default CTM(" << 
                        pageCTM[0] << ", " <<
                        pageCTM[1] << ", " <<
                        pageCTM[2] << ", " <<
                        pageCTM[3] << ", " <<
                        pageCTM[4] << ", " <<
                        pageCTM[5] << ") ";

                    LOG(DEBUG) << "Start pdfDoc->displayPage() Page:" << i;

                    pdfPage->display(this, DEFAULT_DPI, DEFAULT_DPI,
                            0, 
                            0, //(!(param.use_cropbox)),
                            true,  // crop
                            false, // printing
                            nullptr, nullptr, nullptr, nullptr);

                } else {
                    LOG(WARNING) << "Page " << i << " is nullptr.";
                }
            }

            if(numPages >= 0)
                std::cerr << "Preprocessing: " << numPages << "/" << numPages;
            std::cerr << std::endl;

            ClosePDFFile();

            std::fstream file;
            file.open("content.txt", std::ios::binary | std::ios::out | std::ios::trunc);
            file.write(content.c_str(), content.length());
            file.close();

            content0 = unicode_to_string(&g_content);
            std::fstream file0;
            file0.open("content0.txt", std::ios::binary | std::ios::out | std::ios::trunc);
            file0.write(content0.c_str(), content0.length());
            file0.close();

            ok = true;
        } else {
            LOG(WARNING) << "pdfDoc is nullptr";
        }
    }

    return ok;
}

// ======== PDFExtractor::OpenPDFFile() ========
bool PDFExtractor::OpenPDFFile(const std::string &pdfFilename, const std::string &ownerPasswd, const std::string &userPasswd){
    bool ok = false;

    GooString fileName(pdfFilename.c_str());
    GooString *ownerPW = (ownerPasswd == "") ? nullptr: new GooString(ownerPasswd.c_str()); 
    GooString *userPW = (userPasswd == "") ? nullptr : new GooString(userPasswd.c_str());

    PDFDoc *pdfDoc = PDFDocFactory().createPDFDoc(fileName, ownerPW, userPW);
    if ( pdfDoc != nullptr ){
        if ( pdfDoc->isOk() ){
            if (pdfDoc->okToCopy()){
                size_t num_pages = pdfDoc->getNumPages();
                LOG(DEBUG) << "Total " << num_pages << " pages in pdf file: " << pdfFilename;
                //m_innerData->m_pdfDoc = std::move(pdfDoc);
                m_innerData->m_pdfDoc = pdfDoc;
                ok = true;
            } else {
                LOG(ERROR) << "PDF file is not okToCopy. " << pdfFilename;
            }
        } else {
            LOG(ERROR) << "Cann't read pdf file: " << pdfFilename;
        }

        if ( !ok ){
            delete pdfDoc;
            pdfDoc = nullptr;
        }
    } else {
        LOG(WARNING) << "pdfDoc == nullptr";
    }

    if ( ownerPW != nullptr ){
        delete ownerPW;
        ownerPW = nullptr;
    }
    if ( userPW != nullptr ){
        delete userPW;
        userPW = nullptr;
    }

    return ok;
}

// ======== PDFExtractor::ClosePDFFile() ========
void PDFExtractor::ClosePDFFile(){
}


// ======== PDFExtractor::drawChar() ========
void PDFExtractor::drawChar(GfxState *state, double x, double y,
        double dx, double dy,
        double originX, double originY,
        CharCode code, int nBytes, Unicode *u, int uLen){
    LOG(DEBUG) << "call drawChar()";
}

// ======== PDFExtractor::drawString() ========
void PDFExtractor::drawString(GfxState * state, GooString * s){
    //std::string text = std::string(s->getCString());
    //std::string text = unicode_to_string(s);
    //if ( text.empty() ) return;

    //content0 += text;
    //LOG(DEBUG) << "call drawString()" << text << " len=" << text.length();

    g_content.append(s);

    auto font = state->getFont();
    double cur_letter_space = state->getCharSpace();
    double cur_word_space   = state->getWordSpace();
    double cur_horiz_scaling = state->getHorizScaling();

    if( (font == nullptr) 
        || (font->getWMode())
        || ((font->getType() == fontType3) && (!process_type3))
      ) {
        return;
    }

    m_innerData->check_state_change(state);
     
    m_innerData->prepare_text_line(state);

    char *p = s->getCString();
    int len = s->getLength();


    //accumulated displacement of chars in this string, in text object space
    double dx = 0;
    double dy = 0;
    //displacement of current char, in text object space, including letter space but not word space.
    double ddx, ddy;
    //advance of current char, in glyph space
    double ax, ay;
    //origin of current char, in glyph space
    double ox, oy;

    int uLen;

    CharCode code;
    Unicode *u = nullptr;


    while (len > 0) 
    {
        auto n = font->getNextChar(p, len, &code, &u, &uLen, &ax, &ay, &ox, &oy);
        //HR_DEBUG(printf("HTMLRenderer::drawString:unicode=%lc(%d)\n", (wchar_t)u[0], u[0]));
        //char unicodeBuf[32];
        //sprintf(unicodeBuf, "unicode=%lc(%d)", (wchar_t)u[0], u[0]);
        //sprintf(unicodeBuf, "unicode=%X(%x)", (wchar_t)u[0], u[0]);
        //LOG(DEBUG) << "----->" <<  std::string(unicodeBuf) << "<-------";
        //unicodeBuf[0] = u[0];
        //unicodeBuf[1] = u[1];

        if(!(equal(ox, 0) && equal(oy, 0))) {
            std::cerr << "TODO: non-zero origins" << std::endl;
        }
        ddx = ax * m_innerData->cur_font_size + cur_letter_space;
        ddy = ay * m_innerData->cur_font_size;

        //tracer.draw_char(state, dx, dy, ax, ay);


        bool is_space = false;
        if (n == 1 && *p == ' ') {
            is_space = true;
        }

        if(is_space && (space_as_offset)) {
            //html_text_page.get_cur_line()->append_padding_char();
            //// ignore horiz_scaling, as it has been merged into CTM
            //html_text_page.get_cur_line()->append_offset((ax * m_innerData->cur_font_size + cur_letter_space + cur_word_space) * draw_text_scale);
        } else {
            char cbuf[32];
            for ( auto k = 0 ; k < uLen ; k++ ){
                cbuf[k] = u[k];
            }
            cbuf[uLen] = '\0';
            content += std::string(cbuf);

            bool decompose_ligature = true;
            if((decompose_ligature) && (uLen > 1) && std::none_of(u, u+uLen, is_illegal_unicode)) {
                //html_text_page.get_cur_line()->append_unicodes(u, uLen, ddx);
            } else {
                Unicode uu;
                //if(cur_text_state.font_info->use_tounicode) {
                if ( true ) {
                    uu = check_unicode(u, uLen, code, font);
                    char buf[16];
                    sprintf(buf, "%X -->(%X)", uu, code);
                    LOG(INFO) << buf << "**********";

                    char *u0 = (char*)&uu;
                    for (auto i = 0 ; i < uLen ; i++ ){
                        buf[i] = u0[i];
                    }
                    buf[uLen] = 0;
                    //LOG(DEBUG) << std::string(buf) << std::endl;
                    //content += std::string(buf);

                    // Font
                    PDFFont pdfFont;
                    m_innerData->GfxFont2PDFFont(*font, pdfFont);
                    //LOG(INFO) << pdfFont.ToString();

                } else {
                    //uu = unicode_from_font(code, font);
                }
                //html_text_page.get_cur_line()->append_unicodes(&uu, 1, ddx);

                /*
                 * In PDF, word_space is appended if (n == 1 and *p = ' ')
                 * but in HTML, word_space is appended if (uu == ' ')
                 */
                int space_count = (is_space ? 1 : 0) - ((uu == ' ') ? 1 : 0);
                if(space_count != 0) {
                    //html_text_page.get_cur_line()->append_offset(cur_word_space * draw_text_scale * space_count);
                }
            }
        }

        dx += ddx * cur_horiz_scaling;
        dy += ddy;
        if (is_space){
            dx += cur_word_space * cur_horiz_scaling;
        }

        p += n;
        len -= n;
    }


    //cur_tx += dx;
    //cur_ty += dy;
        
    //draw_tx += dx;
    //draw_ty += dy;
}


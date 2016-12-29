#include <iostream>
#include <iomanip>
#include "OFDCommon.h"
#include "OFDOutputDev.h"
#include "OFDPage.h"
#include "OFDTextObject.h"
#include "utils/logger.h"

using namespace ofd;

GBool rawOrder = gFalse;

OFDOutputDev::OFDOutputDev(ofd::OFDPackagePtr ofdPackage) :
    m_pdfDoc(nullptr),
    m_xref(nullptr), m_textPage(nullptr), 
    m_actualText(nullptr),
    m_ofdPackage(ofdPackage), m_ofdDocument(nullptr), m_currentOFDPage(nullptr),
    m_currentFont(nullptr), m_currentFontSize(14.0), m_currentCTM(nullptr) {

    m_textPage = new TextPage(rawOrder);
    m_actualText = new ActualText(m_textPage);

    if ( ofdPackage != nullptr ){
        m_ofdDocument = ofdPackage->AddNewDocument();
    }
}

OFDOutputDev::~OFDOutputDev(){

    if ( m_textPage != nullptr ){ 
        m_textPage->decRefCnt();
    }
    if ( m_actualText != nullptr ){
        delete m_actualText;  
    }
}

void OFDOutputDev::ProcessDoc(PDFDocPtr pdfDoc){
    if ( pdfDoc == nullptr ) return;
    m_pdfDoc = pdfDoc;

    double resolution = 72.0;
    GBool useMediaBox = gTrue;
    GBool crop = gFalse;
    GBool printing = gFalse;

    auto numPages = pdfDoc->getNumPages();
    LOG(INFO) << "Total " << numPages << " pages in pdf file"; 

    for ( auto i = 0 ; i < numPages ; i++ ){
        pdfDoc->displayPage(this, i + 1, resolution, resolution, 0, useMediaBox, crop, printing);
    }
}

void OFDOutputDev::SetTextPage(TextPage *textPage)
{
    if ( m_textPage != nullptr ){ 
        m_textPage->decRefCnt();
    }
    if ( m_actualText != nullptr ){
        delete m_actualText;
    }
    if ( textPage != nullptr ){
        m_textPage = textPage;
        m_textPage->incRefCnt();
        m_actualText = new ActualText(m_textPage);
    } else {
        m_textPage = nullptr;
        m_actualText = nullptr;
    }
}

TextPage *OFDOutputDev::TakeTextPage(){
    TextPage *textPage;

    textPage = m_textPage;
    if ( textPage != nullptr ){
        delete m_actualText;
    }

    m_textPage = new TextPage(rawOrder);
    m_actualText = new ActualText(m_textPage);

    return textPage;
}

void OFDOutputDev::startPage(int pageNum, GfxState *state, XRef *xrefA) {
    if ( m_textPage != nullptr ){
        delete m_actualText;
        m_textPage->decRefCnt();
        m_textPage = new TextPage(rawOrder);
        m_actualText = new ActualText(m_textPage);

        m_textPage->startPage(state);
    }
    if ( xrefA != nullptr){
        m_xref = xrefA;
    }

    if ( m_ofdDocument != nullptr ){
        LOG(INFO) << "******** startPage(" << pageNum << ") ********";
        m_currentOFDPage = m_ofdDocument->AddNewPage();
        m_currentOFDPage->AddNewLayer(ofd::Layer::Type::BODY);

        Page *pdfPage = m_pdfDoc->getPage(pageNum);
        PDFRectangle *mediaBox = pdfPage->getMediaBox();
        LOG(INFO) << "mdeiaBox:(" << mediaBox->x1 << ", " << mediaBox->y1
            << ", " << mediaBox->x2 << ", " << mediaBox->y2 << ")";
        PDFRectangle *cropBox = pdfPage->getCropBox();
        LOG(INFO) << "cropBox:(" << cropBox->x1 << ", " << cropBox->y1
            << ", " << cropBox->x2 << ", " << cropBox->y2 << ")";
        //pdfPage->isCroped();

        double pageMediaWidth = pdfPage->getMediaWidth();
        double pageMediaHeight = pdfPage->getMediaHeight();
        double pageCropWidth = pdfPage->getCropWidth();
        double pageCropHeight = pdfPage->getCropHeight();
        int pageRotate = pdfPage->getRotate();

        LOG(INFO) << "Page " << pageNum;
        LOG(INFO) << " Media(" << pageMediaWidth << ", " << pageMediaHeight << ") ";
        LOG(INFO) << "Crop(" << pageCropWidth << ", " << pageCropHeight << ") ";
        LOG(INFO) << "Rotate: " << pageRotate;


        double pageCTM[6];
        pdfPage->getDefaultCTM(&pageCTM[0], 72, 72, 0, false, false);
        LOG(INFO) << "Default CTM(" << 
            pageCTM[0] << ", " <<
            pageCTM[1] << ", " <<
            pageCTM[2] << ", " <<
            pageCTM[3] << ", " <<
            pageCTM[4] << ", " <<
            pageCTM[5] << ") ";


        CT_PageArea pageArea;
        pageArea.PhysicalBox = ST_Box(cropBox->x1, cropBox->y1, cropBox->x2, cropBox->y2);
        pageArea.ApplicationBox = ST_Box(mediaBox->x1, mediaBox->y1, mediaBox->x2, mediaBox->y2);
        pageArea.EnableApplicationBox(true);
        m_currentOFDPage->SetPageArea(pageArea);

        LOG(INFO) << "\n";
    }
}

void processTextLine(OFDOutputDev *ofdOutputDev, TextLine *line, OFDLayerPtr bodyLayer){
    double xMin, yMin, xMax, yMax;
    double lineXMin = 0, lineYMin = 0, lineXMax = 0, lineYMax = 0;
    TextWord *word;
    std::stringstream wordXML;
    wordXML << std::fixed << std::setprecision(6);

    for (word = line->getWords(); word; word = word->getNext()) {
        word->getBBox(&xMin, &yMin, &xMax, &yMax);

        if (lineXMin == 0 || lineXMin > xMin) lineXMin = xMin;
        if (lineYMin == 0 || lineYMin > yMin) lineYMin = yMin;
        if (lineXMax < xMax) lineXMax = xMax;
        if (lineYMax < yMax) lineYMax = yMax;

        const std::string myString = word->getText()->getCString();

        int charPos = word->getCharPos();
        int charLen = word->getCharLen();
        LOG(INFO) << "TextWord charPos: " << charPos << " charLen: " << charLen;

        // TextWord Rotation
        int rotation = word->getRotation();

        double baseLine = word->getBaseline();
        
        // TextWord Color
        double red, green, blue;
        word->getColor(&red, &green, &blue);
        LOG(INFO) << "TextWord Baseline: " << baseLine << " Rotation: " << rotation << " Color: (" << red << ", " << green << ", " << blue << ")";
        // TextWord Font
        double fontSize = word->getFontSize();
        int numChars = word->getLength();
        LOG(INFO) << "TextWord FontSize=" << fontSize << " numChars=" << numChars << " len(string)=" << myString.length() << " size(string)=" << myString.size();
        for ( int k = 0 ; k < numChars ; k++ ){
            TextFontInfo *fontInfo = word->getFontInfo(k);

            Ref *ref = fontInfo->gfxFont->getID();
            uint64_t fontID = ref->num;

            GooString *fontName = fontInfo->getFontName();
            char tCh[4];
            tCh[0] = myString[k*3];
            tCh[1] = myString[k*3+1];
            tCh[2] = myString[k*3+2];
            tCh[3] = '\0';

            double ascent = fontInfo->getAscent();
            double descent = fontInfo->getDescent();

            double xMinA, yMinA, xMaxA, yMaxA;
            word->getCharBBox(k, &xMinA, &yMinA, &xMaxA, &yMaxA);

            double edge = word->getEdge(k);

            LOG(INFO) << tCh << "(" << xMinA << ", " << yMinA << ", " << xMaxA << ", " << yMaxA << ") " << " Edge: " << edge << " Ascent: " << ascent << " Descent: " << descent << " Font[" << k << "] name: " << std::string(fontName->getCString()) << " (" << fontID << ")";

            auto iter = ofdOutputDev->m_fonts.find(fontID);
            if ( iter != ofdOutputDev->m_fonts.end() ){
                auto font = iter->second;
                LOG(INFO) << "fontID: " << fontID << " fontName: " << std::string(fontName->getCString()) << " OFDFont::FontName: " << font->FontName;
            } else {
                LOG(WARNING) << "fontID: " << fontID << " Not found in m_fonts.";
            }
        }

        TextFontInfo *fontInfo = word->getFontInfo(0);
        Ref *ref = fontInfo->gfxFont->getID();
        uint64_t fontID = ref->num;
        auto iter = ofdOutputDev->m_fonts.find(fontID);
        auto textFont = iter->second;

        std::stringstream ss;
        ss << "          <word xMin=\"" << xMin << "\" yMin=\"" << yMin << "\" xMax=\"" <<
            xMax << "\" yMax=\"" << yMax << "\">" << myString << "</word>\n";
        LOG(DEBUG) << ss.str();

        wordXML << ss.str();

        if ( bodyLayer != nullptr ){
            OFDTextObject *textObject = new OFDTextObject();

            // OFDObject::Boundary
            textObject->Boundary = ST_Box(xMin, yMin, xMax - xMin, yMax - yMin);

            // OFDTextObject::TextCode
            Text::TextCode textCode;
            textCode.X = xMin;
            textCode.Y = yMax;
            textCode.Text = myString;
            textObject->AddTextCode(textCode);

            textObject->SetFont(textFont);
            textObject->SetFontSize(fontSize);

            OFDObjectPtr object = std::shared_ptr<OFDObject>(textObject);
            bodyLayer->AddObject(object);
        }
    }

    LOG(DEBUG) << wordXML.str();
}

void processTextPage(OFDOutputDev *ofdOutputDev, TextPage *textPage, OFDPagePtr currentOFDPage){
    OFDLayerPtr bodyLayer = nullptr;
    if ( currentOFDPage != nullptr ){
        bodyLayer = currentOFDPage->GetBodyLayer();
    }

    LOG(DEBUG) << "processTextPage(). page ID: " << currentOFDPage->GetID();

    double xMin, yMin, xMax, yMax;
    for ( auto flow = textPage->getFlows(); flow != nullptr ; flow = flow->getNext()){
        for ( auto blk = flow->getBlocks(); blk != nullptr ; blk = blk->getNext()){
            blk->getBBox(&xMin, &yMin, &xMax, &yMax);
            for ( auto line = blk->getLines(); line != nullptr ; line = line->getNext()){
                processTextLine(ofdOutputDev, line, bodyLayer);
            }
        }
    }
}

void OFDOutputDev::endPage() {
  if ( m_textPage != nullptr ){
    m_textPage->endPage();
    m_textPage->coalesce(gTrue, 0, gFalse);

    processTextPage(this, m_textPage, m_currentOFDPage);
  }
}

void OFDOutputDev::saveState(GfxState *state){
    if ( m_textPage != nullptr ){
        m_textPage->updateFont(state);
    }
}

void OFDOutputDev::restoreState(GfxState *state){
    if ( m_textPage != nullptr ){
        m_textPage->updateFont(state);
    }
}


void showGfxFont(GfxFont *gfxFont){

    Ref *ref = gfxFont->getID();
    GooString *family = gfxFont->getFamily();
    std::string fontFamily;
    if ( family != nullptr ){
        fontFamily = std::string(family->getCString());
    }

    GooString *name = gfxFont->getName();
    std::string fontName;
    if ( name != nullptr ){
        fontName = std::string(name->getCString());
    }

    GooString *encodingName = gfxFont->getEncodingName();
    std::string fontEncodingName;
    if ( encodingName != nullptr ){
        fontEncodingName = std::string(encodingName->getCString());
    }

    GooString *embFontName = gfxFont->getEmbeddedFontName();
    std::string fontEmbeddedName;
    if ( embFontName != nullptr ){
        fontEmbeddedName = std::string(embFontName->getCString());
    }

    Ref embID;
    embID.num = -1;
    embID.gen = -1;
    gfxFont->getEmbeddedFontID(&embID);


    GfxFontType fontType = gfxFont->getType();
    GfxFont::Stretch fontStretch = gfxFont->getStretch();
    GfxFont::Weight fontWeight = gfxFont->getWeight();

    bool isItalic = gfxFont->isItalic();
    bool isBold = gfxFont->isBold();

    double *fontMatrix = gfxFont->getFontMatrix();
    double *fontBBox = gfxFont->getFontBBox();

    std::string fontTypeName = "()";
    if ( fontType == fontCIDType2 ){
        fontTypeName = "(fontCIDType2)";
    }

    LOG(INFO) << "UpdateFont() \n"
        << "fontID(num,gen):(" << ref->num << ", " << ref->gen << ") \n"
        << "FontType: " << int(fontType) << fontTypeName << " \n"
        << "FontFamily:" << fontFamily << " \n"
        << "FontName:" << fontName << " \n"
        << "FontEncodingName:" << fontEncodingName << " \n"
        << "fontEmbeddedName:" << fontEmbeddedName << " \n"
        << "embID(num,gen): (" << embID.num << ", " << embID.gen << ") \n"
        << "FontStretch: " << fontStretch << " \n"
        << "FontWeight: " << fontWeight << " \n"
        << "isItalic: " << isItalic << " \n"
        << "isBold: " << isBold << " \n"
        << "fontMatrix: [" << fontMatrix[0] << ", " << fontMatrix[1] << ", " << fontMatrix[2] << ", "
        << fontMatrix[3] << ", " <<  fontMatrix[4] << ", " << fontMatrix[5] << "] \n"
        << "fontBBox: [" << fontBBox[0] << ", " << fontBBox[1] << ", " << fontBBox[2] << ", " << fontBBox[3] << "] \n"
        ;

}

void OFDOutputDev::PrintFonts() const{
    LOG(INFO) << m_fonts.size() << " fonts in pdf file.";
    for ( auto font : m_fonts ){
        LOG(INFO) << font.second->ToString();
    }
}

OFDFontPtr GfxFont_to_OFDFont(GfxFont *gfxFont, XRef *xref){
    OFDFontPtr ofdFont = std::make_shared<OFDFont>();

    // -------- FontID --------
    Ref *ref = gfxFont->getID();
    ofdFont->ID = ref->num;

    // -------- FontFamily --------
    GooString *family = gfxFont->getFamily();
    if ( family != nullptr ){
        ofdFont->FamilyName = std::string(family->getCString());
    }

    // -------- FontName --------
    GooString *name = gfxFont->getName();
    if ( name != nullptr ){
        ofdFont->FontName = std::string(name->getCString());
    }

    // -------- FontType --------
    GfxFontType fontType = gfxFont->getType();
    if ( fontType == fontCIDType2 ){
        ofdFont->FontType = ofd::Font::Type::CIDType2;
    } else if (fontType == fontType1 ){
        ofdFont->FontType = ofd::Font::Type::Type1;
    } else if (fontType == fontType3 ){
        ofdFont->FontType = ofd::Font::Type::Type3;
    } else if (fontType == fontTrueType ){
        ofdFont->FontType = ofd::Font::Type::TrueType;
    } else {
        ofdFont->FontType = ofd::Font::Type::Unknown;
    }

    // -------- FontLoc --------
    GfxFontLoc *fontLoc = gfxFont->locateFont(xref, nullptr);
    if ( fontLoc != nullptr ){
        if ( fontLoc->locType == gfxFontLocEmbedded ){
            ofdFont->FontLoc = ofd::Font::Location::Embedded;
        } else if ( fontLoc->locType == gfxFontLocExternal ){
            ofdFont->FontLoc = ofd::Font::Location::External;
            ofdFont->FontFile = std::string(fontLoc->path->getCString());
        } else if ( fontLoc->locType == gfxFontLocResident ){
            ofdFont->FontLoc = ofd::Font::Location::Resident;
        } else {
            ofdFont->FontLoc = ofd::Font::Location::Unknown;
        }
    } else {
        LOG(WARNING) << "fontLoc == nullptr.";
    }

    // -------- FontData --------
    int fontDataSize = 0;
    char *fontData = gfxFont->readEmbFontFile(xref, &fontDataSize);

    ofdFont->m_fontData = fontData;
    ofdFont->m_fontDataSize = fontDataSize;

    ofdFont->m_fontData = new char[fontDataSize];
    memcpy(ofdFont->m_fontData, fontData, fontDataSize);
    ofdFont->m_fontDataSize = fontDataSize;

    return ofdFont;
}

void OFDOutputDev::updateFont(GfxState *state){
    GfxFont *gfxFont = state->getFont();
    if ( gfxFont != nullptr ){
        Ref *ref = gfxFont->getID();
        int fontID = ref->num;

        OFDFontPtr ofdFont = nullptr;
        if ( m_fonts.find(fontID) == m_fonts.end() ){
            ofdFont = GfxFont_to_OFDFont(gfxFont, m_xref);

            m_fonts.insert(std::map<int, OFDFontPtr>::value_type(fontID, ofdFont));

            showGfxFont(gfxFont);

            //PrintFonts();
        } else {
            ofdFont = m_fonts[fontID];
        }

        m_currentFont = ofdFont;
        m_currentFontSize = state->getFontSize();
        m_currentCTM = state->getTextMat();
        //LOG(INFO) << "UpdateFont() fontSize: " << fontSize << " sizeof(ctm): " << sizeof(ctm);
        //LOG(INFO) << "ctm: [" << ctm[0] << ", " << ctm[1] << ", " << ctm[2] << ", "
            //<< ctm[3] << ", " << ctm[4] << ", " << ctm[5] << "]";
    }
}

//#include <UTF.h>
//#include <UTF8.h>
void OFDOutputDev::beginString(GfxState *state, GooString *s){

    //Unicode *uni = NULL;
    //__attribute__((unused)) int length = TextStringToUCS4(s, &uni);

    //mapUTF8(u, buf, bufSize);

    //gfree(uni);

    //LOG(INFO) << "beginString() : " << std::string((const char *)uni);
}

void OFDOutputDev::endString(GfxState *state){
}

void OFDOutputDev::drawChar(GfxState *state, double x, double y,
        double dx, double dy,
        double originX, double originY,
        CharCode code, int nBytes, Unicode *u, int uLen){
    //LOG(DEBUG) << "(x,y,dx,dy):" << std::setprecision(3) << "(" << x << ", " << y << ", " << dx << ", " << dy << ")" 
        //<< " (originX, originY):" << "(" << originX << ", " << originY << ")"
        //<< " code:" << std::hex << std::setw(4) << std::setfill('0') << code
        //<< " nBytes:" << nBytes << " uLen:" << uLen;

    //LOG(DEBUG) << "code:" << std::hex << std::setw(4) << std::setfill('0') << code << " nBytes:" << nBytes << " uLen:" << uLen;
    if ( m_textPage != nullptr ){
        //LOG(INFO) << "code:" << std::hex << std::setw(4) << std::setfill('0') << code << " nBytes:" << nBytes << " uLen:" << uLen;
        m_actualText->addChar(state, x, y, dx, dy, code, nBytes, u, uLen);
    }
}

void OFDOutputDev::incCharCount(int nChars){
    if ( m_textPage != nullptr ){
        m_textPage->incCharCount(nChars);
    }
}

//#include <UTF.h>
//void OFDOutputDev::beginActualText(GfxState *state, GooString *text) {
    //LOG(INFO) << "********************* beginActualText() ******************";
    //if ( m_textPage != nullptr ){
        //m_actualText->begin(state, text);

        //GooString *aText = new GooString(text);
        //Unicode *uni = NULL;
        //__attribute__((unused)) int length = TextStringToUCS4(aText, &uni);
        //gfree(uni);
        //delete aText;
        //LOG(INFO) << "beginActualText: " << std::string((const char*)text);
    //}
//}

//void OFDOutputDev::endActualText(GfxState *state) {
    //if ( m_textPage != nullptr ){
        //m_actualText->end(state);
    //}
//}

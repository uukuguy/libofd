#include <iostream>
#include <iomanip>
#include <assert.h>

#include <GlobalParams.h>

#include "OFDOutputDev.h"
#include "FontOutputDev.h"
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Font.h"
#include "utils/logger.h"


GBool rawOrder = gFalse;

bool ps = false;
bool eps = false;
bool pdf = false;
bool svg = false;

bool level2 = false;
bool duplex = false;

#include <cairo/cairo.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

extern "C"{
#include "utils/ffw.h"
}

std::ofstream cairoLogFile;

//class MyTextPage : public TextPage {
//public:
  //MyTextPage(OFDOutputDev * outputDev, GBool rawOrderA) : 
      //TextPage(rawOrderA), m_outputDev(outputDev){
  //}

  //OFDOutputDev *GetOutputDev() const {return m_outputDev;};

  //virtual void beginWord(GfxState *state);
  //virtual void endWord();

//private:
  //OFDOutputDev *m_outputDev;
  //GfxState *m_state;
//}; // class MyTextPage

//void MyTextPage::beginWord(GfxState *state){
    //m_state = state;
    //TextPage::beginWord(state);
//}

//void MyTextPage::endWord(){
    //m_outputDev->OnWord(curWord, m_state);
    //TextPage::endWord();
//}

using namespace ofd;

void OFDOutputDev::OnWord(TextWord *word, GfxState *state){
    GfxRGB strokeColor;
    GfxRGB fillColor;
    state->getStrokeRGB(&strokeColor);
    state->getFillRGB(&fillColor);

    //ExportWord(word, fillColor);

    const std::string myString = word->getText()->getCString();
    double fontSize = word->getFontSize();
    int numChars = word->getLength();
    LOG(INFO) << "........ OnWord() strokeColor=(" << strokeColor.r << "," << strokeColor.g << "," << strokeColor.b << ") fillColor=(" << fillColor.r << "," << fillColor.g << "," << fillColor.b << ")";
    LOG(INFO) << "TextWord FontSize=" << fontSize << " numChars=" << numChars << " len(string)=" << myString.length() << " size(string)=" << myString.size() << " word->getText()->getLength() = " << word->getText()->getLength();

}

// ======== OFDOutputDev::OFDOutputDev() ========
OFDOutputDev::OFDOutputDev(ofd::PackagePtr package) :
    m_pdfDoc(nullptr),
    m_xref(nullptr), m_textPage(nullptr), 
    m_actualText(nullptr),
    //m_imageSurface(nullptr),
    m_cairoRender(nullptr),
    m_package(package), m_document(nullptr), m_currentOFDPage(nullptr),
    m_currentFont(nullptr), m_currentFontSize(14.0), m_currentCTM(nullptr) {


    utils::ffw_init(false);
    cur_mapping.resize(0x10000);
    cur_mapping2.resize(0x100);
    width_list.resize(0x10000);

    // FIXME
    m_textPage = new TextPage(rawOrder);
    //m_textPage = new MyTextPage(this, rawOrder);

    m_actualText = new ActualText(m_textPage);
    m_textClipPath = nullptr;

    m_cropX = 0;
    m_cropY = 0;
    m_cropW = 0;
    m_cropH = 0;
    m_paperWidth = -1;
    m_paperHeight = -1;
    if ( m_paperWidth < 0 || m_paperHeight < 0)
        m_usePDFPageSize = true;
    else
        m_usePDFPageSize = false;

    m_printing = false;
    m_useCropBox = false;
    m_expand = false;
    m_noShrink = false;
    m_transp = false;
    m_noCenter = false;
    m_resolution = 0.0;
    m_resolutionX = 150.0;
    m_resolutionY = 150.0;

    if ( m_resolution != 0.0 && 
            (m_resolutionX == 150.0 || m_resolutionY == 150.0)) {
        m_resolutionX = m_resolution;
        m_resolutionY = m_resolution;
    }

    m_scaleTo= 0;
    m_scaleToX = 0;
    m_scaleToY = 0;

    m_outputSurface = nullptr;
    m_outputFile = nullptr;

    m_cairo = nullptr;
    m_cairoShape = nullptr;

    m_groupPattern = nullptr;
    m_shapePattern = nullptr;
    m_maskPattern = nullptr;
    m_cairoShapeSurface = nullptr;

    m_strokePattern = nullptr;
    m_fillPattern = nullptr;
    m_shading = nullptr;
    m_lineWidth = 1.0;
    m_strokeOpacity = 1.0;
    m_fillOpacity = 1.0;
    m_uncoloredPattern = false;
    m_strokeAdjust = globalParams->getStrokeAdjust();
    m_adjustedStrokeWidth = false;
    m_alignStrokeCoords = false;

    m_needFontUpdate = false;
    m_antialiasEnum = CAIRO_ANTIALIAS_DEFAULT;

    m_inUncoloredPattern = false;
    m_prescaleImages = true;

    m_useShowTextGlyphs = false;
    m_textMatrixValid = true;

    m_strokePathClip = nullptr;
    m_knockoutCount = 0;

    m_strokePathClip = nullptr;
    m_groupColorSpaceStack = nullptr;
    m_maskStack = nullptr;

    if ( package != nullptr ){
        m_document = package->AddNewDocument();
    }
}

// ======== OFDOutputDev::~OFDOutputDev() ========
OFDOutputDev::~OFDOutputDev(){

    utils::ffw_finalize();

    if ( m_cairo != nullptr ){
        cairo_destroy(m_cairo);
    }

    if ( m_strokePattern != nullptr ){
        cairo_pattern_destroy(m_strokePattern);
    }

    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
    }

    if ( m_groupPattern != nullptr ){
        cairo_pattern_destroy(m_groupPattern);
    }

    if ( m_maskPattern != nullptr ){
        cairo_pattern_destroy(m_maskPattern);
    }

    if ( m_shapePattern != nullptr ){
        cairo_pattern_destroy(m_shapePattern);
    }

    if ( m_textPage != nullptr ){ 
        m_textPage->decRefCnt();
    }
    if ( m_actualText != nullptr ){
        delete m_actualText;  
    }
}

std::tuple<double, double, double, double> getPageScaledSize(double originW, double originH, bool bRotate, double scaleTo, double scaleToX, double scaleToY){
    double pg_w = originW;
    double pg_h = originH;

    if ( bRotate ) {
        double tmp = pg_w; pg_w = pg_h; pg_h = tmp;
    }

    double resolution;
    double resolutionX = 150;
    double resolutionY = 150;

    // Scale page to pixel box.
    if ( scaleTo != 0 ){
        resolution = (72.0 * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
        resolutionX = resolutionY = resolution;
    } else {
        if ( scaleToX > 0) {
            resolutionX = (72.0 * scaleToX) / pg_w;
            if ( scaleToY == 0)
                resolutionY = resolutionX;
        }
        if ( scaleToX > 0) {
            resolutionY = (72.0 * scaleToY) / pg_h;
            if ( scaleToX == 0 ){
                resolutionX = resolutionY;
            }
        }
    }

    return std::make_tuple(pg_w, pg_h, resolutionX, resolutionY);
}

// -------- OFDOutputDev::getPageSize() --------
std::tuple<double, double> OFDOutputDev::getPageSize(PDFDocPtr pdfDoc, int pg, int firstPage){
    double pg_w, pg_h;
    if ( m_useCropBox) {
        pg_w = pdfDoc->getPageCropWidth(pg);
        pg_h = pdfDoc->getPageCropHeight(pg);
    } else {
        pg_w = pdfDoc->getPageMediaWidth(pg);
        pg_h = pdfDoc->getPageMediaHeight(pg);
    }
    if ( m_printing && pg == firstPage) {
        if (m_paperWidth < 0 || m_paperHeight < 0) {
            m_paperWidth = (int)ceil(pg_w);
            m_paperHeight = (int)ceil(pg_h);
        }
    }

    bool bRotate = false;
    //if ( ( pdfDoc->getPageRotate(pg) == 90 ) || 
            //( pdfDoc->getPageRotate(pg) == 270 ) ){
    if ( pdfDoc->getPageRotate(pg) != 0 ){
        bRotate = true;
    }

    std::tie(pg_w, pg_h, m_resolutionX, m_resolutionY) = getPageScaledSize(pg_w, pg_h, bRotate, m_scaleTo, m_scaleToX, m_scaleToY);
    //LOG(ERROR) << "pg_w=" << pg_w << " pg_h=" << pg_h << " m_resolutionX=" << m_resolutionX << " m_resolutionY=" << m_resolutionY;

    //if ( bRotate ){
        //double tmp = pg_w; pg_w = pg_h; pg_h = tmp;
    //}

    //// Scale page to pixel box.
    //if ( m_scaleTo != 0 ){
        //m_resolution = (72.0 * m_scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
        //m_resolutionX = m_resolutionY = m_resolution;
    //} else {
        //if ( m_scaleToX > 0) {
            //m_resolutionX = (72.0 * m_scaleToX) / pg_w;
            //if ( m_scaleToY == 0)
                //m_resolutionY = m_resolutionX;
        //}
        //if ( m_scaleToX > 0) {
            //m_resolutionY = (72.0 * m_scaleToY) / pg_h;
            //if ( m_scaleToX == 0 ){
                //m_resolutionX = m_resolutionY;
            //}
        //}
    //}

    //LOG(ERROR) << "pg_w=" << pg_w << " pg_h=" << pg_h << " m_resolutionX=" << m_resolutionX << " m_resolutionY=" << m_resolutionY;

    return std::make_tuple(pg_w, pg_h);
}

// -------- OFDOutputDev::preProcess() --------
void OFDOutputDev::preProcess(PDFDocPtr pdfDoc){

    m_fontOutputDev = std::make_shared<ofd::FontOutputDev>();
    m_fontOutputDev->ProcessDoc(pdfDoc);


    //ofdFont = GfxFont_to_OFDFont(gfxFont, m_xref);
    //commonData.DocumentRes->AddFont(ofdFont);
}

// ======== OFDOutputDev::ProcessDoc() ========
void OFDOutputDev::ProcessDoc(PDFDocPtr pdfDoc){
    if ( pdfDoc == nullptr ) return;
    m_pdfDoc = pdfDoc;
    m_xref = pdfDoc->getXRef();

    preProcess(pdfDoc);

    //double resolution = 72.0;
    //GBool useMediaBox = gTrue;
    //GBool crop = gFalse;
    //GBool printing = gTrue;

    auto numPages = pdfDoc->getNumPages();
    LOG(INFO) << "Total " << numPages << " pages in pdf file"; 

    int firstPage = 1;
    // FIXME
    for ( auto pg = firstPage ; pg <= numPages; pg++ ){
        //// FIXME debug 涠变色缺陷调试
        //if ( pg != 6 ) continue;


        // Cairo log file
        std::string cairoLogFileName = "/tmp/Page_" + std::to_string(pg) + ".cairo";
        cairoLogFile.open(cairoLogFileName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

        // -------- Page widht and height.
        double pg_w, pg_h;
        std::tie(pg_w, pg_h) = getPageSize(pdfDoc, pg, firstPage);

        // -------- Output widht and height.
        double output_w, output_h;
        getOutputSize(pg_w, pg_h, &output_w, &output_h);

        // -------- beforeDocument()
        if ( pg == firstPage ){
            cairo_surface_t *outputSurface = nullptr;
            FILE *outputFile = nullptr;

            std::string outputFileName = "output.pdf";
            std::tie(outputSurface, outputFile) = beforeDocument(outputFileName, output_w, output_h);

            m_outputSurface = outputSurface;
            m_outputFile = outputFile;
        }

        // -------- beforePage()
        beforePage(output_w, output_h);

        // -------- renderPage()
        renderPage(pg, pg_w, pg_h, output_w, output_h);

        // -------- afterPage()
        utils::MkdirIfNotExist("output");
        std::string strOutputImagePath = "output/pdf2ofd";
        utils::MkdirIfNotExist(strOutputImagePath);

        std::string imageFileName = strOutputImagePath + "/Page_" + std::to_string(pg) + ".jpeg";
        afterPage(imageFileName);


        cairoLogFile.write("Cairo", 5);
        cairoLogFile.close();
    }

    // -------- afterDocumnet()
    afterDocument();

    postProcess();
}

// -------- OFDOutputDev::postProcess() --------
void OFDOutputDev::postProcess(){
}

// -------- writeStream --------
static cairo_status_t writeStream(void *closure, const unsigned char *data, unsigned int length){
    FILE *file = (FILE *)closure;

    if (fwrite(data, length, 1, file) == 1)
        return CAIRO_STATUS_SUCCESS;
    else
        return CAIRO_STATUS_WRITE_ERROR;
}

// -------- OFDOutputDev::beforeDocument() --------
std::tuple<cairo_surface_t*, FILE*> OFDOutputDev::beforeDocument(const std::string &outputFileName, double w, double h) {

    cairo_surface_t *outputSurface = nullptr;
    FILE *outputFile = nullptr;

    if ( m_printing ) { 

        if (outputFileName == std::string("fd://0")){
            outputFile = stdout;
        } else {
            outputFile = fopen(outputFileName.c_str(), "wb");
            if (!outputFile) {
                LOG(ERROR) << "Error opening output file " << outputFileName;
                exit(2);
            }
        }

        if (ps || eps) {
            outputSurface = cairo_ps_surface_create_for_stream(writeStream, outputFile, w, h);
            if (level2)
                cairo_ps_surface_restrict_to_level (outputSurface, CAIRO_PS_LEVEL_2);
            if (eps)
                cairo_ps_surface_set_eps (outputSurface, 1);
            if (duplex) {
                cairo_ps_surface_dsc_comment(outputSurface, "%%Requirements: duplex");
                cairo_ps_surface_dsc_begin_setup(outputSurface);
                cairo_ps_surface_dsc_comment(outputSurface, "%%IncludeFeature: *Duplex DuplexNoTumble");
            }
            cairo_ps_surface_dsc_begin_page_setup (outputSurface);
        } else if (pdf) {
            outputSurface = cairo_pdf_surface_create_for_stream(writeStream, outputFile, w, h);
        } else if (svg) {
            outputSurface = cairo_svg_surface_create_for_stream(writeStream, outputFile, w, h);
            cairo_svg_surface_restrict_to_version (outputSurface, CAIRO_SVG_VERSION_1_2);
        }

    }

    return std::make_tuple(outputSurface, outputFile);
}

// -------- OFDOutputDev::afterDocumnet() --------
void OFDOutputDev::afterDocument(){
    cairo_surface_t *outputSurface = m_outputSurface;
    FILE *outputFile = m_outputFile;

    if ( m_printing ) {

        if ( outputSurface != nullptr ){
          cairo_surface_finish(outputSurface);
          cairo_status_t status = cairo_surface_status(outputSurface);
          if (status){
              LOG(ERROR) << "cairo error: " << cairo_status_to_string(status);
          }
          cairo_surface_destroy(outputSurface);
        }

        if ( outputFile != nullptr ){
          fclose(outputFile);
        }
    }
}

// -------- OFDOutputDev::beforePage() --------
void OFDOutputDev::beforePage(double w, double h) {
    if ( m_printing ){
        if ( ps || eps ) {
            if (w > h) {
                cairo_ps_surface_dsc_comment(m_outputSurface, "%%PageOrientation: Landscape");
                cairo_ps_surface_set_size(m_outputSurface, h, w);
            } else {
                cairo_ps_surface_dsc_comment(m_outputSurface, "%%PageOrientation: Portrait");
                cairo_ps_surface_set_size(m_outputSurface, w, h);
            }
        } else if ( pdf ) {
            cairo_pdf_surface_set_size(m_outputSurface, w, h);
        }

        cairo_surface_set_fallback_resolution(m_outputSurface, m_resolutionX, m_resolutionY);

    } else {
        m_outputSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(w), ceil(h));

        //int imageWidth = w;//794;
        //int imageHeight = h;//1122;
        //m_imageSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
        //if ( m_imageSurface == nullptr ){
            //LOG(ERROR) << "create_image_surface() failed. ";
            //return;
        //}
        //m_cairoRender = std::make_shared<OFDCairoRender>(m_imageSurface);
        m_cairoRender = std::make_shared<ofd::CairoRender>(w, h, m_resolutionX, m_resolutionY);
    }
}

// -------- OFDOutputDev::afterPage() --------
void OFDOutputDev::afterPage(const std::string &imageFileName){
    if ( m_printing ){
        cairo_surface_show_page(m_outputSurface);
    } else {
        // TODO
        writeCairoSurfaceImage(m_outputSurface, imageFileName);
        cairo_surface_finish(m_outputSurface);
        cairo_status_t status = cairo_surface_status(m_outputSurface);
        if (status){
            LOG(ERROR) << "cairo error: " << cairo_status_to_string(status);
        }
        cairo_surface_destroy(m_outputSurface);


        if ( m_cairoRender != nullptr ){
            uint64_t pageID = m_currentOFDPage->ID;
            std::string png_filename = std::string("output/pdf2ofd/Page") + std::to_string(pageID) + ".png";
            m_cairoRender->WriteToPNG(png_filename);
            m_cairoRender = nullptr;

            //cairo_surface_destroy(m_imageSurface);
            //m_imageSurface = nullptr;
        }
    }
}

// -------- OFDOutputDev::renderPage() --------
void OFDOutputDev::renderPage(int pg, double page_w, double page_h, double output_w, double output_h) {
    cairo_t *cr;
    cairo_status_t status;
    cairo_matrix_t m;

    cr = cairo_create(m_outputSurface);

    this->SetCairo(cr);
    //this->setPrinting(printing);
    this->SetAntialias(m_antialiasEnum);

    cairo_save(cr);
    if ( m_cairoRender != nullptr ){
        m_cairoRender->SaveState();
    }

    if ( ps && output_w > output_h) {
        // rotate 90 deg for landscape
        cairo_translate (cr, 0, output_w);
        cairo_matrix_init (&m, 0, -1, 1, 0, 0, 0);
        cairo_transform (cr, &m);
    }
    cairo_translate (cr, -m_cropX, -m_cropY);
    if ( m_printing ) {
        double cropped_w, cropped_h;
        getCropSize(page_w, page_h, &cropped_w, &cropped_h);
        getFitToPageTransform(cropped_w, cropped_h, output_w, output_h, &m);
        cairo_transform (cr, &m);
        cairo_rectangle(cr, m_cropX, m_cropY, cropped_w, cropped_h);
        cairo_clip(cr);
    } else {
        cairo_scale (cr, m_resolutionX/72.0, m_resolutionY/72.0);
    }

    double resolution = 72.0;
    GBool useMediaBox = !m_useCropBox;
    GBool crop = m_useCropBox;
    GBool printing = m_printing;
    m_pdfDoc->displayPage(this, pg, resolution, resolution, 0, useMediaBox, crop, printing);
    //m_pdfDoc->displayPageSlice(this,
            //pg,
            //72.0, 72.0,
            //0, [> rotate <]
            //!m_useCropBox, [> useMediaBox <]
            //gFalse, [> Crop <]
            //m_printing,
            //-1, -1, -1, -1);
    cairo_restore(cr);
    if ( m_cairoRender != nullptr ){
        m_cairoRender->RestoreState();
    }
    this->SetCairo(nullptr);

    // Blend onto white page
    if (!m_printing && !m_transp) {
        cairo_save(cr);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->SaveState();
        }

        cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_paint(cr);
        cairo_restore(cr);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->RestoreState();
        }

    }

    status = cairo_status(cr);
    if (status) {
        LOG(ERROR) << "cairo error: " << cairo_status_to_string(status);
    }
    cairo_destroy (cr);
}

#include <iostream>
#include <iomanip>
#include <assert.h>

#include <fofi/FoFiTrueType.h>
#include <GlobalParams.h>
#include <UnicodeMap.h>

#include "OFDCommon.h"
#include "OFDOutputDev.h"
#include "OFDPage.h"
#include "OFDTextObject.h"
#include "utils/logger.h"

using namespace ofd;

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

OFDOutputDev::OFDOutputDev(ofd::OFDPackagePtr ofdPackage) :
    m_pdfDoc(nullptr),
    m_xref(nullptr), m_textPage(nullptr), 
    m_actualText(nullptr),
    m_ofdPackage(ofdPackage), m_ofdDocument(nullptr), m_currentOFDPage(nullptr),
    m_currentFont(nullptr), m_currentFontSize(14.0), m_currentCTM(nullptr) {

    m_textPage = new TextPage(rawOrder);
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
    m_strokePattern = nullptr;
    m_fillPattern = nullptr;
    m_strokeOpacity = 1.0;
    m_fillOpacity = 1.0;
    m_uncoloredPattern = false;
    m_adjustedStrokeWidth = false;
    m_strokeAdjust = globalParams->getStrokeAdjust();
    m_antialiasEnum = CAIRO_ANTIALIAS_DEFAULT;

    m_use_show_text_glyphs = false;
    m_textMatrixValid = true;

    if ( ofdPackage != nullptr ){
        m_ofdDocument = ofdPackage->AddNewDocument();
    }
}

OFDOutputDev::~OFDOutputDev(){

    if ( m_cairo != nullptr ){
        cairo_destroy(m_cairo);
    }

    if ( m_strokePattern != nullptr ){
        cairo_pattern_destroy(m_strokePattern);
    }

    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
    }

    if ( m_textPage != nullptr ){ 
        m_textPage->decRefCnt();
    }
    if ( m_actualText != nullptr ){
        delete m_actualText;  
    }
}

void OFDOutputDev::getCropSize(double page_w, double page_h, double *width, double *height){
  int w = m_cropW;
  int h = m_cropH;

  if (w == 0)
    w = (int)ceil(page_w);

  if (h == 0)
    h = (int)ceil(page_h);

  *width =  (m_cropX + w > page_w ? (int)ceil(page_w - m_cropX) : w);
  *height = (m_cropY + h > page_h ? (int)ceil(page_h - m_cropY) : h);
}

void OFDOutputDev::getOutputSize(double page_w, double page_h, double *width, double *height){
    if ( m_printing ) {
        if (m_usePDFPageSize) {
            *width = page_w;
            *height = page_h;
        } else {
            if (page_w > page_h) {
                *width = m_paperHeight;
                *height = m_paperWidth;
            } else {
                *width = m_paperWidth;
                *height = m_paperHeight;
            }
        }
    } else {
        getCropSize(page_w * (m_resolutionX / 72.0), page_h * (m_resolutionY / 72.0), width, height);
    }
}

void OFDOutputDev::getFitToPageTransform(double page_w, double page_h, double paper_w, double paper_h, cairo_matrix_t *m) {
  double x_scale, y_scale, scale;

  x_scale = paper_w / page_w;
  y_scale = paper_h / page_h;
  if (x_scale < y_scale)
    scale = x_scale;
  else
    scale = y_scale;

  if (scale > 1.0 && !m_expand)
    scale = 1.0;
  if (scale < 1.0 && m_noShrink)
    scale = 1.0;

  cairo_matrix_init_identity (m);
  if (!m_noCenter) {
    // centre page
    cairo_matrix_translate(m, (paper_w - page_w*scale)/2, (paper_h - page_h*scale)/2);
  } else if (!svg){
    // move to PostScript origin
    cairo_matrix_translate (m, 0, (paper_h - page_h*scale));
  }
  cairo_matrix_scale (m, scale, scale);
}

void OFDOutputDev::ProcessDoc(PDFDocPtr pdfDoc){
    if ( pdfDoc == nullptr ) return;
    m_pdfDoc = pdfDoc;

    //double resolution = 72.0;
    //GBool useMediaBox = gTrue;
    //GBool crop = gFalse;
    //GBool printing = gTrue;

    auto numPages = pdfDoc->getNumPages();
    LOG(INFO) << "Total " << numPages << " pages in pdf file"; 

    int firstPage = 1;
    for ( auto pg = firstPage ; pg <= numPages ; pg++ ){
        // Page widht and height.
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

        if ( ( pdfDoc->getPageRotate(pg) == 90 ) || 
                ( pdfDoc->getPageRotate(pg) == 270 ) ){
            double tmp = pg_w; pg_w = pg_h; pg_h = tmp;
        }

        // Scale page to pixel box.
        if ( m_scaleTo != 0 ){
            m_resolution = (72.0 * m_scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
            m_resolutionX = m_resolutionY = m_resolution;
        } else {
            if ( m_scaleToX > 0) {
                m_resolutionX = (72.0 * m_scaleToX) / pg_w;
                if ( m_scaleToY == 0)
                    m_resolutionY = m_resolutionX;
            }
            if ( m_scaleToX > 0) {
                m_resolutionY = (72.0 * m_scaleToY) / pg_h;
                if ( m_scaleToX == 0 ){
                    m_resolutionX = m_resolutionY;
                }
            }
        }

        double output_w, output_h;
        getOutputSize(pg_w, pg_h, &output_w, &output_h);

        if ( pg == firstPage ){
            cairo_surface_t *outputSurface = nullptr;
            FILE *outputFile = nullptr;

            std::string inputFileName;
            std::string outputFileName;
            std::tie(outputSurface, outputFile) = beforeDocument(inputFileName, outputFileName, output_w, output_h);

            m_outputSurface = outputSurface;
            m_outputFile = outputFile;
        }

        beforePage(output_w, output_h);

        renderPage(pg, pg_w, pg_h, output_w, output_h);
        //pdfDoc->displayPage(this, pg, resolution, resolution, 0, useMediaBox, crop, printing);

        std::string imageFileName;
        assert(!imageFileName.empty());
        afterPage(imageFileName);
    }
    afterDocument();
}

static cairo_status_t writeStream(void *closure, const unsigned char *data, unsigned int length){
    FILE *file = (FILE *)closure;

    if (fwrite(data, length, 1, file) == 1)
        return CAIRO_STATUS_SUCCESS;
    else
        return CAIRO_STATUS_WRITE_ERROR;
}

std::tuple<cairo_surface_t*, FILE*> OFDOutputDev::beforeDocument(const std::string &inputFileName, const std::string &outputFileName, double w, double h) {

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
#if CAIRO_HAS_PS_SURFACE
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
#endif
        } else if (pdf) {
#if CAIRO_HAS_PDF_SURFACE
            outputSurface = cairo_pdf_surface_create_for_stream(writeStream, outputFile, w, h);
#endif
        } else if (svg) {
#if CAIRO_HAS_SVG_SURFACE
            outputSurface = cairo_svg_surface_create_for_stream(writeStream, outputFile, w, h);
            cairo_svg_surface_restrict_to_version (outputSurface, CAIRO_SVG_VERSION_1_2);
#endif
        }

    }

    return std::make_tuple(outputSurface, outputFile);
}

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

void OFDOutputDev::beforePage(double w, double h)
{
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
    }
}

void OFDOutputDev::afterPage(const std::string &imageFileName){
    if ( m_printing ){
        cairo_surface_show_page(m_outputSurface);
    } else {
        // TODO
        //writePageImage(imageFileName);
        cairo_surface_finish(m_outputSurface);
        cairo_status_t status = cairo_surface_status(m_outputSurface);
        if (status){
            LOG(ERROR) << "cairo error: " << cairo_status_to_string(status);
        }
        cairo_surface_destroy(m_outputSurface);
    }
}

void OFDOutputDev::renderPage(int pg, double page_w, double page_h, double output_w, double output_h) {
    cairo_t *cr;
    cairo_status_t status;
    cairo_matrix_t m;

    cr = cairo_create(m_outputSurface);

    this->SetCairo(cr);
    //this->setPrinting(printing);
    this->SetAntialias(m_antialiasEnum);

    cairo_save(cr);
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

    m_pdfDoc->displayPageSlice(this,
            pg,
            72.0, 72.0,
            0, /* rotate */
            !m_useCropBox, /* useMediaBox */
            gFalse, /* Crop */
            m_printing,
            -1, -1, -1, -1);
    cairo_restore(cr);
    this->SetCairo(nullptr);

    // Blend onto white page
    if (!m_printing && !m_transp) {
        cairo_save(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    status = cairo_status(cr);
    if (status) {
        LOG(ERROR) << "cairo error: " << cairo_status_to_string(status);
    }
    cairo_destroy (cr);
}

void setContextAntialias(cairo_t *cr, cairo_antialias_t antialias) {
    cairo_set_antialias(cr, antialias);

    cairo_font_options_t *font_options = cairo_font_options_create();

    cairo_get_font_options(cr, font_options);
    cairo_font_options_set_antialias(font_options, antialias);
    cairo_set_font_options(cr, font_options);

    cairo_font_options_destroy(font_options);
}

void OFDOutputDev::SetCairo(cairo_t *cairo) {
    if ( m_cairo != nullptr) {
        cairo_status_t status = cairo_status(m_cairo);
        if (status) {
            LOG(ERROR) << "cairo context error: {0:s}\n" << cairo_status_to_string(status);
        }
        cairo_destroy (m_cairo);
        assert(!m_cairoShape);
    }
    if (cairo != nullptr) {
        m_cairo = cairo_reference(cairo);
        /* save the initial matrix so that we can use it for type3 fonts. */
        //XXX: is this sufficient? could we miss changes to the matrix somehow?
        cairo_get_matrix(cairo, &m_origMatrix);
        setContextAntialias(cairo, m_antialias);
    } else {
        m_cairo = nullptr;
        m_cairoShape = nullptr;
    }
}

void OFDOutputDev::SetAntialias(cairo_antialias_t antialias)
{
  m_antialias = antialias;
  if ( m_cairo != nullptr ){
      setContextAntialias(m_cairo, antialias);
  }
  if ( m_cairoShape != nullptr ){
      setContextAntialias(m_cairoShape, antialias);
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

void OFDOutputDev::processTextLine(TextLine *line, OFDLayerPtr bodyLayer){
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
        }

        TextFontInfo *fontInfo = word->getFontInfo(0);
        Ref *ref = fontInfo->gfxFont->getID();
        uint64_t fontID = ref->num;

        OFDDocument::CommonData &commonData = m_ofdDocument->GetCommonData();
        assert(commonData.DocumentRes != nullptr );
        auto textFont = commonData.DocumentRes->GetFont(fontID);

        std::stringstream ss;
        ss << "          <word xMin=\"" << xMin << "\" yMin=\"" << yMin << "\" xMax=\"" <<
            xMax << "\" yMax=\"" << yMax << "\">" << myString << "</word>\n";
        LOG(DEBUG) << ss.str();

        wordXML << ss.str();

        if ( bodyLayer != nullptr ){
            OFDTextObject *textObject = new OFDTextObject(m_currentOFDPage);

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

void OFDOutputDev::processTextPage(TextPage *textPage, OFDPagePtr currentOFDPage){
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
                processTextLine(line, bodyLayer);
            }
        }
    }
}

void OFDOutputDev::endPage() {
  if ( m_textPage != nullptr ){
    m_textPage->endPage();
    m_textPage->coalesce(gTrue, 0, gFalse);

    processTextPage(m_textPage, m_currentOFDPage);
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

std::tuple<int*, size_t> getCodeToGID(GfxFont *gfxFont, char *fontData, size_t fontDataLen) {
    assert(gfxFont != nullptr);
    assert(fontData != nullptr && fontDataLen > 0);

    int *codeToGID = nullptr;
    size_t codeToGIDLen = 0;
    FoFiTrueType *ff;
    GfxFontType fontType = gfxFont->getType();

    switch ( fontType ){
        case fontCIDType2:
        case fontCIDType2OT: {
                codeToGID = NULL;
                int n = 0;
                if (((GfxCIDFont *)gfxFont)->getCIDToGID()) {
                    n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
                    if (n) {
                        codeToGID = (int *)gmallocn(n, sizeof(int));
                        memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(), n * sizeof(int));
                    }
                } else {
                    ff = FoFiTrueType::make(fontData, fontDataLen);
                    if ( ff != nullptr ){
                        codeToGID = ((GfxCIDFont *)gfxFont)->getCodeToGIDMap(ff, &n);
                        delete ff;
                    } else {
                        LOG(ERROR) << "FofiTrueType make or load failed.";
                    }
                }
                codeToGIDLen = n;
            } break;
        default:
            break;
    }; // switch ( fontType ){

    return std::make_tuple(codeToGID, codeToGIDLen);
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
        delete fontLoc;
        fontLoc = nullptr;
    } else {
        LOG(WARNING) << "fontLoc == nullptr.";
    }

    // -------- FontData --------
    int fontDataSize = 0;
    char *fontData = gfxFont->readEmbFontFile(xref, &fontDataSize);

    ofdFont->m_fontData = fontData;
    ofdFont->m_fontDataSize = fontDataSize;

    ofdFont->m_fontData = fontData;
    ofdFont->m_fontDataSize = fontDataSize;

    int *codeToGID = nullptr;
    size_t codeToGIDLen = 0;
    std::tie(codeToGID, codeToGIDLen) = getCodeToGID(gfxFont, fontData, fontDataSize);

    return ofdFont;
}

void OFDOutputDev::updateFont(GfxState *state){
    GfxFont *gfxFont = state->getFont();
    if ( gfxFont != nullptr ){

        m_use_show_text_glyphs = gfxFont->hasToUnicodeCMap() && cairo_surface_has_show_text_glyphs (cairo_get_target(m_cairo));

        Ref *ref = gfxFont->getID();
        int fontID = ref->num;

        OFDFontPtr ofdFont = nullptr;
        OFDDocument::CommonData &commonData = m_ofdDocument->GetCommonData();
        assert(commonData.DocumentRes != nullptr );
        ofdFont = commonData.DocumentRes->GetFont(fontID);

        if ( ofdFont == nullptr ){
            ofdFont = GfxFont_to_OFDFont(gfxFont, m_xref);
            commonData.DocumentRes->AddFont(ofdFont);
            showGfxFont(gfxFont);
        }

        m_currentFont = ofdFont;

        m_currentFontSize = state->getFontSize();
        m_currentCTM = state->getTextMat();
        //LOG(INFO) << "UpdateFont() fontSize: " << fontSize << " sizeof(ctm): " << sizeof(ctm);
        //LOG(INFO) << "ctm: [" << ctm[0] << ", " << ctm[1] << ", " << ctm[2] << ", "
            //<< ctm[3] << ", " << ctm[4] << ", " << ctm[5] << "]";
    }
}

void OFDOutputDev::setDefaultCTM(double *ctm) {
  cairo_matrix_t matrix;
  matrix.xx = ctm[0];
  matrix.yx = ctm[1];
  matrix.xy = ctm[2];
  matrix.yy = ctm[3];
  matrix.x0 = ctm[4];
  matrix.y0 = ctm[5];

  cairo_transform(m_cairo, &matrix);
  if (m_cairoShape){
      cairo_transform(m_cairoShape, &matrix);
  }

  OutputDev::setDefaultCTM(ctm);
}

void OFDOutputDev::updateCTM(GfxState *state, double m11, double m12, double m21, double m22, double m31, double m32) {
  cairo_matrix_t matrix, invert_matrix;
  matrix.xx = m11;
  matrix.yx = m12;
  matrix.xy = m21;
  matrix.yy = m22;
  matrix.x0 = m31;
  matrix.y0 = m32;

  /* Make sure the matrix is invertible before setting it.
   * cairo will blow up if we give it a matrix that's not
   * invertible, so we need to check before passing it
   * to cairo_transform. Ignoring it is likely to give better
   * results than not rendering anything at all. See #14398
   *
   * Ideally, we could do the cairo_transform
   * and then check if anything went wrong and fix it then
   * instead of having to invert the matrix. */
  invert_matrix = matrix;
  if (cairo_matrix_invert(&invert_matrix)) {
    LOG(ERROR) << "matrix not invertible\n";
    return;
  }

  cairo_transform(m_cairo, &matrix);
  if (m_cairoShape != nullptr){
    cairo_transform(m_cairoShape, &matrix);
  }
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
}

void OFDOutputDev::updateLineDash(GfxState *state){
  double *dashPattern;
  int dashLength;
  double dashStart;

  state->getLineDash(&dashPattern, &dashLength, &dashStart);
  cairo_set_dash(m_cairo, dashPattern, dashLength, dashStart);
  if (m_cairoShape != nullptr ){
    cairo_set_dash(m_cairoShape, dashPattern, dashLength, dashStart);
  }
}

void OFDOutputDev::updateLineJoin(GfxState *state){
  switch (state->getLineJoin()){
      case 0:
          cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_MITER);
          break;
      case 1:
          cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_ROUND);
          break;
      case 2:
          cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_BEVEL);
          break;
  }
  if ( m_cairoShape != nullptr ){
    cairo_set_line_join(m_cairoShape, cairo_get_line_join(m_cairo));
  }
}

void OFDOutputDev::updateLineCap(GfxState *state){
  switch (state->getLineCap()) {
  case 0:
    cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_BUTT);
    break;
  case 1:
    cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_ROUND);
    break;
  case 2:
    cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_SQUARE);
    break;
  }
  if ( m_cairoShape != nullptr ){
    cairo_set_line_cap(m_cairoShape, cairo_get_line_cap(m_cairo));
  }
}

void OFDOutputDev::updateMiterLimit(GfxState *state){
  cairo_set_miter_limit(m_cairo, state->getMiterLimit());
  if ( m_cairoShape != nullptr ){
    cairo_set_miter_limit(m_cairoShape, state->getMiterLimit());
  }
}

#define MIN(a,b) a <= b ? a : b

void OFDOutputDev::updateLineWidth(GfxState *state){
  LOG(DEBUG) <<  "line width: " << state->getLineWidth();
  m_adjustedStrokeWidth = false;
  double width = state->getLineWidth();
  if ( m_strokeAdjust && !m_printing ) {
    double x, y;
    x = y = width;

    /* find out line width in device units */
    cairo_user_to_device_distance(m_cairo, &x, &y);
    if (fabs(x) <= 1.0 && fabs(y) <= 1.0) {
      /* adjust width to at least one device pixel */
      x = y = 1.0;
      cairo_device_to_user_distance(m_cairo, &x, &y);
      width = MIN(fabs(x),fabs(y));
      m_adjustedStrokeWidth = true;
    }
  } else if (width == 0.0) {
    /* Cairo does not support 0 line width == 1 device pixel. Find out
     * how big pixels (device unit) are in the x and y
     * directions. Choose the smaller of the two as our line width.
     */
    double x = 1.0, y = 1.0;
    if (m_printing) {
      // assume printer pixel size is 1/600 inch
      x = 72.0/600;
      y = 72.0/600;
    }
    cairo_device_to_user_distance(m_cairo, &x, &y);
    width = MIN(fabs(x),fabs(y));
  }
  cairo_set_line_width(m_cairo, width);
  if (m_cairoShape)
    cairo_set_line_width(m_cairoShape, cairo_get_line_width (m_cairo));
}

void OFDOutputDev::updateFillColor(GfxState *state){
    GfxRGB color = m_fillColor;

    if ( m_uncoloredPattern ) return;

    state->getFillRGB(&m_fillColor);
    if (cairo_pattern_get_type(m_fillPattern) != CAIRO_PATTERN_TYPE_SOLID ||
            color.r != m_fillColor.r ||
            color.g != m_fillColor.g ||
            color.b != m_fillColor.b)
    {
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = cairo_pattern_create_rgba(colToDbl(m_fillColor.r),
                colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b),
                m_fillOpacity);

        LOG(DEBUG) <<  "fill color: " << m_fillColor.r << ", " <<  m_fillColor.g << ", " << m_fillColor.b;
    }
}

void OFDOutputDev::updateStrokeColor(GfxState *state){
    GfxRGB color = m_strokeColor;

    if ( m_uncoloredPattern ) return;

    state->getStrokeRGB(&m_strokeColor);
    if (cairo_pattern_get_type(m_fillPattern) != CAIRO_PATTERN_TYPE_SOLID ||
            color.r != m_strokeColor.r ||
            color.g != m_strokeColor.g ||
            color.b != m_strokeColor.b)
    {
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = cairo_pattern_create_rgba(colToDbl(m_strokeColor.r),
                colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b),
                m_strokeOpacity);

        LOG(DEBUG) <<  "stroke color: " << m_strokeColor.r << ", " << m_strokeColor.g << ", " <<  m_strokeColor.b;
    }
}

void OFDOutputDev::updateFillOpacity(GfxState *state){
    double opacity = m_fillOpacity;

    if ( m_uncoloredPattern ) return;

    m_fillOpacity = state->getFillOpacity();
    if ( opacity != m_fillOpacity) {
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = cairo_pattern_create_rgba(colToDbl(m_fillColor.r),
                colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b),
                m_fillOpacity);

        LOG(DEBUG) << "fill opacity: " << m_fillOpacity;
    }
}

void OFDOutputDev::updateStrokeOpacity(GfxState *state){
    double opacity = m_strokeOpacity;

    if ( m_uncoloredPattern ) return;

    m_strokeOpacity = state->getStrokeOpacity();
    if ( opacity != m_strokeOpacity) {
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = cairo_pattern_create_rgba(colToDbl(m_strokeColor.r),
                colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b),
                m_strokeOpacity);

        LOG(DEBUG) <<  "stroke opacity: " << m_strokeOpacity;
    }
}

void OFDOutputDev::updateFillColorStop(GfxState *state, double offset){
    if ( m_uncoloredPattern ) return;

    state->getFillRGB(&m_fillColor);

    cairo_pattern_add_color_stop_rgba(m_fillPattern, offset,
            colToDbl(m_fillColor.r),
            colToDbl(m_fillColor.g),
            colToDbl(m_fillColor.b),
            m_fillOpacity);
    LOG(DEBUG) << "fill color stop: " << offset << " (" <<
        m_fillColor.r << ", " <<
        m_fillColor.g << ", " <<
        m_fillColor.b;
}

void OFDOutputDev::updateBlendMode(GfxState *state){
    switch (state->getBlendMode()) {
        default:
        case gfxBlendNormal:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_OVER);
            break;
        case gfxBlendMultiply:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_MULTIPLY);
            break;
        case gfxBlendScreen:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_SCREEN);
            break;
        case gfxBlendOverlay:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_OVERLAY);
            break;
        case gfxBlendDarken:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_DARKEN);
            break;
        case gfxBlendLighten:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_LIGHTEN);
            break;
        case gfxBlendColorDodge:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_COLOR_DODGE);
            break;
        case gfxBlendColorBurn:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_COLOR_BURN);
            break;
        case gfxBlendHardLight:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HARD_LIGHT);
            break;
        case gfxBlendSoftLight:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_SOFT_LIGHT);
            break;
        case gfxBlendDifference:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_DIFFERENCE);
            break;
        case gfxBlendExclusion:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_EXCLUSION);
            break;
        case gfxBlendHue:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_HUE);
            break;
        case gfxBlendSaturation:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_SATURATION);
            break;
        case gfxBlendColor:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_COLOR);
            break;
        case gfxBlendLuminosity:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_LUMINOSITY);
            break;
    }

    LOG(DEBUG) << "blend mode: " << (int)state->getBlendMode();
}

//#include <UTF.h>
//#include <UTF8.h>
void OFDOutputDev::beginString(GfxState *state, GooString *s){

    //Unicode *uni = NULL;
    //__attribute__((unused)) int length = TextStringToUCS4(s, &uni);

    //mapUTF8(u, buf, bufSize);

    //gfree(uni);

    //LOG(INFO) << "beginString() : " << std::string((const char *)uni);

    int len = s->getLength();

    //if (needFontUpdate)
        //updateFont(state);

    if (!m_currentFont)
        return;

    m_cairoGlyphs = (cairo_glyph_t *) gmallocn (len, sizeof (cairo_glyph_t));
    m_glyphsCount = 0;
    if (m_use_show_text_glyphs) {
        m_cairoTextClusters = (cairo_text_cluster_t *) gmallocn (len, sizeof (cairo_text_cluster_t));
        m_clustersCount = 0;
        m_utf8Max = len*2; // start with twice the number of glyphs. we will realloc if we need more.
        m_utf8 = (char *) gmalloc (m_utf8Max);
        m_utf8Count = 0;
    }
}

void OFDOutputDev::endString(GfxState *state){

    int render;

    if (!m_currentFont)
        return;

    // endString can be called without a corresponding beginString. If this
    // happens glyphs will be null so don't draw anything, just return.
    // XXX: OutputDevs should probably not have to deal with this...
    if (!m_cairoGlyphs)
        return;

    // ignore empty strings and invisible text -- this is used by
    // Acrobat Capture
    render = state->getRender();
    if (render == 3 || m_glyphsCount == 0 || !m_textMatrixValid) {
        goto finish;
    }

    if (!(render & 1)) {
        //LOG (printf ("fill string\n"));
        cairo_set_source (m_cairo, m_fillPattern);
        if (m_use_show_text_glyphs)
            cairo_show_text_glyphs (m_cairo, m_utf8, m_utf8Count, m_cairoGlyphs, m_glyphsCount, m_cairoTextClusters, m_clustersCount, (cairo_text_cluster_flags_t)0);
        else
            cairo_show_glyphs (m_cairo, m_cairoGlyphs, m_glyphsCount);
        if (m_cairoShape)
            cairo_show_glyphs (m_cairoShape, m_cairoGlyphs, m_glyphsCount);
    }

    // stroke
    if ((render & 3) == 1 || (render & 3) == 2) {
        LOG(DEBUG) << "stroke string";
        cairo_set_source (m_cairo, m_strokePattern);
        cairo_glyph_path (m_cairo, m_cairoGlyphs, m_glyphsCount);
        cairo_stroke (m_cairo);
        if (m_cairoShape) {
            cairo_glyph_path (m_cairoShape, m_cairoGlyphs, m_glyphsCount);
            cairo_stroke (m_cairoShape);
        }
    }

    // clip
    if ((render & 4)) {
        LOG(DEBUG) << "clip string";
        // append the glyph path to m_textClipPath.

        // set textClipPath as the currentPath
        if (m_textClipPath != nullptr ) {
            cairo_append_path (m_cairo, m_textClipPath);
            if (m_cairoShape) {
                cairo_append_path (m_cairoShape, m_textClipPath);
            }
            cairo_path_destroy (m_textClipPath);
        }

        // append the glyph path
        cairo_glyph_path (m_cairo, m_cairoGlyphs, m_glyphsCount);

        // move the path back into textClipPath 
        // and clear the current path
        m_textClipPath = cairo_copy_path (m_cairo);
        cairo_new_path (m_cairo);
        if (m_cairoShape) {
            cairo_new_path (m_cairoShape);
        }
    }

finish:
    gfree (m_cairoGlyphs);
    m_cairoGlyphs = nullptr;
    if (m_use_show_text_glyphs) {
        gfree (m_cairoTextClusters);
        m_cairoTextClusters = nullptr;
        gfree (m_utf8);
        m_utf8 =  nullptr;
    }
}

void OFDOutputDev::beginTextObject(GfxState *state) {
}

void OFDOutputDev::endTextObject(GfxState *state) {
    if (m_textClipPath) {
        // clip the accumulated text path
        cairo_append_path (m_cairo, m_textClipPath);
        cairo_clip (m_cairo);
        if (m_cairoShape) {
            cairo_append_path (m_cairoShape, m_textClipPath);
            cairo_clip (m_cairoShape);
        }
        cairo_path_destroy (m_textClipPath);
        m_textClipPath = NULL;
    }
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


    if ( m_currentFont != nullptr ) {
        m_cairoGlyphs[m_glyphsCount].index = m_currentFont->GetGlyph (code, u, uLen);
        m_cairoGlyphs[m_glyphsCount].x = x - originX;
        m_cairoGlyphs[m_glyphsCount].y = y - originY;
        m_glyphsCount++;
        if (m_use_show_text_glyphs) {
            GooString enc("UTF-8");
            UnicodeMap *utf8Map = globalParams->getUnicodeMap(&enc);
            if (m_utf8Max - m_utf8Count < uLen*6) {
                // utf8 encoded characters can be up to 6 bytes
                if (m_utf8Max > uLen*6)
                    m_utf8Max *= 2;
                else
                    m_utf8Max += 2*uLen*6;
                m_utf8 = (char *) grealloc (m_utf8, m_utf8Max);
            }
            m_cairoTextClusters[m_clustersCount].num_bytes = 0;
            for (int i = 0; i < uLen; i++) {
                int size = utf8Map->mapUnicode(u[i], m_utf8 + m_utf8Count, m_utf8Max - m_utf8Count);
                m_utf8Count += size;
                m_cairoTextClusters[m_clustersCount].num_bytes += size;
            }
            m_cairoTextClusters[m_clustersCount].num_glyphs = 1;
            m_clustersCount++;
        }
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

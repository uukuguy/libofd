#include <iomanip>
#include <UTF.h>
#include <cairo.h>
#include "OFDOutputDev.h"
#include "OFDTextObject.h"
#include "utils/logger.h"

/***
 *
 * Inherited functions:
 *
 * OFDOutputDev::startPage()
 * OFDOutputDev::endPage()
 * OFDOutputDev::saveState()
 * OFDOutputDev::restoreState()
 *
 * Private help functions:
 *
 * OFDOutputDev::processTextPage()
 * OFDOutputDev::processTextLine()
 *
 * **/

using namespace ofd;

// FIXME
extern GBool rawOrder;

// -------- OFDOutputDev::startPage() --------
void OFDOutputDev::startPage(int pageNum, GfxState *state, XRef *xrefA) {

    /* set up some per page defaults */
    //if ( m_fillPattern != nullptr ){
        //cairo_pattern_destroy(m_fillPattern);
    //}
    //if ( m_strokePattern != nullptr ){
        //cairo_pattern_destroy(m_strokePattern);
    //}

    m_fillPattern = cairo_pattern_create_rgb(0., 0., 0.);
    m_fillColor.r = m_fillColor.g = m_fillColor.b = 0;
    m_strokePattern = cairo_pattern_reference(m_fillPattern);
    m_strokeColor.r = m_strokeColor.g = m_strokeColor.b = 0;

    if ( m_textPage != nullptr ){
        delete m_actualText;
        m_textPage->decRefCnt();
        m_textPage = new TextPage(rawOrder);
        m_actualText = new ActualText(m_textPage);

        m_textPage->startPage(state);
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


        int imageWidth = 794;
        int imageHeight = 1122;
        m_imageSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
        if ( m_imageSurface == nullptr ){
            LOG(ERROR) << "create_image_surface() failed. ";
            return;
        }
        m_cairoRender = std::make_shared<OFDCairoRender>(m_imageSurface);
    }
}

// -------- OFDOutputDev::processTextLine() --------
void OFDOutputDev::processTextLine(TextLine *line, OFDLayerPtr bodyLayer, OFDCairoRenderPtr cairoRender){
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

        //int charPos = word->getCharPos();
        //int charLen = word->getCharLen();
        //LOG(INFO) << "TextWord charPos: " << charPos << " charLen: " << charLen;

        // TextWord Rotation
        //int rotation = word->getRotation();

        //double baseLine = word->getBaseline();
        
        // TextWord Color
        //double red, green, blue;
        //word->getColor(&red, &green, &blue);
        //LOG(INFO) << "TextWord Baseline: " << baseLine << " Rotation: " << rotation << " Color: (" << red << ", " << green << ", " << blue << ")";

        // TextWord Font
        double fontSize = word->getFontSize();
        int numChars = word->getLength();
        //LOG(INFO) << "TextWord FontSize=" << fontSize << " numChars=" << numChars << " len(string)=" << myString.length() << " size(string)=" << myString.size();
        for ( int k = 0 ; k < numChars ; k++ ){
            //TextFontInfo *fontInfo = word->getFontInfo(k);

            //Ref *ref = fontInfo->gfxFont->getID();
            //uint64_t fontID = ref->num;

            //GooString *fontName = fontInfo->getFontName();
            char tCh[4];
            tCh[0] = myString[k*3];
            tCh[1] = myString[k*3+1];
            tCh[2] = myString[k*3+2];
            tCh[3] = '\0';

            //double ascent = fontInfo->getAscent();
            //double descent = fontInfo->getDescent();

            //double xMinA, yMinA, xMaxA, yMaxA;
            //word->getCharBBox(k, &xMinA, &yMinA, &xMaxA, &yMaxA);

            //double edge = word->getEdge(k);


            GooString *aText = new GooString(tCh);
            //Unicode *uni = NULL;
            //__attribute__((unused)) int length = TextStringToUCS4(aText, &uni);


            //LOG(INFO) << tCh << "(" << xMinA << ", " << yMinA << ", " << xMaxA << ", " << yMaxA << ") " << " Edge: " << edge << " Ascent: " << ascent << " Descent: " << descent << " Font[" << k << "] name: " << std::string(fontName->getCString()) << " (" << fontID << ")";

            //LOG(DEBUG) << "UCS code:" << std::hex << std::setw(4) << std::setfill('0') << *uni;
            
            //gfree(uni);
            delete aText;
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

        assert( bodyLayer != nullptr );
        {
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
            m_currentOFDPage->AddObject(object);

            if ( cairoRender != nullptr ){
                cairoRender->DrawObject(object);
            }
        }
    }

    LOG(DEBUG) << wordXML.str();
}

// -------- OFDOutputDev::processTextPage() --------
void OFDOutputDev::processTextPage(TextPage *textPage, OFDPagePtr currentOFDPage){
    OFDLayerPtr bodyLayer = nullptr;
    if ( currentOFDPage != nullptr ){
        bodyLayer = currentOFDPage->GetBodyLayer();
    }

    uint64_t pageID = currentOFDPage->GetID();
    LOG(DEBUG) << "processTextPage(). page ID: " << pageID;


    // OFDCairoRender

    //int imageWidth = 794;
    //int imageHeight = 1122;
    //cairo_surface_t *imageSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
    //if ( imageSurface == nullptr ){
        //LOG(ERROR) << "create_image_surface() failed. ";
        //return;
    //}
    //OFDCairoRenderPtr cairoRender(new OFDCairoRender(imageSurface));
    ////OFDCairoRenderPtr cairoRender = nullptr;

    double xMin, yMin, xMax, yMax;
    for ( auto flow = textPage->getFlows(); flow != nullptr ; flow = flow->getNext()){
        for ( auto blk = flow->getBlocks(); blk != nullptr ; blk = blk->getNext()){
            blk->getBBox(&xMin, &yMin, &xMax, &yMax);
            for ( auto line = blk->getLines(); line != nullptr ; line = line->getNext()){
                processTextLine(line, bodyLayer, m_cairoRender);
            }
        }
    }

    //std::string png_filename = std::string("output/pdf2ofd/Page") + std::to_string(pageID) + ".png";
    //cairo_surface_write_to_png(imageSurface, png_filename.c_str());

    //cairo_surface_destroy(imageSurface);
}

// -------- OFDOutputDev::endPage() --------
void OFDOutputDev::endPage() {
  if ( m_textPage != nullptr ){
    m_textPage->endPage();
    m_textPage->coalesce(gTrue, 0, gFalse);

    processTextPage(m_textPage, m_currentOFDPage);
  }

  if ( m_imageSurface != nullptr ){
      m_cairoRender = nullptr;
      uint64_t pageID = m_currentOFDPage->GetID();
      std::string png_filename = std::string("output/pdf2ofd/Page") + std::to_string(pageID) + ".png";
      cairo_surface_write_to_png(m_imageSurface, png_filename.c_str());

      cairo_surface_destroy(m_imageSurface);
      m_imageSurface = nullptr;
  }
}

// -------- OFDOutputDev::saveState() --------
void OFDOutputDev::saveState(GfxState *state){
    //if ( m_textPage != nullptr ){
        //m_textPage->updateFont(state);
    //}

    cairo_save(m_cairo);
    if ( m_cairoShape != nullptr ){
        cairo_save(m_cairoShape);
    }

    //MaskStack *ms = new MaskStack;
    //ms->mask = cairo_pattern_reference(mask);
    //ms->mask_matrix = mask_matrix;
    //ms->next = maskStack;
    //maskStack = ms;

    if ( m_strokePathClip != nullptr ){
        m_strokePathClip->ref_count++;
    }
}

// -------- OFDOutputDev::restoreState() --------
void OFDOutputDev::restoreState(GfxState *state){
    //if ( m_textPage != nullptr ){
        //m_textPage->updateFont(state);
    //}

    cairo_restore(m_cairo);
    if ( m_cairoShape != nullptr ){
        cairo_restore(m_cairoShape);
    }

    m_textMatrixValid = true;

    /* These aren't restored by cairo_restore() since we keep them in
     * the output device. */
    updateFillColor(state);
    updateStrokeColor(state);
    updateFillOpacity(state);
    updateStrokeOpacity(state);
    updateBlendMode(state);

    //MaskStack* ms = maskStack;
    //if (ms) {
        //if ( m_maskPattern != nullptr )
            //cairo_pattern_destroy(m_maskPattern);
        //m_maskPattern = ms->mask;
        //m_mask_matrix = ms->mask_matrix;
        //maskStack = ms->next;
        //delete ms;
    //}

    if ( m_strokePathClip && --m_strokePathClip->ref_count == 0) {
        delete m_strokePathClip->path;
        if ( m_strokePathClip->dashes ){
            gfree(m_strokePathClip->dashes);
        }
        gfree(m_strokePathClip);
        m_strokePathClip = nullptr;
    }
}

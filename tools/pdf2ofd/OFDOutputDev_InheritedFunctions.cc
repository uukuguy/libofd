#include <iomanip>
#include <UTF.h>
#include <cairo.h>
#include <GlobalParams.h>
#include <UnicodeMap.h>
#include "OFDOutputDev.h"
#include "ofd/Page.h"
#include "ofd/TextObject.h"
#include "OFDRes.h"
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

    if ( m_document != nullptr ){
        LOG(INFO) << "******** startPage(" << pageNum << ") ********";
        m_currentOFDPage = m_document->AddNewPage();
        m_currentOFDPage->AddNewLayer(ofd::LayerType::BODY);

        ::Page *pdfPage = m_pdfDoc->getPage(pageNum);
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
        m_currentOFDPage->Area = pageArea;

        LOG(INFO) << "\n";


    }
}

void OFDOutputDev::ExportWord(TextWord *word){
    double xMin, yMin, xMax, yMax;
    double lineXMin = 0, lineYMin = 0, lineXMax = 0, lineYMax = 0;

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
    LOG(INFO) << "TextWord FontSize=" << fontSize << " numChars=" << numChars << " len(string)=" << myString.length() << " size(string)=" << myString.size() << " word->getText()->getLength() = " << word->getText()->getLength();

    __attribute__((unused)) std::function<int (TextWord*)> WordToTextObject = [=](TextWord *word) -> int{
        UnicodeMap *uMap = globalParams->getTextEncoding();
        int numChars = word->getLength();
        double fontSize = word->getFontSize();
        //uint64_t prevFontID = 0;
        std::vector<std::pair<int, uint64_t> > positions;
        for ( int k = 0 ; k < numChars ; k++){
            TextFontInfo *fontInfo = word->getFontInfo(k);
            Ref *ref = fontInfo->gfxFont->getID();
            uint64_t fontID = ref->num;
            //if ( fontID != prevFontID ){
            positions.push_back(std::make_pair(k, fontID));
            //prevFontID = fontID;
            //}
        }
        positions.push_back(std::make_pair(numChars, 0));

        double x = xMin;
        double y = yMin;
        size_t numStrings = positions.size() - 1;
        for ( size_t i = 0 ; i < numStrings ; i++ ){
            GooString *aText = new GooString();
            int pos0 = positions[i].first;
            uint64_t fontID = positions[i].second;

            double xMinA, yMinA, xMaxA, yMaxA;
            word->getCharBBox(i, &xMinA, &yMinA, &xMaxA, &yMaxA);
            // FIXME
            //fontSize = (xMaxA - xMinA);
            LOG(DEBUG) << "........ " << i << " CharBBox(" << xMinA << ", " << yMinA << ", " << xMaxA << ", " << yMaxA << ") fontID: " << fontID << " fontSize: " << fontSize;

            Document::CommonData &commonData = m_document->GetCommonData();
            assert(commonData.DocumentRes != nullptr );
            auto textFont = commonData.DocumentRes->GetFont(fontID);

            int pos1 = positions[i+1].first;
            for ( int j = pos0 ; j < pos1 ; j++ ){
                const Unicode *uni = word->getChar(j); 
                char buf[8];
                int n = uMap->mapUnicode(*uni, buf, sizeof(buf));
                aText->append(buf, n);
            }
            std::string str(aText->getCString());
            int nChars = pos1 - pos0;
            double edge = word->getEdge(pos0);
            double baseLine = word->getBaseline();
            x = edge;
            y = baseLine;

            LOG(DEBUG) << "-------- (" << x << "," << y << ") edge: " << edge << " baseLine: " << baseLine << " str: " << str << " nChars: " << nChars;


            TextObject *textObject = new TextObject(m_currentOFDPage->GetBodyLayer());

            // OFDObject::Boundary
            //textObject->Boundary = ST_Box(xMin, yMin, xMax - xMin, yMax - yMin);

            // OFDTextObject::TextCode
            Text::TextCode textCode;
            textCode.X = x;
            textCode.Y = y;
            textCode.Text = str;
            textObject->AddTextCode(textCode);

            textObject->SetFont(textFont);
            textObject->SetFontSize(fontSize);

            double r, g, b; 
            word->getColor(&r, &g, &b);
            uint32_t R = colToDbl(r) * 255.0;
            uint32_t G = colToDbl(g) * 255.0;
            uint32_t B = colToDbl(b) * 255.0;

            ColorPtr wordColor = Color::Instance(B, G, R, 255);
            if ( !wordColor->Equal(TextObject::DefaultFillColor) ){
                textObject->SetFillColor(wordColor);
            }
            if ( R != 0 || G != 0 || B != 0 ){
                LOG(DEBUG) << "ExportWord wordColor=(" << R << "," << G << "," << B << ")";
            }

            ObjectPtr object = std::shared_ptr<ofd::Object>(textObject);
            m_currentOFDPage->AddObject(object);

            if ( m_cairoRender != nullptr ){
                //cairo_matrix_t matrix = {1.0, 0.0, 0.0, -1.0, 0.0, 841.89};
                //m_cairoRender->Transform(&matrix);
                m_cairoRender->DrawObject(object);
            }

            //x += fontSize * (pos1 - pos0);
            delete aText;
        }
        uMap->decRefCnt();
        return 0;
    };

    WordToTextObject(word);
    //UnicodeMap *uMap = globalParams->getTextEncoding();
    //for ( int k = 0 ; k < numChars ; k++ ){
    //TextFontInfo *fontInfo = word->getFontInfo(k);
    //Ref *ref = fontInfo->gfxFont->getID();
    //uint64_t fontID = ref->num;
    ////GooString *fontName = fontInfo->getFontName();

    //const Unicode *uni = word->getChar(k); 
    //assert(uMap != nullptr);
    //GooString *aText = new GooString();
    //char buf[8];
    //int n;
    //n = uMap->mapUnicode(*uni, buf, sizeof(buf));
    //aText->append(buf, n);


    ////char tCh[4];
    ////tCh[0] = myString[k*3];
    ////tCh[1] = myString[k*3+1];
    ////tCh[2] = myString[k*3+2];
    ////tCh[3] = '\0';

    ////double ascent = fontInfo->getAscent();
    ////double descent = fontInfo->getDescent();

    ////double xMinA, yMinA, xMaxA, yMaxA;
    ////word->getCharBBox(k, &xMinA, &yMinA, &xMaxA, &yMaxA);

    ////double edge = word->getEdge(k);


    ////GooString *aText = new GooString(tCh);
    ////Unicode *uni = NULL;
    ////__attribute__((unused)) int length = TextStringToUCS4(aText, &uni);

    //LOG(DEBUG) << "[" << k << "] " << std::string(aText->getCString()) << " font: " << fontID;

    ////LOG(INFO) << tCh << "(" << xMinA << ", " << yMinA << ", " << xMaxA << ", " << yMaxA << ") " << " Edge: " << edge << " Ascent: " << ascent << " Descent: " << descent << " Font[" << k << "] name: " << std::string(fontName->getCString()) << " (" << fontID << ")";

    ////LOG(DEBUG) << "UCS code:" << std::hex << std::setw(4) << std::setfill('0') << *uni << std::dec;

    ////gfree(uni);
    //delete aText;
    //}
    //uMap->decRefCnt();

    //TextFontInfo *fontInfo = word->getFontInfo(0);
    //Ref *ref = fontInfo->gfxFont->getID();
    //uint64_t fontID = ref->num;

    //OFDDocument::CommonData &commonData = m_document->GetCommonData();
    //assert(commonData.DocumentRes != nullptr );
    //auto textFont = commonData.DocumentRes->GetFont(fontID);

    //std::stringstream ss;
    //ss << "          <word xMin=\"" << xMin << "\" yMin=\"" << yMin << "\" xMax=\"" <<
    //xMax << "\" yMax=\"" << yMax << "\">" << myString << "</word>\n";
    //LOG(DEBUG) << ss.str() << " Length: " << myString.length();

    //wordXML << ss.str();

    //assert( bodyLayer != nullptr );
    //{
    //OFDTextObject *textObject = new OFDTextObject(m_currentOFDPage);

    //// OFDObject::Boundary
    //textObject->Boundary = ST_Box(xMin, yMin, xMax - xMin, yMax - yMin);

    //// OFDTextObject::TextCode
    //Text::TextCode textCode;
    //textCode.X = xMin;
    //textCode.Y = yMin;
    //textCode.Text = myString;
    //textObject->AddTextCode(textCode);

    //textObject->SetFont(textFont);
    //textObject->SetFontSize(fontSize);

    //OFDObjectPtr object = std::shared_ptr<OFDObject>(textObject);
    //m_currentOFDPage->AddObject(object);

    //if ( m_cairoRender != nullptr ){
    //m_cairoRender->DrawObject(object);
    //}
    //}
}

// -------- OFDOutputDev::processTextLine() --------
void OFDOutputDev::processTextLine(TextLine *line, LayerPtr bodyLayer){
    TextWord *word;
    //std::stringstream wordXML;
    //wordXML << std::fixed << std::setprecision(6);

    for (word = line->getWords(); word; word = word->getNext()) {
        //ExportWord(word);
    }

    //LOG(DEBUG) << wordXML.str();
}

// -------- OFDOutputDev::processTextPage() --------
void OFDOutputDev::processTextPage(TextPage *textPage, PagePtr currentOFDPage){
    LayerPtr bodyLayer = nullptr;
    if ( currentOFDPage != nullptr ){
        bodyLayer = currentOFDPage->GetBodyLayer();
    }

    uint64_t pageID = currentOFDPage->ID;
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
                processTextLine(line, bodyLayer);
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
}

// -------- OFDOutputDev::saveState() --------
void OFDOutputDev::saveState(GfxState *state){
    //if ( m_textPage != nullptr ){
        //m_textPage->updateFont(state);
    //}

    LOG(INFO) << "[imageSurface] SaveState";

    cairo_save(m_cairo);
    if ( m_cairoShape != nullptr ){
        cairo_save(m_cairoShape);
    }

    if ( m_cairoRender != nullptr ){
        m_cairoRender->SaveState();
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

    LOG(INFO) << "[imageSurface] RestoreState";

    cairo_restore(m_cairo);
    if ( m_cairoShape != nullptr ){
        cairo_restore(m_cairoShape);
    }

    if ( m_cairoRender != nullptr ){
        m_cairoRender->RestoreState();
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

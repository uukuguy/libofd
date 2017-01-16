#include "FontOutputDev.h"
#include "utils/logger.h"

using namespace ofd;

FontOutputDev::FontOutputDev() : OutputDev(),
    m_pdfDoc(nullptr), m_xref(nullptr),
    m_allChanged(false), m_fontChanged(false){
}

FontOutputDev::~FontOutputDev(){
}

// -------- FontOutputDev::preProcess() --------
void FontOutputDev::preProcess(PDFDocPtr pdfDoc){
    m_preprocessor.ProcessDoc(pdfDoc.get());
}

// ======== FontOutputDev::ProcessDoc() ========
void FontOutputDev::ProcessDoc(PDFDocPtr pdfDoc){
    if ( pdfDoc == nullptr ) return;
    m_pdfDoc = pdfDoc;
    m_xref = pdfDoc->getXRef();

    preProcess(pdfDoc);

    auto numPages = pdfDoc->getNumPages();
    LOG(INFO) << "Total " << numPages << " pages in pdf file"; 

    int firstPage = 1;
    for ( auto pg = firstPage ; pg <= numPages ; pg++ ){
        double resolution = 72.0;
        GBool useMediaBox = true;
        GBool crop = false;
        GBool printing = true;
        pdfDoc->displayPage(this, pg, resolution, resolution, 0, useMediaBox, crop, printing);
    }

    postProcess();

}

// -------- FontOutputDev::postProcess() --------
void FontOutputDev::postProcess(){
}

// -------- FontOutputDev::drawString() --------
void FontOutputDev::drawString(GfxState * state, GooString * s) {
    //LOG(DEBUG) << "drawString() " << std::string(s->getCString());

    if ( s->getLength() == 0 ){
        return;
    }

    auto font = state->getFont();
    if( font == nullptr) {
        LOG(WARNING) << "font == nullptr.";
        return;
    }

    //double curLetterSpace = state->getCharSpace();
    //double curWordSpace   = state->getWordSpace();
    //double curHorizScaling = state->getHorizScaling();

}

void FontOutputDev::checkStateChange(GfxState * state){

    if ( m_allChanged || m_fontChanged ){
    }

    resetStateChange();
}

void FontOutputDev::resetStateChange(){
    m_allChanged = false;
    m_fontChanged = false;
}

//void FontOutputDev::startPage(int pageNum, GfxState *state, XRef * xref){
    //LOG(DEBUG) << "startPage() Page " << pageNum;
//}

//void FontOutputDev::endPage(){
    //LOG(DEBUG) << "endPage()";
//}

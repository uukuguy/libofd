#include "FontOutputDev.h"
#include "utils/logger.h"

#include "utils/ffw.h"
using namespace utils;

using namespace ofd;

//Param param;

FontOutputDev::FontOutputDev() : OutputDev(),
    m_pdfDoc(nullptr), m_xref(nullptr),
    m_allChanged(false), m_fontChanged(false){


    // -------- Params

    //memset(&param, 0, sizeof(Param));
    //param.tounicode = 1;
    //param.stretch_narrow_glyph = 0;
    //param.squeeze_wide_glyph = 1;
    //param.tmp_dir = "/tmp";
    //param.dest_dir = ".";
    //param.external_hint_tool = "";
    //param.auto_hint = 0;
    //param.override_fstype = 0;
    //param.embed_font = 1;
    //param.font_format = "woff";

    //int tounicode            = 1;      // how to handle ToUnicode CMaps. 
                                       //// (0=auto, 1=force, -1=ignore)
    //int stretch_narrow_glyph = 0;      // stretch narrow glyphs instead of padding them.
    //int squeeze_wide_glyph   = 1;      // shrink wide glyphs instaed of truncating them.
    //std::string tmp_dir      = "/tmp";
    //std::string dest_dir     = ".";
    //std::string external_hint_tool = ""; // external tool for hinting fonts (overrides --auto-hint)
    //bool auto_hint = true;            // use fontforge autohint on fonts without hints
    //bool override_fstype = false;     // clear the fstype bits in TTF/OTF fonts.
    //bool embedFont = true;            // embed font files into output.
    //std::string font_format = "woff"; // suffix for embedded font files (ttf, otf, woff, svg)

    ffw_init(false);
    cur_mapping.resize(0x10000);
    cur_mapping2.resize(0x100);
    width_list.resize(0x10000);
}

FontOutputDev::~FontOutputDev(){
}

// -------- FontOutputDev::preProcess() --------
void FontOutputDev::preProcess(PDFDocPtr pdfDoc){
    m_preprocessor.ProcessDoc(pdfDoc);
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
    //for ( auto font : m_fonts ){
        //__attribute__((unused)) const FontInfo *fontInfo = installFont(font);
    //}
    for ( auto ff : m_fontFiles ){
        LOG(DEBUG) << "fontRef: " << ff.first << " FontFile: " << ff.second;
    }
}

//// -------- FontOutputDev::drawString() --------
//void FontOutputDev::drawString(GfxState * state, GooString * s) {
    //if ( s->getLength() == 0 ){
        //return;
    //}

    //auto font = state->getFont();
    //if( font == nullptr) {
        //LOG(WARNING) << "font == nullptr.";
        //return;
    //}

    ////double curLetterSpace = state->getCharSpace();
    ////double curWordSpace   = state->getWordSpace();
    ////double curHorizScaling = state->getHorizScaling();

    ////checkStateChange(state);
//}

void FontOutputDev::drawChar(GfxState *state, double x, double y,
      double dx, double dy,
      double originX, double originY,
      CharCode code, int nBytes, Unicode *u, int uLen) {
}

void FontOutputDev::checkStateChange(GfxState * state){

    if ( m_allChanged || m_fontChanged ){
        auto font = state->getFont();
        //m_fonts.push_back(font);
        __attribute__((unused)) const FontInfo *fontInfo = installFont(font);
    }

    resetChangedState();
}

void FontOutputDev::resetState(){

    resetChangedState();

    m_allChanged = true;
}

void FontOutputDev::resetChangedState(){
    m_allChanged = false;

    m_fontChanged = false;
}

void FontOutputDev::updateAll(GfxState * state){
    m_allChanged = true;
}

void FontOutputDev::updateFont(GfxState * state){
    m_fontChanged = true;
    checkStateChange(state);
}


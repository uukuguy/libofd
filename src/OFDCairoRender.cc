#include <assert.h>
#include <cairo.h>
#include <cairo-ft.h>
#include "OFDCairoRender.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "OFDObject.h"
#include "OFDTextObject.h"
#include "OFDPathObject.h"
#include "OFDImageObject.h"
#include "OFDVideoObject.h"
#include "OFDCompositeObject.h"
#include "ofd/OfdPath.h"
#include "utils/logger.h"
#include "utils/unicode.h"

using namespace ofd;

// **************** class OFDCairoRender::ImplCls ****************

class OFDCairoRender::ImplCls {
public:
    //ImplCls(OFDCairoRender *cairoRender, cairo_surface_t *surface);
    ImplCls(OFDCairoRender *cairoRender, double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
    ~ImplCls();

    void Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
    //void SetCairoSurface(cairo_surface_t *surface);
    void DrawPage(OFDPagePtr page, Render::DrawParams drawParams);
    void DrawObject(OFDObjectPtr object);

    void Paint(cairo_surface_t *surface);

    void SetLineWidth(double lineWidth);
    void UpdateStrokePattern(double R, double G, double B, double opacity);
    void UpdateFillPattern(double R, double G, double B, double opacity);
    void Transform(cairo_matrix_t *matrix);

    void SaveState();
    void RestoreState();
    void Clip(OfdPathPtr clipPath);

private:
    void Destroy();
    void DrawTextObject(cairo_t *cr, OFDTextObject *textObject);
    void DrawPathObject(cairo_t *cr, OFDPathObject *pathObject);
    void DrawImageObject(cairo_t *cr, OFDImageObject *imageObject);
    void DrawVideoObject(cairo_t *cr, OFDVideoObject *videoObject);
    void DrawCompositeObject(cairo_t *cr, OFDCompositeObject *compositeObject);

public:
    OFDCairoRender *m_cairoRender;
    cairo_surface_t *m_surface;
    cairo_t *m_cr;
    double m_pixelWidth;
    double m_pixelHeight;
    double m_resolutionX;
    double m_resolutionY;
    double m_lineWidth;

    cairo_pattern_t *m_fillPattern, *m_strokePattern;
    //ofd::OfdRGB m_strokeColor;
    //ofd::OfdRGB m_fillColor;

}; // class OFDCairoRender::ImplCls

//OFDCairoRender::ImplCls::ImplCls(OFDCairoRender *cairoRender, cairo_surface_t *surface) : 
    //m_cairoRender(cairoRender),
    //m_lineWidth(1.0){

    //SetCairoSurface(surface);

    //m_fillPattern = cairo_pattern_create_rgb(0., 0., 0.);
    //m_strokePattern = cairo_pattern_reference(m_fillPattern);
//}

OFDCairoRender::ImplCls::ImplCls(OFDCairoRender *cairoRender, double pixelWidth, double pixelHeight, double resolutionX, double resolutionY) :
    m_cairoRender(cairoRender), m_surface(nullptr), m_cr(nullptr),
    m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight), 
    m_resolutionX(resolutionX), m_resolutionY(resolutionY),
    m_lineWidth(1.0),
    m_fillPattern(nullptr), m_strokePattern(nullptr){

    Rebuild(pixelWidth, pixelHeight, resolutionX, resolutionY);
}

void OFDCairoRender::ImplCls::Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    Destroy();

    m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, m_pixelWidth, m_pixelHeight);
    if ( m_surface == nullptr ){
        LOG(ERROR) << "create_image_surface() failed. ";
        return;
    }

    m_cr = cairo_create(m_surface);

    m_fillPattern = cairo_pattern_create_rgb(0., 0., 0.);
    m_strokePattern = cairo_pattern_reference(m_fillPattern);

    cairo_scale(m_cr, m_resolutionX/ 72.0, m_resolutionY / 72.0);

    // Repaint background
    cairo_set_source_rgb(m_cr, 1., 1., 1.);
    cairo_paint(m_cr);
}

void OFDCairoRender::ImplCls::Paint(cairo_surface_t *surface){
    if ( m_surface == nullptr ) return;
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_surface(cr, m_surface, 0, 0);
    cairo_paint(cr);
}

void OFDCairoRender::ImplCls::Destroy(){
    if ( m_cr != nullptr ){
        cairo_destroy(m_cr);
    }

    if ( m_strokePattern != nullptr ){
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = nullptr;
    }

    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = nullptr;
    }

    if ( m_surface != nullptr ){
        cairo_surface_destroy(m_surface);
        m_surface = nullptr;
    }
}

OFDCairoRender::ImplCls::~ImplCls(){
    Destroy();
}

//void OFDCairoRender::ImplCls::SetCairoSurface(cairo_surface_t *surface){
    //assert(surface != nullptr);
    //m_surface = surface;
    //m_cr = cairo_create(surface);
    ////cairo_set_source_rgb(m_cr, .6, .6, .3);
    //cairo_set_source_rgb(m_cr, 1., 1., 1.);
    //cairo_paint(m_cr);

    //// FIXME
    //double m_resolutionX = 150.0;
    //double m_resolutionY = 150.0;
    //cairo_scale(m_cr, m_resolutionX/ 72.0, m_resolutionY / 72.0);
//}

void TestDrawPage(OFDPagePtr page, cairo_surface_t *surface) {

    const OFDLayerPtr bodyLayer = page->GetBodyLayer(); 
    if ( bodyLayer == nullptr ) {
        LOG(WARNING) << "page->GetBodyLayer() return nullptr. Maybe NULL content.";
        return;
    }
    size_t numObjects = bodyLayer->GetObjectsCount();
    if ( numObjects == 0 ){
        return;
    }

    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.5);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    cairo_stroke(cr);

    double mm_per_inch = 25.4;
    double dpi = 96;
    double pixels_per_mm = dpi / mm_per_inch;

    // -------- Set FontFace --------
    cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    cairo_scale(cr, pixels_per_mm, pixels_per_mm);

    for ( size_t i = 0 ; i < numObjects ; i++ ){
        const OFDObjectPtr object = bodyLayer->GetObject(i);
        assert(object != nullptr);
        if ( object->Type == ofd::Object::Type::TEXT ){
            OFDTextObject *textObject = static_cast<OFDTextObject*>(object.get());

            // FIXME
            OFDFontPtr font = textObject->GetFont();
            if ( font == nullptr ) {
                LOG(ERROR) << "TextObject Font  is nullptr.";
                continue;
            }

            //cairo_font_face_t *font_face = font->GetFontFace();
            //assert(font_face != nullptr);
            //cairo_set_font_face(cr, font_face);

            size_t numTextCodes = textObject->GetTextCodesCount();
            //LOG(DEBUG) << "numTextCodes: " << numTextCodes;
            for ( size_t n = 0 ; n < numTextCodes ; n++ ){

                // -------- fontMatrix --------
                double ctm[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
                __attribute__((unused)) double xx = ctm[0];
                __attribute__((unused)) double xy = ctm[1];
                __attribute__((unused)) double yx = ctm[2];
                __attribute__((unused)) double yy = ctm[3];
                __attribute__((unused)) double x0 = ctm[4];
                __attribute__((unused)) double y0 = ctm[5];

                cairo_matrix_t fontMatrix;
                cairo_get_font_matrix(cr, &fontMatrix);
                double fontSize = textObject->GetFontSize();
                double fontPixels = dpi * fontSize / 72;
                fontMatrix.xx = fontPixels * xx;
                fontMatrix.yy = fontPixels * yy;
                fontMatrix.x0 = x0;
                fontMatrix.y0 = y0;
                cairo_set_font_matrix(cr, &fontMatrix);

                // -------- Draw Text --------
                const Text::TextCode &textCode = textObject->GetTextCode(n);
                double X = textCode.X;
                double Y = textCode.Y;
                std::string text = textCode.Text;

                double X1 = X * dpi / 72;
                double Y1 = Y - 500;// * dpi / 72;
                cairo_text_extents_t te;
                cairo_text_extents(cr, text.c_str(), &te);
                cairo_move_to(cr, X1 + 0.5 - te.width / 2 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
                cairo_show_text(cr, text.c_str());
                //LOG(DEBUG) << "X: " << X1 << " Y: " << Y1 << " Text: " << text;
            }
        }
    }
}

void DrawFreeTypeString(double X, double Y, const std::string &text, cairo_t *cr, cairo_font_face_t *font_face,
        const cairo_matrix_t *font_matrix, const cairo_matrix_t *ctm, const cairo_font_options_t *font_options, cairo_pattern_t *strokePattern){

    cairo_scaled_font_t *scaled_font = cairo_scaled_font_create(font_face, font_matrix, ctm, font_options);

//void DrawFreeTypeString(double X, double Y, const std::string &text, cairo_t *cr, 
        //const cairo_matrix_t *font_matrix, const cairo_matrix_t *ctm, const cairo_font_options_t *font_options){

    //cairo_scaled_font_t *scaled_font = cairo_get_scaled_font(cr);

    //FT_Face face;
    //FT_UInt idx;
    //face = cairo_ft_scaled_font_lock_face(scaled_font);
    //{
        //FT_ULong charcode;
        //charcode = FT_Get_First_Char(face, &idx);
        ////LOG(DEBUG) << " !!! charcode: " << std::hex << charcode;
        ////LOG(DEBUG) << " !!! idx: " << idx;
        //int i = 0;
        //const int NUM_GLYPHS = 40;
        //while (idx && (i < NUM_GLYPHS)) {
            //cairo_glyph_t glyph = (cairo_glyph_t) {idx, 1, 2};
            ////LOG(DEBUG) << " charcode: " << std::hex << charcode;
            ////LOG(DEBUG) << " glyph = {" << glyph.index << ", " << glyph.x << ", " << glyph.y;
            //i++;
            //charcode = FT_Get_Next_Char(face, charcode, &idx);
        //}
    //}
    //cairo_ft_scaled_font_unlock_face(scaled_font);

    cairo_status_t status = cairo_scaled_font_status(scaled_font);
    if ( status  != CAIRO_STATUS_SUCCESS ){
        LOG(ERROR) << "cairo_scaled_font_create() failed. Cairo status: " << cairo_status_to_string(status);
        return;
    }

    cairo_font_extents_t font_extents;
    cairo_scaled_font_extents(scaled_font, &font_extents);
    //LOG(DEBUG) << "======== cairo_scaled_font_extents ========\n"
        //<< " ascent: " << font_extents.ascent << "\n"
        //<< " descent: " << font_extents.descent << "\n"
        //<< " height: " << font_extents.height << "\n"
        //<< " max_x_advance: " << font_extents.max_x_advance << "\n"
        //<< " max_y_advance: " << font_extents.max_y_advance << "\n";

    
    cairo_text_extents_t text_extents;
    cairo_scaled_font_text_extents(scaled_font, text.c_str(), &text_extents);
    //LOG(DEBUG) << "-------- cairo_scaled_font_text_extents --------\n"
        //<< " x_bearing: " << text_extents.x_bearing << "\n"
        //<< " y_bearing: " << text_extents.y_bearing << "\n"
        //<< " width: " << text_extents.width << "\n"
        //<< " height: " << text_extents.height << "\n"
        //<< " x_advance: " << text_extents.x_advance << "\n"
        //<< " y_advance: " << text_extents.y_advance << "\n";


    cairo_glyph_t *glyphs = nullptr;
    int num_glyphs = 0;

    __attribute__((unused)) cairo_text_cluster_t *clusters = nullptr;
    __attribute__((unused))int num_clusters;
    __attribute__((unused))cairo_text_cluster_flags_t cluster_flags;

    //// utf8 -> unicode
    //unsigned long unic[text.length()];
    //int ucLen = 0;
    //const char *pText = text.c_str();
    //int tLen = text.length();
    //int tPos = 0;
    //int ucPos = 0;
    //while ( tPos < tLen ){
        //int len = enc_utf8_to_unicode_one((const unsigned char*)pText, &unic[ucPos]);  
        //pText += len;
        //tPos += len;
        //ucPos++;
    //}
    //ucLen = ucPos;

    //status = cairo_scaled_font_text_to_glyphs(scaled_font, X, Y, (const char *)&unic[0], ucLen,
    status = cairo_scaled_font_text_to_glyphs(scaled_font, X, Y, text.c_str(), text.length(),
            &glyphs, &num_glyphs, 
            &clusters, &num_clusters, &cluster_flags
            //nullptr, nullptr, nullptr
            );

    if ( status != CAIRO_STATUS_SUCCESS ){
        LOG(ERROR) << "cairo_scaled_font_text_to_glyphs() failed. Cairo status: " << cairo_status_to_string(status);
        return;
    }

    cairo_text_extents_t glyph_extents;
    cairo_scaled_font_glyph_extents(scaled_font, glyphs, num_glyphs, &glyph_extents);
    //LOG(DEBUG) << "-------- cairo_scaled_font_glyph_extents --------\n"
        //<< " x_bearing: " << glyph_extents.x_bearing << "\n"
        //<< " y_bearing: " << glyph_extents.y_bearing << "\n"
        //<< " width: " << glyph_extents.width << "\n"
        //<< " height: " << glyph_extents.height << "\n"
        //<< " x_advance: " << glyph_extents.x_advance << "\n"
        //<< " y_advance: " << glyph_extents.y_advance << "\n";

    //LOG(DEBUG) << "******** " << num_glyphs << " glyphs ********";
    //for ( auto n = 0 ; n < num_glyphs ; n++ ){
        //LOG(DEBUG) << "[" << n << "] " << " index: " << glyphs[n].index << " x: " << glyphs[n].x << " y: " << glyphs[n].y;
    //}

    cairo_show_text_glyphs(cr, text.c_str(), text.length(), glyphs, num_glyphs, clusters, num_clusters, cluster_flags);
    //cairo_show_glyphs(cr, glyphs, num_glyphs);

    //cairo_set_source (cr, strokePattern);
    //cairo_glyph_path (cr, glyphs, num_glyphs);
    //cairo_stroke (cr);

    if ( glyphs != nullptr ){
        cairo_glyph_free(glyphs);
    }
    if ( clusters != nullptr ){
        cairo_text_cluster_free(clusters);
    }

    cairo_scaled_font_destroy(scaled_font);
}

// ======== OFDCairoRender::ImplCls::Draw() ========
void OFDCairoRender::ImplCls::DrawPage(OFDPagePtr page, Render::DrawParams drawParams){
    if ( page == nullptr ) return;
    if ( m_surface == nullptr ) return;
    double offsetX;
    double offsetY;
    double scaling;
    std::tie(offsetX, offsetY, scaling) = m_cairoRender->GetDrawParams();

    // FIXME
    // first version to render the page.
    //TestDrawPage(page, m_surface);
    //return;

    const OFDLayerPtr bodyLayer = page->GetBodyLayer(); 
    if ( bodyLayer == nullptr ) {
        LOG(WARNING) << "page->GetBodyLayer() return nullptr. Maybe NULL content.";
        return;
    }
    size_t numObjects = bodyLayer->GetObjectsCount();
    if ( numObjects == 0 ){
        return;
    }

    cairo_t *cr = cairo_create(m_surface);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    //cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    //cairo_stroke(cr);

    // -------- default font --------
    assert(page->GetOFDDocument() != nullptr);
    assert(page->GetOFDDocument()->GetDocumentRes() != nullptr);
    // FIXME
    //OFDFontPtr defaultFont = page->GetOFDDocument()->GetDocumentRes()->GetFont(0);
    //assert(defaultFont != nullptr);

    //double mm_per_inch = 25.4;
    //double dpi = 96;
    //double pixels_per_mm = dpi / mm_per_inch;

    // -------- Set FontFace --------
    //cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    //cairo_scale(cr, pixels_per_mm, pixels_per_mm);

    for ( size_t i = 0 ; i < numObjects ; i++ ){
        const OFDObjectPtr object = bodyLayer->GetObject(i);
        assert(object != nullptr);

        DrawObject(object);
        //if ( object->Type == ofd::Object::Type::TEXT ){
            //OFDTextObject *textObject = static_cast<OFDTextObject*>(object.get());

            ////cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

            ////// FIXME
            ////OFDFontPtr font = defaultFont;
            //OFDFontPtr font = textObject->GetFont();

            //assert(font != nullptr);
            //assert(font->IsLoaded());
            //cairo_font_face_t *font_face = font->GetFontFace();
            //assert(font_face != nullptr);
            //cairo_set_font_face(cr, font_face);

            //size_t numTextCodes = textObject->GetTextCodesCount();
            ////LOG(DEBUG) << "numTextCodes: " << numTextCodes;
            //for ( size_t n = 0 ; n < numTextCodes ; n++ ){

                //// -------- fontMatrix --------
                //double ctm[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
                //__attribute__((unused)) double xx = ctm[0];
                //__attribute__((unused)) double xy = ctm[1];
                //__attribute__((unused)) double yx = ctm[2];
                //__attribute__((unused)) double yy = ctm[3];
                //__attribute__((unused)) double x0 = ctm[4];
                //__attribute__((unused)) double y0 = ctm[5];

                //cairo_matrix_t fontMatrix;
                //cairo_get_font_matrix(cr, &fontMatrix);
                //double fontSize = textObject->GetFontSize();
                //double fontPixels = dpi * fontSize / 72;
                //fontMatrix.xx = fontPixels * xx;
                //fontMatrix.yy = fontPixels * yy;
                //fontMatrix.x0 = x0;
                //fontMatrix.y0 = y0;
                //cairo_set_font_matrix(cr, &fontMatrix);

                //// -------- Draw Text --------
                //const Text::TextCode &textCode = textObject->GetTextCode(n);
                //double X = textCode.X;
                //double Y = textCode.Y;
                //std::string text = textCode.Text;

                //double X1 = X * dpi / 72;
                //double Y1 = Y * dpi / 72;
                ////double Y1 = Y - 500;// * dpi / 72;

                ////double fontSize = textObject->GetFontSize();
                ////double fontPixels = dpi * fontSize / 72;
                //cairo_matrix_t font_matrix = {fontPixels, 0.0, 0.0, fontPixels, 0.0, 0.0};
                //cairo_matrix_t font_ctm = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}; 
                //cairo_font_options_t *font_options = cairo_font_options_create();
                //cairo_get_font_options(cr, font_options);
                //cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_DEFAULT);
                //DrawFreeTypeString(X1, Y1, text, cr, font_face,
                ////DrawFreeTypeString(X1, Y1, text, cr, 
                        //&font_matrix, &font_ctm, font_options);
                //cairo_font_options_destroy(font_options);

                ////cairo_text_extents_t te;
                ////cairo_text_extents(cr, text.c_str(), &te);
                //////cairo_move_to(cr, X1 + 0.5 - te.width / 2 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
                ////cairo_move_to(cr, X1 + 0.5 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
                ////cairo_show_text(cr, text.c_str());
                ////////LOG(DEBUG) << "X: " << X1 << " Y: " << Y1 << " Text: " << text;

            //}
        //}
    }

}

void OFDCairoRender::ImplCls::DrawObject(OFDObjectPtr object){
    cairo_t *cr = m_cr;

    // FIXME
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    //cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    //cairo_stroke(cr);

    if ( object->Type == ofd::Object::Type::TEXT ) {
        OFDTextObject *textObject = (OFDTextObject*)object.get();
        DrawTextObject(cr, textObject);
    } else if ( object->Type == ofd::Object::Type::PATH ){
        OFDPathObject *pathObject = (OFDPathObject*)object.get();
        DrawPathObject(cr, pathObject);
    } else if ( object->Type == ofd::Object::Type::IMAGE ){
        OFDImageObject *imageObject = (OFDImageObject*)object.get();
        DrawImageObject(cr, imageObject);
    } else if ( object->Type == ofd::Object::Type::VIDEO ){
        OFDVideoObject *videoObject = (OFDVideoObject*)object.get();
        DrawVideoObject(cr, videoObject);
    } else if ( object->Type == ofd::Object::Type::COMPOSITE ){
        OFDCompositeObject *compositeObject = (OFDCompositeObject*)object.get();
        DrawCompositeObject(cr, compositeObject);
    }
}

void OFDCairoRender::ImplCls::DrawTextObject(cairo_t *cr, OFDTextObject *textObject){
    if ( textObject == nullptr ) return;

    //double dpi = 150;

    // -------- Font Face --------
    // FIXME
    //cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    OFDFontPtr font = textObject->GetFont();

    // FIXME
    // fondID:
    //     2 - 标题等
    //     3 - 正文等
    //     4 - 标点符号（不显示）  
    //     5 - 数字（显示位置右下偏移）
    //     10 - 13 - 字体未载入
    //     16 - 19 18- 黑体小标题 19 - 黑体顿号、小括号
    //OFDFontPtr defaultFont = textObject->GetPage()->GetOFDDocument()->GetDocumentRes()->GetFont(4);
    //assert(defaultFont != nullptr);
    //font = defaultFont;


    assert(font != nullptr);
    //LOG(DEBUG) << "DrawTextObject using font (ID=" << font->ID << ")";
    assert(font->IsLoaded());

    if ( font->ID != 3 ) {
        return;
    }

    cairo_font_face_t *font_face = font->GetFontFace();
    assert(font_face != nullptr);

    cairo_set_font_face(cr, font_face);

    // -------- fontMatrix --------
    double ctm[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    __attribute__((unused)) double xx = ctm[0];
    __attribute__((unused)) double xy = ctm[1];

    // FIXME
    __attribute__((unused)) double yx = ctm[2];
    __attribute__((unused)) double yy = ctm[3];

    __attribute__((unused)) double x0 = ctm[4];
    __attribute__((unused)) double y0 = ctm[5];

    cairo_matrix_t fontMatrix;
    cairo_get_font_matrix(cr, &fontMatrix);
    double fontSize = textObject->GetFontSize();
    // FIXME
    //double fontPixels = dpi * fontSize / 72;
    double fontPixels = fontSize;

    fontMatrix.xx = fontPixels * xx;
    fontMatrix.yy = fontPixels * yy;
    fontMatrix.x0 = x0;
    fontMatrix.y0 = y0;
    cairo_set_font_matrix(cr, &fontMatrix);

    // -------- Draw Text --------
    const Text::TextCode &textCode = textObject->GetTextCode(0);
    double X = textCode.X;
    //double Y = 841.89 - textCode.Y;
    double Y = textCode.Y;
    std::string text = textCode.Text;

    // FIXME
    //double X1 = X * dpi / 72;
    //double Y1 = Y * dpi / 72;
    double X1 = X;
    double Y1 = Y;
    //double Y1 = Y - 500;// * dpi / 72;

    //double fontSize = textObject->GetFontSize();
    //double fontPixels = dpi * fontSize / 72;
    cairo_matrix_t font_matrix = {fontPixels, 0.0, 0.0, fontPixels, 0.0, 0.0};
    cairo_matrix_t font_ctm = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}; 
    cairo_font_options_t *font_options = cairo_font_options_create();
    cairo_get_font_options(cr, font_options);
    cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_DEFAULT);

    cairo_set_source (cr, m_fillPattern);
    DrawFreeTypeString(X1, Y1, text, cr, font_face,
            //DrawFreeTypeString(X1, Y1, text, cr, 
            &font_matrix, &font_ctm, font_options, m_strokePattern);
            cairo_font_options_destroy(font_options);

            //cairo_text_extents_t te;
            //cairo_text_extents(cr, text.c_str(), &te);
            //cairo_move_to(cr, X1 + 0.5 - te.width / 2 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
            //cairo_show_text(cr, text.c_str());
            ////LOG(DEBUG) << "X: " << X1 << " Y: " << Y1 << " Text: " << text;

    return;
}

void DoCairoPath(cairo_t *cr, OfdPathPtr path){
    size_t numSubpaths = path->GetNumSubpaths();
    if ( numSubpaths == 0 ) return;

    cairo_new_path(cr);

    for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
        OfdSubpathPtr subpath = path->GetSubpath(idx);
        if ( subpath == nullptr ) continue;
        size_t numPoints = subpath->GetNumPoints();
        if ( numPoints < 2 ) continue;

        const Point &p0 = subpath->GetPoint(0);
        cairo_move_to(cr, p0.x, p0.y);

        for ( size_t n = 1 ; n < numPoints ; n++ ){
            const Point &p = subpath->GetPoint(n);
            cairo_line_to(cr, p.x, p.y);
            LOG(DEBUG) << "[" << n << "] " << "(" << p.x << ", " << p.y << ") ";
        }
        if ( subpath->IsClosed() ){
            cairo_close_path(cr);
        }
    }
}

void OFDCairoRender::ImplCls::DrawPathObject(cairo_t *cr, OFDPathObject *pathObject){
    if ( pathObject == nullptr ) return;

    OfdPathPtr path = pathObject->GetPath();
    DoCairoPath(cr, path);

    cairo_set_source(cr, m_strokePattern);
    cairo_stroke(cr);
}

void OFDCairoRender::ImplCls::DrawImageObject(cairo_t *cr, OFDImageObject *imageObject){
    if ( imageObject == nullptr ) return;
}

void OFDCairoRender::ImplCls::DrawVideoObject(cairo_t *cr, OFDVideoObject *videoObject){
    if ( videoObject == nullptr ) return;
}

void OFDCairoRender::ImplCls::DrawCompositeObject(cairo_t *cr, OFDCompositeObject *compositeObject){
    if ( compositeObject == nullptr ) return;
}

void OFDCairoRender::ImplCls::SetLineWidth(double lineWidth){
    cairo_set_line_width(m_cr, lineWidth);
    m_lineWidth = lineWidth;
}

void OFDCairoRender::ImplCls::UpdateStrokePattern(double r, double g, double b, double opacity){
    if ( m_strokePattern != nullptr ){
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = nullptr;
    }
    m_strokePattern = cairo_pattern_create_rgba(r, g, b, opacity);
}

void OFDCairoRender::ImplCls::UpdateFillPattern(double r, double g, double b, double opacity){
    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = nullptr;
    }
    m_fillPattern = cairo_pattern_create_rgba(r, g, b, opacity);
}

void OFDCairoRender::ImplCls::Transform(cairo_matrix_t *matrix){
    cairo_transform(m_cr, matrix);
}

void OFDCairoRender::ImplCls::SaveState(){
    cairo_save(m_cr);
}

void OFDCairoRender::ImplCls::RestoreState(){
    cairo_restore(m_cr);
}

void OFDCairoRender::ImplCls::Clip(OfdPathPtr clipPath){
    DoCairoPath(m_cr, clipPath);
    cairo_set_fill_rule(m_cr, CAIRO_FILL_RULE_WINDING);
    cairo_clip(m_cr);
}

// **************** class OFDCairoRender ****************

// ======== OFDCairoRender::OFDCairoRender() ========
//OFDCairoRender::OFDCairoRender(){
    //m_impl = std::unique_ptr<OFDCairoRender::ImplCls>(new OFDCairoRender::ImplCls(this, nullptr));
//}

// ======== OFDCairoRender::OFDCairoRender() ========
//OFDCairoRender::OFDCairoRender(cairo_surface_t *surface){
    //m_impl = std::unique_ptr<OFDCairoRender::ImplCls>(new OFDCairoRender::ImplCls(this, surface));
//}

OFDCairoRender::OFDCairoRender(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    m_impl = std::unique_ptr<OFDCairoRender::ImplCls>(new OFDCairoRender::ImplCls(this, pixelWidth, pixelHeight, resolutionX, resolutionY));
}

// ======== OFDCairoRender::~OFDCairoRender() ========
OFDCairoRender::~OFDCairoRender(){
}

//void OFDCairoRender::SetCairoSurface(cairo_surface_t *surface){
    //m_impl->SetCairoSurface(surface);
//}

cairo_surface_t *OFDCairoRender::GetCairoSurface() const{
    return m_impl->m_surface;
}

cairo_t *OFDCairoRender::GetCairoContext() const{
    return m_impl->m_cr;
}

// ======== OFDCairoRender::DrawPage() ========
void OFDCairoRender::DrawPage(OFDPagePtr page, Render::DrawParams drawParams){
    OFDRender::DrawPage(page, drawParams);
    m_impl->DrawPage(page, drawParams);
}

void OFDCairoRender::DrawObject(OFDObjectPtr object){
    m_impl->DrawObject(object);
}

void OFDCairoRender::SetLineWidth(double lineWidth){
    m_impl->SetLineWidth(lineWidth);
}

void OFDCairoRender::UpdateStrokePattern(double r, double g, double b, double opacity){
    m_impl->UpdateStrokePattern(r, g, b, opacity);
}

void OFDCairoRender::UpdateFillPattern(double r, double g, double b, double opacity){
    m_impl->UpdateFillPattern(r, g, b, opacity);
}

void OFDCairoRender::Transform(cairo_matrix_t *matrix){
    m_impl->Transform(matrix);
}

void OFDCairoRender::SaveState(){
    m_impl->SaveState();
}

void OFDCairoRender::RestoreState(){
    m_impl->RestoreState();
}

void OFDCairoRender::Clip(OfdPathPtr clipPath){
    m_impl->Clip(clipPath);
}

void OFDCairoRender::Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    m_impl->Rebuild(pixelWidth, pixelHeight, resolutionX, resolutionY);
}

void OFDCairoRender::Paint(cairo_surface_t *surface){
    m_impl->Paint(surface);
}


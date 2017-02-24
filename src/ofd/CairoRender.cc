#include <assert.h>
#include <cairo.h>
#include <cairo-ft.h>
#include "ofd/CairoRender.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Layer.h"
#include "ofd/Object.h"
#include "ofd/Font.h"
#include "ofd/Color.h"
#include "ofd/Shading.h"
#include "ofd/Pattern.h"
#include "ofd/TextObject.h"
#include "ofd/PathObject.h"
#include "ofd/ImageObject.h"
#include "ofd/VideoObject.h"
#include "ofd/CompositeObject.h"
#include "ofd/Path.h"
#include "utils/logger.h"
#include "utils/unicode.h"

using namespace ofd;

void showCairoMatrix(cairo_t *cr, const std::string &title, const std::string &msg){
    //cairo_matrix_t matrix;
    //cairo_get_matrix(cr, &matrix);
    //LOG(DEBUG) << "[" << title << "] " << msg << " ShowCairoMatrix() cairo_get_matrix()=(" << matrix.xx << "," << matrix.yx << "," << matrix.xy << "," << matrix.yy << "," << matrix.x0 << "," << matrix.y0 << ")";
}

// **************** class CairoRender::ImplCls ****************

class CairoRender::ImplCls {
public:
    //ImplCls(CairoRender *cairoRender, cairo_surface_t *surface);
    ImplCls(CairoRender *cairoRender, double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
    ~ImplCls();

    void Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
    //void SetCairoSurface(cairo_surface_t *surface);
    void DrawPage(PagePtr page, VisibleParams visibleParams);
    void DrawObject(ObjectPtr object);

    void Paint(cairo_surface_t *surface);
    bool WriteToPNG(const std::string &filename);

    void SetLineWidth(double lineWidth);
    void UpdateStrokePattern(double R, double G, double B, double opacity);
    void UpdateFillPattern(double R, double G, double B, double opacity);
    void Transform(cairo_matrix_t *matrix);

    void SaveState();
    void RestoreState();
    void Clip(PathPtr clipPath);

private:
    void Destroy();
    void DrawTextObject(cairo_t *cr, TextObject *textObject);
    void DrawPathObject(cairo_t *cr, PathObject *pathObject);
    void DrawImageObject(cairo_t *cr, ImageObject *imageObject);
    void DrawVideoObject(cairo_t *cr, VideoObject *videoObject);
    void DrawCompositeObject(cairo_t *cr, CompositeObject *compositeObject);

public:
    CairoRender *m_cairoRender;
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

}; // class CairoRender::ImplCls

//CairoRender::ImplCls::ImplCls(CairoRender *cairoRender, cairo_surface_t *surface) : 
    //m_cairoRender(cairoRender),
    //m_lineWidth(1.0){

    //SetCairoSurface(surface);

    //m_fillPattern = cairo_pattern_create_rgb(0., 0., 0.);
    //m_strokePattern = cairo_pattern_reference(m_fillPattern);
//}

CairoRender::ImplCls::ImplCls(CairoRender *cairoRender, double pixelWidth, double pixelHeight, double resolutionX, double resolutionY) :
    m_cairoRender(cairoRender), m_surface(nullptr), m_cr(nullptr),
    m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight), 
    m_resolutionX(resolutionX), m_resolutionY(resolutionY),
    m_lineWidth(1.0),
    m_fillPattern(nullptr), m_strokePattern(nullptr){

    //LOG(INFO) << "New CairoRender with pixelWidth=" << pixelWidth << " pixelHeight=" << std::dec << pixelHeight << " resolutionX=" << resolutionX << " resolutionY=" << resolutionY;
    Rebuild(pixelWidth, pixelHeight, resolutionX, resolutionY);
}

void setDefaultCTM(cairo_t *cr){
    cairo_matrix_t matrix0;
    matrix0.xx = 1.0;
    matrix0.yx = 0.0;
    matrix0.xy = 0.0;
    matrix0.yy = -1.0;
    //matrix0.yy = -1.0;
    matrix0.x0 = -0.0;
    //matrix0.y0 = 0.0;
    matrix0.y0 = 841.89;
    cairo_transform(cr, &matrix0);
}

void clearCTM(cairo_t *cr){
    cairo_matrix_t matrix0;
    matrix0.xx = 1.0;
    matrix0.yx = 0.0;
    matrix0.xy = 0.0;
    matrix0.yy = -1.0;
    matrix0.x0 = 0.0;
    matrix0.y0 = 0.0;
    cairo_transform(cr, &matrix0);
}

void CairoRender::ImplCls::Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
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

    // FIXME
    //setDefaultCTM(m_cr);

    showCairoMatrix(m_cr, "CairoRender", "Init Cairo");
}

bool CairoRender::ImplCls::WriteToPNG(const std::string &filename){
    return cairo_surface_write_to_png(m_surface, filename.c_str());
}

void CairoRender::ImplCls::Paint(cairo_surface_t *surface){
    if ( m_surface == nullptr ) return;
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_surface(cr, m_surface, 0, 0);
    cairo_paint(cr);
}

void CairoRender::ImplCls::Destroy(){
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

CairoRender::ImplCls::~ImplCls(){
    Destroy();
}

//void CairoRender::ImplCls::SetCairoSurface(cairo_surface_t *surface){
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

// ======== CairoRender::ImplCls::Draw() ========
void CairoRender::ImplCls::DrawPage(PagePtr page, VisibleParams visibleParams){
    if ( page == nullptr ) return;
    if ( m_surface == nullptr ) return;
    double pixelX;
    double pixelY;
    double scaling;
    std::tie(pixelX, pixelY, scaling) = m_cairoRender->GetVisibleParams();

    const LayerPtr bodyLayer = page->GetBodyLayer(); 
    if ( bodyLayer == nullptr ) {
        LOG(WARNING) << "page->GetBodyLayer() return nullptr. Maybe NULL content.";
        return;
    }
    size_t numObjects = bodyLayer->GetNumObjects();
    if ( numObjects == 0 ){
        return;
    }

    cairo_set_source_rgb(m_cr, 1.0, 1.0, 1.0);
    cairo_paint(m_cr);

    cairo_translate(m_cr, -pixelX, -pixelY);
    cairo_scale(m_cr, scaling, scaling);

    //cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
    //cairo_rectangle(m_cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    //cairo_stroke(m_cr);

    assert(page->GetDocument() != nullptr);
    assert(page->GetDocument()->GetDocumentRes() != nullptr);
    // FIXME
    // -------- default font --------
    //FontPtr defaultFont = page->GetOFDDocument()->GetDocumentRes()->GetFont(0);
    //assert(defaultFont != nullptr);

    for ( size_t i = 0 ; i < numObjects ; i++ ){
        const ObjectPtr object = bodyLayer->GetObject(i);
        assert(object != nullptr);

        DrawObject(object);
    }

}

void CairoRender::ImplCls::DrawObject(ObjectPtr object){
    cairo_t *cr = m_cr;

    //LOG(INFO) << "DrawObject() *************************************";

    SaveState();

    // FIXME
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    //cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    //cairo_stroke(cr);

    if ( object->Type == ofd::ObjectType::TEXT ) {
        TextObject *textObject = (TextObject*)object.get();
        DrawTextObject(cr, textObject);
    } else if ( object->Type == ofd::ObjectType::PATH ){
        PathObject *pathObject = (PathObject*)object.get();
        DrawPathObject(cr, pathObject);
    } else if ( object->Type == ofd::ObjectType::IMAGE ){
        ImageObject *imageObject = (ImageObject*)object.get();
        DrawImageObject(cr, imageObject);
    } else if ( object->Type == ofd::ObjectType::VIDEO ){
        VideoObject *videoObject = (VideoObject*)object.get();
        DrawVideoObject(cr, videoObject);
    } else if ( object->Type == ofd::ObjectType::COMPOSITE ){
        CompositeObject *compositeObject = (CompositeObject*)object.get();
        DrawCompositeObject(cr, compositeObject);
    }

    RestoreState();
}

void CairoRender::ImplCls::DrawTextObject(cairo_t *cr, TextObject *textObject){
    if ( textObject == nullptr ) return;

    //setDefaultCTM(cr);

    //cairo_matrix_t matrix0;
    //matrix0.xx = 1.0;
    //matrix0.yx = 0.0;
    //matrix0.xy = 0.0;
    //matrix0.yy = -1.0;
    ////matrix0.yy = -1.0;
    //matrix0.x0 = -0.0;
    ////matrix0.y0 = 0.0;
    //matrix0.y0 = 841.89;
    // Transform(&matrix0);
    ////cairo_transform(cr, &matrix0);

    showCairoMatrix(cr, "CairoRender", "DrawTextObject");

    //double dpi = 150;

    // -------- Font Face --------
    // FIXME
    //cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    FontPtr font = textObject->GetFont();

    // FIXME
    // fondID:
    //     2 - 标题等
    //     3 - 正文等
    //     4 - 标点符号（不显示）  
    //     5 - 数字（显示位置右下偏移）
    //     10 - 13 - 字体未载入
    //     16 - 19 18- 黑体小标题 19 - 黑体顿号、小括号
    //FontPtr defaultFont = textObject->GetPage()->GetOFDDocument()->GetDocumentRes()->GetFont(4);
    //assert(defaultFont != nullptr);
    //font = defaultFont;


    assert(font != nullptr);
    //LOG(DEBUG) << "DrawTextObject using font (ID=" << font->ID << ")";
    assert(font->IsLoaded());

    //if ( font->ID != 3 ) {
        //return;
    //}

    cairo_font_face_t *font_face = font->GetCairoFontFace();
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
    double Y = textCode.Y;
    //double Y = 841.89 - textCode.Y;
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


    ColorPtr fillColor = textObject->GetFillColor();
    if ( fillColor != nullptr ){
        const ColorRGB &rgb = fillColor->Value.RGB;
        double r = (double)rgb.Red / 255.0;
        double g = (double)rgb.Green / 255.0;
        double b = (double)rgb.Blue / 255.0;
        double alpha = (double)textObject->Alpha / 255.0;
        //UpdateFillPattern(r, g, b, alpha);
        //LOG(DEBUG) << "textObject->FillColor=(" << r << "," << g << "," << b << "," << alpha << ")";
        cairo_set_source_rgba(cr, b, g, r, alpha);
    }

    //cairo_set_source (cr, m_fillPattern);
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

void DoCairoPath(cairo_t *cr, PathPtr path){
    if ( path == nullptr ) return;

    size_t numSubpaths = path->GetNumSubpaths();
    if ( numSubpaths == 0 ) return;

    cairo_new_path(cr);

    for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
        SubpathPtr subpath = path->GetSubpath(idx);
        if ( subpath == nullptr ) continue;
        size_t numPoints = subpath->GetNumPoints();
        if ( numPoints < 2 ) continue;

        const Point_t &p0 = subpath->GetPoint(0);
        cairo_move_to(cr, p0.x, p0.y);

        for ( size_t n = 1 ; n < numPoints ; n++ ){
            const Point_t &p = subpath->GetPoint(n);
            cairo_line_to(cr, p.x, p.y);
            //LOG(DEBUG) << "[" << n << "] " << "(" << p.x << ", " << p.y << ") ";
        }
        if ( subpath->IsClosed() ){
            cairo_close_path(cr);
        }
    }
}

void CairoRender::ImplCls::DrawPathObject(cairo_t *cr, PathObject *pathObject){
    if ( pathObject == nullptr ) return;

    //setDefaultCTM(cr);
    //clearCTM(cr);

    cairo_matrix_t matrix;
    matrix.xx = pathObject->CTM[0];
    matrix.yx = pathObject->CTM[1];
    matrix.xy = pathObject->CTM[2];
    matrix.yy = pathObject->CTM[3];
    matrix.x0 = pathObject->CTM[4];
    matrix.y0 = pathObject->CTM[5];

    Transform(&matrix);
    //cairo_transform(cr, &matrix);

    showCairoMatrix(cr, "CairoRender", "DrawPathObject");


    PathPtr path = pathObject->GetPath();
    DoCairoPath(cr, path);

    cairo_set_line_width(cr, pathObject->LineWidth);

    ColorPtr strokeColor = pathObject->GetStrokeColor();
    if ( strokeColor != nullptr ){
        const ColorRGB &rgb = pathObject->GetStrokeColor()->Value.RGB;

        double r = (double)rgb.Red / 255.0;
        double g = (double)rgb.Green / 255.0;
        double b = (double)rgb.Blue / 255.0;
        double a = (double)pathObject->Alpha / 255.0;
        //LOG(DEBUG) << "DrawPathObject() rgb = (" << r << "," << g << "," << b << ")";
        UpdateStrokePattern(r, g, b, a);
        cairo_set_source(cr, m_strokePattern);
        //cairo_set_source_rgba(cr, b, g, r, alpha);
        cairo_stroke(cr);
    } else {
        if ( pathObject->FillShading != nullptr ){
            cairo_pattern_destroy(m_fillPattern);
            m_fillPattern = pathObject->FillShading->CreateFillPattern(cr);
        } else {
            ColorPtr fillColor = pathObject->GetFillColor();
            if ( fillColor != nullptr ){
                const ColorRGB &rgb = fillColor->Value.RGB;

                double r = (double)rgb.Red / 255.0;
                double g = (double)rgb.Green / 255.0;
                double b = (double)rgb.Blue / 255.0;
                double alpha = (double)pathObject->Alpha / 255.0;
                //LOG(DEBUG) << "DrawPathObject() rgb = (" << r << "," << g << "," << b << ")";

                UpdateFillPattern(r, g, b, alpha);
            }
        }
        cairo_set_source(cr, m_fillPattern);
        //cairo_set_source_rgba(cr, b, g, r, alpha);
        cairo_fill(cr);
        Clip(path);
    }

}

void CairoRender::ImplCls::DrawImageObject(cairo_t *cr, ImageObject *imageObject){
    if ( imageObject == nullptr ) return;
}

void CairoRender::ImplCls::DrawVideoObject(cairo_t *cr, VideoObject *videoObject){
    if ( videoObject == nullptr ) return;
}

void CairoRender::ImplCls::DrawCompositeObject(cairo_t *cr, CompositeObject *compositeObject){
    if ( compositeObject == nullptr ) return;
}

void CairoRender::ImplCls::SetLineWidth(double lineWidth){
    cairo_set_line_width(m_cr, lineWidth);
    m_lineWidth = lineWidth;
}

void CairoRender::ImplCls::UpdateStrokePattern(double r, double g, double b, double a){
    if ( m_strokePattern != nullptr ){
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = nullptr;
    }
    //LOG(DEBUG) << "UpdateStrokePattern() rgba = (" << r << "," << g << "," << b << "," << a << ")";
    m_strokePattern = cairo_pattern_create_rgba(b, g, r, a);
}

void CairoRender::ImplCls::UpdateFillPattern(double r, double g, double b, double a){
    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = nullptr;
    }
    m_fillPattern = cairo_pattern_create_rgba(b, g, r, a);
}

void CairoRender::ImplCls::Transform(cairo_matrix_t *matrix){

    //LOG(DEBUG) << "[CairoRender] Transform (" << matrix->xx << ", " << matrix->yx << ", " << matrix->xy 
        //<< ", " << matrix->yy << ", " << matrix->x0 << ", " << matrix->y0 << ")";
    cairo_transform(m_cr, matrix);
}

void CairoRender::ImplCls::SaveState(){
    //LOG(DEBUG) << "[CairoRender] SaveState";
    cairo_save(m_cr);
}

void CairoRender::ImplCls::RestoreState(){
    //LOG(DEBUG) << "[CairoRender] RestoreState";
    cairo_restore(m_cr);
}

void CairoRender::ImplCls::Clip(PathPtr clipPath){
    DoCairoPath(m_cr, clipPath);
    cairo_set_fill_rule(m_cr, CAIRO_FILL_RULE_WINDING);
    cairo_clip(m_cr);
}

// **************** class CairoRender ****************

// ======== CairoRender::CairoRender() ========
//CairoRender::CairoRender(){
    //m_impl = std::unique_ptr<CairoRender::ImplCls>(new CairoRender::ImplCls(this, nullptr));
//}

// ======== CairoRender::CairoRender() ========
//CairoRender::CairoRender(cairo_surface_t *surface){
    //m_impl = std::unique_ptr<CairoRender::ImplCls>(new CairoRender::ImplCls(this, surface));
//}

CairoRender::CairoRender(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    m_impl = std::unique_ptr<CairoRender::ImplCls>(new CairoRender::ImplCls(this, pixelWidth, pixelHeight, resolutionX, resolutionY));
}

// ======== CairoRender::~CairoRender() ========
CairoRender::~CairoRender(){
}

void CairoRender::Paint(cairo_surface_t *surface){
    m_impl->Paint(surface);
}

bool CairoRender::WriteToPNG(const std::string &filename){
    return m_impl->WriteToPNG(filename);
}

//void CairoRender::SetCairoSurface(cairo_surface_t *surface){
    //m_impl->SetCairoSurface(surface);
//}

cairo_surface_t *CairoRender::GetCairoSurface() const{
    return m_impl->m_surface;
}

cairo_t *CairoRender::GetCairoContext() const{
    return m_impl->m_cr;
}

// ======== CairoRender::DrawPage() ========
void CairoRender::DrawPage(PagePtr page, VisibleParams visibleParams){
    Render::DrawPage(page, visibleParams);
    m_impl->DrawPage(page, visibleParams);
}

void CairoRender::DrawObject(ObjectPtr object){
    m_impl->DrawObject(object);
}

void CairoRender::SetLineWidth(double lineWidth){
    m_impl->SetLineWidth(lineWidth);
}

void CairoRender::UpdateStrokePattern(double r, double g, double b, double opacity){
    m_impl->UpdateStrokePattern(r, g, b, opacity);
}

void CairoRender::UpdateFillPattern(double r, double g, double b, double opacity){
    m_impl->UpdateFillPattern(r, g, b, opacity);
}

void CairoRender::Transform(cairo_matrix_t *matrix){
    m_impl->Transform(matrix);
}

void CairoRender::SaveState(){
    m_impl->SaveState();
}

void CairoRender::RestoreState(){
    m_impl->RestoreState();
}

void CairoRender::Clip(PathPtr clipPath){
    m_impl->Clip(clipPath);
}

void CairoRender::Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    m_impl->Rebuild(pixelWidth, pixelHeight, resolutionX, resolutionY);
}


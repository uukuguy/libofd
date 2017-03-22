#include <assert.h>

// Poppler MemStream
#include <Object.h>
#include <Stream.h>

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
#include "ofd/Image.h"
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
    void UpdateFillPattern(ShadingPtr fillShading);
    void Transform(cairo_matrix_t *matrix);

    void SaveState();
    void RestoreState();
    void Clip(PathPtr clipPath);
    void EoClip(PathPtr clipPath);

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

    void DrawRadialPathObject(cairo_t *cr, PathObject *pathObject);
    void doRadialShFill(cairo_t *cr, PathObject *pathObject);

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

        // FIXME
        //uint64_t id = object->ID;
        //if ( id <= 5 ){
            ////if ( id == 0 ){
                ////continue;
            ////} else if (id == 1 ){
                ////continue;
            ////} else if (id == 2 ){
                ////continue;
            ////} else if (id == 3 ){
                ////continue;
            ////} else if (id == 4 ){
                ////continue;
            ////} else { 
                ////continue;
            ////}

        //} else if (id >= 38 && id <= 42 ){
            //continue;
        //} else {
        //}

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


    //assert(font != nullptr);
    //LOG(DEBUG) << "DrawTextObject using font (ID=" << font->ID << ")";
    //assert(font->IsLoaded());

    //if ( font->ID != 3 ) {
        //return;
    //}

    cairo_font_face_t *font_face = nullptr;
    if ( font != nullptr && font->IsLoaded() ){
        font_face = font->GetCairoFontFace();
        assert(font_face != nullptr);

        cairo_set_font_face(cr, font_face);
    } else {
        return;
    }

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
        cairo_move_to(cr, p0.X, p0.Y);

        for ( size_t n = 1 ; n < numPoints ; n++ ){
            char flag = subpath->GetFlag(n);
            if ( flag == 'L' ){
                const Point_t &p = subpath->GetPoint(n);
                cairo_line_to(cr, p.X, p.Y);
            } else if ( flag == 'B' ){
                // 三次贝塞尔曲线
                const Point_t &p1 = subpath->GetPoint(n);
                const Point_t &p2 = subpath->GetPoint(n+1);
                const Point_t &p3 = subpath->GetPoint(n+2);
                cairo_curve_to(cr, p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y);
                //cairo_line_to(cr, p1.X, p1.Y);
                //cairo_line_to(cr, p2.X, p2.Y);
                //cairo_line_to(cr, p3.X, p3.Y);
                n += 2;
            } else if ( flag == 'Q' ){
                // 二次贝塞尔曲线
                // 需要转换成三次贝塞尔曲线，才能用cairo绘制。
                // http://blog.csdn.net/ch_soft/article/details/7401655
                // P_i^' = i/(n+1) P_(i-1) + (n+1-i)/(n+1) P_i
                Point_t p_2[2];
                Point_t p_3[3];
                p_2[0] = subpath->GetPoint(n);
                p_2[1] = subpath->GetPoint(n+1);
                n += 1;

                p_3[0] = p_2[0];
                p_3[1].X = (1.0/(2+1)) * p_2[0].X  + ((2+1-1)/(2+1)) * p_2[1].X;
                p_3[1].Y = (1.0/(2+1)) * p_2[0].Y  + ((2+1-1)/(2+1)) * p_2[1].Y;
                p_3[2].X = (2.0/(2+1)) * p_2[1].X  + ((2+1-2)/(2+1)) * p_2[2].X;
                p_3[2].Y = (2.0/(2+1)) * p_2[1].Y  + ((2+1-2)/(2+1)) * p_2[2].Y;

                cairo_curve_to(cr, p_3[0].X, p_3[0].Y, p_3[1].X, p_3[1].Y, p_3[2].X, p_3[2].Y);
            }
        }
        if ( subpath->IsClosed() ){
            cairo_line_to(cr, p0.X, p0.Y);
            //cairo_close_path(cr);
        }
    }
}

void CairoRender::ImplCls::DrawPathObject(cairo_t *cr, PathObject *pathObject){
    if ( pathObject == nullptr ) return;

    // FIXME 渐变色缺陷调试
    if ( pathObject->ID != 71 ){
        return;
        //LOG(DEBUG) << "Debug missing path image.";
    }
    //LOG(ERROR) << pathObject->to_string();

    //if ( pathObject->FillShading != nullptr ){
        //doRadialShFill(cr, pathObject);
        //return;
    //}


    //setDefaultCTM(cr);
    //clearCTM(cr);

    cairo_matrix_t matrix;
    matrix.xx = pathObject->CTM[0];

    matrix.yx = pathObject->CTM[1];
    matrix.xy = pathObject->CTM[2];

    //// FIXME
    //matrix.yx = 0;//pathObject->CTM[2];
    //matrix.xy = 0;//pathObject->CTM[1];


    matrix.yy = pathObject->CTM[3];
    matrix.x0 = pathObject->CTM[4];
    matrix.y0 = pathObject->CTM[5];
    cairo_transform(cr, &matrix);

    showCairoMatrix(cr, "CairoRender", "DrawPathObject");


    PathPtr path = pathObject->GetPath();
    DoCairoPath(cr, path);

    cairo_set_line_width(cr, pathObject->LineWidth);

    ColorPtr strokeColor = pathObject->GetStrokeColor();
    if ( strokeColor != nullptr ){
        double r, g, b, a;
        std::tie(r, g, b, a) = strokeColor->GetRGBA();
        UpdateStrokePattern(r, g, b, a);
        //LOG(DEBUG) << "DrawPathObject() rgb = (" << r << "," << g << "," << b << ")";

        cairo_set_source(cr, m_strokePattern);
        cairo_stroke(cr);
    } else {
        // FIXME 渐变色缺陷
        if ( pathObject->FillShading != nullptr ){
            UpdateFillPattern(pathObject->FillShading);
        } else {
            ColorPtr fillColor = pathObject->GetFillColor();
            if ( fillColor != nullptr ){
                double r, g, b, a;
                std::tie(r, g, b, a) = fillColor->GetRGBA();
                UpdateFillPattern(r, g, b, a);
                //LOG(DEBUG) << "DrawPathObject() rgb = (" << r << "," << g << "," << b << ")";
            }
        }
        cairo_set_source(cr, m_fillPattern);

        if ( pathObject->Rule == ofd::PathRule::EvenOdd ){
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
        }

        cairo_fill(cr);

        if ( pathObject->Rule == ofd::PathRule::EvenOdd ){
            EoClip(path);
        } else {
            Clip(path);
        }
    }

}

//// Defined in CairoRender_Poppler.cc
//cairo_surface_t *createImageSurface(Stream *str, int widthA, int heightA, int scaledWidth, int scaledHeight, int nComps, int nBits);

namespace ofd{
class MemStream : public Stream{
    public:
        MemStream(char *data, size_t startA, size_t dataSize): 
            buf(data), length(dataSize), start(startA), bufPtr(data), bufEnd(data+startA+dataSize),
            needFree(false){
        }
        virtual ~MemStream(){
            if (needFree){
                delete[] buf;
            }
        }

        virtual StreamKind getKind(){return strWeird;} ;
        virtual void reset(){
            bufPtr = buf + start;
        }
        virtual void close(){
        }
        // Get next char from stream.
        virtual int getChar(){
            return (bufPtr < bufEnd) ? (*bufPtr++ & 0xff) : EOF; 
        }

        // Peek at next char in stream.
        virtual int lookChar(){
            return (bufPtr < bufEnd) ? (*bufPtr & 0xff) : EOF; 
        }

        virtual Goffset getPos() { return (int)(bufPtr - buf); }
        virtual void setPos(Goffset pos, int dir = 0){
            Guint i;

            if (dir >= 0) {
                i = pos;
            } else {
                i = start + length - pos;
            }
            if (i < start) {
                i = start;
            } else if (i > start + length) {
                i = start + length;
            }
            bufPtr = buf + i;
        }
        virtual Goffset getStart() { return start; }
        //virtual void moveStart(Goffset delta);

        //if needFree = true, the stream will delete buf when it is destroyed
        //otherwise it will not touch it. Default value is false
        virtual void setNeedFree (GBool val) { needFree = val; }

        virtual int getUnfilteredChar () { return getChar(); }
        virtual void unfilteredReset () { reset (); } 

        virtual GBool hasGetChars() { return true; }
        virtual int getChars(int nChars, Guchar *buffer) {
            int n;

            if (nChars <= 0) {
                return 0;
            }
            if (bufEnd - bufPtr < nChars) {
                n = (int)(bufEnd - bufPtr);
            } else {
                n = nChars;
            }
            memcpy(buffer, bufPtr, n);
            bufPtr += n;
            return n;
        }


        virtual GBool isBinary(GBool last = gTrue) { return last; }
        virtual BaseStream *getBaseStream() { return nullptr; }
        virtual Stream *getUndecodedStream() { return this; }
        virtual Dict *getDict() { return nullptr; }
    private:
        char *buf;
        size_t length;
        size_t start;
        char *bufPtr;
        char *bufEnd;
        bool needFree;

}; // class MemStream
}

void CairoRender::ImplCls::DrawImageObject(cairo_t *cr, ImageObject *imageObject){
    if ( imageObject == nullptr ) return;


    ofd::ImagePtr image = imageObject->GetImage();
    if ( image == nullptr ) return;
    int widthA = image->width;
    int heightA = image->height;
    int nComps = image->nComps;
    int nBits = image->nBits;

    char *imageData = image->GetImageData();
    size_t imageDataSize = image->GetImageDataSize();

    //MemStream *memStream = new MemStream(imageData, 0, imageDataSize, nullptr);
    ofd::MemStream *memStream = new ofd::MemStream(imageData, 0, imageDataSize);
    //memStream->reset();

    cairo_surface_t *imageSurface = nullptr;
    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);

    int scaledWidth, scaledHeight;

    cairo_save(cr);

    cairo_matrix_t objMatrix;
    objMatrix.xx = imageObject->CTM[0];
    objMatrix.yx = imageObject->CTM[1];
    objMatrix.xy = imageObject->CTM[2];
    objMatrix.yy = imageObject->CTM[3];
    objMatrix.x0 = imageObject->CTM[4];
    objMatrix.y0 = imageObject->CTM[5];
    cairo_transform(cr, &objMatrix);

    cairo_get_matrix(cr, &matrix);
    getImageScaledSize (&matrix, widthA, heightA, &scaledWidth, &scaledHeight);

    imageSurface = createImageSurface(memStream, widthA, heightA, scaledWidth, scaledHeight, nComps, nBits);
    if ( imageSurface == nullptr ){
        delete memStream;
        cairo_restore(cr);
        return;
    }
    std::string pngFileName = "/tmp/Image_draw_" + std::to_string(image->ID) + ".png";
    cairo_surface_write_to_png(imageSurface, pngFileName.c_str());

    int width = cairo_image_surface_get_width (imageSurface);
    int height = cairo_image_surface_get_height (imageSurface);
    cairo_filter_t filter = CAIRO_FILTER_BILINEAR;
    if (width == widthA && height == heightA){
        bool interpolate = false;
        filter = getFilterForSurface (imageSurface, cr, interpolate);
    }

    //if (!inlineImg) [> don't read stream twice if it is an inline image <]
        //setMimeData(state, str, ref, colorMap, imageSurface);

    cairo_pattern_t *pattern = cairo_pattern_create_for_surface(imageSurface);
    cairo_surface_destroy (imageSurface);
    if (cairo_pattern_status (pattern))
        return;

    cairo_pattern_set_filter (pattern, filter);

    //if (!m_printing)
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    cairo_matrix_init_translate (&matrix, 0, height);
    cairo_matrix_scale (&matrix, width, -height);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
        cairo_pattern_destroy (pattern);
        return;
    }

    cairo_pattern_t *maskPattern = nullptr;
    //if (!m_maskPattern && m_fillOpacity != 1.0) {
        //maskPattern = cairo_pattern_create_rgba (1., 1., 1., m_fillOpacity);
    //} else if ( m_maskPattern != nullptr ) {
        //maskPattern = cairo_pattern_reference(m_maskPattern);
    //}


    cairo_set_source(cr, pattern);
    //if (!m_printing)
    cairo_rectangle(cr, 0., 0., 1., 1.);

    if (maskPattern != nullptr ) {
        //if (!m_printing)
            cairo_clip(cr);
        //if ( m_maskPattern != nullptr )
            //cairo_set_matrix(m_cairo, &m_mask_matrix);
        cairo_mask(cr, maskPattern);
    } else {
        //if (m_printing)
            //cairo_paint(cr);
        //else
            cairo_fill(cr);
    }
    cairo_restore(cr);

    cairo_pattern_destroy(maskPattern);

    //if ( m_cairoShape) {
        //cairo_save(m_cairoShape);
        //cairo_set_source(m_cairoShape, pattern);
        //if ( m_printing ) {
            //cairo_paint(m_cairoShape);
        //} else {
            //cairo_rectangle(m_cairoShape, 0., 0., 1., 1.);
            //cairo_fill(m_cairoShape);
        //}
        //cairo_restore(m_cairoShape);
    //}

    cairo_pattern_destroy (pattern);

    delete memStream;
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

void CairoRender::ImplCls::UpdateFillPattern(ShadingPtr fillShading){
    if ( m_fillPattern != nullptr ){
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = nullptr;
    }
    m_fillPattern = fillShading->CreateFillPattern(m_cr);
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

void CairoRender::ImplCls::EoClip(PathPtr clipPath){
    DoCairoPath(m_cr, clipPath);
    cairo_set_fill_rule(m_cr, CAIRO_FILL_RULE_EVEN_ODD);
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

void CairoRender::EoClip(PathPtr clipPath){
    m_impl->EoClip(clipPath);
}

static inline ofd::ColorPtr getShadingColorRadialHelper(double t0, double t1, double t, ofd::AxialShading*shading) {
    ofd::ColorPtr color = nullptr;
    if (t0 < t1) {
        if (t < t0) {
            color = shading->GetColor(t0);
        } else if (t > t1) {
            color = shading->GetColor(t1);
        } else {
            color = shading->GetColor(t);
        }
    } else {
        if (t > t0) {
            color = shading->GetColor(t0);
        } else if (t < t1) {
            color = shading->GetColor(t1);
        } else {
            color = shading->GetColor(t);
        }
    }
    return color;
}

void CairoRender::Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY){
    m_impl->Rebuild(pixelWidth, pixelHeight, resolutionX, resolutionY);
}

// Max number of splits along the t axis for an axial shading fill.
#define axialMaxSplits 256

// Max delta allowed in any color component for an axial shading fill.
#define axialColorDelta (1 / 256.0)

// Max number of splits along the t axis for a radial shading fill.
#define radialMaxSplits 256

// Max delta allowed in any color component for a radial shading fill.
#define radialColorDelta (1 / 256.0)

//GfxRadialShading *shading

void CairoRender::ImplCls::DrawRadialPathObject(cairo_t *cr, PathObject *pathObject){
        //if ( pathObject->FillShading != nullptr )
    cairo_matrix_t matrix;
    matrix.xx = pathObject->CTM[0];

    matrix.yx = pathObject->CTM[1];
    matrix.xy = pathObject->CTM[2];

    //// FIXME
    //matrix.yx = 0;//pathObject->CTM[2];
    //matrix.xy = 0;//pathObject->CTM[1];


    matrix.yy = pathObject->CTM[3];
    matrix.x0 = pathObject->CTM[4];
    matrix.y0 = pathObject->CTM[5];
    cairo_transform(cr, &matrix);

    showCairoMatrix(cr, "CairoRender", "DrawPathObject");


    PathPtr path = pathObject->GetPath();
    DoCairoPath(cr, path);

    cairo_set_line_width(cr, pathObject->LineWidth);

            UpdateFillPattern(pathObject->FillShading);
        cairo_set_source(cr, m_fillPattern);

        if ( pathObject->Rule == ofd::PathRule::EvenOdd ){
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
        }

        cairo_fill(cr);

        if ( pathObject->Rule == ofd::PathRule::EvenOdd ){
            EoClip(path);
        } else {
            Clip(path);
        }
}

    //ofd::RadialShading *shading = (ofd::RadialShading*)m_shading.get();
void CairoRender::ImplCls::doRadialShFill(cairo_t *cr, PathObject *pathObject){
    if (pathObject == nullptr) return;

    assert(pathObject->FillShading != nullptr);
    ofd::RadialShading *shading  = (ofd::RadialShading*)pathObject->FillShading.get();

    double *ctm = &pathObject->CTM[0];
    double xMin = pathObject->Boundary.XMin;
    double yMin = pathObject->Boundary.YMin;
    double xMax = pathObject->Boundary.XMax;
    double yMax = pathObject->Boundary.YMax;

    double x0, y0, r0, x1, y1, r1, t0, t1;
    //int nComps;
    ofd::ColorPtr colorA, colorB;
    double xa, ya, xb, yb, ra, rb;
    double ta, tb, sa, sb;
    double sz, xz, yz, sMin, sMax;
    GBool enclosed;
    int ia, ib, k, n;
    //double *ctm;
    double theta, alpha, angle, t;
    GBool needExtend = gTrue;


    // get the shading info
    //shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
    x0 = shading->StartPoint.X;
    y0 = shading->StartPoint.Y;
    x1 = shading->EndPoint.X;
    y1 = shading->EndPoint.Y;
    r0 = shading->StartRadius;
    r1 = shading->EndRadius;

    // TODO ???
    //t0 = shading->getDomain0();
    //t1 = shading->getDomain1();
    t0 = 0;
    t1 = 1;

    //nComps = shading->getColorSpace()->getNComps();
    //nComps = 4;

    // Compute the point at which r(s) = 0; check for the enclosed
    // circles case; and compute the angles for the tangent lines.
    if (x0 == x1 && y0 == y1) {
        enclosed = gTrue;
        theta = 0; // make gcc happy
        sz = 0; // make gcc happy
    } else if (r0 == r1) {
        enclosed = gFalse;
        theta = 0;
        sz = 0; // make gcc happy
    } else {
        sz = (r1 > r0) ? -r0 / (r1 - r0) : -r1 / (r0 - r1);
        xz = x0 + sz * (x1 - x0);
        yz = y0 + sz * (y1 - y0);
        enclosed = (xz - x0) * (xz - x0) + (yz - y0) * (yz - y0) <= r0 * r0;
        theta = asin(r0 / sqrt((x0 - xz) * (x0 - xz) + (y0 - yz) * (y0 - yz)));
        if (r0 > r1) {
            theta = -theta;
        }
    }
    if (enclosed) {
        alpha = 0;
    } else {
        alpha = atan2(y1 - y0, x1 - x0);
    }

    // compute the (possibly extended) s range
    // FIXME
    sMin = 0;
    sMax = 1;
    //state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    //if (enclosed) {
        //sMin = 0;
        //sMax = 1;
    //} else {
        //sMin = 1;
        //sMax = 0;
        //// solve for x(s) + r(s) = xMin
        //if ((x1 + r1) - (x0 + r0) != 0) {
            //sa = (xMin - (x0 + r0)) / ((x1 + r1) - (x0 + r0));
            //if (sa < sMin) {
                //sMin = sa;
            //} else if (sa > sMax) {
                //sMax = sa;
            //}
        //}
        //// solve for x(s) - r(s) = xMax
        //if ((x1 - r1) - (x0 - r0) != 0) {
            //sa = (xMax - (x0 - r0)) / ((x1 - r1) - (x0 - r0));
            //if (sa < sMin) {
                //sMin = sa;
            //} else if (sa > sMax) {
                //sMax = sa;
            //}
        //}
        //// solve for y(s) + r(s) = yMin
        //if ((y1 + r1) - (y0 + r0) != 0) {
            //sa = (yMin - (y0 + r0)) / ((y1 + r1) - (y0 + r0));
            //if (sa < sMin) {
                //sMin = sa;
            //} else if (sa > sMax) {
                //sMax = sa;
            //}
        //}
        //// solve for y(s) - r(s) = yMax
        //if ((y1 - r1) - (y0 - r0) != 0) {
            //sa = (yMax - (y0 - r0)) / ((y1 - r1) - (y0 - r0));
            //if (sa < sMin) {
                //sMin = sa;
            //} else if (sa > sMax) {
                //sMax = sa;
            //}
        //}
        //// check against sz
        //if (r0 < r1) {
            //if (sMin < sz) {
                //sMin = sz;
            //}
        //} else if (r0 > r1) {
            //if (sMax > sz) {
                //sMax = sz;
            //}
        //}
        //// check the 'extend' flags
        //// FIXME
        ////if (!shading->getExtend0() && sMin < 0) {
            ////sMin = 0;
        ////}
        ////if (!shading->getExtend1() && sMax > 1) {
            ////sMax = 1;
        ////}
    //}

    // FIXME
    //if (out->useShadedFills( shading->getType() ) &&
            //out->radialShadedFill(state, shading, sMin, sMax)) {
        //return;
    //}

    // compute the number of steps into which circles must be divided to
    // achieve a curve flatness of 0.1 pixel in device space for the
    // largest circle (note that "device space" is 72 dpi when generating
    // PostScript, hence the relatively small 0.1 pixel accuracy)
    // FIXME
    //ctm = state->getCTM();
    t = fabs(ctm[0]);
    if (fabs(ctm[1]) > t) {
        t = fabs(ctm[1]);
    }
    if (fabs(ctm[2]) > t) {
        t = fabs(ctm[2]);
    }
    if (fabs(ctm[3]) > t) {
        t = fabs(ctm[3]);
    }
    if (r0 > r1) {
        t *= r0;
    } else {
        t *= r1;
    }
    if (t < 1) {
        n = 3;
    } else {
        n = (int)(M_PI / acos(1 - 0.1 / t));
        if (n < 3) {
            n = 3;
        } else if (n > 200) {
            n = 200;
        }
    }

    // setup for the start circle
    ia = 0;
    sa = sMin;
    ta = t0 + sa * (t1 - t0);
    xa = x0 + sa * (x1 - x0);
    ya = y0 + sa * (y1 - y0);
    ra = r0 + sa * (r1 - r0);
    colorA = getShadingColorRadialHelper(t0, t1, ta, shading);

    //needExtend = !out->radialShadedSupportExtend(state, shading);

    // fill the circles
    while (ia < radialMaxSplits) {

        // go as far along the t axis (toward t1) as we can, such that the
        // color difference is within the tolerance (radialColorDelta) --
        // this uses bisection (between the current value, t, and t1),
        // limited to radialMaxSplits points along the t axis; require at
        // least one split to avoid problems when the innermost and
        // outermost colors are the same
        ib = radialMaxSplits;
        sb = sMax;
        tb = t0 + sb * (t1 - t0);
        colorB = getShadingColorRadialHelper(t0, t1, tb, shading);
        while (ib - ia > 1) {
            //if (isSameGfxColor(colorB, colorA, nComps, radialColorDelta)) {
            if ( colorB->IsSameColor(colorA, radialColorDelta) ){
                // The shading is not necessarily lineal so having two points with the
                // same color does not mean all the areas in between have the same color too
                int ic = ia + 1;
                for (; ic <= ib; ic++) {
                    ofd::ColorPtr colorC;
                    const double sc = sMin + ((double)ic / (double)radialMaxSplits) * (sMax - sMin);
                    const double tc = t0 + sc * (t1 - t0);
                    colorC = getShadingColorRadialHelper(t0, t1, tc, shading);
                    //if (!isSameGfxColor(colorC, colorA, nComps, radialColorDelta)) {
                    if ( !colorC->IsSameColor(colorA, radialColorDelta) ){
                        break;
                    }
                }
                ib = (ic > ia + 1) ? ic - 1 : ia + 1;
                sb = sMin + ((double)ib / (double)radialMaxSplits) * (sMax - sMin);
                tb = t0 + sb * (t1 - t0);
                colorB = getShadingColorRadialHelper(t0, t1, tb, shading);
                break;
            }
            ib = (ia + ib) / 2;
            sb = sMin + ((double)ib / (double)radialMaxSplits) * (sMax - sMin);
            tb = t0 + sb * (t1 - t0);
            colorB = getShadingColorRadialHelper(t0, t1, tb, shading);
        }

        // compute center and radius of the circle
        xb = x0 + sb * (x1 - x0);
        yb = y0 + sb * (y1 - y0);
        rb = r0 + sb * (r1 - r0);

        // use the average of the colors at the two circles

        //for (k = 0; k < nComps; ++k) {
            //colorA.c[k] = (colorA.c[k] + colorB.c[k]) / 2;
        //}
        colorA->AverageColor(colorB);
        //state->setFillColor(&colorA);
        //m_fillColor = colorA

        // FIXME
        //if (out->useFillColorStop())
            //out->updateFillColorStop(state, (sa - sMin)/(sMax - sMin));
        //else
            //out->updateFillColor(state);

        if (needExtend) {
            if (enclosed) {
                // construct path for first circle (counterclockwise)
                //state->moveTo(xa + ra, ya);
                cairo_move_to(cr, xa + ra, ya);
                for (k = 1; k < n; ++k) {
                    angle = ((double)k / (double)n) * 2 * M_PI;
                    //state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
                    cairo_line_to(cr, xa + ra * cos(angle), ya + ra * sin(angle));
                }
                //state->closePath();
                cairo_close_path(cr);

                // construct and append path for second circle (clockwise)
                //state->moveTo(xb + rb, yb);
                cairo_move_to(cr, xb + rb, yb);
                for (k = 1; k < n; ++k) {
                    angle = -((double)k / (double)n) * 2 * M_PI;
                    //state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
                    cairo_line_to(cr, xb + rb * cos(angle), yb + rb * sin(angle));
                }
                //state->closePath();
                cairo_close_path(cr);
            } else {
                // construct the first subpath (clockwise)
                //state->moveTo(xa + ra * cos(alpha + theta + 0.5 * M_PI),
                        //ya + ra * sin(alpha + theta + 0.5 * M_PI));
                cairo_move_to(cr, xa + ra * cos(alpha + theta + 0.5 * M_PI),
                        ya + ra * sin(alpha + theta + 0.5 * M_PI));
                for (k = 0; k < n; ++k) {
                    angle = alpha + theta + 0.5 * M_PI
                        - ((double)k / (double)n) * (2 * theta + M_PI);
                    //state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
                    cairo_line_to(cr, xb + rb * cos(angle), yb + rb * sin(angle));
                }
                for (k = 0; k < n; ++k) {
                    angle = alpha - theta - 0.5 * M_PI
                        + ((double)k / (double)n) * (2 * theta - M_PI);
                    //state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
                    cairo_line_to(cr, xa + ra * cos(angle), ya + ra * sin(angle));
                }
                //state->closePath();
                cairo_close_path(cr);

                // construct the second subpath (counterclockwise)
                //state->moveTo(xa + ra * cos(alpha + theta + 0.5 * M_PI),
                        //ya + ra * sin(alpha + theta + 0.5 * M_PI));
                cairo_move_to(cr, xa + ra * cos(alpha + theta + 0.5 * M_PI),
                        ya + ra * sin(alpha + theta + 0.5 * M_PI));
                for (k = 0; k < n; ++k) {
                    angle = alpha + theta + 0.5 * M_PI
                        + ((double)k / (double)n) * (-2 * theta + M_PI);
                    //state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
                    cairo_line_to(cr, xb + rb * cos(angle), yb + rb * sin(angle));
                }
                for (k = 0; k < n; ++k) {
                    angle = alpha - theta - 0.5 * M_PI
                        + ((double)k / (double)n) * (2 * theta + M_PI);
                    //state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
                    cairo_line_to(cr, xa + ra * cos(angle), ya + ra * sin(angle));
                }
                //state->closePath();
                cairo_close_path(cr);
            }
        }

        //if (!out->useFillColorStop()) {
            // fill the path
            //out->fill(state);
            //state->clearPath();
        //}
        DrawRadialPathObject(cr, pathObject);

        // step to the next value of t
        ia = ib;
        sa = sb;
        ta = tb;
        xa = xb;
        ya = yb;
        ra = rb;
        colorA = colorB;
    }

    // FIXME
    //if (out->useFillColorStop()) {
        // make sure we add stop color when sb = sMax
        // FIXME
        //state->setFillColor(&colorA);
        //out->updateFillColorStop(state, (sb - sMin)/(sMax - sMin));

        // fill the path
        //state->moveTo(xMin, yMin);
        //state->lineTo(xMin, yMax);
        //state->lineTo(xMax, yMax);
        //state->lineTo(xMax, yMin);
        //state->closePath();
        cairo_move_to(cr, xMin, yMin);
        cairo_line_to(cr, xMin, yMax);
        cairo_line_to(cr, xMax, yMax);
        cairo_line_to(cr, xMax, yMin);
        cairo_close_path(cr);

        //out->fill(state);
        //state->clearPath();
        //cairo_clear_path(cr);
        DrawRadialPathObject(cr, pathObject);
    //}

    if (!needExtend)
        return;

    if (enclosed) {
        // extend the smaller circle
        //if ((shading->getExtend0() && r0 <= r1) ||
                //(shading->getExtend1() && r1 < r0)) {
        if ( shading->Extend ){
            if (r0 <= r1) {
                ta = t0;
                ra = r0;
                xa = x0;
                ya = y0;
            } else {
                ta = t1;
                ra = r1;
                xa = x1;
                ya = y1;
            }
            colorA = shading->GetColor(ta);
            // FIXME
            //state->setFillColor(&colorA);
            //out->updateFillColor(state);
            //state->moveTo(xa + ra, ya);
            cairo_move_to(cr, xa + ra, ya);
            for (k = 1; k < n; ++k) {
                angle = ((double)k / (double)n) * 2 * M_PI;
                //state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
                cairo_line_to(cr, xa + ra * cos(angle), ya + ra * sin(angle));
            }
            //state->closePath();
            cairo_close_path(cr);
            //out->fill(state);
            //state->clearPath();
            //cairo_clear_path(cr);

            DrawRadialPathObject(cr, pathObject);

        }

        // extend the larger circle
        // FIXME
        //if ((shading->getExtend0() && r0 > r1) ||
                //(shading->getExtend1() && r1 >= r0)) {
            if (r0 > r1) {
                ta = t0;
                ra = r0;
                xa = x0;
                ya = y0;
            } else {
                ta = t1;
                ra = r1;
                xa = x1;
                ya = y1;
            }
            colorA = shading->GetColor(ta);
            // FIXME
            //state->setFillColor(&colorA);
            //out->updateFillColor(state);

            //state->moveTo(xMin, yMin);
            //state->lineTo(xMin, yMax);
            //state->lineTo(xMax, yMax);
            //state->lineTo(xMax, yMin);
            //state->closePath();
            cairo_move_to(cr, xMin, yMin);
            cairo_line_to(cr, xMin, yMax);
            cairo_line_to(cr, xMax, yMax);
            cairo_line_to(cr, xMax, yMin);
            cairo_close_path(cr);

            //state->moveTo(xa + ra, ya);
            cairo_move_to(cr, xa + ra, ya);
            for (k = 1; k < n; ++k) {
                angle = ((double)k / (double)n) * 2 * M_PI;
                //state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
                cairo_line_to(cr, xa + ra * cos(angle), ya + ra * sin(angle));
            }
            //state->closePath();
            cairo_close_path(cr);
            //out->fill(state);
            //state->clearPath();
            //cairo_clear_path();

            DrawRadialPathObject(cr, pathObject);
        //}
    }
}

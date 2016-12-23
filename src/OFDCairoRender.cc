#include <assert.h>
#include "OFDCairoRender.h"
#include "OFDPage.h"
#include "OFDObject.h"
#include "OFDTextObject.h"
#include "utils/logger.h"

using namespace ofd;

// **************** class OFDCairoRender::ImplCls ****************

class OFDCairoRender::ImplCls {
public:
    ImplCls(cairo_surface_t *surface);
    ~ImplCls();

    void Draw(OFDPage *page);

    cairo_surface_t *m_surface;

}; // class OFDCairoRender::ImplCls

OFDCairoRender::ImplCls::ImplCls(cairo_surface_t *surface) : m_surface(surface){
}


OFDCairoRender::ImplCls::~ImplCls(){
}

void OFDCairoRender::SetCairoSurface(cairo_surface_t *surface){
    m_impl->m_surface = surface;
}

// ======== OFDCairoRender::ImplCls::Draw() ========
void OFDCairoRender::ImplCls::Draw(OFDPage *page){
    if ( m_surface == nullptr ) return;

    cairo_t *cr = cairo_create(m_surface);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    cairo_stroke(cr);

    if ( page == nullptr ) return;
    //size_t numLayers = page->GetLayersCount();
    const OFDLayerPtr bodyLayer = page->GetBodyLayer(); 
    if ( bodyLayer == nullptr ) {
        //LOG(WARNING) << "page->GetBodyLayer() return nullptr. Maybe NULL content.";
        return;
    }
    size_t numObjects = bodyLayer->GetObjectsCount();
    if ( numObjects == 0 ) return;
    //LOG(DEBUG) << numLayers << " layers in page.";
    //LOG(DEBUG) << numObjects << " objects in body layer.";

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

// **************** class OFDCairoRender ****************

OFDCairoRender::OFDCairoRender(){
    m_impl = std::unique_ptr<OFDCairoRender::ImplCls>(new OFDCairoRender::ImplCls(nullptr));
}

OFDCairoRender::OFDCairoRender(cairo_surface_t *surface){
    m_impl = std::unique_ptr<OFDCairoRender::ImplCls>(new OFDCairoRender::ImplCls(surface));
}

OFDCairoRender::~OFDCairoRender(){
}

void OFDCairoRender::Draw(OFDPage *page){
    m_impl->Draw(page);
}


#include "OFDCairoRender.h"
#include "OFDPage.h"

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
    if ( page == nullptr ) return;

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


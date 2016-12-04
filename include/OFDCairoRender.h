#ifndef __OFD_CAIRORENDER_H__
#define __OFD_CAIRORENDER_H__

#include <memory>
#include <cairo/cairo.h>
#include "OFDRender.h"

namespace ofd{

    class OFDCairoRender : public OFDRender {
    public:
        OFDCairoRender();
        OFDCairoRender(cairo_surface_t *surface);
        virtual ~OFDCairoRender();

        void SetCairoSurface(cairo_surface_t *surface);
        virtual void Draw(OFDPage *page) override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDCairoRender

}; // namespace ofd

#endif // __OFD_CAIRORENDER_H__

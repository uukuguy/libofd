#ifndef __OFD_CAIRORENDER_H__
#define __OFD_CAIRORENDER_H__

#include <memory>
#include <cairo/cairo.h>
#include "OFDRender.h"

namespace ofd{

    // ======== class OFDCairoRender ========
    class OFDCairoRender : public OFDRender {
    public:
        OFDCairoRender();
        OFDCairoRender(cairo_surface_t *surface);
        virtual ~OFDCairoRender();

        void SetCairoSurface(cairo_surface_t *surface);
        cairo_surface_t *GetCairoSurface() const;
        cairo_t *GetCairoContext() const;

        virtual void DrawPage(OFDPagePtr page, Render::DrawParams drawParams) override;
        void DrawObject(OFDObjectPtr object);

        void UpdateStrokePattern(double r, double g, double b, double opacity);
        void UpdateFillPattern(double r, double g, double b, double opacity);
        void Transform(cairo_matrix_t *matrix);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDCairoRender
    typedef std::shared_ptr<OFDCairoRender> OFDCairoRenderPtr;

}; // namespace ofd

#endif // __OFD_CAIRORENDER_H__

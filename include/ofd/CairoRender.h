#ifndef __OFD_CAIRORENDER_H__
#define __OFD_CAIRORENDER_H__

#include <memory>
#include <cairo/cairo.h>
#include "ofd/Render.h"

namespace ofd{

    // ======== class CairoRender ========
    class CairoRender : public Render {
    public:
        //CairoRender();
        CairoRender(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
        //CairoRender(cairo_surface_t *surface);
        virtual ~CairoRender();

        void Paint(cairo_surface_t *surface);
        bool WriteToPNG(const std::string &filename);

        void Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
        //void SetCairoSurface(cairo_surface_t *surface);
        cairo_surface_t *GetCairoSurface() const;
        cairo_t *GetCairoContext() const;

        virtual void DrawPage(PagePtr page, VisibleParams visibleParams) override;
        void DrawObject(ObjectPtr object);

        void SetLineWidth(double lineWidth);
        void UpdateStrokePattern(double r, double g, double b, double opacity);
        void UpdateFillPattern(double r, double g, double b, double opacity);
        void Transform(cairo_matrix_t *matrix);

        void SaveState();
        void RestoreState();
        void Clip(PathPtr clipPath);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class CairoRender
    typedef std::shared_ptr<CairoRender> CairoRenderPtr;

}; // namespace ofd

#endif // __OFD_CAIRORENDER_H__

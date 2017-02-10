#ifndef __OFD_CAIRORENDER_H__
#define __OFD_CAIRORENDER_H__

#include <memory>
#include <cairo/cairo.h>
#include "OFDRender.h"
#include "ofd/OfdPath.h"

namespace ofd{

    // ======== class OFDCairoRender ========
    class OFDCairoRender : public OFDRender {
    public:
        //OFDCairoRender();
        OFDCairoRender(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
        //OFDCairoRender(cairo_surface_t *surface);
        virtual ~OFDCairoRender();

        void Paint(cairo_surface_t *surface);
        bool WriteToPNG(const std::string &filename);

        void Rebuild(double pixelWidth, double pixelHeight, double resolutionX, double resolutionY);
        //void SetCairoSurface(cairo_surface_t *surface);
        cairo_surface_t *GetCairoSurface() const;
        cairo_t *GetCairoContext() const;

        virtual void DrawPage(OFDPagePtr page, Render::DrawParams drawParams) override;
        void DrawObject(OFDObjectPtr object);

        void SetLineWidth(double lineWidth);
        void UpdateStrokePattern(double r, double g, double b, double opacity);
        void UpdateFillPattern(double r, double g, double b, double opacity);
        void Transform(cairo_matrix_t *matrix);

        void SaveState();
        void RestoreState();
        void Clip(OfdPathPtr clipPath);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDCairoRender
    typedef std::shared_ptr<OFDCairoRender> OFDCairoRenderPtr;

}; // namespace ofd

#endif // __OFD_CAIRORENDER_H__

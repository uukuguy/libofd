#ifndef __OFD_RENDER_H__
#define __OFD_RENDER_H__

#include <memory>
#include <tuple>
#include "OFDCommon.h"

namespace ofd{

    namespace Render{
        typedef std::tuple<double, double, double> DrawParams;
    }; // namespace Render

    // ======== class OFDRender ========
    class OFDRender {
    public:
        OFDRender();
        virtual ~OFDRender();

        virtual void DrawPage(OFDPagePtr page, Render::DrawParams draParams);
        Render::DrawParams GetDrawParams() const;
        void SetDrawParams(Render::DrawParams drawParams);

    private:
        Render::DrawParams m_drawParams;

    }; // class OFDRender

    typedef std::shared_ptr<OFDRender> OFDRenderPtr;

    enum class RenderType{
        Cairo,
    };

    class OFDRenderFactory{
    public:
        static OFDRenderPtr CreateRenderInstance(RenderType renderType); 
    }; // class OFDRenderFactory

}; // namespace ofd

#endif // __OFD_RENDER_H__

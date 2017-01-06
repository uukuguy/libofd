#ifndef __OFD_RENDER_H__
#define __OFD_RENDER_H__

#include <memory>
#include <tuple>

namespace ofd{

    class OFDPage;

    namespace Render{
        typedef std::tuple<double, double, double> DrawParams;
    }; // namespace Render

    // ======== class OFDRender ========
    class OFDRender {
    public:
        OFDRender();
        virtual ~OFDRender();

        virtual void Draw(OFDPage *page, Render::DrawParams draParams);
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

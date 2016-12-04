#ifndef __OFD_RENDER_H__
#define __OFD_RENDER_H__

#include <memory>

namespace ofd{

    class OFDPage;

    class OFDRender {
    public:
        OFDRender();
        virtual ~OFDRender();

        virtual void Draw(OFDPage *page) = 0;

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

#include "ofd/Render.h"
#include "ofd/Page.h"
//#include "ofd/CairoRender.h"

using namespace ofd;

Render::Render() : m_visibleParams(std::make_tuple(0.0, 0.0, 1.0)){
}

Render::~Render(){
}

void Render::DrawPage(PagePtr page, VisibleParams visibleParams){
    m_visibleParams = visibleParams;
}

VisibleParams Render::GetVisibleParams() const{
    return m_visibleParams;
}

void Render::SetVisibleParams(VisibleParams visibleParams){
    m_visibleParams = visibleParams;
}

//RenderPtr RenderFactory::CreateRenderInstance(RenderType renderType){
    //if ( renderType == RenderType::Cairo ){
        //return std::make_shared<OFDCairoRender>();
    //} else {
        //return nullptr;
    //}
//}

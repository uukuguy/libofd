#include "OFDRender.h"
#include "OFDCairoRender.h"

using namespace ofd;

OFDRender::OFDRender() : m_drawParams(std::make_tuple(0.0, 0.0, 1.0)){
}

OFDRender::~OFDRender(){
}

void OFDRender::DrawPage(OFDPagePtr page, Render::DrawParams drawParams){
    m_drawParams = drawParams;
}

Render::DrawParams OFDRender::GetDrawParams() const{
    return m_drawParams;
}

void OFDRender::SetDrawParams(Render::DrawParams drawParams){
    m_drawParams = drawParams;
}

//OFDRenderPtr OFDRenderFactory::CreateRenderInstance(RenderType renderType){
    //if ( renderType == RenderType::Cairo ){
        //return std::make_shared<OFDCairoRender>();
    //} else {
        //return nullptr;
    //}
//}

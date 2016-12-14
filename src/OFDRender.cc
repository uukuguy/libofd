#include "OFDRender.h"
#include "OFDCairoRender.h"

using namespace ofd;

OFDRender::OFDRender(){
}

OFDRender::~OFDRender(){
}

void OFDRender::Draw(OFDPage *page){
}

OFDRenderPtr OFDRenderFactory::CreateRenderInstance(RenderType renderType){
    if ( renderType == RenderType::Cairo ){
        return std::make_shared<OFDCairoRender>();
    } else {
        return nullptr;
    }
}

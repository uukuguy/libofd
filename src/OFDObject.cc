#include "OFDObject.h"
#include "OFDTextObject.h"
#include "logger.h"

using namespace ofd;

OFDObject::OFDObject() :
    Visible(true), LineWidth(0.353), Alpha(255){
}

OFDObject::~OFDObject(){
}

OFDObjectPtr OFDObjectFactory::CreateObject(Object::Type objType){
    OFDObjectPtr object = nullptr;

    switch ( objType ){
    case Object::Type::TEXT:{
        OFDTextObject *textObject = new OFDTextObject();
        object = std::shared_ptr<OFDObject>(textObject);
        } break;
    case Object::Type::PATH:
        break;
    case Object::Type::IMAGE:
        break;
    case Object::Type::COMPOSITE:
        break;
    default:
        break;
    };

    return object;
}


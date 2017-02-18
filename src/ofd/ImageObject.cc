#include "ofd/ImageObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;


// **************** class OFDImageObject ****************

ImageObject::ImageObject(LayerPtr layer) :
    Object(layer, ObjectType::IMAGE, "ImageObject"){
    Type = ofd::ObjectType::IMAGE;
}

ImageObject::~ImageObject(){
}

void ImageObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);
}

void ImageObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);
}

bool ImageObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool ImageObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}


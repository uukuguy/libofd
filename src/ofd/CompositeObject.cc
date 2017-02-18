#include "ofd/CompositeObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDCompositeObject ****************

CompositeObject::CompositeObject(LayerPtr layer) :
    Object(layer, ObjectType::COMPOSITE, "CompositeObject"){
}

CompositeObject::~CompositeObject(){
}

void CompositeObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);
}

void CompositeObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);
}

bool CompositeObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool CompositeObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}


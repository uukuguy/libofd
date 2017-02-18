#include "ofd/PathObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class PathObject ****************

PathObject::PathObject(LayerPtr layer) :
    Object(layer, ObjectType::PATH, "PathObject"),
    m_path(nullptr){
}

PathObject::~PathObject(){
}

void PathObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);
}

void PathObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);
}

bool PathObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool PathObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}


#include "OFDImageObject.h"
#include "OFDPage.h"
#include "OFDDocument.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDImageObject::ImplCls ****************

class OFDImageObject::ImplCls{
public:
    ImplCls(OFDImageObject *imageObject);
    ~ImplCls();


    bool FromAttributesXML(XMLElementPtr objectElement);
    bool IterateElementsXML(XMLElementPtr objectElement);

    void GenerateAttributesXML(XMLWriter &writer) const;
    void GenerateElementsXML(XMLWriter &writer) const;

    // -------- Private Attributes --------

private:
    OFDImageObject *m_imageObject;

}; // class OFDImageObject::ImplCls


OFDImageObject::ImplCls::ImplCls(OFDImageObject *imageObject) :
    m_imageObject(imageObject){
}

OFDImageObject::ImplCls::~ImplCls(){
}


void OFDImageObject::ImplCls::GenerateAttributesXML(XMLWriter &writer) const{
}

void OFDImageObject::ImplCls::GenerateElementsXML(XMLWriter &writer) const{
}

bool OFDImageObject::ImplCls::FromAttributesXML(XMLElementPtr objectElement){
    bool ok = true;
    return ok;
}

bool OFDImageObject::ImplCls::IterateElementsXML(XMLElementPtr childElement){
    bool ok = true;
    return ok;
}


// **************** class OFDImageObject ****************

OFDImageObject::OFDImageObject(OFDPagePtr page) :
    OFDObject(page) {
    Type = Object::Type::IMAGE;
    ObjectLabel = "ImageObject";
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this));

}

OFDImageObject::~OFDImageObject(){
}

void OFDImageObject::GenerateAttributesXML(XMLWriter &writer) const{
    OFDObject::GenerateAttributesXML(writer);
    m_impl->GenerateAttributesXML(writer);
}

void OFDImageObject::GenerateElementsXML(XMLWriter &writer) const{
    OFDObject::GenerateElementsXML(writer);
    m_impl->GenerateElementsXML(writer);
}

bool OFDImageObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( OFDObject::FromAttributesXML(objectElement) ){
        return m_impl->FromAttributesXML(objectElement);
    }
    return false;
}

bool OFDImageObject::IterateElementsXML(XMLElementPtr childElement){
    if ( OFDObject::IterateElementsXML(childElement) ){
        return m_impl->IterateElementsXML(childElement);
    }
    return false;
}


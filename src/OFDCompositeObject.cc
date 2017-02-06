#include "OFDCompositeObject.h"
#include "OFDPage.h"
#include "OFDDocument.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDCompositeObject::ImplCls ****************

class OFDCompositeObject::ImplCls{
public:
    ImplCls(OFDCompositeObject *compositeObject);
    ~ImplCls();

    bool FromAttributesXML(XMLElementPtr objectElement);
    bool IterateElementsXML(XMLElementPtr objectElement);

    void GenerateAttributesXML(XMLWriter &writer) const;
    void GenerateElementsXML(XMLWriter &writer) const;

    // -------- Private Attributes --------

private:
    OFDCompositeObject *m_compositeObject;

    const OFDPagePtr GetPage() const {return m_compositeObject->GetPage();};

}; // class OFDCompositeObject::ImplCls


OFDCompositeObject::ImplCls::ImplCls(OFDCompositeObject *compositeObject) :
    m_compositeObject(compositeObject){
}

OFDCompositeObject::ImplCls::~ImplCls(){
}

void OFDCompositeObject::ImplCls::GenerateAttributesXML(XMLWriter &writer) const{
}

void OFDCompositeObject::ImplCls::GenerateElementsXML(XMLWriter &writer) const{
}

bool OFDCompositeObject::ImplCls::FromAttributesXML(XMLElementPtr objectElement){
    bool ok = true;
    return ok;
}

bool OFDCompositeObject::ImplCls::IterateElementsXML(XMLElementPtr childElement){
    bool ok = true;
    return ok;
}

// **************** class OFDCompositeObject ****************

OFDCompositeObject::OFDCompositeObject(OFDPagePtr page) :
    OFDObject(page) {
    Type = Object::Type::COMPOSITE;
    ObjectLabel = "CompositeObject";
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this));

}

OFDCompositeObject::~OFDCompositeObject(){
}

void OFDCompositeObject::GenerateAttributesXML(XMLWriter &writer) const{
    OFDObject::GenerateAttributesXML(writer);
    m_impl->GenerateAttributesXML(writer);
}

void OFDCompositeObject::GenerateElementsXML(XMLWriter &writer) const{
    OFDObject::GenerateElementsXML(writer);
    m_impl->GenerateElementsXML(writer);
}

bool OFDCompositeObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( OFDObject::FromAttributesXML(objectElement) ){
        return m_impl->FromAttributesXML(objectElement);
    }
    return false;
}

bool OFDCompositeObject::IterateElementsXML(XMLElementPtr childElement){
    if ( OFDObject::IterateElementsXML(childElement) ){
        return m_impl->IterateElementsXML(childElement);
    }
    return false;
}


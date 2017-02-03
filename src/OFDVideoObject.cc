#include "OFDVideoObject.h"
#include "OFDPage.h"
#include "OFDDocument.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDVideoObject::ImplCls ****************

class OFDVideoObject::ImplCls{
public:
    ImplCls(OFDVideoObject *pathObject);
    ~ImplCls();


    bool FromAttributesXML(XMLElementPtr objectElement);
    bool IterateElementsXML(XMLElementPtr objectElement);

    void GenerateAttributesXML(XMLWriter &writer) const;
    void GenerateElementsXML(XMLWriter &writer) const;

    // -------- Private Attributes --------

private:
    __attribute__((unused)) OFDVideoObject *m_videoObject;

}; // class OFDVideoObject::ImplCls


OFDVideoObject::ImplCls::ImplCls(OFDVideoObject *videoObject) :
    m_videoObject(videoObject){
}

OFDVideoObject::ImplCls::~ImplCls(){
}


void OFDVideoObject::ImplCls::GenerateAttributesXML(XMLWriter &writer) const{
}

void OFDVideoObject::ImplCls::GenerateElementsXML(XMLWriter &writer) const{
}

bool OFDVideoObject::ImplCls::FromAttributesXML(XMLElementPtr objectElement){
    bool ok = true;
    return ok;
}

bool OFDVideoObject::ImplCls::IterateElementsXML(XMLElementPtr childElement){
    bool ok = true;
    return ok;
}


// **************** class OFDVideoObject ****************

OFDVideoObject::OFDVideoObject(OFDPagePtr page) :
    OFDObject(page) {
    Type = Object::Type::VIDEO;
    ObjectLabel = "VideoObject";
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this));

}

OFDVideoObject::~OFDVideoObject(){
}

void OFDVideoObject::GenerateAttributesXML(XMLWriter &writer) const{
    OFDObject::GenerateAttributesXML(writer);
    m_impl->GenerateAttributesXML(writer);
}

void OFDVideoObject::GenerateElementsXML(XMLWriter &writer) const{
    OFDObject::GenerateElementsXML(writer);
    m_impl->GenerateElementsXML(writer);
}

bool OFDVideoObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( OFDObject::FromAttributesXML(objectElement) ){
        return m_impl->FromAttributesXML(objectElement);
    }
    return false;
}

bool OFDVideoObject::IterateElementsXML(XMLElementPtr childElement){
    if ( OFDObject::IterateElementsXML(childElement) ){
        return m_impl->IterateElementsXML(childElement);
    }
    return false;
}



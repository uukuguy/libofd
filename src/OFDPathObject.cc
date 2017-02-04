#include "OFDPathObject.h"
#include "OFDPage.h"
#include "OFDDocument.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDPathObject::ImplCls ****************

class OFDPathObject::ImplCls{
public:
    ImplCls(OFDPathObject *pathObject);
    ~ImplCls();

    bool FromAttributesXML(XMLElementPtr objectElement);
    bool IterateElementsXML(XMLElementPtr objectElement);

    void GenerateAttributesXML(XMLWriter &writer) const;
    void GenerateElementsXML(XMLWriter &writer) const;

    OfdPathPtr GetPath() const {return m_path;};
    void SetPath(OfdPathPtr path) {m_path = path;};

    // -------- Private Attributes --------

private:
    OFDPathObject *m_pathObject;
    OfdPathPtr m_path;

}; // class OFDPathObject::ImplCls


OFDPathObject::ImplCls::ImplCls(OFDPathObject *pathObject) :
    m_pathObject(pathObject), m_path(nullptr){
}

OFDPathObject::ImplCls::~ImplCls(){
}


void OFDPathObject::ImplCls::GenerateAttributesXML(XMLWriter &writer) const{
}

void OFDPathObject::ImplCls::GenerateElementsXML(XMLWriter &writer) const{
}

bool OFDPathObject::ImplCls::FromAttributesXML(XMLElementPtr objectElement){
    bool ok = true;
    return ok;
}

bool OFDPathObject::ImplCls::IterateElementsXML(XMLElementPtr childElement){
    bool ok = true;
    return ok;
}

// **************** class OFDPathObject ****************

OFDPathObject::OFDPathObject(OFDPagePtr page) :
    OFDObject(page) {
    Type = Object::Type::PATH;
    ObjectLabel = "PathObject";
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this));

}

OFDPathObject::~OFDPathObject(){
}

void OFDPathObject::GenerateAttributesXML(XMLWriter &writer) const{
    OFDObject::GenerateAttributesXML(writer);
    m_impl->GenerateAttributesXML(writer);
}

void OFDPathObject::GenerateElementsXML(XMLWriter &writer) const{
    OFDObject::GenerateElementsXML(writer);
    m_impl->GenerateElementsXML(writer);
}

bool OFDPathObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( OFDObject::FromAttributesXML(objectElement) ){
        return m_impl->FromAttributesXML(objectElement);
    }
    return false;
}

bool OFDPathObject::IterateElementsXML(XMLElementPtr childElement){
    if ( OFDObject::IterateElementsXML(childElement) ){
        return m_impl->IterateElementsXML(childElement);
    }
    return false;
}

OfdPathPtr OFDPathObject::GetPath() const{
    return m_impl->GetPath();
}

void OFDPathObject::SetPath(OfdPathPtr path){
    m_impl->SetPath(path);
}


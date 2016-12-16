#include "OFDObject.h"
#include "OFDTextObject.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

OFDObject::OFDObject() :
    ID(0), Visible(true), LineWidth(0.353), Alpha(255){
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

// Called by OFDPage::ImplCls::generateContentXML()
void OFDObject::GenerateXML(XMLWriter &writer) const{
    writer.StartElement(ObjectLabel);{
        GenerateAttributesXML(writer);
        GenerateElementsXML(writer);
    } writer.EndElement();
}

void OFDObject::GenerateAttributesXML(XMLWriter &writer) const{

    // -------- GraphUnit attributes --------
    // OFD (section 8.5) P50. Page.xsd.

    // -------- <Object ID="">
    // Required
    writer.WriteAttribute("ID", ID);

    // -------- <Object Boundary="">
    // Required.
    writer.WriteAttribute("Boundary", Boundary.to_xmlstring());

    // -------- <Object Name="">
    // Optional
    if ( !Name.empty() ){
        writer.WriteAttribute("Name", Name);
    }

    // -------- <Object Visible="">
    // Optional, default value: true.
    if ( !Visible ){
        writer.WriteAttribute("Visible", "false");
    }

    // TODO
    // -------- <Object CTM="">


    // TODO
    // -------- <Object LineWidth="">
    if ( LineWidth > 0 ){
        writer.WriteAttribute("LineWidth", LineWidth, 3);
    }

    // -------- <Object Alpha="">
    // Optional, default value: 0.
    if ( Alpha > 0 ){
        writer.WriteAttribute("Alpha", std::to_string(Alpha));
    }
}

void OFDObject::GenerateElementsXML(XMLWriter &writer) const{

    // -------- GraphUnit elements --------
    // OFD P50. Page.xsd.

    // TODO
    // -------- <Actions>

    // TODO
    // -------- <Clips>

}


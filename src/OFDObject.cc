#include "OFDObject.h"
#include "OFDTextObject.h"
#include "utils/logger.h"
#include "utils/xml.h"

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

// Called by OFDPage::ImplCls::generateContentXML()
void OFDObject::GenerateXML(XMLWriter &writer) const{
    writer.StartElement(ObjectLabel);{
        GenerateAttributesXML(writer);
        GenerateElementsXML(writer);
    } writer.EndElement();
}

void OFDObject::GenerateAttributesXML(XMLWriter &writer) const{

    // -------- GraphUnit attributes --------
    // OFD P50. Page.xsd.

    // -------- <Object Boundary="">
    // Required.
    const ST_Box &box = Boundary;
    std::string strBoundary = 
        std::to_string(box.Left) + " " +
        std::to_string(box.Top) + " " +
        std::to_string(box.Width) + " " +
        std::to_string(box.Height);

    writer.WriteAttribute("Boundary", strBoundary);

    /*std::string  Name;*/
    /*bool         Visible;*/
    /*double       CTM[6];*/
    /*double       LineWidth;*/
    /*int          Alpha;*/

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
        writer.WriteAttribute("LineWidth", std::to_string(LineWidth));
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


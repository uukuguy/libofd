#include "ofd/Color.h"
#include "ofd/PathObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class PathObject ****************

ColorPtr PathObject::DefaultStrokeColor = Color::Instance(0,0,0,255);
ColorPtr PathObject::DefaultFillColor = Color::Instance(0,0,0,0);

PathObject::PathObject(LayerPtr layer) :
    Object(layer, ObjectType::PATH, "PathObject"),
    Stroke(true), Fill(false), Rule(PathRule::NonZero),
    FillColor(nullptr), StrokeColor(nullptr),
    m_path(nullptr){
}

PathObject::~PathObject(){
}

void PathObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);

    // -------- <PathObject Stroke="">
    // Optional, default value: true.
    if ( !Stroke ){
        writer.WriteAttribute("Stroke", false);
    }

    // -------- <PathObject Fill="">
    // Optional, default value: false.
    if ( Fill ){
        writer.WriteAttribute("Fill", true);
    }

    // -------- <PathObject Rule="Even-Odd">
    // Optional, default value: "NonZero".
    if ( Rule != PathRule::NonZero ){
        writer.WriteAttribute("Rule", "Even-Odd");
    }
}

void PathObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);

    // -------- <FillColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if ( FillColor != nullptr ){
        writer.StartElement("FillColor");{
            FillColor->WriteXML(writer);
        } writer.EndElement();
    }
    
    // -------- <StrokeColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if ( StrokeColor != nullptr ){
        writer.StartElement("StrokeColor");{
            StrokeColor->WriteXML(writer);
        } writer.EndElement();
    }

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


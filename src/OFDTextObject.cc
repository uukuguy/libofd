#include <sstream>
#include "OFDTextObject.h"

using namespace ofd;

OFDTextObject::OFDTextObject() 
    : OFDObject(OFDObjectType::TEXT){
}

OFDTextObject::~OFDTextObject() {
}

std::string OFDTextObject::ToString() const {
    std::string baseString = OFDObject::ToString();

    std::stringstream ss;
    ss << "X: " << X << std::endl;
    ss << "Y: " << Y << std::endl;
    ss << "Text: " << Text << std::endl;
    ss << "Font:" << Font << std::endl;
    ss << "FontSize:" << FontSize << std::endl;

    return baseString + "\n" + ss.str();
}

bool OFDTextObject::ParseFromXML(const XMLElement* xmlElement){
    bool ok = false;

    if ( !OFDObject::ParseFromXML(xmlElement) ) return false;

    // <ofd:FillColor>
    const XMLElement *fillColorElement = xmlElement->FirstChildElement("ofd:FillColor");
    this->FillColor.ColorSpace = fillColorElement->IntAttribute("ColorSpace");
    this->FillColor.Value = fillColorElement->DoubleAttribute("Value");

    // <ofd:StrokeColor>
    const XMLElement *strokeColorElement = xmlElement->FirstChildElement("ofd:StrokeColor");
    this->StrokeColor.ColorSpace = strokeColorElement->IntAttribute("ColorSpace");
    this->StrokeColor.Value = strokeColorElement->DoubleAttribute("Value");

    // Font attribute.
    this->Font = xmlElement->DoubleAttribute("Font");

    // FontSize attribute.
    this->FontSize = xmlElement->DoubleAttribute("Size");


    // <ofd:TextCode>
    const XMLElement *textCodeElement = xmlElement->FirstChildElement("ofd:TextCode");
    this->X = textCodeElement->DoubleAttribute("X");
    this->Y = textCodeElement->DoubleAttribute("Y");
    this->Text = textCodeElement->GetText();

    ok = true;

    return ok;
}

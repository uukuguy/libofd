#include <sstream>
#include <vector>
#include "OFDPathObject.h"
#include "utils/tinyxml2.h"
#include "utils/utils.h"

using namespace utils;
using namespace ofd;

OFDPathObject::OFDPathObject() 
    : OFDObject(OFDObjectType::PATH){
}

OFDPathObject::~OFDPathObject() {
}

std::string OFDPathObject::ToString() const {
    std::string baseString = OFDObject::ToString();

    std::stringstream ss;
    //ss << "X: " << X << std::endl;
    //ss << "Y: " << Y << std::endl;
    //ss << "Text: " << Text << std::endl;

    return baseString + "\n" + ss.str();
}

bool OFDPathObject::ParseFromXML(const XMLElement* xmlElement){
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

    // <ofd::AbbreviatedData>
    const XMLElement *abbreviatedDataElement = xmlElement->FirstChildElement("ofd:AbbreviatedData");
    if ( abbreviatedDataElement != nullptr ) {
        std::string text = abbreviatedDataElement->GetText();
        std::vector<std::string> tokens = SplitString(text);
        if ( tokens.size() == 6 ){
            this->AbbreviatedData.m0 = atof(tokens[1].c_str());
            this->AbbreviatedData.m1 = atof(tokens[2].c_str());
            this->AbbreviatedData.l0 = atof(tokens[4].c_str());
            this->AbbreviatedData.l1 = atof(tokens[5].c_str());
        }
    }

    ok = true;

    return ok;
}

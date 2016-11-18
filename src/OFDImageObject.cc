#include <sstream>
#include <vector>
#include "OFDImageObject.h"
#include "utils.h"

using namespace ofd;

OFDImageObject::OFDImageObject() 
    : OFDObject(OFDObjectType::IMAGE){
}

OFDImageObject::~OFDImageObject() {
}

std::string OFDImageObject::ToString() const {
    std::string baseString = OFDObject::ToString();

    std::stringstream ss;
    //ss << "X: " << X << std::endl;
    //ss << "Y: " << Y << std::endl;
    //ss << "Text: " << Text << std::endl;

    return baseString + "\n" + ss.str();
}

bool OFDImageObject::ParseFromXML(const XMLElement* xmlElement){
    bool ok = false;

    if ( !OFDObject::ParseFromXML(xmlElement) ) return false;

    // <ofd:ResourceID>
    const XMLAttribute *attrResourceID = xmlElement->FindAttribute("ofd:ResourceID");
    if ( attrResourceID != nullptr ){
        this->ResourceID = atoi(attrResourceID->Value());
    }


    ok = true;

    return ok;
}

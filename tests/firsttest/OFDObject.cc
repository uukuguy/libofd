#include <sstream>
#include <vector>
#include "OFDObject.h"
#include "OFDTextObject.h"
#include "OFDPathObject.h"
#include "OFDImageObject.h"
#include "utils/logger.h"
#include "utils/tinyxml2.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;

OFDObject::OFDObject(OFDObjectType objectType) 
    : m_objectType(objectType){
}

OFDObject::~OFDObject() {
}

OFDObject *OFDObject::CreateObject(const std::string &elementName) {
    OFDObject *object = nullptr;

    if ( elementName == "ofd:TextObject" ){
        OFDTextObject *textObject = new OFDTextObject();        
        object = static_cast<OFDObject*>(textObject);
    } else if ( elementName == "ofd:PathObject" ){
        OFDPathObject *pathObject = new OFDPathObject();        
        object = static_cast<OFDObject*>(pathObject);
    } else if ( elementName == "ofd:ImageObject" ){
        OFDImageObject *imageObject = new OFDImageObject();        
        object = static_cast<OFDObject*>(imageObject);
    } else {
        LOG(WARNING) << "Unknown objec type: " << elementName;
    }

    return object;
}

std::string OFDObject::ToString() const {
    std::stringstream ss;
    ss << "CTM: ( " << CTM.xx << ", " << CTM.xy << ", " << CTM.yx << ", " << CTM.yy << ", " << CTM.x0 << ", " << CTM.y0 << ") " << std::endl;
    ss << "Boundary: ( " << Boundary.x0 << ", " << Boundary.y0 << ", " << Boundary.w << ", " << Boundary.h << ") " << std::endl;
    ss << " LineWidth:" << LineWidth << std::endl;
    ss << " MiterLimit:" << MiterLimit << std::endl;
    ss << " Stroke:" << Stroke << std::endl;
    ss << " Fill:" << Fill << std::endl;

    ss << "FillColor: (" << FillColor.ColorSpace << "," << FillColor.Value << ") " << std::endl;

    ss << "StrokeColor: (" << StrokeColor.ColorSpace << "," << StrokeColor.Value << ") " << std::endl;

    return ss.str();
}


bool OFDObject::ParseFromXML(const XMLElement* xmlElement){
    bool ok = false;

    this->ID = xmlElement->IntAttribute("ID");

    // CTM attribute.
    const XMLAttribute *attrCTM = xmlElement->FindAttribute("CTM");
    if ( attrCTM != NULL ){
        std::string c = attrCTM->Value();
        //std::string c = xmlElement->Attribute("CTM"); 
        std::vector<std::string> ctmTokens = utils::SplitString(c);
        if ( ctmTokens.size() == 6 ){
            this->CTM.xx = atof(ctmTokens[0].c_str());
            this->CTM.xy = atof(ctmTokens[1].c_str());
            this->CTM.yx = atof(ctmTokens[2].c_str());
            this->CTM.yy = atof(ctmTokens[3].c_str());
            this->CTM.x0 = atof(ctmTokens[4].c_str());
            this->CTM.y0 = atof(ctmTokens[5].c_str());
        }
    }

    // Boundary attribute.
    const XMLAttribute *attrBoundary = xmlElement->FindAttribute("Boundary");
    if ( attrBoundary != NULL ){
        std::string b = attrBoundary->Value();
        //std::string b = xmlElement->Attribute("Boundary");
        std::vector<std::string> boundaryTokens = utils::SplitString(b);
        if ( boundaryTokens.size() == 4 ){
            this->Boundary.x0 = atof(boundaryTokens[0].c_str());
            this->Boundary.y0 = atof(boundaryTokens[1].c_str());
            this->Boundary.w = atof(boundaryTokens[2].c_str());
            this->Boundary.h = atof(boundaryTokens[3].c_str());
        }
    }

    // LineWidth attribute.
    this->LineWidth = xmlElement->DoubleAttribute("LineWidth");

    // MiterLimit attribute.
    this->MiterLimit = xmlElement->DoubleAttribute("MiterLimit");

    // Stroke attribute.
    this->Stroke = xmlElement->BoolAttribute("Stroke");

    // Fill attribute.
    this->Fill = xmlElement->BoolAttribute("Fill");

    ok = true;

    return ok;
}


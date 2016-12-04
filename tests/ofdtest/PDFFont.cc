#include <sstream>
#include "PDFFont.h"

PDFFont::PDFFont(){
    clear();
}

PDFFont::~PDFFont(){
}

void PDFFont::clear(){
    ID = -1;
    tag = "";
    family = "";
    name = "";
    type = PDFFontType::Unknown;
    embID = -1;
    embeddedName = "";
    encodingName = "";
    flags = 0;
    location = PDFFontLocation::Unknown;
    filePath = "";
    memset(ctm, 0, sizeof(double) * 6);
    memset(bbox, 0, sizeof(double) * 4);
}

std::string getFontTypeString(PDFFontType type){
    if ( type == PDFFontType::TrueType ){
        return "Font TrueType";
    } else if ( type == PDFFontType::CIDType2 ){
        return "Font CIDType2";
    } else if ( type == PDFFontType::Type1 ){
        return "Font Type1";
    } else if ( type == PDFFontType::Type3 ){
        return "Font Type3";
    } else {
        return "Font Type Unknown";
    }
}

std::string getFontLocationString(PDFFontLocation location){
    if ( location == PDFFontLocation::Embedded ){
        return "Location Embedded";
    } else if ( location == PDFFontLocation::External ){
        return "Location External";
    } else if ( location == PDFFontLocation::Resident ){
        return "Location Resident";
    } else {
        return "Location Unknown";
    }
}

std::string PDFFont::ToString() const {
    std::stringstream ss;

    ss << "\n--------------------------------\n";
    ss << " Font" << "\n";
    ss << " ID: " << ID << "\n";
    ss << " tag: " << tag << "\n";
    ss << " family: " << family << "\n";
    ss << " name: " << name << "\n";
    ss << " type: " << getFontTypeString(type) << "(" << int(type) << ")" << "\n";
    ss << " embID: " << embID << "\n";
    ss << " embeddedName: " << embeddedName << "\n";
    ss << " encodingName: " << encodingName << "\n";
    ss << " flags: " << flags << "\n";
    ss << " location: " << getFontLocationString(location) << "(" << int(location) << ")" << "\n";
    ss << " filePath: " << filePath << "\n";
    ss << " ctm: " << "(" << ctm[0] << ", " << ctm[1] << ", " << ctm[2] << ", " << ctm[3] << ", " << ctm[4] << ", " << ctm[5] << ")" << "\n";
    ss << " bbox: " << "(" << bbox[0] << ", " << bbox[1] << ", " << bbox[2] << ", " << bbox[3] << ")" << "\n";

    return ss.str();
}


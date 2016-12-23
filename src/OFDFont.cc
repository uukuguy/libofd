#include <sstream>
#include "OFDFont.h"

using namespace ofd;

OFDFont::OFDFont() : 
    Charset("unicode"), Serif(false), Bold(false), Italic(false), FixedWidth(false),
    FontType(Font::Type::Unknown), FontLoc(Font::Location::Unknown),
    FontStream(nullptr), FontStreamSize(0){
}

OFDFont::~OFDFont(){
    if ( FontStream != nullptr ){
        delete FontStream;
        FontStream = nullptr;
    }
}

std::string OFDFont::ToString() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDFont" << std::endl;

        ss << "ID: " << ID << std::endl;
        ss << "FontName: " << FontName << std::endl;
        ss << "FamilyName: " << FamilyName << std::endl;
        ss << "Charset: " << Charset << std::endl;
        ss << "FontFile: " << FontFile << std::endl;

        if ( FontType == Font::Type::CIDType2 ){
            ss << "FontType: CIDType2" << std::endl;
        } else if ( FontType == Font::Type::Type1 ){
            ss << "FontType: Type1" << std::endl;
        } else if ( FontType == Font::Type::Type3 ){
            ss << "FontType: Type3" << std::endl;
        } else if ( FontType == Font::Type::TrueType ){
            ss << "FontType: TrueType" << std::endl;
        } else {
            ss << "FontType: Unknown" << std::endl;
        }

        if ( FontLoc == Font::Location::Embedded ){
            ss << "FontLoc: Embedded" << std::endl;
        } else if ( FontLoc == Font::Location::External ){
            ss << "FontLoc: External" << std::endl;
        } else if ( FontLoc == Font::Location::Resident ){
            ss << "FontLoc: Resident" << std::endl;
        } else {
            ss << "FontLoc: Unknown" << std::endl;
        }

        ss << "FontStreamSize: " << FontStreamSize << std::endl;
        ss << std::endl
           << "------------------------------" << std::endl;

    return ss.str();
}


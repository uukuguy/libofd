#include <sstream>
#include "OFDFont.h"

using namespace ofd;

OFDFont::OFDFont(){
}

OFDFont::~OFDFont(){
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

        ss << std::endl
           << "------------------------------" << std::endl;

    return ss.str();
}


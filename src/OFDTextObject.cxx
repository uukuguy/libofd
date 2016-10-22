#include <sstream>
#include "OFDTextObject.h"

using namespace ofd;

OFDTextObject::OFDTextObject() {
}

OFDTextObject::~OFDTextObject() {
}

std::string OFDTextObject::ToString() const {
    std::string baseString = OFDObject::ToString();

    std::stringstream ss;
    ss << "X: " << X << std::endl;
    ss << "Y: " << Y << std::endl;
    ss << "Text: " << Text << std::endl;

    return baseString + "\n" + ss.str();
}


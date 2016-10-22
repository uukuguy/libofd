#include <sstream>
#include "OFDObject.h"

using namespace ofd;

OFDObject::OFDObject() {
}

OFDObject::~OFDObject() {
}

std::string OFDObject::ToString() const {
    std::stringstream ss;
    ss << "CTM: ( " << CTM.a << ", " << CTM.b << ", " << CTM.c << ", " << CTM.d << ", " << CTM.p << ", " << CTM.q << ") " << std::endl;
    ss << "Boundary: ( " << Boundary.x0 << ", " << Boundary.y0 << ", " << Boundary.w << ", " << Boundary.h << ") " << std::endl;
    ss << " LineWidth:" << LineWidth << std::endl;
    ss << " MiterLimit:" << MiterLimit << std::endl;
    ss << " Font:" << Font << std::endl;
    ss << " FontSize:" << FontSize << std::endl;
    ss << " Stroke:" << Stroke << std::endl;
    ss << " Fill:" << Fill << std::endl;

    ss << "FillColor: (" << FillColor.ColorSpace << "," << FillColor.Value << ") " << std::endl;

    ss << "StrokeColor: (" << StrokeColor.ColorSpace << "," << StrokeColor.Value << ") " << std::endl;

    return ss.str();
}



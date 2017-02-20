#include <assert.h>
#include "ofd/Color.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

// **************** class ColorSpace ****************

ColorSpacePtr ColorSpace::DefaultInstance = std::make_shared<ColorSpace>();

ColorSpace::ColorSpace() : 
    Type(ColorSpaceType::RGB), BPC(8),
    m_refID(0){
}

// **************** class Color ****************

Color::Color(uint32_t gray, uint32_t alpha) :
    Value(ColorValue(gray)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(uint32_t r, uint32_t g, uint32_t b, uint32_t alpha) :
    Value(ColorValue(r, g, b)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(uint32_t c, uint32_t m, uint32_t y, uint32_t k, uint32_t alpha) :
    Value(ColorValue(c, m, y, k)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(const ColorValue &colorValue, uint32_t alpha) :
    Value(colorValue),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(ColorSpacePtr colorSpace, uint32_t index, uint32_t alpha) :
    weakColorSpace(colorSpace), 
    Index(index), Alpha(alpha),
    m_bUsePalette(true){
}

// ================ Color::Equal() ================
bool Color::Equal(ColorPtr color) const {
    if ( Alpha != color->Alpha ) return false;
    ColorSpacePtr cs1 = GetColorSpace();
    ColorSpacePtr cs2 = color->GetColorSpace();
    if ( cs1->GetRefID() != cs2->GetRefID() ) return false;
    if ( m_bUsePalette == color->m_bUsePalette ){
        if ( m_bUsePalette ){
            return Index == color->Index;
        } else {
            return Value == color->Value;
        }
    } else {
        return false;
    }
};

// ================ Color::GetColorSpace() ================
ColorSpacePtr Color::GetColorSpace() const {
    if ( !weakColorSpace.expired() ){
        return weakColorSpace.lock();
    } else {
        return ColorSpace::DefaultInstance;
    }
}

// ================ Color::WriteXML() ================
void Color::WriteXML(XMLWriter &writer) const{

    if ( !m_bUsePalette ){
        // -------- <Color Value="">
        // Optional

        ColorSpacePtr colorSpace = GetColorSpace();
        assert(colorSpace != nullptr);

        size_t numChannels = 0;
        if ( colorSpace->Type == ColorSpaceType::RGB ){
            numChannels = 3;
        } else if ( colorSpace->Type == ColorSpaceType::GRAY ){
            numChannels = 1;
        } else if ( colorSpace->Type == ColorSpaceType::CMYK ){
            numChannels = 4;
        } else {
            throw "Unknown ColorSpace Type : " + std::to_string((int)colorSpace->Type);
        }

        std::stringstream ss;
        for ( size_t k = 0 ; k < numChannels ; k++ ){
            ss << Value.Values[k] << " ";
        }

        writer.WriteAttribute("Value", ss.str());

    } else {
        // -------- <Color Index="">
        // Optional
        writer.WriteAttribute("Index", (uint64_t)Index);
    }
    // -------- <Color ColorSpace="">
    // Optional
    if ( !weakColorSpace.expired() ){
        ColorSpacePtr colorSpace = weakColorSpace.lock();
        uint64_t refID = colorSpace->GetRefID();
        if ( refID > 0 ){
            writer.WriteAttribute("ColorSpace", refID);
        }
    }

    // -------- <Color Alpha="">
    // Optional
    writer.WriteAttribute("Alpha", (uint64_t)Alpha);
}

// ================ Color::FromXML() ================
bool Color::FromXML(const std::string &strXML){
    bool ok = false;

    return ok;
}

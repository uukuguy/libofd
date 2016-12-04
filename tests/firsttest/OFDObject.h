#ifndef __OFDOBJECT_H__
#define __OFDOBJECT_H__

#include <stdint.h>
#include <string>
#include <tinyxml2.h>
using namespace tinyxml2;
#include "ofd.h"


            //<ofd:TextObject ID="121" CTM="0.3527 0 0 0.3527 -114.807533 111.352325" 
            //                Boundary="114.807533 185.229584 4.083549 4.733795" 
            //                LineWidth="1" MiterLimit="3.527" Font="16" Size="14.749" 
            //                Stroke="false" Fill="true">
                //<ofd:FillColor ColorSpace="15" Value="0"/>
                //<ofd:StrokeColor ColorSpace="15" Value="0"/>
                //<ofd:CGTransform CodePosition="0" CodeCount="1" GlyphCount="1">
                    //<ofd:Glyphs>4460</ofd:Glyphs> 
                //</ofd:CGTransform>
                //<ofd:TextCode X="324.419" Y="-303.723">局</ofd:TextCode>
            //</ofd:TextObject>
namespace ofd {

enum class OFDObjectType {
    UNKNOWN = -1,
    TEXT,
    PATH,
    IMAGE
}; // enum class OFDObjectType

class OFDObject {
public:
    OFDObject(OFDObjectType objectType);
    virtual ~OFDObject();

    OFDObjectType GetObjectType() const {return m_objectType;};

    static OFDObject *CreateObject(const std::string &elementName);

    virtual std::string ToString() const;
    virtual bool ParseFromXML(const XMLElement* xmlElement);

public:
    OFDObjectType m_objectType;
    uint64_t ID;
    OFDCTM CTM;
    OFDBoundary Boundary;

    double LineWidth;
    double MiterLimit;
    bool Stroke;
    bool Fill;

    OFDColor StrokeColor;
    OFDColor FillColor;

}; // class OFDObject

} // namespace ofd

#endif // __OFDOBJECT_H__

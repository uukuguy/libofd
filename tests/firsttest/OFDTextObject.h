#ifndef __OFDTEXTOBJECT_H__
#define __OFDTEXTOBJECT_H__

#include "OFDObject.h"

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

class OFDTextObject : public OFDObject {
public:
    OFDTextObject();
    virtual ~OFDTextObject();

    virtual std::string ToString() const override;
    virtual bool ParseFromXML(const XMLElement* xmlElement) override;

public:
    double X;
    double Y;
    std::string Text;
    int Font;
    double FontSize;


}; // class OFDTextObject

} // namespace ofd

#endif // __OFDTEXTOBJECT_H__

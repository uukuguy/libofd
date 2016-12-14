#ifndef __OFDPATHOBJECT_H__
#define __OFDPATHOBJECT_H__

#include "OFDObject.h"

/***
 * <ofd:PathObject  ID="29" 
 *                  Boundary="27.291925 112.82695 155.50647 0.538925" 
 *                  LineWidth="0.269463" 
 *                  MiterLimit="3.527" 
 *                  Stroke="true">
 *                  <ofd:FillColor ColorSpace="15" Value="0"/>
    <ofd:StrokeColor ColorSpace="4" Value="0 255 255 0"/>
    <ofd:AbbreviatedData>M 0.269461 0.26947 L 155.237 0.26947</ofd:AbbreviatedData>
</ofd:PathObject>
***/

namespace ofd {

class OFDPathObject : public OFDObject {
public:
    OFDPathObject();
    virtual ~OFDPathObject();

    virtual std::string ToString() const override;
    virtual bool ParseFromXML(const XMLElement* xmlElement) override;

public:
    struct OFDAbbreviatedData {
        double m0;
        double m1;
        double l0;
        double l1;
    };
    
    OFDAbbreviatedData AbbreviatedData;


}; // class OFDTextObject

} // namespace ofd

#endif // __OFDPATHOBJECT_H__

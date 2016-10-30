#ifndef __OFDIMAGEOBJECT_H__
#define __OFDIMAGEOBJECT_H__

#include "OFDObject.h"

/***
 * <ofd:ImageObject 
 *          ID="330" 
 *          CTM="49.898937 0 0 49.933151 0 0" 
 *          Boundary="113.703773 47.283279 49.898933 49.933159"
 *          ResourceID="331"/>
***/

namespace ofd {

class OFDImageObject : public OFDObject {
public:
    OFDImageObject();
    virtual ~OFDImageObject();

    virtual std::string ToString() const override;
    virtual bool ParseFromXML(const XMLElement* xmlElement) override;

public:
    uint64_t ResourceID;
    


}; // class OFDIMAGEObject

} // namespace ofd

#endif // __OFDIMAGEOBJECT_H__

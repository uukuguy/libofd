#ifndef __OFDIMAGE_H__
#define __OFDIMAGE_H__

#include <string>

namespace ofd {

class OFDImage {
public:
    OFDImage();
    virtual ~OFDImage();

public:
    std::string ToString() const;

}; // class OFDImage

}; // namespace ofd

#endif // __OFDIMAGE_H__

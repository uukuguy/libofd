#ifndef __OFDFONT_H__
#define __OFDFONT_H__

#include <string>
#include <memory>

namespace ofd {

class OFDFont {
public:
    OFDFont();
    virtual ~OFDFont();

public:
    int ID;
    std::string FontName;
    std::string FamilyName;
    std::string Charset;
    std::string FontFile;

    std::string ToString() const;

}; // class OFDFont

}; // namespace ofd

#endif // __OFDFONT_H__

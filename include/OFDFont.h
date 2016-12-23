#ifndef __OFDFONT_H__
#define __OFDFONT_H__

#include <string>
#include <vector>
#include <memory>

namespace ofd {

    namespace Font{

        enum class Type {
            Unknown = -1,
            TrueType,
            CIDType2,
            Type1,
            Type3,
        };

        enum class Location{
            Unknown = -1,
            Embedded,
            External,
            Resident,
        };
    };

    // ======== class OFDFont ========
    // OFD (section 11.1) P61. Res.xsd.
    typedef struct OFDFont {
    public:
        OFDFont();
        ~OFDFont();

    public:
        int ID;
        std::string FontName;
        std::string FamilyName;
        std::string Charset;
        bool        Serif;
        bool        Bold;
        bool        Italic;
        bool        FixedWidth;
        std::string FontFile;

        Font::Type FontType;
        Font::Location FontLoc;

        char *FontStream;
        size_t FontStreamSize;


        std::string ToString() const;

    } OFDFont_t; // class OFDFont
    typedef std::shared_ptr<OFDFont> OFDFontPtr;

    typedef std::vector<OFDFont> FontArray;

}; // namespace ofd

#endif // __OFDFONT_H__

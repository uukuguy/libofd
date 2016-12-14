#ifndef __OFDFONT_H__
#define __OFDFONT_H__

#include <string>
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

        Font::Type FontType;
        Font::Location FontLoc;

        char *FontStream;
        size_t FontStreamSize;


        std::string ToString() const;

    }; // class OFDFont
    typedef std::shared_ptr<OFDFont> OFDFontPtr;

}; // namespace ofd

#endif // __OFDFONT_H__

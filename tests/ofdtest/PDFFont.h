#ifndef __PDFFONT_H__
#define __PDFFONT_H__

#include <string>
#include <GfxFont.h>

enum class PDFFontType {
    Unknown = -1,
    TrueType,
    CIDType2,
    Type1,
    Type3,
};

std::string getFontTypeString(PDFFontType type);

enum class PDFFontLocation {
    Unknown = -1,
    Embedded,
    External,
    Resident,
};

std::string getFontLocationString(PDFFontLocation location);

class PDFFont {
public:
    long long   ID;
    std::string tag;
    std::string family;
    std::string name;
    PDFFontType type;
    long long   embID;
    std::string embeddedName;
    std::string encodingName;
    int         flags;
    PDFFontLocation location;
    std::string filePath;
    double      ctm[6];
    double      bbox[4];

public:
    PDFFont();
    ~PDFFont();

    void clear();
    std::string ToString() const;

    bool IsFixedWidth() const {return flags & (1 << 0);};
    bool IsSerif() const {return flags & (1 << 1);};
    bool IsSymbolic() const {return flags & (1 << 2);};
    bool IsItalic() const {return flags & (1 << 6);};
    bool IsBold() const {return flags & (1 << 18);};

    bool FromPopplerFont(GfxFont &font);

}; // class PDFFont

#endif // __PDFFONT_H__

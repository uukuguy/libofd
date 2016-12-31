#ifndef __OFDFONT_H__
#define __OFDFONT_H__

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "OFDCommon.h"

struct _cairo_font_face;

namespace utils{
    class XMLWriter;
    class XMLElement;
    typedef std::shared_ptr<XMLElement> XMLElementPtr;
}; // namespace utils

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

    struct OFDFont;
    typedef std::shared_ptr<OFDFont> OFDFontPtr;

    // ======== class OFDFont ========
    // OFD (section 11.1) P61. Res.xsd.
    typedef struct OFDFont {
    public:
        OFDFont();
        ~OFDFont();

    public:
        uint64_t          ID;
        std::string       FontName;
        std::string       FamilyName;
        std::string       Charset;
        bool              Serif;
        bool              Bold;
        bool              Italic;
        bool              FixedWidth;
        std::string       FontFile;
        Font::Type        FontType;
        Font::Location    FontLoc;

        char*             m_fontData;
        size_t            m_fontDataSize;
        _cairo_font_face* m_fontFace;
        int*              m_codeToGID;
        size_t            m_codeToGIDLen;
        bool              m_substitute;
        bool              m_printing;

        std::string ToString() const;
        void GenerateXML(utils::XMLWriter &writer) const;
        bool FromXML(utils::XMLElementPtr fontElement);
        std::string GetFileName() const;

        bool Load(OFDPackagePtr package, bool reload = false);
        //unsigned long GetGlyph(CharCode code, Unicode *u, int uLen) const;

        void SetFontFilePath(const std::string &fontFilePath){m_fontFilePath = fontFilePath;};
        std::string GetFontFilePath() const {return m_fontFilePath;};
        
    private:
        bool              m_bLoaded;
        std::string       m_fontFilePath;

    } OFDFont_t; // class OFDFont

    typedef std::vector<OFDFontPtr> FontArray;
    typedef std::map<uint64_t, OFDFontPtr> FontMap;


    class OFDFontEngine{
    public:
        OFDFontEngine(OFDDocumentPtr document);
        ~OFDFontEngine();

        static const size_t MaxCachedFonts = 64;

        OFDFontPtr GetFont(uint64_t fontID); 

    private:
       class ImplCls;
       std::unique_ptr<ImplCls> m_impl;

    }; // class OFDFontEngine

}; // namespace ofd

#endif // __OFDFONT_H__

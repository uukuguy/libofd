#include <sstream>
#include <list>
#include <assert.h>
#include <tuple>
#include <inttypes.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "OFDFont.h"
#include "OFDDocument.h"
#include "OFDPackage.h"
#include "utils/xml.h"
#include "utils/logger.h"
#include "utils/utils.h"

using namespace ofd;
using namespace utils;

class FreetypeInitiator{
public:
    FreetypeInitiator(){
        InitializeFreetype();
    }

    ~FreetypeInitiator(){
        FinalizeFreetype();
    }

    bool InitializeFreetype();
    void FinalizeFreetype();

public:
    static FT_Library ft_lib;
private:
    static bool ft_lib_initialized;
};

bool FreetypeInitiator::InitializeFreetype(){
    if ( ft_lib_initialized ) return true;

    FT_Error error = FT_Init_FreeType(&ft_lib);
    if ( error ){
        LOG(ERROR) << "FT_Init_FreeType() in OFDRes::InitializeFreetype() failed.";
    } else {
        ft_lib_initialized = true;
    }

    return ft_lib_initialized;
}

void FreetypeInitiator::FinalizeFreetype(){
    if ( ft_lib_initialized ){
        FT_Done_FreeType(ft_lib);
        ft_lib = nullptr;
        ft_lib_initialized = false;
    }
}

FT_Library FreetypeInitiator::ft_lib = nullptr;
bool FreetypeInitiator::ft_lib_initialized = false;
FreetypeInitiator ftInitiator;

OFDFont::OFDFont() : 
    Charset("unicode"), Serif(false), Bold(false), Italic(false), FixedWidth(false),
    FontType(Font::Type::Unknown), FontLoc(Font::Location::Unknown),
    m_fontData(nullptr), m_fontDataSize(0),
    m_fontFace(nullptr), m_codeToGID(nullptr), m_codeToGIDLen(0),
    m_substitute(false), m_printing(false),
    m_bLoaded(false){
}

OFDFont::~OFDFont(){
    if ( m_fontFace != nullptr ){
        cairo_font_face_destroy(m_fontFace);
        m_fontFace = nullptr;
    }
    if ( m_codeToGID != nullptr ){
        delete m_codeToGID;
        m_codeToGID = nullptr;
    }
    if ( m_fontData != nullptr ){
        delete m_fontData;
        m_fontData = nullptr;
    }
}

std::string OFDFont::ToString() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDFont" << std::endl;

        ss << "ID: " << ID << std::endl;
        ss << "FontName: " << FontName << std::endl;
        ss << "FamilyName: " << FamilyName << std::endl;
        ss << "Charset: " << Charset << std::endl;
        ss << "FontFile: " << FontFile << std::endl;

        if ( FontType == Font::Type::CIDType2 ){
            ss << "FontType: CIDType2" << std::endl;
        } else if ( FontType == Font::Type::Type1 ){
            ss << "FontType: Type1" << std::endl;
        } else if ( FontType == Font::Type::Type3 ){
            ss << "FontType: Type3" << std::endl;
        } else if ( FontType == Font::Type::TrueType ){
            ss << "FontType: TrueType" << std::endl;
        } else {
            ss << "FontType: Unknown" << std::endl;
        }

        if ( FontLoc == Font::Location::Embedded ){
            ss << "FontLoc: Embedded" << std::endl;
        } else if ( FontLoc == Font::Location::External ){
            ss << "FontLoc: External" << std::endl;
        } else if ( FontLoc == Font::Location::Resident ){
            ss << "FontLoc: Resident" << std::endl;
        } else {
            ss << "FontLoc: Unknown" << std::endl;
        }

        ss << "FontData Size: " << m_fontDataSize << std::endl;
        ss << std::endl
           << "------------------------------" << std::endl;

    return ss.str();
}

std::string OFDFont::GetFileName() const{
    char buf[1024];
    sprintf(buf, "Font_%" PRIu64 ".ttf", ID);
    //LOG(ERROR) << "------- GetFileName() ID:" << ID << " filename=" << std::string(buf);
    return std::string(buf);
    //return std::string("Font_") + std::to_string(ID) + ".ttf";
}

void OFDFont::GenerateXML(XMLWriter &writer) const{

    writer.StartElement("Font");{
        // -------- <Font ID="">
        writer.WriteAttribute("ID", ID);

        // -------- <Font FontName="">
        // Required.
        writer.WriteAttribute("FontName", FontName);

        // -------- <Font FamilyName="">
        // Optional.
        if ( !FamilyName.empty() ){
            writer.WriteAttribute("FamilyName", FamilyName);
        }

        // -------- <Font Charset="">
        // Optional.
        if ( !Charset.empty() ){
            writer.WriteAttribute("Charset", Charset);
        }

        // -------- <Font Serif="">
        // Optional.
        if ( Serif ){
            writer.WriteAttribute("Serif", true);
        }

        // -------- <Font Bold="">
        // Optional.
        if ( Bold ){
            writer.WriteAttribute("Bold", true);
        }

        // -------- <Font Italic="">
        // Optional.
        if ( Italic ){
            writer.WriteAttribute("Italic", true);
        }

        // -------- <Font FixedWidth="">
        // Optional.
        if ( FixedWidth ){
            writer.WriteAttribute("FixedWidth", true);
        }

        // -------- <FontFile>
        // Optional
        writer.WriteElement("FontFile", FontFile);

    } writer.EndElement();

}

bool OFDFont::FromXML(XMLElementPtr fontElement){

    bool ok = false;
    OFDFontPtr font = nullptr;

    std::string childName = fontElement->GetName();

    // -------- <Font>
    // OFD (section 11.1) P61. Res.xsd.
    if ( childName == "Font" ){
        bool exist = false;

        // -------- <Font FontName="">
        // Required.
        std::tie(ID, exist) = fontElement->GetIntAttribute("ID");
        if ( exist ){
            // -------- <Font FontName="">
            // Required.
            std::tie(FontName, exist) = fontElement->GetStringAttribute("FontName");
            if ( !exist ){
                LOG(ERROR) << "Attribute FontName is required in Font XML.";
            }
        } else {
            LOG(ERROR) << "Attribute ID is required in Font XML.";
        }

        if ( exist ) {

            // -------- <Font FamilyName="">
            // Optional
            std::tie(FamilyName, std::ignore) = fontElement->GetStringAttribute("FamilyName");

            // -------- <Font Charset="">
            // Optional
            std::tie(Charset, std::ignore) = fontElement->GetStringAttribute("Charset");

            // -------- <Font Charset="">
            // Optional
            std::tie(Charset, std::ignore) = fontElement->GetStringAttribute("Charset");

            // -------- <Font Serif="">
            // Optional
            std::tie(Serif, std::ignore) = fontElement->GetBooleanAttribute("Serif");

            // -------- <Font Bold="">
            // Optional
            std::tie(Bold, std::ignore) = fontElement->GetBooleanAttribute("Bold");

            // -------- <Font Italic="">
            // Optional
            std::tie(Italic, std::ignore) = fontElement->GetBooleanAttribute("Italic");

            // -------- <Font FixedWidth="">
            // Optional
            std::tie(FixedWidth, std::ignore) = fontElement->GetBooleanAttribute("FixedWidth");

            XMLElementPtr fontFileElement = fontElement->GetFirstChildElement();
            if ( fontFileElement != nullptr && fontFileElement->GetName() == "FontFile" ){
                std::tie(FontFile, std::ignore) = fontFileElement->GetStringValue();
            }

            ok = true;
        } else {
            ok = false;
        }
    }

    return ok;
}

static cairo_user_data_key_t _ft_cairo_key;
static void _ft_done_face_uncached (void *closure)
{
    FT_Face face = (FT_Face) closure;
    FT_Done_Face (face);
}

std::tuple<FT_Face, cairo_font_face_t*, bool> createCairoFontFace(char *fontData, size_t fontDataLen){
    bool ok = false;

    FT_Face face;
    cairo_font_face_t *font_face;

    if ( FT_New_Memory_Face(FreetypeInitiator::ft_lib, (unsigned char *)fontData, fontDataLen, 0, &face) != 0 ){
        LOG(ERROR) << "FT_New_Memory_Face() in OFDFont::createCairoFontFace() failed.";
    } else {

        font_face = cairo_ft_font_face_create_for_ft_face (face, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
        if ( cairo_font_face_set_user_data (font_face, &_ft_cairo_key, face, _ft_done_face_uncached) != 0 ) {
            LOG(ERROR) << "cairo_font_face_set_user_data() in OFDFont::createCairoFontFace() failed.";
            _ft_done_face_uncached(face);
            cairo_font_face_destroy(font_face);
            font_face = nullptr;
        } else {
            ok = true;
        }
    }

    return std::make_tuple(face, font_face, ok);
}

bool OFDFont::CreateFromData(char *fontData, size_t fontDataSize){
    bool ok = true;

    LOG(INFO) << "@@@@@@@@ ID: " << ID << " FontName: " << FontName << " fontDataSize: " << fontDataSize;

    if ( m_fontData != nullptr ){
        delete m_fontData;
        m_fontData = nullptr;
    }
    m_fontDataSize = fontDataSize;
    m_fontData = fontData;
    //m_fontData = new char[fontDataSize];
    //memcpy(m_fontData, fontData, fontDataSize);

    FT_Face face;
    cairo_font_face_t *font_face;
    std::tie(face, font_face, ok) = createCairoFontFace(m_fontData, m_fontDataSize); 

    m_fontFace = font_face;
    m_bLoaded = true;

    return ok;
}

bool OFDFont::Load(OFDPackagePtr package, bool reload){
    if ( m_bLoaded && !reload ) return true;

    // FIXME
    //if ( ID > 0 ){

        //std::string fontFile = std::string("./data/embed/f") + std::to_string(ID) + ".otf";
        ////std::string fontFile = "./data/embed/f2.otf";
        //LOG(ERROR) << "Load fontFile: " << fontFile;

        //char *fontData = nullptr;
        //size_t fontDataSize = 0;
        //bool readOK = false;
        //std::tie(fontData, fontDataSize, readOK) = utils::ReadFileData(fontFile);

        //m_bLoaded = CreateFromData(fontData, fontDataSize);

    //} else {
    // --------------
        bool ok = true;

        std::string fontFilePath = m_fontFilePath;
        LOG(DEBUG) << "Load Font: " << fontFilePath;

        char *fontData = nullptr;
        size_t fontDataSize = 0;
        bool readOK = false;
        std::tie(fontData, fontDataSize, readOK) = package->ReadZipFileRaw(fontFilePath);
        if ( readOK ){
            if ( CreateFromData(fontData, fontDataSize) ){
                LOG(INFO) << "Font " << FontName << "(ID=" << ID << ") loaded.";
            } else {
                LOG(ERROR) << "createCairoFontFace() in OFDFont::Load() failed.";
                ok = false;
            }
        } else {
            ok = false;
            LOG(ERROR) << "Call ReadZipFileRaw() to read font file " << fontFilePath << " failed.";
        }

        m_bLoaded = ok;
    //}

    return m_bLoaded;
}

unsigned long OFDFont::GetGlyph(unsigned int code, unsigned int *u, int uLen) const{
    FT_UInt gid;

    if ( m_codeToGID && code < m_codeToGIDLen) {
        gid = (FT_UInt)m_codeToGID[code];
    } else {
        gid = (FT_UInt)code;
    }
    return gid;
}

// **************** class OFDFontEngine::ImplCls ****************

class OFDFontEngine::ImplCls{
public:
    ImplCls(OFDFontEngine *fontEngine, OFDDocumentPtr document);
    ~ImplCls();

    OFDFontPtr GetFont(uint64_t fontID);

    // -------- Private Attributes --------
public:
    std::list<OFDFontPtr> m_cachedFonts;
    OFDFontEngine *m_fontEngine;

private:
    std::weak_ptr<OFDDocument> m_document;

}; // class OFDFontEngine::ImplCls


OFDFontEngine::ImplCls::ImplCls(OFDFontEngine *fontEngine, OFDDocumentPtr document) :
    m_fontEngine(fontEngine), m_document(document) {
    assert(fontEngine != nullptr);
    assert(document != nullptr);
}

OFDFontEngine::ImplCls::~ImplCls(){
}

OFDFontPtr OFDFontEngine::ImplCls::GetFont(uint64_t fontID){
    OFDFontPtr font = nullptr;

    for ( auto iter = m_cachedFonts.begin(); iter != m_cachedFonts.end() ; iter++ ){
        font = *iter;
        if ( font != nullptr && font->ID == fontID ){
            m_cachedFonts.erase(iter);
            break;
        }
    }
    if ( font != nullptr ){
        m_cachedFonts.push_front(font);
    } else {
        const FontMap &fonts = m_document.lock()->GetFonts();
        auto it = fonts.find(fontID);
        if ( it != fonts.end() ){
            font = it->second;

            // LoadFont
            OFDPackagePtr package = m_document.lock()->GetOFDPackage();
            assert(package != nullptr);
            if ( !font->Load(package) ){
                LOG(ERROR) << "font->Load(package) in OFDFontEngine::GetFont() failed. fontID=" << fontID;
                font = nullptr;
            } else {
                if ( m_cachedFonts.size() >= OFDFontEngine::MaxCachedFonts ){
                    m_cachedFonts.pop_back();
                }
                m_cachedFonts.push_front(font);
            }
        } else {
            LOG(WARNING) << "The font (ID=" << fontID << ") is not found in document fonts.";
        }
    }

    return font;
}

// **************** class OFDFontEngine ****************

OFDFontEngine::OFDFontEngine(OFDDocumentPtr ofdDocument) :
    m_impl(utils::make_unique<OFDFontEngine::ImplCls>(this, ofdDocument)){ 
    

}

OFDFontEngine::~OFDFontEngine(){
}

OFDFontPtr OFDFontEngine::GetFont(uint64_t fontID){
    return m_impl->GetFont(fontID);
}

OFDFontEnginePtr OFDFontEngine::GetSelf(){
    return shared_from_this();
}


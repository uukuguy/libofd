#include <sstream>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include "OFDFont.h"
#include "FontResource.h"
#include "utils/logger.h"

FT_Face freetype_create_memory_face(FT_Library library, int faceIndex, const char *faceBuf, size_t bufSize){
    FT_Face face;
    FT_Error error = FT_New_Memory_Face(library, (const FT_Byte*)faceBuf, bufSize, faceIndex, &face);
    if ( error == FT_Err_Unknown_File_Format ){
        return nullptr;
    } else if ( error ){
        return nullptr;
    }

    if ( face != nullptr ){
        std::stringstream ss;
        ss << std::endl 
            << "------------------------------" << std::endl
            << "FontFace" << std::endl;
        ss << "num_glyphs: " << face->num_glyphs << std::endl;
        ss << "face_flags: " << face->face_flags << std::endl;
        ss << "units_per_EM: " << face->units_per_EM << std::endl;
        ss << "num_fixed_sizes: " << face->num_fixed_sizes << std::endl;
        ss << "available_sizes: " << face->available_sizes << std::endl;
        ss << std::endl
           << "------------------------------" << std::endl;

        LOG(DEBUG) << ss.str();
    }
    return face;
}

using namespace ofd;

// ********************************************************
// ********         class CairoFontResource        ********
// ********************************************************

class CairoFontResource : public FontResource {
public:
    CairoFontResource();
    virtual ~CairoFontResource();

    virtual bool AddFontFace(int fontID, int faceIndex, const char *fontFaceBuf, size_t bufSize) override;
    virtual void* GetFontFace(int fontID) const override;

private:
    FT_Library m_library;
    std::map<int, cairo_font_face_t*> m_faces;

    bool init();
    void clear();
}; // class CairoFontResource

CairoFontResource::CairoFontResource(){
    init();
}


CairoFontResource::~CairoFontResource(){
}

// -------- CairoFontResource::init() --------
bool CairoFontResource::init(){
    m_library = nullptr;
    FT_Error error = FT_Init_FreeType(&m_library);
    if ( error ){
        return false;
    }
    return true;
}

// -------- CairoFontResource::clear() --------
void CairoFontResource::clear(){
    for ( auto face : m_faces ){
        cairo_font_face_destroy(face.second);
    }
    m_faces.clear();

    if ( m_library != nullptr ){
        FT_Done_FreeType(m_library);
    }
}

/**
 *
 * struct cairo_matrix_t {
 *      double xx; double yx;
 *      double xy; double yy;
 *      double x0; double y0;
 * };
 *
 * x_new = xx * x + xy * y + x0;
 * y_new = yx * x + yy * y + y0;
 *
 * **/
bool CairoFontResource::AddFontFace(int fontID, int faceIndex, const char *fontFaceBuf, size_t bufSize){
    bool ok = false;

    FT_Face ft_face = freetype_create_memory_face(m_library, faceIndex, fontFaceBuf, bufSize);
    if ( ft_face != nullptr ){
        FT_Set_Char_Size(ft_face, 0, 12, 96, 96);
        // Flags to pass to FT_Load_Glyph when loading glyphs from the font. 
        // These flags are OR'ed together with the flags derived from the 
        // cairo_font_options_t passed to cairo_scaled_font_create(), 
        // so only a few values such as FT_LOAD_VERTICAL_LAYOUT, 
        // and FT_LOAD_FORCE_AUTOHINT are useful.  
        int load_flags = 0;
        cairo_font_face_t *cairo_face = cairo_ft_font_face_create_for_ft_face(ft_face, load_flags);

        static const cairo_user_data_key_t key{};
        int status = cairo_font_face_set_user_data(cairo_face, &key, ft_face, (cairo_destroy_func_t) FT_Done_Face);
        if ( status ) {
            LOG(ERROR) << "cairo_font_face_set_user_data() failed. status=" << status;

            cairo_font_face_destroy(cairo_face);
            FT_Done_Face(ft_face);
            return false;
        }

        m_faces[fontID] = cairo_face;

        ok = true;
    } else {
        LOG(ERROR) << "Font not loaded. fontID=" << fontID << " bufSize:" << bufSize;
    }

    return ok;
}


void* CairoFontResource::GetFontFace(int fontID) const{
    auto face_iter = m_faces.find(fontID);
    if ( face_iter != m_faces.end() ){
        return (void*)face_iter->second;
    } else {
        return nullptr;
    }
}

// ********************************************************
// ********           class FontResource           ********
// ********************************************************


FontResource::FontResource(){
}

FontResource::~FontResource(){
}

FontResource *FontResource::NewFontResource(){
    return static_cast<FontResource*>(new CairoFontResource());
}


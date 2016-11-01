#include <map>
#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "ofd.h"
#include "logger.h"
#include "OFDCanvas.h"

using namespace ofd;

// ********************************************************
// ********       class OFDCanvas::DrawDevice      ********
// ********************************************************
class OFDCanvas::DrawDevice {
public:
    DrawDevice(double mmWidth, double mmHeight);
    virtual ~DrawDevice();
    virtual bool SetCharSize(int fontID, int ptSize, int dpiX, int dpiY) = 0;
    virtual bool SetPixelSize(int fontID, int pixelWidth, int pixelHeight) = 0;
    virtual void WriteGlyph(int fontID, double fontSize, uint64_t charcode) = 0;
    virtual void DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize) = 0;

protected:
    cairo_surface_t *m_surface;
    cairo_t *m_cr;

private:
    void initCairo(double mmWidth, double mmHeight);

    std::map<int, cairo_font_face_t*> m_faces;

}; // class OFDCanvas::DrawDevice

// ======== OFDCanvas::DrawDevice::OFDCanvas() ========
OFDCanvas::DrawDevice::DrawDevice(double mmWidth, double mmHeight){
    initCairo(mmWidth, mmHeight);
}

// -------- OFDCanvas::DrawDevice::initCairo() --------
void OFDCanvas::DrawDevice::initCairo(double mmWidth, double mmHeight){

    //double dpi = default_dpi;
    double dpi = 96;
    //double pixels_per_mm = dpi / mm_per_inch;
    double pixelWidth = length_to_pixel(mmWidth, dpi);
    double pixelHeight = length_to_pixel(mmHeight, dpi);
    LOG(DEBUG) << "mmWidth: " << mmWidth << " mmHeight: " << mmHeight;
    LOG(DEBUG) << "pixelWidth: " << pixelWidth << " pixelHeight: " << pixelHeight;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixelWidth, pixelHeight);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    m_surface = surface;
    m_cr = cr;
}

// ======== OFDCanvas::DrawDevice::~OFDCanvas() ========
OFDCanvas::DrawDevice::~DrawDevice(){
    if ( m_surface != nullptr ){
        cairo_surface_destroy(m_surface);
    }
}

// ********************************************************
// ********         class CairoDrawDevice       ********
// ********************************************************
class CairoDrawDevice : public OFDCanvas::DrawDevice {
public:
    CairoDrawDevice(double mmWidth, double mmHeight);
    virtual ~CairoDrawDevice();
    virtual bool SetCharSize(int fontID, int ptSize, int dpiX, int dpiY) override;
    virtual bool SetPixelSize(int fontID, int pixelWidth, int pixelHeight) override;

    virtual void WriteGlyph(int fontID, double fontSize, uint64_t charcode) override;
    virtual void DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize) override;

private:
    std::map<int, cairo_font_face_t*> m_faces;
    FT_Library m_library;

    bool init();
    void clear();
    cairo_font_face_t *getFontFace(int fontID) const{
        auto face_iter = m_faces.find(fontID);
        if ( face_iter != m_faces.end() ){
            return face_iter->second;
        } else {
            return nullptr;
        }
    }

}; // class CairoDrawDevice


cairo_scaled_font_t* cairo_create_scaled_font(cairo_t *cr, cairo_font_face_t *font_face, cairo_matrix_t font_matrix){
    cairo_matrix_t ctm;
    cairo_get_matrix(cr, &ctm);

    cairo_font_options_t *font_options = cairo_font_options_create();
    cairo_get_font_options (cr, font_options);

    cairo_scaled_font_t *scaled_font = 
        cairo_scaled_font_create(font_face, &font_matrix, &ctm, font_options);
    cairo_font_options_destroy(font_options);

    return scaled_font;
}

// ======== CairoDrawDevice::CairoDrawDevice() ========
CairoDrawDevice::CairoDrawDevice(double mmWidth, double mmHeight)
    : OFDCanvas::DrawDevice(mmWidth, mmHeight){
    init();
}

// -------- CairoDrawDevice::init() --------
bool CairoDrawDevice::init(){
    m_library = nullptr;
    FT_Error error = FT_Init_FreeType(&m_library);
    if ( error ){
        return false;
    }
    return true;
}

// ======== CairoDrawDevice::~CairoDrawDevice() ========
CairoDrawDevice::~CairoDrawDevice(){
    clear();
}

// -------- CairoDrawDevice::clear() --------
void CairoDrawDevice::clear(){
    for ( auto face : m_faces ){
        cairo_font_face_destroy(face.second);
    }
    m_faces.clear();

    if ( m_library != nullptr ){
        FT_Done_FreeType(m_library);
    }
}
// ======== CairoDrawDevice::SetCharSize() ========
bool CairoDrawDevice::SetCharSize(int fontID, int ptSize, int dpiX, int dpiY){
    return true;
}

// ======== CairoDrawDevice::SetCharSize() ========
bool CairoDrawDevice::SetPixelSize(int fontID, int pixelWidth, int pixelHeight){
    return true;
}

// ======== CairoDrawDevice::WriteGlyph() ========
void CairoDrawDevice::WriteGlyph(int fontID, double fontSize, uint64_t charcode){
    cairo_font_face_t *cairo_face = m_faces[fontID];
    cairo_set_font_face(m_cr, cairo_face);
    cairo_set_font_size(m_cr, fontSize);
}

// ======== CairoDrawDevice::DrawGlyph() ========
void CairoDrawDevice::DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize){
    cairo_font_face_t *font_face = m_faces[fontID];

    cairo_matrix_t font_matrix{fontSize, 0, 0, fontSize, 0, 0};
    cairo_matrix_init_identity (&font_matrix);
    cairo_scaled_font_t *scaled_font = cairo_create_scaled_font(m_cr, font_face, font_matrix);

    if ( cairo_scaled_font_get_type(scaled_font) != CAIRO_FONT_TYPE_FT) {
        LOG(ERROR) << "Unexpected value from cairo_scaled_font_get_type: " << 
             cairo_scaled_font_get_type (scaled_font) << " (expected " << CAIRO_FONT_TYPE_FT << ")\n";
        cairo_scaled_font_destroy(scaled_font);
        return;
    }

    FT_Face ft_face = cairo_ft_scaled_font_lock_face (scaled_font);

    cairo_save(m_cr);

    cairo_font_face_t *scaled_font_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
    cairo_set_font_face(m_cr, scaled_font_face);

    cairo_font_extents_t font_extents;
    cairo_font_extents(m_cr, &font_extents);

    // Draw something
    cairo_show_text(m_cr, text.c_str());

    cairo_restore(m_cr);

    cairo_font_face_destroy(scaled_font_face);
    cairo_ft_scaled_font_unlock_face(scaled_font);
    cairo_scaled_font_destroy(scaled_font);
}

// class FreetypeDrawDevice
// ********************************************************
// ********         class FreetypeDrawDevice       ********
// ********************************************************
class FreetypeDrawDevice : public OFDCanvas::DrawDevice {
public:
    FreetypeDrawDevice(double mmWidth, double mmHeight);
    virtual ~FreetypeDrawDevice();
    virtual bool SetCharSize(int fontID, int ptSize, int dpiX, int dpiY) override;
    virtual bool SetPixelSize(int fontID, int pixelWidth, int pixelHeight) override;
    virtual void WriteGlyph(int fontID, double fontSize, uint64_t charcode) override;
    virtual void DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize) override;

private:
    FT_Library m_library;
    std::map<int, FT_Face> m_faces;

    bool init();
    void clear();
    FT_Face getFontFace(int fontID) const{
        auto face_iter = m_faces.find(fontID);
        if ( face_iter != m_faces.end() ){
            return face_iter->second;
        } else {
            return nullptr;
        }
    }

}; // class FreetypeDrawDevice


// ======== FreetypeDrawDevice::FreetypeDrawDevice() ========
FreetypeDrawDevice::FreetypeDrawDevice(double mmWidth, double mmHeight)
    : OFDCanvas::DrawDevice(mmWidth, mmHeight){
    init();
}

// -------- FreetypeDrawDevice::init() --------
bool FreetypeDrawDevice::init(){
    FT_Error error = FT_Init_FreeType(&m_library);
    if ( error ){
        return false;
    }
    return true;
}

// ======== FreetypeDrawDevice::~FreetypeDrawDevice() ========
FreetypeDrawDevice::~FreetypeDrawDevice(){
    clear();
}

// -------- FreetypeDrawDevice::clear() --------
void FreetypeDrawDevice::clear(){
    for ( auto face : m_faces ){
        FT_Done_Face(face.second);
    }
    m_faces.clear();
    FT_Done_FreeType(m_library);
}

// ======== FreetypeDrawDevice::SetCharSize() ========
bool FreetypeDrawDevice::SetCharSize(int fontID, int ptSize, int dpiX, int dpiY){
    FT_Face face = getFontFace(fontID);
    if ( face != nullptr ){
        FT_Error error = FT_Set_Char_Size(face, 
                0, /* char_width in 1/64th of points */
                ptSize * 64, /*char_height in 1/64th of points */
                dpiX, /* horizontal device resolution */
                dpiY /* vertical device resolution */
                );
        if ( error == FT_Err_Ok ){
            return true;
        }
    }
    return false;
}

// ======== FreetypeDrawDevice::SetPixelSize() ========
bool FreetypeDrawDevice::SetPixelSize(int fontID, int pixelWidth, int pixelHeight){
    FT_Face face = getFontFace(fontID);
    if ( face != nullptr ){
        FT_Error error = FT_Set_Pixel_Sizes(face, pixelWidth, pixelHeight);
        if ( error == FT_Err_Ok ){
            return true;
        }
    }
    return false;
}

// ======== FreetypeDrawDevice::WriteGlyph() ========
void FreetypeDrawDevice::WriteGlyph(int fontID, double fontSize, uint64_t charcode){
    FT_Error error;

    FT_Face face = getFontFace(fontID);
    if ( face != nullptr ){
        FT_UInt glyphIndex = FT_Get_Char_Index(face, charcode);
        if ( glyphIndex > 0 ) {
            if ( FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP) == 0 ){
                FT_Glyph glyph;
                error = FT_Get_Glyph(face->glyph, &glyph);
                if ( error == 0 ){

                } else {
                    LOG(ERROR) << "FT_Get_Glyph() failed. fontID=" << fontID;
                }
            } else {
                LOG(ERROR) << "FT_Load_Glyph() failed. glyphIndex=" << glyphIndex;
            }
        } else {
            LOG(WARNING) << "charcode= " << charcode << " GlyphIndex not found";
        }
    } else {
        LOG(WARNING) << "Font (ID=" << fontID << ") not found.";
    }
}

// ======== FreetypeDrawDevice::DrawGlyph() ========
void FreetypeDrawDevice::DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize){
}

// ********************************************************
// ********              class OFDCanvas           ********
// ********************************************************

// ======== OFDCanvas::OFDCanvas() ========
OFDCanvas::OFDCanvas(double mmWidth, double mmHeight){
    m_drawDevice = std::unique_ptr<OFDCanvas::DrawDevice>((static_cast<OFDCanvas::DrawDevice*>(new CairoDrawDevice(mmWidth, mmHeight))));
    //m_drawDevice = std::unique_ptr<OFDCanvas::DrawDevice>((static_cast<OFDCanvas::DrawDevice*>(new FreetypeDrawDevice())));
}

// ======== OFDCanvas::~OFDCanvas() ========
OFDCanvas::~OFDCanvas(){
    m_drawDevice = nullptr;
}

// ======== OFDCanvas::SetCharSize() ========
bool OFDCanvas::SetCharSize(int fontID, int ptSize, int dpiX, int dpiY){
    return m_drawDevice->SetCharSize(fontID, ptSize, dpiX, dpiY);
}

// ======== OFDCanvas::SetPixelSize() ========
bool OFDCanvas::SetPixelSize(int fontID, int pixelWidth, int pixelHeight){
    return m_drawDevice->SetPixelSize(fontID, pixelWidth, pixelHeight);
}

// ======== OFDCanvas::WriteGlyph() ========
void OFDCanvas::WriteGlyph(int fontID, double fontSize, uint64_t charcode){
    return m_drawDevice->WriteGlyph(fontID, fontSize, charcode);
}

// ======== OFDCanvas::DrawGlyph() ========
void OFDCanvas::DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize){
    m_drawDevice->DrawGlyph(x, y, text, fontID, fontSize);
}


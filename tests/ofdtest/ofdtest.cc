#include <iostream>
#include <assert.h>
//#include "OFDPackage.h"
//#include "OFDDocument.h"
//#include "OFDPage.h"
#include "logger.h"

//using namespace ofd;

void test_poppler(int argc, char *argv[]);
void test_mupdf(int argc, char *argv[]);


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
//#include <ftadvanc.h>
//#include <ftsnames.h>
//#include <tttables.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>
//#include <harfbuzz/hb-glib.h>

// -------- test_freetype() --------
void test_freetype(int argc, char *argv[]){
    if ( argc < 2 ){
        LOG(WARNING) << "Usage: " << argv[0] << " <freetype_filename>";
        return;
    }
    std::string freetype_filename = argv[1];
    std::string text = argv[2];
    std::string png_filename = "test_freetype.png";

    double pixelWidth = 595.0;
    double pixelHeight = 842.0;
    double ptSize = 50.0;
    int hdpi = 72;
    int vdpi = 72;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixelWidth, pixelHeight);
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);

    FT_Library ft_library;
    assert(!FT_Init_FreeType(&ft_library));

    FT_Face ft_face;
    assert(!FT_New_Face(ft_library, freetype_filename.c_str(), 0, &ft_face));
    assert(!FT_Set_Char_Size(ft_face, 0, ptSize, hdpi, vdpi));

    cairo_font_face_t *cairo_ft_face;
    cairo_ft_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);

    hb_font_t *hb_ft_font = hb_ft_font_create(ft_face, nullptr);
    //hb_face_t *hb_ft_face = hb_ft_face_create(ft_face, nullptr);

    hb_buffer_t *hb_buf = hb_buffer_create();
    hb_buffer_set_unicode_funcs(hb_buf, hb_icu_get_unicode_funcs());
    //hb_buffer_set_unicode_funcs(hb_buf, hb_glib_get_unicode_funcs());
    hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buf, HB_SCRIPT_HAN);
    hb_buffer_set_language(hb_buf, hb_language_from_string("ch", 2));

    // Layout the text
    hb_buffer_add_utf8(hb_buf, text.c_str(), text.length(), 0, text.length());
    hb_shape(hb_ft_font, hb_buf, nullptr, 0);

    // Hand the layout tot cairo to render
    uint32_t num_glyphs = 0;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buf, &num_glyphs);
    LOG(DEBUG) << "hb_buffer_get_glyph_info() get num_glyphs=" << num_glyphs;
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buf, &num_glyphs);
    LOG(DEBUG) << "hb_buffer_get_glyph_positions() get num_glyphs=" << num_glyphs;
    cairo_glyph_t *cairo_glyphs = (cairo_glyph_t*)new char[sizeof(cairo_glyph_t) * num_glyphs];

    double pixelsStringWidth = 0.0;
    for ( uint32_t i = 0 ; i < num_glyphs ; i++ ){
        pixelsStringWidth += glyph_pos[i].x_advance / 64;
    }
    int x = 20;
    int y = 50;
    for ( uint32_t i = 0 ; i < num_glyphs ; i++ ){
        cairo_glyphs[i].index = glyph_info[i].codepoint;
        cairo_glyphs[i].x = x + (glyph_pos[i].x_offset / 64);
        cairo_glyphs[i].y = y - (glyph_pos[i].y_offset / 64);
        LOG(DEBUG) << "[" << i << "] " << "x:" << cairo_glyphs[i].x << " y:" << cairo_glyphs[i].y;
        x += glyph_pos[i].x_advance / 64;
        y -= glyph_pos[i].y_advance / 64;
    }

    cairo_set_font_face(cr, cairo_ft_face);
    cairo_set_font_size(cr, ptSize);
    cairo_show_glyphs(cr, cairo_glyphs, num_glyphs);

    cairo_surface_write_to_png(surface, png_filename.c_str());

    delete cairo_glyphs;
    hb_buffer_destroy(hb_buf);
}

int main(int argc, char *argv[]){
    if ( argc <= 1 ){
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 0;
    }

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];

    //test_poppler(argc, argv);
    test_mupdf(argc, argv);
    //test_freetype(argc, argv);

    LOG(INFO) << "Done.";

    return 0;
}

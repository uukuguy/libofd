#include <tuple>
#include <cairo.h>
#include <cairo-ft.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "utils/logger.h"

//using namespace ofd;

static FT_Library ft_lib;

static cairo_user_data_key_t _ft_cairo_key;
static void _ft_done_face_uncached (void *closure)
{
    FT_Face face = (FT_Face) closure;
    FT_Done_Face (face);
}

std::tuple<char*, size_t, bool> ReadFontData(const std::string &fontfile){
    bool ok = false;
    char *fontData = nullptr;
    size_t fontDataSize = 0;

    std::ifstream ifile(fontfile, std::ios::binary);

    ifile.seekg(0, std::ios::end);
    fontDataSize = ifile.tellg();
    LOG(INFO) << "fontfile: " << fontfile << " len: " << fontDataSize;

    fontData = new char[fontDataSize];
    ifile.seekg(0, std::ios::beg);
    ifile.read(fontData, fontDataSize);

    ok = true;

    ifile.close();

    return std::make_tuple(fontData, fontDataSize, ok);
}

std::tuple<FT_Face, cairo_font_face_t*, bool> createCairoFontFace(char *fontData, size_t fontDataLen){
    bool ok = false;

    FT_Face face;
    cairo_font_face_t *font_face;

    if ( FT_New_Memory_Face(ft_lib, (unsigned char *)fontData, fontDataLen, 0, &face) != 0 ){
        LOG(ERROR) << "FT_New_Memory_Face() in OFDFont::createCairoFontFace() failed.";
    } else {
        LOG(INFO) << "FT_New_Memory_Face() OK.";

        font_face = cairo_ft_font_face_create_for_ft_face (face, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
        if ( cairo_font_face_set_user_data (font_face, &_ft_cairo_key, face, _ft_done_face_uncached) != 0 ) {
            LOG(ERROR) << "cairo_font_face_set_user_data() in OFDFont::createCairoFontFace() failed.";
            _ft_done_face_uncached(face);
            cairo_font_face_destroy(font_face);
            font_face = nullptr;
        } else {
            LOG(INFO) << " cairo_font_face_set_user_data() OK.";
            ok = true;
        }
    }

    return std::make_tuple(face, font_face, ok);
}

void test_freetype(int argc, char *argv[]){
    if ( argc <= 1 ){
        std::cout << "Usage: ofdtest <fontfile>" << std::endl;
    }

    std::string fontfile = argv[1];
    double dpi = 96.0;

    // -------- FT_Init_FreeType()
    FT_Error error = FT_Init_FreeType(&ft_lib);
    if ( error ){
        LOG(ERROR) << "FT_Init_FreeType() failed.";
        return;
    }

    // -------- cairo_image_surface_create()
    int imageWidth = 794;
    int imageHeight = 1122;
    cairo_surface_t *imageSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
    if ( imageSurface == nullptr ){
        LOG(ERROR) << "create_image_surface() failed. ";
        return;
    }
    cairo_t *crImage = cairo_create(imageSurface); 
    cairo_set_source_rgb(crImage, .3, .6, .3);
    cairo_paint(crImage);
    cairo_set_source_rgb(crImage, 0.0, 0.0, 0.0);
    cairo_rectangle(crImage, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    cairo_stroke(crImage);


    // -------- ReadFontData()
    char *fontData = nullptr;
    size_t fontDataSize = 0;
    bool ok = false;
    std::tie(fontData, fontDataSize, ok) = ReadFontData(fontfile);
    

    // -------- Create Font Face
    FT_Face face;
    cairo_font_face_t *font_face;
    std::tie(face, font_face, ok) = createCairoFontFace(fontData, fontDataSize); 

    cairo_t *cr = crImage;

    cairo_set_font_face(cr, font_face);
    //cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    // -------- fontMatrix --------
    double ctm[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    __attribute__((unused)) double xx = ctm[0];
    __attribute__((unused)) double xy = ctm[1];
    __attribute__((unused)) double yx = ctm[2];
    __attribute__((unused)) double yy = ctm[3];
    __attribute__((unused)) double x0 = ctm[4];
    __attribute__((unused)) double y0 = ctm[5];

    cairo_matrix_t fontMatrix;
    cairo_get_font_matrix(cr, &fontMatrix);
    double fontSize = 32.0;
    double fontPixels = dpi * fontSize / 72;
    fontMatrix.xx = fontPixels * xx;
    fontMatrix.yy = fontPixels * yy;
    fontMatrix.x0 = x0;
    fontMatrix.y0 = y0;
    cairo_set_font_matrix(cr, &fontMatrix);


    // -------- Draw Text --------
    double X = 100;
    double Y = 150;
    std::string text = "矢量图形abc123";

    double X1 = X * dpi / 72;
    double Y1 = Y * dpi / 72;

    cairo_text_extents_t te;
    cairo_text_extents(cr, text.c_str(), &te);
    //cairo_move_to(cr, X1 + 0.5 - te.width / 2 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
    cairo_move_to(cr, X1, Y1);
    cairo_show_text(cr, text.c_str());

    // -------- cairo_surface_write_to_png()
    std::string png_filename = "test_freetype.png";
    cairo_surface_write_to_png(imageSurface, png_filename.c_str());

    // -------- cairo_surface_destroy()
    cairo_destroy(crImage);
    cairo_surface_destroy(imageSurface);

    // -------- FT_Done_FreeType()
    FT_Done_FreeType(ft_lib);
}

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

void test_freetype(){
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if ( error ){
        return;
    }

    FT_Face face;
    //error = FT_New_Face(library "arial.ttf", 0, &face);
    char buffer[1024];
    size_t buffer_size = 1024;
    error = FT_New_Memory_Face(library, (const FT_Byte*)buffer, buffer_size, 0, &face);
    if ( error == FT_Err_Unknown_File_Format ){
        return;
    } else if ( error ){
        return;
    }
    // face->num_faces
    // face->num_glyphs
    // face->flags
    // face->num_fixed_size

    error = FT_Set_Char_Size( face, 
            0, // char_width in 1/64th of points.
            16 * 64, // char_height in 1/64th of points
            300, // horizontal device resolution
            300 // vertical device resolution
            );
    error = FT_Set_Pixel_Sizes(face, 
            0, // pixel_width
            16 // pixel_height
            );

    FT_ULong charcode = 0;
    int glyph_index = FT_Get_Char_Index(face, charcode);

    FT_Glyph glyph;
    int load_flags = FT_LOAD_DEFAULT;
    error = FT_Load_Glyph(face, glyph_index, load_flags);
    error = FT_Get_Glyph(face->glyph, &glyph);

    FT_Render_Mode render_mode;
    error = FT_Render_Glyph( face->glyph, render_mode);

    FT_Matrix matrix;
    // 水平剪切
    matrix.xx = 0x10000L;
    matrix.xy = 0x12 * 0x10000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L;

    FT_Vector delta;
    delta.x = -100 * 64;
    delta.y = 50 * 64;

    FT_Set_Transform(face, &matrix, &delta);

    FT_BBox bbox;
    int bbox_mode = FT_GLYPH_BBOX_UNSCALED; // FT_GLYPH_BBOX_TRUNCATE
    FT_Glyph_Get_CBox(glyph, bbox_mode, &bbox);
    int width = bbox.xMax - bbox.xMin;
    int height = bbox.yMax - bbox.yMin;

    FT_Vector origin;
    origin.x = 32;
    origin.y = 0;

    //draw_bitmap(&slot->bitmap, pen_x + slot->bitmap_left, pen_y + slot->bitmap_top);

}

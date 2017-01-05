#include <goo/JpegWriter.h>
#include "OFDOutputDev.h"
#include "utils/logger.h"

void OFDOutputDev::writePageImage(const std::string &filename){
    ImgWriter *writer = nullptr;
    FILE *file;

    bool mono = false;
    bool gray = false;
    if ( gray ){
        writer = new JpegWriter(JpegWriter::GRAY);
    } else {
        writer = new JpegWriter(JpegWriter::RGB);
    }

    if (!writer)
        return;

    if ( filename == std::string("fd://0") ){
        file = stdout;
    } else {
        file = fopen(filename.c_str(), "wb");
    }

    if (!file) {
        LOG(ERROR) << "Error opening output file" <<  filename;
        exit(2);
    }

    int height = cairo_image_surface_get_height(m_outputSurface);
    int width = cairo_image_surface_get_width(m_outputSurface);
    int stride = cairo_image_surface_get_stride(m_outputSurface);
    cairo_surface_flush(m_outputSurface);
    unsigned char *data = cairo_image_surface_get_data(m_outputSurface);

    if (!writer->init(file, width, height, m_resolutionX, m_resolutionY)) {
        LOG(ERROR) << "Error writing " << filename;
        exit(2);
    }
    unsigned char *row = (unsigned char *) gmallocn(width, 4);

    for (int y = 0; y < height; y++ ) {
        uint32_t *pixel = (uint32_t *) (data + y * stride);
        unsigned char *rowp = row;
        int bit = 7;
        for (int x = 0; x < width; x++, pixel++) {
            if (m_transp) {
                //if (tiff) {
                    //// RGBA premultipled format
                    //*rowp++ = (*pixel &   0xff0000) >> 16;
                    //*rowp++ = (*pixel &   0x00ff00) >>  8;
                    //*rowp++ = (*pixel &   0x0000ff) >>  0;
                    //*rowp++ = (*pixel & 0xff000000) >> 24;
                //} else {
                    // unpremultiply into RGBA format
                    uint8_t a;
                    a = (*pixel & 0xff000000) >> 24;
                    if (a == 0) {
                        *rowp++ = 0;
                        *rowp++ = 0;
                        *rowp++ = 0;
                    } else {
                        *rowp++ = (((*pixel & 0xff0000) >> 16) * 255 + a / 2) / a;
                        *rowp++ = (((*pixel & 0x00ff00) >>  8) * 255 + a / 2) / a;
                        *rowp++ = (((*pixel & 0x0000ff) >>  0) * 255 + a / 2) / a;
                    }
                    *rowp++ = a;
                //}
            } else if (gray || mono) {
                // convert to gray
                // The PDF Reference specifies the DeviceRGB to DeviceGray conversion as
                // gray = 0.3*red + 0.59*green + 0.11*blue
                int r = (*pixel & 0x00ff0000) >> 16;
                int g = (*pixel & 0x0000ff00) >>  8;
                int b = (*pixel & 0x000000ff) >>  0;
                // an arbitrary integer approximation of .3*r + .59*g + .11*b
                int y = (r*19661+g*38666+b*7209 + 32829)>>16;
                if (mono) {
                    if (bit == 7)
                        *rowp = 0;
                    if (y > 127)
                        *rowp |= (1 << bit);
                    bit--;
                    if (bit < 0) {
                        bit = 7;
                        rowp++;
                    }
                } else {
                    *rowp++ = y;
                }
            } else {
                // copy into RGB format
                *rowp++ = (*pixel & 0x00ff0000) >> 16;
                *rowp++ = (*pixel & 0x0000ff00) >>  8;
                *rowp++ = (*pixel & 0x000000ff) >>  0;
            }
        }
        writer->writeRow(&row);
    }
    gfree(row);
    writer->close();
    delete writer;
    if (file == stdout){ 
        fflush(file);
    } else {
        fclose(file);
    }
}

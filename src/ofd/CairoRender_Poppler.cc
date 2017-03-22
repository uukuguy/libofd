
#include <stdint.h>
#include "CairoRescaleBox.h"
#include <GfxState.h>
#include <OutputDev.h>
class RescaleDrawImage : public CairoRescaleBox {
private:
  ImageStream *imgStr;
  GfxRGB *lookup;
  int width;
  GfxImageColorMap *colorMap;
  int nComps;
  int nBits;
  int *maskColors;
  int current_row;
  GBool imageError;

public:
  RescaleDrawImage() :
    imgStr(nullptr), lookup(nullptr),
    width(-1), colorMap(nullptr), nComps(-1), nBits(-1),
    maskColors(nullptr), current_row(-1), imageError(gFalse){

  }

  cairo_surface_t *getSourceImage(Stream *str,
                                  int widthA, int heightA,
                                  int scaledWidth, int scaledHeight,
                                  GBool printing,
                                  GfxImageColorMap *colorMapA,
                                  int *maskColorsA) {

      colorMap = colorMapA;
      int nCompsA = colorMap->getNumPixelComps();
      int nBitsA = colorMap->getBits();
      return getSourceImage(str, widthA, heightA, scaledWidth, scaledHeight, printing, nCompsA, nBitsA, maskColorsA);
  }

  cairo_surface_t *getSourceImage(Stream *str,
                                  int widthA, int height,
                                  int scaledWidth, int scaledHeight,
                                  GBool printing,
                                  int nCompsA,
                                  int nBitsA,
                                  //GfxImageColorMap *colorMapA,
                                  int *maskColorsA) {
    cairo_surface_t *image = NULL;
    //int i;

    lookup = NULL;
    //colorMap = colorMapA;
    nComps = nCompsA;
    nBits = nBitsA;
    maskColors = maskColorsA;
    width = widthA;
    current_row = -1;
    imageError = gFalse;

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width,
            nComps, nBits);
                             //colorMap->getNumPixelComps(),
                             //colorMap->getBits());
    imgStr->reset();

#if 0
    /* ICCBased color space doesn't do any color correction
     * so check its underlying color space as well */
    int is_identity_transform;
    is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
      (colorMap->getColorSpace()->getMode() == csICCBased &&
       ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

    // special case for one-channel (monochrome/gray/separation) images:
    // build a lookup table here
    if ( colorMap != nullptr && colorMap->getNumPixelComps() == 1) {
        int n;
        Guchar pix;

        n = 1 << colorMap->getBits();
        lookup = (GfxRGB *)gmallocn(n, sizeof(GfxRGB));
        for (int i = 0; i < n; ++i) {
            pix = (Guchar)i;

            colorMap->getRGB(&pix, &lookup[i]);
        }
    }

    if (printing || scaledWidth >= width || scaledHeight >= height) {
      // No downscaling. Create cairo image containing the source image data.
      unsigned char *buffer;
      int stride;

      image = cairo_image_surface_create (maskColors ?
                                          CAIRO_FORMAT_ARGB32 :
                                          CAIRO_FORMAT_RGB24,
                                          width, height);
      if (cairo_surface_status (image))
        goto cleanup;

      buffer = cairo_image_surface_get_data (image);
      stride = cairo_image_surface_get_stride (image);
      for (int y = 0; y < height; y++) {
        uint32_t *dest = (uint32_t *) (buffer + y * stride);
        getRow(y, dest);
      }
    } else {
      // // Downscaling required. Create cairo image the size of the
      // rescaled image and // downscale the source image data into
      // the cairo image. downScaleImage() will call getRow() to read
      // source image data from the image stream. This avoids having
      // to create an image the size of the source image which may
      // exceed cairo's 32676x32767 image size limit (and also saves a
      // lot of memory).
      image = cairo_image_surface_create (maskColors ?
                                          CAIRO_FORMAT_ARGB32 :
                                          CAIRO_FORMAT_RGB24,
                                          scaledWidth, scaledHeight);
      if (cairo_surface_status (image))
        goto cleanup;

      downScaleImage(width, height,
                     scaledWidth, scaledHeight,
                     0, 0, scaledWidth, scaledHeight,
                     image);
    }
    cairo_surface_mark_dirty (image);

  cleanup:
    gfree(lookup);
    imgStr->close();
    delete imgStr;
    return image;
  }

  void getRow(int row_num, uint32_t *row_data) {
    int i;
    Guchar *pix;

    if (row_num <= current_row)
      return;

    while (current_row  < row_num) {
      pix = imgStr->getLine();
      current_row++;
    }

    if (unlikely(pix == NULL)) {
        memset(row_data, 0, width*4);
        if (!imageError) {
            error(errInternal, -1, "Bad image stream");
            imageError = gTrue;
        }
    } else if (lookup) {
        Guchar *p = pix;
        GfxRGB rgb;

        for (i = 0; i < width; i++) {
            rgb = lookup[*p];
            row_data[i] =
                ((int) colToByte(rgb.r) << 16) |
                ((int) colToByte(rgb.g) << 8) |
                ((int) colToByte(rgb.b) << 0);
            p++;
        }
    } else {
        // FIXME
        if ( colorMap != nullptr ){
            colorMap->getRGBLine (pix, row_data, width);
        } else {
            memcpy(row_data, pix, width * 4);
        }
    }

    if ( maskColors != nullptr ) {
        for (int x = 0; x < width; x++) {
            bool is_opaque = false;
            for (int i = 0; i < nComps; ++i) {
                if (pix[i] < maskColors[2*i] ||
                        pix[i] > maskColors[2*i+1]) {
                    is_opaque = true;
                    break;
                }
            }
            if (is_opaque)
                *row_data |= 0xff000000;
            else
                *row_data = 0;
            row_data++;
            pix += nComps;
        }
        }
    }

};

namespace ofd{
    cairo_surface_t *createImageSurface(Stream *str, int widthA, int heightA, int scaledWidth, int scaledHeight, int nComps, int nBits){  
        RescaleDrawImage rescale;
        cairo_surface_t *imageSurface = rescale.getSourceImage(str, widthA, heightA, scaledWidth, scaledHeight, false, nComps, nBits, nullptr);//colorMap, maskColors);
        return imageSurface;
    }


    cairo_surface_t *createImageSurface(Stream *str, int widthA, int heightA, int scaledWidth, int scaledHeight, GfxImageColorMap *colorMap, int *maskColors){  
        RescaleDrawImage rescale;
        cairo_surface_t *imageSurface = rescale.getSourceImage(str, widthA, heightA, scaledWidth, scaledHeight, false, colorMap, maskColors);
        return imageSurface;
    }
  ;
}


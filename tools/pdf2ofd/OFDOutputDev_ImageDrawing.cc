#include <splash/SplashBitmap.h>
#include <JBIG2Stream.h>
#include "CairoRescaleBox.h"
#include "OFDOutputDev.h"
#include "ofd/Document.h"
#include "ofd/Resource.h"
#include "ofd/Page.h"
#include "ofd/ImageObject.h"
#include "ofd/Image.h"
#include "utils/logger.h"


static inline int splashRound(SplashCoord x) {
  return (int)floor(x + 0.5);
}

static inline int splashCeil(SplashCoord x) {
  return (int)ceil(x);
}

static inline int splashFloor(SplashCoord x) {
  return (int)floor(x);
}

/* Taken from cairo/doc/tutorial/src/singular.c */
static void get_singular_values (const cairo_matrix_t *matrix,
		     double               *major,
		     double               *minor) {
    double xx = matrix->xx, xy = matrix->xy;
    double yx = matrix->yx, yy = matrix->yy;

    double a = xx*xx+yx*yx;
    double b = xy*xy+yy*yy;
    double k = xx*xy+yx*yy;

    double f = (a+b) * .5;
    double g = (a-b) * .5;
    double delta = sqrt (g*g + k*k);

    if (major)
        *major = sqrt (f + delta);
    if (minor)
        *minor = sqrt (f - delta);
}

void OFDOutputDev::getImageScaledSize(const cairo_matrix_t *matrix,
                                   int                   orig_width,
				   int                   orig_height,
				   int                  *scaledWidth,
				   int                  *scaledHeight) {
    double xScale;
    double yScale;
    if (orig_width > orig_height)
        get_singular_values(matrix, &xScale, &yScale);
    else
        get_singular_values (matrix, &yScale, &xScale);

    int tx, tx2, ty, ty2; /* the integer co-oridinates of the resulting image */
    if (xScale >= 0) {
        tx = splashRound(matrix->x0 - 0.01);
        tx2 = splashRound(matrix->x0 + xScale + 0.01) - 1;
    } else {
        tx = splashRound(matrix->x0 + 0.01) - 1;
        tx2 = splashRound(matrix->x0 + xScale - 0.01);
    }
    *scaledWidth = abs(tx2 - tx) + 1;
    //scaledWidth = splashRound(fabs(xScale));
    if (*scaledWidth == 0) {
        // technically, this should draw nothing, but it generally seems
        // better to draw a one-pixel-wide stripe rather than throwing it
        // away
        *scaledWidth = 1;
    }
    if (yScale >= 0) {
        ty = splashFloor(matrix->y0 + 0.01);
        ty2 = splashCeil(matrix->y0 + yScale - 0.01);
    } else {
        ty = splashCeil(matrix->y0 - 0.01);
        ty2 = splashFloor(matrix->y0 + yScale + 0.01);
    }
    *scaledHeight = abs(ty2 - ty);
    if (*scaledHeight == 0) {
        *scaledHeight = 1;
    }
}

cairo_filter_t OFDOutputDev::getFilterForSurface(cairo_surface_t *image, GBool interpolate) {
    if (interpolate)
        return CAIRO_FILTER_BILINEAR;

    int orig_width = cairo_image_surface_get_width(image);
    int orig_height = cairo_image_surface_get_height(image);
    if (orig_width == 0 || orig_height == 0)
        return CAIRO_FILTER_NEAREST;

    /* When printing, don't change the interpolation. */
    if ( m_printing )
        return CAIRO_FILTER_NEAREST;

    cairo_matrix_t matrix;
    cairo_get_matrix(m_cairo, &matrix);
    int scaled_width, scaled_height;
    getImageScaledSize(&matrix, orig_width, orig_height, &scaled_width, &scaled_height);

    /* When scale factor is >= 400% we don't interpolate. See bugs #25268, #9860 */
    if (scaled_width / orig_width >= 4 || scaled_height / orig_height >= 4)
        return CAIRO_FILTER_NEAREST;

    return CAIRO_FILTER_BILINEAR;
}


class RescaleDrawImage : public CairoRescaleBox {
private:
  ImageStream *imgStr;
  GfxRGB *lookup;
  int width;
  GfxImageColorMap *colorMap;
  int *maskColors;
  int current_row;
  GBool imageError;

public:
  cairo_surface_t *getSourceImage(Stream *str,
                                  int widthA, int height,
                                  int scaledWidth, int scaledHeight,
                                  GBool printing,
                                  GfxImageColorMap *colorMapA,
                                  int *maskColorsA) {
    cairo_surface_t *image = NULL;
    int i;

    lookup = NULL;
    colorMap = colorMapA;
    maskColors = maskColorsA;
    width = widthA;
    current_row = -1;
    imageError = gFalse;

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width,
                             colorMap->getNumPixelComps(),
                             colorMap->getBits());
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
    if (colorMap->getNumPixelComps() == 1) {
      int n;
      Guchar pix;

      n = 1 << colorMap->getBits();
      lookup = (GfxRGB *)gmallocn(n, sizeof(GfxRGB));
      for (i = 0; i < n; ++i) {
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
      colorMap->getRGBLine (pix, row_data, width);
    }

    if (maskColors) {
      for (int x = 0; x < width; x++) {
        bool is_opaque = false;
        for (int i = 0; i < colorMap->getNumPixelComps(); ++i) {
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
        pix += colorMap->getNumPixelComps();
      }
    }
  }

};

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 2)
static cairo_status_t setMimeIdFromRef(cairo_surface_t *surface,
				       const char *mime_type,
				       const char *mime_id_prefix,
				       Ref ref) {
    GooString *mime_id;
    char *idBuffer;
    cairo_status_t status;

    mime_id = new GooString;

    if (mime_id_prefix)
        mime_id->append(mime_id_prefix);

    mime_id->appendf("{0:d}-{1:d}", ref.gen, ref.num);

    idBuffer = copyString(mime_id->getCString());
    status = cairo_surface_set_mime_data (surface, mime_type,
            (const unsigned char *)idBuffer,
            mime_id->getLength(),
            gfree, idBuffer);
    delete mime_id;
    if (status)
        gfree (idBuffer);
    return status;
}
#endif

GBool OFDOutputDev::getStreamData (Stream *str, char **buffer, int *length) {
    int len, i;
    char *strBuffer;

    len = 0;
    str->close();
    str->reset();
    while (str->getChar() != EOF) len++;
    if (len == 0)
        return gFalse;

    strBuffer = (char *)gmalloc (len);

    str->close();
    str->reset();
    for (i = 0; i < len; ++i)
        strBuffer[i] = str->getChar();

    *buffer = strBuffer;
    *length = len;

    return gTrue;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
GBool OFDOutputDev::setMimeDataForJBIG2Globals(Stream  *str, cairo_surface_t *image) {
    JBIG2Stream *jb2Str = static_cast<JBIG2Stream *>(str);
    Object* globalsStr = jb2Str->getGlobalsStream();
    char *globalsBuffer;
    int globalsLength;

    // nothing to do for JBIG2 stream without Globals
    if (!globalsStr->isStream())
        return gTrue;

    if (setMimeIdFromRef(image, CAIRO_MIME_TYPE_JBIG2_GLOBAL_ID, NULL,
                jb2Str->getGlobalsStreamRef()))
        return gFalse;

    if (!getStreamData(globalsStr->getStream(), &globalsBuffer, &globalsLength))
        return gFalse;

    if (cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2_GLOBAL,
                (const unsigned char*)globalsBuffer,
                globalsLength,
                gfree, (void*)globalsBuffer))
    {
        gfree (globalsBuffer);
        return gFalse;
    }

    return gTrue;
}
#endif

static GBool colorMapHasIdentityDecodeMap(GfxImageColorMap *colorMap) {
  for (int i = 0; i < colorMap->getNumPixelComps(); i++) {
    if (colorMap->getDecodeLow(i) != 0.0 || colorMap->getDecodeHigh(i) != 1.0)
      return gFalse;
  }
  return gTrue;
}

void OFDOutputDev::setMimeData(GfxState *state, Stream *str, Object *ref, GfxImageColorMap *colorMap, cairo_surface_t *image) {
    char *strBuffer;
    int len;
    Object obj;
    GfxColorSpace *colorSpace;
    StreamKind  strKind = str->getKind();
    const char *mime_type;

    if (!m_printing)
        return;

    switch (strKind) {
        case strDCT:
            mime_type = CAIRO_MIME_TYPE_JPEG;
            break;
        case strJPX:
            mime_type = CAIRO_MIME_TYPE_JP2;
            break;
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
        case strJBIG2:
            mime_type = CAIRO_MIME_TYPE_JBIG2;
            break;
#endif
        default:
            return;
    }

    str->getDict()->lookup("ColorSpace", &obj);
    colorSpace = GfxColorSpace::parse(NULL, &obj, this, state);
    obj.free();

    // colorspace in stream dict may be different from colorspace in jpx
    // data
    if (strKind == strJPX && colorSpace)
        return;

    // only embed mime data for gray, rgb, and cmyk colorspaces.
    if (colorSpace) {
        GfxColorSpaceMode mode = colorSpace->getMode();
        delete colorSpace;
        switch (mode) {
            case csDeviceGray:
            case csCalGray:
            case csDeviceRGB:
            case csCalRGB:
            case csDeviceCMYK:
            case csICCBased:
                break;

            case csLab:
            case csIndexed:
            case csSeparation:
            case csDeviceN:
            case csPattern:
                return;
        }
    }

    if (!colorMapHasIdentityDecodeMap(colorMap))
        return;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
    if (strKind == strJBIG2 && !setMimeDataForJBIG2Globals(str, image))
        return;
#endif

    if (getStreamData (str->getNextStream(), &strBuffer, &len)) {
        cairo_status_t status = CAIRO_STATUS_SUCCESS;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 2)
        if (ref && ref->isRef()) {
            status = setMimeIdFromRef(image, CAIRO_MIME_TYPE_UNIQUE_ID,
                    "poppler-surface-", ref->getRef());
        }
#endif
        if (!status) {
            status = cairo_surface_set_mime_data (image, mime_type,
                    (const unsigned char *)strBuffer, len,
                    gfree, strBuffer);
        }

        if (status)
            gfree (strBuffer);
    }
}

#include <fstream>

bool SaveImageStream(const std::string &filename, Stream *str, int widthA, int heightA, int nComps, int nBits){

    ofd::ImageDataHead imageDataHead(widthA, heightA, nComps, nBits);
    int lineSize = imageDataHead.GetLineSize();
    uint8_t *buffer = new uint8_t[lineSize];

    std::ofstream datFile;
    datFile.open(filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

    datFile.write((const char*)&imageDataHead, sizeof(ofd::ImageDataHead));
    int n = 0;
    while ( n++ < heightA ){

        int readedChars = str->doGetChars(lineSize, buffer);
        LOG(DEBUG) << "Write image data file. readedChars:" << readedChars << " lineSize:" <<  lineSize;
        if ( readedChars > 0 ){
            datFile.write((const char *)buffer, readedChars);
        }
    };
    datFile.close();

    delete buffer;

    return true;

}

void OFDOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			       int widthA, int heightA,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       int *maskColors, GBool inlineImg) {


    str->reset();

    //// -------- Write image stream to zip file. --------
    ofd::ImagePtr image = std::make_shared<ofd::Image>();
    uint64_t imageID = image->ID;

    __attribute__((unused)) std::string jpgFileName = "/tmp/Image_" + std::to_string(imageID) + ".jpg";
    __attribute__((unused)) std::string pngFileName = "/tmp/Image_" + std::to_string(imageID) + ".png";


    __attribute__((unused)) std::string imageDataFile = "/tmp/Image_" + std::to_string(imageID) + ".dat";

    int nComps = colorMap->getNumPixelComps();
    int nBits = colorMap->getBits();
    SaveImageStream(imageDataFile, str, widthA, heightA, nComps, nBits);

    //std::ofstream imgFile(strImageFileName, std::ios::binary | std::ios::out);
    //int row = 0;
    //while ( row++ < heightA ){
        //char *line = (char*)imgStream->getLine();
        //if ( line == nullptr ) break;
        //imgFile.write(line, widthA);
    //}
    //imgFile.close();
    //delete imgStream;

    // ----------------
    cairo_surface_t *imageSurface = nullptr;
    cairo_pattern_t *pattern, *maskPattern;
    cairo_matrix_t matrix;
    int width, height;
    int scaledWidth, scaledHeight;
    cairo_filter_t filter = CAIRO_FILTER_BILINEAR;
    RescaleDrawImage rescale;

    LOG(DEBUG) << "drawImage " << widthA << " x " <<  heightA;

    cairo_get_matrix(m_cairo, &matrix);
    getImageScaledSize (&matrix, widthA, heightA, &scaledWidth, &scaledHeight);
    imageSurface = rescale.getSourceImage(str, widthA, heightA, scaledWidth, scaledHeight, m_printing, colorMap, maskColors);
    if (!imageSurface)
        return;

    //writeCairoSurfaceImage(imageSurface, jpgFileName);
    cairo_surface_write_to_png(imageSurface, pngFileName.c_str());

    ofd::Document::CommonData &commonData = m_document->GetCommonData();
    assert(commonData.DocumentRes != nullptr );
    commonData.DocumentRes->AddImage(image);

    // Add image object into current page.
    ofd::ImageObject *imageObject = new ofd::ImageObject(m_currentOFDPage->GetBodyLayer());
    imageObject->SetImage(image);

    double *gfxCTM = state->getCTM();
    cairo_matrix_t matrix1;
    matrix1.xx = gfxCTM[0];
    matrix1.yx = gfxCTM[1];
    matrix1.xy = gfxCTM[2];
    matrix1.yy = gfxCTM[3];
    matrix1.x0 = gfxCTM[4];
    matrix1.y0 = gfxCTM[5];
    // FIXME
    cairo_matrix_t objMatrix = matrix1;

    imageObject->CTM[0] = objMatrix.xx;
    imageObject->CTM[1] = objMatrix.yx;
    imageObject->CTM[2] = objMatrix.xy;
    imageObject->CTM[3] = objMatrix.yy;
    imageObject->CTM[4] = objMatrix.x0;
    imageObject->CTM[5] = objMatrix.y0;

    ofd::ObjectPtr object = std::shared_ptr<ofd::Object>(imageObject);
    m_currentOFDPage->AddObject(object);

    width = cairo_image_surface_get_width (imageSurface);
    height = cairo_image_surface_get_height (imageSurface);
    if (width == widthA && height == heightA)
        filter = getFilterForSurface (imageSurface, interpolate);

    if (!inlineImg) /* don't read stream twice if it is an inline image */
        setMimeData(state, str, ref, colorMap, imageSurface);

    pattern = cairo_pattern_create_for_surface (imageSurface);
    cairo_surface_destroy (imageSurface);
    if (cairo_pattern_status (pattern))
        return;

    cairo_pattern_set_filter (pattern, filter);

    if (!m_printing)
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    cairo_matrix_init_translate (&matrix, 0, height);
    cairo_matrix_scale (&matrix, width, -height);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
        cairo_pattern_destroy (pattern);
        return;
    }

    if (!m_maskPattern && m_fillOpacity != 1.0) {
        maskPattern = cairo_pattern_create_rgba (1., 1., 1., m_fillOpacity);
    } else if ( m_maskPattern != nullptr ) {
        maskPattern = cairo_pattern_reference(m_maskPattern);
    } else {
        maskPattern = nullptr;
    }

    cairo_save(m_cairo);
    cairo_set_source(m_cairo, pattern);
    if (!m_printing)
        cairo_rectangle(m_cairo, 0., 0., 1., 1.);
    if (maskPattern != nullptr ) {
        if (!m_printing)
            cairo_clip(m_cairo);
        if ( m_maskPattern != nullptr )
            cairo_set_matrix(m_cairo, &m_mask_matrix);
        cairo_mask(m_cairo, maskPattern);
    } else {
        if (m_printing)
            cairo_paint(m_cairo);
        else
            cairo_fill(m_cairo);
    }
    cairo_restore(m_cairo);

    cairo_pattern_destroy(maskPattern);

    if ( m_cairoShape) {
        cairo_save(m_cairoShape);
        cairo_set_source(m_cairoShape, pattern);
        if ( m_printing ) {
            cairo_paint(m_cairoShape);
        } else {
            cairo_rectangle(m_cairoShape, 0., 0., 1., 1.);
            cairo_fill(m_cairoShape);
        }
        cairo_restore(m_cairoShape);
    }

    cairo_pattern_destroy (pattern);
}


//XXX: is this affect by AIS(alpha is shape)?
void OFDOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
					 int width, int height,
					 GfxImageColorMap *colorMap,
					 GBool interpolate,
					 Stream *maskStr,
					 int maskWidth, int maskHeight,
					 GfxImageColorMap *maskColorMap,
					 GBool maskInterpolate) {
    ImageStream *maskImgStr, *imgStr;
    int row_stride;
    unsigned char *maskBuffer, *buffer;
    unsigned char *maskDest;
    unsigned int *dest;
    cairo_surface_t *maskImage, *image;
    cairo_pattern_t *maskPattern, *pattern;
    cairo_matrix_t maskMatrix, matrix;
    Guchar *pix;
    int y;
    cairo_filter_t filter;
    cairo_filter_t maskFilter;

    maskImgStr = new ImageStream(maskStr, maskWidth,
            maskColorMap->getNumPixelComps(),
            maskColorMap->getBits());
    maskImgStr->reset();

    maskImage = cairo_image_surface_create (CAIRO_FORMAT_A8, maskWidth, maskHeight);
    if (cairo_surface_status (maskImage)) {
        maskImgStr->close();
        delete maskImgStr;
        return;
    }

    maskBuffer = cairo_image_surface_get_data (maskImage);
    row_stride = cairo_image_surface_get_stride (maskImage);
    for (y = 0; y < maskHeight; y++) {
        maskDest = (unsigned char *) (maskBuffer + y * row_stride);
        pix = maskImgStr->getLine();
        maskColorMap->getGrayLine (pix, maskDest, maskWidth);
    }

    maskImgStr->close();
    delete maskImgStr;

    maskFilter = getFilterForSurface (maskImage, maskInterpolate);

    cairo_surface_mark_dirty (maskImage);
    maskPattern = cairo_pattern_create_for_surface (maskImage);
    cairo_surface_destroy (maskImage);
    if (cairo_pattern_status (maskPattern))
        return;

#if 0
    /* ICCBased color space doesn't do any color correction
     * so check its underlying color space as well */
    int is_identity_transform;
    is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
        (colorMap->getColorSpace()->getMode() == csICCBased &&
         ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width,
            colorMap->getNumPixelComps(),
            colorMap->getBits());
    imgStr->reset();

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    if (cairo_surface_status (image))
        goto cleanup;

    buffer = cairo_image_surface_get_data (image);
    row_stride = cairo_image_surface_get_stride (image);
    for (y = 0; y < height; y++) {
        dest = (unsigned int *) (buffer + y * row_stride);
        pix = imgStr->getLine();
        colorMap->getRGBLine (pix, dest, width);
    }

    filter = getFilterForSurface (image, interpolate);

    cairo_surface_mark_dirty (image);

    setMimeData(state, str, ref, colorMap, image);

    pattern = cairo_pattern_create_for_surface (image);
    cairo_surface_destroy (image);
    if (cairo_pattern_status (pattern))
        goto cleanup;

    LOG(DEBUG) << "drawSoftMaskedImage " << width << " x " << height;

    cairo_pattern_set_filter (pattern, filter);
    cairo_pattern_set_filter (maskPattern, maskFilter);

    if (!m_printing) {
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
        cairo_pattern_set_extend (maskPattern, CAIRO_EXTEND_PAD);
    }

    cairo_matrix_init_translate (&matrix, 0, height);
    cairo_matrix_scale (&matrix, width, -height);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
        cairo_pattern_destroy (pattern);
        cairo_pattern_destroy (maskPattern);
        goto cleanup;
    }

    cairo_matrix_init_translate (&maskMatrix, 0, maskHeight);
    cairo_matrix_scale (&maskMatrix, maskWidth, -maskHeight);
    cairo_pattern_set_matrix (maskPattern, &maskMatrix);
    if (cairo_pattern_status (maskPattern)) {
        cairo_pattern_destroy (maskPattern);
        cairo_pattern_destroy (pattern);
        goto cleanup;
    }

    if (m_fillOpacity != 1.0)
        cairo_push_group(m_cairo);
    else
        cairo_save(m_cairo);

    cairo_set_source(m_cairo, pattern);
    if (!m_printing) {
        cairo_rectangle(m_cairo, 0., 0., 1., 1.);
        cairo_clip(m_cairo);
    }
    cairo_mask(m_cairo, maskPattern);

    if (m_fillOpacity != 1.0) {
        cairo_pop_group_to_source(m_cairo);
        cairo_save(m_cairo);
        if (!m_printing) {
            cairo_rectangle(m_cairo, 0., 0., 1., 1.);
            cairo_clip(m_cairo);
        }
        cairo_paint_with_alpha(m_cairo, m_fillOpacity);
    }
    cairo_restore(m_cairo);

    if ( m_cairoShape != nullptr ) {
        cairo_save(m_cairoShape);
        cairo_set_source(m_cairoShape, pattern);
        if (!m_printing) {
            cairo_rectangle(m_cairoShape, 0., 0., 1., 1.);
            cairo_fill(m_cairoShape);
        } else {
            cairo_mask(m_cairoShape, pattern);
        }
        cairo_restore(m_cairoShape);
    }

    cairo_pattern_destroy(maskPattern);
    cairo_pattern_destroy(pattern);

cleanup:
    imgStr->close();
    delete imgStr;
}

void OFDOutputDev::drawMaskedImage(GfxState *state, Object *ref,
				     Stream *str, int width, int height,
				     GfxImageColorMap *colorMap,
				     GBool interpolate,
				     Stream *maskStr, int maskWidth,
				     int maskHeight, GBool maskInvert,
				     GBool maskInterpolate) {
    ImageStream *maskImgStr, *imgStr;
    int row_stride;
    unsigned char *maskBuffer, *buffer;
    unsigned char *maskDest;
    unsigned int *dest;
    cairo_surface_t *maskImage, *image;
    cairo_pattern_t *maskPattern, *pattern;
    cairo_matrix_t matrix;
    cairo_matrix_t maskMatrix;
    Guchar *pix;
    int x, y;
    int invert_bit;
    cairo_filter_t filter;
    cairo_filter_t maskFilter;

    maskImgStr = new ImageStream(maskStr, maskWidth, 1, 1);
    maskImgStr->reset();

    maskImage = cairo_image_surface_create (CAIRO_FORMAT_A8, maskWidth, maskHeight);
    if (cairo_surface_status (maskImage)) {
        maskImgStr->close();
        delete maskImgStr;
        return;
    }

    maskBuffer = cairo_image_surface_get_data (maskImage);
    row_stride = cairo_image_surface_get_stride (maskImage);

    invert_bit = maskInvert ? 1 : 0;

    for (y = 0; y < maskHeight; y++) {
        pix = maskImgStr->getLine();
        maskDest = maskBuffer + y * row_stride;
        for (x = 0; x < maskWidth; x++) {
            if (pix[x] ^ invert_bit)
                *maskDest++ = 0;
            else
                *maskDest++ = 255;
        }
    }

    maskImgStr->close();
    delete maskImgStr;

    maskFilter = getFilterForSurface (maskImage, maskInterpolate);

    cairo_surface_mark_dirty (maskImage);
    maskPattern = cairo_pattern_create_for_surface (maskImage);
    cairo_surface_destroy (maskImage);
    if (cairo_pattern_status (maskPattern))
        return;

#if 0
    /* ICCBased color space doesn't do any color correction
     * so check its underlying color space as well */
    int is_identity_transform;
    is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
        (colorMap->getColorSpace()->getMode() == csICCBased && 
         ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width,
            colorMap->getNumPixelComps(),
            colorMap->getBits());
    imgStr->reset();

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    if (cairo_surface_status (image))
        goto cleanup;

    buffer = cairo_image_surface_get_data (image);
    row_stride = cairo_image_surface_get_stride (image);
    for (y = 0; y < height; y++) {
        dest = (unsigned int *) (buffer + y * row_stride);
        pix = imgStr->getLine();
        colorMap->getRGBLine (pix, dest, width);
    }

    filter = getFilterForSurface (image, interpolate);

    cairo_surface_mark_dirty (image);
    pattern = cairo_pattern_create_for_surface (image);
    cairo_surface_destroy (image);
    if (cairo_pattern_status (pattern))
        goto cleanup;

    LOG(DEBUG) << "drawMaskedImage " << width << " x " << height;

    cairo_pattern_set_filter (pattern, filter);
    cairo_pattern_set_filter (maskPattern, maskFilter);

    if (!m_printing) {
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
        cairo_pattern_set_extend (maskPattern, CAIRO_EXTEND_PAD);
    }

    cairo_matrix_init_translate (&matrix, 0, height);
    cairo_matrix_scale (&matrix, width, -height);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
        cairo_pattern_destroy (pattern);
        cairo_pattern_destroy (maskPattern);
        goto cleanup;
    }

    cairo_matrix_init_translate (&maskMatrix, 0, maskHeight);
    cairo_matrix_scale (&maskMatrix, maskWidth, -maskHeight);
    cairo_pattern_set_matrix (maskPattern, &maskMatrix);
    if (cairo_pattern_status (maskPattern)) {
        cairo_pattern_destroy (maskPattern);
        cairo_pattern_destroy (pattern);
        goto cleanup;
    }

    if (!m_printing) {
        cairo_save(m_cairo);
        cairo_set_source(m_cairo, pattern);
        cairo_rectangle(m_cairo, 0., 0., 1., 1.);
        cairo_clip(m_cairo);
        cairo_mask(m_cairo, maskPattern);
        cairo_restore(m_cairo);
    } else {
        cairo_set_source(m_cairo, pattern);
        cairo_mask(m_cairo, maskPattern);
    }

    if ( m_cairoShape != nullptr ) {
        cairo_save(m_cairoShape);
        cairo_set_source(m_cairoShape, pattern);
        if (!m_printing) {
            cairo_rectangle(m_cairoShape, 0., 0., 1., 1.);
            cairo_fill(m_cairoShape);
        } else {
            cairo_mask(m_cairoShape, pattern);
        }
        cairo_restore(m_cairoShape);
    }

    cairo_pattern_destroy (maskPattern);
    cairo_pattern_destroy (pattern);

cleanup:
    imgStr->close();
    delete imgStr;
}

void OFDOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
        int width, int height, GBool invert,
        GBool interpolate, GBool inlineImg) {

    /* FIXME: Doesn't the image mask support any colorspace? */
    cairo_set_source(m_cairo, m_fillPattern);

    /* work around a cairo bug when scaling 1x1 surfaces */
    if (width == 1 && height == 1) {
        ImageStream *imgStr;
        Guchar pix;
        int invert_bit;

        imgStr = new ImageStream(str, width, 1, 1);
        imgStr->reset();
        imgStr->getPixel(&pix);
        imgStr->close();
        delete imgStr;

        invert_bit = invert ? 1 : 0;
        if (pix ^ invert_bit)
            return;

        cairo_save(m_cairo);
        cairo_rectangle(m_cairo, 0., 0., width, height);
        cairo_fill(m_cairo);
        cairo_restore(m_cairo);
        if ( m_cairoShape != nullptr ){
            cairo_save(m_cairoShape);
            cairo_rectangle(m_cairoShape, 0., 0., width, height);
            cairo_fill(m_cairoShape);
            cairo_restore(m_cairoShape);
        }
        return;
    }

    /* shape is 1.0 for painted areas, 0.0 for unpainted ones */

    cairo_matrix_t matrix;
    cairo_get_matrix(m_cairo, &matrix);
    //XXX: it is possible that we should only do sub pixel positioning if 
    // we are rendering fonts */
    if (!m_printing && m_prescaleImages
            /* not rotated */
            && matrix.xy == 0 && matrix.yx == 0
            /* axes not flipped / not 180 deg rotated */
            && matrix.xx > 0 && (upsideDown() ? -1 : 1) * matrix.yy > 0) {
        drawImageMaskPrescaled(state, ref, str, width, height, invert, interpolate, inlineImg);
    } else {
        drawImageMaskRegular(state, ref, str, width, height, invert, interpolate, inlineImg);
    }

}


void OFDOutputDev::drawImageMaskPrescaled(GfxState *state, Object *ref, Stream *str,
					    int width, int height, GBool invert,
					    GBool interpolate, GBool inlineImg) {
    unsigned char *buffer;
    cairo_surface_t *image;
    cairo_pattern_t *pattern;
    ImageStream *imgStr;
    Guchar *pix;
    cairo_matrix_t matrix;
    int invert_bit;
    int row_stride;

    /* cairo does a very poor job of scaling down images so we scale them ourselves */

    LOG(DEBUG) << "drawImageMaskPrescaled " <<  width << " x " << height;

    /* this scaling code is adopted from the splash image scaling code */
    cairo_get_matrix(m_cairo, &matrix);
#if 0
    printf("[%f %f], [%f %f], %f %f\n", matrix.xx, matrix.xy, matrix.yx, matrix.yy, matrix.x0, matrix.y0);
#endif
    /* this whole computation should be factored out */
    double xScale = matrix.xx;
    double yScale = matrix.yy;
    int tx, tx2, ty, ty2; /* the integer co-oridinates of the resulting image */
    int scaledHeight;
    int scaledWidth;
    if (xScale >= 0) {
        tx = splashRound(matrix.x0 - 0.01);
        tx2 = splashRound(matrix.x0 + xScale + 0.01) - 1;
    } else {
        tx = splashRound(matrix.x0 + 0.01) - 1;
        tx2 = splashRound(matrix.x0 + xScale - 0.01);
    }
    scaledWidth = abs(tx2 - tx) + 1;
    //scaledWidth = splashRound(fabs(xScale));
    if (scaledWidth == 0) {
        // technically, this should draw nothing, but it generally seems
        // better to draw a one-pixel-wide stripe rather than throwing it
        // away
        scaledWidth = 1;
    }
    if (yScale >= 0) {
        ty = splashFloor(matrix.y0 + 0.01);
        ty2 = splashCeil(matrix.y0 + yScale - 0.01);
    } else {
        ty = splashCeil(matrix.y0 - 0.01);
        ty2 = splashFloor(matrix.y0 + yScale + 0.01);
    }
    scaledHeight = abs(ty2 - ty);
    if (scaledHeight == 0) {
        scaledHeight = 1;
    }
#if 0
    printf("xscale: %g, yscale: %g\n", xScale, yScale);
    printf("width: %d, height: %d\n", width, height);
    printf("scaledWidth: %d, scaledHeight: %d\n", scaledWidth, scaledHeight);
#endif

    /* compute the required padding */
    /* Padding is used to preserve the aspect ratio.
       We compute total_pad to make (height+total_pad)/scaledHeight as close to height/yScale as possible */
    int head_pad = 0;
    int tail_pad = 0;
    int total_pad = splashRound(height*(scaledHeight/fabs(yScale)) - height);

    /* compute the two pieces of padding */
    if (total_pad > 0) {
        //XXX: i'm not positive fabs() is correct
        float tail_error = fabs(matrix.y0 - ty);
        float head_error = fabs(ty2 - (matrix.y0 + yScale));
        float tail_fraction = tail_error/(tail_error + head_error);
        tail_pad = splashRound(total_pad*tail_fraction);
        head_pad = total_pad - tail_pad;
    } else {
        tail_pad = 0;
        head_pad = 0;
    }
    int origHeight = height;
    height += tail_pad;
    height += head_pad;
#if 0
    printf("head_pad: %d tail_pad: %d\n", head_pad, tail_pad);
    printf("origHeight: %d height: %d\n", origHeight, height);
    printf("ty: %d, ty2: %d\n", ty, ty2);
#endif

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width, 1, 1);
    imgStr->reset();

    invert_bit = invert ? 1 : 0;

    image = cairo_image_surface_create (CAIRO_FORMAT_A8, scaledWidth, scaledHeight);
    if (cairo_surface_status (image)) {
        imgStr->close();
        delete imgStr;
        return;
    }

    buffer = cairo_image_surface_get_data (image);
    row_stride = cairo_image_surface_get_stride (image);

    int yp = height / scaledHeight;
    int yq = height % scaledHeight;
    int xp = width / scaledWidth;
    int xq = width % scaledWidth;
    int yt = 0;
    int origHeight_c = origHeight;
    /* use MIN() because yp might be > origHeight because of padding */
    unsigned char *pixBuf = (unsigned char *)malloc(std::min(yp+1, origHeight)*width);
    int lastYStep = 1;
    int total = 0;
    for (int y = 0; y < scaledHeight; y++) {
        // y scale Bresenham
        int yStep = yp;
        yt += yq;

        if (yt >= scaledHeight) {
            yt -= scaledHeight;
            ++yStep;
        }

        // read row (s) from image ignoring the padding as appropriate
        {
            int n = (yp > 0) ? yStep : lastYStep;
            total += n;
            if (n > 0) {
                unsigned char *p = pixBuf;
                int head_pad_count = head_pad;
                int origHeight_count = origHeight;
                int tail_pad_count = tail_pad;
                for (int i=0; i<n; i++) {
                    // get row
                    if (head_pad_count) {
                        head_pad_count--;
                    } else if (origHeight_count) {
                        pix = imgStr->getLine();
                        for (int j=0; j<width; j++) {
                            if (pix[j] ^ invert_bit)
                                p[j] = 0;
                            else
                                p[j] = 255;
                        }
                        origHeight_count--;
                        p += width;
                    } else if (tail_pad_count) {
                        tail_pad_count--;
                    } else {
                        printf("%d %d\n", n, total);
                        assert(0 && "over run\n");
                    }
                }
            }
        }

        lastYStep = yStep;
        int k1 = y;

        int xt = 0;
        int xSrc = 0;
        int x1 = k1;
        int n = yStep > 0 ? yStep : 1;
        int origN = n;

        /* compute the size of padding and pixels that will be used for this row */
        int head_pad_size = std::min(n, head_pad);
        n -= head_pad_size;
        head_pad -= std::min(head_pad_size, yStep);

        int pix_size = std::min(n, origHeight);
        n -= pix_size;
        origHeight -= std::min(pix_size, yStep);

        int tail_pad_size = std::min(n, tail_pad);
        n -= tail_pad_size;
        tail_pad -= std::min(tail_pad_size, yStep);
        if (n != 0) {
            printf("n = %d (%d %d %d)\n", n, head_pad_size, pix_size, tail_pad_size);
            assert(n == 0);
        }

        for (int x = 0; x < scaledWidth; ++x) {
            int xStep = xp;
            xt += xq;
            if (xt >= scaledWidth) {
                xt -= scaledWidth;
                ++xStep;
            }
            int m = xStep > 0 ? xStep : 1;
            float pixAcc0 = 0;
            /* could m * head_pad_size * tail_pad_size  overflow? */
            if (invert_bit) {
                pixAcc0 += m * head_pad_size * tail_pad_size * 255;
            } else {
                pixAcc0 += m * head_pad_size * tail_pad_size * 0;
            }
            /* Accumulate all of the source pixels for the destination pixel */
            for (int i = 0; i < pix_size; ++i) {
                for (int j = 0; j< m; ++j) {
                    if (xSrc + i*width + j > std::min(yp + 1, origHeight_c)*width) {
                        printf("%d > %d (%d %d %d %d) (%d %d %d)\n", xSrc + i*width + j, std::min(yp + 1, origHeight_c)*width, xSrc, i , width, j, yp, origHeight_c, width);
                        printf("%d %d %d\n", head_pad_size, pix_size, tail_pad_size);
                        assert(0 && "bad access\n");
                    }
                    pixAcc0 += pixBuf[xSrc + i*width + j];
                }
            }
            buffer[y * row_stride + x] = splashFloor(pixAcc0 / (origN*m));
            xSrc += xStep;
            x1 += 1;
        }

    }
    free(pixBuf);

    cairo_surface_mark_dirty (image);
    pattern = cairo_pattern_create_for_surface (image);
    cairo_surface_destroy (image);
    if (cairo_pattern_status (pattern)) {
        imgStr->close();
        delete imgStr;
        return;
    }

    /* we should actually be using CAIRO_FILTER_NEAREST here. However,
     * cairo doesn't yet do minifaction filtering causing scaled down
     * images with CAIRO_FILTER_NEAREST to look really bad */
    cairo_pattern_set_filter (pattern,
            interpolate ? CAIRO_FILTER_BEST : CAIRO_FILTER_FAST);

    if (state->getFillColorSpace()->getMode() == csPattern) {
        cairo_matrix_init_translate (&matrix, 0, scaledHeight);
        cairo_matrix_scale (&matrix, scaledWidth, -scaledHeight);
        cairo_pattern_set_matrix (pattern, &matrix);
        if (cairo_pattern_status (pattern)) {
            cairo_pattern_destroy (pattern);
            imgStr->close();
            delete imgStr;
            return;
        }

        m_maskPattern = cairo_pattern_reference (pattern);
        cairo_get_matrix(m_cairo, &m_mask_matrix);
    } else {
        cairo_save(m_cairo);

        /* modify our current transformation so that the prescaled image
         * goes where it is supposed to */
        cairo_get_matrix(m_cairo, &matrix);
        cairo_scale(m_cairo, 1.0/matrix.xx, 1.0/matrix.yy);
        // get integer co-ords
        cairo_translate(m_cairo, tx - matrix.x0, ty2 - matrix.y0);
        if (yScale > 0)
            cairo_scale(m_cairo, 1, -1);

        cairo_rectangle(m_cairo, 0., 0., scaledWidth, scaledHeight);
        cairo_clip(m_cairo);
        if (m_strokePathClip != nullptr) {
            cairo_push_group(m_cairo);
            fillToStrokePathClip (state);
            cairo_pop_group_to_source(m_cairo);
        }
        cairo_mask(m_cairo, pattern);

        //cairo_get_matrix(cairo, &matrix);
        //printf("mask at: [%f %f], [%f %f], %f %f\n\n", matrix.xx, matrix.xy, matrix.yx, matrix.yy, matrix.x0, matrix.y0);
        cairo_restore(m_cairo);
    }

    if (m_cairoShape) {
        cairo_save(m_cairoShape);

        /* modify our current transformation so that the prescaled image
         * goes where it is supposed to */
        cairo_get_matrix(m_cairoShape, &matrix);
        cairo_scale(m_cairoShape, 1.0/matrix.xx, 1.0/matrix.yy);
        // get integer co-ords
        cairo_translate(m_cairoShape, tx - matrix.x0, ty2 - matrix.y0);
        if (yScale > 0)
            cairo_scale(m_cairoShape, 1, -1);

        cairo_rectangle(m_cairoShape, 0., 0., scaledWidth, scaledHeight);
        cairo_fill(m_cairoShape);

        cairo_restore(m_cairoShape);
    }

    cairo_pattern_destroy(pattern);

    imgStr->close();
    delete imgStr;
}

void OFDOutputDev::drawImageMaskRegular(GfxState *state, Object *ref, Stream *str,
					  int width, int height, GBool invert,
					  GBool interpolate, GBool inlineImg) {
    unsigned char *buffer;
    unsigned char *dest;
    cairo_surface_t *image;
    cairo_pattern_t *pattern;
    int x, y, i, bit;
    ImageStream *imgStr;
    Guchar *pix;
    cairo_matrix_t matrix;
    int invert_bit;
    int row_stride;
    cairo_filter_t filter;

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width, 1, 1);
    imgStr->reset();

    image = cairo_image_surface_create (CAIRO_FORMAT_A1, width, height);
    if (cairo_surface_status (image))
        goto cleanup;

    buffer = cairo_image_surface_get_data (image);
    row_stride = cairo_image_surface_get_stride (image);

    invert_bit = invert ? 1 : 0;

    for (y = 0; y < height; y++) {
        pix = imgStr->getLine();
        dest = buffer + y * row_stride;
        i = 0;
        bit = 0;
        for (x = 0; x < width; x++) {
            if (bit == 0)
                dest[i] = 0;
            if (!(pix[x] ^ invert_bit)) {
#ifdef WORDS_BIGENDIAN
                dest[i] |= (1 << (7 - bit));
#else
                dest[i] |= (1 << bit);
#endif
            }
            bit++;
            if (bit > 7) {
                bit = 0;
                i++;
            }
        }
    }

    filter = getFilterForSurface (image, interpolate);

    cairo_surface_mark_dirty (image);
    pattern = cairo_pattern_create_for_surface (image);
    cairo_surface_destroy (image);
    if (cairo_pattern_status (pattern))
        goto cleanup;

    LOG(DEBUG) << "drawImageMask " << width << " x " << height;

    cairo_pattern_set_filter (pattern, filter);

    if (!m_printing)
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

    cairo_matrix_init_translate (&matrix, 0, height);
    cairo_matrix_scale (&matrix, width, -height);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
        cairo_pattern_destroy (pattern);
        goto cleanup;
    }

    if (state->getFillColorSpace()->getMode() == csPattern) {
        m_maskPattern = cairo_pattern_reference(pattern);
        cairo_get_matrix(m_cairo, &m_mask_matrix);
    } else if (!m_printing) {
        cairo_save (m_cairo);
        cairo_rectangle (m_cairo, 0., 0., 1., 1.);
        cairo_clip(m_cairo);
        cairo_mask(m_cairo, pattern);
        cairo_restore(m_cairo);
    } else {
        cairo_mask(m_cairo, pattern);
    }

    if ( m_cairoShape) {
        cairo_save (m_cairoShape);
        cairo_set_source (m_cairoShape, pattern);
        if (!m_printing) {
            cairo_rectangle (m_cairoShape, 0., 0., 1., 1.);
            cairo_fill (m_cairoShape);
        } else {
            cairo_mask (m_cairoShape, pattern);
        }
        cairo_restore (m_cairoShape);
    }

    cairo_pattern_destroy(pattern);

cleanup:
    imgStr->close();
    delete imgStr;
}

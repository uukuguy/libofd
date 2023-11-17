#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <cairo/cairo.h>
#include "ofd/Package.h"
#include "ofd/Image.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace ofd;

namespace ofd{
    std::string generateImageFileName(uint64_t imageID){
        char buf[1024];
        sprintf(buf, "Image_%" PRIu64 ".png", imageID);
        LOG(DEBUG) << "------- generateImageFileName() imageID:" << imageID << " filename=" << std::string(buf);
        return std::string(buf);
    }
}


#include <png.h>
// **************** class ofd::PNGStream ****************
class PNGStream{

    public:
        PNGStream();
        virtual ~PNGStream();

        bool Open(const std::string pngFileName);
        void Close();
        void Reset();

        inline int DoGetChars(int nChars, uint8_t *buffer);


}; // class PNGStream

class ImageSource {
    public:
        ImageSource(char *dataA, size_t dataSizeA) : data(dataA), dataSize(dataSizeA), offset(0){};

        char *data;
        size_t dataSize;
        size_t offset;
};

static void pngReadCallback(png_structp png_ptr, png_byte *data, png_size_t length){
    ImageSource *imageSource = (ImageSource*)png_get_io_ptr(png_ptr);
    if ( imageSource->offset + length <= imageSource->dataSize ){
        memcpy((void*)data, (void*)(imageSource->data + imageSource->offset), length);
        imageSource->offset += length;
    } else {
        png_error(png_ptr, "pngReadCallback() failed.");
    }
}

PNGStream::PNGStream(){
}

PNGStream::~PNGStream(){
}

void PNGStream::Reset(){
}

void PNGStream::Close(){
}

std::tuple<ImageDataHead, char*, size_t> LoadPNGData(char* data, size_t dataSize){
    ImageDataHead imageDataHead;
    char *imageData = nullptr;
    size_t imageDataSize = 0; 

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if ( png_ptr != 0 ){
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if ( info_ptr != 0 ){
            if ( setjmp(png_jmpbuf(png_ptr)) == 0 ){
                ImageSource imageSource(data, dataSize);
                
                png_set_read_fn(png_ptr, (png_voidp)&imageSource, pngReadCallback);
                png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND /*| PNG_TRANSFORM_STRIP_ALPHA*/, 0);
                // 图像的色彩类型
                int color_type = png_get_color_type(png_ptr, info_ptr);
                // 图像的宽高
                int w = png_get_image_width(png_ptr, info_ptr);
                int h = png_get_image_height(png_ptr, info_ptr);
                imageDataHead.Width = w;
                imageDataHead.Height = h;
                imageDataHead.Components = 4;
                imageDataHead.Bits = 8;

                imageDataSize = (size_t)w * (size_t)h * 4;
                imageData = (char*)new char[imageDataSize];
                size_t idx = 0;

                // 获得图像的所有行像素数据
                png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
                switch ( color_type ){
                    case PNG_COLOR_TYPE_RGB_ALPHA:

                        for ( int y = 0 ; y < h ; ++y ){
                            for ( int x = 0 ; x < w * 4; ){
                                __attribute__((unused)) uint8_t red = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t green = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t blue = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t alpha = row_pointers[y][x++];
                                imageData[idx++] = red;
                                imageData[idx++] = green;
                                imageData[idx++] = blue;
                                imageData[idx++] = alpha;
                            }
                        }
                        break;

                    case PNG_COLOR_TYPE_RGB:

                        for ( int y = 0 ; y < h ; ++y ){
                            for ( int x = 0 ; x < w * 3; ){
                                __attribute__((unused)) uint8_t red = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t green = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t blue = row_pointers[y][x++];
                                __attribute__((unused)) uint8_t alpha = 255;

                                imageData[idx++] = red;
                                imageData[idx++] = green;
                                imageData[idx++] = blue;
                                imageData[idx++] = alpha;
                            }
                        }
                        break;

                    default:
                        break;

                }; // switch ( color_type )

            }
            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        } else {
            png_destroy_read_struct(&png_ptr, 0, 0);
        }
    }

    return std::make_tuple(imageDataHead, imageData, imageDataSize);
}

int PNGStream::DoGetChars(int nChars, uint8_t *buffer){
    int dataSize = 0;
    return dataSize;
}

// **************** class ofd::Image ****************

static uint64_t IMAGE_ID = 1;

Image::Image():
    ID(IMAGE_ID++),
    width(0), height(0),
    nComps(0), nBits(0), nVals(0),
    inputLineSize(0), inputLine(nullptr),
    imgLine(nullptr), imgIdx(0),
    m_bLoaded(false), m_imageData(nullptr), m_imageDataSize(0){
}

Image::Image(int widthA, int heightA, int nCompsA, int nBitsA) :
    ID(IMAGE_ID++),
    width(widthA), height(heightA),
    nComps(nCompsA), nBits(nBitsA), nVals(0),
    inputLineSize(0), inputLine(nullptr),
    imgLine(nullptr), imgIdx(0),
    m_bLoaded(false){

    int imgLineSize = 0;
    nVals = width * nComps;
    inputLineSize = (nVals * nBits + 7) >> 3;
    if (nBits <= 0 || nVals > INT_MAX / nBits - 7 || width > INT_MAX / nComps) {
        inputLineSize = -1;
    }
    inputLine = (uint8_t *)new uint8_t(inputLineSize* sizeof(uint8_t));
    if (nBits == 8) {
        imgLine = (uint8_t *)inputLine;
    } else {
        if (nBits == 1) {
            imgLineSize = (nVals + 7) & ~7;
        } else {
            imgLineSize = nVals;
        }
        if (width > INT_MAX / nComps) {
            // force a call to gmallocn(-1,...), which will throw an exception
            imgLineSize = -1;
        }
        imgLine = (uint8_t *)new uint8_t(imgLineSize *  sizeof(uint8_t));
    }
    imgIdx = nVals;
}

Image::~Image(){
    if ( imgLine != (uint8_t *)inputLine ) {
        delete imgLine;
    }
    delete inputLine;

    if ( m_imageData != nullptr ){
        delete m_imageData;
        m_imageData = nullptr;
    }
}

std::string Image::GenerateImageFileName(){
    return generateImageFileName(ID);
}

bool Image::Load(PackagePtr package, bool reload){
    if ( m_bLoaded && !reload ) return true;

    bool ok = false;

    std::string imageFilePath = m_imageFilePath;
    LOG(DEBUG) << "Load Image: " << imageFilePath;

    char *imageData = nullptr;
    size_t imageDataSize = 0;
    bool readOK = false;

    // FIXME
    char *pngData = nullptr;
    size_t pngDataSize = 0;
    std::tie(pngData, pngDataSize, readOK) = package->ReadZipFileRaw(imageFilePath);
    if ( pngDataSize > 0 ){
        ImageDataHead imageDataHead;
        std::tie(imageDataHead, imageData, imageDataSize) = LoadPNGData(pngData, pngDataSize);
        if ( imageDataSize > 0 ){
            width = imageDataHead.Width;
            height = imageDataHead.Height;
            nComps = imageDataHead.Components;
            nBits = imageDataHead.Bits;
            readOK = true;
        } else {
            readOK = false;
        }
    }

    // ----------
    // FIXME
    // Load from tmp image data
    //imageFilePath = "/tmp/Image_" + std::to_string(ID) + ".dat";
    //char *fileData = nullptr;
    //size_t fileSize = 0;
    //std::tie(fileData, fileSize, readOK) = utils::ReadFileData(imageFilePath);

    //ImageDataHead *imageDataHead = (ImageDataHead*)fileData;
    //width = imageDataHead->Width;
    //height = imageDataHead->Height;
    //nComps = imageDataHead->Components;
    //nBits = imageDataHead->Bits;

    //imageDataSize = fileSize - sizeof(ImageDataHead);
    //imageData = new char[imageDataSize];
    //memcpy(imageData, &fileData[sizeof(ImageDataHead)], imageDataSize);

    //delete fileData;
    // ----------


    if ( readOK ){
        m_imageData = imageData;
        m_imageDataSize = imageDataSize;
        ok = true;
    } else {
        ok = false;
        LOG(ERROR) << "Call ReadZipFileRaw() to read image file " << imageFilePath << " failed.";
    }

    m_bLoaded = ok;

    return m_bLoaded;
}

void Image::Reset(){
}

void Image::Close(){
}

bool Image::GetPixel(uint8_t *pix){

    if (imgIdx >= nVals) {
        if (!GetLine()) {
            return false;
        }
        imgIdx = 0;
    }
    for (int i = 0; i < nComps; ++i) {
        pix[i] = imgLine[imgIdx++];
    }
    return true;
}

uint8_t *Image::GetLine(){
    uint64_t buf, bitMask;

    if ( unlikely(inputLine == nullptr) ) return nullptr;

    int readChars = 0;
    //int readChars = str->doGetChars(inputLineSize, inputLine);

    for ( ; readChars < inputLineSize; readChars++) {
        inputLine[readChars] = EOF;
    }

    uint8_t *p;
    if (nBits == 1) {
        p = inputLine;
        for (int i = 0; i < nVals; i += 8) {
            int c = *p++;
            imgLine[i+0] = (uint8_t)((c >> 7) & 1);
            imgLine[i+1] = (uint8_t)((c >> 6) & 1);
            imgLine[i+2] = (uint8_t)((c >> 5) & 1);
            imgLine[i+3] = (uint8_t)((c >> 4) & 1);
            imgLine[i+4] = (uint8_t)((c >> 3) & 1);
            imgLine[i+5] = (uint8_t)((c >> 2) & 1);
            imgLine[i+6] = (uint8_t)((c >> 1) & 1);
            imgLine[i+7] = (uint8_t)(c & 1);
        }
    } else if (nBits == 8) {
        // special case: imgLine == inputLine
    } else if (nBits == 16) {
        // this is a hack to support 16 bits images, everywhere
        // we assume a component fits in 8 bits, with this hack
        // we treat 16 bit images as 8 bit ones until it's fixed correctly.
        // The hack has another part on GfxImageColorMap::GfxImageColorMap
        p = inputLine;
        for (int i = 0; i < nVals; ++i) {
            imgLine[i] = *p++;
            p++;
        }
    } else {
        bitMask = (1 << nBits) - 1;
        buf = 0;
        int bits = 0;
        p = inputLine;
        for (int i = 0; i < nVals; ++i) {
            if (bits < nBits) {
                buf = (buf << 8) | (*p++ & 0xff);
                bits += 8;
            }
            imgLine[i] = (uint8_t)((buf >> (bits - nBits)) & bitMask);
            bits -= nBits;
        }
    }

    return imgLine;
}

void Image::SkipLine(){
}

void Image::GenerateXML(utils::XMLWriter &writer) const{
    writer.StartElement("Image");{
        // -------- <Font ID="">
        writer.WriteAttribute("ID", ID);
    }; writer.EndElement();

}

bool Image::FromXML(utils::XMLElementPtr imageElement){
    bool ok = false;
    ImagePtr image = nullptr;

    std::string childName = imageElement->GetName();

    // -------- <Font>
    // OFD (section 11.1) P61. Res.xsd.
    if ( childName == "Image" ){
        bool exist = false;

        // -------- <Image FontName="">
        // Required.
        std::tie(ID, exist) = imageElement->GetIntAttribute("ID");
        if ( exist ){
            // -------- <Image ="">
            // Required.

        } else {
            LOG(ERROR) << "Attribute ID is required in Image XML.";
        }

        if ( exist ){
            ok = true;
        } else {
            ok = false;
        }
    }

    return ok;
}

namespace ofd{

/* Taken from cairo/doc/tutorial/src/singular.c */
static void get_singular_values (const cairo_matrix_t *matrix, double *major, double *minor){
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

void getImageScaledSize(const cairo_matrix_t *matrix,
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

cairo_filter_t getFilterForSurface(cairo_surface_t *image, cairo_t *cr, bool interpolate) {
    if (interpolate)
        return CAIRO_FILTER_BILINEAR;

    int orig_width = cairo_image_surface_get_width(image);
    int orig_height = cairo_image_surface_get_height(image);
    if (orig_width == 0 || orig_height == 0)
        return CAIRO_FILTER_NEAREST;

    /* When printing, don't change the interpolation. */
    //if ( m_printing )
    //return CAIRO_FILTER_NEAREST;

    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);
    int scaled_width, scaled_height;
    getImageScaledSize(&matrix, orig_width, orig_height, &scaled_width, &scaled_height);

    /* When scale factor is >= 400% we don't interpolate. See bugs #25268, #9860 */
    if (scaled_width / orig_width >= 4 || scaled_height / orig_height >= 4)
        return CAIRO_FILTER_NEAREST;

    return CAIRO_FILTER_BILINEAR;
}

} // namespace ofd

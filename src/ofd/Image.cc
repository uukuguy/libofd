#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include "ofd/Package.h"
#include "ofd/Image.h"
#include "utils/logger.h"

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
        ImageSource() : data(nullptr), dataSize(0), offset(0){};

        uint8_t *data;
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

int PNGStream::DoGetChars(int nChars, uint8_t *buffer){
    int dataSize = 0;
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if ( png_ptr != 0 ){
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if ( info_ptr != 0 ){
            if ( setjmp(png_jmpbuf(png_ptr)) == 0 ){
                ImageSource imageSource;
                // imageSource.data = 
                // imageSource.dataSize = 
                
                png_set_read_fn(png_ptr, (png_voidp)&imageSource, pngReadCallback);
                png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND /*| PNG_TRANSFORM_STRIP_ALPHA*/, 0);
                // 图像的色彩类型
                int color_type = png_get_color_type(png_ptr, info_ptr);
                // 图像的宽高
                int w = png_get_image_width(png_ptr, info_ptr);
                int h = png_get_image_height(png_ptr, info_ptr);
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

bool Image::Load(PackagePtr package, bool reload){
    if ( m_bLoaded && !reload ) return true;

    bool ok = false;

    std::string imageFilePath = m_imageFilePath;
    LOG(DEBUG) << "Load Image: " << imageFilePath;

    char *imageData = nullptr;
    size_t imageDataSize = 0;
    bool readOK = false;

    // FIXME
    //std::tie(imageData, imageDataSize, readOK) = package->ReadZipFileRaw(imageFilePath);

    imageFilePath = "/tmp/Image_" + std::to_string(ID) + ".dat";
    char *fileData = nullptr;
    size_t fileSize = 0;
    std::tie(fileData, fileSize, readOK) = utils::ReadFileData(imageFilePath);

    ImageDataHead *imageDataHead = (ImageDataHead*)fileData;
    width = imageDataHead->Width;
    height = imageDataHead->Height;
    nComps = imageDataHead->Components;
    nBits = imageDataHead->Bits;

    imageDataSize = fileSize - sizeof(ImageDataHead);
    imageData = new char[imageDataSize];
    memcpy(imageData, &fileData[sizeof(ImageDataHead)], imageDataSize);

    delete fileData;

    if ( readOK ){
        m_imageData = imageData;
        m_imageDataSize = imageDataSize;
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

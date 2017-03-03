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
    ID(0),
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
    std::tie(imageData, imageDataSize, readOK) = package->ReadZipFileRaw(imageFilePath);
    if ( readOK ){

    } else {
        ok = false;
        LOG(ERROR) << "Call ReadZipFileRaw() to read image file " << imageFilePath << " failed.";
    }

    m_bLoaded = ok;

    return m_bLoaded;
}

#ifndef __OFD_IMAGE_H__
#define __OFD_IMAGE_H__

#include <string>
#include <cairo/cairo.h>
#include "ofd/Common.h"

namespace ofd{

    typedef float SplashCoord;

    static inline int splashRound(SplashCoord x) {
        return (int)floor(x + 0.5);
    }

    static inline int splashCeil(SplashCoord x) {
        return (int)ceil(x);
    }

    static inline int splashFloor(SplashCoord x) {
        return (int)floor(x);
    }

    // Defined in Image.cc
    void getImageScaledSize(const cairo_matrix_t *matrix,
            int                   orig_width,
            int                   orig_height,
            int                  *scaledWidth,
            int                  *scaledHeight); 

    // Defined in Image.cc
    cairo_filter_t getFilterForSurface(cairo_surface_t *image, cairo_t *cr, bool interpolate); 

    typedef struct ImageDataHead{
        int Width;
        int Height;
        int Components;
        int Bits;

        ImageDataHead(int width = 0, int height = 0, int comps = 0, int bits = 0) :
            Width(width), Height(height), Components(comps), Bits(bits)
        {
        }

        int GetLineSize() const{
            int nVals = Width * Components;
            int lineSize = (nVals * Bits + 7) >> 3;
            return lineSize;
        }
    } ImageDataHead_t;

    std::string generateImageFileName(uint64_t imageID);

    class Image {
        public:
            Image();
            Image(int widthA, int heightA, int nCompsA, int nBitsA); 
            ~Image();

            // =============== Public Attributes ================
        public:
            uint64_t ID;
            int width;          // pixels
            int height;         // pixels
            int nComps;         // components per pixel
            int nBits;	        // bits per component
            int nVals;	        // components per line
            int inputLineSize;  // input line buffer size
            uint8_t *inputLine; // input line buffer
            uint8_t *imgLine;   // line buffer
            int imgIdx;	        // current index in imgLine

            //std::string ImageFile;
            // =============== Public Methods ================
        public:
            bool Load(PackagePtr package, bool reload = false);

            // Reset the stream.
            void Reset();

            // Close the stream previously reset
            void Close();

            // Gets the next pixel from the stream.  <pix> should be able to hold
            // at least nComps elements.  Returns false at end of file.
            bool GetPixel(uint8_t *pix);

            // Returns a pointer to the next line of pixels.  Returns NULL at
            // end of file.
            uint8_t *GetLine();

            // Skip an entire line from the image.
            void SkipLine();

            void GenerateXML(utils::XMLWriter &writer) const;
            bool FromXML(utils::XMLElementPtr imageElement);
            std::string GenerateImageFileName();

            // ---------------- Private Attributes ----------------
        public:
            const char *GetImageData() const {return m_imageData;};
            char *GetImageData(){return m_imageData;};
            size_t GetImageDataSize() const {return m_imageDataSize;};
            void SetImageFilePath(const std::string &imageFilePath){m_imageFilePath = imageFilePath;};
            std::string GetImageFilePath() const {return m_imageFilePath;};

        private:
            bool m_bLoaded;
            char *m_imageData;
            size_t m_imageDataSize;
            std::string m_imageFilePath;

    }; // class Image

}; // namespace ofd

#endif // __OFD_IMAGE_H__

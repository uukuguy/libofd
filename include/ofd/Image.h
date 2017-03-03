#ifndef __OFD_IMAGE_H__
#define __OFD_IMAGE_H__

#include <string>
#include "ofd/Common.h"

namespace ofd{

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

            std::string ImageFile;
            // =============== Public Methods ================
        public:
            bool Load(PackagePtr package, bool reload);

            // ---------------- Private Attributes ----------------
        public:
            const char *GetImageData() const {return m_imageData;};
            size_t GetImageDataSize() const {return m_imageDataSize;};
            void SetImageFilePaht(const std::string &imageFilePath){m_imageFilePath = imageFilePath;};
            std::string GetImageFilePath() const {return m_imageFilePath;};

        private:
            bool m_bLoaded;
            char *m_imageData;
            size_t m_imageDataSize;
            std::string m_imageFilePath;

    }; // class Image

}; // namespace ofd

#endif // __OFD_IMAGE_H__

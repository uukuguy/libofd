#ifndef __OFD_IMAGEOBJECT_H__
#define __OFD_IMAGEOBJECT_H__

#include <memory>
#include "ofd/Object.h"
#include "ofd/Color.h"

namespace ofd{

    class ImageBlock{
        public:
            ImageBlock(int widthA, int heightA, int nCompsA, int nBitsA); 
            virtual ~ImageBlock();

            // =============== Public Attributes ================
        public:
            int width;          // pixels
            int height;         // pixels
            int nComps;         // components per pixel
            int nBits;	        // bits per component
            int nVals;	        // components per line
            int inputLineSize;  // input line buffer size
            uint8_t *inputLine; // input line buffer
            uint8_t *imgLine;   // line buffer
            int imgIdx;	        // current index in imgLine
    }; // class ImageBlock

    typedef struct ImageBorder{
        ImageBorder() : 
            LineWidth(0.353), HorizonalCornerRadius(0.0), VerticalCornerRadius(0.0), DashOffset(0.0),
            BorderColor(COLOR_BLACK)
        {
        }

        double LineWidth;
        double HorizonalCornerRadius;
        double VerticalCornerRadius;
        double DashOffset;
        //FIXME
        //CT_Array DashPattern;

        ColorPtr BorderColor;
    } ImageBorder_t; // 

    // ======== class ImageObject ========
    // OFD P52，Page.xsd.
    class ImageObject : public Object{
        public:

            ImageObject(LayerPtr layer);
            virtual ~ImageObject();

            // =============== Public Attributes ================
        public:
            uint64_t ResourceID;
            uint64_t Substitution;
            uint64_t ImageMask;

            ImageBorder Border;

        protected:
            virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
            virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

            virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
            virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    }; // class OFDImageObject
    typedef std::shared_ptr<ImageObject> ImageObjectPtr;

}; // namespace ofd

#endif // __OFD_IMAGEOBJECT_H__

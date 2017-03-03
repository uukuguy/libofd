#ifndef __OFD_IMAGEOBJECT_H__
#define __OFD_IMAGEOBJECT_H__

#include <memory>
#include "ofd/Object.h"
#include "ofd/Common.h"

namespace ofd{

    typedef struct ImageBorder{
        ImageBorder(); 

        double LineWidth;
        double HorizonalCornerRadius;
        double VerticalCornerRadius;
        double DashOffset;
        //FIXME
        //CT_Array DashPattern;

        ColorPtr BorderColor;
    } ImageBorder_t; 

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

            // =============== Public Methods ================
        public:

        protected:
            virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
            virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

            virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
            virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

            // ---------------- Private Attributes ----------------
        public:
            const ImagePtr GetImage() const {return m_image;};
            ImagePtr GetImage() {return m_image;};
            void SetImage(ImagePtr image);

        private:
            ImagePtr m_image;

    }; // class OFDImageObject
    typedef std::shared_ptr<ImageObject> ImageObjectPtr;

}; // namespace ofd

#endif // __OFD_IMAGEOBJECT_H__

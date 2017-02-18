#ifndef __OFD_IMAGEOBJECT_H__
#define __OFD_IMAGEOBJECT_H__

#include <memory>
#include "ofd/Object.h"

namespace ofd{

    // ======== class ImageObject ========
    // OFD P52，Page.xsd.
    class ImageObject : public Object{
    public:

        ImageObject(LayerPtr layer);
        virtual ~ImageObject();

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    }; // class OFDImageObject
    typedef std::shared_ptr<ImageObject> ImageObjectPtr;

}; // namespace ofd

#endif // __OFD_IMAGEOBJECT_H__

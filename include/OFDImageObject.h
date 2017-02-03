#ifndef __OFD_IMAGEOBJECT_H__
#define __OFD_IMAGEOBJECT_H__

#include <memory>
#include "OFDObject.h"

namespace ofd{


    // ======== class OFDImageObject ========
    // OFD P52，Page.xsd.
    class OFDImageObject : public OFDObject{
    public:

        OFDImageObject(OFDPagePtr page);
        virtual ~OFDImageObject();

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDImageObject
    typedef std::shared_ptr<OFDImageObject> OFDImageObjectPtr;

}; // namespace ofd

#endif // __OFD_IMAGEOBJECT_H__

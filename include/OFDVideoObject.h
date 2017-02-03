#ifndef __OFD_VIDEOOBJECT_H__
#define __OFD_VIDEOOBJECT_H__

#include <memory>
#include "OFDObject.h"

namespace ofd{


    // ======== class OFDVideoObject ========
    // OFD P71，Page.xsd.
    class OFDVideoObject : public OFDObject{
    public:

        OFDVideoObject(OFDPagePtr page);
        virtual ~OFDVideoObject();

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDVideoObject
    typedef std::shared_ptr<OFDVideoObject> OFDVideoObjectPtr;

}; // namespace ofd

#endif // __OFD_VIDEOOBJECT_H__

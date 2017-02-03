#ifndef __OFD_COMPOSITEOBJECT_H__
#define __OFD_COMPOSITEOBJECT_H__

#include <memory>
#include "OFDObject.h"

namespace ofd{


    // ======== class OFDCompositeObject ========
    // OFD P52，Page.xsd.
    class OFDCompositeObject : public OFDObject{
    public:

        OFDCompositeObject(OFDPagePtr page);
        virtual ~OFDCompositeObject();

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDCompositeObject
    typedef std::shared_ptr<OFDCompositeObject> OFDCompositeObjectPtr;

}; // namespace ofd

#endif // __OFD_COMPOSITEOBJECT_H__

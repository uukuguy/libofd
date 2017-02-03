#ifndef __OFD_PATHOBJECT_H__
#define __OFD_PATHOBJECT_H__

#include <memory>
#include "OFDObject.h"
#include "ofd/OfdPath.h"

namespace ofd{


    namespace Path{

        typedef struct Path{
        public:

        } *Path_t;

    }; // namespace ofd::Path

    // ======== class OFDPathObject ========
    // OFD P52，Page.xsd.
    class OFDPathObject : public OFDObject{
    public:

        OFDPathObject(OFDPagePtr page);
        virtual ~OFDPathObject();

        OfdPathPtr GetPath() const;
        void SetPath(OfdPathPtr path);

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDPathObject
    typedef std::shared_ptr<OFDPathObject> OFDPathObjectPtr;

}; // namespace ofd

#endif // __OFD_PATHOBJECT_H__

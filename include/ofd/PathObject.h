#ifndef __OFD_PATHOBJECT_H__
#define __OFD_PATHOBJECT_H__

#include <memory>
#include "ofd/Object.h"
#include "ofd/Path.h"
#include "ofd/Common.h"

namespace ofd{

    class PathObject;
    typedef std::shared_ptr<PathObject> PathObjectPtr;

    // ======== class PathObject ========
    // OFD P52，Page.xsd.
    class PathObject : public Object{
    public:

        PathObject(LayerPtr layer);
        virtual ~PathObject();

        const PathPtr GetPath() const {return m_path;};
        PathPtr GetPath() {return m_path;};
        void SetPath(PathPtr path){m_path = path;};

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;

    private:
        PathPtr m_path;

    }; // class OFDPathObject

}; // namespace ofd

#endif // __OFD_PATHOBJECT_H__

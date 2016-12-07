#ifndef __OFD_OBJECT_H__
#define __OFD_OBJECT_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "OFDColor.h"

namespace ofd{

    namespace Object{
        enum class Type{
            UNKNOWN = -1, // 未知对象
            TEXT,         // 文本对象
            PATH,         // 图形对象
            IMAGE,        // 图像对象
            COMPOSITE,    // 复合对象
        };
    }; // namespace Object

    // ======== class OFDObject ========
    class OFDObject {
    public:
        OFDObject();
        virtual ~OFDObject();

        uint64_t     ID;
        Object::Type Type;
        ST_Box       Boundary; 
        std::string  Name;
        bool         Visible;
        double       CTM[6];
        double       LineWidth;
        int          Alpha;

    }; // class OFDObject

    typedef std::shared_ptr<OFDObject> OFDObjectPtr;

    // ======== class PageBlock ========
    class PageBlock{
    public:

        size_t GetObjectsCount() const {return Objects.size();};
        const OFDObjectPtr GetObject(size_t idx) const {return Objects[idx];};
        OFDObjectPtr GetObject(size_t idx) {return Objects[idx];};
        void AddObject(OFDObjectPtr object) {Objects.push_back(object);};

        std::vector<OFDObjectPtr> Objects;

    }; // class PageBlock

    class OFDObjectFactory{
    public:
        static OFDObjectPtr CreateObject(Object::Type objType);

    }; // class OFDObjectFactory

}; // namespace ofd

#endif // __OFD_OBJECT_H__

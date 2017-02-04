#ifndef __OFD_OBJECT_H__
#define __OFD_OBJECT_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "OFDColor.h"

namespace utils{
    class XMLWriter;
    class XMLReader;
    class XMLElement;
    typedef std::shared_ptr<utils::XMLElement> XMLElementPtr;
};

namespace ofd{

    namespace Object{
        enum class Type{
            UNKNOWN = -1, // 未知对象
            TEXT,         // 文本对象
            PATH,         // 图形对象
            IMAGE,        // 图像对象
            VIDEO,        // 视频对象
            COMPOSITE,    // 复合对象
        };
    }; // namespace Object

    // ======== class OFDObject ========
    class OFDObject : public std::enable_shared_from_this<OFDObject> {
    public:
        OFDObject(OFDPagePtr page);
        virtual ~OFDObject();

        uint64_t     ID;
        Object::Type Type;
        std::string  ObjectLabel;

        // -------- GraphUnit attributes --------
        // OFD P50. Page.xsd.
        ST_Box       Boundary; 
        std::string  Name;
        bool         Visible;
        double       CTM[6];
        double       LineWidth;
        int          Alpha;

        const OFDPagePtr GetPage() const;

        void GenerateXML(utils::XMLWriter &writer) const;
        //bool FromXML(utils::XMLReader &reader, const std::string &tagName);
        bool FromXML(utils::XMLElementPtr objectElement);

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement);
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement);

        std::weak_ptr<OFDPage> m_page;

    private:
        bool FromElementsXML(utils::XMLReader &reader);
        bool FromElementsXML(utils::XMLElementPtr objectElement);


    }; // class OFDObject


    // ======== class PageBlock ========
    class PageBlock{
    public:

        size_t GetObjectsCount() const {return Objects.size();};
        const OFDObjectPtr GetObject(size_t idx) const {return Objects[idx];};
        OFDObjectPtr GetObject(size_t idx) {return Objects[idx];};
        void AddObject(OFDObjectPtr object) {
            if ( object != nullptr ){
                Objects.push_back(object);
            }
        }

        std::vector<OFDObjectPtr> Objects;

    }; // class PageBlock

    //class OFDObjectFactory{
    //public:
        //static OFDObjectPtr CreateObject(Object::Type objType);

    //}; // class OFDObjectFactory

}; // namespace ofd

#endif // __OFD_OBJECT_H__

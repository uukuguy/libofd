#ifndef __OFD_OBJECT_H__
#define __OFD_OBJECT_H__

#include <memory>
#include "ofd/Common.h"
#include "ofd/Layer.h"
#include "utils/xml.h"

namespace ofd{

    enum class ObjectType{
        UNKNOWN = -1, // 未知对象
        TEXT,         // 文本对象
        PATH,         // 图形对象
        IMAGE,        // 图像对象
        VIDEO,        // 视频对象
        COMPOSITE,    // 复合对象
    };

    class Object : public std::enable_shared_from_this<Object>{
        public:
            Object(LayerPtr layer, ObjectType objectType, const std::string& objectLabel);
            virtual ~Object();
            ObjectPtr GetSelf(){return shared_from_this();};

            // =============== Public Attributes ================
        public:
            uint64_t     ID;
            ObjectType   Type;
            std::string  ObjectLabel;

            // -------- GraphUnit attributes --------
            // OFD P50. Page.xsd.
            ST_Box       Boundary; 
            std::string  Name;
            bool         Visible;
            double       CTM[6];
            double       LineWidth;
            int          Alpha;

            // =============== Public Methods ================
        public:
            void GenerateXML(utils::XMLWriter &writer) const;
            bool FromXML(utils::XMLElementPtr objectElement);

        protected:
            virtual void GenerateAttributesXML(utils::XMLWriter &writer) const;
            virtual void GenerateElementsXML(utils::XMLWriter &writer) const;

            virtual bool FromAttributesXML(utils::XMLElementPtr objectElement);
            virtual bool IterateElementsXML(utils::XMLElementPtr childElement);

            // ---------------- Private Attributes ----------------
        public:
            const LayerPtr GetLayer() const {return m_layer.lock();};
            LayerPtr GetLayer() {return m_layer.lock();};
            const PagePtr GetPage() const {return GetLayer()->GetPage();};
            PagePtr GetPage() {return GetLayer()->GetPage();};

        protected:
            std::weak_ptr<Layer> m_layer;

        private:
            bool FromElementsXML(utils::XMLElementPtr objectElement);

    }; // class Object


}; // namespace ofd

#endif // __OFD_OBJECT_H__


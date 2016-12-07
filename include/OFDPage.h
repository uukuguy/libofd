#ifndef __OFD_PAGE_H__
#define __OFD_PAGE_H__

#include <memory>
#include <string>
#include "OFDObject.h"

namespace ofd{

    namespace Layer{

        enum class Type{
            BODY,
            BACKGROUND,
            FOREGROUND,
            CUSTOM,
        };

    }; // namespace Layer

    class OFDLayer : public PageBlock {
    public:
        OFDLayer(){};
        OFDLayer(Layer::Type layerType) : Type(layerType){};
        virtual ~OFDLayer(){};

        uint64_t    ID;
        Layer::Type Type;
        // ST_REfID DrawParam

    }; // class OFDLayer

    typedef std::shared_ptr<OFDLayer> OFDLayerPtr;

    class OFDPage{
    public:
        OFDPage();
        virtual ~OFDPage();

        std::string to_string() const;

        bool Open();
        void Close();
        bool IsOpened() const;

        size_t GetLayersCount() const;
        const OFDLayerPtr GetLayer(size_t idx) const;
        OFDLayerPtr GetLayer(size_t idx);
        OFDLayerPtr AddNewLayer(Layer::Type layerType);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // OFDPage

}; // namespace ofd

#endif // __OFD_PAGE_H__

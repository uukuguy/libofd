#ifndef __OFD_PAGE_H__
#define __OFD_PAGE_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
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
        OFDLayer(): ID(0), Type(Layer::Type::BODY){};
        OFDLayer(Layer::Type layerType) : Type(layerType){};
        virtual ~OFDLayer(){};

        uint64_t    ID;
        Layer::Type Type;
        /*ST_REfID    DrawParam;*/

    }; // class OFDLayer

    class OFDPage : public std::enable_shared_from_this<OFDPage> {
    public:
        OFDPage(OFDDocumentPtr ofdDocument);
        virtual ~OFDPage();

        const OFDDocumentPtr GetOFDDocument() const;
        OFDDocumentPtr GetOFDDocument();
        OFDPagePtr GetSelf();

        std::string to_string() const;

        bool Open();
        void Close();
        bool IsOpened() const;

        uint64_t GetID() const;
        void SetID(uint64_t id);
        std::string GetBaseLoc() const;
        void SetBaseLoc(const std::string &baseLoc);
        const CT_PageArea& GetPageArea() const;
        void SetPageArea(const CT_PageArea &pageArea);

        size_t GetLayersCount() const;
        const OFDLayerPtr GetLayer(size_t idx) const;
        OFDLayerPtr GetLayer(size_t idx);
        OFDLayerPtr AddNewLayer(Layer::Type layerType);
        const OFDLayerPtr GetBodyLayer() const;
        OFDLayerPtr GetBodyLayer();

        std::string GeneratePageXML() const;

        bool FromPageXML(const std::string &strPageXML);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // OFDPage

}; // namespace ofd

#endif // __OFD_PAGE_H__

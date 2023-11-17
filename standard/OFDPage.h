#ifndef __OFDPAGE_H__
#define __OFDPAGE_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "OFDObject.h"

//namespace ofd{

    //enum class LayerType{
        //BODY,
        //BACKGROUND,
        //FOREGROUND,
        //CUSTOM,
    //};


    //class OFDLayer : public PageBlock {
    //public:
        //OFDLayer(): ID(0), Type(LayerType::BODY){};
        //OFDLayer(LayerType layerType) : Type(layerType){};
        //virtual ~OFDLayer(){};

        //uint64_t    ID;
        //LayerType Type;
        //[>ST_REfID    DrawParam;<]

    //}; // class OFDLayer

    //class OFDPage : public std::enable_shared_from_this<OFDPage> {
    //private:
        //OFDPage(OFDDocumentPtr ofdDocument);
    //public:
        //virtual ~OFDPage();

        //static OFDPagePtr CreateNewPage(OFDDocumentPtr ofdDocument);

        //const OFDDocumentPtr GetOFDDocument() const;
        //OFDDocumentPtr GetOFDDocument();
        //OFDPagePtr GetSelf();

        //std::string to_string() const;

        //bool Open();
        //void Close();
        //bool IsOpened() const;

        //uint64_t GetID() const;
        //void SetID(uint64_t id);
        //std::string GetBaseLoc() const;
        //void SetBaseLoc(const std::string &baseLoc);
        //const CT_PageArea& GetPageArea() const;
        //void SetPageArea(const CT_PageArea &pageArea);

        //size_t GetLayersCount() const;
        //const OFDLayerPtr GetLayer(size_t idx) const;
        //OFDLayerPtr GetLayer(size_t idx);
        //OFDLayerPtr AddNewLayer(LayerType layerType);
        //const OFDLayerPtr GetBodyLayer() const;
        //OFDLayerPtr GetBodyLayer();

        //bool AddObject(OFDObjectPtr object);

        //std::string GeneratePageXML() const;

        //bool FromPageXML(const std::string &strPageXML);

    //private:
        //class ImplCls;
        //std::unique_ptr<ImplCls> m_impl;

    //}; // OFDPage

//}; // namespace ofd

#endif // __OFDPAGE_H__

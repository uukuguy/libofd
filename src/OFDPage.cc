#include <sstream>
#include <assert.h>
#include "OFDPage.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDPage::ImplCls ****************

class OFDPage::ImplCls{
public:
    ImplCls();
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    size_t GetLayersCount() const;
    const OFDLayerPtr GetLayer(size_t idx) const;
    OFDLayerPtr GetLayer(size_t idx);

    OFDLayerPtr AddNewLayer(Layer::Type layerType);
    const OFDLayerPtr GetBodyLayer() const;
    OFDLayerPtr GetBodyLayer();

    std::string GeneratePageXML() const;

    // -------- Private Attributes --------

    uint64_t    ID;
    CT_PageArea Area;

    std::vector<OFDLayerPtr> Layers;

    bool m_opened;

private:
    void generateContentXML(XMLWriter &writer) const;


}; // class OFDPage::ImplCls

OFDPage::ImplCls::ImplCls() : ID(0), m_opened(false){
}

OFDPage::ImplCls::~ImplCls(){
}

std::string OFDPage::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDPage ========\n";
    ss << std::endl;
    return ss.str();
}

bool OFDPage::ImplCls::Open(){
    if ( m_opened ) return true;

    return m_opened;
}

void OFDPage::ImplCls::Close(){
    if ( !m_opened ) return;
}

size_t OFDPage::ImplCls::GetLayersCount() const{
    return Layers.size();
}

const OFDLayerPtr OFDPage::ImplCls::GetLayer(size_t idx) const{
    return Layers[idx];
}

OFDLayerPtr OFDPage::ImplCls::GetLayer(size_t idx){
    return Layers[idx];
}

OFDLayerPtr OFDPage::ImplCls::AddNewLayer(Layer::Type layerType){
    OFDLayerPtr layer = std::make_shared<OFDLayer>(layerType);
    layer->ID = Layers.size();
    Layers.push_back(layer);
    return layer;
}

const OFDLayerPtr OFDPage::ImplCls::GetBodyLayer() const{
    assert( Layers.size() > 0 );
    const OFDLayerPtr bodyLayer = Layers[0]; 
    return bodyLayer;
}

OFDLayerPtr OFDPage::ImplCls::GetBodyLayer(){
    assert( Layers.size() > 0 );
    OFDLayerPtr bodyLayer = Layers[0]; 
    return bodyLayer;
}

// Called by OFDPage::ImplCls::GeneratePageXML()
void OFDPage::ImplCls::generateContentXML(XMLWriter &writer) const{
    if ( Layers.size() == 0 ) return;

    // -------- <Content>
    writer.StartElement("Content");{

        for ( auto layer : Layers ){

            // -------- <Layer>
            // OFD (section 7.7) P20.
            // Required.
            writer.StartElement("Layer");{

                // -------- <Layer ID="">
                writer.WriteAttribute("ID", layer->ID);

                // -------- CT_Layer --------

                // TODO
                // -------- <Layer Type="">

                // TODO
                // -------- <Layer DrawParam="">

                // -------- CT_PageBlock --------
                for ( auto object : layer->Objects ){
                    object->GenerateXML(writer);
                }

            } writer.EndElement();
        }

    } writer.EndElement();
}

// Defined in OFDDocument.cc
void writePageAreaXML(XMLWriter &writer, const CT_PageArea &pageArea);

// Generate content in Doc_N/Pages/Page_N/Content.xml
// Called by OFDFile::Save()
// OFD (section 7.7) P18, Page.xsd。
std::string OFDPage::ImplCls::GeneratePageXML() const{

    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <Page>
    writer.StartElement("Page");{
        OFDXML_HEAD_ATTRIBUTES;

        // TODO
        // -------- <Area>
        // Optional.
        writer.StartElement("Area");{
           writePageAreaXML(writer, Area); 
        } writer.EndElement();

        // TODO
        // -------- <Template>
        // OFD (section 7.7) P19.
        // Optional.

        // TODO
        // -------- <PageRes>
        // Optional.

        // -------- <Content>
        // Optional. Blank page if not exist.
        generateContentXML(writer);

        // TODO
        // -------- <Actions>
        // Optional.

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

// **************** class OFDPage ****************

OFDPage::OFDPage(){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());

    AddNewLayer(Layer::Type::BODY);
}

OFDPage::~OFDPage(){
}

uint64_t OFDPage::GetID() const{
    return m_impl->ID;
}

void OFDPage::SetID(uint64_t id){
    m_impl->ID = id;
}

std::string OFDPage::to_string() const{
    return m_impl->to_string();
}

bool OFDPage::Open(){
    return m_impl->Open();
}

void OFDPage::Close(){
    m_impl->Close();
}

bool OFDPage::IsOpened() const{
    return m_impl->m_opened;
}

size_t OFDPage::GetLayersCount() const{
    return m_impl->GetLayersCount();
}

const OFDLayerPtr OFDPage::GetLayer(size_t idx) const{
    return m_impl->GetLayer(idx);
}

OFDLayerPtr OFDPage::GetLayer(size_t idx){
    return m_impl->GetLayer(idx);
}

OFDLayerPtr OFDPage::AddNewLayer(Layer::Type layerType){
    return m_impl->AddNewLayer(layerType);
}

const OFDLayerPtr OFDPage::GetBodyLayer() const{
    return m_impl->GetBodyLayer();
}

OFDLayerPtr OFDPage::GetBodyLayer(){
    return m_impl->GetBodyLayer();
}

std::string OFDPage::GeneratePageXML() const{
    return m_impl->GeneratePageXML();
}

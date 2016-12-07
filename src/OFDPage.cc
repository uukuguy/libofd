#include <sstream>
#include "logger.h"
#include "OFDPage.h"

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

    // -------- Private Attributes --------

    bool m_opened;

    std::vector<OFDLayerPtr> Layers;

}; // class OFDPage::ImplCls

OFDPage::ImplCls::ImplCls() : m_opened(false) {
    AddNewLayer(Layer::Type::BODY);
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
    Layers.push_back(layer);
    return layer;
}

// **************** class OFDPage ****************

OFDPage::OFDPage(){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());
}

OFDPage::~OFDPage(){
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


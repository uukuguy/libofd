#include <sstream>
#include <assert.h>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "OFDTextObject.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDPage::ImplCls ****************

class OFDPage::ImplCls{
public:
    ImplCls(OFDDocumentPtr ofdDocument, OFDPage *ofdPage);
    ~ImplCls();

    void Init_After_Construct();

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

    bool FromPageXML(const std::string &strPageXML);

private:
    bool FromContentXML(XMLElementPtr contentElement);

    // -------- Private Attributes --------
public:
    
    std::weak_ptr<OFDDocument> m_ofdDocument;
    OFDPage *m_ofdPage;
    std::string BaseLoc;
    uint64_t    ID;

    CT_PageArea Area;

    std::vector<OFDLayerPtr> Layers;

    bool m_opened;

private:
    void generateContentXML(XMLWriter &writer) const;

    OFDLayerPtr FromLayerXML(XMLElementPtr layerElement);

}; // class OFDPage::ImplCls

OFDPage::ImplCls::ImplCls(OFDDocumentPtr ofdDocument, OFDPage *ofdPage) : 
    m_ofdDocument(ofdDocument), m_ofdPage(ofdPage),
    ID(0), m_opened(false){
}

OFDPage::ImplCls::~ImplCls(){
}

void OFDPage::ImplCls::Init_After_Construct(){
}

std::string OFDPage::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDPage ========\n";
    ss << " ID: " << ID << "\n";
    ss << " BaseLoc: " << BaseLoc << "\n";
    ss << " Layers: " << Layers.size() << "\n";
    if ( Layers.size() > 0 ){
        // FIXME
        OFDLayerPtr bodyLayer = Layers[0];
        assert(bodyLayer != nullptr);
    }
    ss << " Area: " << Area.to_string() << "\n";

    ss << std::endl;
    return ss.str();
}

bool OFDPage::ImplCls::Open(){
    if ( m_opened ) return true;
    if ( BaseLoc.empty() ) return false;
    OFDDocumentPtr ofdDocument = m_ofdDocument.lock();
    if ( ofdDocument == nullptr ) return false;

    const OFDPackagePtr  ofdPackage = ofdDocument->GetOFDPackage();     
    if ( ofdPackage == nullptr ) return false;

    std::string docRoot = ofdDocument->GetDocRoot();
    std::string pageXMLFile = docRoot + "/" + BaseLoc + "/Content.xml";
    LOG(INFO) << "Try to open zipfile " << pageXMLFile;

    bool ok = false;
    std::string strPageXML;
    std::tie(strPageXML, ok) = ofdPackage->ReadZipFileString(pageXMLFile);

    if ( ok ) {
        m_opened = FromPageXML(strPageXML);

        if ( m_opened ){
            LOG(INFO) << "Open page success.";
            LOG(INFO) << to_string();
        } else {
            LOG(ERROR) << "Open page failed. ID: " << ID << " BaseLoc: " << BaseLoc;
        }
    } else {
        LOG(ERROR) << "OFDPage::Open() ReadZipFileString() failed. " << pageXMLFile;
    }

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
    if ( Layers.size() > 0 ){
        const OFDLayerPtr bodyLayer = Layers[0]; 
        return bodyLayer;
    } else {
        return nullptr;
    }
}

OFDLayerPtr OFDPage::ImplCls::GetBodyLayer(){
    if ( Layers.size() > 0 ){
        OFDLayerPtr bodyLayer = Layers[0]; 
        return bodyLayer;
    } else {
        return nullptr;
    }
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
                //LOG(DEBUG) << "############## There are " << layer->Objects.size() << " objects in layer";
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
// Called by OFDPackage::Save()
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

std::tuple<ST_Box, bool> ReadBoxFromXML(XMLElementPtr boxElement){
    ST_Box box;
    bool exist = false;

    std::string boxString;
    std::tie(boxString, exist) = boxElement->GetStringValue();
    LOG(INFO) << "Box String: " << boxString;
    if ( exist ){
        std::vector<std::string> tokens = utils::SplitString(boxString);
        if ( tokens.size() >= 4 ){
            box.Left = atof(tokens[0].c_str());
            box.Top = atof(tokens[1].c_str());
            box.Width = atof(tokens[2].c_str());
            box.Height = atof(tokens[3].c_str());
        } else {
            LOG(ERROR) << "Box String tokens size >= 4 failed. boxString:" << boxString;
            exist = false;
        }
    }

    return std::make_tuple(box, exist);
}

// OFD (section 7.5) P11. Definitions.xsd
std::tuple<CT_PageArea,bool> FromPageAreaXML(XMLElementPtr pageAreaElement){
    bool ok = true;
    CT_PageArea pageArea;

    LOG(INFO) << "******** FromPagAreaXML()";

    XMLElementPtr childElement = pageAreaElement->GetFirstChildElement();
    while ( childElement != nullptr ){

        std::string childName = childElement->GetName();
        LOG(INFO) << "PageArea child name: " << childName;
        bool exist = false;

        // -------- <PhysicalBox>
        // Required
        if ( childName == "PhysicalBox" ){
            std::tie(pageArea.PhysicalBox, exist) = ReadBoxFromXML(childElement);
            if ( !exist ){
                LOG(ERROR) << "Attribute PhysicalBox is requred in PageArea XML";
                break;
            } else {
                ok = true;
            }

        // -------- <ApplicationBox>
        } else if ( childName == "ApplicationBox" ){
            std::tie(pageArea.ApplicationBox, exist) = ReadBoxFromXML(childElement);
            if ( exist ){
                pageArea.EnableApplicationBox(true);
            }

        // -------- <ContentBox>
        } else if ( childName == "ContentBox" ){
            std::tie(pageArea.ContentBox, exist) = ReadBoxFromXML(childElement);
            if ( exist ){
                pageArea.EnableContentBox(true);
            }

        // -------- <BleedBox>
        } else if ( childName == "BleedBox" ){
            std::tie(pageArea.BleedBox, exist) = ReadBoxFromXML(childElement);
            if ( exist ){
                pageArea.EnableBleedBox(true);
            }
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return std::make_tuple(pageArea, ok);
}

// ======== OFDPage::ImplCls::FromPageXML() ========
// OFD (section 7.7) P18. Page.xsd
bool OFDPage::ImplCls::FromPageXML(const std::string &strPageXML){
    bool ok = false;

    XMLElementPtr pageElement = XMLElement::ParseRootElement(strPageXML);
    if ( pageElement != nullptr ){
        std::string elementName = pageElement->GetName();
        if ( elementName == "Page" ){

            XMLElementPtr childElement = pageElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                if ( childName == "Area" ){
                    // -------- <Area>
                    // Optional.
                    std::tie(Area, ok) = FromPageAreaXML(childElement);
                } else if ( childName == "Content" ) {
                    // -------- <Content>
                    // Optional.
                    FromContentXML(childElement);


                //if ( childName == "Template" ) {
                    //// TODO
                    //// -------- <Template>
                    //// Optional.

                //} else if ( childName == "PageRes" ) {
                    //// TODO
                    //// -------- <PageRes>
                    //// Optional.

                //} else if ( childName == "Actions" ) {
                        // TODO
                        // -------- <Actions>
                        // OFD (section 14.1) P73. Document.xsd
                        // Optional.
                        //FromActionsXML(childElement);

                    //}

                }

                childElement = childElement->GetNextSiblingElement();
            }
        }
    } else {
        LOG(ERROR) << "No root element in Content.xml";
    }

    return ok;
}

// -------- FromLayerXML() --------
// Called by OFDPage::ImplCls::FromContentXML()
OFDLayerPtr OFDPage::ImplCls::FromLayerXML(XMLElementPtr layerElement){
    OFDLayerPtr layer = nullptr;

    assert(layerElement != nullptr);

    layer = std::make_shared<OFDLayer>();

    bool exist = false;

    uint64_t layerID = 0;
    uint64_t layerType = 0;
    uint64_t drawParamID = 0;

    std::tie(layerID, exist) = layerElement->GetIntAttribute("ID");
    if ( !exist ) return nullptr;
    std::tie(layerType, std::ignore) = layerElement->GetIntAttribute("Type");
    std::tie(drawParamID, std::ignore) = layerElement->GetIntAttribute("DrawParam");

    layer->ID = layerID;
    layer->Type = (Layer::Type)layerType;

    XMLElementPtr childElement = layerElement->GetFirstChildElement();
    while ( childElement != nullptr ){

        OFDObjectPtr object = nullptr;

        std::string childName = childElement->GetName();
        if ( childName == "TextObject" ){
            OFDTextObjectPtr textObject = std::make_shared<OFDTextObject>(m_ofdPage->GetSelf());
            textObject->FromXML(childElement);
            object = textObject;
            //LOG(DEBUG) << "Load text object. total: " << layer->Objects.size() << " GetObjectsCount() = " << layer->GetObjectsCount();

        } else if ( childName == "PathObject" ){
            // TODO
            //OFDPathObjectPtr pathObject = std::make_shared<OFDPathObject>(m_ofdPage.GetSelf());
            //pathObject->FromXML(childElement);
            //object = pathObject;
        } else if ( childName == "ImageObject" ){
            // TODO
            //OFDImageObjectPtr imageObject = std::make_shared<OFDImageObject>(m_ofdPage->GetSelf());
            //imageObject->FromXML(childElement);
            //object = imageObject;
        } else if ( childName == "CompositeObject" ){
            // TODO
            //OFDCompositeObjectPtr compositeObject = std::make_shared<OFDCompositeObject>(m_ofdPage->GetSelf());
            //compositeObject->FromXML(childElement);
            //object = compositeObject;
        }

        if ( object != nullptr ){
            layer->Objects.push_back(object);
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return layer;
}

// ======== OFDPage::ImplCls::FromContentXML() ========
// OFD (section 7.7) P20. Page.xsd
bool OFDPage::ImplCls::FromContentXML(XMLElementPtr contentElement){
    bool ok = false;

    assert(contentElement != nullptr);

    XMLElementPtr childElement = contentElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        if ( childName == "Layer" ){
            OFDLayerPtr layer = FromLayerXML(childElement);
            if ( layer != nullptr ){
                Layers.push_back(layer);
                LOG(INFO) << "layer added. GetObjectsCount() = " << layer->GetObjectsCount();
                ok = true;
            }
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// **************** class OFDPage ****************

OFDPage::OFDPage(OFDDocumentPtr ofdDocument){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(ofdDocument, this));
}

OFDPage::~OFDPage(){
}

OFDPagePtr OFDPage::GetSelf(){
    return shared_from_this();
}

const OFDDocumentPtr OFDPage::GetOFDDocument() const{
    return m_impl->m_ofdDocument.lock();
}

OFDDocumentPtr OFDPage::GetOFDDocument() {
    return m_impl->m_ofdDocument.lock();
}

uint64_t OFDPage::GetID() const{
    return m_impl->ID;
}

void OFDPage::SetID(uint64_t id){
    m_impl->ID = id;
}

std::string OFDPage::GetBaseLoc() const {
    return m_impl->BaseLoc;
}

void OFDPage::SetBaseLoc(const std::string &baseLoc){
    m_impl->BaseLoc = baseLoc;
}

const CT_PageArea& OFDPage::GetPageArea() const{
    return m_impl->Area;
}

void OFDPage::SetPageArea(const CT_PageArea &pageArea){
    m_impl->Area = pageArea;
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

bool OFDPage::FromPageXML(const std::string &strPageXML){
    return m_impl->FromPageXML(strPageXML);
}

OFDPagePtr OFDPage::CreateNewPage(OFDDocumentPtr ofdDocument){
    OFDPagePtr ofdPage = std::shared_ptr<OFDPage>(new OFDPage(ofdDocument));
    ofdPage->m_impl->Init_After_Construct();
    return ofdPage;
}


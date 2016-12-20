#include <sstream>
#include <assert.h>
#include "OFDFile.h"
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
    ImplCls(OFDDocument *ofdDocument);
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

    bool FromPageXML(const std::string &strPageXML);

private:
    bool FromContentXML(XMLReader &reader, const std::string &tagName);

    // -------- Private Attributes --------
public:
    
    OFDDocument *m_ofdDocument;
    std::string BaseLoc;
    uint64_t    ID;

    CT_PageArea Area;

    std::vector<OFDLayerPtr> Layers;

    bool m_opened;

private:
    void generateContentXML(XMLWriter &writer) const;


}; // class OFDPage::ImplCls

OFDPage::ImplCls::ImplCls(OFDDocument *ofdDocument) : 
    m_ofdDocument(ofdDocument),
    ID(0), m_opened(false){
}

OFDPage::ImplCls::~ImplCls(){
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
    if ( m_ofdDocument == nullptr ) return false;

    const OFDFile * ofdFile = m_ofdDocument->GetOFDFile();     
    if ( ofdFile == nullptr ) return false;

    std::string docRoot = m_ofdDocument->GetDocRoot();
    std::string pageXMLFile = docRoot + "/" + BaseLoc + "/Content.xml";
    LOG(INFO) << "Try to open zipfile " << pageXMLFile;

    bool ok = false;
    std::string strPageXML;
    std::tie(strPageXML, ok) = ofdFile->ReadZipFileString(pageXMLFile);

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

// defined in OFDDocument.cc
std::tuple<CT_PageArea,bool> FromPageAreaXML(XMLReader &reader, const std::string &tagName);

// ======== OFDPage::ImplCls::FromPageXML() ========
// OFD (section 7.7) P18. Page.xsd
bool OFDPage::ImplCls::FromPageXML(const std::string &strPageXML){
    bool ok = true;

    XMLReader reader;
    if ( reader.ParseXML(strPageXML) ){

        if ( reader.CheckElement("Page") ){
            if ( reader.EnterChildElement("Page") ){

                while ( reader.HasElement() ){

                    // TODO
                    // -------- <Template>
                    // Optional.
                    if ( reader.CheckElement("Template") ) {


                    // -------- <Area>
                    // Optional.
                    } else if ( reader.CheckElement("Area") ) {
                        std::tie(Area, ok) = FromPageAreaXML(reader, "Area");

                    // TODO
                    // -------- <PageRes>
                    // Optional.
                    } else if ( reader.CheckElement("PageRes") ) {

                    // -------- <Content>
                    // Optional.
                    } else if ( reader.CheckElement("Content") ) {
                        FromContentXML(reader, "Content");


                    // TODO
                    // -------- <Actions>
                    // OFD (section 14.1) P73. Document.xsd
                    // Optional.
                    /*} else if ( reader.CheckElement("Actions") ) {*/
                        /*FromActionsXML(reader);*/

                    }

                    reader.NextElement();
                };
                reader.BackParentElement();
            } 
        }
    }
    
    return ok;
}

OFDObjectPtr FromTextObjectXML(XMLReader &reader){

    OFDTextObjectPtr textObject = std::make_shared<OFDTextObject>();

    if ( reader.EnterChildElement("TextObject") ){

        // -------- <TextObject ID="">
        // Required.
        uint64_t objectId = 0;
        reader.ReadAttribute("ID", objectId);
        textObject->ID = objectId;

        // -------- CT_GraphicUnit --------


        // -------- CT_Text --------

        reader.BackParentElement(); 
    }; 

    return textObject;
}

// TODO
OFDObjectPtr FromPathObjectXML(XMLReader &reader){
    OFDObjectPtr object = nullptr;

    return object;
}

// TODO
OFDObjectPtr FromImageObjectXML(XMLReader &reader){
    OFDObjectPtr object = nullptr;

    return object;
}

// TODO
OFDObjectPtr FromCompositeObjectXML(XMLReader &reader){
    OFDObjectPtr object = nullptr;

    return object;
}

OFDLayerPtr FromLayerXML(XMLReader &reader){
    OFDLayerPtr layer = nullptr;

    if ( reader.EnterChildElement("Layer") ){

        layer = std::make_shared<OFDLayer>();

        uint64_t layerID = 0;
        uint64_t layerType = 0;
        uint64_t drawParamID = 0;
        reader.ReadAttribute("ID", layerID);
        reader.ReadAttribute("Type", layerType);
        reader.ReadAttribute("DrawParam", drawParamID);

        layer->ID = layerID;
        layer->Type = (Layer::Type)layerType;
        // TODO
        /*layer->drawParamID = drawParamID;*/

        while ( reader.HasElement() ){

            OFDObjectPtr object = nullptr;

            if ( reader.CheckElement("TextObject") ){
                object = FromTextObjectXML(reader); 


            } else if ( reader.CheckElement("PathObject") ){
                object = FromPathObjectXML(reader); 

            } else if ( reader.CheckElement("ImageObject") ){
                object = FromImageObjectXML(reader); 

            } else if ( reader.CheckElement("CompositeObject") ){
                object = FromCompositeObjectXML(reader); 

            }

            if ( object != nullptr ){
                layer->Objects.push_back(object);
            }

            reader.NextElement();
        };

        reader.BackParentElement();
    } 
    return layer;
}

// ======== OFDPage::ImplCls::FromContentXML() ========
// OFD (section 7.7) P20. Page.xsd
bool OFDPage::ImplCls::FromContentXML(XMLReader &reader, const std::string &tagName){
    bool ok = false;

    if ( reader.EnterChildElement(tagName) ){
    
        while ( reader.HasElement() ){

            if ( reader.CheckElement("Layer") ){
                OFDLayerPtr layer = FromLayerXML(reader);
                if ( layer != nullptr ){
                    Layers.push_back(layer);
                    ok = true;
                }
            }

            reader.NextElement();
        };
        reader.BackParentElement();
    } 

    return ok;
}

// **************** class OFDPage ****************

OFDPage::OFDPage(OFDDocument *ofdDocument){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(ofdDocument));

    AddNewLayer(Layer::Type::BODY);
}

OFDPage::~OFDPage(){
}

const OFDDocument *OFDPage::GetOFDDocument() const{
    return m_impl->m_ofdDocument;
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


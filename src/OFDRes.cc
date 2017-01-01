#include <vector>
#include <assert.h>

#include "OFDRes.h"
#include "OFDPage.h"
#include "OFDDocument.h"
#include "OFDPackage.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

// **************** class OFDRes::ImplCls ****************

class OFDRes::ImplCls{
public:
    ImplCls(OFDRes *ofdRes, OFDPackagePtr ofdPackage, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, OFDDocumentPtr ofdDocument, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, OFDPagePtr ofdPage, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, OFDPackagePtr ofdPackage, OFDDocumentPtr ofdDocument, OFDPagePtr ofdPage, const std::string &resDescFile); 

    ~ImplCls();

    void Init_After_Construct();

    void AddColorSpace(const OFDColorSpace &ofdColorSpace){m_colorSpaces.push_back(ofdColorSpace);};
    void AddFont(OFDFontPtr font);
    const FontMap &GetFonts() const {return m_fonts;};
    const OFDFontPtr GetFont(uint64_t fontID) const;

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

    bool LoadFonts();
    bool LoadImages();

    Res::Level GetResLevel() const;
    std::string GetEntryRoot() const;

private:
    bool FromColorSpacesXML(XMLElementPtr colorSpacesElement);
    bool FromFontsXML(XMLElementPtr fontsElement);

    // -------- Private Attributes --------
public:
    OFDRes* m_ofdRes;
    std::weak_ptr<OFDPackage> m_ofdPackage;
    std::weak_ptr<OFDDocument> m_ofdDocument;
    std::weak_ptr<OFDPage> m_ofdPage;
    std::string m_baseLoc;
    std::string m_resDescFile;

    ColorSpaceArray m_colorSpaces;
    FontMap m_fonts;


}; // class OFDRes::ImplCls


OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, OFDPackagePtr ofdPackage, const std::string &resDescFile) : 
    m_ofdRes(ofdRes),m_ofdPackage(ofdPackage), 
    m_baseLoc("Res"), m_resDescFile(resDescFile) {
}

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, OFDDocumentPtr ofdDocument, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    m_ofdDocument(ofdDocument), 
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, OFDPagePtr ofdPage, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    m_ofdPage(ofdPage),
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

void OFDRes::ImplCls::Init_After_Construct(){
    if ( !m_ofdPage.expired() ){
        OFDDocumentPtr ofdDocument = m_ofdPage.lock()->GetOFDDocument();
        assert(ofdDocument != nullptr);
        m_ofdDocument = ofdDocument;

        OFDPackagePtr ofdPackage = ofdDocument->GetOFDPackage();
        assert(ofdPackage != nullptr);
        m_ofdPackage = ofdPackage;

        if ( m_resDescFile.empty() ){
            m_resDescFile = "PageRes.xml";
        }
    } else {
        if ( !m_ofdDocument.expired() ){
            OFDPackagePtr ofdPackage = m_ofdDocument.lock()->GetOFDPackage();
            assert(ofdPackage != nullptr);
            m_ofdPackage = ofdPackage;

            if ( m_resDescFile.empty() ){
                m_resDescFile = "DocumentRes.xml";
            }
        } else {
            if ( m_resDescFile.empty() ){
                m_resDescFile = "PublicRes.xml";
            }
        }
    }
}

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, OFDPackagePtr ofdPackage, OFDDocumentPtr ofdDocument, OFDPagePtr ofdPage, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    //m_ofdPackage(ofdPackage), m_ofdDocument(ofdDocument), m_ofdPage(ofdPage),
    m_baseLoc("Res"), m_resDescFile(resDescFile){
    
    m_ofdPackage.reset();
    m_ofdDocument.reset();
    m_ofdPage.reset();

    if ( ofdPackage != nullptr ){
        m_ofdPackage = ofdPackage;
    }

    if ( ofdDocument != nullptr ){
        m_ofdDocument = ofdDocument;
    }

    if ( ofdPage != nullptr ){
        m_ofdPage = ofdPage;
    }

    if ( m_resDescFile.empty() ){
        if ( m_ofdPackage.lock() != nullptr ){
            if ( m_ofdDocument.lock() != nullptr ){
                if ( m_ofdPage.lock() != nullptr ){
                    m_resDescFile = "PageRes.xml";
                } else {
                    m_resDescFile = "DocumentRes.xml";
                }
            } else {
                m_resDescFile = "PublicRes.xml";
            }
        } else {
            LOG(ERROR) << "m_ofdPackage == nullptr in OFDRes.";
        }
    }
}

OFDRes::ImplCls::~ImplCls(){
}

void OFDRes::ImplCls::AddFont(OFDFontPtr font){
    uint64_t fontID = font->ID;
    if ( m_fonts.find(fontID) != m_fonts.end() ){
        m_fonts[fontID] = font;
    } else {
        m_fonts.insert(FontMap::value_type(fontID, font));
    }
    font->FontFile = font->GetFileName();
}

const OFDFontPtr OFDRes::ImplCls::GetFont(uint64_t fontID) const{
    auto iter = m_fonts.find(fontID);
    if ( iter != m_fonts.end() ){
        return iter->second;
    } else {
        return nullptr;
    }
}

void generateColorSpacesXML(XMLWriter &writer){
}

void generateDrawParamsXML(XMLWriter &writer){
}

// -------- generateFontsXML() --------
// OFD (section 11.1) P61. Res.xsd.
void generateFontsXML(XMLWriter &writer, const FontMap &fonts){

    for ( auto iter : fonts ){
        auto font = iter.second;

        font->GenerateXML(writer);
    }
}

void generateMultiMediasXML(XMLWriter &writer){
}

void generateCompositeGraphicUnitsXML(XMLWriter &writer){
}

// ======== OFDRes::ImplCls::GenerateResXML() ========
// OFD (section 7.9) P23. Res.xml.
std::string OFDRes::ImplCls::GenerateResXML() const{
    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <Page>
    writer.StartElement("Res");{
        OFDXML_HEAD_ATTRIBUTES;

        writer.WriteAttribute("BaseLoc", m_baseLoc);

        // TODO
        // -------- <ColorSpaces>
        // Optional.
        if ( m_colorSpaces.size() > 0 ){
            writer.StartElement("ColorSpaces");{
                generateColorSpacesXML(writer);    
            } writer.EndElement();
        }

        // TODO
        // -------- <DrawParams>
        // Optional.
        //if ( m_drawParams.size() > 0 ){
            //writer.StartElement("DrawParams");{
                //generateDrawParamsXML(writer);    
            //} writer.EndElement();
        //}

        // -------- <Fonts>
        // Optional.
        if ( m_fonts.size() > 0 ){
            writer.StartElement("Fonts");{
                generateFontsXML(writer, m_fonts);    
            } writer.EndElement();
        }

        // TODO
        // -------- <MultiMedias>
        // Optional.
        //if ( m_multiMedias.size() > 0 ){
            //writer.StartElement("MultiMedias");{
                //generateMultiMediasXML(writer);    
            //} writer.EndElement();
        //}

        // TODO
        // -------- <CompositeGraphicUnits>
        // Optional.
        //if ( m_compositeGraphicUnits.size() > 0 ){
            //writer.StartElement("CompositeGraphicUnits");{
                //generateCompositeGraphicUnitsXML(writer);    
            //} writer.EndElement();
        //}

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

bool OFDRes::ImplCls::FromColorSpacesXML(XMLElementPtr colorSpacesElement){
    bool ok = true;

    return ok;
}

bool OFDRes::ImplCls::LoadFonts(){
    bool ok = true;

    if ( m_ofdPackage.expired() ){
        return false;
    }

    for ( auto fontIter : m_fonts ){
        auto font = fontIter.second;

        if ( !font->Load(m_ofdPackage.lock()) ){
            LOG(ERROR) << "Load font (" << font->FontName << " failed.";
        }
    }

    return ok;
}

bool OFDRes::ImplCls::LoadImages(){
    bool ok = true;

    return ok;
}

bool OFDRes::ImplCls::FromFontsXML(XMLElementPtr fontsElement){
    bool ok = false;

    XMLElementPtr childElement = fontsElement->GetFirstChildElement();
    while ( childElement != nullptr ){

        OFDFontPtr font = std::make_shared<OFDFont>();
        if ( font->FromXML(childElement) ){
            std::string fontFilePath = GetEntryRoot() + "/" + m_baseLoc + "/" + font->FontFile;
            font->SetFontFilePath(fontFilePath);
            AddFont(font);
            ok = true;
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// ======== OFDRes::ImplCls::FromResXML() ========
// OFD (section 7.9) P23. Res.xml.
bool OFDRes::ImplCls::FromResXML(const std::string &strResXML){
    bool ok = true;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strResXML);
    if ( rootElement != nullptr ){
        if ( rootElement->GetName() == "Res" ){

            // -------- <Res BaseLoc="">
            // Required.
            bool exist = false;
            std::tie(m_baseLoc, exist) = rootElement->GetStringAttribute("BaseLoc");
            if ( !exist ){
                LOG(ERROR) << "BaseLoc attribute is required in Res.xsd";
                return false;
            }

            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                if ( childName == "ColorSpaces" ){
                    // -------- <ColorSpaces>
                    // Optional
                    FromColorSpacesXML(childElement);

                //} else if ( childName == "DrawParams" ){
                    // TODO
                    // -------- <DrawParams>
                    // Optional

                } else if ( childName == "Fonts" ){
                    // -------- <Fonts>
                    // Optional
                    FromFontsXML(childElement);

                //} else if ( childName == "MultiMedias" ){
                    // TODO
                    // -------- <MultiMedias>
                    // Optional

                //} else if ( childName == "CompositeGraphicUnits" ){
                    // TODO
                    // -------- <CompositeGraphicUnits>
                    // Optional

                }

                childElement = rootElement->GetNextSiblingElement();
            }
        }
    }

    return ok;
}

// **************** class OFDRes ****************

OFDRes::OFDRes(OFDPackagePtr ofdPackage, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, ofdPackage, resDescFile))){
}

OFDRes::OFDRes(OFDDocumentPtr ofdDocument, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, ofdDocument, resDescFile))){
}

OFDRes::OFDRes(OFDPagePtr ofdPage, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, ofdPage, resDescFile))){
}

OFDRes::~OFDRes(){
}

OFDResPtr OFDRes::GetSelf() {
    return shared_from_this();
}

std::string OFDRes::GetBaseLoc() const{
    return m_impl->m_baseLoc;
}

Res::Level OFDRes::ImplCls::GetResLevel() const {
    assert(m_ofdPackage.lock() != nullptr);

    if ( m_ofdPage.lock() != nullptr ){
        assert(m_ofdDocument.lock() != nullptr);
        return Res::Level::PAGE;
    } else if ( m_ofdDocument.lock() != nullptr ) {
        return Res::Level::DOCUMENT;
    } else {
        return Res::Level::PACKAGE;
    }
}

Res::Level OFDRes::GetResLevel() const {
    return m_impl->GetResLevel();
}

// FIXME
std::string OFDRes::ImplCls::GetEntryRoot() const{
    Res::Level level = GetResLevel();
    if ( level == Res::Level::PAGE ){
        // FIXME
        return "";
    } else if ( level == Res::Level::DOCUMENT ){
        return "Doc_0";
    } else {
        return "";
    }

}

std::string OFDRes::GetEntryRoot() const{
    return m_impl->GetEntryRoot();
}

const OFDPackagePtr OFDRes::GetOFDPackage() const{
    return m_impl->m_ofdPackage.lock();
}

const OFDDocumentPtr OFDRes::GetOFDDocument() const{
    return m_impl->m_ofdDocument.lock();
}

const OFDPagePtr OFDRes::GetOFDPage() const{
    return m_impl->m_ofdPage.lock();
}

void OFDRes::SetBaseLoc(const std::string &baseLoc){
    m_impl->m_baseLoc = baseLoc;
}

void OFDRes::AddColorSpace(const OFDColorSpace &ofdColorSpace){
    m_impl->AddColorSpace(ofdColorSpace);
}

void OFDRes::AddFont(OFDFontPtr font){
    m_impl->AddFont(font);
}

const FontMap &OFDRes::GetFonts() const {
    return m_impl->GetFonts();
}

const OFDFontPtr OFDRes::GetFont(uint64_t fontID) const {
    return m_impl->GetFont(fontID);
}

std::string OFDRes::GenerateResXML() const{
    return m_impl->GenerateResXML();
}

bool OFDRes::FromResXML(const std::string &strResXML){
    return m_impl->FromResXML(strResXML);
}

bool OFDRes::LoadFonts(){
    return m_impl->LoadFonts();
}

std::string OFDRes::GetResDescFile() const{
    return m_impl->m_resDescFile;
}

OFDResPtr OFDRes::CreateNewRes(OFDPackagePtr ofdPackage, const std::string &resDescFile){

    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(ofdPackage, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}

OFDResPtr OFDRes::CreateNewRes(OFDDocumentPtr ofdDocument, const std::string &resDescFile){
    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(ofdDocument, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}

OFDResPtr OFDRes::CreateNewRes(OFDPagePtr ofdPage, const std::string &resDescFile){
    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(ofdPage, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}


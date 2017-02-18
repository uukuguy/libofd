#include <vector>
#include <assert.h>

#include "OFDRes.h"
#include "ofd/Document.h"
#include "ofd/Package.h"
#include "ofd/Page.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

// **************** class OFDRes::ImplCls ****************

class OFDRes::ImplCls{
public:
    ImplCls(OFDRes *ofdRes, PackagePtr package, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, DocumentPtr document, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, PagePtr page, const std::string &resDescFile);
    ImplCls(OFDRes *ofdRes, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile); 

    ~ImplCls();

    void Init_After_Construct();

    const ColorSpaceArray &GetColorSpaces() const{return m_colorSpaces;};
    void AddColorSpace(const ColorSpacePtr &colorSpace){m_colorSpaces.push_back(colorSpace);};

    void AddFont(FontPtr font);
    const FontMap &GetFonts() const {return m_fonts;};
    const FontPtr GetFont(uint64_t fontID) const;

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

    bool LoadFonts();
    bool LoadImages();

    ResLevel GetResLevel() const;
    std::string GetEntryRoot() const;

private:
    bool FromColorSpacesXML(XMLElementPtr colorSpacesElement);
    bool FromFontsXML(XMLElementPtr fontsElement);

    // -------- Private Attributes --------
public:
    OFDRes* m_ofdRes;
    std::weak_ptr<Package> m_package;
    std::weak_ptr<Document> m_document;
    std::weak_ptr<Page> m_page;
    std::string m_baseLoc;
    std::string m_resDescFile;

    ColorSpaceArray m_colorSpaces;
    FontMap m_fonts;


}; // class OFDRes::ImplCls


OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, PackagePtr package, const std::string &resDescFile) : 
    m_ofdRes(ofdRes),m_package(package), 
    m_baseLoc("Res"), m_resDescFile(resDescFile) {
}

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, DocumentPtr document, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    m_document(document), 
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, PagePtr page, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    m_page(page),
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

void OFDRes::ImplCls::Init_After_Construct(){
    if ( !m_page.expired() ){
        DocumentPtr document = m_page.lock()->GetDocument();
        assert(document != nullptr);
        m_document = document;

        PackagePtr package = document->GetPackage();
        assert(package != nullptr);
        m_package = package;

        if ( m_resDescFile.empty() ){
            m_resDescFile = "PageRes.xml";
        }
    } else {
        if ( !m_document.expired() ){
            PackagePtr package = m_document.lock()->GetPackage();
            assert(package != nullptr);
            m_package = package;

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

OFDRes::ImplCls::ImplCls(OFDRes *ofdRes, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile) :
    m_ofdRes(ofdRes), 
    //m_package(package), m_document(document), m_page(page),
    m_baseLoc("Res"), m_resDescFile(resDescFile){
    
    m_package.reset();
    m_document.reset();
    m_page.reset();

    if ( package != nullptr ){
        m_package = package;
    }

    if ( document != nullptr ){
        m_document = document;
    }

    if ( page != nullptr ){
        m_page = page;
    }

    if ( m_resDescFile.empty() ){
        if ( m_package.lock() != nullptr ){
            if ( m_document.lock() != nullptr ){
                if ( m_page.lock() != nullptr ){
                    m_resDescFile = "PageRes.xml";
                } else {
                    m_resDescFile = "DocumentRes.xml";
                }
            } else {
                m_resDescFile = "PublicRes.xml";
            }
        } else {
            LOG(ERROR) << "m_package == nullptr in OFDRes.";
        }
    }
}

OFDRes::ImplCls::~ImplCls(){
}

void OFDRes::ImplCls::AddFont(FontPtr font){
    uint64_t fontID = font->ID;
    if ( m_fonts.find(fontID) != m_fonts.end() ){
        m_fonts[fontID] = font;
    } else {
        m_fonts.insert(FontMap::value_type(fontID, font));
    }
    font->FontFile = generateFontFileName(font->ID);
}

const FontPtr OFDRes::ImplCls::GetFont(uint64_t fontID) const{
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

    if ( m_package.expired() ){
        return false;
    }

    for ( auto fontIter : m_fonts ){
        auto font = fontIter.second;

        if ( !font->Load(m_package.lock()) ){
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

        FontPtr font = std::make_shared<Font>();
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

OFDRes::OFDRes(PackagePtr package, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, package, resDescFile))){
}

OFDRes::OFDRes(DocumentPtr document, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, document, resDescFile))){
}

OFDRes::OFDRes(PagePtr page, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, page, resDescFile))){
}

OFDRes::~OFDRes(){
}

OFDResPtr OFDRes::GetSelf() {
    return shared_from_this();
}

std::string OFDRes::GetBaseLoc() const{
    return m_impl->m_baseLoc;
}

ResLevel OFDRes::ImplCls::GetResLevel() const {
    assert(m_package.lock() != nullptr);

    if ( m_page.lock() != nullptr ){
        assert(m_document.lock() != nullptr);
        return ResLevel::PAGE;
    } else if ( m_document.lock() != nullptr ) {
        return ResLevel::DOCUMENT;
    } else {
        return ResLevel::PACKAGE;
    }
}

ResLevel OFDRes::GetResLevel() const {
    return m_impl->GetResLevel();
}

// FIXME
std::string OFDRes::ImplCls::GetEntryRoot() const{
    ResLevel level = GetResLevel();
    if ( level == ResLevel::PAGE ){
        // FIXME
        return "";
    } else if ( level == ResLevel::DOCUMENT ){
        return "Doc_0";
    } else {
        return "";
    }

}

std::string OFDRes::GetEntryRoot() const{
    return m_impl->GetEntryRoot();
}

const PackagePtr OFDRes::GetPackage() const{
    return m_impl->m_package.lock();
}

const DocumentPtr OFDRes::GetDocument() const{
    return m_impl->m_document.lock();
}

const PagePtr OFDRes::GetPage() const{
    return m_impl->m_page.lock();
}

void OFDRes::SetBaseLoc(const std::string &baseLoc){
    m_impl->m_baseLoc = baseLoc;
}

const ColorSpaceArray &OFDRes::GetColorSpaces() const{
    return m_impl->GetColorSpaces();
}

void OFDRes::AddColorSpace(const ColorSpacePtr &colorSpace){
    m_impl->AddColorSpace(colorSpace);
}

void OFDRes::AddFont(FontPtr font){
    m_impl->AddFont(font);
}

const FontMap &OFDRes::GetFonts() const {
    return m_impl->GetFonts();
}

const FontPtr OFDRes::GetFont(uint64_t fontID) const {
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

OFDResPtr OFDRes::CreateNewRes(PackagePtr package, const std::string &resDescFile){

    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(package, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}

OFDResPtr OFDRes::CreateNewRes(DocumentPtr document, const std::string &resDescFile){
    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(document, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}

OFDResPtr OFDRes::CreateNewRes(PagePtr page, const std::string &resDescFile){
    OFDResPtr ofdRes = std::shared_ptr<OFDRes>(new OFDRes(page, resDescFile));
    ofdRes->m_impl->Init_After_Construct();
    return ofdRes;
}


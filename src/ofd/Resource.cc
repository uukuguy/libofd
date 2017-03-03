#include <vector>
#include <assert.h>

#include "ofd/Resource.h"
#include "ofd/Document.h"
#include "ofd/Package.h"
#include "ofd/Page.h"
#include "ofd/Font.h"
#include "ofd/Image.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;

// **************** class Resource::ImplCls ****************

class Resource::ImplCls{
public:
    ImplCls(Resource *resource, PackagePtr package, const std::string &resDescFile);
    ImplCls(Resource *resource, DocumentPtr document, const std::string &resDescFile);
    ImplCls(Resource *resource, PagePtr page, const std::string &resDescFile);
    ImplCls(Resource *resource, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile); 

    ~ImplCls();

    void Init_After_Construct();

    const ColorSpaceArray &GetColorSpaces() const{return m_colorSpaces;};
    void AddColorSpace(const ColorSpacePtr &colorSpace){m_colorSpaces.push_back(colorSpace);};

    void AddFont(FontPtr font);
    const FontMap &GetFonts() const {return m_fonts;};
    const FontPtr GetFont(uint64_t fontID) const;

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

    void AddImage(ImagePtr image);
    const ImageMap &GetImages() const {return m_images;};
    const ImagePtr GetImage(uint64_t imageID) const;

    bool LoadFonts();
    bool LoadImages();

    ResourceLevel GetResourceLevel() const;
    std::string GetEntryRoot() const;

private:
    bool FromColorSpacesXML(utils::XMLElementPtr colorSpacesElement);
    bool FromFontsXML(utils::XMLElementPtr fontsElement);

    // -------- Private Attributes --------
public:
    Resource* m_resource;
    std::weak_ptr<Package> m_package;
    std::weak_ptr<Document> m_document;
    std::weak_ptr<Page> m_page;
    std::string m_baseLoc;
    std::string m_resDescFile;

    ColorSpaceArray m_colorSpaces;
    FontMap m_fonts;
    ImageMap m_images;


}; // class Resource::ImplCls


Resource::ImplCls::ImplCls(Resource *resource, PackagePtr package, const std::string &resDescFile) : 
    m_resource(resource),m_package(package), 
    m_baseLoc("Res"), m_resDescFile(resDescFile) {
}

Resource::ImplCls::ImplCls(Resource *resource, DocumentPtr document, const std::string &resDescFile) :
    m_resource(resource), 
    m_document(document), 
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

Resource::ImplCls::ImplCls(Resource *resource, PagePtr page, const std::string &resDescFile) :
    m_resource(resource), 
    m_page(page),
    m_baseLoc("Res"), m_resDescFile(resDescFile){
}

void Resource::ImplCls::Init_After_Construct(){
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

Resource::ImplCls::ImplCls(Resource *resource, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile) :
    m_resource(resource), 
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
            LOG(ERROR) << "m_package == nullptr in Resource.";
        }
    }
}

Resource::ImplCls::~ImplCls(){
}

void Resource::ImplCls::AddFont(FontPtr font){
    uint64_t fontID = font->ID;
    if ( m_fonts.find(fontID) != m_fonts.end() ){
        m_fonts[fontID] = font;
    } else {
        m_fonts.insert(FontMap::value_type(fontID, font));
    }
    font->FontFile = generateFontFileName(font->ID);
}

const FontPtr Resource::ImplCls::GetFont(uint64_t fontID) const{
    auto iter = m_fonts.find(fontID);
    if ( iter != m_fonts.end() ){
        return iter->second;
    } else {
        return nullptr;
    }
}

void generateColorSpacesXML(utils::XMLWriter &writer){
}

void generateDrawParamsXML(utils::XMLWriter &writer){
}

// -------- generateFontsXML() --------
// OFD (section 11.1) P61. Res.xsd.
void generateFontsXML(utils::XMLWriter &writer, const FontMap &fonts){

    for ( auto iter : fonts ){
        auto font = iter.second;

        font->GenerateXML(writer);
    }
}

void generateMultiMediasXML(utils::XMLWriter &writer){
}

void generateCompositeGraphicUnitsXML(utils::XMLWriter &writer){
}

// ======== Resource::ImplCls::GenerateResXML() ========
// OFD (section 7.9) P23. Res.xml.
std::string Resource::ImplCls::GenerateResXML() const{
    utils::XMLWriter writer(true);

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

bool Resource::ImplCls::FromColorSpacesXML(utils::XMLElementPtr colorSpacesElement){
    bool ok = true;

    return ok;
}

bool Resource::ImplCls::LoadFonts(){
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

void Resource::ImplCls::AddImage(ImagePtr image){
    uint64_t imageID = image->ID;

    if ( m_images.find(imageID) != m_images.end() ){
        m_images[imageID] = image;
    } else {
        m_images.insert(ImageMap::value_type(imageID, image));
    }
    image->ImageFile = generateImageFileName(imageID);
}

const ImagePtr Resource::ImplCls::GetImage(uint64_t imageID) const{

    auto iter = m_images.find(imageID);
    if ( iter != m_images.end() ){
        return iter->second;
    } else {
        return nullptr;
    }
}

bool Resource::ImplCls::LoadImages(){
    bool ok = true;

    if ( m_package.expired() ){
        return false;
    }

    for ( auto imageIter : m_images ){
        auto image = imageIter.second;

        if ( !image->Load(m_package.lock()) ){
            LOG(ERROR) << "Load image " << image->ImageFile << " failed.";
        }
    }

    return ok;
}

bool Resource::ImplCls::FromFontsXML(utils::XMLElementPtr fontsElement){
    bool ok = false;

    utils::XMLElementPtr childElement = fontsElement->GetFirstChildElement();
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

// ======== Resource::ImplCls::FromResXML() ========
// OFD (section 7.9) P23. Res.xml.
bool Resource::ImplCls::FromResXML(const std::string &strResXML){
    bool ok = true;

    utils::XMLElementPtr rootElement = utils::XMLElement::ParseRootElement(strResXML);
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

            utils::XMLElementPtr childElement = rootElement->GetFirstChildElement();
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

// **************** class Resource ****************

Resource::Resource(PackagePtr package, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, package, resDescFile))){
}

Resource::Resource(DocumentPtr document, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, document, resDescFile))){
}

Resource::Resource(PagePtr page, const std::string &resDescFile):
    m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, page, resDescFile))){
}

Resource::~Resource(){
}

ResourcePtr Resource::GetSelf() {
    return shared_from_this();
}

std::string Resource::GetBaseLoc() const{
    return m_impl->m_baseLoc;
}

ResourceLevel Resource::ImplCls::GetResourceLevel() const {
    assert(m_package.lock() != nullptr);

    if ( m_page.lock() != nullptr ){
        assert(m_document.lock() != nullptr);
        return ResourceLevel::PAGE;
    } else if ( m_document.lock() != nullptr ) {
        return ResourceLevel::DOCUMENT;
    } else {
        return ResourceLevel::PACKAGE;
    }
}

ResourceLevel Resource::GetResourceLevel() const {
    return m_impl->GetResourceLevel();
}

// FIXME
std::string Resource::ImplCls::GetEntryRoot() const{
    ResourceLevel level = GetResourceLevel();
    if ( level == ResourceLevel::PAGE ){
        // FIXME
        return "";
    } else if ( level == ResourceLevel::DOCUMENT ){
        return "Doc_0";
    } else {
        return "";
    }

}

std::string Resource::GetEntryRoot() const{
    return m_impl->GetEntryRoot();
}

const PackagePtr Resource::GetPackage() const{
    return m_impl->m_package.lock();
}

const DocumentPtr Resource::GetDocument() const{
    return m_impl->m_document.lock();
}

const PagePtr Resource::GetPage() const{
    return m_impl->m_page.lock();
}

void Resource::SetBaseLoc(const std::string &baseLoc){
    m_impl->m_baseLoc = baseLoc;
}

const ColorSpaceArray &Resource::GetColorSpaces() const{
    return m_impl->GetColorSpaces();
}

void Resource::AddColorSpace(const ColorSpacePtr &colorSpace){
    m_impl->AddColorSpace(colorSpace);
}

void Resource::AddFont(FontPtr font){
    m_impl->AddFont(font);
}

const FontMap &Resource::GetFonts() const {
    return m_impl->GetFonts();
}

const FontPtr Resource::GetFont(uint64_t fontID) const {
    return m_impl->GetFont(fontID);
}

std::string Resource::GenerateResXML() const{
    return m_impl->GenerateResXML();
}

bool Resource::FromResXML(const std::string &strResXML){
    return m_impl->FromResXML(strResXML);
}

bool Resource::LoadFonts(){
    return m_impl->LoadFonts();
}

void Resource::AddImage(ImagePtr image){
    m_impl->AddImage(image);
}

const ImageMap& Resource::GetImages() const{
    return m_impl->GetImages();
}

const ImagePtr Resource::GetImage(uint64_t imageID) const {
    return m_impl->GetImage(imageID);
}

bool Resource::LoadImages(){
    return m_impl->LoadImages();
}

std::string Resource::GetResDescFile() const{
    return m_impl->m_resDescFile;
}

ResourcePtr Resource::CreateNewResource(PackagePtr package, const std::string &resDescFile){

    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(package, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}

ResourcePtr Resource::CreateNewResource(DocumentPtr document, const std::string &resDescFile){
    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(document, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}

ResourcePtr Resource::CreateNewResource(PagePtr page, const std::string &resDescFile){
    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(page, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}


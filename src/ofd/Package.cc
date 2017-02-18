#include <assert.h>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "OFDRes.h"
#include "utils/xml.h"
#include "utils/zip.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

Package::Package() :
    Version("1.0"), DocType("OFD"),
    m_opened(false), m_zip(nullptr){
}

Package::Package(const std::string &filename) : 
    Version("1.0"), DocType("OFD"),
    m_filename(filename), m_opened(false), m_zip(nullptr){
}

Package::~Package(){
}

PackagePtr Package::GetSelf(){
    return shared_from_this();
}

// ======== Package::Open() ========
bool Package::Open(const std::string &filename){
    if ( m_opened ) return true;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    m_zip = std::make_shared<utils::Zip>();
    if ( !m_zip->Open(m_filename, false) ){
        LOG(ERROR) << "Error: Open " << m_filename << " failed.";
        return false;
    }

    bool ok = false;
    std::string strOFDXML;
    std::tie(strOFDXML, ok) = ReadZipFileString("OFD.xml");

    if ( ok ) {
        m_opened = fromOFDXML(strOFDXML);
    }

    return m_opened;
}

// ======== Package::Close() ========
void Package::Close(){
    if ( m_opened ){
        if ( m_zip != nullptr ){
            m_zip->Close();
            m_zip = nullptr;
        }
    }
}

const DocumentPtr Package::GetDefaultDocument() const{
    DocumentPtr defaultDocument = nullptr;
    if ( m_documents.size() > 0 ){
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

DocumentPtr Package::GetDefaultDocument(){
    DocumentPtr defaultDocument = nullptr;
    if ( m_documents.size() > 0 ){
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

// ======== Package::Save() ========
bool Package::Save(const std::string &filename){
    //if ( !m_opened ) return false;

    LOG(INFO) << "Save OFD file: " << filename;

    bool ok = false;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    ZipPtr zip = std::make_shared<utils::Zip>();
    if ( !zip->Open(m_filename, true) ){
        LOG(ERROR) << "Error: Open " << m_filename << " failed."; 
        return false;
    }

    // -------- OFD.xml
    std::string strOFDXML = generateOFDXML();
    zip->AddFile("OFD.xml", strOFDXML);


    int n = 0;
    for ( auto document : m_documents ) {

        // -------- mkdir Doc_N
        std::string Doc_N = document->GetDocRoot();
        zip->AddDir(Doc_N);

        // Doc_N/Document.xml
        std::string strDocumentXML;
        strDocumentXML = m_documents[0]->GenerateDocumentXML();
        zip->AddFile(Doc_N + "/Document.xml", strDocumentXML); 

        __attribute__((unused)) const Document::CommonData &commonData = document->GetCommonData();

        // Doc_N/PublicRes.xml
        std::string strPublicResXML;
        if ( commonData.PublicRes != nullptr ){
            strPublicResXML = commonData.PublicRes->GenerateResXML();
        }
        zip->AddFile(Doc_N + "/PublicRes.xml", strPublicResXML); 

        // Doc_N/DocumentRes.xml


        std::string strDocumentResXML;
        if ( commonData.DocumentRes != nullptr ){

            // -------- Default Font --------
            // FIXME
            // Default font. fontID=0, AdobeSongStd-Light.otf
            FontPtr defaultFont = commonData.DocumentRes->GetFont(0);
            if ( defaultFont == nullptr ){
                defaultFont = std::make_shared<Font>();
                defaultFont->ID = 0;
                defaultFont->FontName = "Default";
                defaultFont->FontType = ofd::FontType::TrueType;
                defaultFont->FontLoc = ofd::FontLocation::Embedded;
                commonData.DocumentRes->AddFont(defaultFont);

                char *data = nullptr;
                size_t dataSize = 0;
                bool dataOK = false;
                std::tie(data, dataSize, dataOK) = utils::ReadFileData("data/default.otf");
                if ( dataOK ){
                    defaultFont->CreateFromData(data, dataSize);
                } else {
                    LOG(ERROR) << "Read default font data failed.";
                }
            }

            strDocumentResXML = commonData.DocumentRes->GenerateResXML();
        }
        zip->AddFile(Doc_N + "/DocumentRes.xml", strDocumentResXML); 
        
        // mkdir Doc_N/Pages
        zip->AddDir(Doc_N + "/Pages"); 

        size_t numPages = document->GetNumPages();
        for ( size_t k = 0 ; k < numPages ; k++ ){
            PagePtr page = document->GetPage(k);
            std::string Page_K = std::string("Page_") + std::to_string(k);

            std::string pageDir = Doc_N + "/Pages/" + Page_K;
            zip->AddDir(pageDir);

            // Doc_N/Pages/Page_K/Content.xml
            std::string strPageXML = page->GeneratePageXML();

            zip->AddFile(pageDir + "/Content.xml", strPageXML);

            // Doc_N/Pages/Page_K/PageRes.xml
            std::string strPageResXML;
            zip->AddFile(pageDir + "/PageRes.xml", strPageResXML);

            // mkdir Doc_N/Pages/Page_K/Res
            std::string pageResDir = pageDir + "/Res";
            zip->AddDir(pageResDir);

            for ( auto m = 0 ; m < 1 ; m++ ){
                // Doc_N/Pages/Page_K/Res/Image_M.png
                std::string imageFileName = pageResDir + "/Image_" + std::to_string(m) + ".png";

                std::string strImage;
                zip->AddFile(imageFileName, strImage);
            }
        }

        // mkdir Doc_N/Signs
        std::string signsDir = Doc_N + "/Signs";
        zip->AddDir(signsDir); 

        // Doc_N/Signs/Signatures.xml
        std::string strSignaturesXML;
        zip->AddFile(signsDir + "/Signatures.xml", strSignaturesXML);

        for ( auto m = 0 ; m < 1 ; m++ ){
            // mkdir Doc_N/Signs/Sign_N
            std::string Sign_N = std::string("Sign_") + std::to_string(m);

            std::string signDir = Doc_N + "/Signs/" + Sign_N; 
            zip->AddDir(signDir);

            // Doc_N/Signs/Sign_N/Seal.esl
            std::string strSealESL;
            zip->AddFile(signDir + "/Seal.esl", strSealESL); 

            // Doc_N/Signs/Sign_N/Signature.xml
            std::string strSignatureXML;
            zip->AddFile(signDir + "/Signature.xml", strSignatureXML);
            
            // Doc_N/Signs/Sign_N/SignedValue.dat
            std::string strSignedValueDAT;
            zip->AddFile(signDir + "/SignedValue.dat", strSignedValueDAT);
        }
        
        // mkdir Doc_N/Res
        std::string resDir = Doc_N + "/Res";
        zip->AddDir(resDir); 

        for ( auto m = 0 ; m < 1 ; m++ ){
            // Doc_N/Res/Image_M.png
            std::string imageFileName = resDir + "/Image_" + std::to_string(m) + ".png";

            std::string strImage;
            zip->AddFile(imageFileName, strImage);
        }

        OFDResPtr documentRes = document->GetDocumentRes();
        assert(documentRes != nullptr);

        const FontMap &fonts = documentRes->GetFonts();
        for ( auto iter : fonts){
            auto font = iter.second;
            const char *fontData = font->GetFontData();
            size_t fontDataSize = font->GetFontDataSize();
            if ( fontData != nullptr && fontDataSize > 0 ){
                std::string fontFileName = resDir + "/" + generateFontFileName(font->ID);
                //LOG(ERROR) << "zip->AddFile() while save Font. file = " << fontFileName;
                zip->AddFile(fontFileName, fontData, fontDataSize);
            }
        }

        n++;
    }

    zip->Close();
    zip = nullptr;

    ok = true;
    LOG(INFO) << "Save " << filename << " done.";

    return ok;
}

// -------- Package::generateOFDXML() --------
std::string Package::generateOFDXML() const{
    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <OFD>
    writer.StartElement("OFD");{
        OFDXML_HEAD_ATTRIBUTES;

        // -------- <OFD Version="">
        writer.WriteAttribute("Version", Version);
        // -------- <OFD DocType="">
        writer.WriteAttribute("DocType", DocType);

        for ( auto document : m_documents ){

            // -------- <DocBody>
            writer.StartElement("DocBody");{

                std::string strDocBody = document->GenerateDocBodyXML();
                writer.WriteRaw(strDocBody);

            } writer.EndElement();
        }

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

// ======== Package::AddNewDocument()() ========
DocumentPtr Package::AddNewDocument(){
    size_t idx = m_documents.size();
    std::string docRoot = std::string("Doc_") + std::to_string(idx);
    LOG(DEBUG) << "Calling OFDPackage::AddNewDocument(). docRoot: " << docRoot;

    DocumentPtr document = Document::CreateNewDocument(GetSelf(), docRoot);

    LOG(DEBUG) << "After create document.";
    m_documents.push_back(document);

    return document;
}

// ======== Package::GetDocumentsCount() ========
size_t Package::GetDocumentsCount() const{
    return m_documents.size();
}

// ======== Package::GetDocument() ========
const DocumentPtr Package::GetDocument(size_t idx) const{
    return m_documents[idx];
}

// ======== Package::GetDocument() ========
DocumentPtr Package::GetDocument(size_t idx) {
    return m_documents[idx];
}

// ======== Package::ReadZipFileString() ========
std::tuple<std::string, bool> Package::ReadZipFileString(const std::string &fileinzip) const{
    string content;
    bool ok = false;

    if ( m_zip != nullptr ){
        std::tie(content, ok) = m_zip->ReadFileString(fileinzip);
    }

    return std::make_tuple(content, ok);
}

// ======== Package::ReadZipFileRaw() ========
std::tuple<char*, size_t, bool> Package::ReadZipFileRaw(const std::string &fileinzip) const{
    char *buf = nullptr;
    size_t buflen = 0;
    bool ok = false;

    if ( m_zip != nullptr ){
        std::tie(buf, buflen, ok) = m_zip->ReadFileRaw(fileinzip);
    }

    return std::make_tuple(buf, buflen, ok);
}

// -------- Package::fromOFDXML() --------
// OFD (section 7.4) P6. OFD.xsd
bool Package::fromOFDXML(const std::string &strOFDXML){
    bool ok = true;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strOFDXML);
    if ( rootElement != nullptr ){
        std::string rootName = rootElement->GetName();
        if ( rootName == "OFD" ){

            std::string Version, DocType;
            bool exist = false;

            // -------- <OFD Version="">
            // Required.
            std::tie(Version, exist) = rootElement->GetStringAttribute("Version");
            if ( !exist ){
                LOG(ERROR) << "Attribute Version is Required in OFD.xsd";
                return false;
            }

            // -------- <OFD DocType="">
            // Required.
            std::tie(DocType, exist) = rootElement->GetStringAttribute("DocType");
            if ( !exist ){
                LOG(ERROR) << "Attribute DocType is Required in OFD.xsd";
                return false;
            }

            bool hasDocBody = false;
            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                // -------- <DocBody>
                // Required.
                if ( childName == "DocBody" ){
                    hasDocBody = true;

                    DocumentPtr document = AddNewDocument();
                    document->FromDocBodyXML(childElement);

                    std::string docRoot = document->GetDocRoot();
                    std::string docXMLFile = docRoot + "/Document.xml";
                    LOG(INFO) << "Document xml:" << docXMLFile;

                    std::string strDocumentXML;
                    std::tie(strDocumentXML, ok) = ReadZipFileString(docXMLFile);
                    ok = document->FromDocumentXML(strDocumentXML);
                }

                childElement = childElement->GetNextSiblingElement();
            }
            if ( !hasDocBody ){
            }
        } else {
            LOG(ERROR) << "Root element in OFD.xml is not named 'OFD'";
        }
    } else {
        LOG(ERROR) << "No root element in OFD.xml";
    }

    return ok;
}

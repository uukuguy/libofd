#include <sstream>
#include <assert.h>
//#define ZIP_DISABLE_DEPRECATED
//#include <zip.h>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "utils/logger.h"
#include "utils/xml.h"
#include "utils/zip.h"

using namespace ofd;

// **************** class OFDPackage::ImplCls ****************

class OFDPackage::ImplCls{
public:
    ImplCls(OFDPackage *ofdPackage);
    ImplCls(OFDPackage *ofdPackage, const std::string &filename);
    ~ImplCls();

    std::string to_string() const;

    bool Open(const std::string &filename);
    void Close();
    bool Save(const std::string &filename);

    const OFDDocumentPtr GetDefaultDocument() const;
    OFDDocumentPtr GetDefaultDocument();

    OFDDocumentPtr AddNewDocument();

    std::tuple<std::string, bool> ReadZipFileString(const std::string &fileinzip) const;
    std::tuple<char*, size_t, bool> ReadZipFileRaw(const std::string &fileinzip) const;

public:
    bool FromOFDXML(const std::string &strOFDXML);

    // -------- Private Attributes --------
public:
    OFDPackage *m_ofdPackage;
    std::string   Version;   // 文件格式的版本号，取值为”1.0“
    std::string   DocType;   // 文件格式子集类型，取值为”OFD“，表明此文件符合本标准
                             // 取值为”OFD-A“，表明此文件符合OFD存档规范
    std::string m_filename;  // 包文件绝对路径
    bool m_opened;           // 包文件是否成功打开标志

    typedef std::vector<OFDDocumentPtr> DocumentsList;
    DocumentsList m_documents; // 文件对象入口集合

private:
    utils::ZipPtr m_zip;
    std::string GenerateOFDXML() const;

}; // class OFDPackage::ImplCls

OFDPackage::ImplCls::ImplCls(OFDPackage *ofdPackage) :
    m_ofdPackage(ofdPackage),
    Version("1.0"), DocType("OFD"),
    m_opened(false), m_zip(nullptr){
}

OFDPackage::ImplCls::ImplCls(OFDPackage *ofdPackage, const std::string &filename) :
    m_ofdPackage(ofdPackage),
    Version("1.0"), DocType("OFD"),
    m_filename(filename), m_opened(false){
}

OFDPackage::ImplCls::~ImplCls(){
    //if ( m_archive != nullptr ){
        //zip_close(m_archive);
        //m_archive = nullptr;
    //}
}

std::string OFDPackage::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDPackage ========\n";
    ss << std::endl;
    return ss.str();
}

//std::tuple<std::string, bool> ReadZipFile(zip *archive, const std::string &fileinzip){
    //bool ok = false;
    //std::string fileContent;

    //struct zip_stat st;
    //zip_stat_init(&st);
    //zip_stat(archive, fileinzip.c_str(), ZIP_FL_NOCASE, &st);
    //LOG(DEBUG) << "zip_stat:" << st.valid;

    //size_t filesize = st.size;
    //__attribute__((unused)) size_t compsize = st.comp_size;

    //zip_file *file = zip_fopen(archive, fileinzip.c_str(), ZIP_FL_NOCASE);
    //char *content = new char[filesize + 1];
    //size_t did_read = zip_fread(file, content, filesize);
    //LOG(DEBUG) << "did_read:" << did_read << " filesize:" << filesize;
    //if (did_read != filesize ) {
        //LOG(WARNING) << "File " << fileinzip << " readed " << did_read << " bytes, but is not equal to excepted filesize " << filesize << " bytes.";
        //delete[] content;
    //} else {
        //content[filesize] = '\0';
        //fileContent = std::string(content);
        //ok = true;
        //delete[] content;
    //}
    //zip_fclose(file);

    //return std::make_tuple(fileContent, ok);
//}

std::tuple<std::string, bool> OFDPackage::ImplCls::ReadZipFileString(const std::string &fileinzip) const{
    std::string content;
    bool ok = false;

    if ( m_zip != nullptr ){
        std::tie(content, ok) = m_zip->ReadFileString(fileinzip);
    }

    return std::make_tuple(content, ok);
}

std::tuple<char*, size_t, bool> OFDPackage::ImplCls::ReadZipFileRaw(const std::string &fileinzip) const{
    char *buf = nullptr;
    size_t buflen = 0;
    bool ok = false;

    if ( m_zip != nullptr ){
        std::tie(buf, buflen, ok) = m_zip->ReadFileRaw(fileinzip);
    }

    return std::make_tuple(buf, buflen, ok);
}

bool OFDPackage::ImplCls::Open(const std::string &filename){
    if ( m_opened ) return true;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    m_zip = std::make_shared<utils::Zip>();
    if ( !m_zip->Open(m_filename, false) ){
        LOG(ERROR) << "Error: Open " << m_filename << " failed.";
        return false;
    }

    //int error = 0;
    //m_archive = zip_open(m_filename.c_str(), 0, &error);
    //if ( m_archive == nullptr ){
        //LOG(ERROR) << "Error: Open " << m_filename << " failed. error=" << error;
        //return false;
    //}

    bool ok = false;
    std::string strOFDXML;
    std::tie(strOFDXML, ok) = ReadZipFileString("OFD.xml");

    if ( ok ) {
        m_opened = FromOFDXML(strOFDXML);
    }

    return m_opened;
}

void OFDPackage::ImplCls::Close(){
    if ( m_opened ){
        if ( m_zip != nullptr ){
            m_zip->Close();
            m_zip = nullptr;
        }

        //if ( m_archive != nullptr ){
            //zip_close(m_archive);
            //m_archive = nullptr;
        //}
    }
}

//bool AddZipFile(zip *archive, const std::string &filename, const std::string &text){
    //size_t len = text.length();
    //char *buf = new char[len + 1];
    //memcpy(buf, text.c_str(), len);
    //buf[len] = '\0';
    //[>zip_source *s = zip_source_buffer(archive, text.c_str(), text.length(), 0);<]
    //zip_source *s = zip_source_buffer(archive, buf, len, 1);
    //if ( s == nullptr ) {
        //LOG(ERROR) << "zip_source_buffer_create() failed. filename:" << filename;
        //return false;
    //}
    //int ret = zip_file_add(archive, filename.c_str(), s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    //if ( ret >= 0 ){
        //return true;
    //} else {
        //LOG(ERROR) << "zip_file_add() failed. filename:" << filename;
        //zip_source_free(s);
        //return false;
    //}
//}

//bool AddZipDir(zip *archive, const std::string &dirName){
    //int ret = zip_dir_add(archive, dirName.c_str(), ZIP_FL_ENC_UTF_8);
    //return ret != -1;
//}

std::string OFDPackage::ImplCls::GenerateOFDXML() const{
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


// OFD (section 7.4) P6. OFD.xsd
bool OFDPackage::ImplCls::FromOFDXML(const std::string &strOFDXML){
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

                    OFDDocumentPtr ofdDoc = AddNewDocument();
                    ofdDoc->FromDocBodyXML(childElement);

                    std::string docRoot = ofdDoc->GetDocRoot();
                    std::string docXMLFile = docRoot + "/Document.xml";
                    LOG(INFO) << "Document xml:" << docXMLFile;

                    std::string strDocumentXML;
                    std::tie(strDocumentXML, ok) = ReadZipFileString(docXMLFile);
                    ok = ofdDoc->FromDocumentXML(strDocumentXML);
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

bool OFDPackage::ImplCls::Save(const std::string &filename){
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
    std::string strOFDXML = GenerateOFDXML();
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

        __attribute__((unused)) const OFDDocument::CommonData &commonData = document->GetCommonData();

        // Doc_N/PublicRes.xml
        std::string strPublicResXML;
        if ( commonData.PublicRes != nullptr ){
            strPublicResXML = commonData.PublicRes->GenerateResXML();
        }
        zip->AddFile(Doc_N + "/PublicRes.xml", strPublicResXML); 

        // Doc_N/DocumentRes.xml
        std::string strDocumentResXML;
        if ( commonData.DocumentRes != nullptr ){
            strDocumentResXML = commonData.DocumentRes->GenerateResXML();
        }
        zip->AddFile(Doc_N + "/DocumentRes.xml", strDocumentResXML); 
        
        // mkdir Doc_N/Pages
        zip->AddDir(Doc_N + "/Pages"); 

        size_t numPages = document->GetPagesCount();
        for ( size_t k = 0 ; k < numPages ; k++ ){
            OFDPagePtr page = document->GetPage(k);
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
            if ( font->m_fontData != nullptr && font->m_fontDataSize > 0 ){
                std::string fontFileName = resDir + "/" + font->GetFileName();
                zip->AddFile(fontFileName, font->m_fontData, font->m_fontDataSize);
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

const OFDDocumentPtr OFDPackage::ImplCls::GetDefaultDocument() const{
    OFDDocumentPtr defaultDocument = nullptr;
    if ( m_documents.size() > 0 ){
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

OFDDocumentPtr OFDPackage::ImplCls::GetDefaultDocument(){
    OFDDocumentPtr defaultDocument = nullptr;
    if ( m_documents.size() > 0 ){
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

OFDDocumentPtr OFDPackage::ImplCls::AddNewDocument(){
    size_t idx = m_documents.size();
    std::string docRoot = std::string("Doc_") + std::to_string(idx);
    LOG(DEBUG) << "Calling OFDPackage::AddNewDocument(). docRoot: " << docRoot;

    assert(m_ofdPackage != nullptr);
    //OFDPackagePtr of = m_ofdPackage->GetSelf();
    //LOG(DEBUG) << "before make_shared";

    //OFDDocumentPtr document = std::make_shared<OFDDocument>(of, docRoot);
    //document->InitRes();
    OFDDocumentPtr document = OFDDocument::CreateNewDocument(m_ofdPackage->GetSelf(), docRoot);

    LOG(DEBUG) << "After create document.";
    m_documents.push_back(document);

    return document;
}

// **************** class OFDPackage ****************

OFDPackage::OFDPackage() {
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this));
}

OFDPackage::OFDPackage(const std::string &filename){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(this, filename));
}

OFDPackage::~OFDPackage(){
}

OFDPackagePtr OFDPackage::GetSelf(){
    return shared_from_this();
}

bool OFDPackage::Open(const std::string &filename){
    return m_impl->Open(filename);
}

void OFDPackage::Close(){
    m_impl->Close();
}

bool OFDPackage::IsOpened() const{
    return m_impl->m_opened;
}

const OFDDocumentPtr OFDPackage::GetDefaultDocument() const{
    return m_impl->GetDefaultDocument();
}

OFDDocumentPtr OFDPackage::GetDefaultDocument(){
    return m_impl->GetDefaultDocument();
}

OFDDocumentPtr OFDPackage::AddNewDocument(){
    return m_impl->AddNewDocument();
}

std::string OFDPackage::GetVersion() const{
    return m_impl->Version;
}

std::string OFDPackage::GetDocType() const{
    return m_impl->DocType;
}

size_t OFDPackage::GetDocumentsCount() const{
    return m_impl->m_documents.size();
}

const OFDDocumentPtr OFDPackage::GetDocument(size_t idx) const{
    return m_impl->m_documents[idx];
}

OFDDocumentPtr OFDPackage::GetDocument(size_t idx) {
    return m_impl->m_documents[idx];
}

bool OFDPackage::Save(const std::string &filename){
    return m_impl->Save(filename);
}

std::string OFDPackage::to_string() const{
    return m_impl->to_string();
}

std::tuple<std::string, bool> OFDPackage::ReadZipFileString(const std::string &fileinzip) const{
    return m_impl->ReadZipFileString(fileinzip);
}

std::tuple<char*, size_t, bool> OFDPackage::ReadZipFileRaw(const std::string &fileinzip) const{
    return m_impl->ReadZipFileRaw(fileinzip);
}


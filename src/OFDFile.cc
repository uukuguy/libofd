#include <sstream>
#include <assert.h>
#include <zip.h>
/*#include <tinyxml2.h>*/
/*using namespace tinyxml2;*/
#include "OFDFile.h"
#include "OFDDocument.h"
#include "logger.h"

/*#include <xercesc/dom/DOM.hpp>*/
/*#include <xercesc/dom/DOMDocument.hpp>*/
/*#include <xercesc/dom/DOMDocumentType.hpp>*/
/*#include <xercesc/dom/DOMElement.hpp>*/
/*#include <xercesc/dom/DOMImplementation.hpp>*/
/*#include <xercesc/dom/DOMImplementationLS.hpp>*/
/*#include <xercesc/dom/DOMNodeIterator.hpp>*/
/*#include <xercesc/dom/DOMNodeList.hpp>*/
/*#include <xercesc/dom/DOMText.hpp>*/

/*#include <xercesc/parsers/XercesDOMParser.hpp>*/
/*#include <xercesc/util/XMLUni.hpp>*/

/*using namespace xercesc;*/

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

using namespace ofd;

// **************** class OFDFile::ImplCls ****************

class OFDFile::ImplCls{
public:
    ImplCls();
    ImplCls(const std::string &filename);
    ~ImplCls();

    std::string to_string() const;

    bool Open(const std::string &filename);
    void Close();
    bool Save(const std::string &filename);

    const OFDDocumentPtr GetDefaultDocument() const;
    OFDDocumentPtr GetDefaultDocument();

    OFDDocumentPtr AddNewDocument();

    // -------- Private Attributes --------

    std::string   Version;   // 文件格式的版本号，取值为”1.0“
    std::string   DocType;   // 文件格式子集类型，取值为”OFD“，表明此文件符合本标准
                             // 取值为”OFD-A“，表明此文件符合OFD存档规范
    std::string m_filename;  // 包文件绝对路径
    bool m_opened;           // 包文件是否成功打开标志

    typedef std::vector<OFDDocumentPtr> DocumentsList;
    DocumentsList m_documents; // 文件对象入口集合

private:
    zip *m_archive;
    std::string GenerateOFDXML() const;

}; // class OFDFile::ImplCls

/*bool InitializeXercesC(){*/
    /*bool ok = false;*/
    /*try{*/
        /*XMLPlatformUtils::Initialize();*/
        /*ok = true;*/
    /*} catch ( XMLException &e ){*/
        /*char *message = XMLString::transcode(e.getMessage());*/
        /*LOG(ERROR) << "Initialize xercesC failed. Error: " << message;*/
        /*XMLString::release(&message);*/

    /*}*/
    /*return ok;*/
/*}*/

/*void TerminateXercesC(){*/
    /*try{*/
        /*XMLPlatformUtils::Terminate();*/
    /*} catch ( XMLException &e ){*/
        /*char *message = XMLString::transcode(e.getMessage());*/
        /*LOG(ERROR) << "Terminate xercesC failed. Error: " << message;*/
        /*XMLString::release(&message);*/
    /*}*/
/*}*/

OFDFile::ImplCls::ImplCls() :
    Version("1.0"), DocType("OFD"),
    m_opened(false), m_archive(nullptr){

    /*InitializeXercesC();*/
}

OFDFile::ImplCls::ImplCls(const std::string &filename) :
    Version("1.0"), DocType("OFD"),
    m_filename(filename), m_opened(false){

    /*InitializeXercesC();*/
}

OFDFile::ImplCls::~ImplCls(){
    /*TerminateXercesC();*/
    if ( m_archive != nullptr ){
        zip_close(m_archive);
        m_archive = nullptr;
    }
}

std::string OFDFile::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDFile ========\n";
    ss << std::endl;
    return ss.str();
}

bool OFDFile::ImplCls::Open(const std::string &filename){
    if ( m_opened ) return true;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    return m_opened;
}

void OFDFile::ImplCls::Close(){
    if ( m_opened ){
    }
}

bool AddZipFile(zip *archive, const std::string &filename, const std::string &text){
    zip_source *s = zip_source_buffer(archive, text.c_str(), text.length(), 0);
    if ( s == nullptr ) return false;
    int ret = zip_add(archive, filename.c_str(), s);
    return ret != -1;
}

bool AddZipDir(zip *archive, const std::string &dirName){
    int ret = zip_add_dir(archive, dirName.c_str());
    return ret != -1;
}

#define XMLENCODING "utf-8"

std::string OFDFile::ImplCls::GenerateOFDXML() const{

    xmlBufferPtr xmlBuf = xmlBufferCreate();
    xmlTextWriterPtr writer = xmlNewTextWriterMemory(xmlBuf, 0);
    if ( writer != nullptr ){
        int rc;
        rc = xmlTextWriterStartDocument(writer, nullptr, XMLENCODING, nullptr);
        if ( rc >= 0 ){

            // -------- <OFD>
            xmlTextWriterStartElement(writer, BAD_CAST "OFD");{

                // -------- <OFD Version="">
                xmlTextWriterWriteAttribute(writer, BAD_CAST "Version", BAD_CAST Version.c_str());
                // -------- <OFD DocType="">
                xmlTextWriterWriteAttribute(writer, BAD_CAST "DocType", BAD_CAST DocType.c_str());

                for ( auto document : m_documents ){
                    // -------- <DocBody>
                    xmlTextWriterStartElement(writer, BAD_CAST "DocBody");{

                        std::string strDocBody = document->GenerateDocBodyXML();

                        std::cout << "strDocBody = " << strDocBody << std::endl;

                        xmlTextWriterWriteRaw(writer, BAD_CAST strDocBody.c_str());

                    } xmlTextWriterEndElement(writer);

                }

            } xmlTextWriterEndElement(writer);
            
            xmlTextWriterEndDocument(writer);

        } else {
            LOG(ERROR) << "xmlTextWriterStartDocument() failed.";
        }

        xmlFreeTextWriter(writer);

    } else {
        LOG(ERROR) << "xmlNewTextWriterMemory() failed.";
    }

    std::string strXML = std::string((const char *)xmlBuf->content);
    xmlBufferFree(xmlBuf);

    return strXML;
}

bool OFDFile::ImplCls::Save(const std::string &filename){
    //if ( !m_opened ) return false;

    LOG(INFO) << "Save OFD file: " << filename;

    bool ok = false;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    int error = 0;
    m_archive = zip_open(filename.c_str(), ZIP_CREATE | ZIP_EXCL, &error);
    if ( m_archive == nullptr ){
        if ( error == ZIP_ER_EXISTS ){
            LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error << " The file exists and ZIP_EXCL is set.";
        } else {
            LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error;
        }
        return false;
    }

    // -------- OFD.xml
    std::string strOFDXML = GenerateOFDXML();
    std::cout << strOFDXML << std::endl;

    AddZipFile(m_archive, "OFD.xml", strOFDXML);

    int n = 0;
    for ( auto document : m_documents ) {
        // -------- mkdir Doc_N
        std::stringstream ss;
        ss << "Doc_" << n;
        std::string Doc_N = ss.str();
        AddZipDir(m_archive, Doc_N);

        // Doc_N/Document.xml
        std::string strDocumentXML;
        AddZipFile(m_archive, Doc_N + "/Document.xml", strDocumentXML); 

        // Doc_N/PublicRes.xml
        std::string strPublicResXML;
        AddZipFile(m_archive, Doc_N + "/PublicRes.xml", strPublicResXML); 

        // Doc_N/DocumentRes.xml
        std::string strDocumentResXML;
        AddZipFile(m_archive, Doc_N + "/DocumentRes.xml", strDocumentResXML); 
        
        // mkdir Doc_N/Pages
        AddZipDir(m_archive, Doc_N + "/Pages"); 

        size_t numPages = document->GetPagesCount();
        for ( size_t k = 0 ; k < numPages ; k++ ){
            OFDPagePtr page = document->GetPage(n);

            std::stringstream ssPageK;
            ssPageK << "Page_" << k;
            std::string Page_K = ssPageK.str();

            std::string pageDir = Doc_N + "/Pages/" + Page_K;
            AddZipDir(m_archive, pageDir);

            // Doc_N/Pages/Page_K/Content.xml
            std::string strContentXML;
            AddZipFile(m_archive, pageDir + "/Content.xml", strContentXML);

            // Doc_N/Pages/Page_K/PageRes.xml
            std::string strPageResXML;
            AddZipFile(m_archive, pageDir + "/PageRes.xml", strPageResXML);

            // mkdir Doc_N/Pages/Page_K/Res
            std::string pageResDir = pageDir + "/Res";
            AddZipDir(m_archive, pageResDir);

            for ( auto m = 0 ; m < 1 ; m++ ){
                // Doc_N/Pages/Page_K/Res/Image_M.png
                std::stringstream ssImage;
                ssImage << "Image_" << m << ".png";
                std::string imageFileName = pageResDir + "/" + ssImage.str();

                std::string strImage;
                AddZipFile(m_archive, imageFileName, strImage);
            }

        }

        // mkdir Doc_N/Signs
        std::string signsDir = Doc_N + "/Signs";
        AddZipDir(m_archive, signsDir); 

        // Doc_N/Signs/Signatures.xml
        std::string strSignaturesXML;
        AddZipFile(m_archive, signsDir + "/Signatures.xml", strSignaturesXML);

        for ( auto m = 0 ; m < 1 ; m++ ){
            // mkdir Doc_N/Signs/Sign_N
            std::stringstream ssSignN;
            ssSignN << "Sign_" << m;
            std::string Sign_N = ssSignN.str();
            std::string signDir = Doc_N + "/Signs/" + Sign_N; 
            AddZipDir(m_archive, signDir);
            // Doc_N/Signs/Sign_N/Seal.esl
            std::string strSealESL;
            AddZipFile(m_archive, signDir + "/Seal.esl", strSealESL); 
            // Doc_N/Signs/Sign_N/Signature.xml
            std::string strSignatureXML;
            AddZipFile(m_archive, signDir + "/Signature.xml", strSignatureXML);
            // Doc_N/Signs/Sign_N/SignedValue.dat
            std::string strSignedValueDAT;
            AddZipFile(m_archive, signDir + "/SignedValue.dat", strSignedValueDAT);
        }
        
        // mkdir Doc_N/Res
        std::string resDir = Doc_N + "/Res";
        AddZipDir(m_archive, resDir); 

        for ( auto m = 0 ; m < 1 ; m++ ){
            // Doc_N/Res/Image_M.png
            std::stringstream ssImage;
            ssImage << "Image_" << m << ".png";
            std::string imageFileName = resDir + "/" + ssImage.str();

            std::string strImage;
            AddZipFile(m_archive, imageFileName, strImage);
        }

        for ( auto m = 0 ; m < 1 ; m++ ){
            // Doc_N/Res/Font_M.ttf
            std::stringstream ssFont;
            ssFont << "Font_" << m << ".ttf";
            std::string fontFileName = resDir + "/" + ssFont.str();

            std::string strFont;
            AddZipFile(m_archive, fontFileName, strFont);
        }
        
        n++;
    }

    zip_close(m_archive);
    m_archive = nullptr;

    ok = true;
    LOG(INFO) << "Save " << filename << " done.";

    return ok;
}

const OFDDocumentPtr OFDFile::ImplCls::GetDefaultDocument() const{
    assert( m_documents.size() > 0 );
    const OFDDocumentPtr defaultDocument = m_documents[0];
    return defaultDocument;
}

OFDDocumentPtr OFDFile::ImplCls::GetDefaultDocument(){
    assert( m_documents.size() > 0 );
    OFDDocumentPtr defaultDocument = m_documents[0];
    return defaultDocument;
}

OFDDocumentPtr OFDFile::ImplCls::AddNewDocument(){
    OFDDocumentPtr document = std::make_shared<OFDDocument>();
    m_documents.push_back(document);
    return document;
}

// **************** class OFDFile ****************

OFDFile::OFDFile() {
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());

    AddNewDocument();
}

OFDFile::OFDFile(const std::string &filename){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(filename));
}

OFDFile::~OFDFile(){
}

bool OFDFile::Open(const std::string &filename){
    return m_impl->Open(filename);
}

void OFDFile::Close(){
    m_impl->Close();
}

bool OFDFile::IsOpened() const{
    return m_impl->m_opened;
}

const OFDDocumentPtr OFDFile::GetDefaultDocument() const{
    return m_impl->GetDefaultDocument();
}

OFDDocumentPtr OFDFile::GetDefaultDocument(){
    return m_impl->GetDefaultDocument();
}

OFDDocumentPtr OFDFile::AddNewDocument(){
    return m_impl->AddNewDocument();
}

std::string OFDFile::GetVersion() const{
    return m_impl->Version;
}

std::string OFDFile::GetDocType() const{
    return m_impl->DocType;
}

size_t OFDFile::GetDocumentsCount() const{
    return m_impl->m_documents.size();
}

const OFDDocumentPtr OFDFile::GetDocument(size_t idx) const{
    return m_impl->m_documents[idx];
}

OFDDocumentPtr OFDFile::GetDocument(size_t idx) {
    return m_impl->m_documents[idx];
}

bool OFDFile::Save(const std::string &filename){
    return m_impl->Save(filename);
}

std::string OFDFile::to_string() const{
    return m_impl->to_string();
}


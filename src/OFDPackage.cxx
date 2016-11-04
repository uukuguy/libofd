#include <iostream>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "utils.h"
#include "logger.h"
#include <tinyxml2.h>
using namespace tinyxml2;


using namespace ofd;

OFDPackage::OFDPackage() : m_document(nullptr), m_opened(false), m_zip(nullptr){

}

OFDPackage::~OFDPackage(){
    if ( m_opened ){
        Close();
    }
}

bool OFDPackage::Open(const std::string& filename){
    if ( m_opened ) {
        LOG(DEBUG) << "Package is already opened.";
        return true;
    }

    this->m_filename = filename;
    int error = 0;
    m_zip = zip_open(filename.c_str(), 0, &error);
    if ( m_zip == nullptr ){
        LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error;
        return false;
    }

    m_opened = init();

    return m_opened;
}

bool OFDPackage::init() {
    if ( !checkFilesInZIP() ) {
        LOG(WARNING) << "checkFilesINZIP() failed";
        return false;
    }

    if ( !initAttributes() ) {
        LOG(WARNING) << "initAttributes() failed.";
        return false;
    }

    if ( !initRootDocument() ) {
        LOG(WARNING) << "initRootDocument() failed.";
        return false;
    }

    return true;
}

void OFDPackage::Close(){
    if (!m_opened) return;

    if ( m_document != nullptr ){
        //delete m_document;
        m_document = nullptr;
    }

    zip_close(m_zip);
    m_zip = nullptr;

    m_opened = false;
}

size_t OFDPackage::getZipFileSize(zip* handle, const char *filename){
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(m_zip, filename, ZIP_FL_NOCASE, &st);
    return st.size;
}

bool OFDPackage::ReadFile(const std::string &filename, char **buffer, size_t *bufSize) const {
    bool ok = false;
    std::string fileContent;

    auto it = m_files.find(filename);
    if ( it != m_files.end() ){
        std::string filename = it->first;
        size_t filesize = it->second;
        LOG(DEBUG) << "filesize:" << filesize;

        zip_file *file = zip_fopen(m_zip, filename.c_str(), ZIP_FL_NOCASE);
        char *content = new char[filesize+1];
        size_t did_read = zip_fread(file, content, filesize);
        LOG(DEBUG) << "After zip_frea() did_read:" << did_read << " , while filesize=" << filesize;
        if (did_read != filesize ) {
            LOG(WARNING) << "File " << filename << " readed " << did_read << " bytes, but is not equal to excepted filesize " << filesize << " bytes.";
            delete[] content;
        } else {
            content[filesize] = '\0';
            *buffer = content;
            *bufSize = filesize;
            ok = true;
        }
        //if ( did_read > 0 ){
            //if ( strlen(content) < filesize ){
                //LOG(WARNING) << "File " << filename << " is truncated. from " << filesize << " to " << strlen(content) << " did_read: " << did_read;
            //}
            //if ( strlen(content) > filesize ){
                //LOG(WARNING) << "File readed " << strlen(content) << " more then " << filesize << ". did_read: " << did_read;
                //content[filesize] = '\0';
            //}

            //if ( VLOG_IS_ON(5) ){
                //VLOG(5) << "\n[ " << filename << " ]\n\n" << content << "\n\n--------\n\n";
            //}

            //fileContent = std::string(content);
            //ok = true;
            //delete[] content;
        //}
        zip_fclose(file);
    } else {
        LOG(ERROR) << filename << " is not exist in zipfile.";
    }

    return ok;
}

std::tuple<std::string, bool> OFDPackage::GetFileContent(const std::string &filename) const {
    bool ok = false;
    std::string fileContent;

    auto it = m_files.find(filename);
    if ( it != m_files.end() ){
        std::string filename = it->first;
        size_t filesize = it->second;
        LOG(DEBUG) << "filesize:" << filesize;

        zip_file *file = zip_fopen(m_zip, filename.c_str(), ZIP_FL_NOCASE);
        char *content = new char[filesize];
        size_t did_read = zip_fread(file, content, filesize);
        LOG(DEBUG) << "did_read:" << did_read;
        if (did_read != filesize ) {
            LOG(WARNING) << "File " << filename << " readed " << did_read << " bytes, but is not equal to excepted filesize " << filesize << " bytes.";
            delete[] content;
        } else {
            content[filesize] = '\0';
            fileContent = std::string(content);
            ok = true;
            delete[] content;
        }
        //if ( did_read > 0 ){
            //if ( strlen(content) < filesize ){
                //LOG(WARNING) << "File " << filename << " is truncated. from " << filesize << " to " << strlen(content) << " did_read: " << did_read;
            //}
            //if ( strlen(content) > filesize ){
                //LOG(WARNING) << "File readed " << strlen(content) << " more then " << filesize << ". did_read: " << did_read;
                //content[filesize] = '\0';
            //}

            //if ( VLOG_IS_ON(5) ){
                //VLOG(5) << "\n[ " << filename << " ]\n\n" << content << "\n\n--------\n\n";
            //}

            //fileContent = std::string(content);
            //ok = true;
            //delete[] content;
        //}
        zip_fclose(file);
    } else {
        LOG(ERROR) << filename << " is not exist in zipfile.";
    }


    return std::make_tuple(fileContent, ok);
}

bool OFDPackage::checkFilesInZIP(){
    m_files.clear();
    zip_int64_t n_entries = zip_get_num_entries(m_zip, ZIP_FL_UNCHANGED);
    LOG(DEBUG) << n_entries << " entries in ofd file " << m_filename;

    for ( zip_int64_t i = 0 ; i < n_entries ; i++ ){
        const char *filename = zip_get_name(m_zip, i, ZIP_FL_ENC_GUESS);

        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(m_zip, filename, ZIP_FL_NOCASE, &st);

        size_t filesize = st.size;
        size_t compsize = st.comp_size;

        LOG(DEBUG) << "[" << i << "] " << filename << " size:  " << filesize << " comp_size: " <<  compsize; 

        m_files.insert(std::map<std::string, size_t>::value_type(filename, filesize));
    }
    return true;
}

std::string OFDPackage::String() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDPackage" << std::endl;

        ss << "Title: " << m_attributes.Title << std::endl;
        ss << "Author: " << m_attributes.Author << std::endl;
        ss << "Subject: " << m_attributes.Subject << std::endl;
        ss << "DocID: " << m_attributes.DocID << std::endl;
        ss << "Creator: " << m_attributes.Creator << std::endl;
        ss << "CreatorVersion: " << m_attributes.CreatorVersion << std::endl;
        ss << "CreationDate: " << m_attributes.CreationDate << std::endl;

        ss << std::endl;
        ss << "DocRoot: " << m_attributes.DocRoot << std::endl;

        ss << std::endl
           << "------------------------------" << std::endl;

    return ss.str();
}

bool OFDPackage::initAttributes(){
    bool ok = false;
    std::string content;
    std::tie(content, ok) = GetFileContent("OFD.xml");
    if ( !ok ) return false;

    ok = parseAttributesXML(content);

    LOG(DEBUG) << "DocRoot: " << GetDocRoot();

    return ok;
}

bool OFDPackage::parseAttributesXML(const std::string &content){
    XMLDocument *xmldoc = new XMLDocument();
    XMLError rc = xmldoc->Parse(content.c_str());
    if ( rc != XML_SUCCESS ){
        delete xmldoc;
        return false;
    }

    XMLElement *rootElement = xmldoc->RootElement();
    if ( rootElement != nullptr ){
        LOG(DEBUG) << "Root Element Name: " << rootElement->Name();
        VLOG(3) << GetChildElements(rootElement);

        const XMLElement *bodyElement = rootElement->FirstChildElement("ofd:DocBody");
        if ( bodyElement != nullptr ){
            VLOG(3) << GetChildElements(bodyElement);

            const XMLElement *docInfoElement = bodyElement->FirstChildElement("ofd:DocInfo");
            if ( docInfoElement != nullptr ){
                VLOG(3) << GetChildElements(docInfoElement);

                GetChildElementText(docInfoElement, ofd:Title, m_attributes.Title); 
                GetChildElementText(docInfoElement, ofd:Author, m_attributes.Author); 
                GetChildElementText(docInfoElement, ofd:Subject, m_attributes.Subject); 
                GetChildElementText(docInfoElement, ofd:DocID, m_attributes.DocID); 
                GetChildElementText(docInfoElement, ofd:Creator, m_attributes.Creator); 
                GetChildElementText(docInfoElement, ofd:CreatorVersion, m_attributes.CreatorVersion); 
                GetChildElementText(docInfoElement, ofd:CreationDate, m_attributes.CreationDate); 
            }

            GetChildElementText(bodyElement, ofd:DocRoot, m_attributes.DocRoot);
            //const XMLElement *docRootElement = bodyElement->FirstChildElement("ofd:DocRoot");
            //if ( docRootElement != nulptr ){
                //m_attributes.DocRoot = docRootElement->GetText();
            //}

            LOG(INFO) << String();
        }
    } else {
        LOG(ERROR) << "rootElement == nullptr";
    }
    delete xmldoc;
    xmldoc = nullptr;

    return true;
}

bool OFDPackage::initRootDocument() {
    bool ok = false;

    //m_document = new OFDDocument(this, GetDocRoot());
    m_document = OFDDocumentPtr(new OFDDocument(this, GetDocRoot()));
    ok = m_document->Open();

    return ok;
}



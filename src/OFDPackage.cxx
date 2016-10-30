#include <iostream>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "utils.h"
#include "logger.h"
#include <tinyxml2.h>
using namespace tinyxml2;


using namespace ofd;

OFDPackage::OFDPackage() : m_ofdDocument(NULL), m_zip(NULL){

}

OFDPackage::~OFDPackage(){
    if ( opened ){
        Close();
    }
}

bool OFDPackage::Open(const std::string& filename){
    if ( opened )return true;

    this->m_filename = filename;
    int error = 0;
    m_zip = zip_open(filename.c_str(), 0, &error);
    if ( m_zip == NULL ){
        LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error;
        return false;
    }

    opened = init();

    return opened;
}

bool OFDPackage::init() {
    if ( !checkFilesInZIP() ) return false;

    if ( !initAttributes() ) return false;

    if ( !initRootDocument() ) return false;

    return true;
}

void OFDPackage::Close(){
    if (!opened) return;

    if ( m_ofdDocument != NULL ){
        delete m_ofdDocument;
        m_ofdDocument = NULL;
    }

    zip_close(m_zip);
    m_zip = NULL;

    opened = false;
}

size_t OFDPackage::getZipFileSize(zip* handle, const char *filename){
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(m_zip, filename, ZIP_FL_NOCASE, &st);
    return st.size;
}

std::tuple<std::string, bool> OFDPackage::GetFileContent(const std::string &filename) const {
    bool ok = false;
    std::string fileContent;
    zip_int64_t len = 0;
    size_t sum = 0;
    char buf[100] = {0};

    std::map<std::string, size_t>::const_iterator it = m_files.find(filename);
    if ( it != m_files.end() ){
        std::string filename = it->first;
        size_t filesize = it->second;

        zip_file *file = zip_fopen(m_zip,filename.c_str(), ZIP_FL_NOCASE);
        if(file){
            while( sum != filesize ){
                len = zip_fread(file,buf,100);
                if( len < 0 ){
                    LOG(ERROR) << "zip_fread error\n";
                    zip_fclose(file);
                    return std::make_tuple("",ok);
                }
                fileContent.append(buf,len);
                sum += len;
            }
            zip_fclose(file);
            LOG(INFO) << "filesize:" << filesize << " fileContent size:" << fileContent.size();
            ok = true;
        }


        //zip_file *file = zip_fopen(m_zip, filename.c_str(), ZIP_FL_NOCASE);
        //char *content = new char[filesize];
        //const zip_int64_t did_read = zip_fread(file, content, filesize);
        //if ( did_read > 0 ){
        //    if ( strlen(content) < filesize ){
        //        LOG(WARNING) << "File " << filename << " is truncated. from " << filesize << " to " << strlen(content);
        //    }
        //    if ( strlen(content) > filesize ){
        //        LOG(WARNING) << "File readed " << strlen(content) << " more then " << filesize;
        //        content[filesize] = '\0';
        //    }

        //    if ( VLOG_IS_ON(5) ){
        //        VLOG(5) << "\n[ " << filename << " ]\n\n" << content << "\n\n--------\n\n";
        //    }

        //    fileContent = std::string(content);
        //    ok = true;
        //    delete[] content;
        //}
        //zip_fclose(file);
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
    if ( rootElement != NULL ){
        LOG(DEBUG) << "Root Element Name: " << rootElement->Name();
        VLOG(3) << GetChildElements(rootElement);

        const XMLElement *bodyElement = rootElement->FirstChildElement("ofd:DocBody");
        if ( bodyElement != NULL ){
            VLOG(3) << GetChildElements(bodyElement);

            const XMLElement *docInfoElement = bodyElement->FirstChildElement("ofd:DocInfo");
            if ( docInfoElement != NULL ){
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
            //if ( docRootElement != NULL ){
                //m_attributes.DocRoot = docRootElement->GetText();
            //}

            LOG(INFO) << String();
        }
    } else {
        LOG(ERROR) << "rootElement == NULL";
    }
    delete xmldoc;
    xmldoc = NULL;

    return true;
}

bool OFDPackage::initRootDocument() {
    bool ok = false;
    this->m_ofdDocument = new OFDDocument(this, GetDocRoot());
    ok = this->m_ofdDocument->Open();

    return ok;
}



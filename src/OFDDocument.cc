#include <sstream>
#include <libxml/xmlwriter.h>
#include "logger.h"
#include "OFDDocument.h"
#include "OFDPage.h"

using namespace ofd;

// **************** class OFDDocument::ImplCls ****************

class OFDDocument::ImplCls{
public:
    ImplCls();
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    OFDPagePtr AddNewPage();
    std::string GenerateDocBodyXML() const;

    // -------- Private Attributes --------
    bool m_opened;

    DocBody    m_docBody;
    CommonData m_commonData;
    std::vector<OFDPagePtr> m_pages;

}; // class OFDDocument::ImplCls

OFDDocument::ImplCls::ImplCls() : m_opened(false) {
}

OFDDocument::ImplCls::~ImplCls(){
}

std::string OFDDocument::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDDocument ========\n";
    ss << "Pages: " << m_pages.size() << "\n";
    ss << std::endl;
    return ss.str();
}

bool OFDDocument::ImplCls::Open(){
    if ( m_opened ) return true;

    return m_opened;
}

void OFDDocument::ImplCls::Close(){
    if ( !m_opened ) return;
}

OFDPagePtr OFDDocument::ImplCls::AddNewPage(){
    OFDPagePtr page = std::make_shared<OFDPage>();
    m_pages.push_back(page);
    return page;
}

class XMLWriter{
public:
    XMLWriter(){
        m_xmlBuf = xmlBufferCreate();
        m_writer = xmlNewTextWriterMemory(m_xmlBuf, 0);
    }

    ~XMLWriter(){

        xmlFreeTextWriter(m_writer);
        xmlBufferFree(m_xmlBuf);
    }

    std::string GetString() const {
        return std::string((const char *)m_xmlBuf->content);
    }

    void StartElement(const std::string &name){
        xmlTextWriterStartElement(m_writer, BAD_CAST name.c_str());
    }

    void EndElement(){
        xmlTextWriterEndElement(m_writer);
    }

    void WriteElement(const std::string &name, const std::string &value){
        xmlTextWriterWriteElement(m_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
    }

    void WriteAttribute(const std::string &name, const std::string &value){
        xmlTextWriterWriteAttribute(m_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
    }


    void WriteText(const std::string &text){
        xmlTextWriterWriteString(m_writer, BAD_CAST text.c_str());
    }

    void StartDocument(const std::string &encoding){
        xmlTextWriterStartDocument(m_writer, nullptr, encoding.c_str(), nullptr);
    }

    void EndDocument(){
        xmlTextWriterEndElement(m_writer);
    }

private:
    xmlBufferPtr     m_xmlBuf;
    xmlTextWriterPtr m_writer;
};

std::string OFDDocument::ImplCls::GenerateDocBodyXML() const{

    /*XMLWriter writer;*/
    /*writer.StartElement("DocInfo");*/
    /*const CT_DocInfo &docInfo = m_docBody.DocInfo;*/
    /*writer.WriteElement("DocID", docInfo.DocID);*/
    /*writer.EndElement();*/
    /*writer.EndDocument();*/

    /*std::string strXML = writer.GetString();*/

    /*return strXML;*/

    xmlBufferPtr xmlBuf = xmlBufferCreate();
    xmlTextWriterPtr writer = xmlNewTextWriterMemory(xmlBuf, 0);
    if ( writer != nullptr ){

        // -------- <DocInfo> 必选
        xmlTextWriterStartElement(writer, BAD_CAST "DocInfo");{

            const CT_DocInfo &docInfo = m_docBody.DocInfo;

            xmlTextWriterWriteElement(writer, BAD_CAST "DocID", BAD_CAST docInfo.DocID.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "Title", BAD_CAST docInfo.Title.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "Author", BAD_CAST docInfo.Author.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "Subject", BAD_CAST docInfo.Subject.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "Abstract", BAD_CAST docInfo.Abstract.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "CreationDate", BAD_CAST docInfo.CreationDate.c_str());
            xmlTextWriterWriteElement(writer, BAD_CAST "ModDate", BAD_CAST docInfo.ModDate.c_str());
            // FIXME
            /*xmlTextWriterWriteElement(writer, BAD_CAST "DocUsage", BAD_CAST docInfo.DocUsage.c_str());*/
            xmlTextWriterWriteElement(writer, BAD_CAST "Cover", BAD_CAST docInfo.Cover.c_str());

        } xmlTextWriterEndElement(writer);
        
        // -------- <DocRoot>

        // -------- <Versions>

        // -------- <Signatures>

        xmlTextWriterEndDocument(writer);
    } else {
        LOG(ERROR) << "xmlNewTextWriterMemory() failed.";
    }

    std::string strXML = std::string((const char *)xmlBuf->content);
    xmlBufferFree(xmlBuf);

    
    return strXML;
}

// **************** class OFDDocument ****************

OFDDocument::OFDDocument(){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());
}

OFDDocument::~OFDDocument(){
}

const DocBody& OFDDocument::GetDocBody() const{
    return m_impl->m_docBody;
}

DocBody& OFDDocument::GetDocBody(){
    return m_impl->m_docBody;
}

const OFDDocument::CommonData& OFDDocument::GetCommonData() const{
    return m_impl->m_commonData;
}

OFDDocument::CommonData& OFDDocument::GetCommonData(){
    return m_impl->m_commonData;
}

size_t OFDDocument::GetPagesCount() const{
    return m_impl->m_pages.size();
}

const OFDPagePtr OFDDocument::GetPage(size_t idx) const{
    return m_impl->m_pages[idx];
}

OFDPagePtr OFDDocument::GetPage(size_t idx){
    return m_impl->m_pages[idx];
}

OFDPagePtr OFDDocument::AddNewPage(){
    return m_impl->AddNewPage();
}

std::string OFDDocument::GenerateDocBodyXML() const{
    return m_impl->GenerateDocBodyXML();
}

std::string OFDDocument::to_string() const {
    return m_impl->to_string();
}

bool OFDDocument::Open() {
    return m_impl->Open();
}

void OFDDocument::Close(){
    m_impl->Close();
}

bool OFDDocument::IsOpened() const{
    return m_impl->m_opened;
}


#include <sstream>
#include <iomanip>
#include <libxml/xmlwriter.h>
#include "xml.h"

// **************** class XMLWriter::ImplCls ****************

class XMLWriter::ImplCls{
public:

    ImplCls(bool bHead);
    ~ImplCls();

    std::string GetString() const;

    void StartDocument(const std::string &encoding = "utf-8");
    void StartElement(const std::string &name);
    void EndElement();
    void WriteElement(const std::string &name, const std::string &value);
    void WriteElement(const std::string &name, uint64_t value);
    void WriteElement(const std::string &name, double value);
    void WriteAttribute(const std::string &name, const std::string &value);
    void WriteRaw(const std::string &text);
    void WriteString(const std::string &text);
    void EndDocument();

    // -------- Private Attributes --------

    bool             m_bHead;
    xmlBufferPtr     m_xmlBuf;
    xmlTextWriterPtr m_writer;
};

XMLWriter::ImplCls::ImplCls(bool bHead) : m_bHead(bHead) {
    m_xmlBuf = xmlBufferCreate();
    m_writer = xmlNewTextWriterMemory(m_xmlBuf, 0);
}

XMLWriter::ImplCls::~ImplCls(){
    xmlFreeTextWriter(m_writer);
    xmlBufferFree(m_xmlBuf);
}

std::string XMLWriter::ImplCls::GetString() const {
    return std::string((const char *)m_xmlBuf->content);
}

void XMLWriter::ImplCls::StartDocument(const std::string &encoding){
    if ( m_bHead ){
        xmlTextWriterStartDocument(m_writer, nullptr, encoding.c_str(), nullptr);
    }
}

void XMLWriter::ImplCls::StartElement(const std::string &name){
    xmlTextWriterStartElement(m_writer, BAD_CAST name.c_str());
}

void XMLWriter::ImplCls::EndElement(){
    xmlTextWriterEndElement(m_writer);
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, const std::string &value){
    xmlTextWriterWriteElement(m_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, uint64_t value){
    std::stringstream ss;
    ss << value;
    WriteElement(name, ss.str());    
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, double value){
    std::stringstream ss;
    ss << std::setprecision(6) << value;
    WriteElement(name, ss.str());    
}

void XMLWriter::ImplCls::WriteAttribute(const std::string &name, const std::string &value){
    xmlTextWriterWriteAttribute(m_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

void XMLWriter::ImplCls::WriteRaw(const std::string &text){
    xmlTextWriterWriteRaw(m_writer, BAD_CAST text.c_str());
}

void XMLWriter::ImplCls::WriteString(const std::string &text){
    xmlTextWriterWriteString(m_writer, BAD_CAST text.c_str());
}

void XMLWriter::ImplCls::EndDocument(){
    xmlTextWriterEndDocument(m_writer);
}
// **************** class XMLWriter::ImplCls ****************

XMLWriter::XMLWriter(bool bHead) : 
    m_impl(std::unique_ptr<XMLWriter::ImplCls>(new XMLWriter::ImplCls(bHead))){
}

XMLWriter::~XMLWriter(){
}

std::string XMLWriter::GetString() const {
    return m_impl->GetString();
}

void XMLWriter::StartDocument(const std::string &encoding){
    m_impl->StartDocument(encoding);
}

void XMLWriter::StartElement(const std::string &name){
    m_impl->StartElement(name);
}

void XMLWriter::EndElement(){
    m_impl->EndElement();
}

void XMLWriter::WriteElement(const std::string &name, const std::string &value){
    m_impl->WriteElement(name, value);
}

void XMLWriter::WriteElement(const std::string &name, uint64_t value){
    m_impl->WriteElement(name, value);
}

void XMLWriter::WriteElement(const std::string &name, double value){
    m_impl->WriteElement(name, value);
}

void XMLWriter::WriteAttribute(const std::string &name, const std::string &value){
    m_impl->WriteAttribute(name, value);
}

void XMLWriter::WriteRaw(const std::string &text){
    m_impl->WriteRaw(text);
}

void XMLWriter::WriteString(const std::string &text){
    m_impl->WriteString(text);
}

void XMLWriter::EndDocument(){
    m_impl->EndDocument();
}

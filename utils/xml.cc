#include <sstream>
#include <iomanip>
#include <assert.h>
#include <libxml/xmlwriter.h>
#include "xml.h"
#include "utils/logger.h"

using namespace utils;

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
    void WriteAttribute(const std::string &name, uint64_t value);
    void WriteAttribute(const std::string &name, double value);
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
    xmlTextWriterStartElement(m_writer, BAD_CAST (std::string("ofd:") + name).c_str());
}

void XMLWriter::ImplCls::EndElement(){
    xmlTextWriterEndElement(m_writer);
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, const std::string &value){
    xmlTextWriterWriteElement(m_writer, BAD_CAST (std::string("ofd:") + name).c_str(), BAD_CAST value.c_str());
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, uint64_t value){
    WriteElement(name, std::to_string(value));    
}

void XMLWriter::ImplCls::WriteElement(const std::string &name, double value){
    WriteElement(name, std::to_string(value));    
}

void XMLWriter::ImplCls::WriteAttribute(const std::string &name, const std::string &value){
    xmlTextWriterWriteAttribute(m_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

void XMLWriter::ImplCls::WriteAttribute(const std::string &name, uint64_t value){
    WriteAttribute(name, std::to_string(value));    
}

void XMLWriter::ImplCls::WriteAttribute(const std::string &name, double value){
    WriteAttribute(name, std::to_string(value));
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

void XMLWriter::WriteAttribute(const std::string &name, uint64_t value){
    m_impl->WriteAttribute(name, value);
}

void XMLWriter::WriteAttribute(const std::string &name, double value){
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


#include <libxml/xmlreader.h>
#include <libxml/parser.h>

// **************** class XMLReader::ImplCls ****************

class XMLReader::ImplCls{
public:

    ImplCls();
    ~ImplCls();

    bool ParseXML(const std::string &xmlString);
    bool HasElement() const;
    void NextElement();
    bool EnterChildElement(const std::string &name);
    void BackParentElement();
    bool CheckElement(const std::string &name);

    bool ReadElement(std::string &value);
    bool ReadElement(uint64_t &value);
    bool ReadElement(double &value);
    bool ReadAttribute(const std::string &name, std::string &value);
    bool ReadAttribute(const std::string &name, uint64_t &value);
    bool ReadAttribute(const std::string &name, double &value);

private:

    xmlNodePtr m_currentNode;

    std::vector<xmlNodePtr> m_nodeStack;

}; // class XMLReader::ImplCls

XMLReader::ImplCls::ImplCls() : m_currentNode(nullptr){
 }

XMLReader::ImplCls::~ImplCls(){
    xmlCleanupParser();
}

bool XMLReader::ImplCls::ParseXML(const std::string &xmlString){
    xmlDocPtr xmlDoc = xmlParseMemory(xmlString.c_str(), xmlString.length());
    if ( xmlDoc != nullptr ){
        m_currentNode = xmlDocGetRootElement(xmlDoc);
        m_nodeStack.clear();
        return true;
    }
    return false;
}

bool XMLReader::ImplCls::HasElement() const{ 

    return m_currentNode != nullptr;
}

void XMLReader::ImplCls::NextElement(){
    if ( m_currentNode != nullptr ){

        xmlNodePtr node = m_currentNode;
        while ( node != nullptr ){
            node = xmlNextElementSibling(node);
            if ( node != nullptr && node->type == XML_ELEMENT_NODE ){
                break;
            }
        }
        m_currentNode = node;
    }
}

bool XMLReader::ImplCls::EnterChildElement(const std::string &name){
    assert(m_currentNode != nullptr &&
            std::string((const char *)m_currentNode->name) == name);

    xmlNodePtr childNode = xmlFirstElementChild(m_currentNode);
    if ( childNode != nullptr ){
        m_nodeStack.push_back(m_currentNode);
        m_currentNode = childNode;
        return true;
    }

    return false;
}

void XMLReader::ImplCls::BackParentElement(){
    assert( m_nodeStack.size() > 0 );

    m_currentNode = m_nodeStack.back();
    m_nodeStack.pop_back();
}

bool XMLReader::ImplCls::CheckElement(const std::string &name){
    return m_currentNode!= nullptr && 
         m_currentNode->type == XML_ELEMENT_NODE &&
        std::string((const char *)m_currentNode->name) == name ;
}

bool XMLReader::ImplCls::ReadElement(std::string &value){
    if ( m_currentNode != nullptr ){
        value = std::string((const char *)xmlNodeGetContent(m_currentNode));

        return true;
    } 
    return false;
}

bool XMLReader::ImplCls::ReadElement(uint64_t &value){
    if ( m_currentNode != nullptr ){
        std::string content((const char *)xmlNodeGetContent(m_currentNode));
        value = std::atoi(content.c_str());
        return true;
    } 
    return false;
}

bool XMLReader::ImplCls::ReadElement(double &value){
    if ( m_currentNode != nullptr ){
        std::string content((const char *)xmlNodeGetContent(m_currentNode));
        value = std::atof(content.c_str());
        return true;
    } 
    return false;
}

bool XMLReader::ImplCls::ReadAttribute(const std::string &name, std::string &value){
    if ( m_currentNode != nullptr ){
        xmlChar *v = xmlGetProp(m_currentNode, BAD_CAST name.c_str());
        if ( v != nullptr ){
            value = std::string((const char *)v);
            return true;
        }
    }
    return false;
}

bool XMLReader::ImplCls::ReadAttribute(const std::string &name, uint64_t &value){
    if ( m_currentNode != nullptr ){
        xmlChar *v = xmlGetProp(m_currentNode, BAD_CAST name.c_str());
        if ( v != nullptr ){
            value = std::atoi((const char*)v);
            return true;
        }
    }
    return false;
}

bool XMLReader::ImplCls::ReadAttribute(const std::string &name, double &value){
    if ( m_currentNode != nullptr ){
        xmlChar *v = xmlGetProp(m_currentNode, BAD_CAST name.c_str());
        if ( v != nullptr ){
            value = std::atof((const char*)v);
            return true;
        }
    }
    return false;
}


// **************** class XMLReader****************

XMLReader::XMLReader() :
    m_impl(std::unique_ptr<XMLReader::ImplCls>(new XMLReader::ImplCls())){
}

XMLReader::~XMLReader(){
}

bool XMLReader::ParseXML(const std::string &xmlString){
    return m_impl->ParseXML(xmlString);
}

bool XMLReader::HasElement() const{
    return m_impl->HasElement();
}

void XMLReader::NextElement(){
    m_impl->NextElement();
}

bool XMLReader::EnterChildElement(const std::string &name){
    return m_impl->EnterChildElement(name);
}

void XMLReader::BackParentElement(){
    m_impl->BackParentElement();
}

bool XMLReader::CheckElement(const std::string &name){
    return m_impl->CheckElement(name);
}

bool XMLReader::ReadElement(std::string &value){
    return m_impl->ReadElement(value);
}

bool XMLReader::ReadElement(uint64_t &value){
    return m_impl->ReadElement(value);
}

bool XMLReader::ReadElement(double &value){
    return m_impl->ReadElement(value);
}

bool XMLReader::ReadAttribute(const std::string &name, std::string &value){
    return m_impl->ReadAttribute(name, value);
}

bool XMLReader::ReadAttribute(const std::string &name, uint64_t &value){
    return m_impl->ReadAttribute(name, value);
}

bool XMLReader::ReadAttribute(const std::string &name, double &value){
    return m_impl->ReadAttribute(name, value);
}

#include <sstream>
#include <iomanip>
#include <libxml/xmlwriter.h>
#include "xml.h"
#include "utils/logger.h"

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

    bool ReadElement(std::string &value);
    bool ReadElement(uint64_t &value);
    bool ReadElement(double &value);
    bool ReadAttribute(const std::string &name, std::string &value);
    bool ReadAttibute(const std::string &name, uint64_t &value);
    bool ReadAttribute(const std::string &name, double &value);

    bool FromOFDXML(const std::string &strOFDXML);

    bool ParseXML(const std::string &xmlString);
    bool HasElement() const;
    void NextElement();

private:
    bool FromDocBodyXML(xmlNodePtr docBodyNode);
    bool FromDocInfoXML(xmlNodePtr docBodyNode);

    xmlNodePtr m_currentNode;

}; // class XMLReader::ImplCls

XMLReader::ImplCls::ImplCls() : m_currentNode(nullptr){
 }

XMLReader::ImplCls::~ImplCls(){
    xmlCleanupParser();
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
            std::string v((const char *)v);
            value = std::atoi(v.c_str());
            return true;
        }
    }
    return false;
}

bool XMLReader::ImplCls::ReadAttribute(const std::string &name, double &value){
    if ( m_currentNode != nullptr ){
        xmlChar *v = xmlGetProp(m_currentNode, BAD_CAST name.c_str());
        if ( v != nullptr ){
            std::string v((const char *)v);
            value = std::atof(v.c_str());
            return true;
        }
    }
    return false;
}

bool CheckElement(xmlNodePtr node, const std::string &name){
    return node != nullptr && 
        node->type == XML_ELEMENT_NODE &&
        std::string((const char *)node->name) == name ;
}

bool XMLReader::ImplCls::ParseXML(const std::string &xmlString){
    xmlDocPtr xmlDoc = xmlParseMemory(xmlString.c_str(), xmlString.length());
    if ( xmlDoc != nullptr ){
        m_currentNode = xmlDocGetRootElement(xmlDoc);
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
        while ( true ){
            node = xmlNextElementSibling(node);
            if ( node != nullptr && node->type == XML_ELEMENT_NODE ){
                break;
            }
        }
    }
}

bool XMLReader::ImplCls::FromDocInfoXML(xmlNodePtr docBodyNode){
    bool ok = true;

    xmlNodePtr childNode = xmlFirstElementChild(docBodyNode);
    while ( childNode != nullptr ){
        if ( CheckElement(childNode, "DocID") ){
            std::string strDocID((const char *)xmlNodeGetContent(childNode));
            LOG(INFO) << "DocID: " << strDocID;
        } else if ( CheckElement(childNode, "Title")){
        } else if ( CheckElement(childNode, "Author")){
        } else if ( CheckElement(childNode, "Subject")){
        } else if ( CheckElement(childNode, "Abstract")){
        } else if ( CheckElement(childNode, "CreationDate")){
        } else if ( CheckElement(childNode, "ModDate")){
        } else if ( CheckElement(childNode, "Cover")){
        } else if ( CheckElement(childNode, "Keywords")){
        } else if ( CheckElement(childNode, "Creator")){
        } else if ( CheckElement(childNode, "CreatorVersion")){
        } else if ( CheckElement(childNode, "CustomDatas")){
        }

        childNode = xmlNextElementSibling(childNode);
    }

    return ok;
}

bool XMLReader::ImplCls::FromDocBodyXML(xmlNodePtr docBodyNode){
    bool ok = true;

    // -------- <DocInfo>
    xmlNodePtr childNode = xmlFirstElementChild(docBodyNode);
    while ( childNode != nullptr ){
        if ( CheckElement(childNode, "DocInfo") ){
            ok = FromDocInfoXML(childNode);
        } else if ( CheckElement(childNode, "DocRoot")){
            std::string content((const char *)xmlNodeGetContent(childNode));
            LOG(INFO) << "DocRoot: " << content;
        } else if ( CheckElement(childNode, "Versions")){
        } else if ( CheckElement(childNode, "Signatures")){
        }
        childNode = xmlNextElementSibling(childNode);
    }

    return ok;
}

bool XMLReader::ImplCls::FromOFDXML(const std::string &strOFDXML){
    bool ok = true;

    if ( Parse(strOFDXML) ){
        ReadAttribute("Version", Version);
        ReadAttribute("DocType", DocType);
        LOG(INFO) << "Version:" << Version << " DocType:" << DocType;

        while ( HasElement() ){

            NextElement();
        };
    } else {
        LOG(ERROR) << "Parse() failed.";
    }

    /*xmlDocPtr xmlDoc = xmlParseMemory(strOFDXML.c_str(), strOFDXML.length());*/
    /*if ( xmlDoc != nullptr ){*/
        /*xmlNodePtr rootNode = xmlDocGetRootElement(xmlDoc);*/
        /*if ( CheckElement(rootNode, "OFD") ){*/
            /*xmlChar *v = xmlGetProp(rootNode, BAD_CAST "Version");*/
            /*xmlChar *d = xmlGetProp(rootNode, BAD_CAST "DocType");*/
            /*std::string version((char*)v);*/
            /*std::string docType((char *)d);*/
            /*LOG(INFO) << "Version:" << version << " DocType:" << docType;*/

            /*// -------- <DocBody>*/
            /*[>size_t num_ChildElements = xmlChildElementCount(rootNode);<]*/
            /*xmlNodePtr childNode = xmlFirstElementChild(rootNode);*/
            /*while ( childNode != nullptr ){*/
                /*if ( CheckElement(childNode, "DocBody" ) ){*/
                    /*LOG(INFO) << "Node name=" << std::string((const char *)childNode->name);*/
                    /*ok = FromDocBodyXML(childNode);*/
                /*}*/
                /*childNode = xmlNextElementSibling(childNode);*/
            /*};*/

        /*} else{*/
            /*LOG(ERROR) << "xmlDocGetRootElement() return nullptr.";*/
        /*}*/

        /*xmlFreeDoc(xmlDoc);*/
    /*} else {*/
        /*LOG(ERROR) << "xmlParseMemory() failed, return nullptr";*/
    /*}*/

    return ok;
}


// **************** class XMLReader****************

XMLReader::XMLReader() :
    m_impl(std::unique_ptr<XMLReader::ImplCls>(new XMLReader::ImplCls())){
}

XMLReader::~XMLReader(){
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

bool XMLReader::FromOFDXML(const std::string &strOFDXML){
    return m_impl->FromOFDXML(strOFDXML);
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

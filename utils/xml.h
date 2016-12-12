#ifndef __UTILS_XML_H__
#define __UTILS_XML_H__

#include <memory>
#include <string>

#define OFDXML_HEAD_ATTRIBUTES \
    writer.WriteAttribute("xmlns:ofd", "http://www.ofdspec.org/2016"); 

class XMLWriter{
public:
    XMLWriter(bool bHead=false);
    ~XMLWriter();

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

private:
    class ImplCls;
    std::unique_ptr<ImplCls> m_impl;

}; // class XMLWriter

class XMLReader{
public:
    XMLReader();
    ~XMLReader();

    bool ReadElement(std::string &value);
    bool ReadElement(uint64_t &value);
    bool ReadElement(double &value);
    bool ReadAttribute(const std::string &name, std::string &value);
    bool ReadAttribute(const std::string &name, uint64_t &value);
    bool ReadAttribute(const std::string &name, double &value);

    bool FromOFDXML(const std::string &strOFDXML);

private:
    class ImplCls;
    std::unique_ptr<ImplCls> m_impl;

}; // class XMLReader

#endif // __UTILS_XML_H__

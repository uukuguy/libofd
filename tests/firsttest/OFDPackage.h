#ifndef __OFDPACKAGE_H__
#define __OFDPACKAGE_H__

// ================ C++ API ================
#ifdef __cplusplus

#include <string>
#include <map>
#include <tuple>
#include <zip.h>
#include "ofd.h"

namespace ofd {

class OFDDocument;

class OFDPackage {
public:
    OFDPackage();
    ~OFDPackage();

    bool Open(const std::string& filename);
    void Close();

    bool ReadFile(const std::string &filename, char **buffer, size_t *bufSize) const;
    std::tuple<std::string, bool> GetFileContent(const std::string &filename) const; 
    std::string GetDocRoot() const {return m_attributes.DocRoot;};

    //OFDDocument *GetOFDDocument() {return m_document;};
    //const OFDDocument *GetOFDDocument() const {return m_document;};
    OFDDocumentPtr GetOFDDocument() {return m_document;};
    const OFDDocumentPtr GetOFDDocument() const {return m_document;};

public:
    struct Attributes {
        // <ofd:DocBody><ofd:DocInfo>
        std::string Title;
        std::string Author;
        std::string Subject;
        std::string Abstract;
        std::string DocID;
        std::string Creator;
        std::string CreatorVersion;
        std::string CreationDate;

        // <ofd:DocBody><ofd:DocRoot>
        std::string DocRoot;
    };

    const Attributes& GetAttributes() const {return m_attributes;};
    Attributes& GetAttributes() {return m_attributes;};

    std::string String() const;

private:
    std::string m_filename;
    Attributes m_attributes;

    //OFDDocument *m_document;
    OFDDocumentPtr m_document;

    bool m_opened;
    zip *m_zip;

    std::map<std::string, size_t> m_files;

    size_t getZipFileSize(zip* handle, const char *filename);

    bool init(); 

    bool checkFilesInZIP();

    bool initAttributes();
    bool parseAttributesXML(const std::string &content);

    bool initRootDocument();

}; // class OFDPackage

} // namespace ofd

#endif // #ifdef __cplusplus

// ================ C API ================
#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __OFDPACKAGE_H__

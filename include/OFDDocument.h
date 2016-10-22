#ifndef __OFDDOCUMENT_H__
#define __OFDDOCUMENT_H__

#ifdef __cplusplus

#include <zip.h>
#include <string>
#include <vector>
#include <tuple>
#include "ofd.h"

namespace ofd {

class OFDFile;
class OFDPage;


class OFDDocument {
public:
    OFDDocument(OFDFile *ofdFile, const std::string &filename);
    ~OFDDocument();

    bool Open();
    void Close();

    OFDFile *GetOFDFile(){return m_ofdFile;};
    const OFDFile *GetOFDFile() const {return m_ofdFile;};

    bool IsOpened() const {return m_opened;};

    size_t GetPagesCount() const {return m_attributes.Pages.size();};
    OFDPage *GetOFDPage(size_t idx) {return m_attributes.Pages[idx];};
    const OFDPage *GetOFDPage(size_t idx) const {return m_attributes.Pages[idx];};

public:
    class OFDCommonData{
    public:
        OFDPageArea pageArea;
        std::string publicRes;
        std::string documentRes;
        uint64_t maxUnitID;

        void clear() {
            memset(&pageArea, 0, sizeof(OFDPageArea));
        }

    }; // class OFDCommonData

    struct Attributes {
        OFDCommonData CommonData;
        std::vector<OFDPage*> Pages;

        void clear();
    };

    const Attributes& GetAttributes() const {return m_attributes;};
    Attributes& GetAttributes() {return m_attributes;};

    std::string String() const;

private:
    OFDFile *m_ofdFile;
    std::string m_filename;

    Attributes m_attributes;

    bool m_opened;
    std::string m_rootDir;

    bool parseXML(const std::string &content);

}; // class OFDDocument

} // namespace ofd

#endif // #ifdef __cplusplus

#endif // __OFDDOCUMENT_H__

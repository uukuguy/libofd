#ifndef __OFDPAGE_H__
#define __OFDPAGE_H__

#ifdef __cplusplus

#include <string>
#include <vector>
#include <tuple>
#include "ofd.h"

namespace ofd {

class OFDDocument;

class OFDPage {
public:
    OFDPage(OFDDocument *ofdDocument, uint64_t id, const std::string &filename);
    ~OFDPage();

    bool Open();
    void Close();

    const OFDDocument *GetOFDDocument() const {return m_ofdDocument;};
    OFDDocument *GetOFDDocument() {return m_ofdDocument;};

    bool IsOpened() const {return m_opened;};
    std::string GetText() const {return m_text;};


public:
    struct Attributes{
        OFDPageArea PageArea;

        void clear(){
            memset((void*)&PageArea, 0, sizeof(PageArea));
        }
    };

    const Attributes& GetAttributes() const {return m_attributes;};
    Attributes& GetAttributes() {return m_attributes;};

    std::string String() const;

private:
    OFDDocument *m_ofdDocument;
    uint64_t m_id;
    std::string m_filename;

    Attributes m_attributes;

    bool m_opened;
    std::string m_text;

    bool parseXML(const std::string &content); 


}; // class OFDPage

} // namespace ofd

#endif // #ifdef __cplusplus

#endif // __OFDPAGE_H__

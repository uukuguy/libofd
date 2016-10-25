#ifndef __OFDPAGE_H__
#define __OFDPAGE_H__

#ifdef __cplusplus

#include <string>
#include <vector>
#include <tuple>
#include "ofd.h"

namespace ofd {

class OFDDocument;
class OFDObject;
class OFDTextObject;

class OFDPage {
public:
    OFDPage(OFDDocument *ofdDocument, uint64_t id, const std::string &filename);
    ~OFDPage();

    bool Open();
    void Close();

    const OFDDocument *GetOFDDocument() const {return m_ofdDocument;};
    OFDDocument *GetOFDDocument() {return m_ofdDocument;};

    bool IsOpened() const {return m_opened;};

    size_t GetOFDObjectsCount() const {return m_ofdObjects.size();};
    const OFDObject *GetOFDObject(size_t idx) const {return m_ofdObjects[idx];};
    OFDObject *GetOFDObject(size_t idx) {return m_ofdObjects[idx];};

    uint64_t GetID() const {return m_id;};
    std::string GetText() const {return m_text;};

    bool RenderToPNGFile(const std::string& filename);

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
    std::vector<OFDObject*> m_ofdObjects;
    std::string m_text;

    void clear();
    bool parseXML(const std::string &content); 

    void drawText(const OFDTextObject *textObject) const;

}; // class OFDPage

} // namespace ofd

#endif // #ifdef __cplusplus

#endif // __OFDPAGE_H__

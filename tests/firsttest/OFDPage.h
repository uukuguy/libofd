#ifndef __OFDPAGE_H__
#define __OFDPAGE_H__

#ifdef __cplusplus

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <cairo/cairo.h>
#include "ofd.h"

namespace ofd {

class FontResource;
class OFDDocument;
class OFDObject;
class OFDTextObject;
class OFDCanvas;

class OFDPage {
public:
    OFDPage(OFDDocument *document, uint64_t id, const std::string &filename);
    //OFDPage(OFDDocumentPtr document, uint64_t id, const std::string &filename);
    ~OFDPage();

    bool Open();
    void Close();

    OFDPackage *GetPackage(); 
    const OFDPackage *GetPackage() const; 
    //OFDPackagePtr GetPackage(); 
    //const OFDPackagePtr GetPackage() const; 

    const OFDDocument *GetOFDDocument() const {return m_document;};
    OFDDocument *GetOFDDocument() {return m_document;};
    //const OFDDocumentPtr GetOFDDocument() const {return m_document.lock();};
    //OFDDocumentPtr GetOFDDocument() {return m_document.lock();};

    bool IsOpened() const {return m_opened;};

    size_t GetOFDObjectsCount() const {return m_ofdObjects.size();};
    const OFDObject *GetOFDObject(size_t idx) const {return m_ofdObjects[idx];};
    OFDObject *GetOFDObject(size_t idx) {return m_ofdObjects[idx];};

    uint64_t GetID() const {return m_id;};
    std::string GetText() const {return m_text;};

    bool Render(cairo_surface_t *surface);
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

    const std::shared_ptr<FontResource> GetFontResource() const {return m_fontResource;};

private:
    OFDDocument *m_document;
    //std::weak_ptr<OFDDocument> m_document;

    uint64_t m_id;
    std::string m_filename;

    Attributes m_attributes;

    bool m_opened;
    std::vector<OFDObject*> m_ofdObjects;
    std::string m_text;

    void clear();
    bool parseXML(const std::string &content); 

    void drawText(const OFDTextObject *textObject) const;

    //std::unique_ptr<OFDCanvas> m_canvas;
    std::shared_ptr<FontResource> m_fontResource;

}; // class OFDPage

} // namespace ofd

#endif // #ifdef __cplusplus

#endif // __OFDPAGE_H__

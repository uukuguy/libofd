#ifndef __OFDDOCUMENT_H__
#define __OFDDOCUMENT_H__

#ifdef __cplusplus

#include <zip.h>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "ofd.h"

namespace ofd {

class OFDPackage;
class OFDPage;


class OFDDocument {
public:
    OFDDocument(OFDPackage *ofdPackage, const std::string &filename);
    ~OFDDocument();

    bool Open();
    void Close();

    OFDPackage *GetPackage(){return m_ofdPackage;};
    const OFDPackage *GetPackage() const {return m_ofdPackage;};

    bool IsOpened() const {return m_opened;};

    size_t GetPagesCount() const {return m_attributes.Pages.size();};
    OFDPage *GetOFDPage(size_t idx) {return m_attributes.Pages[idx];};
    const OFDPage *GetOFDPage(size_t idx) const {return m_attributes.Pages[idx];};

    bool HasFont(int fontID) const {return m_attributes.PublicRes.HasFont(fontID);};
    const OFDFont &GetFontByID(int fontID) const {return m_attributes.PublicRes.GetFontByID(fontID);};

    std::string GetRootDir() const {return m_rootDir;};
    std::string GetPublicResBaseLoc() const {return m_attributes.PublicRes.BaseLoc;};
    std::string GetDocumentResBaseLoc() const {return m_attributes.DocumentRes.BaseLoc;};

public:
    class OFDPublicRes {
    public:
        std::string BaseLoc;
        std::string m_filename;

        size_t GetFontsCount() const {return m_fonts.size();};
        const OFDFont& GetFont(size_t idx) const {return m_fonts[idx];};
        void AppendFont(const OFDFont& font){
            m_fonts.push_back(font);
            m_mapFonts[font.ID] = font;
        };

        size_t GetColorSpacesCount() const {return m_colorSpaces.size();};
        const OFDColorSpace& GetColorSpace(size_t idx) const {return m_colorSpaces[idx];};
        void AppendColorSpace(const OFDColorSpace& colorSpace) {
            m_colorSpaces.push_back(colorSpace);
        };

        void clear(){
            m_mapFonts.clear();
            m_fonts.clear();
            m_colorSpaces.clear();
        }

        bool HasFont(int fontID) const {
            std::map<int, OFDFont>::const_iterator it = m_mapFonts.find(fontID);
            if ( it != m_mapFonts.end() ){
                return true;
            } else{
                return false;
            }
        }

        const OFDFont &GetFontByID(int fontID) const {
            std::map<int, OFDFont>::const_iterator it = m_mapFonts.find(fontID);
            return it->second;
        };

    private:
        std::vector<OFDFont> m_fonts;
        std::map<int, OFDFont> m_mapFonts;
        std::vector<OFDColorSpace> m_colorSpaces;
    };

    class OFDDocumentRes {
    public:
        std::string BaseLoc;
        std::string m_filename;

        size_t GetMultiMediasCount() const {return m_multiMedias.size();};
        const OFDMultiMedia& GetMultiMedia(size_t idx) const {return m_multiMedias[idx];};
        void AppendMultiMedia(const OFDMultiMedia& multiMedia){m_multiMedias.push_back(multiMedia);};

        void clear(){
            m_multiMedias.clear();
        }

    private:
        std::vector<OFDMultiMedia> m_multiMedias;
    };

    class OFDCommonData{
    public:
        OFDPageArea PageArea;
        std::string PublicRes;
        std::string DocumentRes;
        uint64_t MaxUnitID;

        void clear() {
            memset(&PageArea, 0, sizeof(OFDPageArea));
        }

    }; // class OFDCommonData

    struct Attributes {
        OFDPublicRes PublicRes;
        OFDDocumentRes DocumentRes;
        OFDCommonData CommonData;
        std::vector<OFDPage*> Pages;

        void clear();
    };

    const Attributes& GetAttributes() const {return m_attributes;};
    Attributes& GetAttributes() {return m_attributes;};

    std::string String() const;

private:
    OFDPackage *m_ofdPackage;
    std::string m_filename;

    Attributes m_attributes;

    bool m_opened;
    std::string m_rootDir;

    bool parseXML();
    bool parsePublicResXML();
    bool parseDocumentResXML();

}; // class OFDDocument

} // namespace ofd

#endif // #ifdef __cplusplus

#endif // __OFDDOCUMENT_H__

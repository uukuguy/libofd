#ifndef __OFD_RES_H__
#define __OFD_RES_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "OFDColor.h"
#include "OFDFont.h"

namespace ofd{

    namespace Res{
        enum class Level{
            PACKAGE = 0,
            DOCUMENT,
            PAGE,
        };
    }

    // OFD (section 7.9) P23. Res.xsd
    class OFDRes : public std::enable_shared_from_this<OFDRes> {
    private:
        OFDRes(OFDPackagePtr ofdPackage, const std::string &resDescFile = "PublicRes.xml");
        OFDRes(OFDDocumentPtr ofdDocument, const std::string &resDescFile = "DocumentRes.xml");
        OFDRes(OFDPagePtr ofdPage, const std::string &resDescFile = "PageRes.xml");
        //OFDRes(OFDPackagePtr ofdPackage, OFDDocumentPtr ofdDocument, OFDPagePtr ofdPage, const std::string &resDescFile = "");
    public:
        ~OFDRes();

        OFDResPtr GetSelf();

        static OFDResPtr CreateNewRes(OFDPackagePtr ofdPackage, const std::string &resDescFile = "PublicRes.xml");
        static OFDResPtr CreateNewRes(OFDDocumentPtr ofdDocument, const std::string &resDescFile = "DocumentRes.xml");
        static OFDResPtr CreateNewRes(OFDPagePtr ofdPage, const std::string &resDescFile = "PageRes.xml");
        //void Adjust();

        Res::Level GetResLevel() const;

        const OFDPackagePtr GetOFDPackage() const;
        const OFDDocumentPtr GetOFDDocument() const;
        const OFDPagePtr GetOFDPage() const;

        std::string GetBaseLoc() const;
        void SetBaseLoc(const std::string &baseLoc);
        std::string GetResDescFile() const;

        const ColorSpaceArray &GetColorSpaces() const;
        
        void AddColorSpace(const OFDColorSpace &ofdColorSpace);
        void AddFont(OFDFontPtr font);
        const FontMap &GetFonts() const;
        const OFDFontPtr GetFont(uint64_t fontID) const;

        std::string GenerateResXML() const;
        bool FromResXML(const std::string &strResXML);



    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDRes


}; // namespace ofd

#endif // __OFD_RES_H__

#ifndef __OFD_RES_H__
#define __OFD_RES_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "ofd/Common.h"
#include "ofd/Color.h"
#include "ofd/Font.h"

namespace ofd{

    enum class ResLevel{
        PACKAGE = 0,
        DOCUMENT,
        PAGE,
    };

    // OFD (section 7.9) P23. Res.xsd
    class OFDRes : public std::enable_shared_from_this<OFDRes> {
    private:
        OFDRes(PackagePtr package, const std::string &resDescFile);
        OFDRes(DocumentPtr document, const std::string &resDescFile);
        OFDRes(PagePtr page, const std::string &resDescFile);

    public:
        ~OFDRes();

        OFDResPtr GetSelf();

        static OFDResPtr CreateNewRes(PackagePtr package, const std::string &resDescFile = "PublicRes.xml");
        static OFDResPtr CreateNewRes(DocumentPtr document, const std::string &resDescFile = "DocumentRes.xml");
        static OFDResPtr CreateNewRes(PagePtr page, const std::string &resDescFile = "PageRes.xml");

        ResLevel GetResLevel() const;

        const PackagePtr GetPackage() const;
        const DocumentPtr GetDocument() const;
        const PagePtr GetPage() const;

        std::string GetEntryRoot() const;
        std::string GetBaseLoc() const;
        void SetBaseLoc(const std::string &baseLoc);
        std::string GetResDescFile() const;

        const ColorSpaceArray &GetColorSpaces() const;
        void AddColorSpace(const ColorSpacePtr &colorSpace);

        void AddFont(FontPtr font);
        const FontMap &GetFonts() const;
        const FontPtr GetFont(uint64_t fontID) const;

        std::string GenerateResXML() const;
        bool FromResXML(const std::string &strResXML);

        bool LoadFonts();


    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDRes


}; // namespace ofd

#endif // __OFD_RES_H__

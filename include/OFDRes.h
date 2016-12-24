#ifndef __OFD_RES_H__
#define __OFD_RES_H__

#include <memory>
#include <string>
#include "OFDCommon.h"
#include "OFDColor.h"
#include "OFDFont.h"

namespace ofd{

    // OFD (section 7.9) P23. Res.xsd
    class OFDRes{
    public:
        OFDRes(OFDPackagePtr ofdPackage, const std::string &resDescFile);
        ~OFDRes();

        std::string GetBaseLoc() const;
        void SetBaseLoc(const std::string &baseLoc);
        const ColorSpaceArray &GetColorSpaces() const;
        const FontArray &GetFonts() const;
        
        void AddColorSpace(const OFDColorSpace &ofdColorSpace);
        void AddFont(const OFDFont &ofdFont);

        std::string GenerateResXML() const;
        bool FromResXML(const std::string &strResXML);

        std::string GetResDescFile() const;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDRes

    typedef std::shared_ptr<OFDRes> OFDResPtr;

}; // namespace ofd

#endif // __OFD_RES_H__

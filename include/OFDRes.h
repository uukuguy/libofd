#ifndef __OFD_RES_H__
#define __OFD_RES_H__

#include <memory>
#include <string>
#include "OFDColor.h"
#include "OFDFont.h"

namespace ofd{

    class OFDFile;
    typedef std::shared_ptr<OFDFile> OFDFilePtr;


    // OFD (section 7.9) P23. Res.xsd
    class OFDRes{
    public:
        OFDRes(OFDFilePtr ofdFile);
        ~OFDRes();

        std::string GetBaseLoc() const;
        void SetBaseLoc(const std::string &baseLoc);
        const ColorSpaceArray &GetColorSpaces() const;
        const FontArray &GetFonts() const;
        
        void AddColorSpace(const OFDColorSpace &ofdColorSpace);
        void AddFont(const OFDFont &ofdFont);

        std::string GenerateResXML() const;
        bool FromResXML(const std::string &strResXML);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDRes

}; // namespace ofd

#endif // __OFD_RES_H__

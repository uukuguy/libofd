#ifndef __LIBOFD_H__
#define __LIBOFD_H__

#include <sstream>

namespace ofd {

    class OFDPackage;
    class OFDDocument;
    class OFDPage;

    typedef std::shared_ptr<OFDPackage> OFDPackagePtr;
    typedef std::shared_ptr<OFDDocument> OFDDocumentPtr;
    typedef std::shared_ptr<OFDPage> OFDPagePtr;

    class OFDPhysicalBox {
    public:
        double x0;
        double y0;
        double x1;
        double y1;
    }; // class OFDPhysicalBox

    class OFDPageArea {
    public:
        OFDPhysicalBox PhysicalBox;
    }; // class OFDPageArea


    struct OFDColor {
        int ColorSpace;
        double Value;
    };

    struct OFDBoundary {
        double x0, y0, w, h;
    };

    enum class OFDColorSpaceType {
        UNKNOWN = -1,
        CMYK,
        GRAY,
    };

    struct OFDColorSpace {
        int ID;
        OFDColorSpaceType Type;

        std::string ToString() const {
            std::stringstream ss;
            ss << std::endl 
                << "------------------------------" << std::endl
                << "OFDColorSpace" << std::endl;

                ss << "ID: " << ID << std::endl;
                std::string strType = "Unknown";
                if ( Type == OFDColorSpaceType::CMYK ){
                    strType = "CMYK";
                } else if ( Type == OFDColorSpaceType::GRAY ){
                    strType = "Gray";
                }
                ss << "Type: " << strType << std::endl;

                ss << std::endl
                   << "------------------------------" << std::endl;

            return ss.str();
        }
    };

    struct OFDFont {
        int ID;
        std::string FontName;
        std::string FamilyName;
        std::string Charset;
        std::string FontFile;

        std::string ToString() const {
            std::stringstream ss;
            ss << std::endl 
                << "------------------------------" << std::endl
                << "OFDFont" << std::endl;

                ss << "ID: " << ID << std::endl;
                ss << "FontName: " << FontName << std::endl;
                ss << "FamilyName: " << FamilyName << std::endl;
                ss << "Charset: " << Charset << std::endl;
                ss << "FontFile: " << FontFile << std::endl;

                ss << std::endl
                   << "------------------------------" << std::endl;

            return ss.str();
        }
    };

    enum class OFDMultiMediaType {
        UNKNOWN = -1,
        IMAGE,
    };

    struct OFDMultiMedia {
        int ID;
        OFDMultiMediaType Type;
        std::string MediaFile;

        std::string ToString() const {
            std::stringstream ss;
            ss << std::endl 
                << "------------------------------" << std::endl
                << "OFDMultiMedia" << std::endl;

                ss << "ID: " << ID << std::endl;
                std::string strType = "Unknown";
                if ( Type == OFDMultiMediaType::IMAGE ){
                    strType = "Image";
                }
                ss << "Type: " << strType << std::endl;
                ss << "MediaFile: " << MediaFile << std::endl;

                ss << std::endl
                   << "------------------------------" << std::endl;

            return ss.str();
        }
    };

    // CTM (Context Translate Matrix)
    //
    // a  b  0
    // c  d  0
    // p  q  1
    //
    struct OFDCTM {
        double a, b, c, d, p, q;
    };

}; // namespace ofd

//#include "OFDPage.h"
//#include "OFDDocument.h"
//#include "OFDFile.h"
//#include "utils.h"

#endif // __LIBOFD_H__

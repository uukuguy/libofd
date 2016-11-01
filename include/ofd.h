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

    const double mm_per_inch = 25.4;
    const double default_dpi = 300.0;

    #define length_to_pixel(length, dpi) dpi * length / mm_per_inch 

    class OFDPhysicalBox {
    public:
        double x0;
        double y0;
        double x1;
        double y1;

        double Width() const {
            return x1 - x0;
        }

        double Height() const {
            return y1 - y0;
        }

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
    // xx  xy  0
    // yx  yy  0
    // x0  y0  1
    //
    struct OFDCTM {
        double xx; double xy;
        double yx; double yy;
        double x0; double y0;
    };

#define OFDPACKAGE_GET_FILE_CONTENT(package, filename, content) \
    std::string content; \
    { \
        bool ok = false; \
        std::tie(content, ok) = package->GetFileContent(filename); \
        if ( !ok ) { \
            LOG(WARNING) << "package->GetFileContent() failed. filename: " << filename; \
            return false; \
        } \
    }

#define TEXT_TO_XML(content, xmldoc) \
    XMLDocument *xmldoc = new XMLDocument(); \
    { \
        XMLError rc = xmldoc->Parse(content.c_str()); \
        if ( rc != XML_SUCCESS ){ \
            LOG(WARNING) << content; \
            LOG(WARNING) << "xmldoc->Parse() failed."; \
            delete xmldoc; \
            return false; \
        } \
    } \

}; // namespace ofd

#endif // __LIBOFD_H__

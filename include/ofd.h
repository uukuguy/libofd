#ifndef __LIBOFD_H__
#define __LIBOFD_H__

namespace ofd {

    class OFDPhysicalBox {
    public:
        double x0;
        double y0;
        double x1;
        double y1;
    }; // class OFDPhysicalBox

    class OFDPageArea {
    public:
        OFDPhysicalBox physicalBox;
    }; // class OFDPageArea


    struct OFDColor {
        int ColorSpace;
        double Value;
    };

    struct OFDBoundary {
        double x0, y0, w, h;
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

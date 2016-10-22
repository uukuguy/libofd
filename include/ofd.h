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

}; // namespace ofd

//#include "OFDPage.h"
//#include "OFDDocument.h"
//#include "OFDFile.h"
//#include "utils.h"

#endif // __LIBOFD_H__

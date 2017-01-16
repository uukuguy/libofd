#ifndef __OFD_FONTOUTPUTDEV_H__
#define __OFD_FONTOUTPUTDEV_H__

#include <memory>
#include <OutputDev.h>
#include <PDFDoc.h>
#include <GfxState.h>
//#include <Stream.h>
//#include <goo/gtypes.h>
//#include <Object.h>
//#include <GfxFont.h>
#include "Preprocessor.h"

typedef std::shared_ptr<PDFDoc> PDFDocPtr;

namespace ofd{

    class FontOutputDev : public OutputDev {
    public:
        FontOutputDev();
        virtual ~FontOutputDev();

        void ProcessDoc(PDFDocPtr pdfDoc);

        // Does this device use upside-down coordinates?
        // (Upside-down means (0,0) is the top left corner of the page.)
        virtual GBool upsideDown() { return gFalse; }

        // Does this device use drawChar() or drawString()?
        virtual GBool useDrawChar() { return gFalse; }

        // Does this device use beginType3Char/endType3Char?  Otherwise,
        // text in Type 3 fonts will be drawn with drawChar/drawString.
        virtual GBool interpretType3Chars() { return gFalse; }

        // Rendering
        virtual void drawString(GfxState * state, GooString * s);

        //// Start a page.
        //virtual void startPage(int pageNum, GfxState *state, XRef * xref);
        //// End a page.
        //virtual void endPage();


    protected:
        void preProcess(PDFDocPtr pdfDoc);
        void postProcess();

    private:
        Preprocessor m_preprocessor;

    }; // FontOutputDev

}; // namespace ofd

#endif // __OFD_FONTOUTPUTDEV_H__

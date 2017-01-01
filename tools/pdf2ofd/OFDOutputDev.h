#ifndef __OFDOUTPUTDEV_H__
#define __OFDOUTPUTDEV_H__

#include <memory>
#include <OutputDev.h>
#include "TextOutputDev.h"
#include <PDFDoc.h>
#include "OFDCommon.h"
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "OFDFont.h"

class OFDOutputDev;
typedef std::shared_ptr<OFDOutputDev> OFDOutputDevPtr;
typedef std::shared_ptr<PDFDoc> PDFDocPtr;

class OFDOutputDev : public OutputDev {
public:
    OFDOutputDev(ofd::OFDPackagePtr ofdPackage);
    virtual ~OFDOutputDev();

    void ProcessDoc(PDFDocPtr pdfDoc);
    void PrintFonts() const;

    void SetTextPage(TextPage *textPage);
    TextPage *GetTextPage() const {return m_textPage;};
    TextPage *TakeTextPage();

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    virtual GBool upsideDown() { return gTrue; }

    // Does this device use drawChar() or drawString()?
    virtual GBool useDrawChar() { return gTrue; }

    // Does this device use tilingPatternFill()?  If this returns false,
    // tiling pattern fills will be reduced to a series of other drawing
    // operations.
    virtual GBool useTilingPatternFill() { return gTrue; }

    // Does this device use functionShadedFill(), axialShadedFill(), and
    // radialShadedFill()?  If this returns false, these shaded fills
    // will be reduced to a series of other drawing operations.
//#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    //virtual GBool useShadedFills(int type) { return type <= 7; }
//#else
    //virtual GBool useShadedFills(int type) { return type > 1 && type < 4; }
//#endif

    // Does this device use FillColorStop()?
    virtual GBool useFillColorStop() { return gTrue; }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    virtual GBool interpretType3Chars() { return gFalse; }

    // Does this device need to clip pages to the crop box even when the
    // box is the crop box?
    virtual GBool needClipToCropBox() { return gTrue; }

    // Does this device need non-text content?
    virtual GBool needNonText() { return gTrue; }


    //----- initialization and control

    // Start a page.
    virtual void startPage(int pageNum, GfxState *state, XRef *xref);

    // End a page.
    virtual void endPage();

    //----- save/restore graphics state
    virtual void saveState(GfxState *state);
    virtual void restoreState(GfxState *state);

    //----- update text state
    virtual void updateFont(GfxState *state);

    //----- text drawing
    virtual void beginString(GfxState *state, GooString *s);
    virtual void endString(GfxState *state);
    virtual void drawChar(GfxState *state, double x, double y,
            double dx, double dy,
            double originX, double originY,
            CharCode code, int nBytes, Unicode *u, int uLen);
    virtual void incCharCount(int nChars);
    //virtual void beginActualText(GfxState *state, GooString *text);
    //virtual void endActualText(GfxState *state);

private:

    PDFDocPtr m_pdfDoc;
    XRef *m_xref;
    TextPage *m_textPage;	    
    ActualText *m_actualText;

    ofd::OFDPackagePtr m_ofdPackage;
    ofd::OFDDocumentPtr m_ofdDocument;
    ofd::OFDPagePtr m_currentOFDPage;

    void processTextLine(TextLine *line, ofd::OFDLayerPtr bodyLayer);
    void processTextPage(TextPage *textPage, ofd::OFDPagePtr currentOFDPage);

public:
    ofd::OFDFontPtr m_currentFont;
    double m_currentFontSize;
    double *m_currentCTM;


}; // class OFDOutputDev

#endif // __OFDOUTPUTDEV_H__

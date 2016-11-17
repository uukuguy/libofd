#ifndef __PDFEXTRACTOR_H__
#define __PDFEXTRACTOR_H__

#include <string>
#include <memory>

// poppler headers
#include <OutputDev.h>
#include <GfxState.h>
#include <Stream.h>
#include <PDFDoc.h>
#include <PDFDocFactory.h>
#include <goo/gtypes.h>
#include <Object.h>
#include <GfxFont.h>
#include <Annot.h>


class PDFExtractor : public OutputDev {
public:
    PDFExtractor();
    virtual ~PDFExtractor();

    bool Process(const std::string &pdfFilename, const std::string &ownerPasswd, const std::string &userPasswd);

    virtual bool OpenPDFFile(const std::string &pdfFileName, const std::string &ownerPasswd, const std::string &userPasswd);
    virtual void ClosePDFFile();

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    virtual GBool upsideDown() { return gFalse; }

    // Does this device use drawChar() or drawString()?
    virtual GBool useDrawChar() { return gTrue; }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    virtual GBool interpretType3Chars() { return gFalse; }

    //virtual GBool needNonText() { return gFalse; }
    //virtual GBool needClipToCropBox() { return gTrue; }

    virtual void drawChar(GfxState *state, double x, double y,
            double dx, double dy,
            double originX, double originY,
            CharCode code, int nBytes, Unicode *u, int uLen);

    class InnerData;
private:
    std::unique_ptr<InnerData> m_innerData;

}; // class PDFExtractor

#endif // __PDFEXTRACTOR_H__

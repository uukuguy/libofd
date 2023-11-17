/*
 * Preprocessor.h
 *
 * PDF is so complicated that we have to scan twice
 *
 * Check used codes for each font
 * Collect all used link destinations
 *
 * by WangLu
 * 2012.09.07
 */


#ifndef PREPROCESSOR_H__
#define PREPROCESSOR_H__

#include <memory>
#include <unordered_map>

#include <OutputDev.h>
#include <PDFDoc.h>
#include <Annot.h>
#include "Param.h"


typedef std::shared_ptr<PDFDoc> PDFDocPtr;

class Preprocessor : public OutputDev {
public:
    Preprocessor();
    virtual ~Preprocessor(void);

    void ProcessDoc(PDFDocPtr  doc);

    virtual GBool upsideDown() { return gFalse; }
    virtual GBool useDrawChar() { return gTrue; }
    virtual GBool interpretType3Chars() { return gFalse; }
    virtual GBool needNonText() { return gFalse; }
    virtual GBool needClipToCropBox() { return gTrue; }

    virtual void drawChar(GfxState *state, double x, double y,
      double dx, double dy,
      double originX, double originY,
      CharCode code, int nBytes, Unicode *u, int uLen);

    // Start a page.
    // UGLY: These 2 versions are for different versions of poppler
    virtual void startPage(int pageNum, GfxState *state);
    virtual void startPage(int pageNum, GfxState *state, XRef * xref);

    const char * get_code_map (long long font_id) const;
    double get_max_width (void) const { return max_width; }
    double get_max_height (void) const { return max_height; }

protected:
    bool m_useCropBox;
    double max_width, max_height;

    long long cur_font_id;
    char * cur_code_map;

    std::unordered_map<long long, char*> code_maps;
};

#endif //PREPROCESSOR_H__

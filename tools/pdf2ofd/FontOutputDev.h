#ifndef __OFD_FONTOUTPUTDEV_H__
#define __OFD_FONTOUTPUTDEV_H__

#include <memory>
#include <map>
#include <OutputDev.h>
#include <PDFDoc.h>
#include <GfxState.h>
#include <Stream.h>
#include <goo/gtypes.h>
#include <Object.h>
#include <GfxFont.h>
#include "Preprocessor.h"
#include "utils/StringFormatter.h"

typedef std::shared_ptr<PDFDoc> PDFDocPtr;

namespace ofd{

    typedef struct FontInfo {
        long long id;
        bool use_tounicode;
        int em_size;
        double space_width;
        double ascent, descent;
        bool is_type3;
        /*
         * As Type 3 fonts have a font matrix
         * a glyph of 1pt can be very large or very small
         * however it might not be true for other font formats such as ttf
         *
         * Therefore when we save a Type 3 font into ttf,
         * we have to scale the font to about 1,
         * then apply the scaling when using the font
         *
         * The scaling factor is stored as font_size_scale
         *
         * The value is 1 for other fonts
         */
        double font_size_scale;
    } *FontInfo_t;

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

        virtual void updateAll(GfxState * state);
        virtual void updateFont(GfxState * state); 


    protected:
        void preProcess(PDFDocPtr pdfDoc);
        void postProcess();
        void checkStateChange(GfxState * state);

        void resetState();
        void resetChangedState();

        const FontInfo *installFont(GfxFont * font);
        std::string dumpEmbeddedFont(GfxFont *font, FontInfo & info);
        std::string dumpType3Font(GfxFont *font, FontInfo & info); 
        void embedFont(const std::string & filepath, GfxFont * font, FontInfo & info, bool get_metric_only = false);
        void installEmbeddedFont(GfxFont *font, FontInfo & info);
        void installExternalFont(GfxFont *font, FontInfo & info);

        PDFDocPtr m_pdfDoc;
        XRef *m_xref;

        bool m_allChanged;
        bool m_fontChanged;

    private:
        Preprocessor m_preprocessor;
        utils::StringFormatter str_fmt;

        std::vector<int32_t> cur_mapping; 
        std::vector<char*> cur_mapping2;
        std::vector<int> width_list; // width of each char

        std::unordered_map<long long, FontInfo> font_info_map;


    private:
        typedef struct Param{
            bool debug = true;
            int toUnicode = 1;
            double hDPI = 72.0;
            double vDPI = 72.0;
            std::string tmpDir = "/tmp";
            std::string destDir = ".";
            int stretchNarrowGlyph = 0;
            int squeezeWideGlyph = 1;
            bool autoHint = false;
            std::string externalHintTool = "";
            bool embedFont = true;
            // TODO
            bool embedExternalFont = true;
            std::string fontFormat = "woff";
            bool overrideFstype = false;

        } *Param_t;

        Param m_param;

    }; // FontOutputDev

}; // namespace ofd

#endif // __OFD_FONTOUTPUTDEV_H__

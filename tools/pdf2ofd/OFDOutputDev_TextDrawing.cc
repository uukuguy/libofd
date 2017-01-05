#include <GlobalParams.h>
#include <UnicodeMap.h>
#include "OFDOutputDev.h"
#include "utils/logger.h"

//#include <UTF.h>
//#include <UTF8.h>
void OFDOutputDev::beginString(GfxState *state, GooString *s){

    //Unicode *uni = NULL;
    //__attribute__((unused)) int length = TextStringToUCS4(s, &uni);

    //mapUTF8(u, buf, bufSize);

    //gfree(uni);

    //LOG(INFO) << "beginString() : " << std::string((const char *)uni);

    int len = s->getLength();

    if ( m_needFontUpdate ){
        updateFont(state);
    }

    if (!m_currentFont)
        return;

    m_cairoGlyphs = (cairo_glyph_t *) gmallocn (len, sizeof (cairo_glyph_t));
    m_glyphsCount = 0;
    if (m_useShowTextGlyphs) {
        m_cairoTextClusters = (cairo_text_cluster_t *) gmallocn (len, sizeof (cairo_text_cluster_t));
        m_clustersCount = 0;
        m_utf8Max = len*2; // start with twice the number of glyphs. we will realloc if we need more.
        m_utf8 = (char *) gmalloc (m_utf8Max);
        m_utf8Count = 0;
    }
}

void OFDOutputDev::endString(GfxState *state){

    int render;

    if (!m_currentFont)
        return;

    // endString can be called without a corresponding beginString. If this
    // happens glyphs will be null so don't draw anything, just return.
    // XXX: OutputDevs should probably not have to deal with this...
    if (!m_cairoGlyphs)
        return;

    // ignore empty strings and invisible text -- this is used by
    // Acrobat Capture
    render = state->getRender();
    if (render == 3 || m_glyphsCount == 0 || !m_textMatrixValid) {
        goto finish;
    }

    if (!(render & 1)) {
        //LOG (printf ("fill string\n"));
        cairo_set_source (m_cairo, m_fillPattern);
        if (m_useShowTextGlyphs)
            cairo_show_text_glyphs (m_cairo, m_utf8, m_utf8Count, m_cairoGlyphs, m_glyphsCount, m_cairoTextClusters, m_clustersCount, (cairo_text_cluster_flags_t)0);
        else
            cairo_show_glyphs (m_cairo, m_cairoGlyphs, m_glyphsCount);
        if (m_cairoShape)
            cairo_show_glyphs (m_cairoShape, m_cairoGlyphs, m_glyphsCount);
    }

    // stroke
    if ((render & 3) == 1 || (render & 3) == 2) {
        LOG(DEBUG) << "stroke string";
        cairo_set_source (m_cairo, m_strokePattern);
        cairo_glyph_path (m_cairo, m_cairoGlyphs, m_glyphsCount);
        cairo_stroke (m_cairo);
        if (m_cairoShape) {
            cairo_glyph_path (m_cairoShape, m_cairoGlyphs, m_glyphsCount);
            cairo_stroke (m_cairoShape);
        }
    }

    // clip
    if ((render & 4)) {
        LOG(DEBUG) << "clip string";
        // append the glyph path to m_textClipPath.

        // set textClipPath as the currentPath
        if (m_textClipPath != nullptr ) {
            cairo_append_path (m_cairo, m_textClipPath);
            if (m_cairoShape) {
                cairo_append_path (m_cairoShape, m_textClipPath);
            }
            cairo_path_destroy (m_textClipPath);
        }

        // append the glyph path
        cairo_glyph_path (m_cairo, m_cairoGlyphs, m_glyphsCount);

        // move the path back into textClipPath 
        // and clear the current path
        m_textClipPath = cairo_copy_path (m_cairo);
        cairo_new_path (m_cairo);
        if (m_cairoShape) {
            cairo_new_path (m_cairoShape);
        }
    }

finish:
    gfree (m_cairoGlyphs);
    m_cairoGlyphs = nullptr;
    if (m_useShowTextGlyphs) {
        gfree (m_cairoTextClusters);
        m_cairoTextClusters = nullptr;
        gfree (m_utf8);
        m_utf8 =  nullptr;
    }
}

void OFDOutputDev::beginTextObject(GfxState *state) {
}

void OFDOutputDev::endTextObject(GfxState *state) {
    if (m_textClipPath) {
        // clip the accumulated text path
        cairo_append_path (m_cairo, m_textClipPath);
        cairo_clip (m_cairo);
        if (m_cairoShape) {
            cairo_append_path (m_cairoShape, m_textClipPath);
            cairo_clip (m_cairoShape);
        }
        cairo_path_destroy (m_textClipPath);
        m_textClipPath = nullptr;
    }
}

void OFDOutputDev::drawChar(GfxState *state, double x, double y,
        double dx, double dy,
        double originX, double originY,
        CharCode code, int nBytes, Unicode *u, int uLen){

    //LOG(DEBUG) << "(x,y,dx,dy):" << std::setprecision(3) << "(" << x << ", " << y << ", " << dx << ", " << dy << ")" 
        //<< " (originX, originY):" << "(" << originX << ", " << originY << ")"
        //<< " code:" << std::hex << std::setw(4) << std::setfill('0') << code
        //<< " nBytes:" << nBytes << " uLen:" << uLen;

    //LOG(DEBUG) << "code:" << std::hex << std::setw(4) << std::setfill('0') << code << " nBytes:" << nBytes << " uLen:" << uLen;
    if ( m_textPage != nullptr ){
        //LOG(INFO) << "code:" << std::hex << std::setw(4) << std::setfill('0') << code << " nBytes:" << nBytes << " uLen:" << uLen;
        m_actualText->addChar(state, x, y, dx, dy, code, nBytes, u, uLen);
    }


    if ( m_currentFont != nullptr ) {
        m_cairoGlyphs[m_glyphsCount].index = m_currentFont->GetGlyph (code, u, uLen);
        m_cairoGlyphs[m_glyphsCount].x = x - originX;
        m_cairoGlyphs[m_glyphsCount].y = y - originY;
        m_glyphsCount++;
        if (m_useShowTextGlyphs) {
            GooString enc("UTF-8");
            UnicodeMap *utf8Map = globalParams->getUnicodeMap(&enc);
            if (m_utf8Max - m_utf8Count < uLen*6) {
                // utf8 encoded characters can be up to 6 bytes
                if (m_utf8Max > uLen*6)
                    m_utf8Max *= 2;
                else
                    m_utf8Max += 2*uLen*6;
                m_utf8 = (char *) grealloc (m_utf8, m_utf8Max);
            }
            m_cairoTextClusters[m_clustersCount].num_bytes = 0;
            for (int i = 0; i < uLen; i++) {
                int size = utf8Map->mapUnicode(u[i], m_utf8 + m_utf8Count, m_utf8Max - m_utf8Count);
                m_utf8Count += size;
                m_cairoTextClusters[m_clustersCount].num_bytes += size;
            }
            m_cairoTextClusters[m_clustersCount].num_glyphs = 1;
            m_clustersCount++;
        }
    }

}

void OFDOutputDev::incCharCount(int nChars){
    if ( m_textPage != nullptr ){
        m_textPage->incCharCount(nChars);
    }
}


//#include <UTF.h>
//void OFDOutputDev::beginActualText(GfxState *state, GooString *text) {
    //LOG(INFO) << "********************* beginActualText() ******************";
    //if ( m_textPage != nullptr ){
        //m_actualText->begin(state, text);

        //GooString *aText = new GooString(text);
        //Unicode *uni = NULL;
        //__attribute__((unused)) int length = TextStringToUCS4(aText, &uni);
        //gfree(uni);
        //delete aText;
        //LOG(INFO) << "beginActualText: " << std::string((const char*)text);
    //}
//}

//void OFDOutputDev::endActualText(GfxState *state) {
    //if ( m_textPage != nullptr ){
        //m_actualText->end(state);
    //}
//}

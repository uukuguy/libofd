#include <fofi/FoFiTrueType.h>
#include <fstream>
#include "OFDOutputDev.h"
#include "OFDFont.h"
#include "utils/logger.h"

/***
 *
 * OFDOutputDev::updateFont()
 *
 * showGfxFont()
 * getCodeToGID()
 * GfxFont_to_OFDFont()
 * GetSubstitutionCorrection()
 *
 * **/

using namespace ofd;

// -------- showGfxFont --------
void showGfxFont(GfxFont *gfxFont){

    Ref *ref = gfxFont->getID();
    GooString *family = gfxFont->getFamily();
    std::string fontFamily;
    if ( family != nullptr ){
        fontFamily = std::string(family->getCString());
    }

    GooString *name = gfxFont->getName();
    std::string fontName;
    if ( name != nullptr ){
        fontName = std::string(name->getCString());
    }

    GooString *encodingName = gfxFont->getEncodingName();
    std::string fontEncodingName;
    if ( encodingName != nullptr ){
        fontEncodingName = std::string(encodingName->getCString());
    }

    GooString *embFontName = gfxFont->getEmbeddedFontName();
    std::string fontEmbeddedName;
    if ( embFontName != nullptr ){
        fontEmbeddedName = std::string(embFontName->getCString());
    }

    Ref embID;
    embID.num = -1;
    embID.gen = -1;
    gfxFont->getEmbeddedFontID(&embID);


    GfxFontType fontType = gfxFont->getType();
    GfxFont::Stretch fontStretch = gfxFont->getStretch();
    GfxFont::Weight fontWeight = gfxFont->getWeight();

    bool isItalic = gfxFont->isItalic();
    bool isBold = gfxFont->isBold();

    double *fontMatrix = gfxFont->getFontMatrix();
    double *fontBBox = gfxFont->getFontBBox();

    std::string fontTypeName = "()";
    if ( fontType == fontCIDType2 ){
        fontTypeName = "(fontCIDType2)";
    } else if ( fontType == fontTrueType ){
        fontTypeName = "(fontTrueType)";
    }

    LOG(INFO) << "UpdateFont() \n"
        << "fontID(num,gen):(" << ref->num << ", " << ref->gen << ") \n"
        << "FontType: " << int(fontType) << fontTypeName << " \n"
        << "FontFamily:" << fontFamily << " \n"
        << "FontName:" << fontName << " \n"
        << "FontEncodingName:" << fontEncodingName << " \n"
        << "fontEmbeddedName:" << fontEmbeddedName << " \n"
        << "embID(num,gen): (" << embID.num << ", " << embID.gen << ") \n"
        << "FontStretch: " << fontStretch << " \n"
        << "FontWeight: " << fontWeight << " \n"
        << "isItalic: " << isItalic << " \n"
        << "isBold: " << isBold << " \n"
        << "fontMatrix: [" << fontMatrix[0] << ", " << fontMatrix[1] << ", " << fontMatrix[2] << ", "
        << fontMatrix[3] << ", " <<  fontMatrix[4] << ", " << fontMatrix[5] << "] \n"
        << "fontBBox: [" << fontBBox[0] << ", " << fontBBox[1] << ", " << fontBBox[2] << ", " << fontBBox[3] << "] \n"
        ;

}

// -------- getCodeToGID --------
std::tuple<int*, size_t> getCodeToGID(GfxFont *gfxFont, char *fontData, size_t fontDataLen) {
    assert(gfxFont != nullptr);
    assert(fontData != nullptr && fontDataLen > 0);

    int *codeToGID = nullptr;
    size_t codeToGIDLen = 0;
    FoFiTrueType *ff;
    GfxFontType fontType = gfxFont->getType();

    switch ( fontType ){
        case fontCIDType2:
        case fontCIDType2OT: {
                codeToGID = NULL;
                int n = 0;
                if (((GfxCIDFont *)gfxFont)->getCIDToGID()) {
                    n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
                    if (n) {
                        codeToGID = (int *)gmallocn(n, sizeof(int));
                        memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(), n * sizeof(int));
                    }
                } else {
                    ff = FoFiTrueType::make(fontData, fontDataLen);
                    if ( ff != nullptr ){
                        codeToGID = ((GfxCIDFont *)gfxFont)->getCodeToGIDMap(ff, &n);
                        delete ff;
                    } else {
                        LOG(ERROR) << "FofiTrueType make or load failed.";
                    }
                }
                codeToGIDLen = n;
            } break;
        default:
            break;
    }; 

    return std::make_tuple(codeToGID, codeToGIDLen);
}

// -------- GfxFont_to_OFDFont() --------
OFDFontPtr GfxFont_to_OFDFont(GfxFont *gfxFont, XRef *xref){
    OFDFontPtr ofdFont = std::make_shared<OFDFont>();

    // -------- FontID --------
    Ref *ref = gfxFont->getID();
    ofdFont->ID = ref->num;

    // -------- FontFamily --------
    GooString *family = gfxFont->getFamily();
    if ( family != nullptr ){
        ofdFont->FamilyName = std::string(family->getCString());
    }

    // -------- FontName --------
    GooString *name = gfxFont->getName();
    if ( name != nullptr ){
        ofdFont->FontName = std::string(name->getCString());
    }

    // -------- FontType --------
    GfxFontType fontType = gfxFont->getType();
    if ( fontType == fontCIDType2 ){
        ofdFont->FontType = ofd::Font::Type::CIDType2;
    } else if (fontType == fontType1 ){
        ofdFont->FontType = ofd::Font::Type::Type1;
    } else if (fontType == fontType3 ){
        ofdFont->FontType = ofd::Font::Type::Type3;
    } else if (fontType == fontTrueType ){
        ofdFont->FontType = ofd::Font::Type::TrueType;
    } else {
        ofdFont->FontType = ofd::Font::Type::Unknown;
    }

    // -------- FontLoc --------
    GfxFontLoc *fontLoc = gfxFont->locateFont(xref, nullptr);
    if ( fontLoc != nullptr ){
        if ( fontLoc->locType == gfxFontLocEmbedded ){
            ofdFont->FontLoc = ofd::Font::Location::Embedded;
        } else if ( fontLoc->locType == gfxFontLocExternal ){
            ofdFont->FontLoc = ofd::Font::Location::External;
            ofdFont->FontFile = std::string(fontLoc->path->getCString());
        } else if ( fontLoc->locType == gfxFontLocResident ){
            ofdFont->FontLoc = ofd::Font::Location::Resident;
        } else {
            ofdFont->FontLoc = ofd::Font::Location::Unknown;
        }
        delete fontLoc;
        fontLoc = nullptr;
    } else {
        LOG(WARNING) << "fontLoc == nullptr.";
    }

    // -------- FontData --------
    int fontDataSize = 0;
    char *fontData = gfxFont->readEmbFontFile(xref, &fontDataSize);

    // FIXME
    //ofdFont->m_fontData = fontData;
    //ofdFont->m_fontDataSize = fontDataSize;
    ofdFont->CreateFromData(fontData, fontDataSize);

    //// FIXME
    //// Export running font data.
    //{
        //std::string fontfile = std::string("font_running_") + std::to_string(ofdFont->ID) + ".ttf";
        //utils::WriteFileData(fontfile, fontData, fontDataSize);
    //}

    int *codeToGID = nullptr;
    size_t codeToGIDLen = 0;
    std::tie(codeToGID, codeToGIDLen) = getCodeToGID(gfxFont, fontData, fontDataSize);

    return ofdFont;
}

// -------- getSubstitutionCorrection() --------
double getSubstitutionCorrection(OFDFontPtr ofdFont, GfxFont *gfxFont){
    double w1, w2, w3;
    CharCode code;
    char *name;

    // for substituted fonts: adjust the font matrix -- compare the
    // width of 'm' in the original font and the substituted font
    if ( ofdFont->IsSubstitute() && !gfxFont->isCIDFont()) {
        for (code = 0; code < 256; ++code) {
            if ((name = ((Gfx8BitFont *)gfxFont)->getCharName(code)) &&
                    name[0] == 'm' && name[1] == '\0') {
                break;
            }
        }
        if (code < 256) {
            w1 = ((Gfx8BitFont *)gfxFont)->getWidth(code);
            {
                cairo_matrix_t m;
                cairo_matrix_init_identity(&m);
                cairo_font_options_t *options = cairo_font_options_create();
                cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_NONE);
                cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);
                cairo_scaled_font_t *scaled_font = cairo_scaled_font_create(ofdFont->GetFontFace(), &m, &m, options);

                cairo_text_extents_t extents;
                cairo_scaled_font_text_extents(scaled_font, "m", &extents);

                cairo_scaled_font_destroy(scaled_font);
                cairo_font_options_destroy(options);
                w2 = extents.x_advance;
            }
            w3 = ((Gfx8BitFont *)gfxFont)->getWidth(0);
            if (!gfxFont->isSymbolic() && w2 > 0 && w1 > w3) {
                // if real font is substantially narrower than substituted
                // font, reduce the font size accordingly
                if (w1 > 0.01 && w1 < 0.9 * w2) {
                    w1 /= w2;
                    return w1;
                }
            }
        }
    }
    return 1.0;
}

// -------- OFDOutputDev::updateFont --------
void OFDOutputDev::updateFont(GfxState *state){
    GfxFont *gfxFont = state->getFont();
    m_needFontUpdate = false;
    if ( m_textPage != nullptr ){
        m_textPage->updateFont(state);
    }

    if ( gfxFont != nullptr ){
        m_useShowTextGlyphs = gfxFont->hasToUnicodeCMap() && cairo_surface_has_show_text_glyphs (cairo_get_target(m_cairo));

        Ref *ref = gfxFont->getID();
        int fontID = ref->num;

        // FIXME
        // LoadFont!!!
        OFDFontPtr ofdFont = nullptr;
        OFDDocument::CommonData &commonData = m_ofdDocument->GetCommonData();
        assert(commonData.DocumentRes != nullptr );
        ofdFont = commonData.DocumentRes->GetFont(fontID);

        if ( ofdFont == nullptr ){
            dump_embedded_font(gfxFont, m_xref);
            ofdFont = GfxFont_to_OFDFont(gfxFont, m_xref);
            commonData.DocumentRes->AddFont(ofdFont);
            showGfxFont(gfxFont);
        }

        m_currentFont = ofdFont;


        cairo_font_face_t *font_face;
        cairo_matrix_t matrix, invert_matrix;

        // FIXME
        font_face = m_currentFont->GetFontFace();
        cairo_set_font_face(m_cairo, font_face);

        m_useShowTextGlyphs = state->getFont()->hasToUnicodeCMap() &&
            cairo_surface_has_show_text_glyphs(cairo_get_target(m_cairo));

        double fontSize = state->getFontSize();
        double *m = state->getTextMat();
        m_currentFontSize = fontSize;
        //m_currentCTM = m;

        LOG(INFO) << "UpdateFont() ID: " << fontID << " FontName: " << ofdFont->FontName << " fontSize: " << fontSize << " sizeof(m): " << sizeof(m);
        LOG(INFO) << "TextMat: [" << m[0] << ", " << m[1] << ", " << m[2] << ", "
            << m[3] << ", " << m[4] << ", " << m[5] << "]";

        /* NOTE: adjusting by a constant is hack. The correct solution
         * is probably to use user-fonts and compute the scale on a per
         * glyph basis instead of for the entire font */

        //double w = m_currentFont->GetSubstitutionCorrection(state->getFont());
        double w = getSubstitutionCorrection(m_currentFont, state->getFont());
        matrix.xx = m[0] * fontSize * state->getHorizScaling() * w;
        matrix.yx = m[1] * fontSize * state->getHorizScaling() * w;
        matrix.xy = -m[2] * fontSize;
        matrix.yy = -m[3] * fontSize;

        matrix.x0 = 0;
        matrix.y0 = 0;

        LOG(DEBUG) << "font matrix: " << matrix.xx << ", " << matrix.yx << ", " << matrix.xy << ", " << matrix.yy;

        /* Make sure the font matrix is invertible before setting it.  cairo
         * will blow up if we give it a matrix that's not invertible, so we
         * need to check before passing it to cairo_set_font_matrix. Ignoring it
         * is likely to give better results than not rendering anything at
         * all. See #18254.
         */
        invert_matrix = matrix;
        if (cairo_matrix_invert(&invert_matrix)) {
            LOG(ERROR) << "font matrix not invertible";
            m_textMatrixValid = false;
            return;
        }

        cairo_set_font_matrix(m_cairo, &matrix);
        m_textMatrixValid = true;
    }
}



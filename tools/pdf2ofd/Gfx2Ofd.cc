#include "Gfx2Ofd.h"
#include "ofd/Font.h"
#include "ofd/Path.h"
#include "ofd/Color.h"
#include "utils/logger.h"

using namespace ofd;

PathPtr GfxPath_to_OfdPath(GfxPath *gfxPath){
    if ( gfxPath == nullptr ) return nullptr;

    PathPtr ofdPath = nullptr;
    int numSubpaths = gfxPath->getNumSubpaths();
    if ( numSubpaths == 0 ) return nullptr;
    ofdPath = std::make_shared<Path>();

    double x, y;
    int j;
    for ( int i = 0 ; i < numSubpaths ; i++){
        GfxSubpath *subpath = gfxPath->getSubpath(i);
        if ( subpath->getNumPoints() > 0 ){
            x = subpath->getX(0);
            y = subpath->getY(0);
            ofdPath->MoveTo(Point_t(x,y));
            j = 1;
            while ( j < subpath->getNumPoints()) {
                if ( subpath->getCurve(j) ){
                    x = subpath->getX(j+2);
                    y = subpath->getY(j+2);
                    double x0 = subpath->getX(j);
                    double y0 = subpath->getY(j);
                    double x1 = subpath->getX(j+1);
                    double y1 = subpath->getY(j+1);
                    ofdPath->CurveTo(Point_t(x0, y0), Point_t(x1, y1), Point_t(x, y));

                    j += 3;
                } else {
                    x = subpath->getX(j);
                    y = subpath->getY(j);
                    ofdPath->LineTo(Point_t(x, y));
                    ++j;
                }
            }
            //if ( subpath->isClosed() ){
                //ofdPath->ClosePath();
            //}
        }
    }

    return ofdPath;
}

// -------- GfxFont_to_OfdFont() --------
FontPtr GfxFont_to_OfdFont(GfxFont *gfxFont, XRef *xref){
    if ( gfxFont == nullptr ) return nullptr;

    FontPtr ofdFont = std::make_shared<Font>();

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
        ofdFont->FontType = ofd::FontType::CIDType2;
    } else if (fontType == fontType1 ){
        ofdFont->FontType = ofd::FontType::Type1;
    } else if (fontType == fontType3 ){
        ofdFont->FontType = ofd::FontType::Type3;
    } else if (fontType == fontTrueType ){
        ofdFont->FontType = ofd::FontType::TrueType;
    } else {
        ofdFont->FontType = ofd::FontType::Unknown;
    }

    // -------- FontLoc --------
    GfxFontLoc *fontLoc = gfxFont->locateFont(xref, nullptr);
    if ( fontLoc != nullptr ){
        if ( fontLoc->locType == gfxFontLocEmbedded ){
            ofdFont->FontLoc = ofd::FontLocation::Embedded;
        } else if ( fontLoc->locType == gfxFontLocExternal ){
            ofdFont->FontLoc = ofd::FontLocation::External;
            ofdFont->FontFile = std::string(fontLoc->path->getCString());
        } else if ( fontLoc->locType == gfxFontLocResident ){
            ofdFont->FontLoc = ofd::FontLocation::Resident;
        } else {
            ofdFont->FontLoc = ofd::FontLocation::Unknown;
        }
        delete fontLoc;
        fontLoc = nullptr;
    } else {
        LOG(WARNING) << "fontLoc == nullptr.";
    }

    // -------- FontData --------
    //int fontDataSize = 0;
    //char *fontData = gfxFont->readEmbFontFile(xref, &fontDataSize);

    // FIXME
    //ofdFont->m_fontData = fontData;
    //ofdFont->m_fontDataSize = fontDataSize;
    //ofdFont->CreateFromData(fontData, fontDataSize);

    //// FIXME
    //// Export running font data.
    //{
        //std::string fontfile = std::string("font_running_") + std::to_string(ofdFont->ID) + ".ttf";
        //utils::WriteFileData(fontfile, fontData, fontDataSize);
    //}

    //int *codeToGID = nullptr;
    //size_t codeToGIDLen = 0;
    //std::tie(codeToGID, codeToGIDLen) = getCodeToGID(gfxFont, fontData, fontDataSize);

    return ofdFont;
}

ofd::ColorPtr GfxColor_to_OfdColor(GfxRGB *gfxColor){
    uint32_t r = colToDbl(gfxColor->r) * 255.0;
    uint32_t g = colToDbl(gfxColor->g) * 255.0;
    uint32_t b = colToDbl(gfxColor->b) * 255.0;
    LOG(DEBUG) << "GfxColor_to_OfdColor() (" << gfxColor->r << "," << gfxColor->g << "," << gfxColor->b << ")" << " to (" << r << "," << g << "," << b << ")";
    return Color::Instance(r, g, b);
}

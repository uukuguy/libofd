#ifndef __GFX2OFD_H__
#define __GFX2OFD_H__

#include "GfxFont.h"
#include "GfxState.h"
#include "ofd/Font.h"
#include "ofd/Path.h"
#include "ofd/Color.h"

ofd::PathPtr GfxPath_to_OfdPath(GfxPath *gfxPath);
ofd::FontPtr GfxFont_to_OfdFont(GfxFont *gfxFont, XRef *xref);
ofd::ColorPtr GfxColor_to_OfdColor(GfxRGB *gfxColor);

#endif // __GFX2OFD_H__

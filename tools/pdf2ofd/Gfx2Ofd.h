#ifndef __GFX2OFD_H__
#define __GFX2OFD_H__

#include "GfxFont.h"
#include "GfxState.h"
#include "ofd/Font.h"
#include "ofd/Path.h"

ofd::PathPtr GfxPath_to_OfdPath(GfxPath *gfxPath);
ofd::FontPtr GfxFont_to_OFDFont(GfxFont *gfxFont, XRef *xref);

#endif // __GFX2OFD_H__

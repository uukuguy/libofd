#ifndef __GFX2OFD_H__
#define __GFX2OFD_H__

#include "OFDFont.h"
#include "GfxFont.h"
#include "GfxState.h"
#include "ofd/OfdPath.h"

ofd::OfdPathPtr GfxPath_to_OfdPath(GfxPath *gfxPath);
ofd::OFDFontPtr GfxFont_to_OFDFont(GfxFont *gfxFont, XRef *xref);

#endif // __GFX2OFD_H__

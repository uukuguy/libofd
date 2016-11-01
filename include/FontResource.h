#ifndef __FONTRESOURCE_H__
#define __FONTRESOURCE_H__

namespace ofd{
    
class FontResource {
public:
    FontResource();
    virtual ~FontResource();

    static FontResource *NewFontResource();

    virtual bool AddFontFace(int fontID, int faceIndex, const char *fontFaceBuf, size_t bufSize) = 0;
    virtual void* GetFontFace(int fontID) const = 0;

}; // class FontResource

}; // namespace ofd

#endif // __FONTRESOURCE_H__

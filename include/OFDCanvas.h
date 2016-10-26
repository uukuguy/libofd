#ifndef __OFDCANVAS_H__
#define __OFDCANVAS_H__

#include <memory>

namespace ofd {

const int defaultDPI = 300;

class OFDCanvas {
public:
    OFDCanvas();
    virtual ~OFDCanvas();

    bool AddFontFace(int fontID, int faceIndex, const char *fontFaceBuf, size_t bufSize);
    bool SetCharSize(int fontID, int ptSize, int dpiX = defaultDPI, int dpiY = 0);
    bool SetPixelSize(int fontID, int pixelWidth, int pixelHeight = 0);
    void WriteGlyph(int fontID, double fontSize, uint64_t charcode);
    void DrawGlyph(double x, double y, const std::string &text, int fontID, double fontSize);

public:
    class DrawDevice;

private:
    std::unique_ptr<DrawDevice> m_drawDevice;

}; // class OFDCanvas

}; // namespace ofd

#endif // __OFDCANVAS_H__

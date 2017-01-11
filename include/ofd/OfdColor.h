#ifndef __OFD_COLOR_H__
#define __OFD_COLOR_H__

#include <memory>
#include <string.h>
#include <stdint.h>

namespace ofd {

    typedef int OfdColorChannel;

    static inline OfdColorChannel doubleToColor(double x) {
      return (OfdColorChannel)(x * 0x10000);
    }

    static inline double colorToDouble(OfdColorChannel x) {
      return (double)x / (double)0x10000;
    }

    static inline uint8_t doubleToByte(double x) {
      return (x * 255.0);
    }

    static inline double byteToDouble(uint8_t x) {
      return (double)x / (double)255.0;
    }

    // **************** struct OfdColor ****************
    typedef struct OfdColor {

        OfdColorChannel channels[32];

        OfdColor(){
            memset(channels, 0, sizeof(OfdColorChannel) * 32);
        }

    } OfdColor_t;
    typedef std::shared_ptr<OfdColor> OfdColorPtr;

    typedef OfdColorChannel OfdGray;

    typedef struct OfdRGB{
        OfdColorChannel R, G, B;
    } OfdRGB_t;

    typedef struct OfdCMYK{
        OfdColorChannel C, M, Y, K;
    } OfdCMYK_t;

    // **************** class OfdColorSpace ****************
    class OfdColorSpace{
    public:
        OfdColorSpace();
        virtual ~OfdColorSpace();

        virtual void GetGray(OfdColor *color, OfdGray *gray) = 0;
        virtual void GetRGB(OfdColor *color, OfdRGB *rgb) = 0;
        virtual void GetCMYK(OfdColor *color, OfdCMYK *cmyk) = 0;

    }; // class OfdColorSpace

    // **************** class OfdGrayColorSpace ****************
    class OfdGrayColorSpace : public OfdColorSpace {
    public:
        OfdGrayColorSpace();
        virtual ~OfdGrayColorSpace();

        virtual void GetGray(OfdColor *color, OfdGray *gray) override;
        virtual void GetRGB(OfdColor *color, OfdRGB *rgb) override;
        virtual void GetCMYK(OfdColor *color, OfdCMYK *cmyk) override;

    }; // class OfdGrayColorSpace

    // **************** class OfdRGBColorSpace ****************
    class OfdRGBColorSpace : public OfdColorSpace {
    public:
        OfdRGBColorSpace();
        virtual ~OfdRGBColorSpace();

        virtual void GetGray(OfdColor *color, OfdGray *gray) override;
        virtual void GetRGB(OfdColor *color, OfdRGB *rgb) override;
        virtual void GetCMYK(OfdColor *color, OfdCMYK *cmyk) override;

    }; // class OfdRGBColorSpace

    // **************** class OfdCMYKColorSpace ****************
    class OfdCMYKColorSpace : public OfdColorSpace {
    public:
        OfdCMYKColorSpace();
        virtual ~OfdCMYKColorSpace();

        virtual void GetGray(OfdColor *color, OfdGray *gray) override;
        virtual void GetRGB(OfdColor *color, OfdRGB *rgb) override;
        virtual void GetCMYK(OfdColor *color, OfdCMYK *cmyk) override;

    }; // class OfdCMYKColorSpace

}; // namespace ofd

#endif // __OFD_COLOR_H__

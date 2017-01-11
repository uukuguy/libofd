#include "ofd/OfdColor.h"

using namespace ofd;

// **************** class OfdColorSpace ****************

OfdColorSpace::OfdColorSpace(){
}

OfdColorSpace::~OfdColorSpace(){
}


static inline OfdColorChannel clip01(OfdColorChannel x) {
  return (x < 0) ? 0 : (x > 0x10000) ? 0x10000 : x;
}

static inline __attribute__((unused)) double clip01(double x) {
  return (x < 0) ? 0 : (x > 1) ? 1 : x;
}

static inline void cmykToRGBMatrixMultiplication(const double &c, const double &m, const double &y, const double &k, const double &c1, const double &m1, const double &y1, const double &k1, double &r, double &g, double &b) {
  double x;
  // this is a matrix multiplication, unrolled for performance
  //                        C M Y K
  x = c1 * m1 * y1 * k1; // 0 0 0 0
  r = g = b = x;
  x = c1 * m1 * y1 * k;  // 0 0 0 1
  r += 0.1373 * x;
  g += 0.1216 * x;
  b += 0.1255 * x;
  x = c1 * m1 * y  * k1; // 0 0 1 0
  r += x;
  g += 0.9490 * x;
  x = c1 * m1 * y  * k;  // 0 0 1 1
  r += 0.1098 * x;
  g += 0.1020 * x;
  x = c1 * m  * y1 * k1; // 0 1 0 0
  r += 0.9255 * x;
  b += 0.5490 * x;
  x = c1 * m  * y1 * k;  // 0 1 0 1
  r += 0.1412 * x;
  x = c1 * m  * y  * k1; // 0 1 1 0
  r += 0.9294 * x;
  g += 0.1098 * x;
  b += 0.1412 * x;
  x = c1 * m  * y  * k;  // 0 1 1 1
  r += 0.1333 * x;
  x = c  * m1 * y1 * k1; // 1 0 0 0
  g += 0.6784 * x;
  b += 0.9373 * x;
  x = c  * m1 * y1 * k;  // 1 0 0 1
  g += 0.0588 * x;
  b += 0.1412 * x;
  x = c  * m1 * y  * k1; // 1 0 1 0
  g += 0.6510 * x;
  b += 0.3137 * x;
  x = c  * m1 * y  * k;  // 1 0 1 1
  g += 0.0745 * x;
  x = c  * m  * y1 * k1; // 1 1 0 0
  r += 0.1804 * x;
  g += 0.1922 * x;
  b += 0.5725 * x;
  x = c  * m  * y1 * k;  // 1 1 0 1
  b += 0.0078 * x;
  x = c  * m  * y  * k1; // 1 1 1 0
  r += 0.2118 * x;
  g += 0.2119 * x;
  b += 0.2235 * x;
}

// **************** class OfdGrayColorSpace ****************

OfdGrayColorSpace::OfdGrayColorSpace(){
}

OfdGrayColorSpace::~OfdGrayColorSpace(){
}

void OfdGrayColorSpace::GetGray(OfdColor *color, OfdGray *gray){
    *gray = clip01(color->channels[0]);
}

void OfdGrayColorSpace::GetRGB(OfdColor *color, OfdRGB *rgb){
    rgb->R = rgb->G = rgb->B = clip01(color->channels[0]);
}

void OfdGrayColorSpace::GetCMYK(OfdColor *color, OfdCMYK *cmyk){
      cmyk->C = cmyk->M = cmyk->Y = 0;
      cmyk->K = clip01(0x10000 - color->channels[0]);
}

// **************** class OfdRGBColorSpace ****************

OfdRGBColorSpace::OfdRGBColorSpace(){
}

OfdRGBColorSpace::~OfdRGBColorSpace(){
}

void OfdRGBColorSpace::GetGray(OfdColor *color, OfdGray *gray){
    *gray = clip01((OfdColorChannel)(0.3 * color->channels[0] +
                0.59 * color->channels[1] +
                0.11 * color->channels[2] + 0.5));
}

void OfdRGBColorSpace::GetRGB(OfdColor *color, OfdRGB *rgb){
    rgb->R = clip01(color->channels[0]);
    rgb->G = clip01(color->channels[1]);
    rgb->B = clip01(color->channels[2]);
}

void OfdRGBColorSpace::GetCMYK(OfdColor *color, OfdCMYK *cmyk){
    OfdColorChannel c, m, y, k;

    c = clip01(0x10000 - color->channels[0]);
    m = clip01(0x10000 - color->channels[1]);
    y = clip01(0x10000 - color->channels[2]);
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
    cmyk->C = c - k;
    cmyk->M = m - k;
    cmyk->Y = y - k;
    cmyk->K = k;
}

// **************** class OfdCMYKColorSpace ****************

OfdCMYKColorSpace::OfdCMYKColorSpace(){
}

OfdCMYKColorSpace::~OfdCMYKColorSpace(){
}

void OfdCMYKColorSpace::GetGray(OfdColor *color, OfdGray *gray){
    *gray = clip01((OfdColorChannel)(0x10000 - color->channels[3]
                - 0.3  * color->channels[0]
                - 0.59 * color->channels[1]
                - 0.11 * color->channels[2] + 0.5));
}

void OfdCMYKColorSpace::GetRGB(OfdColor *color, OfdRGB *rgb){
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    c = colorToDouble(color->channels[0]);
    m = colorToDouble(color->channels[1]);
    y = colorToDouble(color->channels[2]);
    k = colorToDouble(color->channels[3]);
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    rgb->R = clip01(doubleToColor(r));
    rgb->G = clip01(doubleToColor(g));
    rgb->B = clip01(doubleToColor(b));
}

void OfdCMYKColorSpace::GetCMYK(OfdColor *color, OfdCMYK *cmyk){
    cmyk->C = clip01(color->channels[0]);
    cmyk->M = clip01(color->channels[1]);
    cmyk->Y = clip01(color->channels[2]);
    cmyk->K = clip01(color->channels[3]);
}


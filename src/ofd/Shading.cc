#include <math.h>
#include "ofd/Shading.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;

cairo_pattern_t *RadialShading::CreateFillPattern(cairo_t *cr){

    cairo_pattern_t *fillPattern = nullptr;

    double x0, y0, r0, x1, y1, r1;
    x0 = StartPoint.x;
    y0 = StartPoint.y;
    x1 = EndPoint.x;
    y1 = EndPoint.y;
    r0 = StartRadius;
    r1 = EndRadius;

    double dx, dy, dr;
    dx = x1 - x0;
    dy = y1 - y0;
    dr = r1 - r0;

    // Cairo/pixman do not work well with a very large or small scaled
    // matrix.  See cairo bug #81657.
    //
    // As a workaround, scale the pattern by the average of the vertical
    // and horizontal scaling of the current transformation matrix.
    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);
    double scale;
    scale = (sqrt(matrix.xx * matrix.xx + matrix.yx * matrix.yx)
            + sqrt(matrix.xy * matrix.xy + matrix.yy * matrix.yy)) / 2;
    cairo_matrix_init_scale(&matrix, scale, scale);

    //double sMin;
    //double sMax;
    //fillPattern = cairo_pattern_create_radial ((x0 + sMin * dx) * scale,
            //(y0 + sMin * dy) * scale,
            //(r0 + sMin * dr) * scale,
            //(x0 + sMax * dx) * scale,
            //(y0 + sMax * dy) * scale,
            //(r0 + sMax * dr) * scale);
    for ( size_t i = 0 ; i < ColorSegments.size() ; i++ ){

        const ColorStop_t &cs = ColorSegments[i];
        const ColorPtr color = cs.Color;
        double pos = cs.Position;

        double r = color->Value.RGB.Red / 255.0;
        double g = color->Value.RGB.Green / 255.0;
        double b = color->Value.RGB.Blue / 255.0;
        double a = color->Alpha / 255.0;
        cairo_pattern_add_color_stop_rgba(fillPattern, pos, r, g, b, a);
    } 

    cairo_pattern_set_matrix(fillPattern, &matrix);

    if ( Extend ){
        cairo_pattern_set_extend(fillPattern, CAIRO_EXTEND_PAD);
    } else {
        cairo_pattern_set_extend (fillPattern, CAIRO_EXTEND_NONE);
    }

    return fillPattern;
}

// ======== RadialShading::WriteShadingXML() ========
void RadialShading::WriteShadingXML(utils::XMLWriter &writer) const{

    writer.StartElement("RadialShd");{
    } writer.EndElement();
}


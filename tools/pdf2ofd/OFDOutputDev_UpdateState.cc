
#include "OFDOutputDev.h"
#include "utils/logger.h"

extern std::ofstream cairoLogFile;

void OFDOutputDev::updateAll(GfxState *state){
    updateLineDash(state);
    updateLineJoin(state);
    updateLineCap(state);
    updateLineWidth(state);
    updateFlatness(state);
    updateMiterLimit(state);
    updateFillColor(state);
    updateStrokeColor(state);
    updateFillOpacity(state);
    updateStrokeOpacity(state);
    updateBlendMode(state);
    m_needFontUpdate = gTrue;
    if (m_textPage != nullptr ){
        m_textPage->updateFont(state);
    }
}

void OFDOutputDev::setDefaultCTM(double *ctm) {
    cairo_matrix_t matrix;
    matrix.xx = ctm[0];
    matrix.yx = ctm[1];
    matrix.xy = ctm[2];
    matrix.yy = ctm[3];
    matrix.x0 = ctm[4];
    matrix.y0 = ctm[5];

    LOG(INFO) << "[imageSurface] setDefaultCTM (" << ctm[0] << ", " << ctm[1] << ", " << ctm[2]
        << ", " << ctm[3] << ", " << ctm[4] << ", " << ctm[5] << ")";

    std::stringstream ssCairoLog;

    ssCairoLog << "[imageSurface] setDefaultCTM (" << ctm[0] << ", " << ctm[1] << ", " << ctm[2]
        << ", " << ctm[3] << ", " << ctm[4] << ", " << ctm[5] << ")";

    cairo_transform(m_cairo, &matrix);
    if (m_cairoShape){
        cairo_transform(m_cairoShape, &matrix);
    }
    //if ( m_cairoRender != nullptr ){
    //m_cairoRender->Transform(&matrix);
    //}
    m_matrix = matrix;

    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());

    OutputDev::setDefaultCTM(ctm);
}

void OFDOutputDev::updateCTM(GfxState *state, double m11, double m12, double m21, double m22, double m31, double m32) {
    cairo_matrix_t matrix, invert_matrix;
    matrix.xx = m11;
    matrix.yx = m12;
    matrix.xy = m21;
    matrix.yy = m22;
    matrix.x0 = m31;
    matrix.y0 = m32;

    LOG(INFO) << "[imageSurface] updateCTM (" << m11 << ", " << m12 << ", " << m21 
        << ", " << m22 << ", " << m31 << ", " << m32 << ")";

    std::stringstream ssCairoLog;
    ssCairoLog << "[imageSurface] updateCTM (" << m11 << ", " << m12 << ", " << m21 
        << ", " << m22 << ", " << m31 << ", " << m32 << ")";
    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());

    /* Make sure the matrix is invertible before setting it.
     * cairo will blow up if we give it a matrix that's not
     * invertible, so we need to check before passing it
     * to cairo_transform. Ignoring it is likely to give better
     * results than not rendering anything at all. See #14398
     *
     * Ideally, we could do the cairo_transform
     * and then check if anything went wrong and fix it then
     * instead of having to invert the matrix. */
    invert_matrix = matrix;
    if (cairo_matrix_invert(&invert_matrix)) {
        LOG(ERROR) << "matrix not invertible\n";
        return;
    }

    cairo_transform(m_cairo, &matrix);
    if (m_cairoShape != nullptr){
        cairo_transform(m_cairoShape, &matrix);
    }
    if ( m_cairoRender != nullptr ){
        m_cairoRender->Transform(&matrix);
    }

    // FIXME - clipPath
    m_matrix = matrix;
    //double ctm[6];
    //ctm[0] = m_matrix.xx;
    //ctm[1] = m_matrix.yx;
    //ctm[2] = m_matrix.xy;
    //ctm[3] = m_matrix.yy;
    //ctm[4] = m_matrix.x0;
    //ctm[5] = m_matrix.y0;
    //ofd::ConcatCTM(ctm, m11, m12, m21, m22, m31, m32); 
    //m_matrix.xx = ctm[0];
    //m_matrix.yx = ctm[1];
    //m_matrix.xy = ctm[2];
    //m_matrix.yy = ctm[3];
    //m_matrix.x0 = ctm[4];
    //m_matrix.y0 = ctm[5];

    updateLineDash(state);
    updateLineJoin(state);
    updateLineCap(state);
    updateLineWidth(state);
}

void OFDOutputDev::updateLineDash(GfxState *state){
    double *dashPattern;
    int dashLength;
    double dashStart;

    state->getLineDash(&dashPattern, &dashLength, &dashStart);
    cairo_set_dash(m_cairo, dashPattern, dashLength, dashStart);
    if (m_cairoShape != nullptr ){
        cairo_set_dash(m_cairoShape, dashPattern, dashLength, dashStart);
    }
}

void OFDOutputDev::updateLineJoin(GfxState *state){
    switch (state->getLineJoin()){
        case 0:
            cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_MITER);
            break;
        case 1:
            cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_ROUND);
            break;
        case 2:
            cairo_set_line_join(m_cairo, CAIRO_LINE_JOIN_BEVEL);
            break;
    }
    if ( m_cairoShape != nullptr ){
        cairo_set_line_join(m_cairoShape, cairo_get_line_join(m_cairo));
    }
}

void OFDOutputDev::updateLineCap(GfxState *state){
    switch (state->getLineCap()) {
        case 0:
            cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_BUTT);
            break;
        case 1:
            cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_ROUND);
            break;
        case 2:
            cairo_set_line_cap(m_cairo, CAIRO_LINE_CAP_SQUARE);
            break;
    }
    if ( m_cairoShape != nullptr ){
        cairo_set_line_cap(m_cairoShape, cairo_get_line_cap(m_cairo));
    }
}

void OFDOutputDev::updateMiterLimit(GfxState *state){
    cairo_set_miter_limit(m_cairo, state->getMiterLimit());
    if ( m_cairoShape != nullptr ){
        cairo_set_miter_limit(m_cairoShape, state->getMiterLimit());
    }
}


void OFDOutputDev::updateLineWidth(GfxState *state){
    LOG(INFO) <<  "[imageSurface] updateLineWidth() line width: " << state->getLineWidth();
    m_adjustedStrokeWidth = false;
    double width = state->getLineWidth();
    if ( m_strokeAdjust && !m_printing ) {
        double x, y;
        x = y = width;

        /* find out line width in device units */
        cairo_user_to_device_distance(m_cairo, &x, &y);
        if (fabs(x) <= 1.0 && fabs(y) <= 1.0) {
            /* adjust width to at least one device pixel */
            x = y = 1.0;
            cairo_device_to_user_distance(m_cairo, &x, &y);
            width = std::min(fabs(x),fabs(y));
            m_adjustedStrokeWidth = true;
        }
    } else if (width == 0.0) {
        /* Cairo does not support 0 line width == 1 device pixel. Find out
         * how big pixels (device unit) are in the x and y
         * directions. Choose the smaller of the two as our line width.
         */
        double x = 1.0, y = 1.0;
        if (m_printing) {
            // assume printer pixel size is 1/600 inch
            x = 72.0/600;
            y = 72.0/600;
        }
        cairo_device_to_user_distance(m_cairo, &x, &y);
        width = std::min(fabs(x),fabs(y));
    }
    cairo_set_line_width(m_cairo, width);
    if ( m_cairoRender != nullptr ){
        m_cairoRender->SetLineWidth(width);
    }
    m_lineWidth = width;
    if (m_cairoShape){
        cairo_set_line_width(m_cairoShape, cairo_get_line_width (m_cairo));
    }

    std::stringstream ssCairoLog;
    ssCairoLog << "- updateLineWidth - " << " LineWidth:" << width;

    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
}

void OFDOutputDev::updateFillColor(GfxState *state){
    GfxRGB color = m_fillColor;

    if ( m_uncoloredPattern ) return;

    state->getFillRGB(&m_fillColor);
    if (cairo_pattern_get_type(m_fillPattern) != CAIRO_PATTERN_TYPE_SOLID ||
            color.r != m_fillColor.r ||
            color.g != m_fillColor.g ||
            color.b != m_fillColor.b)
    {
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = cairo_pattern_create_rgba(colToDbl(m_fillColor.r),
                colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b),
                m_fillOpacity);

        if ( m_cairoRender != nullptr ){
            m_cairoRender->UpdateFillPattern(colToDbl(m_fillColor.r), colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b), m_fillOpacity);
        }
        LOG(INFO) <<  "[imageSurface] fill color: " << m_fillColor.r << ", " <<  m_fillColor.g << ", " << m_fillColor.b;


        std::stringstream ssCairoLog;
        ssCairoLog << "- updateFillColor - " << " fillColor:(" << m_fillColor.r << ", " <<  m_fillColor.g << ", " << m_fillColor.b << ") ";

        std::string cairoLog = ssCairoLog.str() + "\n";
        cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
    }
}

void OFDOutputDev::updateStrokeColor(GfxState *state){
    GfxRGB color = m_strokeColor;

    if ( m_uncoloredPattern ) return;

    state->getStrokeRGB(&m_strokeColor);
    if (cairo_pattern_get_type(m_strokePattern) != CAIRO_PATTERN_TYPE_SOLID ||
            color.r != m_strokeColor.r ||
            color.g != m_strokeColor.g ||
            color.b != m_strokeColor.b)
    {
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = cairo_pattern_create_rgba(colToDbl(m_strokeColor.r),
                colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b),
                m_strokeOpacity);

        if ( m_cairoRender != nullptr ){
            m_cairoRender->UpdateStrokePattern(colToDbl(m_strokeColor.r), colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b), m_strokeOpacity);
        }

        LOG(INFO) <<  "[imageSurface] stroke color: " << m_strokeColor.r << ", " << m_strokeColor.g << ", " <<  m_strokeColor.b;

        std::stringstream ssCairoLog;
        ssCairoLog << "- updateStrokeColor - " << " strokeColor:(" << m_strokeColor.r << "," <<  m_strokeColor.g << "," << m_strokeColor.b << ") ";

        std::string cairoLog = ssCairoLog.str() + "\n";
        cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
    }
}

void OFDOutputDev::updateFillOpacity(GfxState *state){
    double opacity = m_fillOpacity;

    if ( m_uncoloredPattern ) return;

    m_fillOpacity = state->getFillOpacity();
    if ( opacity != m_fillOpacity) {
        cairo_pattern_destroy(m_fillPattern);
        m_fillPattern = cairo_pattern_create_rgba(colToDbl(m_fillColor.r),
                colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b),
                m_fillOpacity);

        if ( m_cairoRender != nullptr ){
            m_cairoRender->UpdateFillPattern(colToDbl(m_fillColor.r), colToDbl(m_fillColor.g),
                colToDbl(m_fillColor.b), m_fillOpacity);
        }
        LOG(INFO) << "[imageSurface] updateFillOpacity() fill opacity: " << m_fillOpacity;

        std::stringstream ssCairoLog;
        ssCairoLog << "- updateFillOpacity - " << " Opacity:" << m_fillOpacity << "(old value:" << opacity << ")"; 

        std::string cairoLog = ssCairoLog.str() + "\n";
        cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
    }
}

void OFDOutputDev::updateStrokeOpacity(GfxState *state){
    double opacity = m_strokeOpacity;

    if ( m_uncoloredPattern ) return;

    m_strokeOpacity = state->getStrokeOpacity();
    if ( opacity != m_strokeOpacity) {
        cairo_pattern_destroy(m_strokePattern);
        m_strokePattern = cairo_pattern_create_rgba(colToDbl(m_strokeColor.r),
                colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b),
                m_strokeOpacity);

        if ( m_cairoRender != nullptr ){
            m_cairoRender->UpdateStrokePattern(colToDbl(m_strokeColor.r), colToDbl(m_strokeColor.g),
                colToDbl(m_strokeColor.b), m_strokeOpacity);
        }

        LOG(INFO) <<  "[imageSurface] updateStrokeOpacity() stroke opacity: " << m_strokeOpacity;

        std::stringstream ssCairoLog;
        ssCairoLog << "- updateStrokeOpacity - " << " Opacity:" << m_strokeOpacity << "(old value:" << opacity << ")"; 

        std::string cairoLog = ssCairoLog.str() + "\n";
        cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
    }
}

void OFDOutputDev::updateFillColorStop(GfxState *state, double offset){
    if ( m_uncoloredPattern ) return;

    state->getFillRGB(&m_fillColor);

    double r = colToDbl(m_fillColor.r);
    double g = colToDbl(m_fillColor.g);
    double b = colToDbl(m_fillColor.b);
    double a = m_fillOpacity;

    cairo_pattern_add_color_stop_rgba(m_fillPattern, offset, r, g, b, a);

    ofd::ColorPtr color = ofd::Color::Instance(r * 255, g * 255, b * 255, nullptr, a * 255);

    m_colorStops.push_back(ofd::ColorStopArray::value_type(color, offset));

    LOG(DEBUG) << "fill color stop: " << offset << " (" <<
        m_fillColor.r << ", " <<
        m_fillColor.g << ", " <<
        m_fillColor.b;

    std::stringstream ssCairoLog;
    ssCairoLog << "- updateColorStop - " << " offset:" << offset << " color:(" << m_fillColor.r << ", " << m_fillColor.g << ", " << m_fillColor.b << ") " << " fillOpacity:" << m_fillOpacity;

    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
}

void OFDOutputDev::updateBlendMode(GfxState *state){
    switch (state->getBlendMode()) {
        default:
        case gfxBlendNormal:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_OVER);
            break;
        case gfxBlendMultiply:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_MULTIPLY);
            break;
        case gfxBlendScreen:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_SCREEN);
            break;
        case gfxBlendOverlay:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_OVERLAY);
            break;
        case gfxBlendDarken:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_DARKEN);
            break;
        case gfxBlendLighten:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_LIGHTEN);
            break;
        case gfxBlendColorDodge:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_COLOR_DODGE);
            break;
        case gfxBlendColorBurn:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_COLOR_BURN);
            break;
        case gfxBlendHardLight:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HARD_LIGHT);
            break;
        case gfxBlendSoftLight:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_SOFT_LIGHT);
            break;
        case gfxBlendDifference:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_DIFFERENCE);
            break;
        case gfxBlendExclusion:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_EXCLUSION);
            break;
        case gfxBlendHue:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_HUE);
            break;
        case gfxBlendSaturation:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_SATURATION);
            break;
        case gfxBlendColor:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_COLOR);
            break;
        case gfxBlendLuminosity:
            cairo_set_operator(m_cairo, CAIRO_OPERATOR_HSL_LUMINOSITY);
            break;
    }

    std::stringstream ssCairoLog;
    ssCairoLog << "- updateBlendMode - " << " BlendMode:" << (int)state->getBlendMode(); 

    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());
}


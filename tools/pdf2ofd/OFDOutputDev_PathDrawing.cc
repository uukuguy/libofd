#include <Gfx.h>
#include "ofd/Page.h"
#include "ofd/Path.h"
#include "ofd/PathObject.h"
#include "ofd/Color.h"
#include "ofd/Shading.h"
#include "utils/logger.h"
#include "OFDOutputDev.h"
#include "Gfx2Ofd.h"

using namespace ofd;

extern std::ofstream cairoLogFile;

void showCairoMatrix(cairo_t *cr, const std::string &title, const std::string &msg);

/* Tolerance in pixels for checking if strokes are horizontal or vertical
 * lines in device space */
#define STROKE_COORD_TOLERANCE 0.5

/* Align stroke coordinate i if the point is the start or end of a
 * horizontal or vertical line */
void OFDOutputDev::alignStrokeCoords(GfxSubpath *subpath, int i, double *x, double *y) {
    double x1, y1, x2, y2;
    GBool align = gFalse;

    x1 = subpath->getX(i);
    y1 = subpath->getY(i);
    cairo_user_to_device(m_cairo, &x1, &y1);

    // Does the current coord and prev coord form a horiz or vert line?
    if (i > 0 && !subpath->getCurve(i - 1)) {
        x2 = subpath->getX(i - 1);
        y2 = subpath->getY(i - 1);
        cairo_user_to_device(m_cairo, &x2, &y2);
        if (fabs(x2 - x1) < STROKE_COORD_TOLERANCE || fabs(y2 - y1) < STROKE_COORD_TOLERANCE)
            align = gTrue;
    }

    // Does the current coord and next coord form a horiz or vert line?
    if (i < subpath->getNumPoints() - 1 && !subpath->getCurve(i + 1)) {
        x2 = subpath->getX(i + 1);
        y2 = subpath->getY(i + 1);
        cairo_user_to_device(m_cairo, &x2, &y2);
        if (fabs(x2 - x1) < STROKE_COORD_TOLERANCE || fabs(y2 - y1) < STROKE_COORD_TOLERANCE)
            align = gTrue;
    }

    *x = subpath->getX(i);
    *y = subpath->getY(i);
    if (align) {
        /* see http://www.cairographics.org/FAQ/#sharp_lines */
        cairo_user_to_device(m_cairo, x, y);
        *x = floor(*x) + 0.5;
        *y = floor(*y) + 0.5;
        cairo_device_to_user(m_cairo, x, y);
    }
}

#undef STROKE_COORD_TOLERANCE

void OFDOutputDev::doPath(cairo_t *cairo, GfxState *state, GfxPath *path){
    GfxSubpath *subpath;
    int i, j;
    double x, y;
    cairo_new_path(cairo);
    for (i = 0; i < path->getNumSubpaths(); ++i) {
        subpath = path->getSubpath(i);
        if (subpath->getNumPoints() > 0) {
            if (m_alignStrokeCoords) {
                alignStrokeCoords(subpath, 0, &x, &y);
            } else {
                x = subpath->getX(0);
                y = subpath->getY(0);
            }
            cairo_move_to(cairo, x, y);
            j = 1;
            while (j < subpath->getNumPoints()) {
                if (subpath->getCurve(j)) {
                    if (m_alignStrokeCoords) {
                        alignStrokeCoords(subpath, j + 2, &x, &y);
                    } else {
                        x = subpath->getX(j+2);
                        y = subpath->getY(j+2);
                    }
                    cairo_curve_to( cairo,
                            subpath->getX(j), subpath->getY(j),
                            subpath->getX(j+1), subpath->getY(j+1),
                            x, y);

                    j += 3;
                } else {
                    if (m_alignStrokeCoords) {
                        alignStrokeCoords(subpath, j, &x, &y);
                    } else {
                        x = subpath->getX(j);
                        y = subpath->getY(j);
                    }
                    cairo_line_to(cairo, x, y);
                    ++j;
                }
            }
            if (subpath->isClosed()) {
                cairo_close_path(cairo);
            }
        }
    }
}


void OFDOutputDev::stroke(GfxState *state) {
  //if (inType3Char) {
      //GfxGray gray;
      //state->getFillGray(&gray);
      //if (colToDbl(gray) > 0.5)
	  //return;
  //}

    LOG(INFO) << "[imageSurface] DrawPathObject Stroke Path";

    // Add PathObject
    PathPtr ofdPath = GfxPath_to_OfdPath(state->getPath());
    if ( ofdPath != nullptr ){
        PathObjectPtr pathObject = std::make_shared<PathObject>(m_currentOFDPage->GetBodyLayer());
        pathObject->SetPath(ofdPath);
        LOG(DEBUG) <<  "stroke color in stroke(): " << m_strokeColor.r << ", " << m_strokeColor.g << ", " <<  m_strokeColor.b;
        ColorPtr strokeColor = GfxColor_to_OfdColor(&m_strokeColor);
        strokeColor->Alpha = m_strokeOpacity * 255.0;
        pathObject->SetStrokeColor(strokeColor);
        pathObject->LineWidth = m_lineWidth;

        LOG(INFO) << "[imageSurface] DrawPathObject m_matrix=(" << m_matrix.xx << "," << m_matrix.yx << "," << m_matrix.xy << "," << m_matrix.yy << "," << m_matrix.x0 << "," << m_matrix.y0 << ")";
        showCairoMatrix(m_cairo, "imageSurface", "DrawPathObject cairo_matrix");

        cairo_matrix_t matrix;
        cairo_get_matrix(m_cairo, &matrix);
        //LOG(INFO) << "[imageSurface] DrawPathObject cairo_get_matrix() matrix=(" << matrix.xx << "," << matrix.yx << "," << matrix.xy << "," << matrix.yy << "," << matrix.x0 << "," << matrix.y0 << ")";

        double *gfxCTM = state->getCTM();
        cairo_matrix_t matrix1;
        matrix1.xx = gfxCTM[0];
        matrix1.yx = gfxCTM[1];

        matrix1.xy = gfxCTM[2];
        matrix1.yy = gfxCTM[3];
        matrix1.x0 = gfxCTM[4];
        matrix1.y0 = gfxCTM[5];

        // FIXME
        cairo_matrix_t objMatrix = matrix1;
        //cairo_matrix_t objMatrix = m_matrix;
        //cairo_matrix_t objMatrix = matrix;

        LOG(INFO) << "[imageSurface] DrawPathObject objMatrix=(" << objMatrix.xx << "," << objMatrix.yx << "," << objMatrix.xy << "," << objMatrix.yy << "," << objMatrix.x0 << "," << objMatrix.y0 << ")";

        pathObject->CTM[0] = objMatrix.xx;
        pathObject->CTM[1] = objMatrix.yx;
        pathObject->CTM[2] = objMatrix.xy;
        pathObject->CTM[3] = objMatrix.yy;
        pathObject->CTM[4] = objMatrix.x0;
        pathObject->CTM[5] = objMatrix.y0;

        m_currentOFDPage->AddObject(pathObject);

        if ( m_cairoRender != nullptr ){
            ObjectPtr object = std::shared_ptr<ofd::Object>(pathObject);
            m_cairoRender->DrawObject(object);
        }
    }

    if ( m_adjustedStrokeWidth ){
        m_alignStrokeCoords = true;
    }
    doPath(m_cairo, state, state->getPath());
    m_alignStrokeCoords = false;
    cairo_set_source(m_cairo, m_strokePattern);
    if ( m_strokePathClip ) {
        cairo_push_group(m_cairo);
        cairo_stroke(m_cairo);
        cairo_pop_group_to_source(m_cairo);
        fillToStrokePathClip(state);
    } else {
        cairo_stroke(m_cairo);
    }
    if ( m_cairoShape) {
        doPath(m_cairoShape, state, state->getPath());
        cairo_stroke(m_cairoShape);
    }
}

PathObjectPtr OFDOutputDev::createPathObject(GfxState *state){
    PathObjectPtr pathObject = nullptr;

    PathPtr ofdPath = GfxPath_to_OfdPath(state->getPath());
    if ( ofdPath != nullptr ){
        pathObject = std::make_shared<PathObject>(m_currentOFDPage->GetBodyLayer());
        pathObject->SetPath(ofdPath);
        LOG(DEBUG) <<  "fill color in fill(): " << m_fillColor.r << ", " << m_fillColor.g << ", " <<  m_fillColor.b;
        ColorPtr fillColor = GfxColor_to_OfdColor(&m_fillColor);
        fillColor->Alpha = m_fillOpacity * 255.0;
        pathObject->SetFillColor(fillColor);
        pathObject->LineWidth = m_lineWidth;

        if ( m_shading != nullptr ){
            m_shading->SetColorStops(m_colorStops);
            m_colorStops.clear();

            pathObject->FillShading = m_shading;
        }
        m_shading = nullptr;

        LOG(INFO) << "[imageSurface] DrawPathObject m_matrix=(" << m_matrix.xx << "," << m_matrix.yx << "," << m_matrix.xy << "," << m_matrix.yy << "," << m_matrix.x0 << "," << m_matrix.y0 << ")";
        showCairoMatrix(m_cairo, "imageSurface", "DrawPathObject cairo_matrix");

        cairo_matrix_t matrix;
        cairo_get_matrix(m_cairo, &matrix);
        //LOG(INFO) << "[imageSurface] DrawPathObject cairo_get_matrix() matrix=(" << matrix.xx << "," << matrix.yx << "," << matrix.xy << "," << matrix.yy << "," << matrix.x0 << "," << matrix.y0 << ")";

        double *gfxCTM = state->getCTM();
        cairo_matrix_t matrix1;
        matrix1.xx = gfxCTM[0];
        matrix1.yx = gfxCTM[1];
        matrix1.xy = gfxCTM[2];
        matrix1.yy = gfxCTM[3];
        matrix1.x0 = gfxCTM[4];
        matrix1.y0 = gfxCTM[5];

        // FIXME
        cairo_matrix_t objMatrix = matrix1;
        //cairo_matrix_t objMatrix = m_matrix;
        //cairo_matrix_t objMatrix = matrix;

        LOG(INFO) << "[imageSurface] DrawPathObject objMatrix=(" << objMatrix.xx << "," << objMatrix.yx << "," << objMatrix.xy << "," << objMatrix.yy << "," << objMatrix.x0 << "," << objMatrix.y0 << ")";

        pathObject->CTM[0] = objMatrix.xx;
        pathObject->CTM[1] = objMatrix.yx;
        pathObject->CTM[2] = objMatrix.xy;
        pathObject->CTM[3] = objMatrix.yy;
        pathObject->CTM[4] = objMatrix.x0;
        pathObject->CTM[5] = objMatrix.y0;

    }

    return pathObject;
}

//static int xxo = 0;

void OFDOutputDev::fill(GfxState *state) {

    //if (inType3Char) {
    //GfxGray gray;
    //state->getFillGray(&gray);
    //if (colToDbl(gray) > 0.5)
    //return;
    //}

    //LOG(INFO) << "[imageSurface] DrawPathObject Fill Path";

    //LOG(INFO) << "[imageSurface] DrawPathObject m_matrix=(" << m_matrix.xx << "," << m_matrix.yx << "," << m_matrix.xy << "," << m_matrix.yy << "," << m_matrix.x0 << "," << m_matrix.y0 << ")";

    //showCairoMatrix(m_cairo, "imageSurface", "DrawPathObject cairo_matrix");

    std::stringstream ssCairoLog;

    // Add PathObject

    ssCairoLog << "- Draw PathObject - ";
    PathObjectPtr pathObject = createPathObject(state);
    if ( pathObject != nullptr ){
        m_currentOFDPage->AddObject(pathObject);

        if ( m_cairoRender != nullptr ){
            ObjectPtr object = std::shared_ptr<ofd::Object>(pathObject);
            m_cairoRender->DrawObject(object);

            std::string objectString = pathObject->to_string();
            ssCairoLog << objectString;
        }
    }

    std::string cairoLog = ssCairoLog.str() + "\n";
    cairoLogFile.write(cairoLog.c_str(), cairoLog.length());

    // FIXME 渐变色缺陷调试
    //PathPtr path = pathObject->GetPath();
    //size_t numSubpaths = path->GetNumSubpaths();
    //if ( numSubpaths > 1  ){
        //return;
    //} else {
        //ofd::SubpathPtr subpath = path->GetSubpath(0);
        //size_t numPoints = subpath->GetNumPoints();
        //if ( numPoints >= 6 ){
            //return;
        //} else {
            //xxo++;
            //if ( xxo % 3 == 0 ){
                //return;
            //}
        //}
    //}
    //if ( pathObject->ID != 71 ) return;
    //LOG(ERROR) << pathObject->to_string();

    doPath(m_cairo, state, state->getPath());
    cairo_set_fill_rule(m_cairo, CAIRO_FILL_RULE_WINDING);
    cairo_set_source(m_cairo, m_fillPattern);
    //XXX: how do we get the path
    if ( m_maskPattern != nullptr ) {
        cairo_save(m_cairo);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->SaveState();
        }
        cairo_clip(m_cairo);
        if ( m_strokePathClip) {
            cairo_push_group(m_cairo);
            fillToStrokePathClip(state);
            cairo_pop_group_to_source(m_cairo);
        }
        cairo_set_matrix(m_cairo, &m_mask_matrix);
        cairo_mask(m_cairo, m_maskPattern);
        cairo_restore(m_cairo);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->RestoreState();
        }
    } else if ( m_strokePathClip) {
        fillToStrokePathClip(state);
    } else {
        cairo_fill(m_cairo);
    }

    if ( m_cairoShape) {
        cairo_set_fill_rule(m_cairoShape, CAIRO_FILL_RULE_WINDING);
        doPath(m_cairoShape, state, state->getPath());
        cairo_fill(m_cairoShape);
    }
}

void OFDOutputDev::eoFill(GfxState *state){

    // Add PathObject
    PathObjectPtr pathObject = createPathObject(state);
    if ( pathObject != nullptr ){
        pathObject->Rule = ofd::PathRule::EvenOdd;

        m_currentOFDPage->AddObject(pathObject);

        if ( m_cairoRender != nullptr ){
            ObjectPtr object = std::shared_ptr<ofd::Object>(pathObject);
            m_cairoRender->DrawObject(object);
        }
    }

    doPath(m_cairo, state, state->getPath());
    cairo_set_fill_rule(m_cairo, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source(m_cairo, m_fillPattern);

    if ( m_maskPattern != nullptr) {
        cairo_save(m_cairo);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->SaveState();
        }
        cairo_clip(m_cairo);
        cairo_set_matrix(m_cairo, &m_mask_matrix);
        cairo_mask(m_cairo, m_maskPattern);
        cairo_restore(m_cairo);
        if ( m_cairoRender != nullptr ){
            m_cairoRender->RestoreState();
        }
    } else {
        cairo_fill(m_cairo);
    }
    if ( m_cairoShape != nullptr ) {
        cairo_set_fill_rule(m_cairoShape, CAIRO_FILL_RULE_EVEN_ODD);
        doPath(m_cairoShape, state, state->getPath());
        cairo_fill(m_cairoShape);
    }
}

// Defined in OFDOutputDev_utils.cc
void setContextAntialias(cairo_t *cr, cairo_antialias_t antialias);

GBool OFDOutputDev::tilingPatternFill(GfxState *state, Gfx *gfxA, Catalog *cat, ::Object *str,
					double *pmat, int paintType, int /*tilingType*/, Dict *resDict,
					double *mat, double *bbox,
					int x0, int y0, int x1, int y1,
					double xStep, double yStep) {
    PDFRectangle box;
    Gfx *gfx;
    cairo_pattern_t *pattern;
    cairo_surface_t *surface;
    cairo_matrix_t matrix;
    cairo_matrix_t pattern_matrix;
    cairo_t *old_cairo;
    double xMin, yMin, xMax, yMax;
    double width, height;
    double scaleX, scaleY;
    int surface_width, surface_height;
    StrokePathClip *strokePathTmp;
    GBool adjustedStrokeWidth_tmp;
    cairo_pattern_t *maskTmp;

    width = bbox[2] - bbox[0];
    height = bbox[3] - bbox[1];

    if (xStep != width || yStep != height)
        return gFalse;
    /* TODO: implement the other cases here too */

    // Find the width and height of the transformed pattern
    cairo_get_matrix(m_cairo, &matrix);
    cairo_matrix_init(&pattern_matrix, mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
    cairo_matrix_multiply(&matrix, &matrix, &pattern_matrix);

    double widthX = width, widthY = 0;
    cairo_matrix_transform_distance (&matrix, &widthX, &widthY);
    surface_width = ceil (sqrt (widthX * widthX + widthY * widthY));

    double heightX = 0, heightY = height;
    cairo_matrix_transform_distance (&matrix, &heightX, &heightY);
    surface_height = ceil (sqrt (heightX * heightX + heightY * heightY));
    scaleX = surface_width / width;
    scaleY = surface_height / height;

    surface = cairo_surface_create_similar(cairo_get_target(m_cairo),
            CAIRO_CONTENT_COLOR_ALPHA,
            surface_width, surface_height);
    if (cairo_surface_status(surface))
        return gFalse;

    old_cairo = m_cairo;
    m_cairo = cairo_create(surface);
    cairo_surface_destroy(surface);
    setContextAntialias(m_cairo, m_antialias);

    box.x1 = bbox[0]; box.y1 = bbox[1];
    box.x2 = bbox[2]; box.y2 = bbox[3];
    cairo_scale(m_cairo, scaleX, scaleY);
    cairo_translate(m_cairo, -box.x1, -box.y1);

    strokePathTmp = m_strokePathClip;
    m_strokePathClip = nullptr;
    adjustedStrokeWidth_tmp = m_adjustedStrokeWidth;
    maskTmp = m_maskPattern;
    m_maskPattern = NULL;
    gfx = new Gfx(m_pdfDoc.get(), this, resDict, &box, NULL, NULL, NULL, gfxA->getXRef());
    if (paintType == 2)
        m_inUncoloredPattern = gTrue;
    gfx->display(str);
    if (paintType == 2)
        m_inUncoloredPattern = gFalse;
    delete gfx;
    m_strokePathClip = strokePathTmp;
    m_adjustedStrokeWidth = adjustedStrokeWidth_tmp;
    m_maskPattern = maskTmp;

    pattern = cairo_pattern_create_for_surface(cairo_get_target(m_cairo));
    cairo_destroy(m_cairo);
    m_cairo = old_cairo;
    if (cairo_pattern_status(pattern))
        return false;

    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    cairo_rectangle(m_cairo, xMin, yMin, xMax - xMin, yMax - yMin);

    cairo_matrix_init_scale(&matrix, scaleX, scaleY);
    cairo_matrix_translate(&matrix, -box.x1, -box.y1);
    cairo_pattern_set_matrix(pattern, &matrix);

    cairo_transform(m_cairo, &pattern_matrix);
    cairo_set_source(m_cairo, pattern);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
    if ( m_strokePathClip != nullptr ){
        fillToStrokePathClip(state);
    } else {
        cairo_fill(m_cairo);
    }

    cairo_pattern_destroy(pattern);

    return true;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
GBool OFDOutputDev::functionShadedFill(GfxState *state, GfxFunctionShading *shading) {
    // Function shaded fills are subdivided to rectangles that are the
    // following size in device space.  Note when printing this size is
    // in points.
    const int subdivide_pixels = 10;

    double x_begin, x_end, x1, x2;
    double y_begin, y_end, y1, y2;
    double x_step;
    double y_step;
    GfxColor color;
    GfxRGB rgb;
    double *matrix;
    cairo_matrix_t mat;

    matrix = shading->getMatrix();
    mat.xx = matrix[0];
    mat.yx = matrix[1];
    mat.xy = matrix[2];
    mat.yy = matrix[3];
    mat.x0 = matrix[4];
    mat.y0 = matrix[5];
    if (cairo_matrix_invert(&mat)) {
        error(errSyntaxWarning, -1, "matrix not invertible\n");
        return gFalse;
    }

    // get cell size in pattern space
    x_step = y_step = subdivide_pixels;
    cairo_matrix_transform_distance (&mat, &x_step, &y_step);

    cairo_pattern_destroy(m_fillPattern);
    m_fillPattern = cairo_pattern_create_mesh();
    cairo_pattern_set_matrix(m_fillPattern, &mat);
    shading->getDomain(&x_begin, &y_begin, &x_end, &y_end);

    for (x1 = x_begin; x1 < x_end; x1 += x_step) {
        x2 = x1 + x_step;
        if (x2 > x_end)
            x2 = x_end;

        for (y1 = y_begin; y1 < y_end; y1 += y_step) {
            y2 = y1 + y_step;
            if (y2 > y_end)
                y2 = y_end;

            cairo_mesh_pattern_begin_patch(m_fillPattern);
            cairo_mesh_pattern_move_to (m_fillPattern, x1, y1);
            cairo_mesh_pattern_line_to (m_fillPattern, x2, y1);
            cairo_mesh_pattern_line_to (m_fillPattern, x2, y2);
            cairo_mesh_pattern_line_to (m_fillPattern, x1, y2);

            shading->getColor(x1, y1, &color);
            shading->getColorSpace()->getRGB(&color, &rgb);
            cairo_mesh_pattern_set_corner_color_rgb(m_fillPattern, 0,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));

            shading->getColor(x2, y1, &color);
            shading->getColorSpace()->getRGB(&color, &rgb);
            cairo_mesh_pattern_set_corner_color_rgb(m_fillPattern, 1,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));

            shading->getColor(x2, y2, &color);
            shading->getColorSpace()->getRGB(&color, &rgb);
            cairo_mesh_pattern_set_corner_color_rgb(m_fillPattern, 2,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));

            shading->getColor(x1, y2, &color);
            shading->getColorSpace()->getRGB(&color, &rgb);
            cairo_mesh_pattern_set_corner_color_rgb(m_fillPattern, 3,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));

            cairo_mesh_pattern_end_patch(m_fillPattern);
        }
    }

    double xMin, yMin, xMax, yMax;
    // get the clip region bbox
    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMin, yMax);
    state->lineTo(xMax, yMax);
    state->lineTo(xMax, yMin);
    state->closePath();
    fill(state);
    state->clearPath();

    return gTrue;
}

void OFDOutputDev::clip(GfxState *state) {
    doPath(m_cairo, state, state->getPath());
    cairo_set_fill_rule(m_cairo, CAIRO_FILL_RULE_WINDING);
    cairo_clip(m_cairo);
    if ( m_cairoShape != nullptr ) {
        doPath(m_cairoShape, state, state->getPath());
        cairo_set_fill_rule(m_cairoShape, CAIRO_FILL_RULE_WINDING);
        cairo_clip(m_cairoShape);
    }

    if ( m_cairoRender != nullptr ){
        PathPtr clipPath = GfxPath_to_OfdPath(state->getPath());
        m_cairoRender->Clip(clipPath);
    }
}

void OFDOutputDev::eoClip(GfxState *state) {
    doPath(m_cairo, state, state->getPath());
    cairo_set_fill_rule(m_cairo, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip(m_cairo);
    if ( m_cairoShape != nullptr ) {
        doPath(m_cairoShape, state, state->getPath());
        cairo_set_fill_rule(m_cairoShape, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_clip(m_cairoShape);
    }
}

void OFDOutputDev::clipToStrokePath(GfxState *state){
    m_strokePathClip = (StrokePathClip*)gmalloc (sizeof(*m_strokePathClip));
    m_strokePathClip->path = state->getPath()->copy();
    cairo_get_matrix (m_cairo, &m_strokePathClip->ctm);
    m_strokePathClip->line_width = cairo_get_line_width(m_cairo);
    m_strokePathClip->dash_count = cairo_get_dash_count(m_cairo);
    if (m_strokePathClip->dash_count) {
        m_strokePathClip->dashes = (double*) gmallocn(sizeof(double), m_strokePathClip->dash_count);
        cairo_get_dash(m_cairo, m_strokePathClip->dashes, &m_strokePathClip->dash_offset);
    } else {
        m_strokePathClip->dashes = NULL;
    }
    m_strokePathClip->cap = cairo_get_line_cap(m_cairo);
    m_strokePathClip->join = cairo_get_line_join(m_cairo);
    m_strokePathClip->miter = cairo_get_miter_limit(m_cairo);
    m_strokePathClip->ref_count = 1;
}

#endif /* CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0) */
void OFDOutputDev::fillToStrokePathClip(GfxState *state) {
    cairo_save(m_cairo);
    if ( m_cairoRender != nullptr ){
        m_cairoRender->SaveState();
    }

    cairo_set_matrix(m_cairo, &m_strokePathClip->ctm);
    cairo_set_line_width(m_cairo, m_strokePathClip->line_width);
    cairo_set_dash(m_cairo, m_strokePathClip->dashes, m_strokePathClip->dash_count, m_strokePathClip->dash_offset);
    cairo_set_line_cap(m_cairo, m_strokePathClip->cap);
    cairo_set_line_join(m_cairo, m_strokePathClip->join);
    cairo_set_miter_limit(m_cairo, m_strokePathClip->miter);
    doPath(m_cairo, state, m_strokePathClip->path);
    cairo_stroke(m_cairo);

    cairo_restore(m_cairo);
    if ( m_cairoRender != nullptr ){
        m_cairoRender->RestoreState();
    }
}

GBool OFDOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax) {
    double x0, y0, x1, y1;
    double dx, dy;

    shading->getCoords(&x0, &y0, &x1, &y1);
    dx = x1 - x0;
    dy = y1 - y0;

    cairo_pattern_destroy(m_fillPattern);
    m_fillPattern = cairo_pattern_create_linear (x0 + tMin * dx, y0 + tMin * dy,
            x0 + tMax * dx, y0 + tMax * dy);
    if (!shading->getExtend0() && !shading->getExtend1())
        cairo_pattern_set_extend(m_fillPattern, CAIRO_EXTEND_NONE);
    else
        cairo_pattern_set_extend (m_fillPattern, CAIRO_EXTEND_PAD);


    // -------- Create ofd::AxialShading
    ofd::AxialShading *axialShading = new ofd::AxialShading();
    axialShading->StartPoint = ofd::Point_t(x0, y0);
    axialShading->EndPoint = ofd::Point_t(x1, y1);

    if ( shading->getExtend0() ){
        if (shading->getExtend1() ){
            axialShading->Extend = 3;
        } else {
            axialShading->Extend = 2;
        }
    } else {
        if (shading->getExtend1() ){
            axialShading->Extend = 1;
        } else {
            axialShading->Extend = 0;
        }
    }

    //axialShading->ColorSegments.push_back();


    m_shading = std::shared_ptr<ofd::Shading>(axialShading);


    // TODO: use the actual stops in the shading in the case
    // of linear interpolation (Type 2 Exponential functions with N=1)
    return gFalse;
}

GBool OFDOutputDev::axialShadedSupportExtend(GfxState *state, GfxAxialShading *shading) {
  return (shading->getExtend0() == shading->getExtend1());
}

GBool OFDOutputDev::radialShadedFill(GfxState *state, GfxRadialShading *shading, double sMin, double sMax) {
    double x0, y0, r0, x1, y1, r1;
    double dx, dy, dr;
    cairo_matrix_t matrix;
    double scale;

    shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
    dx = x1 - x0;
    dy = y1 - y0;
    dr = r1 - r0;

    // Cairo/pixman do not work well with a very large or small scaled
    // matrix.  See cairo bug #81657.
    //
    // As a workaround, scale the pattern by the average of the vertical
    // and horizontal scaling of the current transformation matrix.
    cairo_get_matrix(m_cairo, &matrix);
    scale = (sqrt(matrix.xx * matrix.xx + matrix.yx * matrix.yx)
            + sqrt(matrix.xy * matrix.xy + matrix.yy * matrix.yy)) / 2;
    cairo_matrix_init_scale(&matrix, scale, scale);

    cairo_pattern_destroy(m_fillPattern);
    m_fillPattern = cairo_pattern_create_radial ((x0 + sMin * dx) * scale,
            (y0 + sMin * dy) * scale,
            (r0 + sMin * dr) * scale,
            (x0 + sMax * dx) * scale,
            (y0 + sMax * dy) * scale,
            (r0 + sMax * dr) * scale);
    cairo_pattern_set_matrix(m_fillPattern, &matrix);
    if (shading->getExtend0() && shading->getExtend1())
        cairo_pattern_set_extend(m_fillPattern, CAIRO_EXTEND_PAD);
    else
        cairo_pattern_set_extend (m_fillPattern, CAIRO_EXTEND_NONE);


    // -------- Create ofd::RadialShading
    ofd::RadialShading *radialShading = new ofd::RadialShading();
    radialShading->StartPoint = ofd::Point_t(x0, y0);
    radialShading->EndPoint = ofd::Point_t(x1, y1);
    radialShading->StartRadius = r0;
    radialShading->EndRadius = r1;

    if ( shading->getExtend0() ){
        if (shading->getExtend1() ){
            radialShading->Extend = 3;
        } else {
            radialShading->Extend = 2;
        }
    } else {
        if (shading->getExtend1() ){
            radialShading->Extend = 1;
        } else {
            radialShading->Extend = 0;
        }
    }


    //radialShading->ColorSegments.push_back();

    m_shading = std::shared_ptr<ofd::Shading>(radialShading);

    return gFalse;
}

GBool OFDOutputDev::radialShadedSupportExtend(GfxState *state, GfxRadialShading *shading) {
  return (shading->getExtend0() == shading->getExtend1());
}


GBool OFDOutputDev::gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading) {
    double x0, y0, x1, y1, x2, y2;
    GfxColor color[3];
    int i, j;
    GfxRGB rgb;

    cairo_pattern_destroy(m_fillPattern);
    m_fillPattern = cairo_pattern_create_mesh();

    for (i = 0; i < shading->getNTriangles(); i++) {
        if (shading->isParameterized()) {
            double color0, color1, color2;
            shading->getTriangle(i, &x0, &y0, &color0,
                    &x1, &y1, &color1,
                    &x2, &y2, &color2);
            shading->getParameterizedColor(color0, &color[0]);
            shading->getParameterizedColor(color1, &color[1]);
            shading->getParameterizedColor(color2, &color[2]);
        } else {
            shading->getTriangle(i,
                    &x0, &y0, &color[0],
                    &x1, &y1, &color[1],
                    &x2, &y2, &color[2]);

        }

        cairo_mesh_pattern_begin_patch (m_fillPattern);

        cairo_mesh_pattern_move_to(m_fillPattern, x0, y0);
        cairo_mesh_pattern_line_to(m_fillPattern, x1, y1);
        cairo_mesh_pattern_line_to(m_fillPattern, x2, y2);

        for (j = 0; j < 3; j++) {
            shading->getColorSpace()->getRGB(&color[j], &rgb);
            cairo_mesh_pattern_set_corner_color_rgb (m_fillPattern, j,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));
        }

        cairo_mesh_pattern_end_patch(m_fillPattern);
    }

    double xMin, yMin, xMax, yMax;
    // get the clip region bbox
    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMin, yMax);
    state->lineTo(xMax, yMax);
    state->lineTo(xMax, yMin);
    state->closePath();
    fill(state);
    state->clearPath();

    return gTrue;
}

GBool OFDOutputDev::patchMeshShadedFill(GfxState *state, GfxPatchMeshShading *shading) {
    int i, j, k;

    cairo_pattern_destroy(m_fillPattern);
    m_fillPattern = cairo_pattern_create_mesh();

    for (i = 0; i < shading->getNPatches(); i++) {
        GfxPatch *patch = shading->getPatch(i);
        GfxColor color;
        GfxRGB rgb;

        cairo_mesh_pattern_begin_patch(m_fillPattern);

        cairo_mesh_pattern_move_to(m_fillPattern, patch->x[0][0], patch->y[0][0]);
        cairo_mesh_pattern_curve_to(m_fillPattern,
                patch->x[0][1], patch->y[0][1],
                patch->x[0][2], patch->y[0][2],
                patch->x[0][3], patch->y[0][3]);

        cairo_mesh_pattern_curve_to(m_fillPattern,
                patch->x[1][3], patch->y[1][3],
                patch->x[2][3], patch->y[2][3],
                patch->x[3][3], patch->y[3][3]);

        cairo_mesh_pattern_curve_to(m_fillPattern,
                patch->x[3][2], patch->y[3][2],
                patch->x[3][1], patch->y[3][1],
                patch->x[3][0], patch->y[3][0]);

        cairo_mesh_pattern_curve_to(m_fillPattern,
                patch->x[2][0], patch->y[2][0],
                patch->x[1][0], patch->y[1][0],
                patch->x[0][0], patch->y[0][0]);

        cairo_mesh_pattern_set_control_point(m_fillPattern, 0, patch->x[1][1], patch->y[1][1]);
        cairo_mesh_pattern_set_control_point(m_fillPattern, 1, patch->x[1][2], patch->y[1][2]);
        cairo_mesh_pattern_set_control_point(m_fillPattern, 2, patch->x[2][2], patch->y[2][2]);
        cairo_mesh_pattern_set_control_point(m_fillPattern, 3, patch->x[2][1], patch->y[2][1]);

        for (j = 0; j < 4; j++) {
            int u, v;

            switch (j) {
                case 0:
                    u = 0; v = 0;
                    break;
                case 1:
                    u = 0; v = 1;
                    break;
                case 2:
                    u = 1; v = 1;
                    break;
                case 3:
                    u = 1; v = 0;
                    break;
            }

            if (shading->isParameterized()) {
                shading->getParameterizedColor(patch->color[u][v].c[0], &color);
            } else {
                for (k = 0; k < shading->getColorSpace()->getNComps(); k++) {
                    // simply cast to the desired type; that's all what is needed.
                    color.c[k] = GfxColorComp(patch->color[u][v].c[k]);
                }
            }

            shading->getColorSpace()->getRGB(&color, &rgb);
            cairo_mesh_pattern_set_corner_color_rgb(m_fillPattern, j,
                    colToDbl(rgb.r),
                    colToDbl(rgb.g),
                    colToDbl(rgb.b));
        }
        cairo_mesh_pattern_end_patch(m_fillPattern);
    }

    double xMin, yMin, xMax, yMax;
    // get the clip region bbox
    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMin, yMax);
    state->lineTo(xMax, yMax);
    state->lineTo(xMax, yMin);
    state->closePath();
    fill(state);
    state->clearPath();

    return gTrue;
}


#include "OFDOutputDev.h"
#include "utils/logger.h"

// Defined in OFDOutputDev_utils.cc
void setContextAntialias(cairo_t *cr, cairo_antialias_t antialias);

static cairo_surface_t *cairo_surface_create_similar_clip(cairo_t *cairo, cairo_content_t content) {
    cairo_pattern_t *pattern;
    cairo_surface_t *surface = NULL;

    cairo_push_group_with_content (cairo, content);
    pattern = cairo_pop_group (cairo);
    cairo_pattern_get_surface (pattern, &surface);
    cairo_surface_reference (surface);
    cairo_pattern_destroy (pattern);
    return surface;
}

static int luminocity(uint32_t x) {
    int r = (x >> 16) & 0xff;
    int g = (x >>  8) & 0xff;
    int b = (x >>  0) & 0xff;
    // an arbitrary integer approximation of .3*r + .59*g + .11*b
    int y = (r*19661+g*38666+b*7209 + 32829)>>16;
    return y;
}


void OFDOutputDev::beginTransparencyGroup(GfxState * /*state*/, double * /*bbox*/,
                                      GfxColorSpace * blendingColorSpace,
                                      GBool /*isolated*/, GBool knockout,
				      GBool forSoftMask) {
    /* push color space */
    ColorSpaceStack* css = new ColorSpaceStack;
    css->cs = blendingColorSpace;
    css->knockout = knockout;
    cairo_get_matrix(m_cairo, &css->group_matrix);
    css->next = m_groupColorSpaceStack;
    m_groupColorSpaceStack = css;

    LOG(DEBUG) << "begin transparency group. knockout: " << (knockout ? "yes":"no");

    if (knockout) {
        m_knockoutCount++;
        if (!m_cairoShape) {
            /* create a surface for tracking the shape */
            cairo_surface_t *cairo_shape_surface = cairo_surface_create_similar_clip(m_cairo, CAIRO_CONTENT_ALPHA);
            m_cairoShape = cairo_create(cairo_shape_surface);
            cairo_surface_destroy (cairo_shape_surface);
            setContextAntialias(m_cairoShape, m_antialias);

            /* the color doesn't matter as long as it is opaque */
            cairo_set_source_rgb(m_cairoShape, 0, 0, 0);
            cairo_matrix_t matrix;
            cairo_get_matrix(m_cairo, &matrix);
            //printMatrix(&matrix);
            cairo_set_matrix(m_cairoShape, &matrix);
        }
    }
    if (m_groupColorSpaceStack->next && m_groupColorSpaceStack->next->knockout) {
        /* we need to track the shape */
        cairo_push_group(m_cairoShape);
    }
    if (0 && forSoftMask)
        cairo_push_group_with_content(m_cairo, CAIRO_CONTENT_ALPHA);
    else
        cairo_push_group(m_cairo);

    /* push_group has an implicit cairo_save() */
    if (knockout) {
        /*XXX: let's hope this matches the semantics needed */
        cairo_set_operator(m_cairo, CAIRO_OPERATOR_SOURCE);
    } else {
        cairo_set_operator(m_cairo, CAIRO_OPERATOR_OVER);
    }
}

void OFDOutputDev::endTransparencyGroup(GfxState * /*state*/) {
    if ( m_groupPattern != nullptr ){
        cairo_pattern_destroy(m_groupPattern);
    }
    m_groupPattern = cairo_pop_group(m_cairo);

    LOG(DEBUG) << "end transparency group";

    if ( m_groupColorSpaceStack->next && m_groupColorSpaceStack->next->knockout) {
        if ( m_shapePattern != nullptr ){
            cairo_pattern_destroy(m_shapePattern);
        }
        m_shapePattern = cairo_pop_group(m_cairoShape);
    }
}

void OFDOutputDev::popTransparencyGroup() {
    /* pop color space */
    ColorSpaceStack *css = m_groupColorSpaceStack;
    if ( css->knockout ) {
        m_knockoutCount--;
        if (!m_knockoutCount) {
            /* we don't need to track the shape anymore because
             * we are not above any knockout groups */
            cairo_destroy(m_cairoShape);
            m_cairoShape = NULL;
        }
    }
    m_groupColorSpaceStack = css->next;
    delete css;
}

void OFDOutputDev::paintTransparencyGroup(GfxState * /*state*/, double * /*bbox*/) {
    LOG(DEBUG) << "paint transparency group";

    cairo_save(m_cairo);
    cairo_set_matrix(m_cairo, &m_groupColorSpaceStack->group_matrix);

    if ( m_shapePattern) {
        /* OPERATOR_SOURCE w/ a mask is defined as (src IN mask) ADD (dest OUT mask)
         * however our source has already been clipped to mask so we only need to
         * do ADD and OUT */

        /* clear the shape mask */
        cairo_set_source(m_cairo, m_shapePattern);
        cairo_set_operator(m_cairo, CAIRO_OPERATOR_DEST_OUT);
        cairo_paint(m_cairo);
        cairo_set_operator(m_cairo, CAIRO_OPERATOR_ADD);
    }
    cairo_set_source(m_cairo, m_groupPattern);

    if (!m_maskPattern) {
        cairo_paint_with_alpha(m_cairo, m_fillOpacity);
        cairo_status_t status = cairo_status(m_cairo);
        if (status)
            LOG(ERROR) << "BAD status: " <<  cairo_status_to_string(status);
    } else {
        if ( m_fillOpacity < 1.0) {
            cairo_push_group(m_cairo);
        }
        cairo_save(m_cairo);
        cairo_set_matrix(m_cairo, &m_mask_matrix);
        cairo_mask(m_cairo, m_maskPattern);
        cairo_restore(m_cairo);
        if ( m_fillOpacity < 1.0 ) {
            cairo_pop_group_to_source(m_cairo);
            cairo_paint_with_alpha(m_cairo, m_fillOpacity);
        }
        cairo_pattern_destroy(m_maskPattern);
        m_maskPattern = nullptr;
    }

    if ( m_shapePattern != nullptr ) {
        if ( m_cairoShape) {
            cairo_set_source(m_cairoShape, m_shapePattern);
            cairo_paint(m_cairoShape);
            cairo_set_source_rgb(m_cairoShape, 0, 0, 0);
        }
        cairo_pattern_destroy(m_shapePattern);
        m_shapePattern = nullptr;
    }

    popTransparencyGroup();
    cairo_restore(m_cairo);
}

/* XXX: do we need to deal with shape here? */
void OFDOutputDev::setSoftMask(GfxState * state, double * bbox, GBool alpha,
                                 Function * transferFunc, GfxColor * backdropColor) {
    cairo_pattern_destroy(m_maskPattern);

    LOG(DEBUG) << "set softMask";

    if (!alpha || transferFunc) {
        /* We need to mask according to the luminocity of the group.
         * So we paint the group to an image surface convert it to a luminocity map
         * and then use that as the mask. */

        /* Get clip extents in device space */
        double x1, y1, x2, y2, x_min, y_min, x_max, y_max;
        cairo_clip_extents(m_cairo, &x1, &y1, &x2, &y2);
        cairo_user_to_device(m_cairo, &x1, &y1);
        cairo_user_to_device(m_cairo, &x2, &y2);
        x_min = std::min(x1, x2);
        y_min = std::min(y1, y2);
        x_max = std::max(x1, x2);
        y_max = std::max(y1, y2);
        cairo_clip_extents(m_cairo, &x1, &y1, &x2, &y2);
        cairo_user_to_device(m_cairo, &x1, &y2);
        cairo_user_to_device(m_cairo, &x2, &y1);
        x_min = std::min(x_min, std::min(x1, x2));
        y_min = std::min(y_min, std::min(y1, y2));
        x_max = std::max(x_max, std::max(x1, x2));
        y_max = std::max(y_max, std::max(y1, y2));

        int width = (int)(ceil(x_max) - floor(x_min));
        int height = (int)(ceil(y_max) - floor(y_min));

        /* Get group device offset */
        double x_offset, y_offset;
        if (cairo_get_group_target(m_cairo) == cairo_get_target(m_cairo)) {
            cairo_surface_get_device_offset(cairo_get_group_target(m_cairo), &x_offset, &y_offset);
        } else {
            cairo_surface_t *pats;
            cairo_pattern_get_surface(m_groupPattern, &pats);
            cairo_surface_get_device_offset(pats, &x_offset, &y_offset);
        }

        /* Adjust extents by group offset */
        x_min += x_offset;
        y_min += y_offset;

        cairo_surface_t *source = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cairo_t *maskCtx = cairo_create(source);
        setContextAntialias(maskCtx, m_antialias);

        //XXX: hopefully this uses the correct color space */
        if (!alpha && m_groupColorSpaceStack->cs) {
            GfxRGB backdropColorRGB;
            m_groupColorSpaceStack->cs->getRGB(backdropColor, &backdropColorRGB);
            /* paint the backdrop */
            cairo_set_source_rgb(maskCtx,
                    colToDbl(backdropColorRGB.r),
                    colToDbl(backdropColorRGB.g),
                    colToDbl(backdropColorRGB.b));
        }
        cairo_paint(maskCtx);

        /* Copy source ctm to mask ctm and translate origin so that the
         * mask appears it the same location on the source surface.  */
        cairo_matrix_t mat, tmat;
        cairo_matrix_init_translate(&tmat, -x_min, -y_min);
        cairo_get_matrix(m_cairo, &mat);
        cairo_matrix_multiply(&mat, &mat, &tmat);
        cairo_set_matrix(maskCtx, &mat);

        /* make the device offset of the new mask match that of the group */
        cairo_surface_set_device_offset(source, x_offset, y_offset);

        /* paint the group */
        cairo_set_source(maskCtx, m_groupPattern);
        cairo_paint(maskCtx);

        /* XXX status = cairo_status(maskCtx); */
        cairo_destroy(maskCtx);

        /* convert to a luminocity map */
        uint32_t *source_data = (uint32_t*)cairo_image_surface_get_data(source);
        /* get stride in units of 32 bits */
        int stride = cairo_image_surface_get_stride(source)/4;
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                int lum = alpha ? m_fillOpacity : luminocity(source_data[y*stride + x]);
                if (transferFunc) {
                    double lum_in, lum_out;
                    lum_in = lum/256.0;
                    transferFunc->transform(&lum_in, &lum_out);
                    lum = (int)(lum_out * 255.0 + 0.5);
                }
                source_data[y*stride + x] = lum << 24;
            }
        }
        cairo_surface_mark_dirty (source);

        /* setup the new mask pattern */
        m_maskPattern = cairo_pattern_create_for_surface(source);
        cairo_get_matrix(m_cairo, &m_mask_matrix);

        if (cairo_get_group_target(m_cairo) == cairo_get_target(m_cairo)) {
            cairo_pattern_set_matrix(m_maskPattern, &mat);
        } else {
            cairo_matrix_t patMatrix;
            cairo_pattern_get_matrix(m_groupPattern, &patMatrix);
            /* Apply x_min, y_min offset to it appears in the same location as source. */
            cairo_matrix_multiply(&patMatrix, &patMatrix, &tmat);
            cairo_pattern_set_matrix(m_maskPattern, &patMatrix);
        }

        cairo_surface_destroy(source);
    } else if (alpha) {
        m_maskPattern = cairo_pattern_reference(m_groupPattern);
        cairo_get_matrix(m_cairo, &m_mask_matrix);
    }

    popTransparencyGroup();
}

void OFDOutputDev::clearSoftMask(GfxState * /*state*/) {
  if ( m_maskPattern != nullptr ){
    cairo_pattern_destroy(m_maskPattern);
  }
  m_maskPattern = NULL;
}


#include "OFDOutputDev.h"
#include "utils/logger.h"

/***
 *
 * OFDOutputDev::getCropSize()
 * OFDOutputDev::getOutputSize()
 * OFDOutputDev::getFitToPageTransform()
 *
 * setContextAntialias()
 * OFDOutputDev::SetAntialias()
 * OFDOutputDev::SetCairo()
 * OFDOutputDev::SetTextPage()
 * OFDOutputDev::TakeTextPage()
 *
 * **/

// FIXME
extern bool svg;
extern GBool rawOrder;

// -------- OFDOutputDev::getCropSize() --------
void OFDOutputDev::getCropSize(double page_w, double page_h, double *width, double *height){
  int w = m_cropW;
  int h = m_cropH;

  if (w == 0)
    w = (int)ceil(page_w);

  if (h == 0)
    h = (int)ceil(page_h);

  *width =  (m_cropX + w > page_w ? (int)ceil(page_w - m_cropX) : w);
  *height = (m_cropY + h > page_h ? (int)ceil(page_h - m_cropY) : h);
}

// -------- OFDOutputDev::getOutputSize() --------
void OFDOutputDev::getOutputSize(double page_w, double page_h, double *width, double *height){
    if ( m_printing ) {
        if (m_usePDFPageSize) {
            *width = page_w;
            *height = page_h;
        } else {
            if (page_w > page_h) {
                *width = m_paperHeight;
                *height = m_paperWidth;
            } else {
                *width = m_paperWidth;
                *height = m_paperHeight;
            }
        }
    } else {
        getCropSize(page_w * (m_resolutionX / 72.0), page_h * (m_resolutionY / 72.0), width, height);
    }
}

// -------- OFDOutputDev::getFitToPageTransform() --------
void OFDOutputDev::getFitToPageTransform(double page_w, double page_h, double paper_w, double paper_h, cairo_matrix_t *m) {
  double x_scale, y_scale, scale;

  x_scale = paper_w / page_w;
  y_scale = paper_h / page_h;
  if (x_scale < y_scale)
    scale = x_scale;
  else
    scale = y_scale;

  if (scale > 1.0 && !m_expand)
    scale = 1.0;
  if (scale < 1.0 && m_noShrink)
    scale = 1.0;

  cairo_matrix_init_identity (m);
  if (!m_noCenter) {
    // centre page
    cairo_matrix_translate(m, (paper_w - page_w*scale)/2, (paper_h - page_h*scale)/2);
  } else if (!svg){
    // move to PostScript origin
    cairo_matrix_translate (m, 0, (paper_h - page_h*scale));
  }
  cairo_matrix_scale (m, scale, scale);
}

// -------- setContextAntialias() --------
void setContextAntialias(cairo_t *cr, cairo_antialias_t antialias) {
    cairo_set_antialias(cr, antialias);

    cairo_font_options_t *font_options = cairo_font_options_create();

    cairo_get_font_options(cr, font_options);
    cairo_font_options_set_antialias(font_options, antialias);
    cairo_set_font_options(cr, font_options);

    cairo_font_options_destroy(font_options);
}

// ======== OFDOutputDev::SetCairo() ========
void OFDOutputDev::SetCairo(cairo_t *cairo) {
    if ( m_cairo != nullptr) {
        cairo_status_t status = cairo_status(m_cairo);
        if (status) {
            LOG(ERROR) << "cairo context error: {0:s}\n" << cairo_status_to_string(status);
        }
        cairo_destroy (m_cairo);
        assert(!m_cairoShape);
    }
    if (cairo != nullptr) {
        m_cairo = cairo_reference(cairo);
        /* save the initial matrix so that we can use it for type3 fonts. */
        //XXX: is this sufficient? could we miss changes to the matrix somehow?
        cairo_get_matrix(cairo, &m_origMatrix);
        setContextAntialias(cairo, m_antialias);
    } else {
        m_cairo = nullptr;
        m_cairoShape = nullptr;
    }
}

// ======== OFDOutputDev::SetAntialias() ========
void OFDOutputDev::SetAntialias(cairo_antialias_t antialias) {
  m_antialias = antialias;
  if ( m_cairo != nullptr ){
      setContextAntialias(m_cairo, antialias);
  }
  if ( m_cairoShape != nullptr ){
      setContextAntialias(m_cairoShape, antialias);
  }
}

// ======== OFDOutputDev::SetTextPage() ========
void OFDOutputDev::SetTextPage(TextPage *textPage) {
    if ( m_textPage != nullptr ){ 
        m_textPage->decRefCnt();
    }
    if ( m_actualText != nullptr ){
        delete m_actualText;
    }
    if ( textPage != nullptr ){
        m_textPage = textPage;
        m_textPage->incRefCnt();
        m_actualText = new ActualText(m_textPage);
    } else {
        m_textPage = nullptr;
        m_actualText = nullptr;
    }
}

// ======== OFDOutputDev::TakeTextPage() ========
TextPage *OFDOutputDev::TakeTextPage(){
    TextPage *textPage;

    textPage = m_textPage;
    if ( textPage != nullptr ){
        delete m_actualText;
    }

    m_textPage = new TextPage(rawOrder);
    m_actualText = new ActualText(m_textPage);

    return textPage;
}

#include <cairo/cairo.h>
#include <limits.h>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/ImageObject.h"
#include "ofd/Color.h"
#include "ofd/Image.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class ofd::ImageBorder ****************
ImageBorder::ImageBorder() : 
    LineWidth(0.353), HorizonalCornerRadius(0.0), VerticalCornerRadius(0.0), DashOffset(0.0),
    BorderColor(COLOR_BLACK)
{
}

// **************** class ofd::ImageObject ****************

ImageObject::ImageObject(LayerPtr layer) :
    Object(layer, ObjectType::IMAGE, "ImageObject"),
    ResourceID(0), Substitution(0), ImageMask(0),
    m_image(nullptr){
    Type = ofd::ObjectType::IMAGE;
}

ImageObject::~ImageObject(){
}

void ImageObject::SetImage(ImagePtr image){
    ResourceID = image->ID;
    m_image = image;
}

void ImageObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);

    // -------- <ImageObject ResourceID="">
    // Required.
    if ( ResourceID > 0 ){
        writer.WriteAttribute("ResourceID", ResourceID);
    }

    // -------- <ImageObject Substitution="">
    // Optional.
    if ( Substitution > 0 ){
        writer.WriteAttribute("Substitution", Substitution);
    }

    // -------- <ImageObject ImageMask="">
    // Optional.
    if ( ImageMask > 0 ){
        writer.WriteAttribute("ImageMask", ImageMask);
    }

}

void ImageObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);

    // -------- <Border>
    // Optional.
    writer.StartElement("Border");{
    
        writer.WriteAttribute("LineWidth", Border.LineWidth);
        writer.WriteAttribute("HorizonalCornerRadius", Border.HorizonalCornerRadius);
        writer.WriteAttribute("VerticalCornerRadius", Border.VerticalCornerRadius);
        writer.WriteAttribute("DashOffset", Border.DashOffset);

        writer.StartElement("BorderColor");{
            Border.BorderColor->WriteColorXML(writer);
        }; writer.EndElement();

    }; writer.EndElement();
}

bool ImageObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){

        bool exist = false;

        // -------- <ImageObject ResourceID="">
        std::tie(ResourceID, exist) = objectElement->GetIntAttribute("ResourceID");
        if ( !exist ){
            LOG(WARNING) << "ResourceID does not exist in ImageObject";
            return false;
        }

        // -------- <ImageObject Substitution="">
        // Optional.
        std::tie(Substitution, std::ignore) = objectElement->GetIntAttribute("Substitution");

        // -------- <ImageObject ="ImageMask">
        // Optional.
        std::tie(ImageMask, std::ignore) = objectElement->GetIntAttribute("ImageMask");

        return true;
    }

    return false;
}

bool ImageObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){

        std::string childName = childElement->GetName();

        if ( childName == "Border" ){
        }

        return true;
    }
    return false;
}

typedef float SplashCoord;

static inline int splashRound(SplashCoord x) {
  return (int)floor(x + 0.5);
}

static inline int splashCeil(SplashCoord x) {
  return (int)ceil(x);
}

static inline int splashFloor(SplashCoord x) {
  return (int)floor(x);
}

/* Taken from cairo/doc/tutorial/src/singular.c */
static void get_singular_values (const cairo_matrix_t *matrix, double *major, double *minor){
    double xx = matrix->xx, xy = matrix->xy;
    double yx = matrix->yx, yy = matrix->yy;

    double a = xx*xx+yx*yx;
    double b = xy*xy+yy*yy;
    double k = xx*xy+yx*yy;

    double f = (a+b) * .5;
    double g = (a-b) * .5;
    double delta = sqrt (g*g + k*k);

    if (major)
        *major = sqrt (f + delta);
    if (minor)
        *minor = sqrt (f - delta);
}

void getImageScaledSize(const cairo_matrix_t *matrix,
        int                   orig_width,
        int                   orig_height,
        int                  *scaledWidth,
        int                  *scaledHeight) {
    double xScale;
    double yScale;
    if (orig_width > orig_height)
        get_singular_values(matrix, &xScale, &yScale);
    else
        get_singular_values (matrix, &yScale, &xScale);

    int tx, tx2, ty, ty2; /* the integer co-oridinates of the resulting image */
    if (xScale >= 0) {
        tx = splashRound(matrix->x0 - 0.01);
        tx2 = splashRound(matrix->x0 + xScale + 0.01) - 1;
    } else {
        tx = splashRound(matrix->x0 + 0.01) - 1;
        tx2 = splashRound(matrix->x0 + xScale - 0.01);
    }
    *scaledWidth = abs(tx2 - tx) + 1;
    //scaledWidth = splashRound(fabs(xScale));
    if (*scaledWidth == 0) {
        // technically, this should draw nothing, but it generally seems
        // better to draw a one-pixel-wide stripe rather than throwing it
        // away
        *scaledWidth = 1;
    }
    if (yScale >= 0) {
        ty = splashFloor(matrix->y0 + 0.01);
        ty2 = splashCeil(matrix->y0 + yScale - 0.01);
    } else {
        ty = splashCeil(matrix->y0 - 0.01);
        ty2 = splashFloor(matrix->y0 + yScale + 0.01);
    }
    *scaledHeight = abs(ty2 - ty);
    if (*scaledHeight == 0) {
        *scaledHeight = 1;
    }
}

cairo_filter_t getFilterForSurface(cairo_surface_t *image, cairo_t *cr, bool interpolate) {
    if (interpolate)
        return CAIRO_FILTER_BILINEAR;

    int orig_width = cairo_image_surface_get_width(image);
    int orig_height = cairo_image_surface_get_height(image);
    if (orig_width == 0 || orig_height == 0)
        return CAIRO_FILTER_NEAREST;

    /* When printing, don't change the interpolation. */
    //if ( m_printing )
    //return CAIRO_FILTER_NEAREST;

    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);
    int scaled_width, scaled_height;
    getImageScaledSize(&matrix, orig_width, orig_height, &scaled_width, &scaled_height);

    /* When scale factor is >= 400% we don't interpolate. See bugs #25268, #9860 */
    if (scaled_width / orig_width >= 4 || scaled_height / orig_height >= 4)
        return CAIRO_FILTER_NEAREST;

    return CAIRO_FILTER_BILINEAR;
}

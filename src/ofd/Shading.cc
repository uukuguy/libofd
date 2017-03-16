#include <math.h>
#include "ofd/Shading.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;

// Example
//double alpha=1;
//cairo_pattern_t *spat =
    ////cairo_pattern_create_linear(0, 0, 500, 360);
    //cairo_pattern_create_radial(0, 0, 80,  500, 360, 20);
//cairo_pattern_add_color_stop_rgba(spat, 0,  0, 0, 0.8, alpha);
//cairo_pattern_add_color_stop_rgba(spat, 0.25,  1, 1, 0, alpha);
//cairo_pattern_add_color_stop_rgba(spat, 0.5,  0.9, 0.0, 0.0, alpha);
//cairo_pattern_add_color_stop_rgba(spat, 0.75,  0.8, 0.12, 0.56, alpha);
//cairo_pattern_add_color_stop_rgba(spat, 1,  0, 0, 0, alpha);

//cairo_set_source (cr, spat);
//cairo_paint(cr);

// ======== RadialShading::CreateFillPattern() ========
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

    double sMin = 0;
    double sMax = 1;
    fillPattern = cairo_pattern_create_radial ((x0 + sMin * dx) * scale,
            (y0 + sMin * dy) * scale,
            (r0 + sMin * dr) * scale,
            (x0 + sMax * dx) * scale,
            (y0 + sMax * dy) * scale,
            (r0 + sMax * dr) * scale);

    for ( size_t i = 0 ; i < ColorSegments.size() ; i++ ){

        const ColorStop_t &cs = ColorSegments[i];
        const ColorPtr color = cs.Color;
        double pos = cs.Offset;

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

        Shading::WriteShadingXML(writer);

        // -------- <ofd::RadialShd StartPoint="">
        std::stringstream ss;
        ss << StartPoint.x << " " << StartPoint.y;
        writer.WriteAttribute("StartPoint", ss.str());
        ss.str("");

        // -------- <ofd::RadialShd EndPoint="">
        ss << EndPoint.x << " " << EndPoint.y;
        writer.WriteAttribute("EndPoint", ss.str());
        ss.str("");

        // -------- <ofd::RadialShd StartRadius="">
        if ( StartRadius > 0.0 ){
            ss << StartRadius;
            writer.WriteAttribute("StartRadius", ss.str());
            ss.str("");
        }

        // -------- <ofd::RadialShd EndRadius="">
        ss << EndRadius;
        writer.WriteAttribute("EndRadius", ss.str());
        ss.str("");

        for ( auto cs : ColorSegments ){
            writer.StartElement("Segment");{
                writer.WriteAttribute("Position", cs.Offset);


                writer.StartElement("Color");{
                    cs.Color->WriteColorXML(writer);                
                    //std::stringstream ssValue;
                    //ssValue << cs.Color->Value.RGB.Red << " " 
                            //<< cs.Color->Value.RGB.Green << " "
                            //<< cs.Color->Value.RGB.Blue;
                    //writer.WriteAttribute("Value", ssValue.str());

                } writer.EndElement();

            } writer.EndElement();
        }

    } writer.EndElement();
}

// ======== RadialShading::ReadShadingXML() ========
bool RadialShading::ReadShadingXML(utils::XMLElementPtr shadingElement){
    if ( !Shading::ReadShadingXML(shadingElement) ) return false;

    bool exist = false;
    std::string strStartPoint, strEndPoint;
    std::tie(strStartPoint, exist) = shadingElement->GetStringAttribute("StartPoint");
    std::tie(strEndPoint, exist) = shadingElement->GetStringAttribute("EndPoint");

    Point_t startPoint, endPoint;
    std::vector<std::string> tokens0 = utils::SplitString(strStartPoint);
    if ( tokens0.size() == 2 ){
        startPoint.x = atof(tokens0[0].c_str());
        startPoint.y = atof(tokens0[1].c_str());
    }
    std::vector<std::string> tokens1 = utils::SplitString(strEndPoint);
    if ( tokens1.size() == 2 ){
        endPoint.x = atof(tokens1[0].c_str());
        endPoint.y = atof(tokens1[1].c_str());
    }

    double startRadius = 0.0, endRadius = 0.0;
    std::tie(startRadius, exist) = shadingElement->GetFloatAttribute("StartRadius");
    std::tie(endRadius, exist) = shadingElement->GetFloatAttribute("EndRadius");


    // <Segment>
    utils::XMLElementPtr segmentElement = shadingElement->GetFirstChildElement();
    while ( segmentElement != nullptr ){

        std::string childName = segmentElement->GetName();
        if ( childName == "Segment" ){
            double offset = 0.0;
            std::tie(offset, std::ignore) = segmentElement->GetFloatAttribute("Position");

            utils::XMLElementPtr colorElement = segmentElement->GetFirstChildElement();
            if ( colorElement != nullptr && colorElement->GetName() == "Color" ){
                ColorPtr color;
                std::tie(color, std::ignore) = Color::ReadColorXML(colorElement);

                ColorSegments.push_back(ColorStopArray::value_type(color, offset));
            }
        }

        segmentElement = segmentElement->GetNextSiblingElement();
    }

    StartPoint = startPoint;
    EndPoint = endPoint;
    StartRadius = startRadius;
    EndRadius = endRadius;

    return true;
}

// ======== AxialShading::CreateFillPattern() ========
cairo_pattern_t *AxialShading::CreateFillPattern(cairo_t *cr){

    cairo_pattern_t *fillPattern = nullptr;

    double x0, y0, x1, y1;
    x0 = StartPoint.x;
    y0 = StartPoint.y;
    x1 = EndPoint.x;
    y1 = EndPoint.y;

    double dx, dy;
    dx = x1 - x0;
    dy = y1 - y0;

    double tMin = 0;
    double tMax = 1;
    fillPattern = cairo_pattern_create_linear (x0 + tMin * dx, y0 + tMin * dy,
            x0 + tMax * dx, y0 + tMax * dy);

    if ( Extend ){
        cairo_pattern_set_extend(fillPattern, CAIRO_EXTEND_PAD);
    } else {
        cairo_pattern_set_extend (fillPattern, CAIRO_EXTEND_NONE);
    }

    return fillPattern;
}


// ======== AxialShading::WriteShadingXML() ========
void AxialShading::WriteShadingXML(utils::XMLWriter &writer) const{

    writer.StartElement("AxialShd");{

        Shading::WriteShadingXML(writer);

        // -------- <ofd::AxialShd StartPoint="">
        std::stringstream ss;
        ss << StartPoint.x << " " << StartPoint.y;
        writer.WriteAttribute("StartPoint", ss.str());
        ss.str("");

        // -------- <ofd::AxialShd EndPoint="">
        ss << EndPoint.x << " " << EndPoint.y;
        writer.WriteAttribute("EndPoint", ss.str());
        ss.str("");

        for ( auto cs : ColorSegments ){
            writer.StartElement("Segment");{
                writer.WriteAttribute("Position", cs.Offset);

                writer.StartElement("Color");{
                    cs.Color->WriteColorXML(writer);                
                    //std::stringstream ssValue;
                    //ssValue << cs.Color->Value.RGB.Red << " " 
                            //<< cs.Color->Value.RGB.Green << " "
                            //<< cs.Color->Value.RGB.Blue;
                    //writer.WriteAttribute("Value", ssValue.str());

                } writer.EndElement();

            } writer.EndElement();
        }

    } writer.EndElement();

}

// ======== AxialShading::ReadShadingXML() ========
bool AxialShading::ReadShadingXML(utils::XMLElementPtr shadingElement){
    if ( !Shading::ReadShadingXML(shadingElement) ) return false;

    bool exist = false;
    std::string strStartPoint, strEndPoint;
    std::tie(strStartPoint, exist) = shadingElement->GetStringAttribute("StartPoint");
    std::tie(strEndPoint, exist) = shadingElement->GetStringAttribute("EndPoint");

    Point_t startPoint, endPoint;
    std::vector<std::string> tokens0 = utils::SplitString(strStartPoint);
    if ( tokens0.size() == 2 ){
        startPoint.x = atof(tokens0[0].c_str());
        startPoint.y = atof(tokens0[1].c_str());
    }
    std::vector<std::string> tokens1 = utils::SplitString(strEndPoint);
    if ( tokens1.size() == 2 ){
        endPoint.x = atof(tokens1[0].c_str());
        endPoint.y = atof(tokens1[1].c_str());
    }

    // <Segment>
    utils::XMLElementPtr segmentElement = shadingElement->GetFirstChildElement();
    while ( segmentElement != nullptr ){

        std::string childName = segmentElement->GetName();
        if ( childName == "Segment" ){
            double offset = 0.0;
            std::tie(offset, std::ignore) = segmentElement->GetFloatAttribute("Position");

            utils::XMLElementPtr colorElement = segmentElement->GetFirstChildElement();
            if ( colorElement != nullptr && colorElement->GetName() == "Color" ){
                ColorPtr color;
                std::tie(color, std::ignore) = Color::ReadColorXML(colorElement);

                ColorSegments.push_back(ColorStopArray::value_type(color, offset));
            }
        }

        segmentElement = segmentElement->GetNextSiblingElement();
    }

    StartPoint = startPoint;
    EndPoint = endPoint;

    return true;
}


// ======== Shading::WriteShadingXML() ========
void Shading::WriteShadingXML(utils::XMLWriter &writer) const{
    if ( Extend != 0 ){
        writer.WriteAttribute("Extend", (uint64_t)Extend);
    }
}


bool Shading::ReadShadingXML(utils::XMLElementPtr shadingElement){
    bool exist = false;
    std::tie(Extend, exist) = shadingElement->GetIntAttribute("Extend");

    return true;
}

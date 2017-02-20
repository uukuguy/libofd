#include "ofd/Color.h"
#include "ofd/PathObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class PathObject ****************

ColorPtr PathObject::DefaultStrokeColor = Color::Instance(0,0,0,255);
ColorPtr PathObject::DefaultFillColor = Color::Instance(0,0,0,0);

PathObject::PathObject(LayerPtr layer) :
    Object(layer, ObjectType::PATH, "PathObject"),
    Stroke(true), Fill(false), Rule(PathRule::NonZero),
    FillColor(nullptr), StrokeColor(nullptr),
    m_path(nullptr){
}

PathObject::~PathObject(){
}

void PathObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);

    // -------- <PathObject Stroke="">
    // Optional, default value: true.
    if ( !Stroke ){
        writer.WriteAttribute("Stroke", false);
    }

    // -------- <PathObject Fill="">
    // Optional, default value: false.
    if ( Fill ){
        writer.WriteAttribute("Fill", true);
    }

    // -------- <PathObject Rule="Even-Odd">
    // Optional, default value: "NonZero".
    if ( Rule != PathRule::NonZero ){
        writer.WriteAttribute("Rule", "Even-Odd");
    }
}

void PathObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);

    // -------- <FillColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if ( FillColor != nullptr ){
        writer.StartElement("FillColor");{
            FillColor->WriteXML(writer);
        } writer.EndElement();
    }
    
    // -------- <StrokeColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if ( StrokeColor != nullptr ){
        writer.StartElement("StrokeColor");{
            StrokeColor->WriteXML(writer);
        } writer.EndElement();
    }

    // -------- <AbbreviatedData>
    // OFD (section 9.1) P53. Page.xsd
    // Required.
    writer.StartElement("AbbreviatedData");{
        std::stringstream ss;
        size_t numSubpaths = m_path->GetNumSubpaths();
        //LOG(DEBUG) << "AbbreviateData: numSubpaths:" << numSubpaths;
        if ( numSubpaths > 0 ){
            for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
                SubpathPtr subpath = m_path->GetSubpath(idx);
                if ( subpath == nullptr ) continue;
                size_t numPoints = subpath->GetNumPoints();
                //LOG(DEBUG) << "AbbreviateData: numPoints:" << numPoints;
                if ( numPoints < 2 ) continue;

                const Point &startPoint = subpath->GetFirstPoint();
                if ( idx == 0 ){
                    ss << "S " << startPoint.x << " " << startPoint.y << " ";
                } else {
                    ss << "M " << startPoint.x << " " << startPoint.y << " ";
                }
                for ( size_t n = 1 ; n < numPoints ; n++ ){
                    const Point &p = subpath->GetPoint(n);
                    ss << "L " << p.x << " " << p.y << " ";
                }
                if ( subpath->IsClosed() ){
                    ss << "C ";
                }
            }
            writer.WriteString(ss.str());
        }
    } writer.EndElement();
}

bool PathObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool PathObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}


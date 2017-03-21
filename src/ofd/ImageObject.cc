#include <cairo/cairo.h>
#include <assert.h>
#include <limits.h>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Document.h"
#include "ofd/Resource.h"
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

std::string ImageObject::to_string() const{
    return Object::to_string();
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
        } else {
            const ResourcePtr documentRes = GetDocumentRes();
            assert(documentRes != nullptr);
            ImagePtr image = documentRes->GetImage(ResourceID);
            if ( image == nullptr ){
                LOG(ERROR) << "Image ID = " << ResourceID << " not found in documentRes.";
                return false;
            } else {
                m_image = image;
            }
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
            XMLElementPtr borderElement = childElement;
            std::tie(Border.LineWidth, std::ignore)  = borderElement->GetFloatAttribute("LineWidth");
            std::tie(Border.HorizonalCornerRadius, std::ignore)  = borderElement->GetFloatAttribute("HorizonalCornerRadius");
            std::tie(Border.VerticalCornerRadius, std::ignore) = borderElement->GetFloatAttribute("VerticalCornerRadius");
            std::tie(Border.DashOffset, std::ignore) = borderElement->GetFloatAttribute("DashOffset");


            XMLElementPtr borderColorElement = borderElement->GetFirstChildElement();
            if ( borderColorElement != nullptr ){
                if ( borderColorElement->GetName() == "BorderColor" ){
                    std::tie(Border.BorderColor, std::ignore) =  Color::ReadColorXML(borderColorElement);
                }
            }
        }

        return true;
    }
    return false;
}

void ImageObject::RecalculateBoundary(){
}

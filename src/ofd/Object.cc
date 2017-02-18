#include "ofd/Object.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

Object::Object(LayerPtr layer, ObjectType objectType, const std::string& objectLabel) :
    ID(0), Type(objectType),
    ObjectLabel(objectLabel),
    Visible(true), LineWidth(0.353), Alpha(255),
    m_layer(layer){
}

Object::~Object(){
}

// Called by OFDPage::ImplCls::generateContentXML()
void Object::GenerateXML(XMLWriter &writer) const{
    writer.StartElement(ObjectLabel);{
        GenerateAttributesXML(writer);
        GenerateElementsXML(writer);
    } writer.EndElement();
}

void Object::GenerateAttributesXML(XMLWriter &writer) const{

    // -------- GraphUnit attributes --------
    // OFD (section 8.5) P50. Page.xsd.

    // -------- <Object ID="">
    // Required
    writer.WriteAttribute("ID", ID);

    // -------- <Object Boundary="">
    // Required.
    writer.WriteAttribute("Boundary", Boundary.to_xmlstring());

    // -------- <Object Name="">
    // Optional
    if ( !Name.empty() ){
        writer.WriteAttribute("Name", Name);
    }

    // -------- <Object Visible="">
    // Optional, default value: true.
    if ( !Visible ){
        writer.WriteAttribute("Visible", "false");
    }

    // TODO
    // -------- <Object CTM="">


    // TODO
    // -------- <Object LineWidth="">
    if ( LineWidth > 0 ){
        writer.WriteAttribute("LineWidth", LineWidth, 3);
    }

    // -------- <Object Alpha="">
    // Optional, default value: 0.
    if ( Alpha > 0 ){
        writer.WriteAttribute("Alpha", std::to_string(Alpha));
    }
}

void Object::GenerateElementsXML(XMLWriter &writer) const{

    // -------- GraphUnit elements --------
    // OFD P50. Page.xsd.

    // TODO
    // -------- <Actions>

    // TODO
    // -------- <Clips>

}

bool Object::FromXML(XMLElementPtr objectElement){
    bool ok = true;

    ok = FromAttributesXML(objectElement);
    if ( ok ){
        FromElementsXML(objectElement);
    } else {
        LOG(WARNING) << "FromAttributesXML() return false;";
    }

    return ok;
}

// -------- CT_GraphicUnit --------
// OFD (section 8.5) P50.
bool Object::FromAttributesXML(utils::XMLElementPtr objectElement){
    bool ok = true;

    // -------- <TextObject ID="">
    // Required.
    bool exist = false;
    std::tie(ID, exist) = objectElement->GetIntAttribute("ID");
    if ( !exist ){
        LOG(ERROR) << "Attribute ID is required in Object XML."; 
        return false;
    }

    // -------- <Object Boundary="">
    // Required.
    std::string strBoundary;
    std::tie(strBoundary, exist) = objectElement->GetStringAttribute("Boundary");
    //LOG(INFO) << "Boundary: " << strBoundary;

    if ( !exist ){
        LOG(ERROR) << "Attribute ID is required in Object XML."; 
        return false;
    }
    std::vector<std::string> tokens = utils::SplitString(strBoundary);
    if ( tokens.size() >= 4 ){
        Boundary.Left = atof(tokens[0].c_str());
        Boundary.Top = atof(tokens[1].c_str());
        Boundary.Width = atof(tokens[2].c_str());
        Boundary.Height = atof(tokens[3].c_str());
        ok = true;
    } else {
        LOG(ERROR) << "Box String tokens size >= 4 failed. boxString:" << strBoundary << " element name: " << objectElement->GetName();
        return false;
    }

    // -------- <Object LineWidth="">
    // Optional
    std::tie(LineWidth, std::ignore) = objectElement->GetFloatAttribute("LineWidth");

    // -------- <Object Alpha="">
    // Optional
    uint64_t alpha;
    std::tie(alpha, std::ignore) = objectElement->GetFloatAttribute("Alpha");
    Alpha = alpha;

    // -------- <Object Name="">
    // Optional

    // -------- <Visible Name="">
    // Optional

    // -------- <CTM Name="">
    // Optional

    // -------- <DrawParam Name="">
    // Optional

    // -------- <Cap Name="">
    // Optional

    // -------- <Join Name="">
    // Optional

    // -------- <MiterLimit Name="">
    // Optional

    // -------- <DashOffset Name="">
    // Optional

    // -------- <DashPattern Name="">
    // Optional

    return ok;
}

bool Object::FromElementsXML(utils::XMLElementPtr objectElement){
    bool ok = true;

    XMLElementPtr childElement = objectElement->GetFirstChildElement();
    while ( childElement != nullptr ){

        IterateElementsXML(childElement);

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// -------- CT_GraphicUnit --------
// OFD (section 8.5) P50.
bool Object::IterateElementsXML(utils::XMLElementPtr childElement){
    bool ok = true;


    // -------- <Actions Name="">
    // Optional

    // -------- <Clips Name="">
    // Optional


    return ok;
}

#include <math.h>
#include "OFDTextObject.h"
#include "utils/logger.h"
#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDTextObject::ImplCls ****************

class OFDTextObject::ImplCls{
public:
    ImplCls();
    ~ImplCls();

    std::string to_string() const;
  
    size_t GetTextCodesCount() const;
    const Text::TextCode& GetTextCode(size_t idx) const;
    Text::TextCode& GetTextCode(size_t idx);
    void AddTextCode(const Text::TextCode &textCode);
    void ClearTextCodes();

    bool FromAttributesXML(XMLElementPtr objectElement);
    bool IterateElementsXML(XMLElementPtr objectElement);

    void GenerateAttributesXML(XMLWriter &writer) const;
    void GenerateElementsXML(XMLWriter &writer) const;

    // -------- Private Attributes --------

    OFDFontPtr          Font;     // 引用资源文件中定义的字型的标识。
    double              FontSize; // 字号，单位为毫米。
    bool                Stroke;   // 是否勾边，默认值为false。
    bool                Fill;     // 是否填充，默认值为true。
    double              HScale;   // 字型在水平方向的缩放比，默认值为1.0。
                                  // 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
    Text::ReadDirection RD;       // 阅读方向，可选值为0,90,180,270，默认值为0。
    Text::CharDirection CD;       // 字符方向，指定了文字放置的方式，
                                  // 可选值为：0, 90, 180, 270，默认值为0。
    Text::Weight        Weight;   // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 
                                  // 600, 700, 800, 900。 默认值为400。
    bool                Italic;   // 是否是斜体样式，默认值为false。

    OFDColorPtr         FillColor;   // 填充色，默认值为黑色。
    OFDColorPtr         StrokeColor; // 勾边色，默认值为透明色。

    typedef std::vector<Text::TextCode> TextCodeArray;
    TextCodeArray TextCodes;

}; // class OFDTextObject::ImplCls


OFDTextObject::ImplCls::ImplCls() :
    FontSize(12.0), Stroke(false), Fill(true), HScale(1.0), 
    RD(Text::ReadDirection::ANGLE0), CD(Text::CharDirection::ANGLE0),
    Italic(false){
}

OFDTextObject::ImplCls::~ImplCls(){
}

std::string OFDTextObject::ImplCls::to_string() const{
    return "";
}

size_t OFDTextObject::ImplCls::GetTextCodesCount() const{
    return TextCodes.size();
}

const Text::TextCode& OFDTextObject::ImplCls::GetTextCode(size_t idx) const{
    return TextCodes[idx];
}

Text::TextCode& OFDTextObject::ImplCls::GetTextCode(size_t idx){
    return TextCodes[idx];
}

void OFDTextObject::ImplCls::AddTextCode(const Text::TextCode &textCode){
    TextCodes.push_back(textCode);
}

void OFDTextObject::ImplCls::ClearTextCodes(){
    TextCodes.clear();
}

// -------- <TextObject>
// OFD (section 11.2) P63. Page.xsd.
void OFDTextObject::ImplCls::GenerateAttributesXML(XMLWriter &writer) const{

    // FIXME
    // -------- <TextObject Font="">
    // Required.
    writer.WriteAttribute("Font", "1");

    // -------- <TextObject Size="">
    // Required.
    writer.WriteAttribute("Size", FontSize, 1);

    // -------- <TextObject Stroke="">
    // Optional, default value: false.
    if ( Stroke ){
        writer.WriteAttribute("Stroke", "true");
    }

    // -------- <TextObject Fill="">
    // Optional, default value: true.
    if ( !Fill ){
        writer.WriteAttribute("Fill", "false");
    }

    // -------- <Textobject HScale="">
    // Optional, default value: 1.0
    if ( fabs(HScale - 1.0) > 0.0000001 ){
        writer.WriteAttribute("HScale", HScale);
    }

}

// -------- <TextObject>
// OFD (section 11.2) P63. Page.xsd.
void OFDTextObject::ImplCls::GenerateElementsXML(XMLWriter &writer) const{

    // -------- <TextCode>
    // OFD (section 11.3) P65. Page.xsd
    // Required.
    for ( auto textCode : TextCodes ){
        writer.StartElement("TextCode");{

            // -------- <TextCode X="'>
            // Optional.
            writer.WriteAttribute("X", textCode.X);

            // -------- <TextCode Y="'>
            // Optional.
            writer.WriteAttribute("Y", textCode.Y);

            // -------- <TextCode DeltaX="'>
            // Optional.
            std::string strDeltaX;
            for ( auto d : textCode.DeltaX ){
                strDeltaX += std::to_string(d) + " ";
            }
            writer.WriteAttribute("DeltaX", strDeltaX);

            // -------- <TextCode DeltaY="'>
            // Optional.
            std::string strDeltaY;
            for ( auto d : textCode.DeltaY ){
                strDeltaY += std::to_string(d) + " ";
            }
            writer.WriteAttribute("DeltaY", strDeltaY);

            writer.WriteString(textCode.Text);

        } writer.EndElement();
    }
}

// -------- TextObject --------
// OFD (section 11.2) P63
bool OFDTextObject::ImplCls::FromAttributesXML(XMLElementPtr objectElement){
    bool ok = true;

    // -------- <TextObject Font="">
    // Required.
    uint64_t fontID = 0;
    bool exist = false;
    std::tie(fontID, exist) = objectElement->GetIntAttribute("Font");
    if ( !exist ){
        LOG(ERROR) << "Attribute Font is required in TextObject XML."; 
        return false;
    }

    // -------- <TextObject Size="">
    // Required.
    std::tie(FontSize, exist) = objectElement->GetIntAttribute("Size");
    if ( !exist ){
        LOG(ERROR) << "Attribute Size is required in TextObject XML."; 
        return false;
    }

    return ok;
}

// -------- TextObject --------
// OFD (section 11.2) P63
bool OFDTextObject::ImplCls::IterateElementsXML(XMLElementPtr childElement){
    bool ok = false;

    std::string childName = childElement->GetName();

    if ( childName == "TextCode" ){

        Text::TextCode textCode;

        bool exist = false;
        std::tie(textCode.X, exist) = childElement->GetFloatAttribute("X");
        if ( !exist ){
            LOG(ERROR) << "Attribute X is required in TextCode XML";
            return false;
        }

        std::tie(textCode.Y, exist) = childElement->GetFloatAttribute("Y");
        if ( !exist ){
            LOG(ERROR) << "Attribute Y is required in TextCode XML";
            return false;
        }

        // TODO
        //std::string strDeltaX;
        //std::tie(strDeltaX, std::ignore) = childElement->GetStringAttribute("DeltaX");
        //std::string strDeltaY;
        //std::tie(strDeltaY, std::ignore) = childElement->GetStringAttribute("DeltaY");

        std::tie(textCode.Text, std::ignore) = childElement->GetStringValue();

        //LOG(DEBUG) << "X: " << textCode.X << " Y: " << textCode.Y << " Text: " << textCode.Text;
        TextCodes.push_back(textCode);

        ok = true;
    }

    return ok;
}

// **************** class OFDTextObject ****************

OFDTextObject::OFDTextObject() {
    Type = Object::Type::TEXT;
    ObjectLabel = "TextObject";
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());

}

OFDTextObject::~OFDTextObject(){
}

// 引用资源文件中定义的字型的标识。
const OFDFontPtr OFDTextObject::GetFont() const{
    return m_impl->Font;
}

void OFDTextObject::SetFont(const OFDFontPtr font){
    m_impl->Font = font;
}

// 字号，单位为毫米。
double OFDTextObject::GetFontSize() const{
    return m_impl->FontSize;
}

void OFDTextObject::SetFontSize(double fontSize){
    m_impl->FontSize = fontSize;
}

// 是否勾边，默认值为false。
bool OFDTextObject::IsStroke() const{
    return m_impl->Stroke;
}

void OFDTextObject::EnableStroke(bool bStroke){
    m_impl->Stroke = bStroke;
}

// 是否填充，默认值为true。
bool OFDTextObject::IsFill() const{
    return m_impl->Fill;
}

void OFDTextObject::EnableFill(bool bFill){
    m_impl->Fill = bFill;
}

// 字型在水平方向的缩放比，默认值为1.0。
// 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
bool OFDTextObject::GetHScale() const{
    return m_impl->HScale;
}

void OFDTextObject::SetHScale(double dHScale){
    m_impl->HScale = dHScale;
}

// 阅读方向，可选值为0,90,180,270，默认值为0。
Text::ReadDirection OFDTextObject::GetReadDirection() const{
    return m_impl->RD;
}

void OFDTextObject::SetReadDirection(Text::ReadDirection readDirection){
    m_impl->RD = readDirection;
}

// 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
Text::CharDirection OFDTextObject::GetCharDirection() const{
    return m_impl->CD;
}

void OFDTextObject::SetCharDirection(Text::CharDirection charDirection){
    m_impl->CD = charDirection;
}

// 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
// 默认值为400。
Text::Weight OFDTextObject::GetWeight() const{
    return m_impl->Weight;
}

void OFDTextObject::SetWeight(Text::Weight weight){
    m_impl->Weight = weight;
}

// 是否是斜体样式，默认值为false。
bool OFDTextObject::IsItalic() const{
    return m_impl->Italic;
}

void OFDTextObject::SetItalic(bool bItalic){
    m_impl->Italic = bItalic;
}

// 填充色，默认值为黑色。
OFDColorPtr OFDTextObject::GetFillColor() const{
    return m_impl->FillColor;
}

void OFDTextObject::SetFillColor(OFDColorPtr fillColor){
    m_impl->FillColor = fillColor;
}

// 勾边色，默认值为透明色。
OFDColorPtr OFDTextObject::GetStrokeColor() const{
    return m_impl->StrokeColor;
}

void OFDTextObject::SteStrokeColor(OFDColorPtr strokeColor){
    m_impl->StrokeColor = strokeColor;
}

size_t OFDTextObject::GetTextCodesCount() const{
    return m_impl->GetTextCodesCount();
}

const Text::TextCode& OFDTextObject::GetTextCode(size_t idx) const{
    return m_impl->GetTextCode(idx);
}

Text::TextCode& OFDTextObject::GetTextCode(size_t idx){
    return m_impl->GetTextCode(idx);
}

void OFDTextObject::AddTextCode(const Text::TextCode &textCode){
    m_impl->AddTextCode(textCode);
}

void OFDTextObject::ClearTextCodes(){
    m_impl->ClearTextCodes();
}

void OFDTextObject::GenerateAttributesXML(XMLWriter &writer) const{
    OFDObject::GenerateAttributesXML(writer);
    m_impl->GenerateAttributesXML(writer);
}

void OFDTextObject::GenerateElementsXML(XMLWriter &writer) const{
    OFDObject::GenerateElementsXML(writer);
    m_impl->GenerateElementsXML(writer);
}

bool OFDTextObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( OFDObject::FromAttributesXML(objectElement) ){
        return m_impl->FromAttributesXML(objectElement);
    }
    return false;
}

bool OFDTextObject::IterateElementsXML(XMLElementPtr childElement){
    if ( OFDObject::IterateElementsXML(childElement) ){
        return m_impl->IterateElementsXML(childElement);
    }
    return false;
}


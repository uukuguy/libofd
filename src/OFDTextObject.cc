#include "OFDTextObject.h"
#include "logger.h"

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

    std::vector<Text::TextCode> TextCodes;

}; // class OFDTextObject::ImplCls


OFDTextObject::ImplCls::ImplCls(){
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

// **************** class OFDTextObject ****************

OFDTextObject::OFDTextObject(){
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

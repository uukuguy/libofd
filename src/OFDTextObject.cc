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

    // -------- Private Attributes --------

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

// **************** class OFDTextObject ****************

OFDTextObject::OFDTextObject(){
}

OFDTextObject::~OFDTextObject(){
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



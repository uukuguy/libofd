#include <sstream>
#include "OFDFont.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

OFDFont::OFDFont() : 
    Charset("unicode"), Serif(false), Bold(false), Italic(false), FixedWidth(false),
    FontType(Font::Type::Unknown), FontLoc(Font::Location::Unknown),
    m_fontData(nullptr), m_fontDataSize(0){
}

OFDFont::~OFDFont(){
    if ( m_fontData != nullptr ){
        delete m_fontData;
        m_fontData = nullptr;
    }
}

std::string OFDFont::ToString() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDFont" << std::endl;

        ss << "ID: " << ID << std::endl;
        ss << "FontName: " << FontName << std::endl;
        ss << "FamilyName: " << FamilyName << std::endl;
        ss << "Charset: " << Charset << std::endl;
        ss << "FontFile: " << FontFile << std::endl;

        if ( FontType == Font::Type::CIDType2 ){
            ss << "FontType: CIDType2" << std::endl;
        } else if ( FontType == Font::Type::Type1 ){
            ss << "FontType: Type1" << std::endl;
        } else if ( FontType == Font::Type::Type3 ){
            ss << "FontType: Type3" << std::endl;
        } else if ( FontType == Font::Type::TrueType ){
            ss << "FontType: TrueType" << std::endl;
        } else {
            ss << "FontType: Unknown" << std::endl;
        }

        if ( FontLoc == Font::Location::Embedded ){
            ss << "FontLoc: Embedded" << std::endl;
        } else if ( FontLoc == Font::Location::External ){
            ss << "FontLoc: External" << std::endl;
        } else if ( FontLoc == Font::Location::Resident ){
            ss << "FontLoc: Resident" << std::endl;
        } else {
            ss << "FontLoc: Unknown" << std::endl;
        }

        ss << "FontData Size: " << m_fontDataSize << std::endl;
        ss << std::endl
           << "------------------------------" << std::endl;

    return ss.str();
}

std::string OFDFont::GetFileName() const{
    return std::string("Font_") + std::to_string(ID) + ".ttf";
}

void OFDFont::GenerateXML(XMLWriter &writer) const{

    writer.StartElement("Font");{
        // -------- <Font ID="">
        writer.WriteAttribute("ID", ID);

        // -------- <Font FontName="">
        // Required.
        writer.WriteAttribute("FontName", FontName);

        // -------- <Font FamilyName="">
        // Optional.
        if ( !FamilyName.empty() ){
            writer.WriteAttribute("FamilyName", FamilyName);
        }

        // -------- <Font Charset="">
        // Optional.
        if ( !Charset.empty() ){
            writer.WriteAttribute("Charset", Charset);
        }

        // -------- <Font Serif="">
        // Optional.
        if ( Serif ){
            writer.WriteAttribute("Serif", true);
        }

        // -------- <Font Bold="">
        // Optional.
        if ( Bold ){
            writer.WriteAttribute("Bold", true);
        }

        // -------- <Font Italic="">
        // Optional.
        if ( Italic ){
            writer.WriteAttribute("Italic", true);
        }

        // -------- <Font FixedWidth="">
        // Optional.
        if ( FixedWidth ){
            writer.WriteAttribute("FixedWidth", true);
        }

        // -------- <FontFile>
        // Optional
        writer.WriteElement("FontFile", FontFile);

    } writer.EndElement();

}

bool OFDFont::FromXML(XMLElementPtr fontElement){

    bool ok = false;
    OFDFontPtr font = nullptr;

    std::string childName = fontElement->GetName();

    // -------- <Font>
    // OFD (section 11.1) P61. Res.xsd.
    if ( childName == "Font" ){
        bool exist = false;

        // -------- <Font FontName="">
        // Required.
        std::tie(ID, exist) = fontElement->GetIntAttribute("ID");
        if ( exist ){
            // -------- <Font FontName="">
            // Required.
            std::tie(FontName, exist) = fontElement->GetStringAttribute("FontName");
            if ( !exist ){
                LOG(ERROR) << "Attribute FontName is required in Font XML.";
            }
        } else {
            LOG(ERROR) << "Attribute ID is required in Font XML.";
        }

        if ( exist ) {

            // -------- <Font FamilyName="">
            // Optional
            std::tie(FamilyName, std::ignore) = fontElement->GetStringAttribute("FamilyName");

            // -------- <Font Charset="">
            // Optional
            std::tie(Charset, std::ignore) = fontElement->GetStringAttribute("Charset");

            // -------- <Font Charset="">
            // Optional
            std::tie(Charset, std::ignore) = fontElement->GetStringAttribute("Charset");

            // -------- <Font Serif="">
            // Optional
            std::tie(Serif, std::ignore) = fontElement->GetBooleanAttribute("Serif");

            // -------- <Font Bold="">
            // Optional
            std::tie(Bold, std::ignore) = fontElement->GetBooleanAttribute("Bold");

            // -------- <Font Italic="">
            // Optional
            std::tie(Italic, std::ignore) = fontElement->GetBooleanAttribute("Italic");

            // -------- <Font FixedWidth="">
            // Optional
            std::tie(FixedWidth, std::ignore) = fontElement->GetBooleanAttribute("FixedWidth");

            XMLElementPtr fontFileElement = fontElement->GetFirstChildElement();
            if ( fontFileElement != nullptr && fontFileElement->GetName() == "FontFile" ){
                std::tie(FontFile, std::ignore) = fontFileElement->GetStringValue();
            }

            ok = true;
        } else {
            ok = false;
        }
    }

    return ok;
}

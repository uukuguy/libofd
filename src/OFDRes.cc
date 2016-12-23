#include <vector>
#include "OFDRes.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

// **************** class OFDRes::ImplCls ****************

class OFDRes::ImplCls{
public:
    ImplCls(OFDFilePtr ofdFile, const std::string &resDescFile);
    ~ImplCls();

    void AddColorSpace(const OFDColorSpace &ofdColorSpace){m_colorSpaces.push_back(ofdColorSpace);};
    void AddFont(const OFDFont &ofdFont){m_fonts.push_back(ofdFont);};

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

private:
    bool FromColorSpacesXML(XMLElementPtr colorSpacesElement);
    bool FromFontsXML(XMLElementPtr fontsElement);

    // -------- Private Attributes --------
public:
    OFDFilePtr m_ofdFile;
    std::string m_baseLoc;
    ColorSpaceArray m_colorSpaces;
    FontArray m_fonts;
    std::string m_resDescFile;

}; // class OFDRes::ImplCls

OFDRes::ImplCls::ImplCls(OFDFilePtr ofdFile, const std::string &resDescFile) : 
    m_ofdFile(ofdFile), m_baseLoc("Res"), m_resDescFile(resDescFile) {
}

OFDRes::ImplCls::~ImplCls(){
}

void generateColorSpacesXML(XMLWriter &writer){
}

void generateDrawParamsXML(XMLWriter &writer){
}

// -------- generateFontsXML() --------
// OFD (section 11.1) P61. Res.xsd.
void generateFontsXML(XMLWriter &writer, const FontArray &fonts){

    for ( auto font : fonts ){

        writer.StartElement("Font");{

            // -------- <Font FontName="">
            // Required.
            writer.WriteAttribute("FontName", font.FontName);

            // -------- <Font FamilyName="">
            // Optional.
            if ( !font.FamilyName.empty() ){
                writer.WriteAttribute("FamilyName", font.FamilyName);
            }

            // -------- <Font Charset="">
            // Optional.
            if ( !font.Charset.empty() ){
                writer.WriteAttribute("Charset", font.Charset);
            }

            // -------- <Font Serif="">
            // Optional.
            if ( font.Serif ){
                writer.WriteAttribute("Serif", true);
            }

            // -------- <Font Bold="">
            // Optional.
            if ( font.Bold ){
                writer.WriteAttribute("Bold", true);
            }

            // -------- <Font Italic="">
            // Optional.
            if ( font.Italic ){
                writer.WriteAttribute("Italic", true);
            }

            // -------- <Font FixedWidth="">
            // Optional.
            if ( font.FixedWidth ){
                writer.WriteAttribute("FixedWidth", true);
            }

            // -------- <FontFile>
            // Optional
            writer.WriteElement("FontFile", font.FontFile);

        } writer.EndElement();

    }
}

void generateMultiMediasXML(XMLWriter &writer){
}

void generateCompositeGraphicUnitsXML(XMLWriter &writer){
}

// ======== OFDRes::ImplCls::GenerateResXML() ========
// OFD (section 7.9) P23. Res.xml.
std::string OFDRes::ImplCls::GenerateResXML() const{
    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <Page>
    writer.StartElement("Res");{
        OFDXML_HEAD_ATTRIBUTES;

        writer.WriteAttribute("BaseLoc", m_baseLoc);

        // TODO
        // -------- <ColorSpaces>
        // Optional.
        if ( m_colorSpaces.size() > 0 ){
            writer.StartElement("ColorSpaces");{
                generateColorSpacesXML(writer);    
            } writer.EndElement();
        }

        // TODO
        // -------- <DrawParams>
        // Optional.
        //if ( m_drawParams.size() > 0 ){
            //writer.StartElement("DrawParams");{
                //generateDrawParamsXML(writer);    
            //} writer.EndElement();
        //}

        // -------- <Fonts>
        // Optional.
        if ( m_fonts.size() > 0 ){
            writer.StartElement("Fonts");{
                generateFontsXML(writer, m_fonts);    
            } writer.EndElement();
        }

        // TODO
        // -------- <MultiMedias>
        // Optional.
        //if ( m_multiMedias.size() > 0 ){
            //writer.StartElement("MultiMedias");{
                //generateMultiMediasXML(writer);    
            //} writer.EndElement();
        //}

        // TODO
        // -------- <CompositeGraphicUnits>
        // Optional.
        //if ( m_compositeGraphicUnits.size() > 0 ){
            //writer.StartElement("CompositeGraphicUnits");{
                //generateCompositeGraphicUnitsXML(writer);    
            //} writer.EndElement();
        //}

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

bool OFDRes::ImplCls::FromColorSpacesXML(XMLElementPtr colorSpacesElement){
    bool ok = true;

    return ok;
}

bool OFDRes::ImplCls::FromFontsXML(XMLElementPtr fontsElement){
    bool ok = false;

    XMLElementPtr childElement = fontsElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        // -------- <Font>
        // OFD (section 11.1) P61. Res.xsd.
        if ( childName == "Font" ){
            XMLElementPtr fontElement = childElement;

            OFDFont font;
            bool exist = false;

            // -------- <Font FontName="">
            // Required.
            std::tie(font.FontName, exist) = fontElement->GetStringAttribute("FontName");
            if ( !exist ){
                LOG(ERROR) << "Attribute FontName is required in Font XML.";
            } else {

                // -------- <Font FamilyName="">
                // Optional
                std::tie(font.FamilyName, std::ignore) = fontElement->GetStringAttribute("FamilyName");

                // -------- <Font Charset="">
                // Optional
                std::tie(font.Charset, std::ignore) = fontElement->GetStringAttribute("Charset");

                // -------- <Font Charset="">
                // Optional
                std::tie(font.Charset, std::ignore) = fontElement->GetStringAttribute("Charset");
                
                // -------- <Font Serif="">
                // Optional
                std::tie(font.Serif, std::ignore) = fontElement->GetBooleanAttribute("Serif");

                // -------- <Font Bold="">
                // Optional
                std::tie(font.Bold, std::ignore) = fontElement->GetBooleanAttribute("Bold");

                // -------- <Font Italic="">
                // Optional
                std::tie(font.Italic, std::ignore) = fontElement->GetBooleanAttribute("Italic");

                // -------- <Font FixedWidth="">
                // Optional
                std::tie(font.FixedWidth, std::ignore) = fontElement->GetBooleanAttribute("FixedWidth");

                XMLElementPtr fontFileElement = fontElement->GetFirstChildElement();
                if ( fontFileElement != nullptr && fontFileElement->GetName() == "FontFile" ){
                    std::tie(font.FontFile, std::ignore) = fontFileElement->GetStringValue();
                }

                m_fonts.push_back(font);
                ok = true;
            }

        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// ======== OFDRes::ImplCls::FromResXML() ========
// OFD (section 7.9) P23. Res.xml.
bool OFDRes::ImplCls::FromResXML(const std::string &strResXML){
    bool ok = false;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strResXML);
    if ( rootElement != nullptr ){
        if ( rootElement->GetName() == "Res" ){

            // -------- <Res BaseLoc="">
            // Required.
            bool exist = false;
            std::tie(m_baseLoc, exist) = rootElement->GetStringAttribute("BaseLoc");
            if ( !exist ){
                LOG(ERROR) << "BaseLoc attribute is required in Res.xsd";
                return false;
            }

            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                if ( childName == "ColorSpaces" ){
                    // -------- <ColorSpaces>
                    // Optional
                    FromColorSpacesXML(childElement);

                //} else if ( childName == "DrawParams" ){
                    // TODO
                    // -------- <DrawParams>
                    // Optional

                } else if ( childName == "Fonts" ){
                    // -------- <Fonts>
                    // Optional
                    FromFontsXML(childElement);

                //} else if ( childName == "MultiMedias" ){
                    // TODO
                    // -------- <MultiMedias>
                    // Optional

                //} else if ( childName == "CompositeGraphicUnits" ){
                    // TODO
                    // -------- <CompositeGraphicUnits>
                    // Optional

                }

                childElement = rootElement->GetNextSiblingElement();
            }
        }
    }

    return ok;
}

// **************** class OFDRes ****************

OFDRes::OFDRes(OFDFilePtr ofdFile, const std::string &resDescFile){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(ofdFile, resDescFile));
}

OFDRes::~OFDRes(){
}

std::string OFDRes::GetBaseLoc() const{
    return m_impl->m_baseLoc;
}

void OFDRes::SetBaseLoc(const std::string &baseLoc){
    m_impl->m_baseLoc = baseLoc;
}

void OFDRes::AddColorSpace(const OFDColorSpace &ofdColorSpace){
    m_impl->AddColorSpace(ofdColorSpace);
}

void OFDRes::AddFont(const OFDFont &ofdFont){
    m_impl->AddFont(ofdFont);
}

std::string OFDRes::GenerateResXML() const{
    return m_impl->GenerateResXML();
}

bool OFDRes::FromResXML(const std::string &strResXML){
    return m_impl->FromResXML(strResXML);
}

std::string OFDRes::GetResDescFile() const{
    return m_impl->m_resDescFile;
}

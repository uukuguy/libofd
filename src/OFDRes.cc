#include <vector>
#include "OFDRes.h"
#include "utils/xml.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

// **************** class OFDRes::ImplCls ****************

class OFDRes::ImplCls{
public:
    ImplCls(OFDFilePtr ofdFile);
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

}; // class OFDRes::ImplCls

OFDRes::ImplCls::ImplCls(OFDFilePtr ofdFile) : m_ofdFile(ofdFile) {
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
        writer.StartElement("ColorSpaces");{
            generateColorSpacesXML(writer);    
        } writer.EndElement();

        // TODO
        // -------- <DrawParams>
        // Optional.
        writer.StartElement("DrawParams");{
            generateDrawParamsXML(writer);    
        } writer.EndElement();

        // -------- <Fonts>
        // Optional.
        writer.StartElement("Fonts");{
            generateFontsXML(writer, m_fonts);    
        } writer.EndElement();

        // TODO
        // -------- <MultiMedias>
        // Optional.
        writer.StartElement("MultiMedias");{
            generateMultiMediasXML(writer);    
        } writer.EndElement();

        // TODO
        // -------- <CompositeGraphicUnits>
        // Optional.
        writer.StartElement("CompositeGraphicUnits");{
            generateCompositeGraphicUnitsXML(writer);    
        } writer.EndElement();

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

bool OFDRes::ImplCls::FromColorSpacesXML(XMLElementPtr colorSpacesElement){
    bool ok = true;

    return ok;
}

bool OFDRes::ImplCls::FromFontsXML(XMLElementPtr fontsElement){
    bool ok = true;

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

    //XMLReader reader;
    //if ( reader.ParseXML(strResXML) ){

        //if ( reader.CheckElement("Res") ){
            //if ( reader.EnterChildElement("Res") ){

                //// -------- <Res BaseLoc="">
                //// Required.
                //reader.ReadAttribute("BaseLoc", m_baseLoc);
                //if ( m_baseLoc.empty() ) return false;

                //while ( reader.HasElement() ){

                    //// -------- <ColorSpaces>
                    //// Optional
                    //if ( reader.CheckElement("ColorSpaces") ){
                        //FromColorSpacesXML(reader, "ColorSpaces");

                    //// TODO
                    //// -------- <DrawParams>
                    //// Optional
                    ////} else if ( reader.CheckElement("DrawParams") ){

                    //// -------- <Fonts>
                    //// Optional
                    //} else if ( reader.CheckElement("Fonts") ){
                        //FromFontsXML(reader, "Fonts");

                    //// TODO
                    //// -------- <MultiMedias>
                    //// Optional
                    ////} else if ( reader.CheckElement("MultiMedias") ){

                    //// TODO
                    //// -------- <CompositeGraphicUnits>
                    //// Optional
                    ////} else if ( reader.CheckElement("CompositeGraphicUnits") ){

                    //}

                    //reader.NextElement();
                //}

                //reader.BackParentElement();
            //}
        //}
    //}

    return ok;
}

// **************** class OFDRes ****************

OFDRes::OFDRes(OFDFilePtr ofdFile){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(ofdFile));
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

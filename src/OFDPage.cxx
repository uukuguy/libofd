#include <iostream>
#include <sstream>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "utils.h"
#include "logger.h"

#include <tinyxml2.h>
using namespace tinyxml2;
using namespace ofd;

OFDPage::OFDPage(OFDDocument *ofdDocument, uint64_t id, const std::string &filename)
    : m_ofdDocument(ofdDocument), m_id(id), m_filename(filename) {
    m_attributes.clear();
}

OFDPage::~OFDPage() {
    Close();
}

bool OFDPage::Open() {
    if ( IsOpened() ) return true;
    if ( m_ofdDocument == NULL ) return false;

    OFDPackage *ofdPackage = m_ofdDocument->GetOFDPackage();
    if ( ofdPackage == NULL ) return false;

    bool ok = false;
    std::string content;
    std::tie(content, ok) = ofdPackage->GetFileContent(m_filename);
    if ( !ok ) return false;

    m_opened = parseXML(content);

    return IsOpened();
}


void OFDPage::Close() {
    if ( !IsOpened() ) return;
    m_attributes.clear();
    m_text = "";
    m_opened = false;
}

std::string OFDPage::String() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDPage" << std::endl 
        << "PhysicalBox: " << m_attributes.PageArea.physicalBox.x0 << ", " 
        << m_attributes.PageArea.physicalBox.y0 << ", "
        << m_attributes.PageArea.physicalBox.x1 << ", "
        << m_attributes.PageArea.physicalBox.y1 
        << std::endl
        << "------------------------------" << std::endl
        ;
    return ss.str();
}

bool OFDPage::parseXML(const std::string &content) {
    XMLDocument xmldoc;
    XMLError rc = xmldoc.Parse(content.c_str());
    if ( rc != XML_SUCCESS ){
        return false;
    }


    XMLElement *rootElement = xmldoc.RootElement();
    if ( rootElement != NULL ){
        m_text = "";

        VLOG(3) << "Root Element Name: " << rootElement->Name();
        VLOG(3) << GetChildElements(rootElement);

        // <ofd:Area>
        const XMLElement *areaElement = rootElement->FirstChildElement("ofd:Area");
        if ( areaElement == NULL ) return false;
        VLOG(3) << GetChildElements(areaElement);

        //     <ofd:PhysicalBox>
        const XMLElement *physicalBoxElement = areaElement->FirstChildElement("ofd:PhysicalBox");
        if ( physicalBoxElement == NULL ) return false;
        double x0, y0, x1, y1;
        bool ok;
        std::tie(x0, y0, x1, y1, ok) = parsePhysicalBoxElement(physicalBoxElement);
        if ( ok ) {
            this->m_attributes.PageArea.physicalBox.x0 = x0;
            this->m_attributes.PageArea.physicalBox.y0 = y0;
            this->m_attributes.PageArea.physicalBox.x1 = x1;
            this->m_attributes.PageArea.physicalBox.y1 = y1;  
        }

        // <ofd:Content>
        const XMLElement *contentElement = rootElement->FirstChildElement("ofd:Content");
        if ( contentElement == NULL ) return false;
        VLOG(3) << GetChildElements(contentElement);

        //     <ofd:Layer>
        const XMLElement *layerElement = contentElement->FirstChildElement("ofd:Layer");
        if ( layerElement == NULL ) return false;
        //VLOG(3) << GetChildElements(layerElement);

        const XMLElement *textObjectElement = layerElement->FirstChildElement("ofd:TextObject");
        while ( textObjectElement != NULL ) {
            VLOG_N_TIMES(1, 3) << GetChildElements(textObjectElement);

            //<ofd:TextObject ID="121" CTM="0.3527 0 0 0.3527 -114.807533 111.352325" 
            //                Boundary="114.807533 185.229584 4.083549 4.733795" 
            //                LineWidth="1" MiterLimit="3.527" Font="16" Size="14.749" 
            //                Stroke="false" Fill="true">
                //<ofd:FillColor ColorSpace="15" Value="0"/>
                //<ofd:StrokeColor ColorSpace="15" Value="0"/>
                //<ofd:CGTransform CodePosition="0" CodeCount="1" GlyphCount="1">
                    //<ofd:Glyphs>4460</ofd:Glyphs> 
                //</ofd:CGTransform>
                //<ofd:TextCode X="324.419" Y="-303.723">局</ofd:TextCode>
            //</ofd:TextObject>

            struct OFDColor {
                int ColorSpace;
                int Value;
            };

            struct OFDBoundary {
                double x0, y0, w, h;
            };

            // CTM (Context Translate Matrix)
            //
            // a  b  0
            // c  d  0
            // p  q  1
            //
            struct OFDCTM {
                double a, b, c, d, p, q;
            };

            int ID = textObjectElement->IntAttribute("ID");

            // CTM attribute.
            OFDCTM CTM;
            std::string c = textObjectElement->Attribute("CTM"); 
            std::vector<std::string> ctmTokens = SpliteString(c);
            if ( ctmTokens.size() == 6 ){
                CTM.a = atof(ctmTokens[0].c_str());
                CTM.b = atof(ctmTokens[1].c_str());
                CTM.c = atof(ctmTokens[2].c_str());
                CTM.d = atof(ctmTokens[3].c_str());
                CTM.p = atof(ctmTokens[4].c_str());
                CTM.q = atof(ctmTokens[5].c_str());
            }

            // Boundary attribute.
            OFDBoundary Boundary;
            std::string b = textObjectElement->Attribute("Boundary");
            std::vector<std::string> boundaryTokens = SpliteString(b);
            if ( boundaryTokens.size() == 4 ){
                Boundary.x0 = atof(boundaryTokens[0].c_str());
                Boundary.y0 = atof(boundaryTokens[1].c_str());
                Boundary.w = atof(boundaryTokens[2].c_str());
                Boundary.h = atof(boundaryTokens[3].c_str());
            }

            // LineWidth attribute.
            int LineWidth = textObjectElement->IntAttribute("LineWidth");

            // MiterLimit attribute.
            double MiterLimit = textObjectElement->DoubleAttribute("MiterLimit");

            // Font attribute.
            int Font = textObjectElement->IntAttribute("Font");

            // FontSize attribute.
            double FontSize = textObjectElement->DoubleAttribute("Size");

            // Stroke attribute.
            bool Stroke = textObjectElement->BoolAttribute("Stroke");

            // Fill attribute.
            bool Fill = textObjectElement->BoolAttribute("Fill");

            // <ofd:FillColor>
            const XMLElement *fillColorElement = textObjectElement->FirstChildElement("ofd:FillColor");
            OFDColor fillColor;
            fillColor.ColorSpace = fillColorElement->IntAttribute("ColorSpace");
            fillColor.Value = fillColorElement->IntAttribute("Value");

            // <ofd:StrokeColor>
            const XMLElement *strokeColorElement = textObjectElement->FirstChildElement("ofd:StrokeColor");
            OFDColor strokeColor;
            strokeColor.ColorSpace = strokeColorElement->IntAttribute("ColorSpace");
            strokeColor.Value = strokeColorElement->IntAttribute("Value");

            // <ofd:TextCode>
            const XMLElement *textCodeElement = textObjectElement->FirstChildElement("ofd:TextCode");
            std::string text = textCodeElement->GetText();
            m_text += text;

            if ( VLOG_IS_ON(5) ){
                std::stringstream ss;
                ss << "CTM( " << CTM.a << ", " << CTM.b << ", " << CTM.c << ", " << CTM.d << ", " << CTM.p << ", " << CTM.q << ") ";
                ss << "Boundary( " << Boundary.x0 << ", " << Boundary.y0 << ", " << Boundary.w << ", " << Boundary.h << ") ";
                ss << " LineWidth:" << LineWidth;
                ss << " MiterLimit:" << MiterLimit;
                ss << " Font:" << Font;
                ss << " FontSize:" << FontSize;
                ss << " Stroke:" << Stroke;
                ss << " Fill:" << Fill;

                VLOG(5) << ss.str();

                VLOG(5) << "Fill(" << fillColor.ColorSpace << "," << fillColor.Value << ") "
                    << "Stroke(" << strokeColor.ColorSpace << "," << strokeColor.Value << ") " 
                    << text;
            }

            textObjectElement = textObjectElement->NextSiblingElement("ofd:TextObject");
        }

        return true;
    }
    
    return false;
}




#include <iostream>
#include <sstream>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "OFDTextObject.h"
#include "utils.h"
#include "logger.h"

#include <tinyxml2.h>
using namespace tinyxml2;
using namespace ofd;

OFDPage::OFDPage(OFDDocument *ofdDocument, uint64_t id, const std::string &filename)
    : m_ofdDocument(ofdDocument), m_id(id), m_filename(filename), m_opened(false) {
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
    clear();
    m_opened = false;
}

void OFDPage::clear() {
    m_text = "";
    m_attributes.clear();

    for (size_t i = 0 ; i < m_ofdObjects.size() ; i++ ) {
        OFDObject *ofdObject = m_ofdObjects[i];
        if ( ofdObject != NULL ) delete ofdObject;
    }
    m_ofdObjects.clear();
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
        clear();

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

            OFDTextObject* textObject = new OFDTextObject();

            textObject->ID = textObjectElement->IntAttribute("ID");

            // CTM attribute.
            std::string c = textObjectElement->Attribute("CTM"); 
            std::vector<std::string> ctmTokens = SpliteString(c);
            if ( ctmTokens.size() == 6 ){
                textObject->CTM.a = atof(ctmTokens[0].c_str());
                textObject->CTM.b = atof(ctmTokens[1].c_str());
                textObject->CTM.c = atof(ctmTokens[2].c_str());
                textObject->CTM.d = atof(ctmTokens[3].c_str());
                textObject->CTM.p = atof(ctmTokens[4].c_str());
                textObject->CTM.q = atof(ctmTokens[5].c_str());
            }

            // Boundary attribute.
            std::string b = textObjectElement->Attribute("Boundary");
            std::vector<std::string> boundaryTokens = SpliteString(b);
            if ( boundaryTokens.size() == 4 ){
                textObject->Boundary.x0 = atof(boundaryTokens[0].c_str());
                textObject->Boundary.y0 = atof(boundaryTokens[1].c_str());
                textObject->Boundary.w = atof(boundaryTokens[2].c_str());
                textObject->Boundary.h = atof(boundaryTokens[3].c_str());
            }

            // LineWidth attribute.
            textObject->LineWidth = textObjectElement->DoubleAttribute("LineWidth");

            // MiterLimit attribute.
            textObject->MiterLimit = textObjectElement->DoubleAttribute("MiterLimit");

            // Font attribute.
            textObject->Font = textObjectElement->DoubleAttribute("Font");

            // FontSize attribute.
            textObject->FontSize = textObjectElement->DoubleAttribute("Size");

            // Stroke attribute.
            textObject->Stroke = textObjectElement->BoolAttribute("Stroke");

            // Fill attribute.
            textObject->Fill = textObjectElement->BoolAttribute("Fill");

            // <ofd:FillColor>
            const XMLElement *fillColorElement = textObjectElement->FirstChildElement("ofd:FillColor");
            textObject->FillColor.ColorSpace = fillColorElement->IntAttribute("ColorSpace");
            textObject->FillColor.Value = fillColorElement->DoubleAttribute("Value");

            // <ofd:StrokeColor>
            const XMLElement *strokeColorElement = textObjectElement->FirstChildElement("ofd:StrokeColor");
            textObject->StrokeColor.ColorSpace = strokeColorElement->IntAttribute("ColorSpace");
            textObject->StrokeColor.Value = strokeColorElement->DoubleAttribute("Value");

            // <ofd:TextCode>
            const XMLElement *textCodeElement = textObjectElement->FirstChildElement("ofd:TextCode");
            textObject->X = textCodeElement->DoubleAttribute("X");
            textObject->Y = textCodeElement->DoubleAttribute("Y");
            textObject->Text = textCodeElement->GetText();

            m_ofdObjects.push_back(textObject);
            m_text += textObject->Text;

            if ( VLOG_IS_ON(5) ){
                VLOG(5) << textObject->ToString();
            }

            textObjectElement = textObjectElement->NextSiblingElement("ofd:TextObject");
        }

        return true;
    }
    
    return false;
}

#include <cairo/cairo.h>
bool OFDPage::RenderToPNGFile(const std::string& filename){

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1000, 1000);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    cairo_text_extents_t te;
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);

    for ( size_t i = 0 ; i < GetOFDObjectsCount() ; i++ ){
        OFDObject *ofdObject = GetOFDObject(i);
        OFDTextObject *textObject = static_cast<OFDTextObject*>(ofdObject);
        double X = textObject->X;
        double Y = textObject->Y + 800;
        std::string Text = textObject->Text;

        cairo_text_extents(cr, Text.c_str(), &te);
        cairo_move_to(cr, X + 0.5 - te.width / 2 - te.x_bearing, Y + 0.5 - te.height / 2 - te.y_bearing);
        cairo_show_text(cr, Text.c_str());
    }



    cairo_surface_write_to_png(surface, filename.c_str());

    cairo_surface_destroy(surface);
    return true;
}



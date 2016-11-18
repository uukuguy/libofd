#include <iostream>
#include <sstream>
#include "FontResource.h"
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "OFDCanvas.h"
#include "OFDTextObject.h"
#include "utils.h"
#include "logger.h"


/***
 *
    A4纸的尺寸是210mm×297mm， 
    当设定的分辨率是72像素/英寸时，A4纸的尺寸的图像的像素是595×842， 
    当设定的分辨率是150像素/英寸时，A4纸的尺寸的图像的像素是1240×1754， 
    当设定的分辨率是300像素/英寸时，A4纸的尺寸的图像的像素是2479×3508，
    1英寸=25.4毫米。
 *
 * ***/

#include <tinyxml2.h>
using namespace tinyxml2;
using namespace ofd;

OFDPage::OFDPage(OFDDocument *document, uint64_t id, const std::string &filename)
    : m_document(document), m_id(id), m_filename(filename), m_opened(false){
    m_attributes.clear();
}

OFDPage::~OFDPage() {
    Close();
}

//OFDPackagePtr OFDPage::GetPackage() {
OFDPackage *OFDPage::GetPackage() {
    return m_document->GetPackage();
    //OFDDocumentPtr document = m_document.lock();
    //if ( document != nullptr ){
        //return document->GetPackage();
    //} else {
        //return nullptr;
    //}
}

//const OFDPackagePtr OFDPage::GetPackage() const {
const OFDPackage *OFDPage::GetPackage() const {
    return m_document->GetPackage();
    //OFDDocumentPtr document = m_document.lock();
    //if ( document != nullptr ){
        //return document->GetPackage();
    //} else {
        //return nullptr;
    //}
}

bool OFDPage::Open() {
    if ( IsOpened() ) {
        //LOG(DEBUG) << "Page is already opened!";
        return true;
    }

    if ( m_document == nullptr ) {
    //if ( m_document.lock() == nullptr ) {
        LOG(WARNING) << "m_document == nullptr";
        return false;
    }

    OFDPackage *package = m_document->GetPackage();
    //OFDPackagePtr package = GetPackage();
    if ( package == nullptr ) {
        LOG(WARNING) << "package == nullptr";
        return false;
    }

    bool ok = false;
    std::string content;
    std::tie(content, ok) = package->GetFileContent(m_filename);
    if ( !ok ) {
        LOG(WARNING) << "package->GetFileContent() failed. filename: " << m_filename;
        return false;
    }

    m_opened = parseXML(content);

    if ( IsOpened() ){
        //double mmWidth = m_attributes.PageArea.PhysicalBox.Width();
        //double mmHeight = m_attributes.PageArea.PhysicalBox.Height();
        //m_canvas = std::unique_ptr<OFDCanvas>(new OFDCanvas(mmWidth, mmHeight));
    }

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
        if ( ofdObject != nullptr ) delete ofdObject;
    }
    m_ofdObjects.clear();

    //m_canvas = nullptr;
}

std::string OFDPage::String() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDPage" << std::endl 
        << "PhysicalBox: " << m_attributes.PageArea.PhysicalBox.x0 << ", " 
        << m_attributes.PageArea.PhysicalBox.y0 << ", "
        << m_attributes.PageArea.PhysicalBox.x1 << ", "
        << m_attributes.PageArea.PhysicalBox.y1 
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
    if ( rootElement != nullptr ){
        clear();

        VLOG(3) << "Root Element Name: " << rootElement->Name();
        VLOG(3) << GetChildElements(rootElement);

        // <ofd:Area>
        const XMLElement *areaElement = rootElement->FirstChildElement("ofd:Area");
        if ( areaElement == nullptr ) return false;
        VLOG(3) << GetChildElements(areaElement);

        //     <ofd:PhysicalBox>
        const XMLElement *physicalBoxElement = areaElement->FirstChildElement("ofd:PhysicalBox");
        if ( physicalBoxElement == nullptr ) return false;
        double x0, y0, x1, y1;
        bool ok;
        std::tie(x0, y0, x1, y1, ok) = parsePhysicalBoxElement(physicalBoxElement);
        if ( ok ) {
            this->m_attributes.PageArea.PhysicalBox.x0 = x0;
            this->m_attributes.PageArea.PhysicalBox.y0 = y0;
            this->m_attributes.PageArea.PhysicalBox.x1 = x1;
            this->m_attributes.PageArea.PhysicalBox.y1 = y1;  
        }

        // <ofd:Content>
        const XMLElement *contentElement = rootElement->FirstChildElement("ofd:Content");
        if ( contentElement == nullptr ) return false;
        VLOG(3) << GetChildElements(contentElement);

        //     <ofd:Layer>
        const XMLElement *layerElement = contentElement->FirstChildElement("ofd:Layer");
        if ( layerElement == nullptr ) return false;
        //VLOG(3) << GetChildElements(layerElement);

        const XMLElement *objectElement = layerElement->FirstChildElement();
        if ( objectElement == nullptr ) {
            LOG(WARNING) << "No children node under <ofd:Layer>.";
        } else {
            while ( objectElement != nullptr ){
                std::string elementName = objectElement->Name();
                OFDObject *object = OFDObject::CreateObject(elementName);
                if ( object != nullptr ){
                    if ( !object->ParseFromXML(objectElement) ){
                        LOG(WARNING) << "object->ParseFromXML() failed.";
                        delete object;
                    } else {
                        m_ofdObjects.push_back(object);
                        std::string elementName = objectElement->Name();
                        if ( elementName == "ofd:TextObject" ){
                            OFDTextObject *textObject = static_cast<OFDTextObject*>(object);
                            m_text += textObject->Text;
                        }
                    }
                }

                objectElement = objectElement->NextSiblingElement();
            } 
        }

        //const XMLElement *textObjectElement = layerElement->FirstChildElement("ofd:TextObject");
        //while( false){
        ////while ( textObjectElement != nullptr ) {
            //VLOG_N_TIMES(3, 1) << GetChildElements(textObjectElement);

            ////<ofd:TextObject ID="121" CTM="0.3527 0 0 0.3527 -114.807533 111.352325" 
            ////                Boundary="114.807533 185.229584 4.083549 4.733795" 
            ////                LineWidth="1" MiterLimit="3.527" Font="16" Size="14.749" 
            ////                Stroke="false" Fill="true">
                ////<ofd:FillColor ColorSpace="15" Value="0"/>
                ////<ofd:StrokeColor ColorSpace="15" Value="0"/>
                ////<ofd:CGTransform CodePosition="0" CodeCount="1" GlyphCount="1">
                    ////<ofd:Glyphs>4460</ofd:Glyphs> 
                ////</ofd:CGTransform>
                ////<ofd:TextCode X="324.419" Y="-303.723">局</ofd:TextCode>
            ////</ofd:TextObject>

            //OFDTextObject* textObject = new OFDTextObject();

            //textObject->ID = textObjectElement->IntAttribute("ID");

            //// CTM attribute.
            //std::string c = textObjectElement->Attribute("CTM"); 
            //std::vector<std::string> ctmTokens = SpliteString(c);
            //if ( ctmTokens.size() == 6 ){
                //textObject->CTM.xx = atof(ctmTokens[0].c_str());
                //textObject->CTM.xy = atof(ctmTokens[1].c_str());
                //textObject->CTM.yx = atof(ctmTokens[2].c_str());
                //textObject->CTM.yy = atof(ctmTokens[3].c_str());
                //textObject->CTM.x0 = atof(ctmTokens[4].c_str());
                //textObject->CTM.y0 = atof(ctmTokens[5].c_str());
            //}

            //// Boundary attribute.
            //std::string b = textObjectElement->Attribute("Boundary");
            //std::vector<std::string> boundaryTokens = SpliteString(b);
            //if ( boundaryTokens.size() == 4 ){
                //textObject->Boundary.x0 = atof(boundaryTokens[0].c_str());
                //textObject->Boundary.y0 = atof(boundaryTokens[1].c_str());
                //textObject->Boundary.w = atof(boundaryTokens[2].c_str());
                //textObject->Boundary.h = atof(boundaryTokens[3].c_str());
            //}

            //// LineWidth attribute.
            //textObject->LineWidth = textObjectElement->DoubleAttribute("LineWidth");

            //// MiterLimit attribute.
            //textObject->MiterLimit = textObjectElement->DoubleAttribute("MiterLimit");

            //// Font attribute.
            //textObject->Font = textObjectElement->DoubleAttribute("Font");

            //// FontSize attribute.
            //textObject->FontSize = textObjectElement->DoubleAttribute("Size");

            //// Stroke attribute.
            //textObject->Stroke = textObjectElement->BoolAttribute("Stroke");

            //// Fill attribute.
            //textObject->Fill = textObjectElement->BoolAttribute("Fill");

            //// <ofd:FillColor>
            //const XMLElement *fillColorElement = textObjectElement->FirstChildElement("ofd:FillColor");
            //textObject->FillColor.ColorSpace = fillColorElement->IntAttribute("ColorSpace");
            //textObject->FillColor.Value = fillColorElement->DoubleAttribute("Value");

            //// <ofd:StrokeColor>
            //const XMLElement *strokeColorElement = textObjectElement->FirstChildElement("ofd:StrokeColor");
            //textObject->StrokeColor.ColorSpace = strokeColorElement->IntAttribute("ColorSpace");
            //textObject->StrokeColor.Value = strokeColorElement->DoubleAttribute("Value");

            //// <ofd:TextCode>
            //const XMLElement *textCodeElement = textObjectElement->FirstChildElement("ofd:TextCode");
            //textObject->X = textCodeElement->DoubleAttribute("X");
            //textObject->Y = textCodeElement->DoubleAttribute("Y");
            //textObject->Text = textCodeElement->GetText();

            //m_ofdObjects.push_back(textObject);
            //m_text += textObject->Text;

            //if ( VLOG_IS_ON(5) ){
                //VLOG(5) << textObject->ToString();
            //}

            //textObjectElement = textObjectElement->NextSiblingElement("ofd:TextObject");
        //}

        return true;
    }
    
    return false;
}

void OFDPage::drawText(const OFDTextObject *textObject) const {
    std::string Text = textObject->Text;
    OFDDocument *document = m_document;
    //OFDDocumentPtr document = m_document.lock();
    if ( document == nullptr ) return;
    const OFDFont &font = document->GetFontByID(textObject->Font);

    std::string documentRootDir = document->GetRootDir();
    std::string publicResBaseLoc = document->GetPublicResBaseLoc();
    std::string fontFileName = documentRootDir + "/" + publicResBaseLoc + "/" + font.FontFile;
    //VLOG(3) << "Font filename: " << fontFileName;

    OFDPackage *package = document->GetPackage();
    //OFDPackagePtr package = GetPackage();

    bool ok = false;
    std::string content;
    std::tie(content, ok) = package->GetFileContent(fontFileName);
    if ( !ok ) {
        LOG(WARNING) << "Error: Font file " << fontFileName << " can't be loaded.";
        return;
    };


}

#include <cairo/cairo.h>
bool OFDPage::Render(cairo_surface_t *surface){
    //double dpi = default_dpi;
    double dpi = 96;
    double pixels_per_mm = dpi / mm_per_inch;
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    //cairo_set_source_rgb(cr, 0.8, 0, 0);
    //cairo_rectangle(cr, 0, 0, 50, 40);
    //cairo_stroke(cr);
    
    //cairo_set_source_rgb(cr, 0.0, 0, 0.8);
    //cairo_rectangle(cr, 200, 200, 100, 80);
    //cairo_stroke(cr);

    cairo_rectangle(cr, 0, 0 + 0.5, 18.1944, 18.1944 + 0.5);
    cairo_stroke(cr);


    // -------- Set FontFace --------
    cairo_select_font_face(cr, "Simsun", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    //cairo_set_font_size(cr, 12);

    cairo_scale(cr, pixels_per_mm, pixels_per_mm);

    size_t totalObjects = GetOFDObjectsCount(); 
    if ( totalObjects > 0 ) {
        //LOG(DEBUG) << "Total " << GetOFDObjectsCount() << " OFD objects to be drawn.";
    } else{
        LOG(WARNING) << "There are 0 OFD objects in the page. Page ID=" << this->GetID();
    }

#define MAX_LOG_ITEMS 5

    for ( size_t i = 0 ; i < GetOFDObjectsCount() ; i++ ){
        //cairo_save(cr);

        OFDObject *ofdObject = GetOFDObject(i);
        if ( ofdObject->GetObjectType() != OFDObjectType::TEXT ) continue;

        OFDTextObject *textObject = static_cast<OFDTextObject*>(ofdObject);

        //const std::shared_ptr<FontResource> fontResource = m_document->GetFontResource();
        //cairo_font_face_t *fontFace = (cairo_font_face_t*)fontResource->GetFontFace(textObject->Font);
        //if ( fontFace != nullptr ){
            //cairo_set_font_face(cr, fontFace);
            
            ////cairo_font_options_t *font_options = cairo_font_options_create();
            ////cairo_scaled_font_t *scaled_font = cairo_scaled_font_create(fontFace, &font_matrix, &ctm, font_options);
            ////cairo_set_scaled_font(cr, scaled_font);

        //} else {
            //LOG(WARNING) << "Font not found. ID=" << textObject->Font;
        //}

        // -------- textObject->CTM --------
        double xx = textObject->CTM.xx;
        double xy = textObject->CTM.xy;
        double yx = textObject->CTM.yx;
        double yy = textObject->CTM.yy;
        double x0 = textObject->CTM.x0;
        double y0 = textObject->CTM.y0;

        //cairo_matrix_t cairo_matrix{xx, xy, yx, yy, x0, y0};
        VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << "\n------------------------------------------";
        VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << std::endl  
               <<  "xx:" << xx << " xy:" << xy << std::endl
               << " yx:" << yx << " yy:" << yy << std::endl
               << " x0:" << x0 << " y0:" << y0 << std::endl;
        //cairo_matrix_t ctm{xx, xy, yx, yy, x0, y0};

        //cairo_set_matrix(cr, &cairo_matrix);

        // -------- font_matrix --------
        cairo_matrix_t font_matrix;
        cairo_get_font_matrix(cr, &font_matrix);
        //VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << "Before font_matrix: {"<< 
            //font_matrix.xx << ", " <<
            //font_matrix.xy << ", " <<
            //font_matrix.yx << ", " <<
            //font_matrix.yy << ", " <<
            //font_matrix.x0 << "，" <<
            //font_matrix.y0 << "}";

        //double ratio = mmWidth / textObject->Boundary.w;
        double fontSize = textObject->FontSize;
        //double ratio = dpi / 72;
        //double realFontSize = fontSize / pixels_per_mm;
        //double realFontSize = fontSize * pixels_per_mm;

        double fontPixels = dpi * fontSize / 72;
        //cairo_set_font_size(cr, fontPixels);
        //VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << "fontSize:" << fontSize << " fontPixels:" << fontPixels << " ratio:" << ratio;
        font_matrix.xx = fontPixels * xx;
        font_matrix.yy = fontPixels * yy;
        font_matrix.x0 = x0;
        font_matrix.y0 = y0;
        cairo_set_font_matrix(cr, &font_matrix);
        
        cairo_get_font_matrix(cr, &font_matrix);
        //VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << "After font_matrix: {"<< 
            //font_matrix.xx << ", " <<
            //font_matrix.xy << ", " <<
            //font_matrix.yx << ", " <<
            //font_matrix.yy << ", " <<
            //font_matrix.x0 << "，" <<
            //font_matrix.y0 << "}";



        std::string Text = textObject->Text;
        double X = textObject->Boundary.x0;
        double Y = textObject->Boundary.y0;
        double X1 = X;
        double Y1 = Y;
        //double X1 = length_to_pixel(X, dpi);
        //double Y1 = length_to_pixel(Y, dpi);

        // -----------------------------------------------
        //double X = textObject->X;
        //double Y = textObject->Y;
        //LOG(DEBUG) << "X: " << X << " Y: " << Y;

        //cairo_matrix_t ctm{1, 0, 0, 1, 0, 0};
        //cairo_get_matrix(cr, &ctm);
        ////cairo_set_matrix(cr, &ctm);
        //LOG(DEBUG) << "CTM{" << ctm.xx << ", " << ctm.xy << ", " 
                            //<< ctm.yx << ", " << ctm.yy << ", "
                            //<< ctm.x0 << ", " << ctm.y0 << "}";


        //double X1 = X;
        //double Y1 = Y;

        //double X1 = X * xx + x0;
        //double Y1 = Y * yy + y0;
        
        //cairo_user_to_device(cr, &X1, &Y1);
        //LOG(DEBUG) << "user_to_device() X1:" << X1 << " Y1:" << Y1;

        //X1 = length_to_pixel(X1, dpi);
        //Y1 = length_to_pixel(Y1, dpi);
        //Y1 = pixelHeight + Y1 + length_to_pixel(y0, dpi);
        //Y1 = pixelHeight + Y1;

        ////double X1 = (X - x0) / xx;
        ////double Y1 = (Y - y0) / yy;

        //double X1 = X * xx + x0;
        //double Y1 = Y * yy + y0;
        ////Y1 = (pixelHeight + Y1);

        //X1 = length_to_pixel(X, dpi);
        //Y1 = length_to_pixel(Y, dpi);
        //Y1 = (pixelHeight + Y1);
        
        //LOG(DEBUG) << X << " * " << xx << " + " << x0 << " = " << X1;
        //LOG(DEBUG) << Y << " * " << yy << " + " << y0 << " = " << Y1;
        VLOG_N_TIMES(MAX_LOG_ITEMS, 1) << "X: " << X << " Y:" << Y << " => " << "X1: " << X1 << " Y1: " << Y1;
        
        //drawText(textObject);
        

        cairo_text_extents_t te;
        cairo_text_extents(cr, Text.c_str(), &te);
        cairo_move_to(cr, X1 + 0.5 - te.width / 2 - te.x_bearing, Y1 + 0.5 - te.height / 2 - te.y_bearing);
        cairo_show_text(cr, Text.c_str());

        //cairo_restore(cr);
    }
    return true;
}

bool OFDPage::RenderToPNGFile(const std::string& filename){
    //double dpi = default_dpi;
    double dpi = 72;
    //double pixels_per_mm = dpi / mm_per_inch;
    double mmWidth = m_attributes.PageArea.PhysicalBox.Width();
    double mmHeight = m_attributes.PageArea.PhysicalBox.Height();
    double pixelWidth = length_to_pixel(mmWidth, dpi);
    double pixelHeight = length_to_pixel(mmHeight, dpi);
    LOG(DEBUG) << "mmWidth: " << mmWidth << " mmHeight: " << mmHeight;
    LOG(DEBUG) << "pixelWidth: " << pixelWidth << " pixelHeight: " << pixelHeight;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixelWidth, pixelHeight);

    Render(surface);

    cairo_surface_write_to_png(surface, filename.c_str());

    cairo_surface_destroy(surface);
    return true;
}



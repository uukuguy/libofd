#include <iostream>
#include <sstream>
#include <iterator>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "utils.h"
#include "logger.h"

#include <tinyxml2.h>
using namespace tinyxml2;

using namespace ofd;

OFDDocument::OFDDocument(OFDPackage *ofdPackage, const std::string &filename)
    : m_ofdPackage(ofdPackage), m_filename(filename), m_opened(false), 
    m_rootDir(filename.substr(0, filename.rfind('/'))) {
        m_attributes.clear();
}


OFDDocument:: ~OFDDocument() {
    this->Close();
}

bool OFDDocument::Open() {
    if ( IsOpened() ) return true;
    if ( m_ofdPackage == NULL ) return false;

    bool ok = false;
    std::string content;
    std::tie(content, ok) = m_ofdPackage->GetFileContent(m_filename);
    if ( !ok ) return false;

    m_opened = parseXML(content);

    return IsOpened();
}

void OFDDocument::Attributes::clear() {
    CommonData.clear();
    for ( size_t i = 0 ; i < Pages.size() ; i++ ) {
        OFDPage *ofdPage = Pages[i];
        if ( ofdPage != NULL ){
            delete ofdPage;
        }
    }
    Pages.clear();
}

void OFDDocument::Close() {
    if ( !IsOpened() ) return;

    m_attributes.clear();
}

std::string OFDDocument::String() const {
    std::stringstream ss;
    ss << std::endl 
        << "------------------------------" << std::endl
        << "OFDDocument" << std::endl 
        << "PhysicalBox: " << m_attributes.CommonData.pageArea.physicalBox.x0 << ", " 
        << m_attributes.CommonData.pageArea.physicalBox.y0 << ", "
        << m_attributes.CommonData.pageArea.physicalBox.x1 << ", "
        << m_attributes.CommonData.pageArea.physicalBox.y1 << std::endl;
    ss << "PublicRes: " << m_attributes.CommonData.publicRes << std::endl;
    ss << "DocumentRes: " << m_attributes.CommonData.documentRes << std::endl;
    ss << "MaxUnitID: " << m_attributes.CommonData.maxUnitID; 
    ss << std::endl
       << "------------------------------" << std::endl;

    return ss.str();
}


bool OFDDocument::parseXML(const std::string &content){
    XMLDocument *xmldoc = new XMLDocument();
    XMLError rc = xmldoc->Parse(content.c_str());
    if ( rc != XML_SUCCESS ){
        delete xmldoc;
        return false;
    }


    XMLElement *rootElement = xmldoc->RootElement();
    if ( rootElement != NULL ){
        VLOG(3) << "Root Element Name: " << rootElement->Name();
        VLOG(3) << GetChildElements(rootElement);

        const XMLElement *commonDataElement = rootElement->FirstChildElement("ofd:CommonData");
        const XMLElement *pagesElement = rootElement->FirstChildElement("ofd:Pages");
        if ( commonDataElement != NULL ){
            VLOG(3) << GetChildElements(commonDataElement);

            const XMLElement *pageAreaElement = commonDataElement->FirstChildElement("ofd:PageArea");

            const XMLElement *physicalBoxElement = pageAreaElement->FirstChildElement("ofd:PhysicalBox");
            if ( physicalBoxElement != NULL ){
                double x0, y0, x1, y1;
                bool ok;
                std::tie(x0, y0, x1, y1, ok) = parsePhysicalBoxElement(physicalBoxElement);
                if ( ok ) {
                    this->m_attributes.CommonData.pageArea.physicalBox.x0 = x0;
                    this->m_attributes.CommonData.pageArea.physicalBox.y0 = y0;
                    this->m_attributes.CommonData.pageArea.physicalBox.x1 = x1;
                    this->m_attributes.CommonData.pageArea.physicalBox.y1 = y1;  
                }
            }

            const XMLElement *publicResElement = commonDataElement->FirstChildElement("ofd:PublicRes");
            const XMLElement *documentResElement = commonDataElement->FirstChildElement("ofd:DocumentRes");
            const XMLElement *maxUnitIDElement = commonDataElement->FirstChildElement("ofd:MaxUnitID");

            this->m_attributes.CommonData.publicRes = publicResElement->GetText();
            this->m_attributes.CommonData.documentRes = documentResElement->GetText();
            this->m_attributes.CommonData.maxUnitID = atol(maxUnitIDElement->GetText());

        }

        if ( pagesElement != NULL ) {
            VLOG(3) << GetChildElements(pagesElement);
            
            const XMLElement *pageElement = pagesElement->FirstChildElement("ofd:Page");
            while ( pageElement != NULL ) {
                uint64_t pageid = pageElement->UnsignedAttribute("ID");
                std::string baseLoc = pageElement->Attribute("BaseLoc");
                VLOG(3) << "Page ID: "  << pageid << " BaseLoc: " << baseLoc; 

                OFDPage *ofdPage = new OFDPage(this, pageid, m_rootDir + "/" + baseLoc);
                m_attributes.Pages.push_back(ofdPage);

                pageElement = pageElement->NextSiblingElement("ofd:Page");
            }
            LOG(INFO) << GetPagesCount() << " pages in document.";
        }

    } else {
        LOG(ERROR) << "rootElement == NULL";
    }

    delete xmldoc;
    xmldoc = NULL;

    return true;
}

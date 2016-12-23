#include <sstream>
#include <iomanip>
#include <tuple>
#include "OFDFile.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "utils/logger.h"
#include "utils/xml.h"
#include "utils/uuid.h"
#include "utils/utils.h"

using namespace ofd;

CT_DocInfo::CT_DocInfo() : DocUsage(DocInfo::Usage::Normal){
    DocID = generate_uuid32();
}

// **************** class OFDDocument::ImplCls ****************

class OFDDocument::ImplCls{
public:
    ImplCls(OFDFile *ofdFile, OFDDocument *ofdDocument, const std::string &docRoot);
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    OFDPagePtr AddNewPage();
    std::string GenerateDocBodyXML() const;
    std::string GenerateDocumentXML() const;

    bool FromDocBodyXML(XMLElementPtr docBodyElement);
    bool FromDocumentXML(const std::string &strDocumentXML);

private:
    bool FromDocInfoXML(XMLElementPtr docInfoElement);

    bool FromCommonDataXML(XMLElementPtr commonDataElement);
    bool FromPagesXML(XMLElementPtr pagesElement);

    // -------- Private Attributes --------
public:
    OFDFile *m_ofdFile;
    OFDDocument *m_ofdDocument;
    bool m_opened;

    DocBody    m_docBody;
    CommonData m_commonData;
    std::vector<OFDPagePtr> m_pages;

private:

    void generateCommonDataXML(XMLWriter &writer) const;
    void generatePagesXML(XMLWriter &writer) const;

}; // class OFDDocument::ImplCls

OFDDocument::ImplCls::ImplCls(OFDFile *ofdFile, OFDDocument *ofdDocument, const std::string &docRoot) : 
    m_ofdFile(ofdFile), m_ofdDocument(ofdDocument), m_opened(false){
    m_docBody.DocRoot = docRoot;
    m_commonData.PublicRes = std::make_shared<OFDRes>(std::shared_ptr<OFDFile>(ofdFile), "PublicRes.xml");
    m_commonData.DocumentRes = std::make_shared<OFDRes>(std::shared_ptr<OFDFile>(ofdFile), "DocumentRes.xml");
}

OFDDocument::ImplCls::~ImplCls(){
}

std::string OFDDocument::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDDocument ========\n";
    ss << "Pages: " << m_pages.size() << "\n";
    ss << std::endl;
    return ss.str();
}

bool OFDDocument::ImplCls::Open(){
    if ( m_opened ) return true;

    if ( m_commonData.PublicRes != nullptr ){
        std::string strResXML;
        std::tie(strResXML, std::ignore) = m_ofdFile->ReadZipFileString(m_docBody.DocRoot + "/" + m_commonData.PublicRes->GetResDescFile());
        if ( !m_commonData.PublicRes->FromResXML(strResXML) ){
            LOG(ERROR) << "m_commonData.PublicRes.FromResXML() failed.";
            return false;
        }
    }

    if ( m_commonData.DocumentRes != nullptr ){
        std::string strResXML;
        std::tie(strResXML, std::ignore) = m_ofdFile->ReadZipFileString(m_docBody.DocRoot + "/ " + m_commonData.DocumentRes->GetResDescFile());
        if ( !m_commonData.DocumentRes->FromResXML(strResXML) ){
            LOG(ERROR) << "m_commonData.DocumentRes.FromResXML() failed.";
            return false;
        }
    }

    return m_opened;
}

void OFDDocument::ImplCls::Close(){
    if ( !m_opened ) return;
}

OFDPagePtr OFDDocument::ImplCls::AddNewPage(){
    OFDPagePtr page = std::make_shared<OFDPage>(m_ofdDocument);
    page->SetID(m_pages.size());
    m_pages.push_back(page);
    return page;
}

std::string OFDDocument::ImplCls::GenerateDocBodyXML() const{

    XMLWriter writer;

    writer.StartDocument();

    // -------- <DocInfo> 
    // OFD (section 7.4) P7. OFD.xsd.
    // Required.
    writer.StartElement("DocInfo");{

        const CT_DocInfo &docInfo = m_docBody.DocInfo;

        // -------- <DocID>
        // Required.
        writer.WriteElement("DocID", docInfo.DocID);

        // -------- <Title>
        if ( !docInfo.Title.empty() ){
            writer.WriteElement("Title", docInfo.Title);
        }
        // -------- <Author>
        if ( !docInfo.Author.empty() ){
            writer.WriteElement("Author", docInfo.Author);
        }
        // -------- <Subject>
        if ( !docInfo.Subject.empty() ){
            writer.WriteElement("Subject", docInfo.Subject);
        }
        // -------- <Abstract>
        if ( !docInfo.Abstract.empty() ){
            writer.WriteElement("Abstract", docInfo.Abstract);
        }
        // -------- <CreationDate>
        if ( !docInfo.CreationDate.empty() ){
            writer.WriteElement("CreationDate", docInfo.CreationDate);
        }
        // -------- <ModDate>
        if ( !docInfo.ModDate.empty() ){
            writer.WriteElement("ModDate", docInfo.ModDate);
        }
        // FIXME
        // -------- <DocUsage>
        //writer.WriteElement("DocUsage", docInfo.DocUsage);
        // -------- <Cover>
        if ( !docInfo.Cover.empty() ){
            writer.WriteElement("Cover", docInfo.Cover);
        }
        // -------- <Keywords>
        if ( docInfo.Keywords.size() > 0 ){
            writer.StartElement("Keywords");{
                for ( auto keyword : docInfo.Keywords ){
                    writer.WriteElement("Keyword", keyword);
                }
            } writer.EndElement();
        }
        // -------- <Creator>
        if ( !docInfo.Creator.empty() ){
            writer.WriteElement("Creator", docInfo.Creator);
        }
        // -------- <CreatorVersion>
        if ( !docInfo.CreatorVersion.empty() ){
            writer.WriteElement("CreatorVersion", docInfo.CreatorVersion);
        }
        // -------- <CustomDatas>
        if ( docInfo.CustomDatas.size() > 0 ){
            writer.StartElement("CustomDatas");{

                for ( auto customData : docInfo.CustomDatas ){
                    const std::string &name = customData.first;
                    const std::string &value = customData.second;
                    writer.StartElement("CustomData");{

                        writer.WriteAttribute("Name", name);
                        writer.WriteString(value);

                    } writer.EndElement();
                }

            } writer.EndElement();
        }

    } writer.EndElement();

    // -------- <DocRoot> 
    const ST_Loc &docRoot = m_docBody.DocRoot;
    if ( !docRoot.empty() ){
        writer.WriteElement("DocRoot", docRoot);
    }

    // TODO
    // -------- <Versions> 
    const DocBody::VersionsList &versions = m_docBody.Versions;
    if ( versions.size() > 0 ){
        writer.StartElement("Versions");

        writer.EndElement();
    }

    // TODO
    // -------- <Signatures> 
    if ( !m_docBody.Signatures.empty() ){
        writer.WriteElement("Signatures", m_docBody.Signatures);
    }

    writer.EndDocument();

    return writer.GetString();
}

// OFD (section 7.5) P11. Definitions.xsd
void writePageAreaXML(XMLWriter &writer, const CT_PageArea &pageArea){
    // -------- <PhysicalBox> 
    // Required.
    writer.WriteElement("PhysicalBox", pageArea.PhysicalBox.to_xmlstring());

   
    // -------- <ApplicationBox>
    // Optional.
    if ( pageArea.HasApplicationBox() ){
        writer.WriteElement("ApplicationBox", pageArea.ApplicationBox.to_xmlstring());
    }

    // -------- <ContentBox>
    // Optional.
    if ( pageArea.HasContentBox() ){
        writer.WriteElement("ContentBox", pageArea.ContentBox.to_xmlstring());
    }
    
    // -------- <BleedBox>
    // Optional.
    if ( pageArea.HasBleedBox() ){
        writer.WriteElement("BleedBox", pageArea.BleedBox.to_xmlstring());
    }
}

// Called by OFDDocument::GenerateDocumentXML().
void OFDDocument::ImplCls::generateCommonDataXML(XMLWriter &writer) const{

    // -------- <CommonData> 
    // OFD (section 7.5) P10. OFD.xsd
    // Required.
    writer.StartElement("CommonData");{

        // -------- <MaxUnitID> 
        // Required.
        writer.WriteElement("MaxUnitID", m_commonData.MaxUnitID);

        // -------- <PageArea> 
        // Required.
        const CT_PageArea &pageArea = m_commonData.PageArea;
        writer.StartElement("PageArea");{

            writePageAreaXML(writer, pageArea);

        } writer.EndElement();

        // -------- <PublicRes>
        // Optional.
        if ( m_commonData.PublicRes != nullptr ){
            writer.WriteElement("PublicRes", m_commonData.PublicRes->GetResDescFile());
        }

        // -------- <DocumentRes>
        // Optional.
        if ( m_commonData.DocumentRes != nullptr ){
            writer.WriteElement("DocumentRes", m_commonData.DocumentRes->GetResDescFile());
        }

        // TODO
        // -------- <TemplatePage>
        // Optional

        // TODO
        // -------- <DefaultCS>
        // Optional


    } writer.EndElement();
}

// Called by OFDDocument::GenerateDocumentXML().
void OFDDocument::ImplCls::generatePagesXML(XMLWriter &writer) const{

    // -------- <Pages> 
    // OFD (section 7.6) P17. Page.xsd.
    // Required.
    writer.StartElement("Pages");{
        size_t idx = 0;
        for ( auto page : m_pages ){
            uint64_t pageID = page->GetID();
            // -------- <Page>
            // Required.
            writer.StartElement("Page");{

                // -------- <Page ID="">
                // Required
                writer.WriteAttribute("ID", pageID);

                // -------- <Page BaseLoc="">
                // Required.
                writer.WriteAttribute("BaseLoc", std::string("Pages/Page_") + std::to_string(idx));

                idx++;
            } writer.EndElement();
        }
    } writer.EndElement();
}

std::string OFDDocument::ImplCls::GenerateDocumentXML() const{

    XMLWriter writer(true);

    writer.StartDocument();

    writer.StartElement("Document");{
        OFDXML_HEAD_ATTRIBUTES;

        // -------- <CommonData> 
        // Required.
        generateCommonDataXML(writer);    

        // -------- <Pages> 
        // Required.
        generatePagesXML(writer);

    } writer.EndElement();

    writer.EndDocument();

    //std::string strDocumentXML = writer.GetString();
    //std::cout << "***** strDocumentXML: " << strDocumentXML << std::endl;
    //return strDocumentXML;
    return writer.GetString();
}


// OFD (section 7.4) P7. OFD.xsd
bool OFDDocument::ImplCls::FromDocInfoXML(XMLElementPtr docInfoElement){
    bool ok = true;
    
    CT_DocInfo &docInfo = m_docBody.DocInfo;

    XMLElementPtr childElement = docInfoElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        // -------- <DocID>
        // Optional.
        if ( childName == "DocID" ){
            std::tie(docInfo.DocID, std::ignore) = childElement->GetStringValue();
            LOG(DEBUG) << "DocID: " << docInfo.DocID;

        // -------- <Title>
        // Optional.
        } else if ( childName == "Title" ){
            std::tie(docInfo.Title, std::ignore) = childElement->GetStringValue();
            LOG(DEBUG) << "Title: " << docInfo.Title;

        // -------- <Author>
        // Optional.
        } else if ( childName == "Author" ){
            std::tie(docInfo.Author, std::ignore) = childElement->GetStringValue();

        // -------- <Subject>
        // Optional.
        } else if ( childName == "Subject" ){
            std::tie(docInfo.Subject, std::ignore) = childElement->GetStringValue();

        // -------- <Abstract>
        // Optional.
        } else if ( childName == "Abstract" ){
            std::tie(docInfo.Abstract, std::ignore) = childElement->GetStringValue();

        // -------- <CreationDate>
        // Optional.
        } else if ( childName == "CreationDate" ){
            std::tie(docInfo.CreationDate, std::ignore) = childElement->GetStringValue();

        // -------- <ModDate>
        // Optional.
        } else if ( childName == "ModDate" ){
            std::tie(docInfo.ModDate, std::ignore) = childElement->GetStringValue();

        // FIXME
        // -------- <DocUsage>
        // Optional
        //} else if ( childName == "DocUsage" ){
        //

        // -------- <Cover>
        // Optional
        } else if ( childName == "Cover" ){
            std::tie(docInfo.Cover, std::ignore) = childElement->GetStringValue();

        // -------- <Keywords>
        // Optional.
        } else if ( childName == "Keywords" ){ 
            docInfo.Keywords.clear();
            XMLElementPtr keyWordElement = childElement->GetFirstChildElement();
            while ( keyWordElement != nullptr ){
                std::string keyword;
                std::tie(keyword, std::ignore) = keyWordElement->GetStringValue();
                keyWordElement = keyWordElement->GetNextSiblingElement();
                docInfo.Keywords.push_back(keyword);
            }

        // -------- <Creator>
        // Optional.
        } else if ( childName == "Creator" ){ 
            std::tie(docInfo.Creator, std::ignore) = childElement->GetStringValue();

        // -------- <CreatorVersion>
        // Optional
        } else if ( childName == "CreatorVersion" ){ 
            std::tie(docInfo.CreatorVersion, std::ignore) = childElement->GetStringValue();

        // -------- <CustomDatas>
        // Optional
        } else if ( childName == "CustomDatas" ){ 
            docInfo.CustomDatas.clear();
            XMLElementPtr customDataElement = childElement->GetFirstChildElement();
            while ( customDataElement != nullptr ){
                bool exist = false;
                std::string name;
                std::tie(name, exist) = customDataElement->GetStringAttribute("Name");
                if ( exist ){
                    std::string value;
                    std::tie(value, std::ignore) = customDataElement->GetStringValue();
                    docInfo.CustomDatas[name] = value;
                }

                customDataElement = customDataElement->GetNextSiblingElement();
            }

        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;


    //if ( reader.EnterChildElement("DocInfo") ){
        //while ( reader.HasElement() ){

            //// -------- <DocID>
            //if ( reader.CheckElement("DocID") ){
                //std::string docID;
                //reader.ReadElement(docID);
                //LOG(DEBUG) << "DocID: " << docID;

            //// -------- <DocRoot>
            //} else if ( reader.CheckElement("Title") ){
                //std::string title;
                //reader.ReadElement(title);
                //LOG(DEBUG) << "Title: " << title;
            //}

            //reader.NextElement();
        //};
        //reader.BackParentElement();
    //} 

    return ok;
}

// OFD (section 7.4) P7. OFD.xsd.
bool OFDDocument::ImplCls::FromDocBodyXML(XMLElementPtr docBodyElement){
    bool ok = true;

    XMLElementPtr childElement = docBodyElement->GetFirstChildElement();
    bool hasDocInfo = false;
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        // -------- <DocInfo>
        // Required.
        if ( childName == "DocInfo"){
            hasDocInfo = true;
            FromDocInfoXML(childElement);

        // -------- <DocRoot>
        // Optional.
        } else if ( childName == "DocRoot" ){
            std::string docRoot;
            std::tie(docRoot, std::ignore) = childElement->GetStringValue();
            LOG(DEBUG) << "DocRoot: " << docRoot;

        // TODO
        // -------- <Versions>
        // Optional.
        //} else if ( childName == "Versions" ){

        // TODO
        // -------- <Signatures>
        // Optional.
        //} else if ( childName == "Signatures" ){

        }

        childElement = childElement->GetNextSiblingElement();
    }

    if ( !hasDocInfo ){
        LOG(ERROR) << "No DocInfo element in DocBody.";
    }

    //if ( reader.EnterChildElement("DocBody") ){
        //while ( reader.HasElement() ){
            //// -------- <DocInfo>
            //if ( reader.CheckElement("DocInfo") ){
                //FromDocInfoXML(reader);

            //// -------- <DocRoot>
            //} else if ( reader.CheckElement("DocRoot") ){
                //std::string content;
                //reader.ReadElement(content);
                //LOG(DEBUG) << "DocRoot: " << content;

            //// -------- <Versions>
            //} else if ( reader.CheckElement("Versions") ){

            //// -------- <Signatures>
            //} else if ( reader.CheckElement("Signatures") ){
            //}

            //reader.NextElement();
        //};
        //reader.BackParentElement();
    //} 

    return ok;
}

// ======== OFDDocument::ImplCls::FromDocumentXML() ========
// OFD (section 7.5) P9. Document.xsd
bool OFDDocument::ImplCls::FromDocumentXML(const std::string &strDocumentXML){
    bool ok = true;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strDocumentXML);
    if ( rootElement != nullptr ){
        std::string rootName = rootElement->GetName();
        if ( rootName == "Document" ){
            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                // -------- <CommonData>
                // OFD (section 7.5) P10. Document.xsd
                // Required.
                if ( childName == "CommonData" ) {
                    FromCommonDataXML(childElement);

                // -------- <Pages>
                // OFD (section 7.6) P17. Document.xsd
                // Required.
                } else if ( childName == "Pages" ) {
                    FromPagesXML(childElement);

                //// TODO
                //// -------- <Outlines>
                //// OFD (section 7.8) P22. Document.xsd
                //// Optional.
                //} else if ( childName == "Outlines" ) {
                    //FromOutlinesXML(childElement);

                // TODO
                // -------- <Permissions>
                // OFD (section 7.5) P13. Document.xsd
                // Optional.
                /*} else if ( childName == "Permissions" ) {*/
                /*FromPermissionsXML(childElement);*/

                // TODO
                // -------- <Actions>
                // OFD (section 14.1) P73. Document.xsd
                // Optional.
                /*} else if ( childName == "Actions" ) {*/
                /*FromActionsXML(childElement);*/

                // TODO
                // -------- <VPreferences>
                // OFD (section 7.5) P15. Document.xsd
                // Optional.
                /*} else if ( childName == "VPreferences" ) {*/
                /*FromVPreferencesXML(childElement);*/

                // TODO
                // -------- <Bookmarks>
                // OFD (section 7.5) P17. Document.xsd
                // Optional.
                /*} else if ( childName == "Bookmarks" ) {*/
                /*FromBookmarksXML(childElement);*/

                // TODO
                // -------- <Attachments>
                // OFD (section 20) P88. Document.xsd
                // Optional.
                /*} else if ( childName == "Attachments" ) {*/
                /*FromAttachmentsXML(childElement);*/

                // TODO
                // -------- <CustomTags>
                // OFD (section 16) P80. Document.xsd
                // Optional.
                /*} else if ( childName == "CustomTags" ) {*/
                /*FromCustomTagsXML(childElement);*/

                // TODO
                // -------- <Extensions>
                // OFD (section 17) P81. Document.xsd
                // Optional.
                /*} else if ( childName == "Extensions" ) {*/
                /*FromExtensionsXML(childElement);*/

                }

                childElement = childElement->GetNextSiblingElement();
            }
        } else {
            LOG(ERROR) << "Root element in Document Content.xml is not named 'Document'";
        }
    } else {
        LOG(ERROR) << "No root element in Document Content.xml";
    }

    return ok;
}

// Defined in OFDPage.cc
std::tuple<CT_PageArea,bool> FromPageAreaXML(XMLElementPtr pageAreaElement);

// -------- <CommonData>
// OFD (section 7.5) P10. Document.xsd
// Required.
bool OFDDocument::ImplCls::FromCommonDataXML(XMLElementPtr commonDataElement){
    bool ok = true;

    XMLElementPtr childElement = commonDataElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        if ( childName == "MaxUnitID" ){
            // -------- <MaxUnitID>
            // Required.

        } else if ( childName == "PageArea" ){
            // -------- <PageArea>
            // OFD (section 7.5) P11. Definitions.xsd
            // Required.
            std::tie(m_commonData.PageArea, ok) = FromPageAreaXML(childElement);
            LOG(DEBUG) << "CommonData.PageArea = " << m_commonData.PageArea.to_string();

        } else if ( childName == "PublicRes" ){
            // -------- <PublicRes>
            // Optional.
            std::string res;
            std::tie(res, std::ignore) = childElement->GetStringValue();
            if ( !res.empty() ){
                m_commonData.PublicRes = std::make_shared<OFDRes>(std::shared_ptr<OFDFile>(m_ofdFile), res);
            }

        } else if ( childName == "DocumentRes" ){
            // -------- <DocumentRes>
            // Optional.
            std::string res;
            std::tie(res, std::ignore) = childElement->GetStringValue();
            if ( !res.empty() ){
                m_commonData.DocumentRes = std::make_shared<OFDRes>(std::shared_ptr<OFDFile>(m_ofdFile), res);
            }

        //} else if ( childName == "TemplatePage" ){
            //// TODO
            //// -------- <TemplatePage>
            //// Optional.

        //} else if ( childName == "DefaultCS" ){
            //// TODO
            //// -------- <DefaultCS>
            //// Optional.

        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// -------- <Pages>
// OFD (section 7.6) P17. Document.xsd
// Required.
bool OFDDocument::ImplCls::FromPagesXML(XMLElementPtr pagesElement){
    bool ok = true;


    XMLElementPtr childElement = pagesElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        if ( childName == "Page" ){
            // -------- <Page>
            // OFD (section 7.7) P18. Page.xsd
            // Required.
            uint64_t pageID = 0;
            bool exist = false;
            std::tie(pageID, exist) = childElement->GetIntAttribute("ID");
            if ( !exist ){
                LOG(ERROR) << "Attribute ID is required in Document.xsd";
                return false;
            }

            std::string baseLoc;
            std::tie(baseLoc, exist) = childElement->GetStringAttribute("BaseLoc");
            if ( !exist ){
                LOG(ERROR) << "Attribute BaseLoc is required in Document.xsd";
                return false;
            }

            LOG(DEBUG) << "PageID: " << pageID << " BaseLoc: " << baseLoc;

            OFDPagePtr page = AddNewPage();
            page->SetID(pageID);
            page->SetBaseLoc(baseLoc);
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// **************** class OFDDocument ****************

OFDDocument::OFDDocument(OFDFile *ofdFile, const std::string &docRoot){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(ofdFile, this, docRoot));
}

OFDDocument::~OFDDocument(){
}

const OFDFile *OFDDocument::GetOFDFile() const{
    return m_impl->m_ofdFile;
}

const DocBody& OFDDocument::GetDocBody() const{
    return m_impl->m_docBody;
}

DocBody& OFDDocument::GetDocBody(){
    return m_impl->m_docBody;
}

std::string OFDDocument::GetDocRoot() const {
    const DocBody& docBody = GetDocBody();
    return docBody.DocRoot;
}

void OFDDocument::SetDocRoot(const std::string &docRoot){
    DocBody& docBody = GetDocBody();
    docBody.DocRoot = docRoot;
}

const OFDDocument::CommonData& OFDDocument::GetCommonData() const{
    return m_impl->m_commonData;
}

OFDDocument::CommonData& OFDDocument::GetCommonData(){
    return m_impl->m_commonData;
}

size_t OFDDocument::GetPagesCount() const{
    return m_impl->m_pages.size();
}

const OFDPagePtr OFDDocument::GetPage(size_t idx) const{
    return m_impl->m_pages[idx];
}

OFDPagePtr OFDDocument::GetPage(size_t idx){
    return m_impl->m_pages[idx];
}

OFDPagePtr OFDDocument::AddNewPage(){
    return m_impl->AddNewPage();
}

std::string OFDDocument::GenerateDocBodyXML() const{
    return m_impl->GenerateDocBodyXML();
}

std::string OFDDocument::GenerateDocumentXML() const{
    return m_impl->GenerateDocumentXML();
}

bool OFDDocument::FromDocBodyXML(XMLElementPtr docBodyElement){
    return m_impl->FromDocBodyXML(docBodyElement);
}

bool OFDDocument::FromDocumentXML(const std::string &strDocumentXML){
    return m_impl->FromDocumentXML(strDocumentXML);
}

std::string OFDDocument::to_string() const {
    return m_impl->to_string();
}

bool OFDDocument::Open() {
    return m_impl->Open();
}

void OFDDocument::Close(){
    m_impl->Close();
}

bool OFDDocument::IsOpened() const{
    return m_impl->m_opened;
}


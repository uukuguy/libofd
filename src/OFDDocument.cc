#include <sstream>
#include <iomanip>
#include <tuple>
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
    ImplCls(const std::string &docRoot);
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    OFDPagePtr AddNewPage();
    std::string GenerateDocBodyXML() const;
    std::string GenerateDocumentXML() const;

    bool FromDocBodyXML(XMLReader &reader);
    bool FromDocumentXML(const std::string &strDocumentXML);

private:
    bool FromDocInfoXML(XMLReader &reader);

    bool FromCommonDataXML(XMLReader &reader);
    bool FromPagesXML(XMLReader &reader);

    // -------- Private Attributes --------
public:
    bool m_opened;

    DocBody    m_docBody;
    CommonData m_commonData;
    std::vector<OFDPagePtr> m_pages;

private:

    void generateCommonDataXML(XMLWriter &writer) const;
    void generatePagesXML(XMLWriter &writer) const;

}; // class OFDDocument::ImplCls

OFDDocument::ImplCls::ImplCls(const std::string &docRoot) : m_opened(false) {
    m_docBody.DocRoot = docRoot;
    m_commonData.PublicRes.push_back("PublicRes.xml");
    m_commonData.DocumentRes.push_back("DocumentRes.xml");
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

    return m_opened;
}

void OFDDocument::ImplCls::Close(){
    if ( !m_opened ) return;
}

OFDPagePtr OFDDocument::ImplCls::AddNewPage(){
    OFDPagePtr page = std::make_shared<OFDPage>();
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


void writeBoxXML(XMLWriter &writer, const std::string &boxName, const ST_Box &box){
    std::stringstream ssBox;
    ssBox << std::setprecision(3) << box.Left << " " << box.Top << " " << box.Width << " " << box.Height; 
    writer.WriteElement(boxName, ssBox.str());
}

// OFD (section 7.5) P11. Definitions.xsd
void writePageAreaXML(XMLWriter &writer, const CT_PageArea &pageArea){
    // -------- <PhysicalBox> 
    // Required.
    writeBoxXML(writer, "PhysicalBox", pageArea.PhysicalBox);
   
    // -------- <ApplicationBox>
    // Optional.
    if ( pageArea.HasApplicationBox() ){
        writeBoxXML(writer, "ApplicationBox", pageArea.ApplicationBox);
    }

    // -------- <ContentBox>
    // Optional.
    if ( pageArea.HasContentBox() ){
        writeBoxXML(writer, "ContentBox", pageArea.ContentBox);
    }
    
    // -------- <BleedBox>
    // Optional.
    if ( pageArea.HasBleedBox() ){
        writeBoxXML(writer, "BleedBox", pageArea.BleedBox);
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
        if ( m_commonData.PublicRes.size() > 0 ){
            for ( auto pr : m_commonData.PublicRes ){
                writer.WriteElement("PublicRes", pr);
            }
        }

        // -------- <DocumentRes>
        // Optional.
        if ( m_commonData.DocumentRes.size() > 0 ){
            for ( auto dr : m_commonData.DocumentRes ){
                writer.WriteElement("DocumentRes", dr);
            }
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
                writer.WriteAttribute("ID", std::to_string(pageID));

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


bool OFDDocument::ImplCls::FromDocInfoXML(XMLReader &reader){
    bool ok = true;

    if ( reader.EnterChildElement("DocInfo") ){
        while ( reader.HasElement() ){

            // -------- <DocID>
            if ( reader.CheckElement("DocID") ){
                std::string docID;
                reader.ReadElement(docID);
                LOG(INFO) << "DocID: " << docID;

            // -------- <DocRoot>
            } else if ( reader.CheckElement("Title") ){
                std::string title;
                reader.ReadElement(title);
                LOG(INFO) << "Title: " << title;
            }

            reader.NextElement();
        };
    } reader.BackParentElement();

    return ok;
}

bool OFDDocument::ImplCls::FromDocBodyXML(XMLReader &reader){
    bool ok = true;

    if ( reader.EnterChildElement("DocBody") ){
        while ( reader.HasElement() ){

            // -------- <DocInfo>
            if ( reader.CheckElement("DocInfo") ){
                FromDocInfoXML(reader);

            // -------- <DocRoot>
            } else if ( reader.CheckElement("DocRoot") ){
                std::string content;
                reader.ReadElement(content);
                LOG(INFO) << "DocRoot: " << content;

            // -------- <Versions>
            } else if ( reader.CheckElement("Versions") ){

            // -------- <Signatures>
            } else if ( reader.CheckElement("Signatures") ){
            }

            reader.NextElement();
        };
    } reader.BackParentElement();

    return ok;
}

// ======== OFDDocument::ImplCls::FromDocumentXML() ========
// OFD (section 7.5) P9. Document.xsd
bool OFDDocument::ImplCls::FromDocumentXML(const std::string &strDocumentXML){
    bool ok = true;

    XMLReader reader;
    if ( reader.ParseXML(strDocumentXML) ){

        if ( reader.CheckElement("Document") ){
            if ( reader.EnterChildElement("Document") ){

                while ( reader.HasElement() ){

                    // -------- <CommonData>
                    // OFD (section 7.5) P10. Document.xsd
                    // Required.
                    if ( reader.CheckElement("CommonData") ) {
                        FromCommonDataXML(reader);

                    // -------- <Pages>
                    // OFD (section 7.6) P17. Document.xsd
                    // Required.
                    } else if ( reader.CheckElement("Pages") ) {
                        FromPagesXML(reader);
                    }

                    reader.NextElement();
                }

            } reader.BackParentElement();
        }

        ok = true;
    }

    return ok;
}

// OFD (section 7.5) P11. Definitions.xsd
std::tuple<CT_PageArea,bool> FromPageAreaXML(XMLReader &reader){
    bool ok = false;
    CT_PageArea pageArea;

    if ( reader.EnterChildElement("PageArea") ){
        while ( reader.HasElement() ){


            // -------- <PhysicalBox>
            if ( reader.CheckElement("PhysicalBox") ) {
                if ( reader.EnterChildElement("PhysicalBox") ){
                    std::string boxString;
                    if ( reader.ReadElement(boxString) && !boxString.empty()){
                        std::vector<std::string> tokens = utils::SplitString(boxString);
                        if ( tokens.size() >= 4 ){
                            pageArea.PhysicalBox.Left = atof(tokens[0].c_str());
                            pageArea.PhysicalBox.Top = atof(tokens[1].c_str());
                            pageArea.PhysicalBox.Width = atof(tokens[2].c_str());
                            pageArea.PhysicalBox.Height = atof(tokens[3].c_str());
                            ok = true;
                        } else {
                            LOG(ERROR) << "Box String tokens size >= 4 failed. boxString:" << boxString;
                        }
                    }
                } reader.BackParentElement();

            // -------- <ApplicationBox>
            } else if ( reader.CheckElement("AppicationBox") ){

            // -------- <ContentBox>
            } else if ( reader.CheckElement("ContentBox") ){

            // -------- <BleedBox>
            } else if ( reader.CheckElement("BleedBox") ){

            }

            reader.NextElement();
        };
    } reader.BackParentElement();

    return std::make_tuple(pageArea, ok);
}

// -------- <CommonData>
// OFD (section 7.5) P10. Document.xsd
// Required.
bool OFDDocument::ImplCls::FromCommonDataXML(XMLReader &reader){
    bool ok = true;

    LOG(INFO) << "Enter FromCommonDataXML()";
    if ( reader.EnterChildElement("CommonData") ){
        while ( reader.HasElement() ){

            // -------- <MaxUnitID>
            // Required.
            if ( reader.CheckElement("MaxUnitID") ){

            // -------- <PageArea>
            // OFD (section 7.5) P11. Definitions.xsd
            // Required.
            } else if ( reader.CheckElement("PageArea") ){
                std::tie(m_commonData.PageArea, ok) = FromPageAreaXML(reader);
                LOG(INFO) << "CommonData.PageArea = " << m_commonData.PageArea.to_string();
            }

            reader.NextElement();
        };
    } reader.BackParentElement();

    return ok;
}

// -------- <Pages>
// OFD (section 7.6) P17. Document.xsd
// Required.
bool OFDDocument::ImplCls::FromPagesXML(XMLReader &reader){
    bool ok = true;

    if ( reader.EnterChildElement("Pages") ){
        uint64_t pageID = 0;
        reader.ReadAttribute("ID", pageID);
        std::string baseLoc;
        reader.ReadAttribute("BaseLoc", baseLoc);
        LOG(INFO) << "PageID: " << pageID << " BaseLoc: " << baseLoc;

        while ( reader.HasElement() ){

            // -------- <Page>
            // OFD (section 7.7) P18. Page.xsd
            // Required.
            if ( reader.CheckElement("Page") ){

            }

            reader.NextElement();
        };
    } reader.BackParentElement();

    return ok;
}

// **************** class OFDDocument ****************

OFDDocument::OFDDocument(const std::string &docRoot){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(docRoot));
}

OFDDocument::~OFDDocument(){
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

bool OFDDocument::FromDocBodyXML(XMLReader &reader){
    return m_impl->FromDocBodyXML(reader);
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


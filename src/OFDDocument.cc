#include <sstream>
#include "logger.h"
#include "OFDDocument.h"
#include "OFDPage.h"

using namespace ofd;

// **************** class OFDDocument::ImplCls ****************

class OFDDocument::ImplCls{
public:
    ImplCls();
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    OFDPagePtr AddNewPage();

    CommonData commonData;
    std::vector<OFDPagePtr> Pages;

    // -------- Private Attributes --------
    bool m_opened;

}; // class OFDDocument::ImplCls

OFDDocument::ImplCls::ImplCls() : m_opened(false) {
}

OFDDocument::ImplCls::~ImplCls(){
}

std::string OFDDocument::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDDocument ========\n";
    ss << "Pages: " << Pages.size() << "\n";
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
    Pages.push_back(page);
    return page;
}

// **************** class OFDDocument ****************

OFDDocument::OFDDocument(){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());
}

OFDDocument::~OFDDocument(){
}

const OFDDocument::CommonData& OFDDocument::GetCommonData() const{
    return m_impl->commonData;
}

OFDDocument::CommonData& OFDDocument::GetCommonData(){
    return m_impl->commonData;
}

size_t OFDDocument::GetPagesCount() const{
    return m_impl->Pages.size();
}

const OFDPagePtr OFDDocument::GetPage(size_t idx) const{
    return m_impl->Pages[idx];
}

OFDPagePtr OFDDocument::GetPage(size_t idx){
    return m_impl->Pages[idx];
}

OFDPagePtr OFDDocument::AddNewPage(){
    return m_impl->AddNewPage();
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


#include <sstream>
#include "logger.h"
#include "OFDPage.h"

using namespace ofd;

// **************** class OFDPage::ImplCls ****************

class OFDPage::ImplCls{
public:
    ImplCls();
    ~ImplCls();

    std::string to_string() const;

    bool Open();
    void Close();

    // -------- Private Attributes --------

    bool m_opened;

}; // class OFDPage::ImplCls

OFDPage::ImplCls::ImplCls() : m_opened(false) {
}

OFDPage::ImplCls::~ImplCls(){
}

std::string OFDPage::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDPage ========\n";
    ss << std::endl;
    return ss.str();
}

bool OFDPage::ImplCls::Open(){
    if ( m_opened ) return true;

    return m_opened;
}

void OFDPage::ImplCls::Close(){
    if ( !m_opened ) return;
}

// **************** class OFDPage ****************

OFDPage::OFDPage(){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());
}

OFDPage::~OFDPage(){
}

std::string OFDPage::to_string() const{
    return m_impl->to_string();
}

bool OFDPage::Open(){
    return m_impl->Open();
}

void OFDPage::Close(){
    m_impl->Close();
}

bool OFDPage::IsOpened() const{
    return m_impl->m_opened;
}


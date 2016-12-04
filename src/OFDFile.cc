#include <sstream>
#include "OFDFile.h"
#include "OFDDocument.h"
#include "logger.h"

using namespace ofd;

// **************** class OFDFile::ImplCls ****************

class OFDFile::ImplCls{
public:
    ImplCls();
    ImplCls(const std::string &filename);
    ~ImplCls();

    std::string to_string() const;

    bool Open(const std::string &filename);
    void Close();
    bool Save(const std::string &filename);

    typedef std::vector<DocBodyPtr> DocBodiesList;

    std::string   Version;   // 文件格式的版本号，取值为”1.0“
    std::string   DocType;   // 文件格式子集类型，取值为”OFD“，表明此文件符合本标准
                             // 取值为”OFD-A“，表明此文件符合OFD存档规范
    DocBodiesList DocBodies; // 文件对象入口集合

    // -------- Private Attributes --------

    std::string m_filename;  // 包文件绝对路径
    bool m_opened;           // 包文件是否成功打开标志

}; // class OFDFile::ImplCls

OFDFile::ImplCls::ImplCls() :
    Version("1.0"), DocType("OFD"),
    m_opened(false){
}

OFDFile::ImplCls::ImplCls(const std::string &filename) :
    Version("1.0"), DocType("OFD"),
    m_filename(filename), m_opened(false){
}

OFDFile::ImplCls::~ImplCls(){
}

std::string OFDFile::ImplCls::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDFile ========\n";
    ss << std::endl;
    return ss.str();
}

bool OFDFile::ImplCls::Open(const std::string &filename){
    if ( m_opened ) return true;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;

    return m_opened;
}

void OFDFile::ImplCls::Close(){
    if ( m_opened ){
    }
}

bool OFDFile::ImplCls::Save(const std::string &filename){
    if ( !m_opened ) return false;

    bool ok = false;

    if ( !filename.empty() ) m_filename = filename;
    if ( m_filename.empty() ) return false;


    return ok;
}

// **************** class OFDFile ****************

OFDFile::OFDFile() {
    m_impl = std::unique_ptr<ImplCls>(new ImplCls());
}

OFDFile::OFDFile(const std::string &filename){
    m_impl = std::unique_ptr<ImplCls>(new ImplCls(filename));
}

OFDFile::~OFDFile(){
}

bool OFDFile::Open(const std::string &filename){
    return m_impl->Open(filename);
}

void OFDFile::Close(){
    m_impl->Close();
}

bool OFDFile::IsOpened() const{
    return m_impl->m_opened;
}


OFDDocumentPtr OFDFile::GetDocument(){
    return OFDDocumentPtr(nullptr);
}

const OFDDocumentPtr OFDFile::GetDocument() const{
    return OFDDocumentPtr(nullptr);
}

std::string OFDFile::GetVersion() const{
    return m_impl->Version;
}

std::string OFDFile::GetDocType() const{
    return m_impl->DocType;
}

size_t OFDFile::GetDocBodiesCount() const{
    return m_impl->DocBodies.size();
}

const OFDFile::DocBodyPtr OFDFile::GetDocBody(size_t idx) const{
    return m_impl->DocBodies[idx];
}

OFDFile::DocBodyPtr OFDFile::GetDocBody(size_t idx) {
    return m_impl->DocBodies[idx];
}

bool OFDFile::Save(const std::string &filename){
    return m_impl->Save(filename);
}

std::string OFDFile::to_string() const{
    return m_impl->to_string();
}


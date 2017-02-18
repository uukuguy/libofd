#ifndef __OFDDOCUMENT_H__
#define __OFDDOCUMENT_H__

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "OFDCommon.h"
#include "OFDRes.h"

//namespace utils{
    //class XMLElement;
    //typedef std::shared_ptr<XMLElement> XMLElementPtr;
//};
//using namespace utils;

//namespace ofd{

    //namespace DocInfo{
        //// 文档分类
        //enum class Usage{
            //Normal,     // 普通文档
            //EBook,      // 电子书
            //ENewsPaper, // 电子报纸
            //EMagzine,   // 电子杂志
        //};
        //__attribute__((unused)) static std::string GetUsageLabel(Usage usage){
            //if ( usage == Usage::Normal ){
                //return "Normal";
            //} else if ( usage == Usage::EBook ){
                //return "EBook";
            //} else if ( usage == Usage::ENewsPaper ){
                //return "ENewsPaper";
            //} else if ( usage == Usage::EMagzine ){
                //return "EMagzine";
            //}
            //return "";
        //}
    //}; // namespace DocInfo

    //// ======== struct CT_DocInfo ========
    //// 文档元数据定义。
    //// 标准 P7 页，OFD.xsd。
    //typedef struct CT_DocInfo{
        
        //typedef std::vector<std::string> KeywordsList;
        //typedef std::unordered_map<std::string, std::string> CustomDataMap;

        //std::string    DocID;          // 采用UUID算法生成的由32个字节组成的文件标识。
                                       //// 每个DocID在文档创建或生成的时候进行分配。
        //std::string    Title;          // 文档标题，标题可以与文件名不同 
        //std::string    Author;         // 文档作者
        //std::string    Subject;        // 文档主题
        //std::string    Abstract;       // 文档摘要与注释
        //ST_TIME        CreationDate;   // 文档的创建日期
        //ST_TIME        ModDate;        // 文档的最近修改日期
        //DocInfo::Usage DocUsage;       // 文档分类
        //ST_Loc         Cover;          // 文档封面，此路径指向一个图片文件。
        //KeywordsList   Keywords;       // 关键词集合
        //std::string    Creator;        // 创建文档的应用程序。
        //std::string    CreatorVersion; // 创建文档的应用程序的版本信息。
        //CustomDataMap  CustomDatas;     // 用户自定义数据

        //CT_DocInfo();

    //} CT_DocInfo_t;

    //// ******** struct DocBody ********
    //// 文件对象入口，可以存在多个，以便在一个文档中包含多个版式文件。
    //typedef struct DocBody{
    //public:

        //class Version{
        //public:
            //std::string ID;      // 版本标识
            //int         Index;   // 版本号
            //bool        Current; // 是否默认版本，默认值为false。
            //ST_Loc      BaseLoc; // 指向包内的版本描述文件。

            //Version() : Current(false){};
        //};
        //typedef std::vector<Version> VersionsList;

        //CT_DocInfo   DocInfo;    // 文档元数据信息描述。
        //ST_Loc       DocRoot;    // 指向文档根节点文档。
        //VersionsList Versions;   // 包含多个版本描述节点，
                                 //// 用于定义文件因注释和其它改动产生的版本信息。
        //ST_Loc       Signatures; // 指向文档中签名和签章结构。

    //} DocBody_t;
    //typedef std::shared_ptr<DocBody> DocBodyPtr;

    //// ======== class OFDDocument ========
    //class OFDDocument : public std::enable_shared_from_this<OFDDocument> {
    //private:
        //OFDDocument(PackagePtr package, const std::string &docRoot);
    //public:
        //virtual ~OFDDocument();

        //static OFDDocumentPtr CreateNewDocument(PackagePtr package, const std::string &docRoot);

        //const PackagePtr GetPackage() const;
        //PackagePtr GetPackage();

        ////void InitRes();

        //bool Open();
        //void Close();
        //bool IsOpened() const;

        //OFDDocumentPtr GetSelf();

        //const DocBody& GetDocBody() const;
        //DocBody& GetDocBody();

        //std::string GetDocRoot() const;
        //void SetDocRoot(const std::string &docRoot);

        //// -------- CommonData --------
        //// 文档公共数据，定义了页面区域、公共资源等数据。
        //class CommonData{
        //public:
            //// 当前文档中所有对象使用标识的最大值，初始值为0。
            //// MaxUnitID主要用于文档编辑，在向文档中新增一个对象时，
            //// 需要分配一个新的标识，新标识取值宜为MaxUnitID+1，
            //// 同时需要修改此MaxUnitID值。必选。
            //ST_ID MaxUnitID;

            //// 指定该文档区域的默认大小和位置。必选。
            //CT_PageArea PageArea;

            //// 公共资源序列，每一个节点指向OFD包内的一个资源描述文档。
            ////std::vector<ST_Loc> PublicRes;
            //OFDResPtr PublicRes;

            //// 文档资源序列，每一个节点指向OFD包内的一个资源描述文档。
            ////std::vector<ST_Loc> DocumentRes;
            //OFDResPtr DocumentRes;

            //// 模板页序列，为一系列模板页的集合，模板页内容结构和普通页相同。
            ////CT_TemplatePage TemplatePage;
            
            //// 引用在资源文件中定义的颜色空间标识。如果此项不存在，采用RGB作为默认颜色空间。
            //ST_RefID DefaultCS;


            //CommonData() : MaxUnitID(0) {};

        //}; // class CommonData


        //const CommonData& GetCommonData() const;
        //CommonData& GetCommonData();

        //const FontMap& GetFonts() const;

        //const OFDResPtr GetPublicRes() const{return GetCommonData().PublicRes;};
        //OFDResPtr GetPublicRes() {return GetCommonData().PublicRes;};

        //const OFDResPtr GetDocumentRes() const{return GetCommonData().DocumentRes;};
        //OFDResPtr GetDocumentRes() {return GetCommonData().DocumentRes;};

        //size_t GetPagesCount() const;
        //const OFDPagePtr GetPage(size_t idx) const;
        //OFDPagePtr GetPage(size_t idx);

        //OFDPagePtr AddNewPage();
        //std::string GenerateDocBodyXML() const;
        //std::string GenerateDocumentXML() const;

        //bool FromDocBodyXML(utils::XMLElementPtr docBodyElement);
        //bool FromDocumentXML(const std::string &strDocumentXML);

        //std::string to_string() const;

    //private:
        //class ImplCls;
        //std::unique_ptr<ImplCls> m_impl;

    //}; // class OFDDocument
    //typedef std::shared_ptr<OFDDocument> OFDDocumentPtr;

//}; // namespace ofd

#endif // __OFDDOCUMENT_H__

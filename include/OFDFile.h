#ifndef __LIBOFD_FILE_H__
#define __LIBOFD_FILE_H__

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "OFDCommon.h"

namespace ofd{

    class OFDDocument;
    typedef std::shared_ptr<OFDDocument> OFDDocumentPtr;

    namespace DocInfo{
        // 文档分类
        enum class Usage{
            Normal,     // 普通文档
            EBook,      // 电子书
            ENewsPaper, // 电子报纸
            EMagzine,   // 电子杂志
        };
        __attribute__((unused)) static std::string GetUsageLabel(Usage usage){
            if ( usage == Usage::Normal ){
                return "Normal";
            } else if ( usage == Usage::EBook ){
                return "EBook";
            } else if ( usage == Usage::ENewsPaper ){
                return "ENewsPaper";
            } else if ( usage == Usage::EMagzine ){
                return "EMagzine";
            }
            return "";
        }
    }; // namespace DocInfo

    // 文档元数据定义。
    typedef struct CT_DocInfo{
        
        typedef std::vector<std::string> KeywordsList;
        typedef std::unordered_map<std::string, std::string> CustomDataMap;

        std::string    DocID;          // 采用UUID算法生成的由32个字节组成的文件标识。
                                       // 每个DocID在文档创建或生成的时候进行分配。
        std::string    Title;          // 文档标题，标题可以与文件名不同 
        std::string    Author;         // 文档作者
        std::string    Subject;        // 文档主题
        std::string    Abstract;       // 文档摘要与注释
        ST_TIME        CreationDate;   // 文档的创建日期
        ST_TIME        ModDate;        // 文档的最近修改日期
        DocInfo::Usage DocUsage;       // 文档分类
        ST_Loc         Cover;          // 文档封面，此路径指向一个图片文件。
        KeywordsList   Keywords;       // 关键词集合
        std::string    Creator;        // 创建文档的应用程序。
        std::string    CreatorVersion; // 创建文档的应用程序的版本信息。
        CustomDataMap  CustomData;     // 用户自定义数据

    } CT_DocInfo_t;

    class OFDFile{
    public:

        OFDFile();
        OFDFile(const std::string &filename);
        virtual ~OFDFile();

        // 打开OFD文件
        // 打开成功返回true，否则返回false。
        bool Open(const std::string &filename = "");

        // 关闭已打开的OFD文件。
        void Close();

        bool IsOpened() const;

        OFDDocumentPtr GetDocument();
        const OFDDocumentPtr GetDocument() const;

        // 文件对象入口，可以存在多个，以便在一个文档中包含多个版式文件。
        class DocBody{
        public:

            class Version{
            public:
                std::string ID;      // 版本标识
                int         Index;   // 版本号
                bool        Current; // 是否默认版本，默认值为false。
                ST_Loc      BaseLoc; // 指向包内的版本描述文件。

                Version() : Current(false){};
            };
            typedef std::vector<Version> VersionsList;

            CT_DocInfo   DocInfo;    // 文档元数据信息描述。
            ST_Loc       DocRoot;    // 指向文档根节点文档。
            VersionsList Versions;   // 包含多个版本描述节点，
                                     // 用于定义文件因注释和其它改动产生的版本信息。
            ST_Loc       Signatures; // 指向文档中签名和签章结构。

        }; // DocBody
        typedef std::shared_ptr<DocBody> DocBodyPtr;

        // 获取文件格式的版本号，取值为”1.0“
        std::string GetVersion() const;

        // 获取文件格式子集类型，取值为”OFD“
        std::string GetDocType() const;

        // 获取包文件中文件对象。
        size_t GetDocBodiesCount() const;
        const DocBodyPtr GetDocBody(size_t idx) const;
        DocBodyPtr GetDocBody(size_t idx);

        // 保存OFD包文件
        bool Save(const std::string &filename = "");

        // 返回对象摘要信息字符串。
        std::string to_string() const;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDFile

}; // namespace ofd

#endif


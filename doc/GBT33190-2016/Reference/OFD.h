#ifndef __OFD_OFD_H__
#define __OFD_OFD_H__

#include <unordered_map>
#include "Definitions.h"
#include "Signature.h"

namespace ofd {

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

        std::string DocID;          // 采用UUID算法生成的由32个字节组成的文件标识。
                                    // 每个DocID在文档创建或生成的时候进行分配。
        std::string Title;          // 文档标题，标题可以与文件名不同 
        std::string Author;         // 文档作者
        std::string Subject;        // 文档主题
        std::string Abstract;       // 文档摘要与注释
        ST_TIME     CreationDate;   // 文档的创建日期
        ST_TIME     ModDate;        // 文档的最近修改日期
        DocInfo::Usage DocUsage;    // 文档分类
        ST_Loc      Cover;          // 文档封面，此路径指向一个图片文件。
        KeywordsList Keywords;      // 关键词集合
        std::string Creator;        // 创建文档的应用程序。
        std::string CreatorVersion; // 创建文档的应用程序的版本信息。
        CustomDataMap CustomData;   // 用户自定义数据

    } CT_DocInfo_t;

    // ======== OFD ========
    class OFD {
    public:
        std::string Version; // 文件格式的版本号，取值为”1.0“
        std::string DocType; // 文件格式子集类型，取值为”OFD“，表明此文件符合本标准。
                             // 取值为”OFD-A“，表明此文件符合OFD存档规范。

        // 文件对象入口，可以存在多个，以便在一个文档中包含多个版式文件。
        class DocBody{
        public:

            CT_DocInfo DocInfo; // 文档元数据信息描述。
            ST_Loc     DocRoot; // 指向文档根节点文档。

            class Version{
            public:
                uint64_t ID;
                int      Index;
                bool     Current;
                ST_Loc   BaseLoc;

                Version() : Current(false){};
            };
            typedef std::vector<Version> VersionsList;
            VersionsList Versions;   // 包含多个版本描述节点，
                                     // 用于定义文件因注释和其它改动产生的版本信息。
            ST_Loc       Signatures; // 指向文档中签名和签章结构。

        }; // DocBody
        typedef std::vector<DocBody*> DocBodiesList;
        DocBodiesList DocBodies;     // 文件对象入口集合。

        OFD() : Version("1.0"), DocType("OFD"){
        };

    }; // class OFD

}; // namespace ofd

#endif // __OFD_OFD_H__

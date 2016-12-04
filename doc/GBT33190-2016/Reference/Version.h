#ifndef __OFD_VERSION_H__
#define __OFD_VERSION_H__

#include <string>
#include <vector>
#include "Definitions.h"

namespace ofd{

    // ======== class Versions ========
    // 版本列表
    class Versions{
    public:
        std::string ID;    // 版本标识
        int         Index; // 版本号
        bool        Current; // 是否是默认版本，默认值为false。
        ST_Loc      BaseLoc; // 指向包内的版本描述文件。

        Versions() : Current(false) {};
    }; // Versions

    // ======== class Version ========
    // 文档版本
    class DocVersion {
    public:
        std::string ID;            // 版本标识
        std::string Version;      // 该文件适用的格式版本。
        std::string Name;         // 版本名称
        ST_TIME     CreationDate; // 创建时间

        // 版本包含的文件列表。
        class FileList{
        public:
            class File{
            public:
                std::string ID; // 文件列表文件标识。 
                ST_Loc      FileLoc; // 文件包内位置。
            }; // class File
            std::vector<File> Files;
        }; // class FileList
        FileList FileList; 

        ST_Loc DocRoot;    // 该版本的入口文件。

    }; // class DocVersion

}; // namespace ofd

#endif // __OFD_VERSION_H__

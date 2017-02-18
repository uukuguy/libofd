#ifndef __OFDPACKAGE_H__
#define __OFDPACKAGE_H__

#include <vector>
#include <string>
#include <memory>
#include "OFDCommon.h"
#include "OFDDocument.h"

//namespace ofd{

    //class OFDPackage;
    //typedef std::shared_ptr<OFDPackage> OFDPackagePtr;

    //// ======== class OFDPackage ========
    //class OFDPackage : public std::enable_shared_from_this<OFDPackage> {
    //public:

        //OFDPackage();
        //OFDPackage(const std::string &filename);
        //virtual ~OFDPackage();

        //// 打开OFD文件
        //// 打开成功返回true，否则返回false。
        //bool Open(const std::string &filename = "");

        //// 关闭已打开的OFD文件。
        //void Close();

        //bool IsOpened() const;

        //const OFDDocumentPtr GetDefaultDocument() const;
        //OFDDocumentPtr GetDefaultDocument();

        //OFDPackagePtr GetSelf();

        //OFDDocumentPtr AddNewDocument();

        //// 获取包文件中文件对象。
        //size_t GetDocumentsCount() const;
        //const OFDDocumentPtr GetDocument(size_t idx) const;
        //OFDDocumentPtr GetDocument(size_t idx);


        //// 获取文件格式的版本号，取值为”1.0“
        //std::string GetVersion() const;

        //// 获取文件格式子集类型，取值为”OFD“
        //std::string GetDocType() const;

        //// 保存OFD包文件
        //bool Save(const std::string &filename = "");

        //// ======== utils ========

        //// 返回对象摘要信息字符串。
        //std::string to_string() const;

        //std::tuple<std::string, bool> ReadZipFileString(const std::string &fileinzip) const;
        //std::tuple<char*, size_t, bool> ReadZipFileRaw(const std::string &fileinzip) const;

    //private:
        //class ImplCls;
        //std::unique_ptr<ImplCls> m_impl;

    //}; // class OFDPackage

//}; // namespace ofd

#endif // __OFD_PACKAGE_H__


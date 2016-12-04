#ifndef __OFD_DOCUMENT_H__
#define __OFD_DOCUMENT_H__

#include <memory>
#include <vector>
#include <string>
#include "OFDCommon.h"

namespace ofd{

    class OFDPage;
    typedef std::shared_ptr<OFDPage> OFDPagePtr;

    class OFDDocument{
    public:
        OFDDocument();
        virtual ~OFDDocument();

        bool Open();
        void Close();
        bool IsOpened() const;

        // -------- CommonData --------
        // 文档公共数据，定义了页面区域、公共资源等数据。
        class CommonData{
        public:
            // 当前文档中所有对象使用标识的最大值，初始值为0。
            // MaxUnitID主要用于文档编辑，在向文档中新增一个对象时，
            // 需要分配一个新的标识，新标识取值宜为MaxUnitID+1，
            // 同时需要修改此MaxUnitID值。必选。
            ST_ID MaxUnitID;

            // 指定该文档区域的默认大小和位置。必选。
            CT_PageArea PageArea;

            // 公共资源序列，每一个节点指向OFD包内的一个资源描述文档。
            std::vector<ST_Loc> PublicRes;

            // 文档资源序列，每一个节点指向OFD包内的一个资源描述文档。
            std::vector<ST_Loc> DocumentRes;

            // 模板页序列，为一系列模板页的集合，模板页内容结构和普通页相同。
            //CT_TemplatePage TemplatePage;
            
            // 引用在资源文件中定义的颜色空间标识。如果此项不存在，采用RGB作为默认颜色空间。
            ST_RefID DefaultCS;


            CommonData() : MaxUnitID(0) {};

        }; // class CommonData


        const CommonData& GetCommonData() const;
        CommonData& GetCommonData();

        size_t GetPagesCount() const;
        const OFDPagePtr GetPage(size_t idx) const;
        OFDPagePtr GetPage(size_t idx);

        OFDPagePtr AddNewPage();

        std::string to_string() const;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDDocument

}; // namespace ofd

#endif // __OFD_DOCUMENT_H__

#ifndef __OFD_ANNOTATION_H__
#define __OFD_ANNOTATION_H__

// Annotations.xsd
// Annotation.xsd

#include <vector>
#include <string>
#include <unordered_map>
#include "Definitions.h"
#include "Page.h"

namespace ofd {

    // ======== class Annotations ========
    // 注释列表
    // 注释是版式文档形成后附加的图文信息，用户可通过鼠标或键盘与其进行交互。
    // 本标准中，页面内容与注释内容是分文件描述的。文档的注释在注释列表文件中按照页面
    // 进行组织索引，注释的内容在分页注释文件中描述。
    class Annotations {
    public:
        class AnnotationPage {
        public:
            ST_RefID PageID; // 引用注释所在页面的标识。
            ST_Loc FileLoc;  // 指向包内的分布注释文件。
        }; // class Page
        std::vector<AnnotationPage> AnnotationPages;
    }; // class Annotations

    namespace Annotation{
        enum class Type{
            Link,      // 链接注释
            Path,      // 路径注释，一般为图形对象，如矩形、多边形、贝塞尔曲线等。
            Highlight, // 高亮注释
            Stamp,     // 签章注释
            Watermark, // 水印注释
        };
    }; // namespace Annotation

    // ======== class PageAnnot ========
    // 分页注释文件
    class PageAnnot {
    public:
        class Annot{
        public:
            ST_ID            ID;          // 注释的标识，必选。
            Annotation::Type Type;        // 注释类型，必选。
            std::string      Subtype;     // 注释子类型
            std::string      Creator;     // 注释创建者。
            ST_TIME          LastModDate; // 最近一次修改的时间。
            bool             Visible;     // 是否显示注释，默认值为true。
            bool             Print;       // 是否可打印，默认值为true。
            bool             NoZoom;      // 是否不随页面缩放而同步缩放，默认值为false。
            bool             NoRotate;    // 是否不随页面旋转而同上旋转，默认值为false。
            bool             ReadOnly;    // 是否不能被用户更改，默认值为true。

            std::string      Remark;      // 注释说明内容
            std::unordered_map<std::string, std::string> Parameters; // 一组注释参数。
            CT_PageBlock     Appearance;  // 注释的静态呈现效果，使用页面块定义来描述。
        }; // class Annot
        std::vector<Annot> Annotations;
    }; // class PageAnnot



}; // namespace ofd

#endif // __OFD_ANNOTATION_H__

#ifndef __OFD_DOCUMENT_H__
#define __OFD_DOCUMENT_H__

#include <memory>
#include <vector>
#include <string>
#include "Definitions.h"
#include "Annotation.h"

namespace ofd {

    class Page;

    // ======== CT_Permission ========
    // 文档权限声明结构
    class CT_Permission{
    public:
        
        bool Edit;        // 是否允许编辑，默认值为true，可选。
        bool Annot;       // 是否允许添加或修改标注，默认值为true，可选。
        bool Export;      // 是否允许导出，默认值为true，可选。
        bool Signature;   // 是否允许数字签名，默认值为true，可选。
        bool Watermark;   // 是否允许添加水印，默认值为true，可选。
        bool PrintScreen; // 是否允许截屏，默认值为true，可选。

        // 打印权限。若不设置Print节点，则默认为可以打印，并且打印份数不受限制。
        struct PrintPermission{
            bool Printable; // 文档是否允许被打印，默认值为true。
            int Copies;     // 打印份数，在Printable为true时有效。

            PrintPermission() : Copies(-1){};
        };
        PrintPermission Print;

        // 有效期，即此文档允许访问的期限，不设置表示不受限。
        struct PermissionValidPeriod{
            uint64_t StartDate; // 有效期开始日期
            uint64_t EndDate;   // 有效期结束日期
        };
        PermissionValidPeriod ValidPeriod;

        CT_Permission()
            : Edit(true), Annot(true), Export(true), 
            Signature(true), Watermark(true), PrintScreen(true){
            }

    }; // class CT_Permission


    // ======== CT_VPreferences ========
    // 文档视图首选项结构
    class CT_VPreferences{
    public:
        // 窗口模式
        enum class PageMode{
            None,          // 常规模式
            FullScreen,    // 打开后全屏显示
            UseOutlines,   // 同时呈现文档大纲
            UseThumbs,     // 同时呈现缩略图
            UseCustomTags, // 同时呈现语义结构
            UseLayers,     // 同时呈现图层
            UseAttatchs,   // 同时呈现附件
            UseBookmarks,  // 同时呈现书签
        };
        static std::string GetPageModeLabel(PageMode pageMode){
            if ( pageMode == PageMode::None ){
                return "None";
            } else if ( pageMode == PageMode::FullScreen ){
                return "FullScreen";
            } else if ( pageMode == PageMode::UseOutlines ){
                return "UseOutlines";
            } else if ( pageMode == PageMode::UseCustomTags ){
                return "UseCustomTags";
            } else if ( pageMode == PageMode::UseLayers ){
                return "UseLayers";
            } else if ( pageMode == PageMode::UseAttatchs ){
                return "UseAttatchs";
            } else if ( pageMode == PageMode::UseBookmarks ){
                return "UseBookmarks";
            }
            return "UNKNOWN";
        }
        PageMode PageMode;

        // 页面布局模式
        enum class PageLayout{
            OnePage,    // 单页模式
            OneColumn,  // 单列模式
            TwoPageL,   // 对开模式
            TwoColumnL, // 对开连续模式
            TwoPageR,   // 对开靠右模式
            TwoColumnR, // 对开连续靠右模式
        };
        static std::string GetPageLayoutLabel(PageLayout pageLayout){
            if ( pageLayout == PageLayout::OnePage ){
                return "OnePage";
            } else if ( pageLayout == PageLayout::OneColumn ){
                return "OneColumn";
            } else if ( pageLayout == PageLayout::TwoPageL ){
                return "TwoPageL";
            } else if ( pageLayout == PageLayout::TwoColumnL ){
                return "TwoColumnL";
            } else if ( pageLayout == PageLayout::TwoPageR ){
                return "TwoPageR";
            } else if ( pageLayout == PageLayout::TwoColumnR ){
                return "TwoColumnR";
            }
            return "UNKNOWN";
        }
        PageLayout PageLayout;

        // 标题栏显示模式
        enum class TabDisplay{
            DocTitle, // 呈现元数据中的Title属性
            FileName, // 文件名称
        };
        static std::string GetTabDisplayLabel(TabDisplay tabDisplay){
            if ( tabDisplay == TabDisplay::DocTitle ){
                return "DocTitle";
            } else if ( tabDisplay == TabDisplay::FileName ){
                return "FileName";
            }
            return "UNKNOWN";
        }
        TabDisplay TabDisplay;

        bool HideToolbar;  // 是否隐藏工具栏，默认值为false。
        bool HideMenubar;  // 是否隐藏菜单栏，默认值为false。
        bool HideWindowUI; // 是否隐藏主窗口之外的其它窗体组件。

        // 自动缩放模式
        enum class VPreferencesZoomMode{
            Default,   // 默认缩放
            FitHeight, // 适合高度
            FitWidth,  // 适合宽度
            FitRect,   // 适合区域
        };
        VPreferencesZoomMode ZoomMode;
        double Zoom; // 文档的缩放率

        CT_VPreferences() 
            : PageMode(PageMode::None), 
            PageLayout(PageLayout::OneColumn),
            TabDisplay(TabDisplay::DocTitle),
            HideToolbar(false), HideMenubar(false), HideWindowUI(false),
            ZoomMode(VPreferencesZoomMode::Default), Zoom(1.0){
            }

    }; // class CT_VPreferences


    // ======== class CT_OutlineElem ========
    // 大纲节点
    class CT_OutlineElem{
    public:
        std::string Title; // 大纲节点标题
        size_t Count;      // 该节点下所有叶节点的数目参考值，应根据该节点下实际出现的子节点数为准。
                           // 默认值为0。
        bool Expanded;     // 在有子节点存在时有效，如果为true，表示该大纲在初始状态下展开子节点；
                           // 如果为false，则表示不展开。

        std::vector<std::shared_ptr<CT_Action> > actions; // 当此大纲节点被激活时将执行的动作序列。
        std::vector<std::shared_ptr<CT_OutlineElem> > elems; // 该节点的子大纲节点，层层嵌套，形成树状结构。

        CT_OutlineElem() : Expanded(true){};

    }; // class CT_OutlineElem


    // ======== CT_Bookmark ========
    // 书签结构
    typedef struct CT_Bookmark{
        std::string Name; // 书签名称
        CT_Dest Dest;     // 书签对应的文档位置
    } CT_Bookmark_t;


    // CustomTags.xsd

    // ======== class CustomTags ========
    // 自定义标引列表
    // 外部系统或用户可以添加自定义的标记和信息，从而达到与其它系统、数据进行交互的目的并扩展应用。
    // 一个文档可以带有多个自定义标引。
    class CustomTag{
    public:
        // FIXME 标准中给出TypeID，而xsd中给出NameSpapce？
        std::string NameSpace; // 自定义标引入口，命名空间？ 
        ST_Loc      Schemaloc; // 指向自定义标引内容节点适用的Schema文件。
        ST_Loc      FileLoc;   // 指向自定义标引文件。该类文件中通过“非接触方式”引用版式内容流中
                               // 的图元和相关信息。
    }; // class CustomTag

    // Extensions.xsd

    // ======== class CT_Extension ======== 
    // 扩展信息
    // 扩展信息列表的入口点在文档根节点中定义。
    class CT_Extension {
    public:
        std::string AppName;    // 用于生成或解释该自定义对象数据的扩展应用程序名称。
        std::string Company;    // 形成此扩展信息的软件厂商标识。
        std::string AppVersion; // 形成此扩展信息的软件版本。
        uint64_t    Date;       // 形成此扩展信息的日期和时间。
        ST_RefID    RefID;      // 引用扩展项针对的文档项目的标识。
        std::unordered_map<std::string, std::pair<std::string, std::string> > Properties; // 扩展属性，
                                                                 // “Name Type Value"用于简单的扩展。
        void        *Data;      // 扩展复杂属性，使用xs:anyType，用于较复杂的扩展。
        ST_Loc      ExtendData; // 扩展数据文件所在位置，用于扩展大量信息。
    }; // class CT_Extension

    // Attachments.xsd

    // ======== class CT_Attachment ========
    // 附件
    class CT_Attachment{
    public:
        std::string ID; // 附件标识
        std::string Name; // 附件名称
        std::string Format; // 附件格式
        ST_TIME     CreationDate; // 创建时间
        ST_TIME     ModDate;      // 修改时间
        double      Size;         // 附件大小，以KB为单位。
        bool        Visible;      // 附件是否可见，默认为true。
        std::string Usage;        // 附件用途，默认为none。

        ST_Loc      FileLoc;      // 附件内容在包内的位置。

        CT_Attachment() : Visible(true), Usage("none") {};
    }; // class CT_Attachment

    // ======== Document ========
    class Document {
    public:

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
        CommonData commonData;

        std::vector<Page> Pages;
        CT_Permission Permissions;
        std::vector<CT_OutlineElem> Outlines;   // 大纲
        std::vector<CT_Action> Actions;
        CT_VPreferences VPreferences; // 首选项
        std::vector<CT_Bookmark> Bookmarks; // 书签
        
        //ST_Loc Annotations;
        Annotations Annotations;
        //ST_Loc Attachments;
        std::vector<CT_Attachment> Attachments; // 附件
        //ST_Loc Extensions;
        std::vector<CT_Extension> Extensions;
        std::vector<CustomTag> CustomTags;

    }; // class Document

}; // namespace ofd

#endif // __OFD_DOCUMENT_H__

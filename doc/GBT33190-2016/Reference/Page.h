#ifndef __OFD_PAGE_H__
#define __OFD_PAGE_H__

// Page.xsd

#include <string>
#include <vector>
#include "GraphicUnit.h"

namespace ofd {


    // ======== class CT_PageBlock ========
    // 页面块
    class CT_PageBlock {
    public:
        //std::shared_ptr<CT_Text> TextObject;           // 文字对象
        //std::shared_ptr<CT_Path> PathObject;           // 图形对象
        //std::shared_ptr<CT_Image> ImageObject;         // 图像对象
        //std::shared_ptr<CT_Composite> CompositeObject; // 复合对象
        //std::shared_ptr<CT_PageBlock> BlockObject;     // 嵌套页对象

        std::vector<void*> Objects;
    }; // class PageBlock

    // ======== enum LayerType ========
    // 图层类型。
    // 前景层、正文层、背景层形成多层内容，这些层按照出现的先后顺序
    // 依次进行渲染，每一层的默认颜色采用全透明。层的渲染顺序如下：
    //
    //     前景层    ^  最上层
    //    前景模板   |
    //     正文层    |
    //    正文模板   |
    //     背景层    |
    //    背景模板   |  最下层
    //
    enum class LayerType{
        Body, // 正文层
        Foreground, // 前景层
        Background, // 背景层
    };

    // ======== class CT_Layer ========
    // 图层对象
    class CT_Layer : public CT_PageBlock{
    public:
        LayerType Type; // 图层类型描述，默认值为Body。
        ST_RefID DrawParam; // 图层的绘制参数，引用资源文件中定义的绘制参数标识。
    }; // class CT_Layer


    // ======== class Page ========
    // 页对象
    class Page {
    public:

        // 该页使用的模板页。模板页的内容结构和普通页相同，定义在CommonData指定的XML文件中。
        // 一个页可以使用多个模板页。该节点使用时通过TemplateID来引用具体的模板，并通过ZOrder
        // 属性来控制模板在页面中的呈现顺序。在模板页的内容描述中该属性无效。
        class Template{
        public:

            enum class ZOrder{
                Background, 
                Foreground,
            };
            static std::string GetZOrderLabel(ZOrder zorder){
                if ( zorder == ZOrder::Background ){
                    return "Background";
                } else if ( zorder == ZOrder::Foreground ){
                    return "Foreground";
                }
                return "";
            }

            ST_RefID TemplateID; // 引用在文档公用数据(CommonData)中定义的模板页标识。
            ZOrder ZOrder;       // 控制模板在页面中的呈现顺序，其类型描述和呈现顺序
                                 // 与Layer中Type的描述和处理一致。
                                 // 如果多个图层的此属性相同，则应根据其出现的顺序来显示，
                                 // 先出现者先绘制。默认值为Background。

            Template() : ZOrder(ZOrder::Background){};
        }; // class Template
        std::vector<Template> Templates; // minOccus=0
        ST_Loc                PageRes;   // 页资源，指向该页使用的资源文件。
        CT_PageArea           Area;      // 定义该页页面区域的大小和位置，仅对该页有效。
                                         // 该节点不出现时，则使用模板页中的定义。
                                         // 若模板页不存在或模板页中没有定义页面区域，
                                         // 则使用文件CommonData中的定义。
        std::vector<CT_Action> Actions;  // 与页面关联的动作序列。
                                         // 当存在多个Action对象时，所有动作依次执行。
                                         // 事件类型应为PO（页面打开）。

        class Layer : public CT_Layer {
        public:
            ST_ID ID;
        };

        class Content{
        public:
            std::vector<std::shared_ptr<Layer> > Layers;
            size_t GetLayersCount() const {return Layers.size();};
            const std::shared_ptr<Layer> GetLayerByIndex(size_t idx) const {return Layers[idx];};
            std::shared_ptr<Layer> GetLayerByIndex(size_t idx){return Layers[idx];};
        }; // class Content

        std::unique_ptr<Content> Content;

    }; // class Page

}; // namespace ofd

#endif // __OFD_PAGE_H__

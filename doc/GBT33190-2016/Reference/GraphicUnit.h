#ifndef __OFD_GRAPHICUNIT_H__
#define __OFD_GRAPHICUNIT_H__

// Page.xsd

#include "Definitions.h"

namespace ofd{

    // ======== class CT_Color ========
    // 颜色
    class CT_Color{
    public:
        ST_Array Value;      // 颜色值，指定了当前颜色空间下各通道的取值。
                             // Value的取值应符合“通道1 通道2 通道3 ...”格式。
                             // 此属性不出现时，应采用Index属性从颜色空间的调色板中取值。
                             // 当二者都不出现时，该颜色各通道的值全部为0。
        uint32_t Index;      // 调色板中颜色的编号，非负整数，将从当前颜色空间的调色板中
                             // 取出相应索引的预定义颜色用来绘制。索引从0开始。
        ST_RefID ColorSpace; // 引用资源文件中颜色空间的标识。默认值为文档设定的颜色空间。
        uint32_t Alpha;      // 颜色透明度，在0-255之间取值。默认值为255，表示完全不透明。

        CT_Color() : Alpha(255) {};

        // TODO Pattern

    }; // class CT_Color

    class CT_GraphicUnit;
    // ======== class CT_Clip ========
    // 裁剪区
    // 裁剪区由一级路径或文字构成，用以指定页面上的一个有效绘制区域，
    // 落在裁剪区以外的部分不受绘制指令的影响。
    class CT_Clip{
    public:

        // 裁剪区域，用一个图形对象或文字对象来描述裁剪区的一个组成部分，
        // 最终裁剪区是这些区域的并集。
        struct Area{
            ST_RefID DrawParam; // 引用资源文件中的绘制参数的标识，
                                // 线宽、结合点和端点样式等绘制特性对裁剪效果会产生影响。
            //ST_Array CTM;       // 针对对象坐标系，对Area下包含的Path和Text进行进一步的变换。
            double CTM[6];

            std::unique_ptr<CT_GraphicUnit> Shape;
            //CT_Path Path
            //CT_Text Text
        };
        std::vector<Area> Areas;

    }; // class CT_Clip

    namespace GraphicUnit{
        // 线条连接样式，指定了两个线的端点结合时采用的样式。
        enum class Join{
            Miter,
            Round,
            Bevel,
        };
        __attribute__((unused)) static std::string GetJoinLabel(Join join){
            if ( join == Join::Miter ){
                return "Miter";
            } else if ( join == Join::Round ){
                return "Round";
            } else if ( join == Join::Bevel ){
                return "Bevel";
            }
            return "";
        }


        // 线端点样式，指定了一条线的端点样式
        enum class Cap{
            Butt,
            Round,
            Square,
        };
        __attribute__((unused)) static std::string GetCapLabel(Cap cap){
            if ( cap == Cap::Butt ){
                return "Butt";
            } else if ( cap == Cap::Round ){
                return "Round";
            } else if ( cap == Cap::Square ){
                return "Square";
            }
            return "";
        }

    }; // namespace GraphicUnit

    // ======== class CT_GraphicUnit ========
    // 基本图元定义
    class CT_GraphicUnit{
    public:
        ST_Box Boundary;     // 外接矩形，采用当前空间坐标系（页面坐标或其它容器坐标），
                             // 当图元绘制超出此矩形区域时进行裁剪。
        std::string Name;    // 图元对象的名字，默认值为空。
        bool Visible;        // 图元是否可见，默认值为true。
        ST_Array CTM;        // 对象空间内的图元变换矩阵。
        ST_RefID DrawParam;  // 引用资源文件中的绘制参数标识。
        double LineWidth;    // 绘制路径时使用的线宽。如果图元对象有DrawParam属性，
                             // 则用此值覆盖DrawParam中对应的值。

        GraphicUnit::Cap Cap;

        GraphicUnit::Join Join;

        // TODO 此时默认值与Res.xsd中CT_DrawParam中的定义不同？
        double MiterLimit;    // Join为Miter时，MiterSize的截断值。 如果图元对象有DrawParam属性，
                              // 则用此值覆盖DrawParam中对应的值。默认值4.234。
        double DashOffset;    // 同DrawParam中对应属性。
        ST_Array DashPattern; // 同DrawParam中对应属性。
        int Alpha;            // 颜色透明度，在0-255之间取值。默认值为255，表示完全不透明。

        std::vector<CT_Action> Actions; // 图元对象动作集合。图元动作事件类型应为CLICK。
        std::vector<CT_Clip> Clips;     // 图元对象裁剪区域集合，采用对象空间坐标系。
                                        // 当存在多个Clip对象时，最终裁剪区为所有区域的交集。

        CT_GraphicUnit():
            Visible(true), LineWidth(0.353),
            Cap(GraphicUnit::Cap::Butt), Join(GraphicUnit::Join::Miter),
            MiterLimit(4.234), DashOffset(0), Alpha(255)
        {
        }

    }; // class CT_GraphicUnit


    // ======== class CT_Path ========
    // 图形对象
    // 图形对象具有一般图元对象的一切属性和行为特征。
    class CT_Path : public CT_GraphicUnit {
    public:
        bool Stroke; // 图形是否被勾边，默认值为true。
        bool Fill;   // 图形是否被填充，默认值为false。
    
        // 图形对象的填充规则，当Fill属性存在时出现。默认值为NonZero。
        enum class Rule{
            NonZero,
            EvenOdd,
        };
        Rule Rule;

        CT_Color StrokeColor;        // 勾边颜色，默认为黑色。
        CT_Color FillColor;          // 填充颜色，默认为透明色。

        // 图形轮廓数据，由一系列紧缩的操作符和操作数构成。必选。
        //
        // 操作符  操作数             说明
        //
        // S       x y                定义子绘制图形边线的起始点坐标(x,y)。
        //
        // M       x y                将当前点移动到指定点(x,y)。
        //
        // L       x y                将当前点连接一条到指定点(x,y)的线段，并将当前点移动到指定点。
        //
        // Q       x1 y1 x2 y2        从当前点连接一条到点(x2,y2)的二次贝塞尔曲线，并将当前点移动到
        //                            点(x2,y2)，此贝塞尔曲线使用点(x1,y1)作为控制点。
        //
        // B       x1 y1 x2 y2 x3 y3  从当前点连接一条到点(x3,y3)的三次贝塞尔曲线，并将当前点移动到
        //                            点(x3,y3)，此贝塞尔曲线使用点(x1,y1)和点(x2,y2)作为其控制点。
        //
        // A       r_x r_y angle      从当前点连接一条到点(x,y)的圆弧，并将当前点移动到点(x,y)。
        //         large sweep        r_x表示椭圆的长轴，r_y表示椭圆的短轴，angle表示椭圆在当前坐标系
        //                            下旋转的角度，正值为顺时针，负值为逆时针，large为1时表示对应度数
        //                            大于180度的弧，为0时表示对应度数小于180度的弧，sweep为1时表示
        //                            由圆弧起始点到结束点是顺时针方向，为0时表示由圆弧起始点到结束点
        //                            是逆时针方向。
        //
        // C                          SubPath自动闭合，表示将当前点和SubPath的起始点用线段直接连接。
        std::string AbbreviatedData; 

        CT_Path() : Stroke(true), Fill(false), Rule(Rule::NonZero) {};

    }; // class CT_Path

    // ======== class CT_Path ========
    // 图像对象
    class CT_Image : public CT_GraphicUnit {
    public:
        ST_RefID ResourceID;   // 引用资源文件中定义的多媒体的标识，**必选**。
        ST_RefID Substitution; // 可替换图像，引用资源文件中定义的多媒体的标识，
                               // 用于某些情况如高分辨率输出时进行图替换。
        ST_RefID ImageMask;    // 图像蒙版，引用资源文件中定义的多媒体的标识，用作蒙版的图像
                               // 应是与ResourceID指向的图像相同大小的二值图。
        // 图像边框设置
        class Border {
        public:
            double LineWidth;             // 边框线宽，如果为0则表示边框不进行绘制。默认值为0.353mm。
            double HorizonalCornerRadius; // 边框水平角半径，默认值为0。
            double VerticalCornerRadius;  // 边框垂直角半径，默认值为0。
            double DashOffset;            // 边框虚线重复样式开始的位置，边框的起始点位置为左上角，
                                          // 绕行方向为顺时针。默认值为0。
            ST_Array DashPattern;         // 边框虚线重复样式，边框的起始点位置为左上角，
                                          // 绕行方向为顺时针。
            CT_Color BorderColor;         // 边框颜色，默认为黑色。 

            Border() : LineWidth(0.353), HorizonalCornerRadius(0), VerticalCornerRadius(0) {};

        }; // class Border
        Border Border;

    }; // class CT_Image;

    // ======== class CT_Text ========
    class CT_Text : public CT_GraphicUnit {
    public:
        ST_RefID Font; // 引用资源文件中定义的字型的标识。
        double Size;   // 字号，单位为毫米。
        bool Stroke;   // 是否勾边，默认值为false。
        bool Fill;     // 是否填充，默认值为true。
        double HScale; // 字型在水平方向的缩放比，默认值为1.0。
                       // 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
        int ReadDirection; // 阅读方向，可选值为0,90,180,270，默认值为0。
        int CharDirection; // 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
        int Weight;        // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
                           // 默认值为400。
        bool Italic;       // 是否是斜体样式，默认值为false。

        CT_Color FillColor;   // 填充色，默认值为黑色。
        CT_Color StrokeColor; // 勾边色，默认值为透明色。

        // TODO
        //CT_CGTransform CGTransform; // 指定字符编码到字符索引之间的变换关系。

        class TextCode{
        public:
            double X;        // 第一个文字的字型原点在对象坐标系下的X坐标。
                             // 当X不出现，则采用上一个TextCode的X值，
                             // 文字对象中的第一个TextCode的X属性必选。
            double Y;        // 第一个文字的字型原点在对象坐标系下的Y坐标。
                             // 当Y不出现，则采用上一个TextCode的Y值，
                             // 文字对象中的第一个TextCode的Y属性必选。
            ST_Array DeltaX; // double型数值队列，队列中的每个值代表后一个文字与
                             // 前一个文字之间在X方向的偏移值。DeltaX不出现时，
                             // 表示文字的绘制点在X方向上不做偏移。
            ST_Array DeltaY; // double型数值队列，队列中的每个值代表后一个文字与
                             // 前一个文字之间在Y方向的偏移值。DeltaY不出现时，
                             // 表示文字的绘制点在Y方向上不做偏移。
            std::string Text; 
        }; // class TextCode
        std::vector<TextCode> TextCodes;

    }; // class CT_Text

    // ======== class CT_Composite ========
    // 复合对象
    // 复合对象是一种特殊的图元对象，拥有图元对象的一切特性，但其内容在ResourceID
    // 指向的矢量图像资源中进行描述，一个资源可以被多个复合对象所引用。通过这种方式
    // 可实现对文档内矢量图文内容的复用。
    // 复合对象引用的资源是Res中的矢量图像(CompositeGraphUnit)，其类型为CT_VectorG。
    class CT_Composite : public CT_GraphicUnit {
    public:
        ST_RefID ResourceID; // 引用资源文件中定义的矢量图像的标识。
    }; // class CT_Composite

} // namespace ofd

#endif // __OFD_GRAPHICUNIT_H__

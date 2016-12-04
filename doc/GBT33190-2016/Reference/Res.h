#ifndef __OFD_RES_H__
#define __OFD_RES_H__

#include "GraphicUnit.h"

namespace ofd {

    namespace ColorSpace{
        // 颜色空间的类型。
        enum class Type{
            GRAY,
            RGB,
            CMYK,
        };
        __attribute__((unused)) static std::string GetTypeLabel(Type type){
            if ( type == Type::GRAY ){
                return "GRAY";
            } else if ( type == Type::RGB ){
                return "RGB";
            } else if ( type == Type::CMYK ){
                return "CMYK";
            }
            return "UNKNOWN";
        }
    }; // namespace ColorSpace

    // ======== class CT_Font ========
    // 字型对象
    class CT_Font {
    public:
        std::string FontName; // 字型名 
        std::string FamilyName; // 字型族名，用于匹配替代字型。
        std::string Charset;    // 字型适用的字符分类（字符集），用于匹配替代字型, 默认值为“unicode”。
        bool Italic;            // 是否是斜体字型，用于配置替代字型，默认值为false。
        bool Bold;              // 是否是粗体字型，用于配置替代字型，默认值为false。
        bool Serif;             // 是否是带衬线字型，用于匹配替代字型，默认值为false。
        bool FixedWidth;        // 是否是等宽字型，用于匹配替代字型，默认值为false。
        ST_Loc FontFile;        // 指向内嵌字型文件，嵌入字型文件应使用OpenType格式。

        CT_Font() : Charset("unicode"),
                Italic(false), Bold(false), Serif(false), FixedWidth(false){
        };

    }; // class CT_Font

    // ======== class CT_ColorSpace ========
    // 颜色空间
    class CT_ColorSpace{
    public:

        // 颜色空间的类型，必选。
        ColorSpace::Type Type;

        uint32_t BitsPerComponent;     // 每个颜色通道使用的位数，有效取值为1,2,4,8,16，默认值为8。
        ST_Loc Profile;                // 指向包内颜色配置文件
        std::vector<ST_Array> Palette; // 调色板。调色板中颜色的索引编号从0开始。

        CT_ColorSpace() : BitsPerComponent(8) {};

    };

    // ======== class CT_DrawParam ========
    class CT_DrawParam{
    public:
        ST_RefID    Relative;     // 基础绘制参数，引用资源文件中的绘制参数的标识。
        double      LineWidth;    // 线宽，非负浮点数，指定了路径绘制时线的宽度。
                                  // 由于某些设备不能输出一个像素宽度的线，因此强制
                                  // 规定当线宽大于0时，无论多小都最少要绘制两个像素的宽度；
                                  // 当线宽为0时，绘制一个像素的宽度。由于线宽0的定义与设备相关，
                                  // 所以不推荐使用线宽0.默认值为0.353mm。
        GraphicUnit::Join Join;         

        GraphicUnit::Cap Cap;          

        double      DashOffset;  // 线条虚线样式开始的位置，默认值为0。当DashPattern不出现时，该参数无效。
        ST_Array    DashPattern; // 线条虚线的重复样式，数组中共含两个值，
                                 // 第一个值代表虚线线段的长度，第二个值代表虚线间隔的长度。
                                 // 默认值为空。
        double      MiterLimit;  // Join为Miter时小角度结合点长度的截断值，默认值为3.528。
                                 // 当Join不为Miter时该参数无效。

        CT_Color FillColor;      // 填充颜色，用以填充路径形成的区域以及文字轮廓内的区域，默认值为透明色。
        CT_Color StrokeColor;    // 勾边颜色，指定路径绘制的颜色以及文字轮廓的颜色，默认值为黑色。

        CT_DrawParam() : LineWidth(0.353), 
            Join(GraphicUnit::Join::Miter), Cap(GraphicUnit::Cap::Butt),
            DashOffset(0), MiterLimit(3.528){
        }
    };

    // ======== class CT_MultiMedia ========
    // 多媒体资源
    class CT_MultiMedia {
    public:

        // 多媒体类型
        enum class Type{
            Image, // 图像
            Audio, // 音频
            Video, // 视频
        };
        static std::string GetTypeLabel(Type type){
            if ( type == Type::Image ){
                return "Image";
            } else if ( type == Type::Audio ){
                return "Audio";
            } else if ( type == Type::Video ){
                return "Video";
            }
            return "";
        }
        Type Type;             
        std::string Format;    // 资源的格式，支持BMP, JPEG, PNG, TIFF
                               // 以及AVS等格式，其中TIFF格式不支持多页。
        ST_Loc      MediaFile; // 指向OFD包内的多媒体文件的位置。

    }; // class CT_MultiMedia


    // ======== class CT_VectorG ========
    // 矢量图像对象。
    class CT_VectorG {
    public:
        double Width;          // 矢量图像的宽度，超出部分做裁剪处理，必选。
        double Height;         // 矢量图像的高度，超出部分做裁剪处理，必选。
        ST_RefID Thumbnail;    // 缩略图，指向包内的图像文件。
        ST_RefID Substitution; // 替换图像，用于高分辨率输出时将缩略图替换为此分辨率的图像。
                               // 指向包内的图像文件。
        CT_PageBlock Content;  // 内容的矢量描述。
    }; // class CT_VectorG


    // ======== class Res ========
    class Res {
    public:
        ST_Loc BaseLoc; // 定义此资源文件的通用数据存储路径，BaseLoc属性的意义在于
                        // 明确资源文件存储的位置，比如R1.xml中可以指定BaseLoc为
                        // "./Res"，表明该资源文件中所有数据文件的默认存储位置在
                        // 当前路径的Res目录下。

        // 颜色空间描述
        class ColorSpace : public CT_ColorSpace{
        public:
            ST_ID ID;
        }; // class ColorSpace
        std::vector<ColorSpace> ColorSpaces; 

        // 绘制参数描述
        class DrawParam : public CT_DrawParam{
        public:
            ST_ID ID;
        }; // class DrawParam
        std::vector<DrawParam>  DrawParams;

        // 字型资源描述
        class Font : public CT_Font{
        public:
            ST_ID ID;
        }; // class Font
        std::vector<Font>       Fonts;

        // 多媒体资源描述
        class MultiMedia : public CT_MultiMedia{
        public:
            ST_ID ID;
        }; // class MultiMedia
        std::vector<MultiMedia> MultiMedias;


        // 矢量图像（被复合园元对象所引用）描述
        class CompositeGraphicUnit : CT_VectorG{
        public:
            ST_ID ID;
        }; // class CompositeGraphicUnit
        std::vector<CompositeGraphicUnit> CompositeGraphicUnits;


    }; // class Res


} // namespace ofd

#endif // __OFD_RES_H__
                                 // 第一个值代表虚线线段的长度， // 第二个值代表虚线间隔的长度。double      MiterLimit; 

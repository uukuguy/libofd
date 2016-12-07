#ifndef __OFD_COLOR_H__
#define __OFD_COLOR_H__

#include <memory.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace ofd{

    class OFDColor;
    typedef std::vector<OFDColor> ColorArray;

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
            return "";
        }
    }; // namespace ColorSpace

    namespace Color{

        typedef struct Value{

            typedef struct RGB{
                uint32_t Red;   // 红色
                uint32_t Green; // 绿色
                uint32_t Blue;  // 蓝色
            } RGB_t;

            typedef struct CMYK{
                uint32_t Cyan;    // 青色
                uint32_t Magenta; // 品红色
                uint32_t Yellow;  // 黄色
                uint32_t blacK;   // 黑色
            } CMYK_t;

            union{
                uint32_t Gray; // 灰度，只包含一个通道来表明灰度值。
                RGB_t    RGB;  // 包含三个通道，依次是红、绿、蓝。
                CMYK_t   CMYK; // 包含四个通道，依次是青、黄、品红、黑。
            };

            Value(){
                memset(this, 0, sizeof(Value));
            }

        } ColorValue_t;

    }; // namespace Color

    // ======== struct OFDColorSpace ========
    // 标准 P31 页，Res.xsd。
    typedef struct OFDColorSpace{

        ColorSpace::Type Type;    // 颜色空间的类型，必选。
        uint32_t         BPC;     // 每个颜色通道使用的位数，有效取值为1,2,4,8,16，默认值为8。
        ST_Loc           Profile; // 指向包内颜色配置文件
        ColorArray       Palette; // 调色板。调色板中颜色的索引编号从0开始。

        OFDColorSpace() : BPC(8) {};

    } OFDColorSpace_t;
    typedef std::shared_ptr<OFDColorSpace> OFDColorSpacePtr;

    // ======== struct OFDColor ========
    // 标准 P32 页，Page.xsd。
    typedef struct OFDColor{

        OFDColorSpacePtr ColorSpace; // 引用资源文件中颜色空间的标识。默认值为文档设定的颜色空间。
        Color::Value     Value;      // 颜色值，指定了当前颜色空间下各通道的取值。
                                     // Value的取值应符合“通道1 通道2 通道3 ...”格式。
                                     // 此属性不出现时，应采用Index属性从颜色空间的调色板中取值。
                                     // 当二者都不出现时，该颜色各通道的值全部为0。
        uint32_t         Alpha;      // 颜色透明度，在0-255之间取值。默认值为255，表示完全不透明。
        uint32_t         Index;      // 调色板中颜色的编号，非负整数，将从当前颜色空间的调色板中
                                     // 取出相应索引的预定义颜色用来绘制。索引从0开始。


        // TODO 
        // Pattern;      // 底纹 标准 P34 页，Page.xsd。
        // AxialShd;     // 轴向渐变 标准 P36 页，Page.xsd。
        // RadialShd;    // 径向渐变 标准 P40 页，Page.xsd。
        // GouraudShd;   // 高洛德渐变 标准 P47 页，Page.xsd。
        // LaGouraudShd; // 格构高洛德渐变 标准 P48 页，Page.xsd。

        OFDColor() : ColorSpace(nullptr), Alpha(0), Index(0){
        }

    } OFDColor_t; 
    typedef std::shared_ptr<OFDColor> OFDColorPtr;

}; // namespace ofd

#endif // __OFD_COLOR_H__

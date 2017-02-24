#ifndef __OFD_COMMON_H__
#define __OFD_COMMON_H__

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "utils/utils.h"

namespace ofd{

    // C++14
    //template<class T> std::shared_ptr<T> TPtr;
    //template<class T> std::vector<T> TArray;

    class Package;
    typedef std::shared_ptr<Package> PackagePtr;
    typedef std::vector<PackagePtr> PackageArray;

    class Document;
    typedef std::shared_ptr<Document> DocumentPtr;
    typedef std::vector<DocumentPtr> DocumentArray;

    class Page;
    typedef std::shared_ptr<Page> PagePtr;
    typedef std::vector<PagePtr> PageArray;

    class Layer;
    typedef std::shared_ptr<Layer> LayerPtr;
    typedef std::vector<LayerPtr> LayerArray;

    class PageBlock;
    typedef std::shared_ptr<PageBlock> PageBlockPtr;
    typedef std::vector<PageBlockPtr> PageBlockArray;

    class Object;
    typedef std::shared_ptr<Object> ObjectPtr;
    typedef std::vector<ObjectPtr> ObjectArray;

    class Resource;
    typedef std::shared_ptr<Resource> ResourcePtr;
    typedef std::vector<ResourcePtr> ResourceArray;

    class Font;
    typedef std::shared_ptr<Font> FontPtr;
    typedef std::vector<FontPtr> FontArray;
    typedef std::map<uint64_t, FontPtr> FontMap;

    class Color;
    typedef std::shared_ptr<Color> ColorPtr;
    typedef std::vector<ColorPtr> ColorArray;

    class ColorSpace;
    typedef std::shared_ptr<ColorSpace> ColorSpacePtr;
    typedef std::vector<ColorSpacePtr> ColorSpaceArray;

    class Pattern;
    typedef std::shared_ptr<Pattern> PatternPtr;
    typedef std::vector<PatternPtr> PatternArray;

    class Shading;
    typedef std::shared_ptr<Shading> ShadingPtr;
    typedef std::vector<ShadingPtr> ShadingArray;

    class Subpath;
    typedef std::shared_ptr<Subpath> SubpathPtr;
    typedef std::vector<SubpathPtr> SubpathArray;

    class Path;
    typedef std::shared_ptr<Path> PathPtr;
    typedef std::vector<PathPtr> PathArray;

    // **************** struct Point ****************
    typedef struct _Point{
        double x;
        double y;

        _Point() : x(0.0), y(0.0){};
        _Point(double _x, double _y): x(_x), y(_y){};

        void Clear(){
            x = y = 0.0;
        }

        void Offset(double dx, double dy){
            x += dx; y += dy;
        }

        bool operator ==(const _Point& other) const {
            return x == other.x && y == other.y;
        }

        bool operator !=(const _Point& other) const {
            return !(*this == other);
        }

    } Point_t;


    // 以下类型定义在OFD标准的Definitions.xsd文件中

    // ST_ID: 标识，无符号整数，应在文档内唯一。0表示无标识。
    typedef uint64_t    ST_ID;

    // ST_RefID: 标识引用，无符号整数，此标识应为文档内已定义的标识。
    typedef uint64_t    ST_RefID;

    // ST_Loc: 包结构内文件的路径，”.“表示当前路径，”..“表示父路径。
    // 约定：
    //      1. ”/”代表根节点
    //      2. 未显示指定时，代表当前路径
    //      3. 路径区分大小写
    typedef std::string ST_Loc;

    // ST_Array: 数组，以空格来分割元素。元素可以是除ST_Loc、ST_Array外的数据类型，不可嵌套。
    //typedef std::string ST_Array;
    typedef std::vector<std::string> ST_Array;

    // ST_Pos: 点坐标，以空格分割，前者为x值，后者为y值，可以是整数或者浮点数。
    typedef struct ST_Pos{
        double x;
        double y;
        ST_Pos() : x(0.0), y(0.0){};
    } ST_Pos_t;

    // ST_Box: 矩形区域，以空格分割，前两个值代表了该矩形的左上角坐标，
    // 后两个值依次表示该矩形的宽和高，可以是整数或者浮点数，后两个值应大于0.
    typedef struct ST_Box{
        double Left;
        double Top;
        double Width;
        double Height;

        ST_Box() : Left(0.0), Top(0.0),
            Width(0.0), Height(0.0) {
        }

        ST_Box(double left, double top, double width, double height) :
            Left(left), Top(top), Width(width), Height(height){
            }

        std::string to_string() const {
            std::stringstream ss;
            utils::SetStringStreamPrecision(ss, 3);
            ss << Left << ", " << Top << ", " << Width << ", " << Height; 

            return std::string("[") +
                    ss.str() + 
                   "]";
        }

        std::string to_xmlstring() const {
            std::stringstream ss;
            utils::SetStringStreamPrecision(ss, 3);
            ss << Left << " " << Top << " " << Width << " " << Height; 
            return ss.str();
        }

    } ST_Box_t;

    typedef std::string ST_TIME;
    typedef std::vector<double> DoubleArray;

    // ======== CT_PageArea ========
    // 页面区域结构
    typedef struct CT_PageArea {
        // 页面物理区域，左上角的坐标为页面空间坐标系的原点。
        // = CropBox in PDF.
        ST_Box PhysicalBox;

        // 显示区域，页面内容实际显示或打印输出的区域，
        // 位于页面物理区域内，包含页眉、页脚、版心等内容。
        // = Media Box in PDF.
        ST_Box ApplicationBox;

        // 版心区域，即文件的正文区域，位于显示区域内，
        // 左上角的坐标决定了其在显示区域内的位置。
        // 版心区域不在显示区域内的部份不会被绘制。
        ST_Box ContentBox;

        // 出血区域，即超出设备性能限制的额外出血区域，位于页面物理区域外。
        // 不出现时，默认值为页面物理区域。
        // 出血区域与物理区域相交的部分被忽略。
        ST_Box BleedBox;

        CT_PageArea() : m_bApplicationBox(false), m_bContentBox(false), m_bBleedBox(false){
        }

        bool HasApplicationBox() const {return m_bApplicationBox;};
        bool HasContentBox() const {return m_bContentBox;};
        bool HasBleedBox() const {return m_bBleedBox;};

        void EnableApplicationBox(bool bEnable=true){m_bApplicationBox = bEnable;};
        void EnableContentBox(bool bEnable=true){m_bContentBox = bEnable;};
        void EnableBleedBox(bool bEnable=true){m_bBleedBox = bEnable;};

        std::string to_string() const {
            return std::string("(") + 
                   PhysicalBox.to_string() + 
                   ", " + ApplicationBox.to_string() + 
                   ", " + ContentBox.to_string() + 
                   ", " + BleedBox.to_string() + 
                   ")";
        }

    private:
        bool m_bApplicationBox;
        bool m_bContentBox;
        bool m_bBleedBox;

    } CT_PageArea_t; 

}; // namespace ofd

#endif // __OFD_COMMON_H__

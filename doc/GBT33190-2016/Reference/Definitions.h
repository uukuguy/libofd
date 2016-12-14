#ifndef __OFD_DEFINITIONS_H__
#define __OFD_DEFINITIONS_H__

#include <string>
#include <memory>
#include <vector>

namespace ofd {

    // Definitions.xsd

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
    //typedef std::string ST_Pos;
    typedef struct ST_Pos{
        double x;
        double y;
    } ST_Pos_t;

    // ST_Box: 矩形区域，以空格分割，前两个值代表了该矩形的左上角坐标，后两个值依次表示该矩形的宽和高，
    // 可以是整数或者浮点数，后两个值应大于0.
    //typedef std::string ST_Box;
    typedef struct ST_Box{
        double Left;
        double Top;
        double Width;
        double Height;
    } ST_Box_t;

    typedef uint64_t ST_TIME;

    namespace Dest{
        // 声明目标区域的描述方法。
        enum class Type{
            XYZ,  // 目标区域由左上角位置(Left,Top)以及页面缩放比例(Zoom)确定。
            Fit,  // 适合整个窗口区域。
            FitH, // 适合窗口宽度，目标区域仅由Top确定。
            FitV, // 适合容器高度，目标区域仅由Left确定。
            FitR, // 适合窗口内的目标区域，目标区域为(Left, Top, Right, Bottom)所确定的矩形区域。
        };
        __attribute__((unused)) static std::string GetTypeLabel(Type type){
            if ( type == Type::XYZ ){
                return "XYZ";
            } else if (type == Type::Fit ){
                return "Fit";
            } else if (type == Type::FitH ){
                return "FitH";
            } else if (type == Type::FitV ){
                return "FitV";
            } else if (type == Type::FitR ){
                return "FitR";
            }
            return "";
        }

    }; // namespace Dest

    // ======== CT_Dest ========
    // 目标区域
    class CT_Dest {
    public:

        // 声明目标区域的描述方法，必选。
        Dest::Type    Type;

        ST_RefID    PageID; // 引用跳转目标页面的标识，必选。

        double      Left;   // 目标区域左上角x坐标。
        double      Top;    // 目标区域右上角y坐标。
        double      Right;  // 目标区域右下角x坐标。
        double      Bottom; // 目标区域右下角y坐标。
        double      Zoom;   // 目标区域页面缩放比例，为0 或不出现则按照当前页面缩放比例跳转，
                            // 可取值范围[0.1, 64.0]。
    }; // class CT_Dest

    // ======== CT_PageArea ========
    // 页面区域结构
    class CT_PageArea {
    public:
        // 页面物理区域，左上角的坐标为页面空间坐标系的原点。
        ST_Box PhysicalBox;

        // 显示区域，页面内容实际显示或打印输出的区域，
        // 位于页面物理区域内，包含页眉、页脚、版心等内容。
        ST_Box ApplicationBox;

        // 版心区域，即文件的正文区域，位于显示区域内，
        // 左上角的坐标决定了其在显示区域内的位置。
        // 版心区域不在显示区域内的部份不会被绘制。
        ST_Box ContentBox;

        // 出血区域，即超出设备性能限制的额外出血区域，位于页面物理区域外。
        // 不出现时，默认值为页面物理区域。
        // 出血区域与物理区域相交的部分被忽略。
        ST_Box BleedBox;

    }; // class CT_PageArea

    // ======== class CT_Region ========
    // 区域
    // 区域由一系列的分路径(Area)组成，每个分路径是闭合的。
    class CT_Region {
    public:
        class Area {
        public:
            ST_Pos Start; // 定义子图形的起始点坐标。

            class Op{
            }; // class Op

            // 移动
            class Move : public Op {
            public:
                ST_Pos Point1; // 移动后新的当前绘制点
            };

            // 线段
            class Line : public Op {
            public:
                ST_Pos Point1; // 线段的结束点
            };

            // 二次贝塞尔曲线
            class QuadraticBezier : public Op {
            public:
                ST_Pos Point1; // 二次贝塞尔曲线的控制点
                ST_Pos Point2; // 二次贝塞尔曲线的结束点，下一路径的起始点。
            };

            // 三次贝塞尔曲线
            class CubicBezier : public Op {
            public:
                ST_Pos Point1; // 三次贝塞尔曲线的第一个控制点。
                ST_Pos Point2; // 三次贝塞尔曲线的第二个控制点。
                ST_Pos Point3; // 三次贝塞尔曲线的结束点，下一路径的起始点。
            };

            // 圆弧
            class Arc : public Op {
            public:
                bool     SweepDirection; // 弧线方向是否为顺时针。
                bool     LargeArc;       // 是否为大圆弧。
                double   RotationAngle;  // 在当前坐标系下旋转的角度，正值为顺时针，负值为逆时针。
                                         // 如果角度大于360度，则以360取模。
                ST_Array EllipseSize;    // 形如[200 100]的数组，2个正浮点数值依次对应椭圆的长、短轴长度。
                                         // 如果数组长度超过2个，则只取前两个数值
                                         // 如果数组长度为1，则认为是一个圆，该数值是圆半径。
                                         // 如果数组前两个数值中有一个为0，或者数组为空，则圆弧退化成
                                         // 一个从当前点到EndPoint的线段。
                ST_Pos   EndPoint;       // 圆弧的结束点，下一路径的起始点。
            };

            // 自动闭合到当前分路径的起始点，并以该点为当前点。
            struct Close : public Op {
            };

            std::vector<std::shared_ptr<Op> > Ops;
        };

        std::vector<Area> Areas;

    }; // class CT_Region

    namespace Action{
        // 事件类型
        enum class Event{
            DO,    // 文档打开
            PO,    // 页面打开
            CLICK, // 单击区域
        };
        __attribute__((unused)) static std::string GetEventLabel(Event event){
            if ( event == Event::DO ){
                return "DO";
            } else if ( event == Event::PO ){
                return "PO";
            } else if ( event == Event::CLICK ){
                return "CLICK";
            }
            return "";
        }

        namespace Movie{
            // 放映参数。
            enum class Operator {
                Play,   // 播放
                Stop,   // 停止
                Pause,  // 暂停
                Resume, // 继续
            };
            __attribute__((unused)) static std::string GetOperatorLabel(Operator op){
                if ( op == Operator::Play ){
                    return "Play";
                } else if ( op == Operator::Stop ){
                    return "Stop";
                } else if ( op == Operator::Pause ){
                    return "Pause";
                } else if ( op == Operator::Resume ){
                    return "Resume";
                }
                return "";
            }
        }; // Movie

    }; // namespace Action


    class CT_Action {
    public:

        // 事件类型
        Action::Event Event;

        CT_Region   Region; // 指定多个复杂区域为该链接对象的启动区域，
                            // 不出现时以所在图元或页面的外接矩形作为启动区域。

        struct Action{
        };

        // 跳转动作
        struct ActionGoto : public Action{
            CT_Dest Dest;     // 跳转的目标区域
            std::string Name; // 跳转的目标书签
        };

        // URI动作
        struct ActionURI : public Action{
            std::string URI;    // 目标URI的位置
            std::string Base;   // Base URI，用于相对地址。
            std::string Target;
        };

        // 附件动作
        struct ActionGotoA : public Action{
            uint64_t AttachID; // 附件的标识，必选。
            bool NewWindow;    // 是否在新窗口打开，可选

            ActionGotoA() : NewWindow(true){};
        };

        // 播放音频动作
        struct ActionSound : public Action{
            uint64_t ResourceID;  // 引用资源文件中的音频资源标识
            int      Value;       // 播放的音量，取值范围[0 100]，默认值100，可选。
            bool     Repeat;      // 此音频是否需要循环播放，默认false，可选。
                                  // 若此属性为true，则Synchronous值无效。
            bool     Synchronous; // 是否同步播放，true表示后续动作应等待此音频结束后才能开始，
                                  // false表示立刻返回并开始下一个动作。默认值为false，可选。
        };

        // 播放视频动作
        struct ActionMovie : public Action{
            uint64_t ResourceID; // 引用资源文件中定义的视频资源标识。

            // 放映参数，默认值为Play，可选。
            ofd::Action::Movie::Operator Operator;

            ActionMovie() : Operator(ofd::Action::Movie::Operator::Play){};
        };

        std::shared_ptr<Action> Action;

    }; // class CT_Action

}; // namespace ofd

#endif // __OFD_DEFINITIONS_H__

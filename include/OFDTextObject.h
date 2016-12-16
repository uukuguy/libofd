#ifndef __OFD_TEXTOBJECT_H__
#define __OFD_TEXTOBJECT_H__

#include <string>
#include <vector>
#include "OFDObject.h"
#include "OFDFont.h"

namespace ofd{

    namespace Text {

        // ======== class Text::TextCode ========
        // 标准 P64 页，Page.xsd。
        class TextCode {
        public:
            double      X;
            double      Y;
            DoubleArray DeltaX;
            DoubleArray DeltaY;
            std::string Text;

            TextCode(): X(0.0), Y(0.0){};
        }; // class TextCode

        // 阅读方向，可选值为0,90,180,270，默认值为0。
        enum class ReadDirection{
            ANGLE0,
            ANGLE90,
            ANGLE180,
            ANGLE270,
        };

        // 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
        enum class CharDirection{
            ANGLE0,
            ANGLE90,
            ANGLE180,
            ANGLE270,
        };

        // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
        enum class Weight{
            WEIGHT100,
            WEIGHT200,
            WEIGHT300,
            WEIGHT400,
            WEIGHT500,
            WEIGHT600,
            WEIGHT700,
            WEIGHT800,
            WEIGHT900,
        };

    }; // namespace Text

    // ======== class OFDTextObject ========
    // OFD P63，Page.xsd.
    class OFDTextObject : public OFDObject{
    public:
        OFDTextObject();
        virtual ~OFDTextObject();

        // 引用资源文件中定义的字型的标识。
        const OFDFontPtr GetFont() const;
        void SetFont(const OFDFontPtr font);

        // 字号，单位为毫米。
        double GetFontSize() const;
        void SetFontSize(double fontSize);

        // 是否勾边，默认值为false。
        bool IsStroke() const;
        void EnableStroke(bool bStroke);

        // 是否填充，默认值为true。
        bool IsFill() const; 
        void EnableFill(bool bFill);


        // 字型在水平方向的缩放比，默认值为1.0。
        // 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
        bool GetHScale() const;
        void SetHScale(double dHScale);

        // 阅读方向，可选值为0,90,180,270，默认值为0。
        Text::ReadDirection GetReadDirection() const;
        void SetReadDirection(Text::ReadDirection readDirection);

        // 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
        Text::CharDirection GetCharDirection() const;
        void SetCharDirection(Text::CharDirection charDirection);

        // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
        // 默认值为400。
        Text::Weight GetWeight() const;
        void SetWeight(Text::Weight weight);

        // 是否是斜体样式，默认值为false。
        bool IsItalic() const;
        void SetItalic(bool bItalic);

        // 填充色，默认值为黑色。
        OFDColorPtr GetFillColor() const;
        void SetFillColor(OFDColorPtr fillColor);

        // 勾边色，默认值为透明色。
        OFDColorPtr GetStrokeColor() const;
        void SteStrokeColor(OFDColorPtr strokeColor);

        // TODO
        //CT_CGTransform CGTransform; // 指定字符编码到字符索引之间的变换关系。


        size_t GetTextCodesCount() const;
        const Text::TextCode& GetTextCode(size_t idx) const;
        Text::TextCode& GetTextCode(size_t idx);

        void AddTextCode(const Text::TextCode &textCode);
        void ClearTextCodes();

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDTextObject
    typedef std::shared_ptr<OFDTextObject> OFDTextObjectPtr;

}; // namespace ofd

#endif // __OFD_TEXTOBJECT_H__

#ifndef __OFD_TEXTOBJECT_H__
#define __OFD_TEXTOBJECT_H__

#include <string>
#include <vector>
#include "OFDObject.h"

namespace ofd{

    namespace Text {

        // ======== class Text::TextCode ========
        class TextCode {
        public:
            double      X;
            double      Y;
            DArray      DeltaX;
            DArray      DeltaY;
            std::string Text;
        }; // class TextCode

    }; // namespace Text

    // ======== class OFDTextObject ========
    class OFDTextObject : public OFDObject{
    public:
        OFDTextObject();
        virtual ~OFDTextObject();

        size_t GetTextCodesCount() const;
        const Text::TextCode& GetTextCode(size_t idx) const;
        Text::TextCode& GetTextCode(size_t idx);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // class OFDTextObject

}; // namespace ofd

#endif // __OFD_TEXTOBJECT_H__

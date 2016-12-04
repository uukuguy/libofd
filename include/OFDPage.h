#ifndef __OFD_PAGE_H__
#define __OFD_PAGE_H__

#include <memory>
#include <string>

namespace ofd{

    class OFDPage{
    public:
        OFDPage();
        virtual ~OFDPage();

        std::string to_string() const;

        bool Open();
        void Close();
        bool IsOpened() const;

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;

    }; // OFDPage

}; // namespace ofd

#endif // __OFD_PAGE_H__

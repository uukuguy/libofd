#ifndef __UTILS_UTILS_H__
#define __UTILS_UTILS_H__

#include <memory>
#include <tuple>
#include <string>
#include <vector>
#include <sstream>
// for PRIu64
#include <inttypes.h>

namespace utils{

    std::vector<std::string> SplitString(const std::string& content);

    void SetStringStreamPrecision(std::stringstream &ss, int precision);

    template<typename T, typename... Ts>
    std::unique_ptr<T> make_unique(Ts&&... params){
        return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
    }


    std::tuple<char*, size_t, bool> ReadFileData(const std::string &filename);
    bool WriteFileData(const std::string &filename, const char *data, size_t dataSize); 
}


#endif // __UTILS_UTILS_H__

#ifndef __UTILS_UTILS_H__
#define __UTILS_UTILS_H__

#include <string>
#include <vector>
#include <sstream>

namespace utils{

    std::vector<std::string> SplitString(const std::string& content);

    void SetStringStreamPrecision(std::stringstream &ss, int precision);

}

#endif // __UTILS_UTILS_H__

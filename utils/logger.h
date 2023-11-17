#ifndef __UTILS_LOGGER_H__
#define __UTILS_LOGGER_H__

#include "easylogging++.h"

class Logger {
public:
    //static void Initialize(int argc, char *argv[]);
    static void Initialize(int loggerLevel = 0);
}; // class Logger

#endif // __UTILS_LOGGER_H__

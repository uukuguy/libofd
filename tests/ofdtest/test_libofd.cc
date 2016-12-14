#include "OFDFile.h"
#include "utils/logger.h"

using namespace ofd;

void test_libofd(int argc, char *argv[]){
    OFDFile ofdFile;
    ofdFile.Open(argv[1]);
}

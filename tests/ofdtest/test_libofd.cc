#include "OFDPackage.h"
#include "utils/logger.h"

using namespace ofd;

void test_libofd(int argc, char *argv[]){
    OFDPackagePtr ofdPackage = std::make_shared<OFDPackage>();
    ofdPackage->Open(argv[1]);
}

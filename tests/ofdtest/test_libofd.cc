#include "ofd/Package.h"
#include "utils/logger.h"

using namespace ofd;

void test_libofd(int argc, char *argv[]){
    PackagePtr package = std::make_shared<Package>();
    package->Open(argv[1]);
}

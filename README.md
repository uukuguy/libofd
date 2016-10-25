# libofd
The first open-source C++ development library for OFD.

"Electronic files storage and exchange formats - Fixed layout documents", referred to as OFD (Open Fixed-Layout Document), is China's independent research and development of fixed-layout format standard, is an important part of the national standard system of electronic documents.  

October 13, 2016, the official release of the OFDnational standard, the standard number: 33190-2016. GB/T standard query site http://www.sac.gov.cn/.

The libofd is the first open-source C++ development library for OFD. It was established in October 20, 2016, only 1 week later than the standard release time. 

## Dependencies

Cairo http://cairographics.org

tinyxml2 https://github.com/leethomason/tinyxml2  

libzip https://github.com/nih-at/libzip 

easylogging++ https://github.com/easylogging/easyloggingpp

## Build

$ git clone https://github.com/uukuguy/libofd.git

$ cd libofd

$ mkdir build && cd build && cmake .. && make

## Test

$ cd build

### For production.
$ ./bin/ofdtest ../data/sample0.ofd 

### For debug in production.
$ ./bin/ofdtest ../data/sample0.ofd --v=1

### for debug in development.
$ ./bin/ofdtest ../data/sample0.ofd --v=3

### for detail debug info in development. (maybe too slow).
$ ./bin/ofdtest ../data/sample0.ofd --v=5


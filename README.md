# libofd
The first open-source C++ development library for OFD.

"Electronic files storage and exchange formats - Fixed layout documents", referred to as OFD (Open Fixed-Layout Document), is China's independent research and development of fixed-layout format standard, is an important part of the national standard system of electronic documents.  

October 13, 2016, the official release of the OFDnational standard, the standard number: 33190-2016. GB/T standard query site http://www.sac.gov.cn/.

The libofd is the first open-source C++ development library for OFD. It was established in October 20, 2016, only 1 week later than the standard release time. 

## Dependencies

libcairo2-dev http://cairographics.org

libtinyxml2-dev https://github.com/leethomason/tinyxml2  

libzip-dev https://github.com/nih-at/libzip 

easylogging++ https://github.com/easylogging/easyloggingpp

libsdl2-dev
libsdl2-image-dev
libsdl2-ttf-dev
libsdl2-mixer-dev
libsdl2-net-dev
libsdl2-gfx-dev

libharfbuzz-dev
libicu-dev

libfreetype6-dev

## Build

```
$ git clone https://github.com/idleuncle/libofd.git
$ cd libofd
$ mkdir build && cd build && cmake .. && make
```

## Test

```
$ cd build
$ ./bin/ofdviewer ../data/sample0.ofd --v=1
```


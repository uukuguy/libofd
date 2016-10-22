# libofd
OFD SDK

## Dependencies

tinyxml2 https://github.com/leethomason/tinyxml2  

libzip https://github.com/nih-at/libzip 

easylogging++ https://github.com/easylogging/easyloggingpp

## Build

$ git clone https://github.com/uukuguy/libofd.git

$ cd libofd

$ mkdir build && cd build && cmake .. && make

## Test

$ cd build

# For production.
$ ./bin/ofdtest ../data/sample0.ofd 

# For debug in production.
$ ./bin/ofdtest ../data/sample0.ofd --v=1

# for debug in development.
$ ./bin/ofdtest ../data/sample0.ofd --v=3


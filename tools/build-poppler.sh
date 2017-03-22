#!/bin/bash
POPPLER=poppler-0.53.0
rootdir=$(cd $(dirname $0); pwd)
cd $rootdir
if [ ! -d $rootdir/poppler ]; then
    tar zxvf $POPPLER.tar.gz && ln -s $POPPLER poppler &&  cd poppler && \
        ./configure && make
fi

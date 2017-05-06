#!/bin/bash
#wget https://github.com/fontforge/fontforge/releases/download/20161005/fontforge-20161005.tar.gz

FONTFORGE=fontforge-20161005
rootdir=$(cd $(dirname $0); pwd)
cd $rootdir
if [ ! -d $rootdir/fontforge ]; then
    tar zxvf $FONTFORGE.tar.gz && ln -s $FONTFORGE fontforge &&  cd fontforge && \
        ./configure && make
fi

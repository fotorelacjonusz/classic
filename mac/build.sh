#!/bin/bash

cd ${0%/*}

build_dir='/absolute/path/to/build/dir'
trunk_dir=`readlink -f ..`
drobpox_dir='/absolute/path/to/dropbox/folder/fotorelacjonusz'

cd $trunk_dir
svn update

cd $build_dir
qmake-qt4 $trunk_dir/fotorelacjonusz.pro -r -spec linux-g++
lrelease $trunk_dir/fotorelacjonusz.pro
make clean -w
make -w

macdeployqt fotorelacjonusz.app

cd fotorelacjonusz.app/Contents
version=`./MacOS/fotorelacjonusz -v`

xmlstarlet ed -L -s '/plist/dict' -t elem -n 'key' -v 'CFBundleShortVersionString' Info.plist
xmlstarlet ed -L -s '/plist/dict' -t elem -n 'string' -v "$version" Info.plist

#icons:
icon_512=$trunk_dir/res/ssc_logo_fotorelacjonusz_512.png
icon_256=$trunk_dir/res/ssc_logo_256.png
icon_96=$trunk_dir/res/ssc_logo.png 
#96 nie różni się od 256, więc jeśli zeskaluje sobie w dół, to nie używaj 96

#...



cd $build_dir
cp -r fotorelacjonusz.app $dropbox_dir/fotorelacjonusz_$version.app


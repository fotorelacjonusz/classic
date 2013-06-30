#!/bin/bash

cd ${0%/*}

build_dir='../../../fotorelacjonusz2-build-desktop-Qt_4_8_1_w_PATH__System__Release'
trunk_dir=`readlink -f ..`

(
cd $build_dir
qmake-qt4 $trunk_dir/fotorelacjonusz.pro -r -spec linux-g++
lrelease $trunk_dir/fotorelacjonusz.pro
make clean -w
make -w
)

mkdir -p debian/DEBIAN
mkdir -p debian/usr/bin/
mkdir -p debian/usr/share/doc/fotorelacjonusz
mkdir -p debian/usr/share/icons
mkdir -p debian/usr/share/applications

cp $build_dir/fotorelacjonusz debian/usr/bin/
cp $trunk_dir/res/ssc_logo.png debian/usr/share/icons/fotorelacjonusz_ssc_logo.png
cp copyright changelog* debian/usr/share/doc/fotorelacjonusz/

strip debian/usr/bin/fotorelacjonusz

version=`debian/usr/bin/fotorelacjonusz -v`
sed control -e "s/^Version:.*$/Version: $version/" > debian/DEBIAN/control
sed fotorelacjonusz.desktop -e "s/^Version=.*$/Version=$version/" > debian/usr/share/applications/fotorelacjonusz.desktop

gzip -f --best debian/usr/share/doc/fotorelacjonusz/changelog
gzip -f --best debian/usr/share/doc/fotorelacjonusz/changelog.Debian

find ./debian -type d | xargs chmod 0755
find ./debian -type f | xargs chmod 0644
chmod 0755 debian/usr/bin/fotorelacjonusz

fakeroot dpkg-deb --build debian .

lintian *.deb

rm -r debian

fakeroot alien -r fotorelacjonusz_${version}*.deb


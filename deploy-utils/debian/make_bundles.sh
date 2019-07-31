#!/bin/bash

set -ex

pushd ${0%/*}

trunk_dir=`readlink -f ../..`
release_dir="$trunk_dir/release"

mkdir -p $release_dir

mkdir -p debian/DEBIAN
mkdir -p debian/usr/bin/
mkdir -p debian/usr/share/doc/fotorelacjonusz
mkdir -p debian/usr/share/icons
mkdir -p debian/usr/share/applications

cp $trunk_dir/fotorelacjonusz debian/usr/bin/
cp $trunk_dir/res/ssc_logo.png debian/usr/share/icons/fotorelacjonusz_ssc_logo.png
cp copyright changelog* debian/usr/share/doc/fotorelacjonusz/
cp fotorelacjonusz.desktop debian/usr/share/applications/fotorelacjonusz.desktop
cp control debian/DEBIAN/control

strip debian/usr/bin/fotorelacjonusz

architecture=`dpkg --print-architecture`
version=`debian/usr/bin/fotorelacjonusz -v | tr '-' '~'`

sed debian/DEBIAN/control -i -e "s/^Version:.*$/Version: $version/"
sed debian/DEBIAN/control -i -e "s/^Architecture:.*$/Architecture: $architecture/"
sed debian/usr/share/applications/fotorelacjonusz.desktop -i -e "s/^Version=.*$/Version=$version/"

gzip -f --best debian/usr/share/doc/fotorelacjonusz/changelog
gzip -f --best debian/usr/share/doc/fotorelacjonusz/changelog.Debian

find ./debian -type d | xargs chmod 0755
find ./debian -type f | xargs chmod 0644
chmod 0755 debian/usr/bin/fotorelacjonusz

fakeroot dpkg-deb --build debian .

lintian fotorelacjonusz_*.deb

rm -r debian

fakeroot alien --to-rpm fotorelacjonusz_*.deb

mv *.deb *.rpm $release_dir

popd

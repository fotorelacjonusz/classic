#!/bin/bash

set -ex

pushd ${0%/*}

trunk_dir=`readlink -f ..`
items="$trunk_dir/deploy-utils/debian"
target="$trunk_dir/debian"
release_dir="$trunk_dir/packages"

mkdir -p $release_dir

# Target structure
mkdir -p $target/DEBIAN
mkdir -p $target/usr/bin/
mkdir -p $target/usr/share/doc/fotorelacjonusz
mkdir -p $target/usr/share/icons
mkdir -p $target/usr/share/applications

# Copy items
cp $trunk_dir/fotorelacjonusz $target/usr/bin/
cp $trunk_dir/res/ssc_logo.png $target/usr/share/icons/fotorelacjonusz_ssc_logo.png
cp $items/copyright $target/usr/share/doc/fotorelacjonusz/
cp $items/changelog* $target/usr/share/doc/fotorelacjonusz/
cp $items/fotorelacjonusz.desktop $target/usr/share/applications/fotorelacjonusz.desktop
cp $items/control $target/DEBIAN/control

strip $target/usr/bin/fotorelacjonusz

architecture=`dpkg --print-architecture`
version=`$target/usr/bin/fotorelacjonusz -v | tr '-' '~'`

sed $target/DEBIAN/control -i -e "s/^Version:.*$/Version: $version/"
sed $target/DEBIAN/control -i -e "s/^Architecture:.*$/Architecture: $architecture/"
sed $target/usr/share/applications/fotorelacjonusz.desktop -i -e "s/^Version=.*$/Version=$version/"

gzip -f --best $target/usr/share/doc/fotorelacjonusz/changelog
gzip -f --best $target/usr/share/doc/fotorelacjonusz/changelog.Debian

find $target -type d | xargs chmod 0755
find $target -type f | xargs chmod 0644
chmod 0755 $target/usr/bin/fotorelacjonusz

fakeroot dpkg-deb --build $target .

lintian fotorelacjonusz_*.deb

rm -r $target

fakeroot alien --to-rpm fotorelacjonusz_*.deb

mv *.deb *.rpm $release_dir

popd

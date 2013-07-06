#!/bin/bash

cd ${0%/*}

trunk_dir=`readlink -f ..`
dropbox_dir=~/Dropbox/fotorelacjonusz
sreplace="$trunk_dir/../../sreplace-build-desktop-Qt_4_8_1_w_PATH__System__Release/sreplace"

latest=`ls -t1 $dropbox_dir | head -n 1`
size=`du -sh $dropbox_dir/$latest | cut -f 1`


echo $latest
echo $size

if [[ $size != *M ]] ; then 
	echo "Rozmiar $latest nie jest w MB: $size"
	exit 1
fi

size=${size%?}
(( ++size ))
echo $size

name=${latest%.app}
version=${name#*_}

echo $name
echo $version

#exit 0

dd if=/dev/zero of=$name.dmg bs=1M count=$size
mkfs.hfsplus -v $name $name.dmg

#ln -fs `pwd`/$name.dmg /tmp/mountdmg-image.dmg
ln -fs `readlink -f $name.dmg` /tmp/mountdmg-image.dmg
mount $name.dmg

cp -r $dropbox_dir/$latest /mnt/dmg/
$sreplace /mnt/dmg/$latest/Contents/MacOs/fotorelacjonusz
df -B1 | grep '/mnt/dmg'
umount /mnt/dmg



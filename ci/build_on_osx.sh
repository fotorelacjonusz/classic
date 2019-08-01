#!/bin/bash

set -e

# QtFtp
travis_fold start "install.qtftp"
set -x
pushd third-party/qtftp
qmake CONFIG+=staticlib CONFIG+=c++11 CONFIG+=release
make
popd
set +x
travis_fold end "install.qtftp"

# QuaZIP
travis_fold start "install.quazip"
set -x
pushd third-party/quazip
qmake CONFIG+=staticlib CONFIG+=c++11 CONFIG+=release
make
popd
set +x
travis_fold end "install.quazip"

# Fotorelacjonusz
travis_fold start "install.fotorelacjonusz"
set -x
ci/compile_icon_set.sh
lrelease fotorelacjonusz.pro
qmake CONFIG+=release LIBS+=-lz
make
set +x
travis_fold end "install.fotorelacjonusz"

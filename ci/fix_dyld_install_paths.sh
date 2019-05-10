#!/bin/bash

# Unfortunately, macdeployqt from Homebrew package does really poor job here:
# it uses absolute paths for linking QtWebEngineProcess.app, preventing
# the Fotorelacjonusz.app from working on computers which don't have Qt
# installed.
#
# This tiny script fixes that by replacing dylib install paths with correct
# ones, pointing to dylibs which are contained in the app bundle.

set -x -e

pushd Fotorelacjonusz.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS

for LIB in QtGui QtCore QtWebEngineCore QtQuick QtWebChannel QtQml QtNetwork QtPositioning
do
	OLD_PATH=`otool -L QtWebEngineProcess | grep ${LIB} | cut -f 1 -d ' '`
	NEW_PATH="@loader_path/../../../../../${LIB}.framework/${LIB}"
	install_name_tool -change ${OLD_PATH} ${NEW_PATH} QtWebEngineProcess
done

popd

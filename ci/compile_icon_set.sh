#!/bin/bash

set -e

echo "Compiling icon set..."

case "${TRAVIS_OS_NAME}" in
	osx)
		iconutil -c icns appicon.iconset
		;;
esac

echo "Done compiling icon set."

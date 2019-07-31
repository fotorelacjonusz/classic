#!/bin/bash

set -e

travis_fold start "before_deploy.package"
echo "Packing application"

rm -rf ./release

case "${TRAVIS_OS_NAME}" in
	osx)
		PACKAGE_NAME="Fotorelacjonusz-${TRAVIS_TAG}-macos"
		mkdir -p release
		macdeployqt Fotorelacjonusz.app -verbose=2
		tar czf "release/${PACKAGE_NAME}.tar.gz" Fotorelacjonusz.app
		;;
esac

travis_fold end "before_deploy.package"

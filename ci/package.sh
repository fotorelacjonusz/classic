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

	linux)
		PACKAGE_NAME="Fotorelacjonusz-${TRAVIS_TAG}-linux-${ARCH}"
		# Build artifacts in a container
		docker run -t --name dummy ${DOCKER_IMAGE} deploy-utils/debian/make_bundles.sh
		docker cp dummy:/build/release ./release
		docker rm -fv dummy
		mv release/*.deb "release/${PACKAGE_NAME}.deb"
		mv release/*.rpm "release/${PACKAGE_NAME}.rpm"
		;;

esac

travis_fold end "before_deploy.package"

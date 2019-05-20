#!/bin/bash

set -e

travis_fold start "before_deploy.package"
echo "Packing application"

case "${TRAVIS_OS_NAME}" in
	osx)
		macdeployqt Fotorelacjonusz.app -verbose=2
		tar czf "${PACKAGE_NAME}" Fotorelacjonusz.app
		;;
esac

travis_fold end "before_deploy.package"

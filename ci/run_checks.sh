#!/bin/bash

set -e

echo "Running basic checks"

case "${TRAVIS_OS_NAME}" in
	osx)
		open ./Fotorelacjonusz.app --args -v
		;;

	linux)
		docker run -it ${DOCKER_IMAGE} /build/fotorelacjonusz -v
		;;
esac

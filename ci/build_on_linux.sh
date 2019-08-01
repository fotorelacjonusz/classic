#!/bin/bash

set -e

# Build image
travis_fold start "install.docker.image"
docker build -t ${DOCKER_IMAGE} -f deploy-utils/debian/ubuntu.dockerfile --build-arg BASE_IMAGE=$BASE_DOCKER_IMAGE .
travis_fold end "install.docker.image"

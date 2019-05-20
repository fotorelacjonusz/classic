#!/bin/bash

set -e

if [[ ${TRAVIS_TAG} == "nightly" ]]; then
	git tag -a -f -m "Created by Travis" $TRAVIS_TAG
	git push -f --tags
else
	echo "Not a nightly build."
fi

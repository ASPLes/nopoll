#!/bin/bash

set -x

echo "Preparing sources"
rm -f rpm/SOURCES/*.tar.gz
make dist
cp nopoll-`cat VERSION`.tar.gz rpm/SOURCES

echo "Calling to compile packages.."
LANG=C rpmbuild -ba --define '_topdir /usr/src/nopoll/trunk/rpm' rpm/SPECS/nopoll.spec

echo "Output ready at rpm/RPMS"
find rpm/RPMS -type f


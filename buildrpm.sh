#!/bin/bash

set -x

echo "Preparing sources"
rm -rf rpm/SOURCES/*.tar.gz
rm -rf rpm/BUILD/*
rm -rf rpm/BUILDROOT/*
make dist
cp nopoll-`cat VERSION`.tar.gz rpm/SOURCES

echo "Calling to compile packages.."
LANG=C rpmbuild -ba --define '_topdir /usr/src/nopoll/trunk/rpm' rpm/SPECS/nopoll.spec

echo "Output ready at rpm/RPMS"
find rpm/RPMS -type f



#!/bin/sh

python /usr/src/freebsd-pkg-builder/pkg-builder.py /usr/src/nopoll/trunk /usr/src/nopoll/trunk/freebsd/arch/ --skip-build --version=`cat /usr/src/nopoll/trunk/VERSION` --description="WebSocket OpenSource implementation" --outdir=/usr/src/freebsd-packages


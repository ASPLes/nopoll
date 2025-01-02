#!/bin/bash

rm -rf doc/html
rm -rf debian/tmp
rm -rf debian/libnopoll0-dev

# find all files that have copy right declaration associated to Aspl that don't have 
# the following declaration year
current_year="2025"
LANG=C rgrep "Copyright" doc src web debian-files test rpm 2>&1 | grep "Advanced" | grep -v "Permission denied" | grep -v '~:' | grep -v '/\.svn/' | grep -v "${current_year}"

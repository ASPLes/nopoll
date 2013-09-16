#!/bin/bash
rsync --exclude=.svn --exclude=update-web.sh -avz *.css *.html aspl-web@www.aspl.es:www/nopoll/
rsync --exclude=.svn --exclude=update-web.sh -avz es/*.html aspl-web@www.aspl.es:www/nopoll/es/
rsync --exclude=.svn --exclude=update-web.sh -avz images/*.png images/*.gif aspl-web@www.aspl.es:www/nopoll/images/




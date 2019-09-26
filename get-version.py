#!/usr/bin/python

from core_admin_common import command, support
import sys

import time
start = time.time ()
print "INFO: wait to avoid overwhelming github.."
# implement a random wait to avoid Too many requests error from github.com
from random import randint
from time import sleep
sleep (randint(1,20))
print "INFO: wait done (%d seconds waited).." % (time.time () - start)

(osname, oslongname, osversion) = support.get_os ()
release_name = osversion.split (" ")[1]
no_github_com_access = ["lenny", "squeeze"]

if release_name in no_github_com_access:
    command.run ("cp -f LATEST-VERSION VERSION")
    sys.exit (0)
# end if

(status, info) = command.run ("LANG=C svn update . | grep revision")
if status:
    print "ERROR: unable to get subversion version: %s" % info
    sys.exit (-1)

# get versision
revision = info.split (" ")[2].replace (".", "").strip ()
print "INFO: Revision found: %s" % revision

version = open ("VERSION").read ().split (".b")[0].strip ()
version = "%s.b%s" % (version, revision)
print "INFO: Updated vesion to: %s" % version

open ("VERSION", "w").write ("%s\n" % version)
open ("LATEST-VERSION", "w").write ("%s\n" % version)

# also update Changelog
command.run ("svn log > Changelog")




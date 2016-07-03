#!/usr/bin/python

from core_admin_common import command
import sys

(status, info) = command.run ("LANG=C svn update . | grep 'At revision'")
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

# also update Changelog
command.run ("svn log > Changelog")





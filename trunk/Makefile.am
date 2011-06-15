SUBDIRS = src  test doc
EXTRA_DIST = nopoll.pc.in VERSION 
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nopoll.pc

update-defs:
	cd src; make update-def; cd ..

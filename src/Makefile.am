EXTRA_DIST = libnopoll.def

if ENABLE_NOPOLL_LOG
LOG = -DSHOW_DEBUG_LOG
endif

INCLUDES = -Wall   -Wstrict-prototypes -g -I$(top_srcdir) $(LIBRARIES_CFLAGS) -DVERSION=\""$(NOPOLL_VERSION)"\" \
	-DPACKAGE_DTD_DIR=\""$(datadir)"\" -DPACKAGE_TOP_DIR=\""$(top_srcdir)"\" -Wall -ansi \
	-DVERSION=\"$(NOPOLL_VERSION)\" $(LOG)

libnopollincludedir = $(includedir)/nopoll

lib_LTLIBRARIES = libnopoll.la

libnopoll_la_SOURCES     = nopoll_decl.c
libnopollinclude_HEADERS = nopoll.h nopoll_decl.h

libnopoll_la_LDFLAGS = -no-undefined -export-symbols-regex '^(nopoll|__nopoll|_nopoll).*'

# libnopoll_la_LIBADD = -lm

libnopoll.def: update-def

update-def:
	echo "EXPORTS" > libnopoll.def
	cat .libs/libnopoll.exp >> libnopoll.def


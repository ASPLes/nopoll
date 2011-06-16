EXTRA_DIST = libnopoll.def

if ENABLE_NOPOLL_LOG
LOG = -DSHOW_DEBUG_LOG
endif

INCLUDES = -Wall   -Wstrict-prototypes -g -I$(top_srcdir) $(LIBRARIES_CFLAGS) -DVERSION=\""$(NOPOLL_VERSION)"\" \
	-DPACKAGE_DTD_DIR=\""$(datadir)"\" -DPACKAGE_TOP_DIR=\""$(top_srcdir)"\" -Wall -ansi \
	-DVERSION=\"$(NOPOLL_VERSION)\" $(LOG)

libnopollincludedir = $(includedir)/nopoll

lib_LTLIBRARIES = libnopoll.la

libnopoll_la_SOURCES     = \
	nopoll_decl.c \
	nopoll_ctx.c \
	nopoll_conn.c \
	nopoll_log.c \
	nopoll_listener.c \
	nopoll_loop.c \
	nopoll_io.c

libnopollinclude_HEADERS = \
	nopoll.h \
	nopoll_config.h \
	nopoll_decl.h \
	nopoll_private.h \
	nopoll_ctx.h \
	nopoll_conn.h \
	nopoll_log.h \
	nopoll_listener.h \
	nopoll_loop.h \
	nopoll_io.h

libnopoll_la_LDFLAGS = -no-undefined -export-symbols-regex '^(nopoll|__nopoll|_nopoll).*'

# libnopoll_la_LIBADD = -lm

libnopoll.def: update-def

update-def:
	echo "EXPORTS" > libnopoll.def
	cat .libs/libnopoll.exp >> libnopoll.def


EXTRA_DIST = libnopoll.def Makefile.win

if ENABLE_NOPOLL_LOG
LOG = -DSHOW_DEBUG_LOG
endif

INCLUDES = $(compiler_options) -I$(top_srcdir) $(LIBRARIES_CFLAGS) -DVERSION=\""$(NOPOLL_VERSION)"\" \
	-DPACKAGE_DTD_DIR=\""$(datadir)"\" -DPACKAGE_TOP_DIR=\""$(top_srcdir)"\" \
	-DVERSION=\"$(NOPOLL_VERSION)\" $(LOG)

libnopollincludedir = $(includedir)/nopoll

lib_LTLIBRARIES = libnopoll.la

libnopoll_la_SOURCES     = \
	nopoll.c \
	nopoll_decl.c \
	nopoll_ctx.c \
	nopoll_conn.c \
	nopoll_log.c \
	nopoll_listener.c \
	nopoll_loop.c \
	nopoll_io.c \
	nopoll_msg.c \
	nopoll_win32.c

libnopollinclude_HEADERS = \
	nopoll.h \
	nopoll_handlers.h \
	nopoll_config.h \
	nopoll_decl.h \
	nopoll_private.h \
	nopoll_ctx.h \
	nopoll_conn.h \
	nopoll_log.h \
	nopoll_listener.h \
	nopoll_loop.h \
	nopoll_io.h \
	nopoll_msg.h \
	nopoll_win32.h

libnopoll_la_LDFLAGS = -no-undefined -export-symbols-regex '^(nopoll|__nopoll|_nopoll).*'

libnopoll_la_LIBADD = $(TLS_LIBS)

libnopoll.def: update-def

update-def:
	echo "EXPORTS" > libnopoll.def
	cat .libs/libnopoll.exp >> libnopoll.def


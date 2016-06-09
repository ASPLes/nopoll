EXTRA_DIST = Makefile.win test-certificate.crt test-private.key client.pem server.pem root.pem

# still nothing
if ENABLE_NOPOLL_LOG
LOG = -DSHOW_DEBUG_LOG
endif

AM_CPPFLAGS = -DTEST_DIR=$(top_srcdir)/test -I$(top_srcdir)/src/ $(compiler_options) $(LOG) -DVERSION=\""$(NOPOLL_VERSION)"\" -D__NOPOLL_PTHREAD_SUPPORT__=1

# replace with bin_PROGRAMS to check performance
noinst_PROGRAMS = nopoll-regression-client nopoll-regression-listener
TESTS = nopoll-regression-client nopoll-regression-listener

nopoll_regression_client_SOURCES = nopoll-regression-client.c nopoll-regression-common.c nopoll-regression-common.h
nopoll_regression_client_LDADD   = $(top_builddir)/src/libnopoll.la $(TLS_LIBS)

nopoll_regression_listener_SOURCES = nopoll-regression-listener.c nopoll-regression-common.c nopoll-regression-common.h
nopoll_regression_listener_LDADD   = $(top_builddir)/src/libnopoll.la $(TLS_LIBS)

leak-check:
	libtool --mode=execute valgrind --leak-check=yes ./test_01


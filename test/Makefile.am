# still nothing
if ENABLE_NOPOLL_LOG
LOG = -DSHOW_DEBUG_LOG
endif

INCLUDES = -DTEST_DIR=$(top_srcdir)/test -I$(top_srcdir)/src/ -Wall $(LOG) -DVERSION=\""$(NOPOLL_VERSION)"\"

# replace with bin_PROGRAMS to check performance
noinst_PROGRAMS = test_01
TESTS = test_01

test_01_SOURCES = test_01.c
test_01_LDADD   = $(top_builddir)/src/libnopoll.la $(NS_LIB) $(BABEL_LIB)

leak-check:
	libtool --mode=execute valgrind --leak-check=yes ./test_01


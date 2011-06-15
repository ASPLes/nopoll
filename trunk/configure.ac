dnl  Copyright (C) 2011 Advanced Software Production Line, S.L.
dnl
dnl  This program is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2.1 of the License, or (at your option) any later
dnl  version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public
dnl  License along with this program; if not, write to the Free
dnl  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
dnl  02111-1307 USA
dnl  
dnl  You may find a copy of the license under this software is
dnl  released at COPYING file. This is LGPL software: you are wellcome
dnl  to develop propietary applications using this library withtout
dnl  any royalty or fee but returning back any change, improvement or
dnl  addition in the form of source code, project image, documentation
dnl  patches, etc.
dnl
dnl  For commercial support on build XML enabled solutions contact us:
dnl          
dnl      Postal address:
dnl         Advanced Software Production Line, S.L.
dnl         Edificio Alius A, Oficina 102,
dnl         C/ Antonio Suarez Nº 10,
dnl         Alcalá de Henares 28802 Madrid
dnl         Spain
dnl
dnl      Email address:
dnl         info@aspl.es - http://www.aspl.es/xml
dnl
AC_INIT(src/nopoll.h)

configure_dir=`dirname "$0"`
AXL_VERSION=`cat $configure_dir/VERSION`
AC_SUBST(AXL_VERSION)

AC_CONFIG_AUX_DIR(.)
AM_INIT_AUTOMAKE(nopoll, $AXL_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# check type sizes
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(void *)

dnl get site dinamic library extension
SHARED_EXTENSION=$shrext_cmds
AC_SUBST(SHARED_EXTENSION)

dnl check for poll support
AC_CHECK_HEADER(sys/poll.h, enable_poll=yes, enable_poll=no)
AM_CONDITIONAL(ENABLE_POLL_SUPPORT, test "x$enable_poll" = "xyes")

dnl Check for the Linux epoll interface; epoll* may be available in libc
dnl with Linux kernels 2.6.X
AC_CACHE_CHECK([for epoll(2) support], [enable_cv_epoll],
[AC_TRY_RUN([
#include <sys/epoll.h>
#include <unistd.h>

int main()
{
    return epoll_create(5) == -1;
}], [enable_cv_epoll=yes], [enable_cv_epoll=no], [enable_cv_epoll=no])])
AM_CONDITIONAL(ENABLE_EPOLL_SUPPORT, test "x$enable_cv_epoll" = "xyes")

dnl select the best I/O platform
if test x$enable_cv_epoll = xyes ; then
   default_platform="epoll"
elif test x$enable_poll = xyes ; then
   default_platform="poll"
else 
   default_platform="select"
fi
AM_CONDITIONAL(DEFAULT_EPOLL, test "x$default_platform" = "xepoll")
AM_CONDITIONAL(DEFAULT_POLL, test "x$default_platform" = "xpoll")

AC_ARG_ENABLE(nopoll-log, [  --enable-nopoll-log     Enable building Nopoll with debug log support [default=yes]], enable_nopoll_log="$enableval", enable_nopoll_log=yes)
AM_CONDITIONAL(ENABLE_NOPOLL_LOG, test "x$enable_nopoll_log" = "xyes")


AC_OUTPUT([
Makefile
src/Makefile
doc/Makefile
test/Makefile
nopoll.pc])

echo "------------------------------------------"
echo "--       LibNoPoll LIBRARY SETTINGS     --"
echo "------------------------------------------"
echo "   Installation prefix:            [$prefix]"
echo "      select(2) support:           [yes]"
echo "      poll(2) support:             [$enable_poll]"
echo "      epoll(2) support:            [$enable_cv_epoll]"
echo "------------------------------------------"
echo "--     NOW TYPE: make; make install     --"
echo "------------------------------------------"
echo "--     Mira papa! Sin manos!!!          --"
echo "------------------------------------------"

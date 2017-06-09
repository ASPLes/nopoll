dnl LibNoPoll: A websocket library
dnl  Copyright (C) 2017 Advanced Software Production Line, S.L.
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
dnl  For commercial support on build Websocket enabled solutions
dnl  contact us:
dnl          
dnl      Postal address:
dnl         Advanced Software Production Line, S.L.
dnl         Av. Juan Carlos I, Nº13, 2ºC
dnl         Alcalá de Henares 28806 Madrid
dnl         Spain
dnl
dnl      Email address:
dnl         info@aspl.es - http://www.aspl.es/nopoll
dnl
AC_INIT(nopoll, m4_esyscmd([cat VERSION | tr -d '\n']))
export NOPOLL_VERSION=`cat $srcdir/VERSION`
AC_SUBST(NOPOLL_VERSION)

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/nopoll.h])
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

compiler_options=""
STRICT_PROTOTYPES=""
echo "Detected compiler: $compiler"
if test "$compiler" = "gcc" ; then
   compiler_options="-Wstrict-prototypes -Wall -Werror -ansi"
   echo "Detected gcc compiler: $compiler, adding options: $compiler_options"
fi
AC_SUBST(compiler_options)

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

dnl
dnl Thread detection support mostly taken from the apache project 2.2.3.
dnl
dnl NOPOLL_PTHREADS_TRY_RUN(actions-if-success)
dnl
dnl Try running a program which uses pthreads, executing the
dnl actions-if-success commands on success.
dnl
AC_DEFUN([NOPOLL_PTHREADS_TRY_RUN], [
AC_TRY_RUN( [
#include <pthread.h>
#include <stddef.h>

void *thread_routine(void *data) {
    return data;
}

int main() {
    pthread_t thd;
    pthread_mutexattr_t mattr;
    pthread_once_t once_init = PTHREAD_ONCE_INIT;
    int data = 1;
    pthread_mutexattr_init(&mattr);
    return pthread_create(&thd, NULL, thread_routine, &data);
} ], [nopoll_p_t_r=yes], [nopoll_p_t_r=no], [nopoll_p_t_r=no])

if test $nopoll_p_t_r = yes; then
  $1
fi

])dnl


dnl
dnl NOPOLL_PTHREADS_CHECK()
dnl
dnl Try to find a way to enable POSIX threads.  Sets the 
dnl pthreads_working variable to "yes" on success.
dnl
AC_DEFUN([NOPOLL_PTHREADS_CHECK],[

AC_CACHE_CHECK([for CFLAGS needed for pthreads], [nopoll_cv_pthreads_cflags],
[nopoll_ptc_cflags=$CFLAGS
 for flag in -kthread -pthread -pthreads -mt -mthreads -Kthread -threads; do 
    CFLAGS=$nopoll_ptc_cflags
    test "x$flag" != "xnone" && CFLAGS="$CFLAGS $flag"
    NOPOLL_PTHREADS_TRY_RUN([
      nopoll_cv_pthreads_cflags="$flag"
      break
    ])
 done
 CFLAGS=$nopoll_ptc_cflags
])

# The CFLAGS may or may not be sufficient to ensure that libnopoll
# depends on the pthreads library: some versions of libtool
# drop -pthread when passed on the link line; some versions of
# gcc ignore -pthread when linking a shared object.  So always
# try and add the relevant library to LIBS too.

AC_CACHE_CHECK([for LIBS needed for pthreads], [nopoll_cv_pthreads_lib], [
  nopoll_ptc_libs=$LIBS
  for lib in -lpthread -lpthreads -lc_r; do
    LIBS="$nopoll_ptc_libs $lib"
    NOPOLL_PTHREADS_TRY_RUN([
      nopoll_cv_pthreads_lib=$lib
      break
    ])
  done
  LIBS=$nopoll_ptc_libs
])

if test "$pthreads_working" = "yes"; then
  threads_result="POSIX Threads found"
else
  threads_result="POSIX Threads not found"
fi
])dnl

dnl call to detect thread activation support
NOPOLL_PTHREADS_CHECK

PTHREAD_CFLAGS="$nopoll_cv_pthreads_cflags"
PTHREAD_LIBS="$nopoll_cv_pthreads_lib"
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

my_save_cflags="$CFLAGS"
CFLAGS="-fstack-protector-all -Wstack-protector"
AC_MSG_CHECKING([whether CC supports -fstack-protector-all -Wstack-protector])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [AM_CFLAGS="-fstack-protector-all -Wstack-protector"],
    [AC_MSG_RESULT([no])]
)
CFLAGS="$my_save_cflags"
AC_SUBST([AM_CFLAGS])

AC_ARG_ENABLE(nopoll-log, [  --enable-nopoll-log     Enable building Nopoll with debug log support [default=yes]], enable_nopoll_log="$enableval", enable_nopoll_log=yes)
AM_CONDITIONAL(ENABLE_NOPOLL_LOG, test "x$enable_nopoll_log" = "xyes")

dnl detect openssl support
AC_CHECK_HEADER(openssl/ssl.h,,enable_tls_support=no)
if test x$enable_tls_support = xno ; then
   AC_MSG_ERROR([Cannot find OpenSSL installation. Unable to build noPoll: this is a requirement. ]) 
else
   dnl configure additional TLS flags
   TLS_LIBS="-lssl -lcrypto"
   AC_SUBST(TLS_LIBS)
fi

AC_CHECK_LIB(ssl,SSLv3_method,  ssl_sslv3_supported=yes, ssl_sslv3_supported=no)
ssl_sslv3_header=""
if test x$ssl_sslv3_supported = xyes; then
   export ssl_sslv3_header="/**
 * @brief Indicates where we have support for SSL v3.0 support. The SSLv3 protocol is deprecated and should not be used.
 */
#define NOPOLL_HAVE_SSLv3_ENABLED (1)"
fi


AC_CHECK_LIB(ssl,SSLv23_method,  ssl_sslv23_supported=yes, ssl_sslv23_supported=no)
ssl_sslv23_header=""
if test x$ssl_sslv23_supported = xyes; then
   export ssl_sslv23_header="/**
 * @brief Indicates where we have support for SSL v.3 support.
 */
#define NOPOLL_HAVE_SSLv23_ENABLED (1)"
fi

AC_CHECK_LIB(ssl,TLSv1_method,   ssl_tlsv1_supported=yes,  ssl_tlsv1_supported=no)
ssl_tlsv1_header=""
if test x$ssl_tlsv1_supported = xyes; then
   export ssl_tlsv1_header="/**
 * @brief Indicates where we have support for TLSv1.0 support.
 */
#define NOPOLL_HAVE_TLSv10_ENABLED (1)"
fi

AC_CHECK_LIB(ssl,TLSv1_1_method, ssl_tlsv11_supported=yes, ssl_tlsv11_supported=no)
ssl_tlsv11_header=""
if test x$ssl_tlsv11_supported = xyes; then
   export ssl_tlsv11_header="/**
 * @brief Indicates where we have support for TLSv1.1 support.
 */
#define NOPOLL_HAVE_TLSv11_ENABLED (1)"
fi

AC_CHECK_LIB(ssl,TLSv1_2_method, ssl_tlsv12_supported=yes, ssl_tlsv12_supported=no)
ssl_tlsv12_header=""
if test x$ssl_tlsv12_supported = xyes; then
   export ssl_tlsv12_header="/**
 * @brief Indicates where we have support for TLSv1.2 support.
 */
#define NOPOLL_HAVE_TLSv12_ENABLED (1)"
fi

AC_CHECK_LIB(ssl,TLS_client_method, ssl_tls_flexible_supported=yes, ssl_tls_flexible_supported=no)
ssl_tls_flexible_header=""
if test x$ssl_tls_flexible_supported = xyes; then
   export ssl_tls_flexible_header="/**
 * @brief Indicates where we have support for TLS flexible method where the highest TLS version will be negotiated.
 */
#define NOPOLL_HAVE_TLS_FLEXIBLE_ENABLED (1)"
fi


# The following command also comes to produce the nopoll_config.h file
# required by the tool. If you update this, remember to update the
# af-arch main configure.ac
AC_TRY_LINK([#define _GNU_SOURCE
#include <stdio.h>], 
[
  char * result;
  return vasprintf (&result, "This is a test: %d", NULL);
], [have_vasprintf=yes],[have_vasprintf=no])
echo "Checking vasprintf support: $have_vasprintf"


# produce nopoll_config.h file 
AC_CONFIG_COMMANDS([nopoll_config.h],
[
	outfile=nopoll_config.h-tmp
	cat > $outfile <<_______EOF
/* 
 * Nopoll Library nopoll_config.h
 * Platform dependant definitions.
 *
 * This is a generated file.  Please modify 'configure.in'
 */

#ifndef __NOPOLL_CONFIG_H__
#define __NOPOLL_CONFIG_H__

/** 
 * \addtogroup nopoll_decl_module
 * @{
 */

/** 
 * @brief Allows to convert integer value (including constant values)
 * into a pointer representation. 
 *
 * Use the oposite function to restore the value from a pointer to a
 * integer: \ref PTR_TO_INT. 
 *
 * @param integer The integer value to cast to pointer.
 *
 * @return A \ref noPollPtr reference.
 */
#ifndef INT_TO_PTR
#define INT_TO_PTR(integer)   ((noPollPtr) ${pti_cast} ((int)integer))
#endif

/** 
 * @brief Allows to convert a pointer reference (\ref noPollPtr),
 * which stores an integer that was stored using \ref INT_TO_PTR.
 *
 * Use the oposite function to restore the pointer value stored in the
 * integer value.
 * 
 * @param ptr The pointer to cast to a integer value.
 * 
 * @return A int value.
 */
#ifndef PTR_TO_INT
#define PTR_TO_INT(ptr) ((int) ${pti_cast} (ptr))
#endif

/**
 * @brief Allows to get current platform configuration. This is used
 * by Nopoll library but could be used by applications built on top of
 * Nopoll to change its configuration based on the platform information.
 */
$nopoll_platform

$vasprintf_status

$have_64bit_support

$ssl_sslv23_header

$ssl_sslv3_header

$ssl_tlsv1_header

$ssl_tlsv11_header

$ssl_tlsv12_header

$ssl_tls_flexible_header

/* @} */

#endif
_______EOF

	# Check if the file exists and it is equal, if not, overwrite it 
	if cmp -s $outfile src/nopoll_config.h; then
	  AC_MSG_NOTICE([nopoll_config.h is unchanged])
	  rm -f $outfile
	else
	  mv $outfile src/nopoll_config.h
	fi
],[

# Check size of void pointer against the size of a single
# integer. This will allow us to know if we can cast directly a
# integer to pointer and viceversa.
have_64bit_support=""
case $ac_cv_sizeof_void_p in
$ac_cv_sizeof_int)  	pti_cast=''        ptui_cast=''                ;;
$ac_cv_sizeof_long) 	pti_cast='(long)'  ptui_cast='(unsigned long)'  have_64bit_support="/**
 * @brief Indicates that this platform have support for 64bits.
 */
#define NOPOLL_64BIT_PLATFORM (1)" ;;
*) 			pti_unknown_void_p=yes                         ;;
esac

# Get current configure for the platform and the os we are running.
echo "Host detected: $host"
case $host in
  *-*-beos*)
    nopoll_platform="#define NOPOLL_OS_BEOS (1)"
    ;;
  *-*-cygwin*)
    nopoll_platform="#define NOPOLL_OS_UNIX (1)
#define NOPOLL_PLATFORM_WIN32 (1)
#define NOPOLL_WITH_CYGWIN (1)"
    ;;
  *-*-mingw*)
    nopoll_platform="#define NOPOLL_OS_WIN32 (1)
#define NOPOLL_PLATFORM_WIN32 (1)"
    ;;
  *)
    nopoll_platform="#define NOPOLL_OS_UNIX (1)"
    ;;
esac

case $have_vasprintf in
yes)
     vasprintf_status="/**
 * @internal Allows to now if the platform support vasprintf
 * function. Do not use this macro as it is supposed to be for
 * internal use.
 */
#define NOPOLL_HAVE_VASPRINTF (1)"
     ;;
no)
     vasprintf_status=""
     ;;
esac

])

##########################
# Check for doxygen tool #
##########################
dnl check for doxygen documentation
AC_ARG_ENABLE(nopoll-doc, [  --enable-nopoll-doc     Enable building noPoll documentation (doxygen required) [default=yes]], enable_nopoll_doc="$enableval", enable_nopoll_doc=yes)
if test x$enable_nopoll_doc = xyes ; then
        AC_CHECK_PROG(DOXYGEN, doxygen, "yes", "no")
fi
AM_CONDITIONAL(ENABLE_NOPOLL_DOC, test "x$DOXYGEN" = "xyes")

case $host in
  *-*-mingw*)
    WS2_LIBS="-lws2_32"
    AC_SUBST(WS2_LIBS)
    ;;
esac

AC_OUTPUT([
Makefile
src/Makefile
doc/Makefile
doc/nopoll.doxygen
test/Makefile
nopoll.pc])

sed -i "s/__NOPOLL_VERSION__/$NOPOLL_VERSION/g" doc/nopoll.doxygen
sed -i "s/@NOPOLL_VERSION@/$NOPOLL_VERSION/g" doc/nopoll.doxygen

echo "------------------------------------------"
echo "--       LibNoPoll (${NOPOLL_VERSION}) LIBRARY SETTINGS     --"
echo "------------------------------------------"
echo "   Installation prefix:            [$prefix]"
echo "      select(2) support:           [yes]"
echo "      poll(2) support:             [$enable_poll]"
echo "      epoll(2) support:            [$enable_cv_epoll]"
echo "   OpenSSL TLS protocol versions detected:"
echo "      SSLv3:   $ssl_sslv3_supported"
echo "      SSLv23:  $ssl_sslv23_supported"
echo "      TLSv1.0: $ssl_tlsv1_supported"
echo "      TLSv1.1: $ssl_tlsv11_supported"
echo "      TLSv1.2: $ssl_tlsv12_supported"
echo "      TLS flx: $ssl_tls_flexible_supported"
echo "------------------------------------------"
echo "--     NOW TYPE: make; make install     --"
echo "------------------------------------------"
echo "--     Mira papa! Sin manos!!!          --"
echo "------------------------------------------"

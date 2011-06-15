/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2011 Advanced Software Production Line, S.L.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 *  
 *  You may find a copy of the license under this software is released
 *  at COPYING file. This is LGPL software: you are welcome to develop
 *  proprietary applications using this library without any royalty or
 *  fee but returning back any change, improvement or addition in the
 *  form of source code, project image, documentation patches, etc.
 *
 *  For commercial support on build Websocket enabled solutions
 *  contact us:
 *          
 *      Postal address:
 *         Advanced Software Production Line, S.L.
 *         Edificio Alius A, Oficina 102,
 *         C/ Antonio Suarez Nº 10,
 *         Alcalá de Henares 28802 Madrid
 *         Spain
 *
 *      Email address:
 *         info@aspl.es - http://www.aspl.es/nopoll
 */
#ifndef __NOPOLL_DECL_H__
#define __NOPOLL_DECL_H__

/* include this at this place to load GNU extensions */
#if defined(__GNUC__)
#  ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#  endif
#  define __NOPOLL_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#  define __NOPOLL_LINE__            __LINE__
#  define __NOPOLL_FILE__            __FILE__
#elif defined(_MSC_VER)
#  define __NOPOLL_PRETTY_FUNCTION__ __FUNCDNAME__
#  define __NOPOLL_LINE__            __LINE__
#  define __NOPOLL_FILE__            __FILE__
#else
/* unknown compiler */
#define __NOPOLL_PRETTY_FUNCTION__ ""
#define __NOPOLL_LINE__            0
#define __NOPOLL_FILE__            ""
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* only include unistd.h if unix platform is found or gnu gcc compiler
 * is found */
#if defined(__GNUC__) || defined(NOPOLL_OS_UNIX)
# include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

/** 
 * \defgroup nopoll_decl_module Nopoll Declarations: Common Nopoll declarations, Types, macros, and support functions.
 */

/** 
 * \addtogroup nopoll_decl_module
 * @{
 */

/** 
 * @brief Common definition to have false (\ref nopoll_false) value (which is defined to 0 integer value).
 */
#define nopoll_false ((int)0)
/** 
 * @brief Common definition to have true (\ref nopoll_true) value (which is defined to 1 integer value).
 */
#define nopoll_true  ((int)1)

/** 
 * @brief Pointer to any structure definition. It should be required
 * to use this definition, however, some platforms doesn't support the
 * <b>void *</b> making it necessary to use the <b>char *</b>
 * definition as a general way to represent references.
 */
typedef void * nopollPtr;

/** 
 * @brief Nopoll debug levels.
 * 
 * While reporting log to the console, these levels are used to report
 * the severity for such log.
 */
typedef enum {
	/** 
	 * @brief Debug level. Only used to report common
	 * circumstances that represent the proper functionality.
	 */
	NOPOLL_LEVEL_DEBUG, 
	/** 
	 * @brief Warning level. Only used to report that an internal
	 * issue have happend that could be interesting while
	 * reporting error, but it could also mean common situations.
	 */
	NOPOLL_LEVEL_WARNING, 
	/** 
	 * @brief Critical level. Only used to report critical
	 * situations where some that have happened shouldn't. 
	 *
	 * This level should only be used while reporting critical
	 * situations.
	 */
	NOPOLL_LEVEL_CRITICAL}  
NopollDebugLevel;

/** 
 * @brief Support macro to allocate memory using nopoll_calloc function,
 * making a casting and using the sizeof keyword.
 *
 * @param type The type to allocate
 * @param count How many items to allocate.
 * 
 * @return A newly allocated pointer.
 */
#define nopoll_new(type, count) (type *) nopoll_calloc (count, sizeof (type))

/** 
 * @brief Allows to check a condition and return if it is not meet.
 * 
 * @param expr The expresion to check.
 */
#define nopoll_return_if_fail(expr) \
if (!(expr)) {__nopoll_log ("", NOPOLL_LEVEL_CRITICAL, "Expresion '%s' have failed at %s (%s:%d)", #expr, __NOPOLL_PRETTY_FUNCTION__, __NOPOLL_FILE__, __NOPOLL_LINE__); return;}

/** 
 * @brief Allows to check a condition and return the given value if it
 * is not meet.
 * 
 * @param expr The expresion to check.
 *
 * @param val The value to return if the expression is not meet.
 */
#define nopoll_return_val_if_fail(expr, val) \
if (!(expr)) { __nopoll_log ("", NOPOLL_LEVEL_CRITICAL, "Expresion '%s' have failed, returning: %s at %s (%s:%d)", #expr, #val, __NOPOLL_PRETTY_FUNCTION__, __NOPOLL_FILE__, __NOPOLL_LINE__); return val;}


/** 
 * @internal
 *
 * C++ support declarations borrowed from the libtool webpage. Thanks
 * you guys for this information. 
 *
 * BEGIN_C_DECLS should be used at the beginning of your declarations,
 * so that C++ compilers don't mangle their names.  Use END_C_DECLS at
 * the end of C declarations.
 */
#undef BEGIN_C_DECLS
#undef END_C_DECLS
#ifdef __cplusplus
# define BEGIN_C_DECLS extern "C" {
# define END_C_DECLS }
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
#endif

BEGIN_C_DECLS

nopollPtr  nopoll_calloc  (size_t count, size_t size);

nopollPtr  nopoll_realloc (nopollPtr ref, size_t size);

void        nopoll_free    (nopollPtr ref);

END_C_DECLS

#endif

/* @} */

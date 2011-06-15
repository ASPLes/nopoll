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
#ifndef __NOPOLL_LOG_H__
#define __NOPOLL_LOG_H__

#include <nopoll_decl.h>

BEGIN_C_DECLS

/** 
 * \addtogroup nopoll_log_module
 * @{
 */

nopoll_bool     nopoll_log_is_enabled (nopollCtx * ctx);

nopoll_bool     nopoll_log_color_is_enabled (nopollCtx * ctx);

void            nopoll_log_enable (nopollCtx * ctx, nopoll_bool value);
 
void            nopoll_log_color_enable (nopollCtx * ctx, nopoll_bool value);


#if defined(SHOW_DEBUG_LOG)
# define __nopoll_log nopoll_log
#else
# if defined(NOPOLL_OS_WIN32) && !( defined (__GNUC__) || _MSC_VER >= 1400)
/* default case where '...' is not supported but log is still
 * disabled */
#   define __nopoll_log nopoll_log 
# else
#   define __nopoll_log(domain, level, message, ...) /* nothing */
# endif
#endif

void nopoll_log (nopollCtx * ctx, noPollDebugLevel level, char * message, ...);

/* @} */

END_C_DECLS

#endif


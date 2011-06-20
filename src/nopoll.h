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
#ifndef __NOPOLL_H__
#define __NOPOLL_H__

#include <nopoll_decl.h>
#include <nopoll_handlers.h>
#include <nopoll_ctx.h>
#include <nopoll_io.h>
#include <nopoll_conn.h>
#include <nopoll_msg.h>
#include <nopoll_log.h>
#include <nopoll_listener.h>
#include <nopoll_io.h>
#include <nopoll_loop.h>

BEGIN_C_DECLS

/** 
 * \addtogroup nopoll_module
 * @{
 */

nopoll_bool nopoll_cmp (const char * string1, const char * string2);

nopoll_bool nopoll_ncmp (const char * string1, const char * string2, int bytes);

char      * nopoll_strdup_printf   (const char * chunk, ...);

char      * nopoll_strdup_printfv  (const char * chunk, va_list args);

void        nopoll_trim  (char * chunk, int * trimmed);

void        nopoll_sleep (long microseconds);

nopoll_bool nopoll_base64_encode (const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size);

nopoll_bool nopoll_base64_decode (const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size);

nopoll_bool nopoll_nonce (char * buffer, int nonce_size);

/* @} */

END_C_DECLS

#endif
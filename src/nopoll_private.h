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
#ifndef __NOPOLL_PRIVATE_H__
#define __NOPOLL_PRIVATE_H__

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <nopoll_handlers.h>

struct _noPollCtx {
	/**
	 * @internal Controls logs output..
	 */
	/* context reference counting */
	int             refs;
	int             conn_id;

	/* console log */
	nopoll_bool     not_executed;
	nopoll_bool     debug_enabled;
	
	/* colored log */
	nopoll_bool     not_executed_color;
	nopoll_bool     debug_color_enabled;

	/** 
	 * @internal noPollConn connection timeout.
	 */
	long        conn_connect_std_timeout;

	/** 
	 * @internal Default listener connection backlog
	 */
	int         backlog;

	/** 
	 * @internal Currently selected io engine on this context.
	 */
	noPollIoEngine * io_engine;

	/** 
	 * @internal Connection array list and its length.
	 */
	noPollConn     ** conn_list;
	int               conn_length;
	/** 
	 * @internal Number of connections registered on this context.
	 */
	int               conn_num;

	/** 
	 * @internal Reference to defined on accept handling.
	 */
	noPollActionHandler on_accept;
	noPollPtr           on_accept_data;

	/** 
	 * @internal Reference to defined on accept handling.
	 */
	noPollActionHandler on_open;
	noPollPtr           on_open_data;
};

struct _noPollConn {
	/** 
	 * @internal Connection id.
	 */
	int              id;

	/** 
	 * @internal The context associated to this connection.
	 */
	noPollCtx      * ctx;

	/** 
	 * @internal This is the actual socket handler associated to
	 * the noPollConn object.
	 */
	NOPOLL_SOCKET    session;

	/** 
	 * @internal Flag to signal this connection has finished its
	 * handshake.
	 */
	nopoll_bool      handshake_ok;

	/** 
	 * @internal Current connection receive function.
	 */
	noPollRead       receive;

	/** 
	 * @internal Current connection receive function.
	 */
	noPollRead       send;

	/** 
	 * @internal The connection role.
	 */
	noPollRole       role;

	/** 
	 * @internal Conection host ip location (connecting or listening).
	 */
	char           * host;

	/** 
	 * @internal Connection port location (connecting or
	 * listening).
	 */ 
	char           * port;

	/** 
	 * @internal Host name requested on the connection.
	 */
	char           * host_name;
	/** 
	 * @internal Origin requested on the connection.
	 */
	char           * origin;

	/** 
	 * @internal reference to the get url.
	 */
	char           * get_url;
	
	/** 
	 * @internal Reference to protocols requested to be opened on
	 * this connection.
	 */
	char           * protocols;
	
	/** 
	 * @internal Reference to the defined on message handling.
	 */
	noPollOnMessageHandler on_msg;
	noPollPtr              on_msg_data;

	/* reference to the handshake */
	noPollHandShake  * handshake;

	/* reference to a buffer with pending content */
	char * pending_line;

	/** 
	 * @internal connection reference counting.
	 */
	int    refs;
};

struct _noPollIoEngine {
	noPollPtr              io_object;
	noPollCtx            * ctx;
	noPollIoMechCreate     create;
	noPollIoMechDestroy    destroy;
	noPollIoMechClear      clear;
	noPollIoMechWait       wait;
	noPollIoMechAddTo      addto;
	noPollIoMechIsSet      isset;
};

struct _noPollMsg {
	nopoll_bool has_fin;
	short       op_code;
	nopoll_bool is_masked;

	noPollPtr          payload;
	long int           payload_size;
	int                refs;
};

struct _noPollHandshake {
	/** 
	 * @internal Reference to the to the GET url HTTP/1.1 header
	 * part.
	 */
	nopoll_bool     upgrade_websocket;
	nopoll_bool     connection_upgrade;
	nopoll_bool     received_101; 
	char          * websocket_key;
	char          * websocket_version;
};

#endif

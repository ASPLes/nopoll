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
	 * @internal Connection list and its length.
	 */
	noPollConn     ** conn_list;
	int               conn_length;
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

#endif

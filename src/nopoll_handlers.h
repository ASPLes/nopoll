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
#ifndef __NOPOLL_HANDLERS_H__
#define __NOPOLL_HANDLERS_H__

/** 
 * @brief Async handler definition that is configured at \ref
 * nopoll_ctx_set_action_handler and called by \ref nopoll_loop_wait
 * to notify that a connection has activity. Inside this handler the
 * developer must implement handling required to accept connections,
 * or read data from the remote end point.
 *
 * @param ctx The context where the wait is happening.
 *
 * @param conn The connection where the data or something meaningful
 * was detected.
 *
 * @param user_data Optional user data pointer defined by the user at
 * \ref nopoll_ctx_set_action_handler
 *
 */
typedef void (*noPollActionHandler) (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data);

/** 
 * @brief Handler used to define the create function for an IO mechanism.
 *
 * @param ctx The context where the io mechanism will be created.
 */
typedef noPollPtr (*noPollIoMechCreate)  (noPollCtx * ctx);

/** 
 * @brief Handler used to define the IO wait set destroy function for
 * an IO mechanism.
 *
 * @param ctx The context where the io mechanism will be destroyed.
 *
 * @param io_object The io object to be destroyed as created by \ref
 * noPollIoMechCreate handler.
 */
typedef void (*noPollIoMechDestroy)  (noPollCtx * ctx, noPollPtr io_object);

/** 
 * @brief Handler used to define the IO wait set clear function for an
 * IO mechanism.
 *
 * @param ctx The context where the io mechanism will be cleared.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler.
 */
typedef void (*noPollIoMechClear)  (noPollCtx * ctx, noPollPtr io_object);


/** 
 * @brief Handler used to define the IO wait function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef int (*noPollIoMechWait)  (noPollCtx * ctx, noPollPtr io_object);


/** 
 * @brief Handler used to define the IO add to set function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param conn The noPollConn to be added to the working set.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef nopoll_bool (*noPollIoMechAddTo)  (int               fds, 
					   noPollCtx       * ctx,
					   noPollConn      * conn,
					   noPollPtr         io_object);


/** 
 * @brief Handler used to define the IO is set function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param conn The noPollConn to be added to the working set.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef nopoll_bool (*noPollIoMechIsSet)  (noPollCtx       * ctx,
					   int               fds, 
					   noPollPtr         io_object);

#endif

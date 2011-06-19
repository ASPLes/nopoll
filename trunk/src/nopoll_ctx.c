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
#include <nopoll_ctx.h>
#include <nopoll_private.h>

/** 
 * @brief Creates an empty Nopoll context. 
 */
noPollCtx * nopoll_ctx_new (void) {

	noPollCtx * result = nopoll_new (noPollCtx, 1);
	if (result == NULL)
		return NULL;
	/* set initial reference */
	result->refs = 1;
	result->conn_id = 1;

	/* 20 seconds for connection timeout */
	result->conn_connect_std_timeout = 20000000;

	/* default log initialization */
	result->not_executed  = nopoll_true;
	result->debug_enabled = nopoll_false;
	
	/* colored log */
	result->not_executed_color  = nopoll_true;
	result->debug_color_enabled = nopoll_false;

	/* default back log */
	result->backlog = 5;

	/* current list length */
	result->conn_length = 0;

	return result;
}

/** 
 * @brief Allows to acquire a reference to the provided context. This
 * reference is released by calling to \ref nopoll_ctx_unref.
 *
 * @param ctx The context to acquire a reference.
 *
 * @return The function returns nopoll_true in the case the reference
 * was acquired, otherwise nopoll_false is returned.
 */ 
nopoll_bool    nopoll_ctx_ref (noPollCtx * ctx)
{
	/* return false value */
	nopoll_return_val_if_fail (ctx, ctx, nopoll_false);

	/* acquire mutex here */
	ctx->refs++;
	/* release mutex here */

	return nopoll_true;
}

/** 
 * @brief allows to release a reference acquired to the provided
 * noPoll context.
 *
 * @param ctx The noPoll context reference to release..
 */
void           nopoll_ctx_unref (noPollCtx * ctx)
{
	if (ctx == NULL)
		return;

	/* acquire mutex here */
	ctx->refs--;
	if (ctx->refs != 0) {
		/* release mutex here */
		return;
	}
	/* release mutex here */
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Releasing no poll context %p (%d)", ctx, ctx->refs);

	/* release connection */
	nopoll_free (ctx->conn_list);
	ctx->conn_length = 0;
	nopoll_free (ctx);
	return;
}

/** 
 * @internal Function used to register the provided connection on the
 * provided context.
 *
 * @param ctx The context where the connection will be registered.
 *
 * @param conn The connection to be registered.
 */
void           nopoll_ctx_register_conn (noPollCtx  * ctx, 
					 noPollConn * conn)
{
	int iterator;

	nopoll_return_if_fail (ctx, ctx && conn);

	/* acquire mutex here */
	conn->id = ctx->conn_id;
	ctx->conn_id ++;

	/* register connection */
	iterator = 0;
	while (iterator < ctx->conn_length) {

		/* register reference */
		if (ctx->conn_list[iterator] == 0) {
			ctx->conn_list[iterator] = conn;

			/* update connection list number */
			ctx->conn_num++;

			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "registered connection id %d, role: %d", conn->id, conn->role);

			/* acquire reference */
			ctx->refs++;
			
			/* acquire a reference to the conection */
			nopoll_conn_ref (conn);

			/* release mutex here */

			return;
		}
		
		iterator++;
	} /* end while */

	/* if reached this place it means no more buckets are
	 * available, acquire more memory (increase 10 by 10) */
	ctx->conn_length += 10;
	ctx->conn_list = nopoll_realloc (ctx->conn_list, sizeof (noPollConn *) * (ctx->conn_length));
	if (ctx->conn_list == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "General connection registration error, memory acquisition failed..");
		return;
	} /* end if */
	
	/* clear new positions */
	iterator = (ctx->conn_length - 10);
	while (iterator < ctx->conn_length) {
		ctx->conn_list[iterator] = 0;
		/* next position */
		iterator++;
	} /* end while */

	/* release mutex here */

	/* ok, now register connection because we have memory */
	nopoll_ctx_register_conn (ctx, conn);

	return;
}

/** 
 * @internal Function used to register the provided connection on the
 * provided context.
 *
 * @param ctx The context where the connection will be registered.
 *
 * @param conn The connection to be registered.
 */
void           nopoll_ctx_unregister_conn (noPollCtx  * ctx, 
					   noPollConn * conn)
{
	int iterator;

	nopoll_return_if_fail (ctx, ctx && conn);

	/* acquire mutex here */

	/* find the connection and remove it from the array */
	iterator = 0;
	while (iterator < ctx->conn_length) {

		/* check the connection reference */
		if (ctx->conn_list[iterator]->id == conn->id) {
			/* remove reference */
			ctx->conn_list[iterator] = NULL;

			/* update connection list number */
			ctx->conn_num--;

			/* acquire a reference to the conection */
			nopoll_conn_unref (conn);

			break;
		} /* end if */
		
		iterator++;
	} /* end while */

	/* release mutex here */

	return;
}

/** 
 * @brief Allows to get number of connections currently registered.
 *
 * @param ctx The context where the operation is requested.
 *
 * @return Number of connections registered on this context or -1 if it fails.
 */ 
int            nopoll_ctx_conns (noPollCtx * ctx)
{
	nopoll_return_val_if_fail (ctx, ctx, -1);
	return ctx->conn_num;
}

/** 
 * @brief Allows to configure the on open handler, the handler that is
 * called when it is received an incoming websocket connection and all
 * websocket client handshake data was received.
 *
 * This handler differs from \ref nopoll_ctx_set_on_accept this
 * handler is called after all client handshake data was received.
 *
 * @param ctx The context that will be configured.
 *
 * @param on_open The handler to be configured on this context.
 *
 * @param user_data User defined pointer to be passed to the on open
 * handler
 */
void           nopoll_ctx_set_on_open (noPollCtx            * ctx,
				       noPollActionHandler    on_open,
				       noPollPtr              user_data)
{
	nopoll_return_if_fail (ctx, ctx && on_open);

	/* set the handler */
	ctx->on_open = on_open;
	if (ctx->on_open == NULL)
		ctx->on_open_data = NULL;
	else
		ctx->on_open_data = user_data;
	return;

	return;
}

/** 
 * @brief Allows to configure the accept handler that will be called
 * when a connection is received but before any handshake takes place.
 *
 * @param ctx The context that will be configured.
 *
 * @param on_accept The handler to be called when a connection is
 * received. Here the handler must return nopoll_true to accept the
 * connection, otherwise nopoll_false should be returned.
 *
 * @param user_data Optional user data pointer passed to the on accept
 * handler.
 *
 */
void              nopoll_ctx_set_on_accept (noPollCtx            * ctx,
					    noPollActionHandler    on_accept,
					    noPollPtr              user_data)
{
	nopoll_return_if_fail (ctx, ctx && on_accept);

	/* set the handler */
	ctx->on_accept = on_accept;
	if (ctx->on_accept == NULL)
		ctx->on_accept_data = NULL;
	else
		ctx->on_accept_data = user_data;
	return;
}

/** 
 * @brief Allows to iterate over all connections currently registered
 * on the provided context, optionally stopping the foreach process,
 * returning the connection reference selected if the foreach handler
 * returns nopoll_true.
 *
 * @param ctx The nopoll context where the foreach operation will take
 * place.
 *
 * @param foreach The foreach handler to be called for each connection
 * registered.
 *
 * @param user_data An optional reference to a pointer that will be
 * passed to the handler.
 *
 * @return Returns the connection selected (in the case the foreach
 * function returns nopoll_false) or NULL in the case all foreach
 * executions returned nopoll_true. Keep in mind the function also
 * returns NULL if ctx or foreach parameter is NULL.
 *
 * See \ref noPollForeachConn for a signature example.
 */
noPollConn   * nopoll_ctx_foreach_conn (noPollCtx          * ctx, 
					noPollForeachConn    foreach, 
					noPollPtr            user_data)
{
	noPollConn * result;
	int          iterator;
	nopoll_return_val_if_fail (ctx, ctx && foreach, NULL);

	/* acquire here the mutex to protect connection list */

	/* nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Doing foreach over conn_length array (%p): %d", ctx, ctx->conn_length); */
	
	/* find the connection and remove it from the array */
	iterator = 0;
	while (iterator < ctx->conn_length) {

		/* check the connection reference */
		if (ctx->conn_list[iterator]) {
			/* call to notify connection */
			if (foreach (ctx, ctx->conn_list[iterator], user_data)) {
				/* get a reference to avoid races
				 * after releasing the mutex */
				result = ctx->conn_list[iterator];

				/* release here the mutex to protect connection list */
				return result;
			} /* end if */
		} /* end if */
		
		iterator++;
	} /* end while */

	/* release here the mutex to protect connection list */

	return NULL;
}





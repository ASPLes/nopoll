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
	return;
}

/** 
 * @brief Allows to configure the context action handler that will be
 * called to notify connection changes when called to \ref
 * nopoll_loop_wait.
 *
 * @param ctx The context object to be configured.
 *
 * @param action_handler The action handler to be called for each
 * connection status change to be notified.
 *
 * @param user_data Optional user defined pointer to be passed to the
 * action handler.
 */
void        nopoll_ctx_set_action_handler (noPollCtx            * ctx, 
					   noPollActionHandler    action_handler,
					   noPollPtr              user_data)
{
	nopoll_return_if_fail (ctx, ctx && action_handler);

	return;
}




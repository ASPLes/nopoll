/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2025 Advanced Software Production Line, S.L.
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
 *         Av. Juan Carlos I, Nº13, 2ºC
 *         Alcalá de Henares 28806 Madrid
 *         Spain
 *
 *      Email address:
 *         info@aspl.es - http://www.aspl.es/nopoll
 */
#include <nopoll_ctx.h>
#include <nopoll_private.h>
#include <signal.h>

/** 
 * \defgroup nopoll_ctx noPoll Context: context handling functions used by the library
 */

/** 
 * \addtogroup nopoll_ctx
 * @{
 */
void __nopoll_ctx_sigpipe_do_nothing (int _signal)
{
#if !defined(NOPOLL_OS_WIN32)
	/* do nothing sigpipe handler to be able to manage EPIPE error
	 * returned by write ops. */

	/* the following line is to ensure ancient glibc version that
	 * restores to the default handler once the signal handling is
	 * executed. */
	signal (SIGPIPE, __nopoll_ctx_sigpipe_do_nothing);
#endif
	return;
}


/**
 * @brief Creates an empty noPoll context.
 *
 * The context is created with one reference, so the caller must
 * release it by calling \ref nopoll_ctx_unref once it is done with it
 * (and after having terminated every connection created under it).
 *
 * @return A newly created \ref noPollCtx or NULL if it fails.
 */
noPollCtx * nopoll_ctx_new (void) {
	noPollCtx * result;

	/* create the context and, on Windows, initialize WinSock over it */
	result = nopoll_new (noPollCtx, 1);
	if (result == NULL)
		return NULL;

#if defined(NOPOLL_OS_WIN32)
	if (! nopoll_win32_init (result)) {
		/* release the context just created: returning here
		 * without freeing it leaked it */
		nopoll_free (result);
		return NULL;
	} /* end if */
#endif

	/* set initial reference */
	result->refs    = 1;
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

	/* setup default protocol version */
	result->protocol_version = 13;

	/* setup default maximum frame size accepted */
	result->max_frame_size = NOPOLL_MAX_FRAME_SIZE_DEFAULT;

	/* create mutexes */
	result->ref_mutex = nopoll_mutex_create ();

#if !defined(NOPOLL_OS_WIN32)
	/* install sigpipe handler */
	signal (SIGPIPE, __nopoll_ctx_sigpipe_do_nothing);
#endif

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
	nopoll_mutex_lock (ctx->ref_mutex);

	ctx->refs++;

	/* release mutex here */
	nopoll_mutex_unlock (ctx->ref_mutex);

	return nopoll_true;
}


/** 
 * @brief allows to release a reference acquired to the provided
 * noPoll context.
 *
 * When the last reference is released the context itself is
 * destroyed, releasing the certificate store and the connection
 * registry, so the reference received must not be used after that.
 *
 * @param ctx The noPoll context reference to release.
 */
void           nopoll_ctx_unref (noPollCtx * ctx)
{
	noPollCertificate * cert;
	int iterator;

	nopoll_return_if_fail (ctx, ctx);

	/* acquire mutex here */
	nopoll_mutex_lock (ctx->ref_mutex);

	ctx->refs--;
	if (ctx->refs != 0) {
		/* release mutex here */
		nopoll_mutex_unlock (ctx->ref_mutex);
		return;
	}
	/* release mutex here */
	nopoll_mutex_unlock (ctx->ref_mutex);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Releasing noPoll context %p (refs: %d, conns registered: %d, list size: %d)",
		    ctx, ctx->refs, ctx->conn_num, ctx->conn_length);

	iterator = 0;
	while (iterator < ctx->certificates_length) {
		/* get reference */
		cert = &(ctx->certificates[iterator]);

		/* release */
		nopoll_free (cert->serverName);
		nopoll_free (cert->certificateFile);
		nopoll_free (cert->privateKey);
		nopoll_free (cert->optionalChainFile);

		/* next position */
		iterator++;
	} /* end while */

	/* release the I/O engine in the case it is still created: it
	 * is normally released by nopoll_loop_stop (), but an
	 * application that never ran (or never stopped) the loop
	 * leaked it */
	if (ctx->io_engine) {
		nopoll_io_release_engine (ctx->io_engine);
		ctx->io_engine = NULL;
	} /* end if */

	/* release mutex */
	nopoll_mutex_destroy (ctx->ref_mutex);

	/* release all certificates buckets */
	nopoll_free (ctx->certificates);

	/* release connection */
	nopoll_free (ctx->conn_list);
	ctx->conn_length = 0;
	nopoll_free (ctx);
	return;
}

/** 
 * @brief Allows to get current reference counting for the provided
 * context.
 *
 * @param ctx The context the reference counting is being requested.
 *
 * @return The reference counting or -1 if it fails.
 */
int            nopoll_ctx_ref_count (noPollCtx * ctx)
{
	int result;
	if (! ctx)
		return -1;

	/* lock */
	nopoll_mutex_lock (ctx->ref_mutex);

	result = ctx->refs;

	/* unlock */
	nopoll_mutex_unlock (ctx->ref_mutex);

	return result;
}

/** 
 * @internal Function used to register the provided connection on the
 * provided context.
 *
 * The function acquires a reference to the connection and another one
 * to the context. The connection reference is released by \ref
 * nopoll_ctx_unregister_conn, while the context reference is released
 * when the connection object itself is finally destroyed.
 *
 * @param ctx The context where the connection will be registered.
 *
 * @param conn The connection to be registered.
 *
 * @return nopoll_true if the connection was registered, otherwise
 * nopoll_false is returned.
 */
nopoll_bool           nopoll_ctx_register_conn (noPollCtx  * ctx, 
						noPollConn * conn)
{
	int iterator;

	nopoll_return_val_if_fail (ctx, ctx && conn, nopoll_false);

	/* acquire mutex here */
	nopoll_mutex_lock (ctx->ref_mutex);

	/* get connection */
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

			/* release the mutex before acquiring references:
			 * both nopoll_ctx_ref and nopoll_conn_ref take
			 * their own locks */
			nopoll_mutex_unlock (ctx->ref_mutex);

			/* acquire a reference to the context */
			nopoll_ctx_ref (ctx);

			/* acquire a reference to the connection */
			nopoll_conn_ref (conn);

			return nopoll_true;
		}
		
		iterator++;
	} /* end while */

	/* if reached this place it means no more buckets are
	 * available, acquire more memory (increase 10 by 10) */
	ctx->conn_length += 10;
	ctx->conn_list = (noPollConn**) nopoll_realloc (ctx->conn_list, sizeof (noPollConn *) * (ctx->conn_length));
	if (ctx->conn_list == NULL) {
		/* release mutex */
		nopoll_mutex_unlock (ctx->ref_mutex);

		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "General connection registration error, memory acquisition failed..");
		return nopoll_false;
	} /* end if */
	
	/* clear new positions */
	iterator = (ctx->conn_length - 10);
	while (iterator < ctx->conn_length) {
		ctx->conn_list[iterator] = 0;
		/* next position */
		iterator++;
	} /* end while */

	/* release mutex here */
	nopoll_mutex_unlock (ctx->ref_mutex);

	/* ok, now register connection because we have memory */
	return nopoll_ctx_register_conn (ctx, conn);
}

/** 
 * @internal Function used to unregister the provided connection from
 * the provided context, releasing the reference that was acquired on
 * it by \ref nopoll_ctx_register_conn.
 *
 * @param ctx The context where the connection is registered.
 *
 * @param conn The connection to be unregistered.
 */
void           nopoll_ctx_unregister_conn (noPollCtx  * ctx,
					   noPollConn * conn)
{
	int iterator;

	nopoll_return_if_fail (ctx, ctx && conn);

	/* acquire mutex here */
	nopoll_mutex_lock (ctx->ref_mutex);

	/* find the connection and remove it from the array */
	iterator = 0;
	while (iterator < ctx->conn_length) {

		/* check the connection reference */
		if (ctx->conn_list && ctx->conn_list[iterator] && ctx->conn_list[iterator]->id == conn->id) {
			/* remove reference */
			ctx->conn_list[iterator] = NULL;

			/* update connection list number */
			ctx->conn_num--;

			/* release the mutex before dropping the
			 * reference: nopoll_conn_unref takes its own
			 * lock and may destroy the connection */
			nopoll_mutex_unlock (ctx->ref_mutex);

			/* release the reference acquired at registration */
			nopoll_conn_unref (conn);

			return;
		} /* end if */
		
		iterator++;
	} /* end while */

	/* release mutex here */
	nopoll_mutex_unlock (ctx->ref_mutex);

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
	int result;

	nopoll_return_val_if_fail (ctx, ctx, -1);

	/* read the counter under the mutex: it is updated by
	 * nopoll_ctx_register_conn and nopoll_ctx_unregister_conn */
	nopoll_mutex_lock (ctx->ref_mutex);
	result = ctx->conn_num;
	nopoll_mutex_unlock (ctx->ref_mutex);

	return result;
}

/** 
 * @brief Allows to find the certificate associated to the provided serverName. 
 *
 * @param ctx The context where the operation will take place.
 *
 * @param serverName the servername to use as pattern to find the
 * right certificate. If NULL is provided the function first looks for
 * a certificate that isn't associated to any serverName and, when
 * there is none, it falls back to reporting the first certificate
 * stored, whatever serverName it is associated to.
 *
 * @param certificateFile If provided a reference and the function
 * returns nopoll_true, it will contain the certificateFile found.
 *
 * @param privateKey If provided a reference and the function
 * returns nopoll_true, it will contain the privateKey found.
 *
 * @param optionalChainFile If provided a reference and the function
 * returns nopoll_true, it will contain the optionalChainFile found.
 *
 * NOTE: the values reported are references owned by the context, not
 * copies: they must not be released by the caller and they are only
 * valid while the context is alive and the certificate is not
 * replaced.
 *
 * @return nopoll_true in the case the certificate was found,
 * otherwise nopoll_false is returned.
 */
nopoll_bool    nopoll_ctx_find_certificate (noPollCtx   * ctx, 
					    const char  * serverName, 
					    const char ** certificateFile, 
					    const char ** privateKey, 
					    const char ** optionalChainFile)
{
	noPollCertificate * cert;

	int iterator = 0;
	nopoll_return_val_if_fail (ctx, ctx, nopoll_false);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Finding a certificate for serverName=%s", serverName ? serverName : "<not defined>");

	/* acquire the mutex to walk the certificate store while it may
	 * be updated by nopoll_ctx_set_certificate () */
	nopoll_mutex_lock (ctx->ref_mutex);

	while (iterator < ctx->certificates_length) {
		/* get cert */
		cert = &(ctx->certificates[iterator]);

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "   certificate stored associated to serverName=%s", cert->serverName ? cert->serverName : "<not defined>");
		if ((serverName == NULL && cert->serverName == NULL)  ||
		    (nopoll_cmp (serverName, cert->serverName))) {
			if (certificateFile)
				(*certificateFile)   = cert->certificateFile;
			if (privateKey)
				(*privateKey)        = cert->privateKey;
			if (optionalChainFile)
				(*optionalChainFile) = cert->optionalChainFile;

			nopoll_mutex_unlock (ctx->ref_mutex);
			return nopoll_true;
		} /* end if */

		/* next position */
		iterator++;
	} /* end while */

	/* check for default certificate when serverName isn't defined:
	 * report the first certificate stored, whatever serverName it
	 * is associated to.
	 *
	 * NOTE: this loop used to have the iterator++ placed outside
	 * the while body (a misplaced brace). It did not spin because
	 * cert is the address of an array position, so it is never
	 * NULL and the first iteration always returned */
	if (serverName == NULL && ctx->certificates_length > 0) {
		cert = &(ctx->certificates[0]);

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "   serverName not defined, selecting first certificate from the list");
		if (certificateFile)
			(*certificateFile)   = cert->certificateFile;
		if (privateKey)
			(*privateKey)        = cert->privateKey;
		if (optionalChainFile)
			(*optionalChainFile) = cert->optionalChainFile;

		nopoll_mutex_unlock (ctx->ref_mutex);
		return nopoll_true;
	} /* end if */

	nopoll_mutex_unlock (ctx->ref_mutex);

	return nopoll_false;
}

/**
 * @internal Looks for a certificate stored under exactly the
 * serverName provided, without the fallback to the default
 * certificate implemented by \ref nopoll_ctx_find_certificate.
 *
 * NOTE: the caller must hold ctx->ref_mutex.
 *
 * @param ctx The context whose certificate store is checked.
 *
 * @param serverName The serverName to look for (NULL matches the
 * certificate stored without serverName).
 *
 * @return nopoll_true when a certificate with that exact serverName
 * is stored, otherwise nopoll_false.
 */
static nopoll_bool __nopoll_ctx_find_certificate_exact (noPollCtx * ctx, const char * serverName)
{
	int iterator = 0;

	while (iterator < ctx->certificates_length) {
		if (nopoll_cmp (serverName, ctx->certificates[iterator].serverName))
			return nopoll_true;

		/* next position */
		iterator++;
	} /* end while */

	return nopoll_false;
}

/**
 * @brief Allows to install a certificate to be used in general by all
 * listener connections working under the provided context.
 *
 * @param ctx The context where the certificate will be installed.
 *
 * @param serverName The optional server name to limit the use of
 * this certificate to the value provided here. Provide a NULL value
 * to make the certificate work under any server notified
 * (Host: header) or via SNI (server name identification associated to
 * the TLS transport).
 *
 * @param certificateFile The certificate file to be installed. 
 *
 * @param privateKey The private key file to be used.
 *
 * @param optionalChainFile Optional chain file with additional
 * material to complete the certificate definition.
 *
 * @return nopoll_true if the certificate was installed otherwise
 * nopoll_false. The function returns nopoll_false when ctx, certificateFile or privateKey are NULL.
 */ 
nopoll_bool           nopoll_ctx_set_certificate (noPollCtx  * ctx, 
						  const char * serverName, 
						  const char * certificateFile, 
						  const char * privateKey, 
						  const char * optionalChainFile)
{
	int length;
	noPollCertificate * cert;
	noPollCertificate * certificates;

	/* check values before proceed */
	nopoll_return_val_if_fail (ctx, ctx && certificateFile && privateKey, nopoll_false);

	/* acquire the mutex to protect the certificate store while it
	 * is checked and updated */
	nopoll_mutex_lock (ctx->ref_mutex);

	/* check if the certificate is already installed
	 *
	 * NOTE: this check used to call nopoll_ctx_find_certificate (),
	 * which falls back to reporting the first certificate stored
	 * when serverName is NULL. That made installing a certificate
	 * without serverName a silent no-op as soon as any other
	 * certificate was present. An exact match is what is needed
	 * here (and doing it inside the critical section also removes
	 * the check/update race) */
	if (__nopoll_ctx_find_certificate_exact (ctx, serverName)) {
		nopoll_mutex_unlock (ctx->ref_mutex);
		return nopoll_true;
	} /* end if */

	/* update certificate storage to hold all values */
	length = ctx->certificates_length + 1;
	if (length == 1)
		certificates = nopoll_new (noPollCertificate, 1);
	else
		certificates = (noPollCertificate *) nopoll_realloc (ctx->certificates, sizeof (noPollCertificate) * (length));

	/* check the allocation before using it: storing the result
	 * directly into ctx->certificates and then indexing it
	 * dereferenced NULL when the allocation failed, and left
	 * certificates_length already increased */
	if (certificates == NULL) {
		nopoll_mutex_unlock (ctx->ref_mutex);
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to store the certificate provided");
		return nopoll_false;
	} /* end if */

	ctx->certificates        = certificates;
	ctx->certificates_length = length;

	/* hold certificate */
	cert = &(ctx->certificates[length - 1]);

	cert->serverName = NULL;
	if (serverName)
		cert->serverName         = nopoll_strdup (serverName);

	cert->certificateFile = NULL;
	if (certificateFile)
		cert->certificateFile    = nopoll_strdup (certificateFile);

	cert->privateKey = NULL;
	if (privateKey)
		cert->privateKey         = nopoll_strdup (privateKey);

	cert->optionalChainFile = NULL;
	if (optionalChainFile)
		cert->optionalChainFile  = nopoll_strdup (optionalChainFile);

	/* release the mutex */
	nopoll_mutex_unlock (ctx->ref_mutex);

	return nopoll_true;
}

/** 
 * @brief Allows to configure the on open handler, the handler that is
 * called when an incoming websocket connection is received and all
 * websocket client handshake data was received (but not yet replied).
 *
 * This handler differs from \ref nopoll_ctx_set_on_accept in that this
 * one is called after all client handshake data was received.
 *
 * Note the connection is still not fully working at this point
 * because the handshake hasn't been sent to the remote peer yet. This
 * means that attempting to send any content inside this handler (for
 * example by using \ref nopoll_conn_send_text) will cause a protocol
 * violation (because remote side is expecting a handshake reply but
 * received something different). 
 *
 * In the case you want to sent content right away after receiving a
 * connection (on a listener), you can use \ref
 * nopoll_ctx_set_on_ready "On Ready" handler which is called just
 * after the connection has been fully accepted and handshake reply is
 * fully written.
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
	/* NOTE: a NULL handler is rejected by the check above, so the
	 * handler cannot be uninstalled through this function (unlike
	 * \ref nopoll_ctx_set_on_msg, which does accept NULL) */
	nopoll_return_if_fail (ctx, ctx && on_open);

	/* set the handler */
	ctx->on_open      = on_open;
	ctx->on_open_data = user_data;
	return;
}

/** 
 * @brief Allows to configure a handler that is called when a
 * connection is received and it is ready to send and receive because
 * all WebSocket handshake protocol finished OK.
 *
 * Unlike handlers configured at \ref nopoll_ctx_set_on_open and \ref
 * nopoll_ctx_set_on_accept which get notified when the connection
 * isn't still working (because WebSocket handshake wasn't finished
 * yet), on ready handlers configured here will get called just after
 * the WebSocket handshake has taken place.
 *
 * @param ctx The context that will be configured.
 *
 * @param on_ready The handler to be called when a connection is fully
 * ready to send and receive content because WebSocket handshake has
 * finished. The function must return nopoll_true to accept the
 * connection. By returning nopoll_false the handler is signalling to
 * terminate the connection.
 *
 * @param user_data Optional user data pointer passed to the on ready
 * handler.
 * 
 */
void           nopoll_ctx_set_on_ready (noPollCtx          * ctx,
					noPollActionHandler  on_ready,
					noPollPtr            user_data)
{
	/* see note at nopoll_ctx_set_on_open () about NULL handlers */
	nopoll_return_if_fail (ctx, ctx && on_ready);

	/* set the handler */
	ctx->on_ready      = on_ready;
	ctx->on_ready_data = user_data;
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
	/* see note at nopoll_ctx_set_on_open () about NULL handlers */
	nopoll_return_if_fail (ctx, ctx && on_accept);

	/* set the handler */
	ctx->on_accept      = on_accept;
	ctx->on_accept_data = user_data;
	return;
}

/** 
 * @brief Allows to set a general handler to get notifications about a
 * message received over any connection that is running under the
 * provided context (noPollCtx).
 *
 * @param ctx The context where the notification will happen
 *
 * @param on_msg The handler to be called when an incoming message is
 * received.
 *
 * @param user_data User defined pointer that is passed in into the
 * handler when called.
 *
 * Note that the handler configured here will be overridden by the handler configured by \ref nopoll_conn_set_on_msg
 *
 */
void           nopoll_ctx_set_on_msg    (noPollCtx              * ctx,
					 noPollOnMessageHandler   on_msg,
					 noPollPtr                user_data)
{
	nopoll_return_if_fail (ctx, ctx);
	
	/* set new handler */
	ctx->on_msg      = on_msg;
	ctx->on_msg_data = user_data;

	return;
}

/** 
 * @brief Allows to configure the handler that will be used to let
 * user land code to define OpenSSL SSL_CTX object.
 *
 * By default, SSL_CTX (SSL Context) object is created by default
 * settings that works for most of the cases. In the case you want to
 * configure particular configurations that should be enabled on the
 * provided SSL_CTX that is going to be used by the client ---while
 * connecting--- or server ---while receiving a connection--- then use
 * this function to setup your creator handler.
 *
 * See \ref noPollSslContextCreator for more information about this
 * handler.
 *
 * @param ctx The context that will be configured.
 *
 * @param context_creator The handler to be called to create the
 * SSL_CTX object. Passing NULL restores the default creation done by
 * the library.
 *
 * @param user_data User defined pointer to be passed to the handler
 * when it is called.
 */
void           nopoll_ctx_set_ssl_context_creator (noPollCtx                * ctx,
						   noPollSslContextCreator    context_creator,
						   noPollPtr                  user_data)
{
	if (ctx == NULL)
		return;

	/* set handlers as indicated by the caller */
	ctx->context_creator      = context_creator;
	ctx->context_creator_data = user_data;
	return;
}

/** 
 * @brief Allows to configure a function that will implement a post SSL/TLS check.
 *
 * See the following function to get more information: \ref noPollSslPostCheck
 *
 * @param ctx The context where the operation is taking place.
 *
 * @param post_ssl_check The handler that is going to be called
 * everything a new connection with SSL is established by a client or
 * received by a server. The handler is executed right after the SSL
 * handshake finishes without error.
 *
 * @param user_data A reference to user defined pointer that will be
 * passed in to the handler.
 */
void           nopoll_ctx_set_post_ssl_check (noPollCtx          * ctx,
					      noPollSslPostCheck   post_ssl_check,
					      noPollPtr            user_data)
{
	if (ctx == NULL)
		return;

	/* set handlers as indicated by the caller */
	ctx->post_ssl_check      = post_ssl_check;
	ctx->post_ssl_check_data = user_data;
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
 * @return Returns the connection selected (that is, the one for which
 * the foreach handler returned nopoll_true) or NULL in the case all
 * foreach executions returned nopoll_false. Keep in mind the function
 * also returns NULL if ctx or foreach parameter is NULL.
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
	nopoll_mutex_lock (ctx->ref_mutex);

	/* nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Doing foreach over conn_length array (%p): %d", ctx, ctx->conn_length); */
	
	/* find the connection and remove it from the array */
	iterator = 0;
	while (iterator < ctx->conn_length) {

		/* check the connection reference */
		if (ctx->conn_list[iterator]) {

			result = ctx->conn_list[iterator];

			/* acquire a reference before releasing the
			 * mutex: otherwise another thread may
			 * unregister and destroy this connection while
			 * the handler below is using it */
			nopoll_conn_ref (result);

			/* release the mutex before calling user code:
			 * the handler may call back into the API */
			nopoll_mutex_unlock (ctx->ref_mutex);

			/* call to notify connection */
			if (foreach (ctx, result, user_data)) {

				/* selected: drop our own reference and
				 * report it to the caller (the mutex is
				 * already released here) */
				nopoll_conn_unref (result);
				return result;
			} /* end if */

			/* not selected: drop our own reference */
			nopoll_conn_unref (result);

			/* acquire the mutex again to continue walking
			 * the connection list */
			nopoll_mutex_lock (ctx->ref_mutex);

		} /* end if */
		
		iterator++;
	} /* end while */

	/* release here the mutex to protect connection list */
	nopoll_mutex_unlock (ctx->ref_mutex);

	return NULL;
}


/** 
 * @brief Allows to change the protocol version that is sent in all
 * client connections created under the provided context and the
 * protocol version accepted by listeners created under this context
 * too.
 *
 * This is a really basic (mostly fake) protocol version support
 * because it only allows to change the version string sent (but
 * nothing more for now). It is useful for testing purposes.
 *
 * @param ctx The noPoll context where the protocol version change
 * will be applied.
 *
 * @param version The value representing the protocol version. By
 * default this function isn't required to be called because it
 * already has the right protocol value configured (13). 
 */ 
void           nopoll_ctx_set_protocol_version (noPollCtx * ctx, int version)
{
	/* check input data
	 *
	 * NOTE: this used to check (ctx || version), which lets a NULL
	 * ctx through whenever version is not 0, dereferencing it
	 * right below */
	nopoll_return_if_fail (ctx, ctx);

	/* setup the new protocol version */
	ctx->protocol_version = version;

	return;
}

/**
 * @brief Allows to configure the maximum websocket frame size
 * (payload) that will be accepted by connections created under the
 * provided context.
 *
 * Any incoming frame declaring a payload size bigger than this value
 * causes the connection to be closed without allocating memory for
 * it. This limit protects the application from remote peers declaring
 * huge (or crafted) frame sizes.
 *
 * By default, every context is configured with \ref
 * NOPOLL_MAX_FRAME_SIZE_DEFAULT (16MB). The value can be also
 * configured for a particular connection or listener, which takes
 * precedence over this one, by using \ref
 * nopoll_conn_opts_set_max_frame_size.
 *
 * @param ctx The context where the limit will be configured.
 *
 * @param max_frame_size The maximum frame size to accept. It must be
 * a value bigger than 0 and equal or lower than \ref
 * NOPOLL_MAX_FRAME_SIZE_LIMIT. Any other value is discarded, keeping
 * the current configuration.
 */
void           nopoll_ctx_set_max_frame_size (noPollCtx * ctx, long int max_frame_size)
{
	/* check input data */
	nopoll_return_if_fail (ctx, ctx);

	if (max_frame_size <= 0 || max_frame_size > NOPOLL_MAX_FRAME_SIZE_LIMIT) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received wrong max frame size value (%ld), it must be inside the range 1..%ld, discarding configuration",
			    max_frame_size, (long int) NOPOLL_MAX_FRAME_SIZE_LIMIT);
		return;
	} /* end if */

	/* setup the new limit */
	ctx->max_frame_size = max_frame_size;

	return;
}

/**
 * @brief Allows to get current maximum websocket frame size (payload)
 * accepted by connections created under the provided context.
 *
 * @param ctx The context that is being checked.
 *
 * @return Current limit configured or \ref
 * NOPOLL_MAX_FRAME_SIZE_DEFAULT in the case of wrong reference
 * received or no limit was configured.
 */
long int       nopoll_ctx_get_max_frame_size (noPollCtx * ctx)
{
	if (ctx == NULL || ctx->max_frame_size <= 0)
		return NOPOLL_MAX_FRAME_SIZE_DEFAULT;

	return ctx->max_frame_size;
}

/**
 * @}
 */

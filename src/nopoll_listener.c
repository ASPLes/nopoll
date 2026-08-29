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
#include <nopoll_listener.h>
#include <nopoll_private.h>

/** 
 * \defgroup nopoll_listener noPoll Listener: functions required to create WebSocket listener connections.
 */

/** 
 * \addtogroup nopoll_listener
 * @{
 */

/**
 * @internal Fills the host and port strings out of the socket address
 * provided, supporting both IPv4 and IPv6.
 *
 * PORTABILITY NOTE (inet_ntop): noPoll uses inet_ntop () instead of
 * inet_ntoa (). inet_ntoa () keeps its result in a static buffer (so
 * it is not thread safe: two connections accepted at the same time
 * could report each other's address) and it cannot represent IPv6
 * addresses at all. Platforms that only provide inet_ntoa () are NOT
 * supported: this function is the only place that would have to be
 * revisited to add such support, so grep for "PORTABILITY NOTE
 * (inet_ntop)" if that need ever appears.
 *
 * @param addr The address to format, as reported by getsockname () or
 * getpeername ().
 *
 * @param host Reference where the address will be left (newly
 * allocated).
 *
 * @param port Reference where the port will be left (newly
 * allocated).
 *
 * @return nopoll_true if both values were produced, otherwise
 * nopoll_false.
 */
static nopoll_bool __nopoll_listener_get_host_port (struct sockaddr_storage  * addr,
						    char                    ** host,
						    char                    ** port)
{
	char   buffer[64];
	int    _port;

	memset (buffer, 0, sizeof (buffer));

	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 * sin6 = (struct sockaddr_in6 *) addr;

		if (inet_ntop (AF_INET6, &(sin6->sin6_addr), buffer, sizeof (buffer)) == NULL)
			return nopoll_false;
		_port = ntohs (sin6->sin6_port);
	} else {
		struct sockaddr_in * sin4 = (struct sockaddr_in *) addr;

		if (inet_ntop (AF_INET, &(sin4->sin_addr), buffer, sizeof (buffer)) == NULL)
			return nopoll_false;
		_port = ntohs (sin4->sin_port);
	} /* end if */

	(*host) = nopoll_strdup (buffer);
	(*port) = nopoll_strdup_printf ("%d", _port);

	/* on failure release what was allocated and report both values
	 * as undefined: reporting nopoll_false while leaving one of them
	 * allocated leaked it at the callers that do not release them */
	if ((*host) == NULL || (*port) == NULL) {
		nopoll_free (*host);
		nopoll_free (*port);
		(*host) = NULL;
		(*port) = NULL;
		return nopoll_false;
	} /* end if */

	return nopoll_true;
}

/**
 * @internal Implementation used by all sock listener functions: it
 * resolves the host/port received, creates the socket, binds it and
 * leaves it listening.
 *
 * @return The listening socket, or a negative value if it fails: -1
 * when the listener could not be created and -2 when wrong parameters
 * were received.
 */
NOPOLL_SOCKET     __nopoll_listener_sock_listen_internal      (noPollCtx        * ctx,
							       noPollTransport    transport,
							       const char       * host,
							       const char       * port)
{
	/* NOTE: sockaddr_storage is required to hold an IPv6 address:
	 * with a sockaddr_in, getsockname () reports success but
	 * truncates the address */
	struct sockaddr_storage sin;
	NOPOLL_SOCKET        fd;
	int                  tries;
	struct addrinfo      hints, *res = NULL;
	char               * local_host = NULL;
	char               * local_port = NULL;

#if defined(NOPOLL_OS_WIN32)
	int                  sin_size  = sizeof (sin);
#else    	
	int                  unit      = 1; 
	socklen_t            sin_size  = sizeof (sin);
#endif	
#if defined(SHOW_DEBUG_LOG)
	uint16_t             int_port;
#endif
	int                  bind_res;

	nopoll_return_val_if_fail (ctx, ctx,  -2);
	nopoll_return_val_if_fail (ctx, host, -2);
	/* NOTE: this check used to be (port || strlen (port) == 0),
	 * which evaluates strlen (NULL) precisely when port is NULL:
	 * passing NULL crashed instead of being rejected */
	nopoll_return_val_if_fail (ctx, port && strlen (port) > 0, -2);

	/* clear hints structure */
	memset (&hints, 0, sizeof(struct addrinfo));

	/* resolve hostname */
	switch (transport) {
	case NOPOLL_TRANSPORT_IPV4:
		/* configure hints */
		hints.ai_family   = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags    = AI_PASSIVE | AI_NUMERICHOST;
		
		/* resolve hosting name */
		if (getaddrinfo (host, port, &hints, &res) != 0) {
			nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "unable to resolve host name %s, errno=%d", host, errno);
			return -1;
		} /* end if */
		
		break;
	case NOPOLL_TRANSPORT_IPV6:
		/* configure hints */
		hints.ai_family   = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags    = AI_PASSIVE | AI_NUMERICHOST;

		/* check value received
		 *
		 * NOTE: memcmp () was reading 7 octets unconditionally,
		 * running past the end of shorter host values like "::1" */
		if (nopoll_cmp (host, "0.0.0.0")) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received an address (%s) that is not a valid IPv6 address..", host);
			return -1;
		} /* end if */
		
		/* resolve hosting name */
		if (getaddrinfo (host, port, &hints, &res) != 0) {
			nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "unable to resolve host name %s, errno=%d", host, errno);
			return -1;
		} /* end if */
		break;
	default:
		/* unsupported transport: nothing was resolved, so there
		 * is nothing to release either. Without this case res
		 * stays NULL and it is dereferenced right below */
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received unsupported transport value (%d), unable to create listener", transport);
		return -1;
	} /* end switch */

	/* create socket */
	fd = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
	if (! nopoll_socket_is_valid (fd)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "failed to create listener socket (errno=%d : %s)", errno, strerror (errno));
		freeaddrinfo (res);
		return -1;
	} /* end if */

	if (fd <= 2) {
		/* do not allow creating sockets reusing stdin (0),
		   stdout (1), stderr (2): close the descriptor before
		   reporting the failure, otherwise it is leaked */
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "failed to create listener socket: %d (errno=%d)", fd, errno);
		nopoll_close_socket (fd);
		freeaddrinfo (res);
		return -1;
        } /* end if */

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "socket=%d created for %s:%s", fd, host, port);


#if defined(NOPOLL_OS_WIN32)
	/* Do not issue a reuse addr which causes on windows to reuse
	 * the same address:port for the same process. Under linux,
	 * reusing the address means that consecutive process can
	 * reuse the address without being blocked by a wait
	 * state.  */
	/* setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char  *)&unit, sizeof(BOOL)); */
#else
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &unit, sizeof (unit));
#endif 

#if defined(SHOW_DEBUG_LOG)
	/* get integer port */
	int_port  = (uint16_t) atoi (port);
#endif

	/* call to bind */
	tries    = 0;
	while (1) {
		/* call bind */
		bind_res = bind(fd, res->ai_addr, res->ai_addrlen);
		if (bind_res == NOPOLL_SOCKET_ERROR) {
			/* check if we can retry */
			tries++;
			if (tries < 25) {
				nopoll_log (ctx, NOPOLL_LEVEL_WARNING, 
					    "unable to bind address (port:%u already in use or insufficient permissions, errno=%d : %s), retrying=%d on socket: %d", 
					    int_port, errno, strerror (errno), tries, fd);
				nopoll_sleep (100000);
				continue;
			} /* end if */

			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, 
				    "unable to bind address (port:%u already in use or insufficient permissions, errno=%d : %s). Closing socket: %d", 
				    int_port, errno, strerror (errno), fd);
			nopoll_close_socket (fd);

			/* release addr info */
			freeaddrinfo (res);
			return -1;
		} /* end if */

		/* reached this point, bind was ok */
		break;
	} /* end while */

	/* release addr info */
	freeaddrinfo (res);
	
	if (listen(fd, ctx->backlog) == NOPOLL_SOCKET_ERROR) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "an error was found while executing listen () over socket %d (errno=%d : %s)",
			    fd, errno, strerror (errno));
		nopoll_close_socket (fd);
		return -1;
        } /* end if */

	/* notify listener
	 *
	 * NOTE: the check used to be (< -1), which is never true
	 * because getsockname () reports -1 on failure, so errors were
	 * silently ignored and the socket was leaked on that path */
	if (getsockname (fd, (struct sockaddr *) &sin, &sin_size) < 0) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to get local address for socket %d (errno=%d : %s)",
			    fd, errno, strerror (errno));
		nopoll_close_socket (fd);
		return -1;
	} /* end if */

	/* report and return fd */
	if (__nopoll_listener_get_host_port (&sin, &local_host, &local_port)) {
		nopoll_log  (ctx, NOPOLL_LEVEL_DEBUG, "running listener at %s:%s (socket: %d)", local_host, local_port, fd);
		nopoll_free (local_host);
		nopoll_free (local_port);
	} /* end if */

	return fd;
}

/**
 * @internal Function to create a WebSocket listener
 *
 * NOTE about the options object: unless it is flagged for reuse (see
 * \ref nopoll_conn_opts_set_reuse), the caller hands over the object
 * to this function, so every exit path must release it. On success the
 * object is kept at listener->opts and released when the listener is
 * destroyed.
 */
noPollConn      * __nopoll_listener_new_opts_internal (noPollCtx      * ctx,
						       noPollTransport  transport,
						       noPollConnOpts * opts,
						       const char     * host,
						       const char     * port)
{
	NOPOLL_SOCKET   session;
	noPollConn    * listener;

	if (ctx == NULL || host == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received NULL ctx or host reference, unable to create listener");

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (opts);
		return NULL;
	} /* end if */

	/* call to create the socket
	 *
	 * NOTE: the internal function also reports -2 when it receives
	 * wrong parameters, and checking only against
	 * NOPOLL_INVALID_SOCKET (-1) let that value through, building
	 * a listener over an invalid socket. \ref nopoll_socket_is_valid
	 * covers every failure indication */
	session = __nopoll_listener_sock_listen_internal (ctx, transport, host, port);
	if (! nopoll_socket_is_valid (session)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to start listener error was: errno=%d", errno);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (opts);
		return NULL;
	} /* end if */

	/* create the noPollConn object */
	listener           = nopoll_new (noPollConn, 1);
	if (listener == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory for the listener, closing socket %d", session);
		nopoll_close_socket (session);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (opts);
		return NULL;
	} /* end if */
	listener->refs     = 1;
	/* create mutex */
	listener->ref_mutex = nopoll_mutex_create ();
	listener->handshake_mutex = nopoll_mutex_create ();
	listener->session   = session;
	listener->ctx       = ctx;
	listener->role      = NOPOLL_ROLE_MAIN_LISTENER;

	/* record host and port */
	listener->host      = nopoll_strdup (host);
	listener->port      = nopoll_strdup (port);
	if (listener->host == NULL || listener->port == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to store listener host and port, unable to create listener");

		/* release what was acquired so far: the connection is
		 * not registered yet, so no context reference was taken */
		nopoll_free (listener->host);
		nopoll_free (listener->port);
		nopoll_mutex_destroy (listener->handshake_mutex);
		nopoll_mutex_destroy (listener->ref_mutex);
		nopoll_free (listener);
		nopoll_close_socket (session);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (opts);
		return NULL;
	} /* end if */

	/* register connection into context */
	if (! nopoll_ctx_register_conn (ctx, listener)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to register listener into the context, unable to create listener");

		/* release what was acquired so far: the context
		 * reference is only taken on a successful
		 * registration, so nopoll_conn_unref () must not be
		 * used here */
		nopoll_free (listener->host);
		nopoll_free (listener->port);
		nopoll_mutex_destroy (listener->handshake_mutex);
		nopoll_mutex_destroy (listener->ref_mutex);
		nopoll_free (listener);
		nopoll_close_socket (session);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (opts);
		return NULL;
	} /* end if */

	/* configure default handlers */
	listener->receive   = nopoll_conn_default_receive;
	listener->send      = nopoll_conn_default_send;

	/* configure connection options */
	listener->opts      = opts;

	/* record max frame size accepted (it is also transferred to
	 * every connection accepted by this listener) */
	__nopoll_conn_set_max_frame_size (listener, opts);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Listener created, started: %s:%s (socket: %d, transport: %s)",
		    listener->host, listener->port, listener->session, (transport == NOPOLL_TRANSPORT_IPV4 ? "IPv4" : "IPv6"));

	return listener;
}


/** 
 * @internal Creates a listener socket on the provided port.
 */
NOPOLL_SOCKET     nopoll_listener_sock_listen      (noPollCtx   * ctx,
						    const char  * host,
						    const char  * port)
{
	return __nopoll_listener_sock_listen_internal (ctx, NOPOLL_TRANSPORT_IPV4, host, port);
}

/** 
 * @brief Creates a new websocket server listener on the provided host
 * name and port (IPv4)
 *
 * @param ctx The context where the operation will take place.
 *
 * @param host The hostname or address interface to bind on.
 *
 * @param port The port where to listen. It is required: passing NULL or an empty string makes the function to fail.
 *
 * @return A reference to a \ref noPollConn object representing the
 * listener or NULL if it fails.
 */
noPollConn      * nopoll_listener_new (noPollCtx  * ctx,
				       const char * host,
				       const char * port)
{
	return nopoll_listener_new_opts (ctx, NULL, host, port);
}

/** 
 * @brief Creates a new websocket server listener on the provided host
 * name and port (IPv6).
 *
 * See \ref nopoll_listener_new for more information.
 *
 * @param ctx See \ref nopoll_listener_new for more information.
 *
 * @param host See \ref nopoll_listener_new for more information.
 *
 * @param port See \ref nopoll_listener_new for more information.
 *
 * @return See \ref nopoll_listener_new for more information.
 */
noPollConn      * nopoll_listener_new6 (noPollCtx  * ctx,
					const char * host,
					const char * port)
{
	return __nopoll_listener_new_opts_internal (ctx, NOPOLL_TRANSPORT_IPV6, NULL, host, port);
}

/** 
 * @brief Creates a new websocket server listener on the provided host
 * name and port (IPv4).
 *
 * @param ctx The context where the operation will take place.
 *
 * @param opts Optional connection options to configure this listener.
 *
 * @param host The hostname or address interface to bind on.
 *
 * @param port The port where to listen. It is required: passing NULL or an empty string makes the function to fail.
 *
 * @return A reference to a \ref noPollConn object representing the
 * listener or NULL if it fails.
 */
noPollConn      * nopoll_listener_new_opts (noPollCtx      * ctx,
					    noPollConnOpts * opts,
					    const char     * host,
					    const char     * port)
{
	/* call common implementation */
	return __nopoll_listener_new_opts_internal (ctx, NOPOLL_TRANSPORT_IPV4, opts, host, port);
}

/** 
 * @brief Creates a new websocket server listener on the provided host
 * name and port (IPv6).
 *
 * See \ref nopoll_listener_new_opts for more information.
 *
 * @param ctx See \ref nopoll_listener_new_opts
 *
 * @param opts See \ref nopoll_listener_new_opts
 *
 * @param host See \ref nopoll_listener_new_opts
 *
 * @param port See \ref nopoll_listener_new_opts
 *
 * @return See \ref nopoll_listener_new_opts
 */
noPollConn      * nopoll_listener_new_opts6 (noPollCtx      * ctx,
					     noPollConnOpts * opts,
					     const char     * host,
					     const char     * port)
{
	/* call common implementation */
	return __nopoll_listener_new_opts_internal (ctx, NOPOLL_TRANSPORT_IPV6, opts, host, port);
}

/** 
 * @brief Allows to create a new WebSocket listener but expecting the
 * incoming connection to be under TLS supervision. The function works
 * like \ref nopoll_listener_new (providing wss:// services) (IPv4 version).
 *
 * @param ctx The context where the operation will take place.
 *
 * @param host The hostname or address interface to bind on.
 *
 * @param port The port where to listen. It is required: passing NULL or an empty string makes the function to fail.
 *
 * @return A reference to a \ref noPollConn object representing the
 * listener or NULL if it fails.
 */
noPollConn      * nopoll_listener_tls_new (noPollCtx  * ctx,
					   const char * host,
					   const char * port)
{
	return nopoll_listener_tls_new_opts (ctx, NULL, host, port);
}

/** 
 * @brief Allows to create a new WebSocket listener but expecting the
 * incoming connection to be under TLS supervision. The function works
 * like \ref nopoll_listener_new (providing wss:// services) (IPv6 version).
 *
 * See \ref nopoll_listener_tls_new for more information.
 *
 * @param ctx See \ref nopoll_listener_tls_new for more information.
 *
 * @param host See \ref nopoll_listener_tls_new for more information.
 *
 * @param port See \ref nopoll_listener_tls_new for more information.
 *
 * @return See \ref nopoll_listener_tls_new for more information.
 */
noPollConn      * nopoll_listener_tls_new6 (noPollCtx  * ctx,
					    const char * host,
					    const char * port)
{
	return nopoll_listener_tls_new_opts6 (ctx, NULL, host, port);
}


/** 
 * @internal Common implementation 
 */
noPollConn      * __nopoll_listener_tls_new_opts_internal (noPollCtx      * ctx,
							   noPollTransport  transport,
							   noPollConnOpts * opts,
							   const char     * host,
							   const char     * port)
{
	noPollConn * listener;

	/* call to get listener from base function */
	listener = __nopoll_listener_new_opts_internal (ctx, transport, opts, host, port);
	if (! listener)
		return listener;

	/* setup TLS support
	 *
	 * NOTE: listener->opts is already configured by
	 * __nopoll_listener_new_opts_internal (), so assigning it
	 * again here was redundant */
	listener->tls_on = nopoll_true;

	return listener;
}

/** 
 * @brief Allows to create a new WebSocket listener but expecting the
 * incoming connection to be under TLS supervision. The function works
 * like \ref nopoll_listener_new (providing wss:// services) (IPv4 version).
 *
 * @param ctx The context where the operation will take place.
 *
 * @param opts The connection options to configure this particular
 * listener.
 *
 * @param host The hostname or address interface to bind on.
 *
 * @param port The port where to listen. It is required: passing NULL or an empty string makes the function to fail.
 *
 * @return A reference to a \ref noPollConn object representing the
 * listener or NULL if it fails.
 */
noPollConn      * nopoll_listener_tls_new_opts (noPollCtx      * ctx,
						noPollConnOpts * opts,
						const char     * host,
						const char     * port)
{
	return __nopoll_listener_tls_new_opts_internal (ctx, NOPOLL_TRANSPORT_IPV4, opts, host, port);
}


/** 
 * @brief Allows to create a new WebSocket listener but expecting the
 * incoming connection to be under TLS supervision. The function works
 * like \ref nopoll_listener_new (providing wss:// services) (IPv6 version).
 *
 * See \ref nopoll_listener_tls_new_opts for more information.
 *
 * @param ctx See \ref nopoll_listener_tls_new_opts for more information.
 *
 * @param opts See \ref nopoll_listener_tls_new_opts for more information.
 *
 * @param host See \ref nopoll_listener_tls_new_opts for more information.
 *
 * @param port See \ref nopoll_listener_tls_new_opts for more information.
 *
 * @return See \ref nopoll_listener_tls_new_opts for more information.
 */
noPollConn      * nopoll_listener_tls_new_opts6 (noPollCtx      * ctx,
						 noPollConnOpts * opts,
						 const char     * host,
						 const char     * port)
{
	return __nopoll_listener_tls_new_opts_internal (ctx, NOPOLL_TRANSPORT_IPV6, opts, host, port);
}

/** 
 * @brief Allows to configure the TLS certificate and key to be used
 * on the provided connection.
 *
 * @param listener The listener that is going to be configured with the provided certificate and key.
 *
 * @param certificate The path to the public certificate file (PEM
 * format) to be used for every TLS connection received under the
 * provided listener.
 *
 * @param private_key The path to the key file (PEM format) to be used for
 * every TLS connection received under the provided listener.
 *
 * @param chain_file The path to additional chain certificates (PEM
 * format). You can safely pass here a NULL value.
 *
 * @return nopoll_true if the certificates were configured, otherwise
 * nopoll_false is returned.
 */
nopoll_bool           nopoll_listener_set_certificate (noPollConn * listener,
						       const char * certificate,
						       const char * private_key,
						       const char * chain_file)
{
	FILE * handle;

	if (! listener || ! certificate || ! private_key)
		return nopoll_false;

	/* check certificate file */
	handle = fopen (certificate, "r");
	if (! handle) {
		nopoll_log (listener->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to open certificate file from %s", certificate);
		return nopoll_false;
	} /* end if */
	fclose (handle);

	/* check private file */
	handle = fopen (private_key, "r");
	if (! handle) {
		nopoll_log (listener->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to open private key file from %s", private_key);
		return nopoll_false;
	} /* end if */
	fclose (handle);

	if (chain_file) {
		/* check private file */
		handle = fopen (chain_file, "r");
		if (! handle) {
			nopoll_log (listener->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to open chain certificate file from %s", chain_file);
			return nopoll_false;
		} /* end if */
		fclose (handle);
	} /* end if */

	/* copy certificates to be used
	 *
	 * NOTE: release any value configured by a previous call:
	 * assigning over the previous pointers leaked them */
	nopoll_free (listener->certificate);
	nopoll_free (listener->private_key);
	listener->certificate   = nopoll_strdup (certificate);
	listener->private_key   = nopoll_strdup (private_key);
	if (chain_file) {
		nopoll_free (listener->chain_certificate);
		listener->chain_certificate = nopoll_strdup (chain_file);
	} /* end if */

	/* check the duplications before reporting success: the previous
	 * values were already released above, so a memory failure here
	 * left the listener without the certificate it had configured
	 * while still reporting the new one was installed */
	if (listener->certificate == NULL || listener->private_key == NULL ||
	    (chain_file && listener->chain_certificate == NULL)) {
		nopoll_log (listener->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to store the certificates provided, conn id: %d", listener->id);
		return nopoll_false;
	} /* end if */


	nopoll_log (listener->ctx, NOPOLL_LEVEL_DEBUG, "Configured certificate: %s, key: %s, for conn id: %d",
		    listener->certificate, listener->private_key, listener->id);

	/* certificates configured */
	return nopoll_true;
}

/**
 * @brief Creates a websocket connection object from the socket
 * provided, representing a connection already accepted by a listener:
 * the object is created with role \ref NOPOLL_ROLE_LISTENER, not with
 * \ref NOPOLL_ROLE_MAIN_LISTENER.
 *
 * The socket received must be a connected one, because the function
 * queries the remote peer through getpeername () to record its
 * address and port.
 *
 * This is the function used to implement port sharing, where the
 * socket is accepted by the application and then handed over to
 * noPoll (see \ref nopoll_conn_accept_complete).
 *
 * @param ctx The context where the connection will be associated.
 *
 * @param session The already accepted socket to associate to the
 * connection.
 *
 * @return A reference to the connection object created or NULL if it
 * fails.
 */
noPollConn   * nopoll_listener_from_socket (noPollCtx      * ctx,
					    NOPOLL_SOCKET    session)
{
	noPollConn         * listener;
	/* NOTE: sockaddr_storage is required to hold an IPv6 peer:
	 * with a sockaddr_in, getpeername () reports success but the
	 * address is truncated, so every IPv6 connection accepted was
	 * recording 0.0.0.0 as remote host */
	struct sockaddr_storage sin;
#if defined(NOPOLL_OS_WIN32)
	/* windows flavors */
	int                  sin_size = sizeof (sin);
#else
	/* unix flavors */
	socklen_t            sin_size = sizeof (sin);
#endif

	nopoll_return_val_if_fail (ctx, ctx && session > 0, NULL);
	
	/* create the noPollConn object */
	listener            = nopoll_new (noPollConn, 1);
	if (listener == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to create the connection object");
		return NULL;
	} /* end if */
	listener->refs      = 1;
	/* create mutex */
	listener->ref_mutex = nopoll_mutex_create ();
	listener->handshake_mutex = nopoll_mutex_create ();
	listener->session   = session;
	listener->ctx       = ctx;
	listener->role      = NOPOLL_ROLE_LISTENER;

	/* get peer value
	 *
	 * NOTE: the check used to be (< -1), which is never true
	 * because getpeername () reports -1 on failure, so a failure
	 * went unnoticed and the connection was created recording
	 * 0.0.0.0:0 as the remote peer */
	memset (&sin, 0, sizeof (sin));
	if (getpeername (session, (struct sockaddr *) &sin, &sin_size) < 0) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to get remote hostname and port (errno=%d : %s)", errno, strerror (errno));

		/* release what was acquired so far (the connection is
		 * not registered yet, so no context reference was
		 * taken) */
		nopoll_mutex_destroy (listener->handshake_mutex);
		nopoll_mutex_destroy (listener->ref_mutex);
		nopoll_free (listener);
		return NULL;
	} /* end if */

	/* record host and port */
	if (! __nopoll_listener_get_host_port (&sin, &(listener->host), &(listener->port))) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to format remote hostname and port for the connection received");

		/* release what was acquired so far (the connection is
		 * not registered yet, so no context reference was
		 * taken) */
		nopoll_free (listener->host);
		nopoll_free (listener->port);
		nopoll_mutex_destroy (listener->handshake_mutex);
		nopoll_mutex_destroy (listener->ref_mutex);
		nopoll_free (listener);
		return NULL;
	} /* end if */

	/* configure default handlers */
	listener->receive = nopoll_conn_default_receive;
	listener->send    = nopoll_conn_default_send;

	/* register connection into context */
	if (! nopoll_ctx_register_conn (ctx, listener)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to register connection into the context, unable to create connection");

		/* NOTE: this used to call nopoll_conn_ref (), acquiring
		 * a reference instead of releasing anything, so the
		 * whole connection object was leaked. It cannot call
		 * nopoll_conn_unref () either, because that would drop
		 * a context reference that was never acquired (the
		 * registration is what takes it) */
		nopoll_free (listener->host);
		nopoll_free (listener->port);
		nopoll_mutex_destroy (listener->handshake_mutex);
		nopoll_mutex_destroy (listener->ref_mutex);
		nopoll_free (listener);
		return NULL;
	} /* end if */

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Listener created, started: %s:%s (socket: %d)", listener->host, listener->port, listener->session);

	/* reduce reference counting here because ctx_register_conn
	 * already acquired a reference */
	nopoll_conn_unref (listener); 
	
	return listener;
}

/** 
 * @internal Function that performs a TCP listener accept.
 *
 * @param server_socket The listener socket where the accept()
 * operation will be called.
 *
 * @return Returns a connected socket descriptor or -1 if it fails.
 */
NOPOLL_SOCKET nopoll_listener_accept (NOPOLL_SOCKET server_socket)
{
	struct sockaddr_in inet_addr;
#if defined(NOPOLL_OS_WIN32)
	int               addrlen;
#else
	socklen_t         addrlen;
#endif
	NOPOLL_SOCKET     result;
	int               tries = 0;

	addrlen       = sizeof(struct sockaddr_in);

	/* accept the new connection, retrying a bounded number of
	 * times when the call is interrupted by a signal: an EINTR is
	 * not a failure.
	 *
	 * NOTE: the retry is limited on purpose. The listener socket is
	 * blocking, so this is not a busy loop (the process sleeps
	 * inside accept () between signals), but retrying without a
	 * limit would let a high rate of signals keep this function
	 * from ever returning, taking the control away from the
	 * caller. After exhausting the retries the error is reported
	 * back so the caller decides what to do. */
	while (tries < 5) {
		result = accept (server_socket, (struct sockaddr *)&inet_addr, &addrlen);
		if (result == NOPOLL_INVALID_SOCKET && errno == NOPOLL_EINTR) {
			tries++;
			continue;
		} /* end if */

		return result;
	} /* end while */

	return result;
}

/**
 * @}
 */

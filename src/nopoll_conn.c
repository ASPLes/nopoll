/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2017 Advanced Software Production Line, S.L.
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

/** 
 * \defgroup nopoll_conn noPoll Connection: functions required to create WebSocket client connections.
 */

/** 
 * \addtogroup nopoll_conn
 * @{
 */

#include <nopoll_conn.h>
#include <nopoll_private.h>

#if defined(NOPOLL_OS_UNIX)
# include <netinet/tcp.h>
#endif


/** 
 * @brief Allows to enable/disable non-blocking/blocking behavior on
 * the provided socket.
 * 
 * @param socket The socket to be configured.
 *
 * @param enable nopoll_true to enable blocking I/O, otherwise use
 * nopoll_false to enable non blocking I/O.
 * 
 * @return nopoll_true if the operation was properly done, otherwise
 * nopoll_false is returned.
 */
nopoll_bool                 nopoll_conn_set_sock_block         (NOPOLL_SOCKET socket,
								nopoll_bool   enable)
{
#if defined(NOPOLL_OS_UNIX)
	int  flags;
#endif

	if (enable) {
		/* enable blocking mode */
#if defined(NOPOLL_OS_WIN32)
		if (!nopoll_win32_blocking_enable (socket)) {
			return nopoll_false;
		}
#else
		if ((flags = fcntl (socket, F_GETFL, 0)) < -1) {
			return nopoll_false;
		} /* end if */

		flags &= ~O_NONBLOCK;
		if (fcntl (socket, F_SETFL, flags) < -1) {
			return nopoll_false;
		} /* end if */
#endif
	} else {
		/* enable nonblocking mode */
#if defined(NOPOLL_OS_WIN32)
		/* win32 case */
		if (!nopoll_win32_nonblocking_enable (socket)) {
			return nopoll_false;
		}
#else
		/* unix case */
		if ((flags = fcntl (socket, F_GETFL, 0)) < -1) {
			return nopoll_false;
		}
		
		flags |= O_NONBLOCK;
		if (fcntl (socket, F_SETFL, flags) < -1) {
			return nopoll_false;
		}
#endif
	} /* end if */

	return nopoll_true;
}


/** 
 * @brief Allows to get current timeout set for \ref noPollConn
 * connect operation.
 *
 * See also \ref nopoll_conn_connect_timeout.
 *
 * @return Current timeout configured. Returned value is measured in
 * microseconds (1 second = 1000000 microseconds). If a null value is
 * received, 0 is return and no timeout is implemented.
 */
long              nopoll_conn_get_connect_timeout (noPollCtx * ctx)
{
	/* check context recevied */
	if (ctx == NULL) {
		/* get the the default connect */
		return (0);
	} /* end if */
		
	/* return current value */
	return ctx->conn_connect_std_timeout;
}

/** 
 * @brief Allows to configure nopoll connect timeout.
 * 
 * This function allows to set the TCP connect timeout used by \ref
 * nopoll_conn_new.
 *
 * @param ctx The context where the operation will be performed.
 *
 * @param microseconds_to_wait Timeout value to be used. The value
 * provided is measured in microseconds. Use 0 to restore the connect
 * timeout to the default value.
 */
void               nopoll_conn_connect_timeout (noPollCtx * ctx,
						long        microseconds_to_wait)
{
	/* check reference received */
	if (ctx == NULL)
		return;
	
	/* configure new value */
	ctx->conn_connect_std_timeout = microseconds_to_wait;

	return;
}


/** 
 * @brief Allows to configure tcp no delay flag (enable/disable Nagle
 * algorithm).
 * 
 * @param socket The socket to be configured.
 *
 * @param enable The value to be configured, nopoll_true to enable tcp no
 * delay.
 * 
 * @return nopoll_true if the operation is completed.
 */
nopoll_bool                 nopoll_conn_set_sock_tcp_nodelay   (NOPOLL_SOCKET socket,
								nopoll_bool      enable)
{
	/* local variables */
	int result;

#if defined(NOPOLL_OS_WIN32)
	BOOL   flag = enable ? TRUE : FALSE;
	result      = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char  *)&flag, sizeof(BOOL));
#else
	int    flag = enable;
	result      = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof (flag));
#endif
	if (result == NOPOLL_INVALID_SOCKET) {
		return nopoll_false;
	}

	/* properly configured */
	return nopoll_true;
} /* end */


/** 
 * @brief Allows to configure which network interface to bind to.
 * 
 * @param socket The socket to be configured.
 *
 * @param options The options defining the interface value to be configured.
 * 
 * @return nopoll_true if the operation is completed.
 */
nopoll_bool                 nopoll_conn_set_bind_interface (NOPOLL_SOCKET socket,
							    noPollConnOpts  * options)
{
	/* no local variable here please */

	if ((NULL != options) && (NULL != options->_interface)) {
#if defined(NOPOLL_OS_WIN32) || defined(NOPOLL_OS_WIN64)
		/* Windows still not supported: send us a patch! */ 
		return nopoll_false;
#elif defined(__APPLE__) || defined(__FreeBSD__)
		/* Mac/OSX: that supports  */
		/* bind to the interface */
		return setsockopt (socket, SOL_SOCKET, IP_RECVIF,
				   options->_interface, strlen(options->_interface) ) == 0;
#else
		/* Linux/Unix case: that supports SO_BINDTODEVICE */
		/* bind to the interface */
		return setsockopt (socket, SOL_SOCKET, SO_BINDTODEVICE,
				   options->_interface, strlen (options->_interface) ) == 0;
#endif
	}

	/* properly configured */
	return nopoll_true;
} /* end */

NOPOLL_SOCKET __nopoll_conn_sock_connect_opts_internal (noPollCtx       * ctx,
							noPollTransport   transport,
							const char      * host,
							const char      * port,
							noPollConnOpts  * options)
{

	struct addrinfo      hints, *res = NULL;
	NOPOLL_SOCKET        session     = NOPOLL_INVALID_SOCKET;

	/* clear hints structure */
	memset (&hints, 0, sizeof(struct addrinfo));

	switch (transport) {
	case NOPOLL_TRANSPORT_IPV4:
		/* configure hints */
		hints.ai_family   = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		
		/* resolve hosting name */
		if (getaddrinfo (host, port, &hints, &res) != 0) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "unable to resolve host name %s, errno=%d", host, errno);
			return -1;
		} /* end if */

		/* create the socket and check if it */
		session      = socket (AF_INET, SOCK_STREAM, 0);
		break;
	case NOPOLL_TRANSPORT_IPV6:
		/* configure hints */
		hints.ai_family   = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		
		/* resolve hosting name */
		if (getaddrinfo (host, port, &hints, &res) != 0) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "unable to resolve host name %s, errno=%d", host, errno);
			return -1;
		} /* end if */

		/* create the socket and check if it */
		session      = socket (AF_INET6, SOCK_STREAM, 0);
		break;
	} /* end switch */
	
	if (session == NOPOLL_INVALID_SOCKET) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to create socket");

		/* relase address info */
		freeaddrinfo (res);
		return -1;
	} /* end if */

	/* disable nagle */
	nopoll_conn_set_sock_tcp_nodelay (session, nopoll_true);

	/* bind to specified interface */
	if( nopoll_true != nopoll_conn_set_bind_interface (session, options) ) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to bind to specified interface");
		nopoll_close_socket (session);

		/* relase address info */
		freeaddrinfo (res);
		
		return -1;
	} /* end if */

	/* set non blocking status */
	nopoll_conn_set_sock_block (session, nopoll_false);
	
	/* do a tcp connect */
        if (connect (session, res->ai_addr, res->ai_addrlen) < 0) {
		if(errno != NOPOLL_EINPROGRESS && errno != NOPOLL_EWOULDBLOCK && errno != NOPOLL_ENOTCONN) { 
			nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "unable to connect to remote host %s:%s errno=%d",
				    host, port, errno);

		        shutdown (session, SHUT_RDWR);
                        nopoll_close_socket (session);

			/* relase address info */
			freeaddrinfo (res);
			
			return -1;
		} /* end if */
	} /* end if */

	/* relase address info */
	freeaddrinfo (res);

	/* return socket created */
	return session;
}

/** 
 * @internal Allows to create a plain socket connection against the
 * host and port provided.
 *
 * @param ctx The context where the connection happens.
 *
 * @param host The host server to connect to.
 *
 * @param port The port server to connect to.
 *
 * @param options The connection options to apply.
 *
 * @return A connected socket or -1 if it fails. 
 */
NOPOLL_SOCKET nopoll_conn_sock_connect_opts (noPollCtx       * ctx,
					     const char      * host,
					     const char      * port,
					     noPollConnOpts  * options)
{

	return __nopoll_conn_sock_connect_opts_internal (ctx, NOPOLL_TRANSPORT_IPV4, host, port, options);
}


/** 
 * @internal Allows to create a plain socket connection against the
 * host and port provided.
 *
 * @param ctx The context where the connection happens.
 *
 * @param host The host server to connect to.
 *
 * @param port The port server to connect to.
 *
 * @return A connected socket or -1 if it fails. 
 */
NOPOLL_SOCKET nopoll_conn_sock_connect (noPollCtx       * ctx,
					const char      * host,
					const char      * port)
{
	return nopoll_conn_sock_connect_opts (ctx, host, port, NULL);
}


/** 
 * @internal Function that builds the client init greetings that will
 * be send to the server according to registered implementation.
 */ 
char * __nopoll_conn_get_client_init (noPollConn * conn, noPollConnOpts * opts)
{
	/* build sec-websocket-key */
	char key[50];
	int  key_size = 50;
	char nonce[17];

	/* get the nonce */
	if (! nopoll_nonce (nonce, 16)) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to get nonce, unable to produce Sec-WebSocket-Key.");
		return NULL;
	} /* end if */

	/* now base 64 */
	if (! nopoll_base64_encode (nonce, 16, key, &key_size)) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to base 64 encode characters for Sec-WebSocket-Key");
		return NULL;
	}
	
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Created Sec-WebSocket-Key nonce: %s", key);

	/* create accept and store */
	conn->handshake = nopoll_new (noPollHandShake, 1);
	conn->handshake->expected_accept = nopoll_strdup (key);

	/* send initial handshake */
	return nopoll_strdup_printf ("GET %s HTTP/1.1"
				     "\r\nHost: %s"
				     "\r\nUpgrade: websocket"
				     "\r\nConnection: Upgrade"
				     "\r\nSec-WebSocket-Key: %s"
				     "\r\nSec-WebSocket-Version: %d"
				     "%s%s"
				     "%s%s"  /* Cookie */
				     "%s%s"  /* protocol part */
				     "%s"    /* extra arbitrary headers */
				     "\r\n\r\n",
				     conn->get_url,
				     conn->host_name,
				     /* sec-websocket-key */
				     key,
				     /* sec-websocket-version */
				     conn->ctx->protocol_version,
				     /* Origin (support not sending Origin: header in case it is not defined) */
				     (conn->origin != NULL && (opts == NULL || opts->add_origin_header)) ? "\r\nOrigin: " : "",
				     (conn->origin != NULL && (opts == NULL || opts->add_origin_header)) ? conn->origin : "",
				     /* Cookie */
				     (opts && opts->cookie) ? "\r\nCookie: " : "",
				     (opts && opts->cookie) ? opts->cookie : "",
				     /* protocol part */
				     conn->protocols ? "\r\nSec-WebSocket-Protocol: " : "",
				     conn->protocols ? conn->protocols : "",
				     /* extra arbitrary headers */
				     (opts && opts->extra_headers) ? opts->extra_headers : "");
}


/**
 * @internal Function that dumps all errors found on current ssl context.
 *
 * @return Always returns 0
 */
int nopoll_conn_log_ssl (noPollConn * conn)
{
#if defined(SHOW_DEBUG_LOG)
        noPollCtx      * ctx = conn->ctx;
#endif
	char             log_buffer [512];
	unsigned long    err;
	int              error_position;
	int              aux_position;
	
	while ((err = ERR_get_error()) != 0) {
		/* clear buffer */
		memset (log_buffer, 0, 512);

		/* dump error */
		ERR_error_string_n (err, log_buffer, sizeof (log_buffer));
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "tls stack: err=%d, %s (find reason(code) at openssl/ssl.h)", err, log_buffer);

		/* Check for recoverable errors:
		 * - openssl errstr 1409442E :: error:1409442E:SSL routines:SSL3_READ_BYTES:tlsv1 alert protocol version
		 */
		if (strstr (log_buffer, "1409442E")) 
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "tls stack: err=%d, %s :: found TLS mismatch (peers running different TLS versions)", err, log_buffer);

		/* find error code position */
		error_position = 0;
		while (log_buffer[error_position] != ':' && log_buffer[error_position] != 0 && error_position < 511)
			error_position++;
		error_position++;
		aux_position = error_position;
		while (log_buffer[aux_position] != 0) {
			if (log_buffer[aux_position] == ':') {
				log_buffer[aux_position] = 0;
				break;
			}
			aux_position++;
		} /* end while */
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "    details, run: openssl errstr %s", log_buffer + error_position);
	}

	recv (conn->session, log_buffer, 1, MSG_PEEK);
	nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "    noPoll id=%d, socket: %d (after testing errno: %d)",
		    conn->id, conn->session, errno);

	return (0);
}

int __nopoll_conn_tls_handle_error (noPollConn * conn, int res, const char * label, nopoll_bool * needs_retry)
{
	int ssl_err;

	(*needs_retry) = nopoll_false;

	/* get error returned */
	ssl_err = SSL_get_error (conn->ssl, res);
	switch (ssl_err) {
	case SSL_ERROR_NONE:
		/* no error, return the number of bytes read */
	        /* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "%s, ssl_err=%d, perfect, no error reported, bytes read=%d", 
		   label, ssl_err, res); */
		return res;
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_X509_LOOKUP:
	        nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "%s, ssl_err=%d returned that isn't ready to read/write: you should retry", 
			    label, ssl_err);
		(*needs_retry) = nopoll_true;
		return -2;
	case SSL_ERROR_SYSCALL:
		if(res < 0) { /* not EOF */
			if(errno == NOPOLL_EINTR) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "%s interrupted by a signal: retrying", label);
				/* report to retry */
				return -2;
			}
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "SSL_read (SSL_ERROR_SYSCALL)");
			return -1;
		}
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "SSL socket closed on %s (res=%d, ssl_err=%d, errno=%d)",
			    label, res, ssl_err, errno);
		nopoll_conn_log_ssl (conn);

		return res;
	case SSL_ERROR_ZERO_RETURN: /* close_notify received */
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "SSL closed on %s", label);
		return res;
	case SSL_ERROR_SSL:
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "%s function error (received SSL_ERROR_SSL) (res=%d, ssl_err=%d, errno=%d)",
			    label, res, ssl_err, errno);
		nopoll_conn_log_ssl (conn);
		return -1;
	default:
		/* nothing to handle */
		break;
	}
	nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "%s/SSL_get_error returned %d", label, res);
	return -1;
	
}

/** 
 * @internal Default connection receive until handshake is complete.
 */
int nopoll_conn_tls_receive (noPollConn * conn, char * buffer, int buffer_size)
{
	int res;
	nopoll_bool needs_retry;

	/* call to read content */
	res = SSL_read (conn->ssl, buffer, buffer_size);
	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "SSL: received %d bytes..", res); */

	/* call to handle error */
	res = __nopoll_conn_tls_handle_error (conn, res, "SSL_read", &needs_retry);
	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "  SSL: after procesing error %d bytes..", res); */
	if (res == -2) {
#if defined(NOPOLL_OS_UNIX)
		errno = NOPOLL_EWOULDBLOCK;
#elif defined(NOPOLL_OS_WIN32)
		WSASetLastError(NOPOLL_EWOULDBLOCK);
#endif
	}

	return res;
}

/** 
 * @internal Default connection send until handshake is complete.
 */
int nopoll_conn_tls_send (noPollConn * conn, char * buffer, int buffer_size)
{
	int         res;
	nopoll_bool needs_retry;

	/* call to read content */
	res = SSL_write (conn->ssl, buffer, buffer_size);
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "SSL: sent %d bytes (requested: %d)..", res, buffer_size); 

	/* call to handle error */
	res = __nopoll_conn_tls_handle_error (conn, res, "SSL_write", &needs_retry);
	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "   SSL: after processing error, sent %d bytes (requested: %d)..",  res, buffer_size); */
	if (res == -2) {
#if defined(NOPOLL_OS_UNIX)
		errno = NOPOLL_EWOULDBLOCK;
#elif defined(NOPOLL_OS_WIN32)
		WSASetLastError(NOPOLL_EWOULDBLOCK);
#endif
	}

	return res;
}


SSL_CTX * __nopoll_conn_get_ssl_context (noPollCtx * ctx, noPollConn * conn, noPollConnOpts * opts, nopoll_bool is_client)
{

	/* call to user defined function if the context creator is defined */
	if (ctx && ctx->context_creator) 
		return ctx->context_creator (ctx, conn, opts, is_client, ctx->context_creator_data);

	if (opts == NULL) {

		/* select a default mechanism according to what's
		 * available: ORDER IS IMPORTANT: make it select first
		 * strong methods over weak/old methods */
		
#if defined(NOPOLL_HAVE_TLS_FLEXIBLE_ENABLED)
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=flexible conn-id=%d", conn->id);
		/* flexible method */
		return SSL_CTX_new (is_client ? TLS_client_method () : TLS_server_method ());
#elif defined(NOPOLL_HAVE_TLSv12_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=tlsv1.2 conn-id=%d", conn->id);
		/* if not use TLSv1.2 */
		return SSL_CTX_new (is_client ? TLSv1_2_client_method () : TLSv1_2_server_method ());
#elif defined(NOPOLL_HAVE_TLSv11_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=tlsv1.1 conn-id=%d", conn->id);
		/* if not use TLSv1.1 */
		return SSL_CTX_new (is_client ? TLSv1_1_client_method () : TLSv1_1_server_method ());
#elif defined(NOPOLL_HAVE_TLSv10_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=tlsv1.0 conn-id=%d", conn->id);
		/* by default use TLSv1.0 */
		return SSL_CTX_new (is_client ? TLSv1_client_method () : TLSv1_server_method ());
#elif defined(NOPOLL_HAVE_SSLv3_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=sslv3 conn-id=%d", conn->id);
		/* if not use SSLv3 */
		return SSL_CTX_new (is_client ? SSLv3_client_method () : SSLv3_server_method ());
#elif defined(NOPOLL_HAVE_SSLv23_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=sslv23 conn-id=%d", conn->id);
		/* if not use SSLv23 */
		return SSL_CTX_new (is_client ? SSLv23_client_method () : SSLv23_server_method ()); 
#else
		/* no default method found */
		return NULL;
#endif
		
	} /* end if */

	switch (opts->ssl_protocol) {

#if defined(NOPOLL_HAVE_TLS_FLEXIBLE_ENABLED)
	case NOPOLL_METHOD_TLS_FLEXIBLE:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=flexible conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? TLS_client_method () : TLS_server_method ());
#endif		
		
#if defined(NOPOLL_HAVE_TLSv10_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	case NOPOLL_METHOD_TLSV1:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=tlsv1.0 conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? TLSv1_client_method () : TLSv1_server_method ());
#endif
		
#if defined(NOPOLL_HAVE_TLSv11_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	case NOPOLL_METHOD_TLSV1_1:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=tlsv1.1 conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? TLSv1_1_client_method () : TLSv1_1_server_method ()); 
#endif
		
#if defined(NOPOLL_HAVE_TLSv12_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	case NOPOLL_METHOD_TLSV1_2:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=tlsv1.2 conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? TLSv1_2_client_method () : TLSv1_2_server_method ()); 
#endif
		
#if defined(NOPOLL_HAVE_SSLv3_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	case NOPOLL_METHOD_SSLV3:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=sslv3 conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? SSLv3_client_method () : SSLv3_server_method ()); 
#endif
#if defined(NOPOLL_HAVE_SSLv23_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	case NOPOLL_METHOD_SSLV23:
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing tls-method=sslv23 conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? SSLv23_client_method () : SSLv23_server_method ());
#endif
	default:
	        /* default case */
#if defined(NOPOLL_HAVE_TLS_FLEXIBLE_ENABLED)
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "choosing default tls-method=flexible conn-id=%d", conn->id);
		return SSL_CTX_new (is_client ? TLS_client_method () : TLS_server_method ());
#else
		return NULL;
#endif		
	}

#if defined(NOPOLL_HAVE_TLSv10_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	/* reached this point, report default TLSv1 method */
	return SSL_CTX_new (is_client ? TLSv1_client_method () : TLSv1_server_method ());
#else
	return NULL;
#endif
}

noPollCtx * __nopoll_conn_ssl_ctx_debug = NULL;

int __nopoll_conn_ssl_verify_callback (int ok, X509_STORE_CTX * store) {
	char   data[256];
	X509 * cert;
#if defined(SHOW_DEBUG_LOG)
	int    depth;
	int    err;
#endif

	if (! ok) {
		cert  = X509_STORE_CTX_get_current_cert (store);
#if defined(SHOW_DEBUG_LOG)
		depth = X509_STORE_CTX_get_error_depth (store);
		err   = X509_STORE_CTX_get_error (store);
#endif

		nopoll_log (__nopoll_conn_ssl_ctx_debug, NOPOLL_LEVEL_CRITICAL, "CERTIFICATE: error=%d at depth: %d", err, depth);

		X509_NAME_oneline (X509_get_issuer_name (cert), data, 256);
		nopoll_log (__nopoll_conn_ssl_ctx_debug, NOPOLL_LEVEL_CRITICAL, "CERTIFICATE: issuer: %s", data);

		X509_NAME_oneline (X509_get_subject_name (cert), data, 256);
		nopoll_log (__nopoll_conn_ssl_ctx_debug, NOPOLL_LEVEL_CRITICAL, "CERTIFICATE: subject: %s", data);

		nopoll_log (__nopoll_conn_ssl_ctx_debug, NOPOLL_LEVEL_CRITICAL, "CERTIFICATE: error %d:%s", err, X509_verify_cert_error_string (err));
			    
	}
	return ok; /* return same value */
}

nopoll_bool __nopoll_conn_set_ssl_client_options (noPollCtx * ctx, noPollConn * conn, noPollConnOpts * options)
{
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Checking to establish SSL options (%p)", options);

	if (options && options->ca_certificate) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting CA certificate: %s", options->ca_certificate);
		if (SSL_CTX_load_verify_locations (conn->ssl_ctx, options->ca_certificate, NULL) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure CA certificate (%s), SSL_CTX_load_verify_locations () failed", options->ca_certificate);
			return nopoll_false;
		} /* end if */
		
	} /* end if */

	/* enable default verification paths */
	/* printf ("conn = %p, conn->ssl_ctx = %p\n", conn, conn->ssl_ctx); */
	if (SSL_CTX_set_default_verify_paths (conn->ssl_ctx) != 1) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to configure default verification paths, SSL_CTX_set_default_verify_paths () failed");
		return nopoll_false;
	} /* end if */

	if (options && options->chain_certificate) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting chain certificate: %s", options->chain_certificate);
		if (SSL_CTX_use_certificate_chain_file (conn->ssl_ctx, options->chain_certificate) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure chain certificate (%s), SSL_CTX_use_certificate_chain_file () failed", options->chain_certificate);
			return nopoll_false;
		} /* end if */
	} /* end if */

	if (options && options->certificate) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting certificate: %s", options->certificate);
		if (SSL_CTX_use_certificate_chain_file (conn->ssl_ctx, options->certificate) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure client certificate (%s), SSL_CTX_use_certificate_file () failed", options->certificate);
			return nopoll_false;
		} /* end if */
	} /* end if */

	if (options && options->private_key) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting private key: %s", options->private_key);
		if (SSL_CTX_use_PrivateKey_file (conn->ssl_ctx, options->private_key, SSL_FILETYPE_PEM) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure private key (%s), SSL_CTX_use_PrivateKey_file () failed", options->private_key);
			return nopoll_false;
		} /* end if */
	} /* end if */

	if (options && options->private_key && options->certificate) {
		if (!SSL_CTX_check_private_key (conn->ssl_ctx)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Certificate and private key do not matches, verification fails, SSL_CTX_check_private_key ()");
			return nopoll_false;
		} /* end if */
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Certificate (%s) and private key (%s) matches", options->certificate, options->private_key);
	} /* end if */

	/* if no option and it is not disabled */
	if (options == NULL || ! options->disable_ssl_verify) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Enabling certificate peer verification");
		/** really, really ugly hack to let
		 * __nopoll_conn_ssl_verify_callback to be able to get
		 * access to the context required to drop some logs */
		__nopoll_conn_ssl_ctx_debug = ctx;
		SSL_CTX_set_verify (conn->ssl_ctx, SSL_VERIFY_PEER, __nopoll_conn_ssl_verify_callback); 
		SSL_CTX_set_verify_depth (conn->ssl_ctx, 10); 
	} /* end if */

	return nopoll_true;
}

/** 
 * @internal Internal implementation used to do a connect.
 */
noPollConn * __nopoll_conn_new_common (noPollCtx       * ctx,
				       noPollConnOpts  * options,
				       noPollTransport   transport,
				       nopoll_bool       enable_tls,
				       int               socket,
				       const char      * host_ip, 
				       const char      * host_port, 
				       const char      * host_name,
				       const char      * get_url, 
				       const char      * protocols,
				       const char      * origin)
{
	noPollConn     * conn;
	NOPOLL_SOCKET    session;
	char           * content;
	int              size;
	int              ssl_error;
	X509           * server_cert;
	int              iterator;
	long             remaining_timeout;

	if (! ctx || ! host_ip) {
		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);
		return NULL;
	} /* end if */

	/* set default connection port */
	if (host_port == NULL)
		host_port = "80";

	session = socket;
	/* create socket connection in a non block manner */
	if (session == NOPOLL_INVALID_SOCKET)
		session = __nopoll_conn_sock_connect_opts_internal (ctx, transport, host_ip, host_port, options);
	if (session == NOPOLL_INVALID_SOCKET) {
		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to connect to remote host %s:%s", host_ip, host_port);
		return NULL;
	} /* end if */

	/* create the connection */
	conn = nopoll_new (noPollConn, 1);
	if (conn == NULL) {
		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);
		return NULL;
	} /* end if */

	conn->refs = 1;

	/* create mutexes */
	conn->ref_mutex = nopoll_mutex_create ();
	conn->handshake_mutex = nopoll_mutex_create ();

	/* register connection into context */
	if (! nopoll_ctx_register_conn (ctx, conn)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to register connection into the context, unable to create connection");
		nopoll_free (conn);
		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);

		return NULL;
	}

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Created noPoll conn-id=%d (ptr: %p, context: %p, socket: %d)",
		    conn->id, conn, ctx, session);
	
	/* configure context */
	conn->ctx     = ctx;
	conn->session = session;
	conn->role    = NOPOLL_ROLE_CLIENT;

	/* record host and port */
	conn->host    = nopoll_strdup (host_ip);
	conn->port    = nopoll_strdup (host_port);

	/* configure default handlers */
	conn->receive = nopoll_conn_default_receive;
	conn->send    = nopoll_conn_default_send;

	/* build host name */
	if (host_name == NULL)
		conn->host_name = nopoll_strdup (host_ip);
	else
		conn->host_name = nopoll_strdup (host_name);

	/* build origin */
	if (origin == NULL)
		conn->origin = nopoll_strdup_printf ("http://%s", conn->host_name);
	else
		conn->origin = nopoll_strdup (origin);

	/* get url */
	if (get_url == NULL)
		conn->get_url = nopoll_strdup ("/");
	else
		conn->get_url = nopoll_strdup (get_url);

	/* protocols */
	if (protocols != NULL)
		conn->protocols = nopoll_strdup (protocols);

	/* default to no close frame received */
	conn->peer_close_status = 1006;

	/* get client init payload */
	content = __nopoll_conn_get_client_init (conn, options);

	if (content == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to build client init message, unable to connect");
		nopoll_conn_shutdown (conn);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);

		return NULL;
	} /* end if */

	/* check for TLS support */
	if (enable_tls) {
		/* found TLS connection request, enable it */
		conn->ssl_ctx  = __nopoll_conn_get_ssl_context (ctx, conn, options, nopoll_true);
		if (conn->ssl_ctx == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to enable TLS, internal __nopoll_conn_get_ssl_context (ctx=%p, conn=%p, options=%p, nopoll_true) failed",
				    ctx, conn, options);
			goto fail_ssl_connection;
		} /* end if */

		/* check for client side SSL configuration */
		if (! __nopoll_conn_set_ssl_client_options (ctx, conn, options)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to configure additional SSL options, unable to continue, conn->ssl_ctx=%p, conn->ssl=%p",
				    conn->ssl_ctx, conn->ssl);
			goto fail_ssl_connection;
		} /* end if */

		/* create context and check for result */
		conn->ssl      = SSL_new (conn->ssl_ctx);       
		if (conn->ssl_ctx == NULL || conn->ssl == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to create SSL context internal references are null (conn->ssl_ctx=%p, conn->ssl=%p)",
				    conn->ssl_ctx, conn->ssl);
		fail_ssl_connection:

			nopoll_free (content);
			nopoll_conn_shutdown (conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return conn;
		} /* end if */
		
		/* set server name indication (SNI) */
		SSL_set_tlsext_host_name(conn->ssl, conn->host_name);

		/* set socket */
		SSL_set_fd (conn->ssl, conn->session);

		/* do the initial connect connect */
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "connecting to remote TLS site %s:%s", conn->host, conn->port);
		iterator = 0;
		while (SSL_connect (conn->ssl) <= 0) {
		
			/* get ssl error */
			ssl_error = SSL_get_error (conn->ssl, -1);
 
			switch (ssl_error) {
			case SSL_ERROR_WANT_READ:
			        nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "still not prepared to continue because read wanted, conn-id=%d (%p, session: %d), errno=%d",
					    conn->id, conn, conn->session, errno);
				break;
			case SSL_ERROR_WANT_WRITE:
			        nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "still not prepared to continue because write wanted, conn-id=%d (%p)",
					    conn->id, conn);
				break;
			case SSL_ERROR_SYSCALL:
				/* Check ENOTCONN on SSL_connect error (only happening on windows). See:
				 * https://github.com/ASPLes/nopoll/pull/19
				 */
				if (errno == NOPOLL_ENOTCONN) {
					nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "the socket is not yet connected, retrying, conn-id=%d (%p)",
					            conn->id, conn);
					break;
				}
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "syscall error while doing TLS handshake, ssl error (code:%d), conn-id: %d (%p), errno: %d, session: %d",
					    ssl_error, conn->id, conn, errno, conn->session);
				nopoll_conn_log_ssl (conn);
				nopoll_conn_shutdown (conn);
				nopoll_free (content);

				/* release connection options */
				__nopoll_conn_opts_release_if_needed (options);

				return conn;
			default:
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "there was an error with the TLS negotiation, ssl error (code:%d) : %s",
					    ssl_error, ERR_error_string (ssl_error, NULL));
				/* show log stack */
				nopoll_conn_log_ssl (conn);
					
				/* call to release connection */
				nopoll_conn_shutdown (conn);
				nopoll_free (content);

				/* release connection options */
				__nopoll_conn_opts_release_if_needed (options);

				return conn;
			} /* end switch */
			
			/* try and limit max reconnect allowed */
			iterator++;

			if (iterator > 1000) {
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Max retry calls=%d to SSL_connect reached, shutting down connection id=%d, errno=%d",
					    iterator, conn->id, errno);
				nopoll_free (content);

				/* release connection options */
				__nopoll_conn_opts_release_if_needed (options);

				return conn;
			} /* end if */

			/* wait a bit before retry */
			nopoll_sleep (10000);

		} /* end while */

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Client TLS handshake finished, configuring I/O handlers");

		/* check remote certificate (if it is present) */
		server_cert = SSL_get_peer_certificate (conn->ssl);
		if (server_cert == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "server side didn't set a certificate for this session, these are bad news");

			/* release connection options */
			nopoll_free (content);
			__nopoll_conn_opts_release_if_needed (options);

			return conn;
		}
		X509_free (server_cert);

		/* call to check post ssl checks after SSL finalization */
		if (conn->ctx && conn->ctx->post_ssl_check) {
			if (! conn->ctx->post_ssl_check (conn->ctx, conn, conn->ssl_ctx, conn->ssl, conn->ctx->post_ssl_check_data)) {
				/* TLS post check failed */
				nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "TLS/SSL post check function failed, dropping connection");
				nopoll_conn_shutdown (conn);
				return NULL;
			} /* end if */
		} /* end if */

		/* configure default handlers */
		conn->receive = nopoll_conn_tls_receive;
		conn->send    = nopoll_conn_tls_send;

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "TLS I/O handlers configured");
		conn->tls_on = nopoll_true;
	} /* end if */

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sending websocket client init: %s", content);
	size = strlen (content);

	/* call to send content */
	remaining_timeout = ctx->conn_connect_std_timeout;
	while (remaining_timeout > 0) {
		if (size != conn->send (conn, content, size)) {
		        /* for some reason, under FreeBSD, a ENOTCONN is reported when they should be returning EINPROGRESS and/or EWOULDBLOCK */
			if (errno == NOPOLL_EWOULDBLOCK || errno == NOPOLL_EINPROGRESS || errno == NOPOLL_ENOTCONN) {
				/* nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Connection in progress (errno=%d), session: %d", errno, session); */
				nopoll_sleep (10000);
				remaining_timeout -= 10000;
				continue;
			} /* end if */

			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to send websocket init message, error code was: %d (2), closing session", errno);
			nopoll_conn_shutdown (conn);
		} /* end if */

		break;
	}

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Web socket initial client handshake sent");

	/* release content */
	nopoll_free (content);

	/* release connection options */
	__nopoll_conn_opts_release_if_needed (options);

	/* return connection created */
	return conn;
}


/** 
 * @brief Creates a new Websocket connection to the provided
 * destination, physically located at host_ip and host_port (IPv4 version).
 *
 * @param ctx The noPoll context to which this new connection will be associated.
 *
 * @param host_ip The websocket server address to connect to.
 *
 * @param host_port The websocket server port to connect to. If NULL
 * is provided, port 80 is used.
 *
 * @param host_name This is the Host: header value that will be
 * sent. This header is used by the websocket server to activate the
 * right virtual host configuration. If null is provided, Host: will
 * use host_ip value.
 *
 * @param get_url As part of the websocket handshake, an url is passed
 * to the remote server inside a GET method. This parameter allows to
 * configure this. If NULL is provided, then / will be used.
 *
 * @param origin Websocket origin to be notified to the server.
 *
 * @param protocols Optional protocols requested to be activated for
 * this connection (an string of list of strings separated by a white
 * space). If the server accepts the connection you can use \ref
 * nopoll_conn_get_accepted_protocol to get the protocol accepted by
 * the server.
 *
 * @return A reference to the connection created or NULL if it
 * fails. Keep in mind the connection reported may not be connected at
 * the time is returned by this function. You can use \ref
 * nopoll_conn_is_ready and \ref nopoll_conn_is_ok to ensure it can be
 * used. There is also a helper function (NOTE it is blocking) that
 * can help you implement a very simple wait until ready operation:
 * \ref nopoll_conn_wait_until_connection_ready (however, it is not
 * recommended for any serious, non-command line programming).
 *
 * <b>Note 1: Is \ref nopoll_conn_new blocking?</b>
 *
 * Partially. It is blocking with a timeout but just for sending the
 * client init (the initial packet that a WebSocket client must send),
 * but the function does not block the caller until a reply from the
 * server is received and the handshake is completed (which is where
 * the blocking it is likely to happen).
 *
 * So, it essence, \ref nopoll_conn_new (and all its variants) is not
 * blocking (unless you have a problem with network connection causing
 * send() API to fail to send the WebSocket init message by returning
 * EWOULD_BLOCK, which is very likely to not happen),
 * 
 * Because of that, the connection (\ref noPollConn) reported by this function have big
 * chances to be not ready (that's why you have to use \ref nopoll_conn_is_ready
 * or the blocking one \ref nopoll_conn_wait_until_connection_ready to ensure
 * you successfully connected),
 *
 * <b>Note 2: Controll connect timeout</b>
 *
 * To control timeout for sending the initial message (and to ensure
 * the engine sends it), you can use the following functions:
 *
 *  - \ref nopoll_conn_connect_timeout
 *  - \ref nopoll_conn_get_connect_timeout
 */
noPollConn * nopoll_conn_new (noPollCtx  * ctx,
			      const char * host_ip, 
			      const char * host_port, 
			      const char * host_name,
			      const char * get_url, 
			      const char * protocols,
			      const char * origin)
{
	/* call common implementation */
	return __nopoll_conn_new_common (ctx, NULL, NOPOLL_TRANSPORT_IPV4, nopoll_false, 
					 NOPOLL_INVALID_SOCKET,
					 host_ip, host_port, host_name, 
					 get_url, protocols, origin);
}

/** 
 * @brief Creates a new Websocket connection to the provided
 * destination, physically located at host_ip and host_port (IPv6 version).
 *
 * See \ref nopoll_conn_new for more information about this
 * function. Both shares same core.
 *
 * @param ctx See \ref nopoll_conn_new
 *
 * @param host_ip See \ref nopoll_conn_new
 *
 * @param host_port See \ref nopoll_conn_new
 *
 * @param host_name See \ref nopoll_conn_new
 *
 * @param get_url See \ref nopoll_conn_new
 *
 * @param origin See \ref nopoll_conn_new
 *
 * @param protocols See \ref nopoll_conn_new
 *
 * @return See \ref nopoll_conn_new
 *
 * See \ref nopoll_conn_new for more information about this
 * function. Both shares same core.
 *
 */
noPollConn * nopoll_conn_new6 (noPollCtx  * ctx,
			       const char * host_ip, 
			       const char * host_port, 
			       const char * host_name,
			       const char * get_url, 
			       const char * protocols,
			       const char * origin)
{
	/* call common implementation */
	return __nopoll_conn_new_common (ctx, NULL, NOPOLL_TRANSPORT_IPV6, nopoll_false, 
					 NOPOLL_INVALID_SOCKET,
					 host_ip, host_port, host_name, 
					 get_url, protocols, origin);
}

/** 
 * @brief Creates a new Websocket connection to the provided
 * destination, physically located at host_ip and host_port and
 * allowing to provide a noPollConnOpts object.
 *
 * See \ref nopoll_conn_new for more information.
 *
 * @param ctx The noPoll context to which this new connection will be associated.
 *
 * @param opts The connection options to use during the connection and
 * the usage of this connection.
 *
 * @param host_ip The websocket server address to connect to.
 *
 * @param host_port The websocket server port to connect to. If NULL
 * is provided, port 80 is used.
 *
 * @param host_name This is the Host: header value that will be
 * sent. This header is used by the websocket server to activate the
 * right virtual host configuration. If null is provided, Host: will
 * use host_ip value.
 *
 * @param get_url As part of the websocket handshake, an url is passed
 * to the remote server inside a GET method. This parameter allows to
 * configure this. If NULL is provided, then / will be used.
 *
 * @param origin Websocket origin to be notified to the server.
 *
 * @param protocols Optional protocols requested to be activated for
 * this connection (an string of list of strings separated by a white
 * space). If the server accepts the connection you can use \ref
 * nopoll_conn_get_accepted_protocol to get the protocol accepted by
 * the server.
 *
 * @return A reference to the connection created or NULL if it
 * fails. Keep in mind the connection reported may not be connected at
 * the time is returned by this function. You can use \ref
 * nopoll_conn_is_ready and \ref nopoll_conn_is_ok to ensure it can be
 * used. There is also a helper function (NOTE it is blocking) that
 * can help you implement a very simple wait until ready operation:
 * \ref nopoll_conn_wait_until_connection_ready (however, it is not
 * recommended for any serious, non-command line programming).
 */
noPollConn * nopoll_conn_new_opts (noPollCtx       * ctx,
				   noPollConnOpts  * opts,
				   const char      * host_ip, 
				   const char      * host_port, 
				   const char      * host_name,
				   const char      * get_url, 
				   const char      * protocols,
				   const char      * origin)
{
	/* call common implementation */
	return __nopoll_conn_new_common (ctx, opts, NOPOLL_TRANSPORT_IPV4, nopoll_false, 
					 NOPOLL_INVALID_SOCKET,
					 host_ip, host_port, host_name, 
					 get_url, protocols, origin);
}

/**
 * @brief Creates a new Websocket connection using a socket
 * with a preestablished connection.
 *
 * @param ctx The noPoll context to which this new connection will be associated.
 *
 * @param options Optional configuration object. See \ref nopoll_conn_opts_new and \ref nopoll_conn_opts_set_ssl_protocol (for example).
 *
 * @param socket Socket FD with an already established connection.
 *
 * @param host_ip The websocket server address to connect to.
 *
 * @param host_port The websocket server port to connect to. If NULL
 * is provided, port 80 is used.
 *
 * @param host_name This is the Host: header value that will be
 * sent. This header is used by the websocket server to activate the
 * right virtual host configuration. If null is provided, Host: will
 * use host_ip value.
 *
 * @param get_url As part of the websocket handshake, an url is passed
 * to the remote server inside a GET method. This parameter allows to
 * configure this. If NULL is provided, then / will be used.
 *
 * @param origin Websocket origin to be notified to the server.
 *
 * @param protocols Optional protocols requested to be activated for
 * this connection (an string of list of strings separated by a white
 * space).
 *
 * <h3>Socket ownership</h3>
 *
 * noPoll takes full ownership. For noPoll, there's no difference
 * between a noPollConn created using regular API or offloaded and
 * then used with \ref nopoll_conn_new_with_socket.
 *
 * However, noPoll will not close anything unless you do it (see calls
 * to \ref nopoll_close_socket throughout the code).
 *
 * For noPoll, ownership here means no one is reading/writing to that
 * socket but noPoll.
 *
 * <h3>Can socket passed be used with my favourite I/O loop mech?</h3>
 *
 * Yes.
 *
 * <h3>Will reading or writing from/to the socket make noPoll unsafe?</h3>
 *
 * Yes. Only noPoll must read and write to that socket using provided
 * public API. Writing/reading directly will break not only noPoll
 * sync but also remote's peer.
 * 
 */
noPollConn * nopoll_conn_new_with_socket (noPollCtx  * ctx,
				   noPollConnOpts  * options,
				   int             socket,
				   const char      * host_ip,
				   const char      * host_port,
				   const char      * host_name,
				   const char      * get_url,
				   const char      * protocols,
				   const char      * origin)
{
	/* call common implementation */
	return __nopoll_conn_new_common (ctx, options, NOPOLL_TRANSPORT_IPV4, nopoll_false,
					 socket, host_ip, host_port, host_name,
					 get_url, protocols, origin);
}

nopoll_bool __nopoll_tls_was_init = nopoll_false;

/** 
 * @brief Allows to create a client WebSocket connection over TLS (IPv4 version).
 *
 * The function works like \ref nopoll_conn_new with the same
 * semantics but providing a way to create a WebSocket session under
 * TLS supervision. See \ref nopoll_conn_new for more information.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param options Optional configuration object. See \ref nopoll_conn_opts_new and \ref nopoll_conn_opts_set_ssl_protocol (for example).
 *
 * @param host_ip The websocket server address to connect to.
 *
 * @param host_port The websocket server port to connect to. If NULL
 * is provided, port 443 is used.
 *
 * @param host_name This is the Host: header value that will be
 * sent. This header is used by the websocket server to activate the
 * right virtual host configuration. If null is provided, Host: will
 * use host_ip value.
 *
 * @param get_url As part of the websocket handshake, an url is passed
 * to the remote server inside a GET method. This parameter allows to
 * configure this. If NULL is provided, then / will be used.
 *
 * @param origin Websocket origin to be notified to the server.
 *
 * @param protocols Optional protocols requested to be activated for
 * this connection (an string of list of strings separated by a white
 * space). If the server accepts the connection you can use \ref
 * nopoll_conn_get_accepted_protocol to get the protocol accepted by
 * the server.
 *
 * @return A reference to the connection created or NULL if it
 * fails. Keep in mind the connection reported may not be connected at
 * the time is returned by this function. You can use \ref
 * nopoll_conn_is_ready and \ref nopoll_conn_is_ok to ensure it can be
 * used. There is also a helper function (NOTE it is blocking) that
 * can help you implement a very simple wait until ready operation:
 * \ref nopoll_conn_wait_until_connection_ready (however, it is not
 * recommended for any serious, non-command line programming).
 * 
 */
noPollConn * nopoll_conn_tls_new (noPollCtx  * ctx,
				  noPollConnOpts  * options,
				  const char * host_ip, 
				  const char * host_port, 
				  const char * host_name,
				  const char * get_url, 
				  const char * protocols,
				  const char * origin)
{
	/* init ssl ciphers and engines */
	if (! __nopoll_tls_was_init) {
		__nopoll_tls_was_init = nopoll_true;
		SSL_library_init ();
	} /* end if */

	/* call common implementation */
	return __nopoll_conn_new_common (ctx, options, NOPOLL_TRANSPORT_IPV4, nopoll_true, 
					 NOPOLL_INVALID_SOCKET,
					 host_ip, host_port, host_name, 
					 get_url, protocols, origin);
}

/** 
 * @brief Allows to create a client WebSocket connection over TLS (IPv6 version).
 *
 * See \ref nopoll_conn_tls_new for more information.
 *
 * @param ctx See \ref nopoll_conn_tls_new for more information.
 *
 * @param options See \ref nopoll_conn_tls_new for more information.
 *
 * @param host_ip See \ref nopoll_conn_tls_new for more information.
 *
 * @param host_port See \ref nopoll_conn_tls_new for more information.
 *
 * @param host_name See \ref nopoll_conn_tls_new for more information.
 *
 * @param get_url See \ref nopoll_conn_tls_new for more information.
 *
 * @param origin See \ref nopoll_conn_tls_new for more information.
 *
 * @param protocols See \ref nopoll_conn_tls_new for more information.
 *
 * @return See \ref nopoll_conn_tls_new for more information.
 * 
 */
noPollConn * nopoll_conn_tls_new6 (noPollCtx  * ctx,
				   noPollConnOpts  * options,
				   const char * host_ip, 
				   const char * host_port, 
				   const char * host_name,
				   const char * get_url, 
				   const char * protocols,
				   const char * origin)
{
	/* init ssl ciphers and engines */
	if (! __nopoll_tls_was_init) {
		__nopoll_tls_was_init = nopoll_true;
		SSL_library_init ();
	} /* end if */

	/* call common implementation */
	return __nopoll_conn_new_common (ctx, options, NOPOLL_TRANSPORT_IPV6, nopoll_true, 
					 NOPOLL_INVALID_SOCKET,
					 host_ip, host_port, host_name, 
					 get_url, protocols, origin);
}

/**
 * @brief Allows to create a client WebSocket connection over TLS using a preestablished socket.
 *
 * The function works like \ref nopoll_conn_tls_new with the same
 * semantics but providing a way to create a WebSocket session under
 * TLS supervision with a preestablished socket. See \ref nopoll_conn_tls_new
 * and \ref nopoll_conn_new_with_socket for more information.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param options Optional configuration object. See \ref nopoll_conn_opts_new and \ref nopoll_conn_opts_set_ssl_protocol (for example).
 *
 * @param socket Socket FD with an already established connection.
 *
 * @param host_ip The websocket server address to connect to.
 *
 * @param host_port The websocket server port to connect to. If NULL
 * is provided, port 443 is used.
 *
 * @param host_name This is the Host: header value that will be
 * sent. This header is used by the websocket server to activate the
 * right virtual host configuration. If null is provided, Host: will
 * use host_ip value.
 *
 * @param get_url As part of the websocket handshake, an url is passed
 * to the remote server inside a GET method. This parameter allows to
 * configure this. If NULL is provided, then / will be used.
 *
 * @param origin Websocket origin to be notified to the server.
 *
 * @param protocols Optional protocols requested to be activated for
 * this connection (an string of list of strings separated by a white
 * space). If the server accepts the connection you can use \ref
 * nopoll_conn_get_accepted_protocol to get the protocol accepted by
 * the server.
 *
 * @return A reference to the connection created or NULL if it
 * fails. Keep in mind the connection reported may not be connected at
 * the time is returned by this function. You can use \ref
 * nopoll_conn_is_ready and \ref nopoll_conn_is_ok to ensure it can be
 * used. There is also a helper function (NOTE it is blocking) that
 * can help you implement a very simple wait until ready operation:
 * \ref nopoll_conn_wait_until_connection_ready (however, it is not
 * recommended for any serious, non-command line programming).
 *
 */
noPollConn * nopoll_conn_tls_new_with_socket (noPollCtx  * ctx,
				  noPollConnOpts  * options,
				  int          socket,
				  const char * host_ip,
				  const char * host_port,
				  const char * host_name,
				  const char * get_url,
				  const char * protocols,
				  const char * origin)
{
	/* init ssl ciphers and engines */
	if (! __nopoll_tls_was_init) {
		__nopoll_tls_was_init = nopoll_true;
		SSL_library_init ();
	} /* end if */

	/* call common implementation */
	return __nopoll_conn_new_common (ctx, options, NOPOLL_TRANSPORT_IPV4, nopoll_true,
					 socket, host_ip, host_port, host_name,
					 get_url, protocols, origin);
}


/** 
 * @brief Allows to acquire a reference to the provided connection.
 *
 * @param conn The conection to be referenced
 *
 * @return nopoll_true In the case the function acquired a reference
 * otherwise nopoll_false is returned.
 */
nopoll_bool    nopoll_conn_ref (noPollConn * conn)
{
	if (conn == NULL)
		return nopoll_false;

	/* lock the mutex */
	nopoll_mutex_lock (conn->ref_mutex);
	conn->refs++;
	nopoll_mutex_unlock (conn->ref_mutex);
		
	/* report */
	return conn->refs > 1;
}

/** 
 * @brief Allows to get current reference counting state for the
 * provided connection.
 *
 * @param conn The connection queried for its reference counting.
 *
 * @return The reference counting or -1 if it fails.
 */
int            nopoll_conn_ref_count (noPollConn * conn)
{
	if (! conn)
		return -1;
	return conn->refs;
}

/** 
 * @brief Allows to check if the provided connection is in connected
 * state (just to the connection). This is different to be ready to send and receive content
 * because the session needs to be first established. 
 *
 * You can use \ref nopoll_conn_is_ready to ensure the connection is
 * ready to be used (read or write operation can be done because
 * handshake has finished).
 *
 * For example, you might connect to a raw socket server and
 * nopoll_conn_is_ok will report that everything is ok because the
 * socket is indeed connected but because you are connecting to a
 * non-websocket server, it will not work because the WebSocket
 * session establishment didn't take place and hence
 * nopoll_conn_is_ready will fail.
 *
 * Please, see the following link for a complete example that connects
 * and ensure the connection is ready (for a client):  http://www.aspl.es/nopoll/html/nopoll_core_library_manual.html#creating_basic_web_socket_client
 *
 * @param conn The websocket connection to be checked.
 *
 * @return nopoll_true in the case the connection is working otherwise
 * nopoll_false is returned.
 */
nopoll_bool    nopoll_conn_is_ok (noPollConn * conn)
{
	if (conn == NULL)
		return nopoll_false;

	/* return current socket status */
	return conn->session != NOPOLL_INVALID_SOCKET;
}

/** 
 * @brief Allows to check if the connection is ready to be used
 * (handshake completed).
 *
 * Note this function may block the caller in the case the hanshake is
 * not completed.
 *
 * @param conn The connection to be checked.
 *
 * @return nopoll_true in the case the handshake was completed
 * otherwise nopoll_false is returned. The function also returns
 * nopoll_false in case \ref nopoll_conn_is_ok is failing.
 */
nopoll_bool    nopoll_conn_is_ready (noPollConn * conn)
{
	if (conn == NULL)
		return nopoll_false;
	if (conn->session == NOPOLL_INVALID_SOCKET)
		return nopoll_false;
	if (! conn->handshake_ok) {
		/* acquire here handshake mutex */
		nopoll_mutex_lock (conn->handshake_mutex);

		/* complete handshake */
		nopoll_conn_complete_handshake (conn);

		/* release here handshake mutex */
		nopoll_mutex_unlock (conn->handshake_mutex);
	}
	return conn->handshake_ok;
}

/** 
 * @brief Allows to check if the provided connection is working under a TLS session.
 *
 * @param conn The connection where the TLS is being queired to be enabled.
 *
 * @return nopoll_true in the case TLS is enabled, otherwise* nopoll_false is returned. Note the function also returns* nopoll_false when the reference received is NULL.
 */
nopoll_bool    nopoll_conn_is_tls_on (noPollConn * conn)
{
	if (! conn)
		return nopoll_false;

	/* return TLS on */
	return conn->tls_on;
}

/** 
 * @brief Allows to get the socket associated to this nopoll
 * connection.
 *
 * @return The socket reference or -1 if it fails.
 */
NOPOLL_SOCKET nopoll_conn_socket (noPollConn * conn)
{
	if (conn == NULL)
		return -1;
	return conn->session;
}

/** 
 * @brief Allows to set up the socket reference to be used by this
 * noPollConn.
 *
 * @param conn The connection to setup with a new socket.
 *
 * @param _socket The socket that will be configured.
 */
void           nopoll_conn_set_socket (noPollConn * conn, NOPOLL_SOCKET _socket)
{
	if (conn == NULL)
		return;
	conn->session = _socket;
	return;
}

/** 
 * @brief Allows to get the connection id from the provided
 * connection.
 *
 * @param conn The connection from where the unique identifier will be
 * returned.
 *
 * @return The identifier or -1 if it fails.
 */
int           nopoll_conn_get_id (noPollConn * conn)
{
	if (conn == NULL)
		return -1;
	return conn->id;
}

/**
 * @brief Allows to get the get_url from the connection
 *
 * @param conn The connection from where the url will be obtained
 *
 * @return The get_url or NULL
 */
const char * nopoll_conn_get_requested_url (noPollConn * conn)
{
        if (!conn)
            return NULL;
        return (conn->get_url == NULL ? "/" : conn->get_url);
}


/** 
 * @brief Allows to get the noPollCtx context object associated to the
 * connection (or where the connection is working).
 *
 * @param conn The connection that is requested to return its context.
 *
 * @return A reference to the context or NULL if it fails.
 */
noPollCtx   * nopoll_conn_ctx    (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->ctx;
}

/** 
 * @brief Allows to get the connection role.
 *
 * @return The connection role, see \ref noPollRole for details.
 */
noPollRole    nopoll_conn_role   (noPollConn * conn)
{
	if (conn == NULL)
		return NOPOLL_ROLE_UNKNOWN;
	return conn->role;
}

/** 
 * @brief Returns the host location this connection connects to or it is
 * listening (according to the connection role \ref noPollRole).
 *
 * If you are looking for a way to get the Host: header value received
 * for this connection during the handshake, use: \ref
 * nopoll_conn_get_host_header.
 *
 * @param conn The connection to check for the host value.
 *
 * @return The host location value or NULL if it fails.
 */
const char  * nopoll_conn_host   (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->host;
}

/** 
 * @brief Allows to get the connection Origin header content received.
 *
 * @param conn The websocket connection where the operation takes place.
 *
 * @return The Origin value received during the handshake for this
 * connection or NULL if it fails to get this value.
 */
const char  * nopoll_conn_get_origin (noPollConn * conn)
{
	if (conn == NULL || conn->handshake == NULL)
		return NULL;
	return conn->origin;
} /* end if */

/** 
 * @brief Allows to get the Host: header value that was received for
 * this connection during the handshake.
 *
 * @param conn The websocket connection where the operation takes place.
 *
 * @return The Host value received during the handshake for this
 * connection or NULL if it fails to get this value (or it wasn't
 * defined).
 */
const char  * nopoll_conn_get_host_header (noPollConn * conn)
{
	if (conn == NULL || conn->host_name == NULL)
		return NULL;
	return conn->host_name;
}

/** 
 * @brief Allows to get cookie header content received during
 * handshake (if received).
 *
 * @param conn The websocket connection where the operation takes place.
 *
 * @return A reference to the cookie value or NULL if nothing is
 * configured or if failed to get it.
 */
const char  * nopoll_conn_get_cookie (noPollConn * conn)
{
	
        if (conn == NULL || conn->handshake == NULL)
                return NULL;
        return conn->handshake->cookie;
}

/** 
 * @brief Allows to get accepted protocol in the case a protocol was
 * requested during connection.
 *
 * @param conn The connection where the operation is taking place.
 *
 * @return A reference to the protocol accepted or NULL if no protocol
 * was mentioned during the handshake.
 */
const char  * nopoll_conn_get_accepted_protocol (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->accepted_protocol; /* report accepted protocol */
}

/** 
 * @brief Allows to get requested protocols for the provided
 * connection
 *
 * @param conn The connection where the operation is taking place.
 *
 * @return A reference to the protocol or list of protocols requested
 * by this connection.
 */
const char  * nopoll_conn_get_requested_protocol (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->protocols; /* report requested protocol */
}

/** 
 * @brief Allows to configure accepted protocol on the provided
 * connection.
 *
 * @param conn The connection where the accepted protocol will be
 * notified. This function is only useful at server side to notify
 * accepted protocol to the connecting client. Caller is entirely
 * responsible on setting an appropriate value that is understood by
 * the client. No especial check is done on the value provided to this function.
 *
 * @param protocol The protocol to configure in the reply, that is,
 * the protocol accepted by the server. 
 *
 * If no protocol is configured and the client requests a protocol,
 * the server will reply by default the same accepting it.
 */
void          nopoll_conn_set_accepted_protocol (noPollConn * conn, const char * protocol)
{
	if (conn == NULL || protocol == NULL)
		return;

	/* set accepted protocol */
	conn->accepted_protocol = nopoll_strdup (protocol);
	return;
}

/** 
 * @brief Returns the port location this connection connects to or it
 * is listening (according to the connection role \ref noPollRole).
 *
 * @param conn The connection to check for the port value.
 *
 * @return The port location value or NULL if it fails.
 */
const char  * nopoll_conn_port   (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->port;
}

/** 
 * @brief Optionally, remote peer can close the connection providing an
 * error code (\ref nopoll_conn_get_close_status) and a reason (\ref nopoll_conn_get_close_reason).
 *
 * These functions are used to get those values.
 *
 * @param conn The connection where the peer reported close status is being asked.
 *
 * @return Status code reported by the remote peer or:
 *  0    - conn is NULL
 *  1005 - empty closing frame
 *  1006 - no closing frame
 */ 
int           nopoll_conn_get_close_status (noPollConn * conn)
{
	if (conn == NULL)
		return 0;
	return conn->peer_close_status;
}

/** 
 * @brief Optionally, remote peer can close the connection providing an
 * error code (\ref nopoll_conn_get_close_status) and a reason (\ref nopoll_conn_get_close_reason).
 *
 * These functions are used to get those values.
 *
 * @param conn The connection where the peer reported close reason is being asked.
 *
 * @return Status close reason reported by the remote peer or NULL if nothing was reported.
 */ 
const char *  nopoll_conn_get_close_reason (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->peer_close_reason;
}

/** 
 * @brief Call to close the connection immediately without going
 * through websocket close negotiation.
 *
 * @param conn The connection to be shutted down.
 */
void          nopoll_conn_shutdown (noPollConn * conn)
{
#if defined(SHOW_DEBUG_LOG)
	const char * role = NULL;
#endif

	if (conn == NULL)
		return;

	/* report connection close */
#if defined(SHOW_DEBUG_LOG)
	if (conn->role == NOPOLL_ROLE_LISTENER)
		role = "listener";
	else if (conn->role == NOPOLL_ROLE_MAIN_LISTENER)
		role = "master-listener";
	else if (conn->role == NOPOLL_ROLE_UNKNOWN)
		role = "unknown";
	else if (conn->role == NOPOLL_ROLE_CLIENT)
		role = "client";
	else
		role = "unknown";

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "shutting down connection id=%d (session: %d, role: %s)", 
		    conn->id, conn->session, role);
#endif

	/* call to on close handler if defined */
	if (conn->session != NOPOLL_INVALID_SOCKET && conn->on_close)
	        conn->on_close (conn->ctx, conn, conn->on_close_data);

	/* shutdown connection here */
	if (conn->session != NOPOLL_INVALID_SOCKET) {
	        shutdown (conn->session, SHUT_RDWR);
		nopoll_close_socket (conn->session);
	}
	conn->session = NOPOLL_INVALID_SOCKET;

	return;
}

/** 
 * @brief Allows to close an opened \ref noPollConn no matter its role
 * (\ref noPollRole).
 *
 * @param conn The connection to close.
 *
 * @param status Optional status code to send to remote side. If
 * status is < 0, no status code is sent.
 *
 * @param reason Pointer to the content to be sent.
 *
 * @param reason_size The amount of bytes that should be used from content pointer.
 */ 
void          nopoll_conn_close_ext  (noPollConn  * conn, int status, const char * reason, int reason_size)
{
	int    refs;
	char * content;
#if defined(SHOW_DEBUG_LOG)
	const char * role = "unknown";
#endif

	/* check input data */
	if (conn == NULL)
		return;

#if defined(SHOW_DEBUG_LOG)
	if (conn->role == NOPOLL_ROLE_LISTENER)
		role = "listener";
	else if (conn->role == NOPOLL_ROLE_MAIN_LISTENER)
		role = "master-listener";
	else if (conn->role == NOPOLL_ROLE_UNKNOWN)
		role = "unknown";
	else if (conn->role == NOPOLL_ROLE_CLIENT)
		role = "client";
	
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Calling to close close id=%d (session %d, refs: %d, role: %s)", 
		    conn->id, conn->session, conn->refs, role);
#endif
	if (conn->session != NOPOLL_INVALID_SOCKET) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "requested proper connection close id=%d (session %d)", conn->id, conn->session);

		/* build reason indication */
		content = NULL;
		if (reason && reason_size > 0) {
			/* send content */
			content = nopoll_new (char, reason_size + 3);
			if (content) {
				nopoll_set_16bit (status, content);
				memcpy (content + 2, reason, reason_size);
			} /* end if */
		} /* end if */

		/* send close without reason */
		nopoll_conn_send_frame (conn, nopoll_true /* has_fin */, 
					/* masked */
					conn->role == NOPOLL_ROLE_CLIENT, NOPOLL_CLOSE_FRAME, 
					/* content size and content */
					reason_size > 0 ? reason_size + 2 : 0, content, 
					/* sleep in header */
					0);

		/* release content (if defined) */
		nopoll_free (content);

		/* call to shutdown connection */
		nopoll_conn_shutdown (conn);
	} /* end if */

	/* unregister connection from context */
	refs = nopoll_conn_ref_count (conn);
	nopoll_ctx_unregister_conn (conn->ctx, conn);

	/* avoid calling next unref in the case not enough references
	 * are found */
	if (refs <= 1)
		return;

	/* call to unref connection */
	nopoll_conn_unref (conn);

	return;	
}

/** 
 * @brief Allows to close an opened \ref noPollConn no matter its role
 * (\ref noPollRole).
 *
 * @param conn The connection to close.
 *
 * There is available an alternative extended version that allows to
 * send the status code and the error message: \ref
 * nopoll_conn_close_ext
 */ 
void          nopoll_conn_close  (noPollConn  * conn)
{
	/* call to close without providing a reason */
	nopoll_conn_close_ext (conn, 0, NULL, 0);
	return;
}

/** 
 * @brief Allows to define a user level pointer associated to this
 * connection. This pointer can be later be retrieved using \ref
 * nopoll_conn_get_hook.
 *
 * @param conn The connection where the pointer will be associated.
 *
 * @param ptr The pointer to associate to the connection.
 */
void          nopoll_conn_set_hook (noPollConn * conn, noPollPtr ptr)
{
	if (conn == NULL)
		return;
	conn->hook = ptr;
	return;
}


/** 
 * @brief Allows to get the user level pointer defined by \ref
 * nopoll_conn_get_hook.
 *
 * @param conn The connection where the uesr level pointer was stored.
 *
 * @return A reference to the pointer stored.
 */
noPollPtr     nopoll_conn_get_hook (noPollConn * conn)
{
	if (conn == NULL)
		return NULL;
	return conn->hook;
}

/** 
 * @brief Allows to unref connection reference acquired via \ref
 * nopoll_conn_ref.
 *
 * @param conn The connection to be unrefered.
 */
void nopoll_conn_unref (noPollConn * conn)
{
	int value;
	
	if (conn == NULL)
		return;

	/* acquire here the mutex */
	nopoll_mutex_lock (conn->ref_mutex);

	conn->refs--;
	value = conn->refs;
	
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Releasing connection id %d reference, current ref count status is: %d", 
		    conn->id, value);
	/* release here the mutex */
	nopoll_mutex_unlock (conn->ref_mutex);
	
	if (value != 0) 
		return;

	/* release message */
	if (conn->pending_msg)
		nopoll_msg_unref (conn->pending_msg);

	/* release ctx */
	if (conn->ctx) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Released context refs, now: %d", conn->ctx->refs);
		nopoll_ctx_unref (conn->ctx);
	} /* end if */
	conn->ctx = NULL;

	/* free all internal strings */
	nopoll_free (conn->host);
	nopoll_free (conn->port);
	nopoll_free (conn->host_name);
	nopoll_free (conn->origin);
	nopoll_free (conn->protocols);
	nopoll_free (conn->accepted_protocol);
	nopoll_free (conn->get_url);

	/* close reason if any */
	nopoll_free (conn->peer_close_reason);

	/* release TLS certificates */
	nopoll_free (conn->certificate);
	nopoll_free (conn->private_key);
	nopoll_free (conn->chain_certificate);

	/* release uncomplete message */
	if (conn->previous_msg) 
		nopoll_msg_unref (conn->previous_msg);

	if (conn->ssl)
		SSL_free (conn->ssl);
	if (conn->ssl_ctx)
		SSL_CTX_free (conn->ssl_ctx);

	/* release handshake internal data */
	if (conn->handshake) {
		nopoll_free (conn->handshake->websocket_key);
		nopoll_free (conn->handshake->websocket_version);
		nopoll_free (conn->handshake->websocket_accept);
		nopoll_free (conn->handshake->expected_accept);
		nopoll_free (conn->handshake->cookie);
		nopoll_free (conn->handshake);
	} /* end if */

	/* release connection options if defined and reuse flag is not defined */
	if (conn->opts && ! conn->opts->reuse)
		nopoll_conn_opts_free (conn->opts);

	/* release pending write buffer */
	nopoll_free (conn->pending_write);

	/* release mutexes */
	nopoll_mutex_destroy (conn->handshake_mutex);
	nopoll_mutex_destroy (conn->ref_mutex);

	nopoll_free (conn);	

	return;
}

/** 
 * @internal Default connection receive until handshake is complete.
 */
int nopoll_conn_default_receive (noPollConn * conn, char * buffer, int buffer_size)
{
	return recv (conn->session, buffer, buffer_size, 0);
}

/** 
 * @internal Default connection send until handshake is complete.
 */
int nopoll_conn_default_send (noPollConn * conn, char * buffer, int buffer_size)
{
	return send (conn->session, buffer, buffer_size, 0);
}

/** 
 * @internal Read the next line, byte by byte until it gets a \n or
 * maxlen is reached. Some code errors are used to manage exceptions
 * (see return values)
 * 
 * @param connection The connection where the read operation will be done.
 *
 * @param buffer A buffer to store content read from the network.
 *
 * @param maxlen max content to read from the network.
 * 
 * @return  values returned by this function follows:
 *  0 - remote peer have closed the connection
 * -1 - an error have happened while reading
 * -2 - could read because this connection is on non-blocking mode and there is no data.
 *  n - some data was read.
 * 
 **/
int          nopoll_conn_readline (noPollConn * conn, char  * buffer, int  maxlen)
{
	int         n, rc;
	int         desp;
	char        c, *ptr;
#if defined(SHOW_DEBUG_LOG)
# if !defined(SHOW_FORMAT_BUGS)
	noPollCtx * ctx = conn->ctx;
# endif
#endif

	/* clear the buffer received */
	/* memset (buffer, 0, maxlen * sizeof (char ));  */

	/* check for pending line read */
	desp         = 0;
	if (conn->pending_line) {
		/* get size and check exceeded values */
		desp = strlen (conn->pending_line);
		if (desp >= maxlen) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, 
				    "found fragmented frame line header but allowed size was exceeded (desp:%d >= maxlen:%d)",
				    desp, maxlen);
			nopoll_conn_shutdown (conn);
			return -1;
		} /* end if */

		/* now store content into the buffer */
		memcpy (buffer, conn->pending_line, desp);

		/* clear from the conn the line */
		nopoll_free (conn->pending_line);
		conn->pending_line = NULL;
	}

	/* read current next line */
	ptr = (buffer + desp);
	for (n = 1; n < (maxlen - desp); n++) {
	nopoll_readline_again:
		if (( rc = conn->receive (conn, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\x0A')
				break;
		}else if (rc == 0) {
			if (n == 1)
				return 0;
			else
				break;
		} else {
			if (errno == NOPOLL_EINTR) 
				goto nopoll_readline_again;
			if ((errno == NOPOLL_EWOULDBLOCK) || (errno == NOPOLL_EAGAIN) || (rc == -2)) {
				if (n > 0) {
					/* store content read until now */
					if ((n + desp - 1) > 0) {
						buffer[n+desp - 1] = 0;
						conn->pending_line = nopoll_strdup (buffer);
					} /* end if */
				} /* end if */
				return -2;
			}

			if (nopoll_conn_is_ok (conn) && errno == 0 && rc == 0) {
				nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "unable to read line, but errno is 0, and connection is ok, return to keep on trying..");
				return -2;
			}
			
			/* if the conn is closed, just return
			 * without logging a message */
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to read line, error code errno: %d, rc: %d (%s)", 
				    errno, rc, strerror (errno));
			return (-1);
		}
	}
	*ptr = 0;
	return (n + desp);

}

/** 
 * @brief Allows to get the master listener that was used to accept
 * the provided connection that represents a listener connection.
 *
 * @param conn A listener connection that was accepted for which we
 * want to get the master listener connection. The connection must be
 * a \ref NOPOLL_ROLE_LISTENER (as reported by \ref nopoll_conn_role).
 *
 * @return A reference to the master listener connection or NULL if it
 * fails. It can only fail when NULL reference is received or the
 * connection is not a \ref NOPOLL_ROLE_LISTENER.
 */
noPollConn  * nopoll_conn_get_listener (noPollConn * conn)
{
	/* check if the incoming connection is a NOPOLL_ROLE_LISTENER
	 * that is an accepted connection */
	if (conn == NULL || conn->role != NOPOLL_ROLE_LISTENER)
		return NULL;

	return conn->listener;
}

void __nopoll_pack_content (char * buffer, int start, int bytes)
{
	int iterator = 0;
	while (iterator < bytes) {
		/* copy bytes to the begining of the array */
		buffer[iterator] = buffer[start + iterator];
		
		/* next position */
		iterator++;
	} /* end while */

	return;
}

/** 
 * @internal Function used to read bytes from the wire. 
 *
 * @return The function returns the number of bytes read, 0 when no
 * bytes were available and -1 when it fails.
 */
int         __nopoll_conn_receive  (noPollConn * conn, char  * buffer, int  maxlen)
{
	int         nread;
	int         bytes;

	if (conn->pending_buf_bytes > 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Calling with bytes we can reuse (%d), requested: %d",
			    conn->pending_buf_bytes, maxlen);

		if (conn->pending_buf_bytes >= maxlen) {
			/* we have more in the buffer to serve than
			 * requested, ok, copy into the buffer and
			 * pack */

			memcpy (buffer, conn->pending_buf, maxlen);
			__nopoll_pack_content (conn->pending_buf, maxlen, conn->pending_buf_bytes - maxlen);
			conn->pending_buf_bytes -= maxlen;
			return maxlen;
		} 

		/* ok, we don't have enough bytes to serve
		 * directly, so copy what we have */
		memcpy (buffer, conn->pending_buf, conn->pending_buf_bytes);

		/* copy number of bytes served to reduce next request */
		nread = conn->pending_buf_bytes;
		conn->pending_buf_bytes = 0;

		/* call again to get bytes reducing the request in the
		 * amount of bytes served */
		bytes = __nopoll_conn_receive (conn, buffer + nread, maxlen - nread);
		if (bytes < 0) {
			return -1;
		}

		return bytes + nread;
	} /* end if */

 keep_reading:
	/* clear buffer */
	/* memset (buffer, 0, maxlen * sizeof (char )); */
#if defined(NOPOLL_OS_UNIX)
	errno = 0;
#elif defined(NOPOLL_OS_WIN32)
	WSASetLastError(0);
#endif
	if ((nread = conn->receive (conn, buffer, maxlen)) < 0) {
		/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, " returning errno=%d (%s)", errno, strerror (errno)); */
		if (errno == NOPOLL_EAGAIN) 
			return 0;
		if (errno == NOPOLL_EWOULDBLOCK) 
			return 0;
		if (errno == NOPOLL_EINTR) 
			goto keep_reading;
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "unable to readn=%d, error code was: %d (%s) (shutting down connection)", maxlen, errno, strerror (errno));
		nopoll_conn_shutdown (conn);
		return -1;
	}

	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, " returning bytes read = %d", nread); */
	if (nread == 0) {
		/* check for blocking operations */
		if (errno == NOPOLL_EAGAIN || errno == NOPOLL_EWOULDBLOCK) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "unable to read from conn-id=%d (%s:%s), connection is not ready (errno: %d : %s)",
				    conn->id, conn->host, conn->port, errno, strerror (errno));
			return 0;
		} /* end if */

		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "received connection close while reading from conn id %d (errno=%d : %s) (%d, %d, %d), shutting down connection..", 
			    conn->id, errno, strerror (errno),
			    NOPOLL_EAGAIN, NOPOLL_EWOULDBLOCK, NOPOLL_EINTR);
		nopoll_conn_shutdown (conn);
	} /* end if */

	return nread;
}

nopoll_bool nopoll_conn_get_http_url (noPollConn * conn, const char * buffer, int buffer_size, const char * method, char ** url)
{
	int          iterator;
	int          iterator2;
#if defined(SHOW_DEBUG_LOG)
# if ! defined(SHOW_FORMAT_BUGS)
	noPollCtx  * ctx = conn->ctx;
# endif
#endif

	/* check if we already received method */
	if (conn->get_url) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received GET method declartion when it was already received during handshake..closing session");
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	} /* end if */
	
	/* the get url must have a minimum size: GET / HTTP/1.1\r\n 16 (15 if only \n) */
	if (buffer_size < 15) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received uncomplete GET method during handshake, closing session");
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	} /* end if */
	
	/* skip white spaces */
	iterator = 4;
	while (iterator <  buffer_size && buffer[iterator] == ' ') 
		iterator++;
	if (buffer_size == iterator) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received a %s method without an starting request url, closing session", method);
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	} /* end if */

	/* now check url format */
	if (buffer[iterator] != '/') {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received a %s method with a request url that do not start with /, closing session", method);
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	}
	
	/* ok now find the rest of the url content util the next white space */
	iterator2 = (iterator + 1);
	while (iterator2 <  buffer_size && buffer[iterator2] != ' ') 
		iterator2++;
	if (buffer_size == iterator2) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received a %s method with an uncomplate request url, closing session", method);
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	} /* end if */
	
	(*url) = nopoll_new (char, iterator2 - iterator + 1);
	memcpy (*url, buffer + iterator, iterator2 - iterator);
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Found url method: '%s'", *url);

	/* now check final HTTP header */
	iterator = iterator2 + 1;
	while (iterator <  buffer_size && buffer[iterator] == ' ') 
		iterator++;
	if (buffer_size == iterator) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received a %s method with an uncomplate request url, closing session", method);
		nopoll_conn_shutdown (conn);
		return nopoll_false;
	} /* end if */

	/* now check trailing content */
	return nopoll_cmp (buffer + iterator, "HTTP/1.1\r\n") || nopoll_cmp (buffer + iterator, "HTTP/1.1\n");
}

/** 
 * @internal Function that parses the mime header found on the
 * provided buffer.
 */
nopoll_bool nopoll_conn_get_mime_header (noPollCtx * ctx, noPollConn * conn, const char * buffer, int buffer_size, char ** header, char ** value)
{
	int iterator = 0;
	int iterator2 = 0;

	if (buffer_size < 3) {
	        nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Expected to find mime header content (but buffer size %d was found)", buffer_size);
		return nopoll_false;
	}

	/* nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Processing content: %s", buffer); */
	
	/* ok, find the first : */
	while (iterator < buffer_size && buffer[iterator] && buffer[iterator] != ':')
		iterator++;
	if (buffer[iterator] != ':') {
	        /* ensure print always ends */
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Expected to find mime header separator : but it wasn't found, buffer_size=%d, iterator=%d..",
			    buffer_size, iterator);
		return nopoll_false;
	} 

	/* copy the header value */
	(*header) = nopoll_new (char, iterator + 1);
	memcpy (*header, buffer, iterator);
	
	/* now get the mime header value */
	iterator2 = iterator + 1;
	while (iterator2 < buffer_size && buffer[iterator2] && buffer[iterator2] != '\n')
		iterator2++;
	if (buffer[iterator2] != '\n') {
	        nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, 
			    "Expected to find mime header value end (13) but it wasn't found (iterator=%d, iterator2=%d, buffer_size=%d, for header: [%s], found value: [%d])..",
			    iterator, iterator2, buffer_size, (*header), (int)buffer[iterator2]);
		nopoll_free (*header);
		(*header) = NULL;
		(*value)  = NULL;
		return nopoll_false;
	} 

	/* copy the value */
	(*value) = nopoll_new (char, (iterator2 - iterator) + 1);
	memcpy (*value, buffer + iterator + 1, iterator2 - iterator);

	/* trim content */
	nopoll_trim (*value, NULL);
	nopoll_trim (*header, NULL);
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Found MIME header: '%s' -> '%s'", *header, *value);
	return nopoll_true;
}

/** 
 * @internal Function that ensures we don't receive any 
 */ 
nopoll_bool nopoll_conn_check_mime_header_repeated (noPollConn   * conn, 
						    char         * header, 
						    char         * value, 
						    const char   * ref_header, 
						    noPollPtr      check)
{
	if (strcasecmp (ref_header, header) == 0) {
		if (check) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Provided header %s twice, closing connection", header);
			nopoll_free (header);
			nopoll_free (value);
			nopoll_conn_shutdown (conn);
			return nopoll_true;
		} /* end if */
	} /* end if */
	return nopoll_false;
}

char * nopoll_conn_produce_accept_key (noPollCtx * ctx, const char * websocket_key)
{
	const char    * static_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";	
	char          * accept_key;	
	int             accept_key_size;
	int             key_length;
	unsigned char   buffer[EVP_MAX_MD_SIZE];
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	EVP_MD_CTX      mdctx;
#else
	EVP_MD_CTX    * mdctx;
#endif
	const EVP_MD  * md = NULL;
	unsigned int    md_len = EVP_MAX_MD_SIZE;

	if (websocket_key == NULL)
		return NULL;

	key_length  = strlen (websocket_key);

	accept_key_size = key_length + 36 + 1;
	accept_key      = nopoll_new (char, accept_key_size);

	memcpy (accept_key, websocket_key, key_length);
	memcpy (accept_key + key_length, static_guid, 36);
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "base key value: %s", accept_key);

	/* now sha-1 */
	md = EVP_sha1 ();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	EVP_DigestInit (&mdctx, md);
	EVP_DigestUpdate (&mdctx, accept_key, strlen (accept_key));
	EVP_DigestFinal (&mdctx, buffer, &md_len);
#else
	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit (mdctx, md);
	EVP_DigestUpdate (mdctx, accept_key, strlen (accept_key));
	EVP_DigestFinal (mdctx, buffer, &md_len);
	EVP_MD_CTX_destroy(mdctx);
#endif

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sha-1 length is: %u", md_len);
	/* now convert into base64 */
	if (! nopoll_base64_encode ((const char *) buffer, md_len, (char *) accept_key, &accept_key_size)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to base64 sec-websocket-key value..");
		return NULL;
	}

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Returning Sec-Websocket-Accept: %s", accept_key);
	
	return accept_key;
	
}

nopoll_bool __nopoll_conn_call_on_ready_if_defined (noPollCtx * ctx, noPollConn * conn)
{
	noPollActionHandler    on_ready;
	noPollPtr              on_ready_data;

	if (ctx == NULL || conn == NULL)
		return nopoll_false; /* this should never happen */

	if (ctx->on_ready || conn->on_ready) {
		/* set on ready handler, first considering conn->on_ready and then ctx->on_ready */
		on_ready      = conn->on_ready;
		on_ready_data = conn->on_ready_data;
		if (! on_ready) {
			on_ready      = ctx->on_ready;
			on_ready_data = ctx->on_ready_data;
		} /* end if */

		if (on_ready &&  ! on_ready (ctx, conn, on_ready_data)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Peer from %s:%s was denied by application level (on ready handler: %p), clossing session", 
				    conn->host, conn->port, on_ready);
			nopoll_conn_shutdown (conn);
			return nopoll_false;
		}
	} /* end if */

	/* reached this point, return ok status (even if we have or no
	   on ready function defined) */
	return nopoll_true;

}

nopoll_bool nopoll_conn_complete_handshake_check_listener (noPollCtx * ctx, noPollConn * conn)
{
	char                 * reply;
	int                    reply_size;
	char                 * accept_key;
	const char           * protocol;
	nopoll_bool            origin_check;

	/* call to check listener handshake */
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Checking client handshake data..");

	/* update default origin check (Origin: header must be
	 * defined, event though RFC says SHOULD it should have been
	 * MUST because otherwise, server side cannot check this
	 * value which is crucial for security reasons */
	origin_check = conn->origin != NULL;

	/* now, update this origin_check in the case it is not defined
	 * and origin header skip was enabled */
	if ( conn->listener && conn->listener->opts && conn->listener->opts->skip_origin_header_check && conn->origin == NULL)
		origin_check = nopoll_true;
	

	/* ensure we have all minumum data */
	if (! conn->handshake->upgrade_websocket ||
	    ! conn->handshake->connection_upgrade ||
	    ! conn->handshake->websocket_key ||
	    ! origin_check ||  /* see above */
	    ! conn->handshake->websocket_version) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Client from %s:%s didn't provide all websocket handshake values required, closing session (Upgraded: websocket %d, Connection: upgrade%d, Sec-WebSocket-Key: %p, Origin: %p, Sec-WebSocket-Version: %p)",
			    conn->host, conn->port,
			    conn->handshake->upgrade_websocket,
			    conn->handshake->connection_upgrade,
			    conn->handshake->websocket_key,
			    conn->origin,
			    conn->handshake->websocket_version);
		return nopoll_false;
	} /* end if */

	/* check protocol support */
	if (strtod (conn->handshake->websocket_version, NULL) != ctx->protocol_version) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received request for an unsupported protocol version: %s, expected: %d",
			    conn->handshake->websocket_version, ctx->protocol_version);
		return nopoll_false;
	} /* end if */
	
	/* now call the user app level to accept the websocket
	   connection */
	if (ctx->on_open) {
		if (! ctx->on_open (ctx, conn, ctx->on_open_data)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Client from %s:%s was denied by application level (on open handler %p), clossing session", 
				    conn->host, conn->port, ctx->on_open);
			nopoll_conn_shutdown (conn);
			return nopoll_false;
		}
	} /* end if */
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Client from %s:%s was accepted, replying accept",
		    conn->host, conn->port);

	/* produce accept key */
	accept_key = nopoll_conn_produce_accept_key (ctx, conn->handshake->websocket_key);
	
	/* ok, send handshake reply */
	if (conn->protocols || conn->accepted_protocol) {
		/* set protocol in the reply taking preference by the
		   value configured at conn->accepted_protocol */
		protocol = conn->accepted_protocol;
		if (! protocol)
			protocol = conn->protocols;

		/* send accept header accepting protocol requested by the user */
		reply = nopoll_strdup_printf ("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nSec-WebSocket-Protocol: %s\r\n\r\n", 
					      accept_key, protocol);
	} else {
		/* send accept header without telling anything about protocols */
		reply = nopoll_strdup_printf ("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", 
					      accept_key);
	}
		
	nopoll_free (accept_key);
	if (reply == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to build reply, closing session");
		return nopoll_false;
	} /* end if */
	
	reply_size = strlen (reply);
	if (reply_size != conn->send (conn, reply, reply_size)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to send reply, there was a failure, error code was: %d", errno);
		nopoll_free (reply);
		return nopoll_false;
	} /* end if */
	
	/* free reply */
	nopoll_free (reply);

	/* now call the user app level to accept the websocket
	   connection */
	if (! __nopoll_conn_call_on_ready_if_defined (ctx, conn))
		return nopoll_false;
	
	return nopoll_true; /* signal handshake was completed */
}

nopoll_bool nopoll_conn_complete_handshake_check_client (noPollCtx * ctx, noPollConn * conn)
{
	char         * accept;
	nopoll_bool    result;

	/* check all data received */
	if (! conn->handshake->websocket_accept ||
	    ! conn->handshake->upgrade_websocket ||
	    ! conn->handshake->connection_upgrade) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received uncomplete listener handshake reply (%p %d %d)",
			    conn->handshake->websocket_accept, conn->handshake->upgrade_websocket, conn->handshake->connection_upgrade);
		return nopoll_false;
	} /* end if */

	/* check accept value here */
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Checking accept key from listener..");
	accept = nopoll_conn_produce_accept_key (ctx, conn->handshake->websocket_key);
	
	result = nopoll_cmp (accept, conn->handshake->websocket_key);
	if (! result) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to accept connection Sec-Websocket-Accept %s is not expected %s, closing session",
			    accept, conn->handshake->websocket_key);
		nopoll_conn_shutdown (conn);
	}
	nopoll_free (accept);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sec-Websocket-Accept matches expected value..nopoll_conn_complete_handshake_check_client (%p, %p)=%d",
		    ctx, conn, result);

	/* now call the user app level to accept the websocket
	   connection */
	if (! __nopoll_conn_call_on_ready_if_defined (ctx, conn))
		return nopoll_false;

	return result;
}


/** 
 * @internal Function used to validate all handshake received until
 * now.
 */
void nopoll_conn_complete_handshake_check (noPollConn * conn)
{
	noPollCtx    * ctx    = conn->ctx;
	nopoll_bool    result = nopoll_false;

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "calling to check handshake received on connection id %d role %d..",
		    conn->id, conn->role);

	if (conn->role == NOPOLL_ROLE_LISTENER) {
		result = nopoll_conn_complete_handshake_check_listener (ctx, conn);
	} else if (conn->role == NOPOLL_ROLE_CLIENT) {
		result = nopoll_conn_complete_handshake_check_client (ctx, conn);
	} /* end if */

	/* flag connection as ready: now we can get messages */
	if (result) {
		conn->handshake_ok = nopoll_true;
	} else {
		nopoll_conn_shutdown (conn);
	} /* end if */

	return;
}

/** 
 * @internal Handler that implements one step of the websocket
 * listener handshake received from client, until it is completed.
 *
 * @return Returns 0 to return (because error or because no data is
 * available) or 1 to signal the caller continue reading if it is
 * possible.
 */
int nopoll_conn_complete_handshake_listener (noPollCtx * ctx, noPollConn * conn, char * buffer, int buffer_size)
{
	char * header;
	char * value;

	/* handle content */
	if (nopoll_ncmp (buffer, "GET ", 4)) {
		/* get url method */
		nopoll_conn_get_http_url (conn, buffer, buffer_size, "GET", &conn->get_url);
		return 1;
	} /* end if */

	/* get mime header */
	if (! nopoll_conn_get_mime_header (ctx, conn, buffer, buffer_size, &header, &value)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to acquire mime header from remote peer during handshake, closing connection");
		nopoll_conn_shutdown (conn);
		return 0;
	}
		
	/* ok, process here predefined headers */
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Host", conn->host_name))
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Upgrade", INT_TO_PTR (conn->handshake->upgrade_websocket))) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Connection", INT_TO_PTR (conn->handshake->connection_upgrade))) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Key", conn->handshake->websocket_key)) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Origin", conn->origin)) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Protocol", conn->protocols)) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Version", conn->handshake->websocket_version)) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Cookie", conn->handshake->cookie)) 
		return 0;
	
	/* set the value if required */
	if (strcasecmp (header, "Host") == 0)
		conn->host_name = value;
	else if (strcasecmp (header, "Sec-Websocket-Key") == 0)
		conn->handshake->websocket_key = value;
	else if (strcasecmp (header, "Origin") == 0)
		conn->origin = value;
	else if (strcasecmp (header, "Sec-Websocket-Protocol") == 0)
		conn->protocols = value;
	else if (strcasecmp (header, "Sec-Websocket-Version") == 0)
		conn->handshake->websocket_version = value;
	else if (strcasecmp (header, "Upgrade") == 0) {
		conn->handshake->upgrade_websocket = 1;
		nopoll_free (value);
	} else if (strcasecmp (header, "Connection") == 0) {
		conn->handshake->connection_upgrade = 1;
		nopoll_free (value);
	} else if (strcasecmp (header, "Cookie") == 0) {
		/* record cookie so it can be used by the application level */
		conn->handshake->cookie = value;
	} else {
		/* release value, no body claimed it */
		nopoll_free (value);
	} /* end if */
	
	/* release the header */
	nopoll_free (header);

	return 1; /* continue reading lines */
}

/** 
 * @internal Handler that implements one step of the websocket
 * client handshake received from the server, until it is completed. 
 *
 * @return Returns 0 to return (because error or because no data is
 * available) or 1 to signal the caller continue reading if it is
 * possible.
 */
int nopoll_conn_complete_handshake_client (noPollCtx * ctx, noPollConn * conn, char * buffer, int buffer_size)
{
	char * header;
	char * value;
	int    iterator;

	/* handle content */
	if (! conn->handshake->received_101 && nopoll_ncmp (buffer, "HTTP/1.1 ", 9)) {
		iterator = 9;
		while (iterator < buffer_size && buffer[iterator] && buffer[iterator] == ' ')
			iterator++;
		if (! nopoll_ncmp (buffer + iterator, "101", 3)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "websocket server denied connection with: %s", buffer + iterator);
			return 0; /* do not continue */
		} /* end if */

		/* flag that we have received HTTP/1.1 101 indication */
		conn->handshake->received_101 = nopoll_true;

		return 1;
	} /* end if */

	/* get mime header */
	if (! nopoll_conn_get_mime_header (ctx, conn, buffer, buffer_size, &header, &value)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to acquire mime header from remote peer during handshake, closing connection");
		nopoll_conn_shutdown (conn);
		return 0;
	}
		
	/* ok, process here predefined headers */
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Upgrade", INT_TO_PTR (conn->handshake->upgrade_websocket))) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Connection", INT_TO_PTR (conn->handshake->connection_upgrade))) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Accept", conn->handshake->websocket_accept)) 
		return 0;
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Protocol", conn->accepted_protocol)) 
		return 0;
	
	/* set the value if required */
	if (strcasecmp (header, "Sec-Websocket-Accept") == 0)
		conn->handshake->websocket_accept = value;
	else if (strcasecmp (header, "Sec-Websocket-Protocol") == 0)
		conn->accepted_protocol = value;
	else if (strcasecmp (header, "Upgrade") == 0) {
		conn->handshake->upgrade_websocket = 1;
		nopoll_free (value);
	} else if (strcasecmp (header, "Connection") == 0) {
		conn->handshake->connection_upgrade = 1;
		nopoll_free (value);
	} else {
		/* release value, no body claimed it */
		nopoll_free (value);
	} /* end if */
	
	/* release the header */
	nopoll_free (header);

	return 1; /* continue reading lines */
}

/** 
 * @internal Function that completes the handshake in an non-blocking
 * manner taking into consideration the connection type (listener or
 * client).
 */
void nopoll_conn_complete_handshake (noPollConn * conn)
{
	/* for NOPOLL_HANDSHAKE_BUFFER_SIZE definition, see
	   nopoll_decl.h */
	char        buffer[NOPOLL_HANDSHAKE_BUFFER_SIZE];
	int         buffer_size;
	noPollCtx * ctx = conn->ctx;

	/* ensure we didn't complete handshake */
	if (conn->handshake_ok)
		return;

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Checking to complete conn-id=%d WebSocket handshake, role %d", conn->id, conn->role);

	/* ensure handshake object is created */
	if (conn->handshake == NULL)
		conn->handshake = nopoll_new (noPollHandShake, 1);

	/* get lines and complete the handshake data */
	while (nopoll_true) {
		/* clear buffer for debugging functions */
		buffer[0] = 0;

		/* get next line to process: for
		   NOPOLL_HANDSHAKE_BUFFER_SIZE definition, see
		   nopoll_decl.h */
		buffer_size = nopoll_conn_readline (conn, buffer, NOPOLL_HANDSHAKE_BUFFER_SIZE);
		if (buffer_size == 0 || buffer_size == -1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unexpected connection close during handshake..closing connection");
			nopoll_conn_shutdown (conn);
			return;
		} /* end if */

		/* no data at this moment, return to avoid consuming data */
		if (buffer_size == -2) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "No more data available on connection id %d", conn->id);
			return;
		}

		/* drop a debug line */
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Bytes read %d from connection id %d: %s", buffer_size, conn->id, buffer);  
			
		/* check if we have received the end of the
		   websocket client handshake */
		if (buffer_size == 2 && nopoll_ncmp (buffer, "\r\n", 2)) {
			nopoll_conn_complete_handshake_check (conn);
			return;
		}

		if (conn->role == NOPOLL_ROLE_LISTENER) {
			/* call to complete listener handshake */
			if (nopoll_conn_complete_handshake_listener (ctx, conn, buffer, buffer_size) == 1)
				continue;
		} else if (conn->role == NOPOLL_ROLE_CLIENT) {
			/* call to complete listener handshake */
			if (nopoll_conn_complete_handshake_client (ctx, conn, buffer, buffer_size) == 1)
				continue;
		} else {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Called to handle connection handshake on a connection with an unexpected role: %d, closing session",
				    conn->role);
			nopoll_conn_shutdown (conn);
			return;
		}
	} /* end while */

	return;
}

void nopoll_conn_mask_content (noPollCtx * ctx, char * payload, int payload_size, char * mask, int desp)
{
	int iter       = 0;
	int mask_index = 0;

	while (iter < payload_size) {
		/* rotate mask and apply it */
		mask_index = (iter + desp) % 4;
		payload[iter] ^= mask[mask_index];
		iter++;
	} /* end while */

	return;
} 


/** 
 * @brief Allows to get the next message available on the provided
 * connection. The function returns NULL in the case no message is
 * still ready to be returned. 
 *
 * This function is design to not block the caller. However,
 * connection socket must be in non-blocking configuration. If you
 * have not configured anything, this is the default.
 *
 * If the function blocks caller then the socket associated to \ref
 * noPollConn is configured to make blocking I/O (maybe because you
 * configured like this or the socket was passed to another library
 * that did such configuration or maybe because you are using \ref
 * nopoll_conn_new_with_socket). 
 *
 * @param conn The connection where the read operation will take
 * place.
 * 
 * @return A reference to a noPollMsg object or NULL if there is
 * nothing available. In case the function returns NULL, check
 * connection status with \ref nopoll_conn_is_ok. If the function
 * blocks the caller check socket configuration.
 */
noPollMsg   * nopoll_conn_get_msg (noPollConn * conn)
{
	char        buffer[20];
	int         bytes;
	noPollMsg * msg;
	int         ssl_error;
	int         header_size = 2;
#if defined(SHOW_DEBUG_LOG)
	long        result;
#endif
	unsigned char *len;

	if (conn == NULL)
		return NULL;

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, 
		    "=== START: conn-id=%d (errno=%d, session: %d, conn->handshake_ok: %d, conn->pending_ssl_accept: %d) ===", 
		    conn->id, errno, conn->session, conn->handshake_ok, conn->pending_ssl_accept);
	
	/* check for accept SSL connection */
	if (conn->pending_ssl_accept) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received connect over a connection (id %d) with TLS handshake pending to be finished, processing..",
			    conn->id);

		/* get ssl error */
		ssl_error = SSL_accept (conn->ssl);
		if (ssl_error == -1) {
			/* get error */
			ssl_error = SSL_get_error (conn->ssl, -1);
 
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "accept function have failed (for listener side) ssl_error=%d : dumping error stack..", ssl_error);
 
			switch (ssl_error) {
			case SSL_ERROR_WANT_READ:
			        nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "still not prepared to continue because read wanted conn-id=%d (%p, session %d)",
					    conn->id, conn, conn->session);
				return NULL;
			case SSL_ERROR_WANT_WRITE:
			        nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "still not prepared to continue because write wanted conn-id=%d (%p)",
					    conn->id, conn);
				return NULL;
			default:
				break;
			} /* end switch */

			/* TLS-fication process have failed */
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "there was an error while accepting TLS connection");
			nopoll_conn_log_ssl (conn);
			nopoll_conn_shutdown (conn);
			return NULL;
		} /* end if */

		/* ssl accept */
		conn->pending_ssl_accept = nopoll_false;
		nopoll_conn_set_sock_block (conn->session, nopoll_false);

#if defined(SHOW_DEBUG_LOG)
		result = SSL_get_verify_result (conn->ssl);
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Completed TLS operation from %s:%s (conn id %d, ssl veriry result: %d)",
			    conn->host, conn->port, conn->id, (int) result);
#endif

		/* configure default handlers */
		conn->receive = nopoll_conn_tls_receive;
		conn->send    = nopoll_conn_tls_send;

		/* call to check post ssl checks after SSL finalization */
		if (conn->ctx && conn->ctx->post_ssl_check) {
			if (! conn->ctx->post_ssl_check (conn->ctx, conn, conn->ssl_ctx, conn->ssl, conn->ctx->post_ssl_check_data)) {
				/* TLS post check failed */
				nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "TLS/SSL post check function failed, dropping connection");
				nopoll_conn_shutdown (conn);
				return NULL;
			} /* end if */
		} /* end if */

		/* set this connection has TLS ok */
		conn->tls_on  = nopoll_true;

#if defined(NOPOLL_OS_UNIX)
		/* report NULL because this was a call to complete TLS */
		errno = NOPOLL_EWOULDBLOCK; /* simulate there is no data available to stop
					       here. If there is no data indeed, on next
					       call it will not fail */
#elif defined(NOPOLL_OS_WIN32)
		WSASetLastError(NOPOLL_EWOULDBLOCK); /* simulate there is no data available */
#endif
		return NULL;
		
	} /* end if */

	/* check connection status */
	if (! conn->handshake_ok) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Connection id %d handshake is not complete, running..", conn->id);
		/* acquire here handshake mutex */
		nopoll_mutex_lock (conn->handshake_mutex);

		/* complete handshake */
		nopoll_conn_complete_handshake (conn);

		/* release here handshake mutex */
		nopoll_mutex_unlock (conn->handshake_mutex);

		if (! conn->handshake_ok) 
			return NULL;
	} /* end if */

	if (conn->previous_msg) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Reading bytes (previously read %d) from a previous unfinished frame (pending: %d) over conn-id=%d",
			    conn->previous_msg->payload_size, conn->previous_msg->remain_bytes, conn->id);

		if (conn->read_pending_header) {
			/* detected pending read header, continue from here */
			msg                       = conn->previous_msg;
			conn->previous_msg        = NULL;
			conn->read_pending_header = nopoll_false;
			goto read_pending_header;
		} /* end if */
		
		/* build next message holder to continue with this content */
		if (conn->previous_msg->payload_size > 0) {
			msg = nopoll_msg_new ();
			if (msg == NULL) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to allocate memory for received message, closing session id: %d", 
					    conn->id);
				nopoll_conn_shutdown (conn);
				return NULL;
			} /* end if */

			/* flag this message as a fragment */
			msg->is_fragment = nopoll_true;

			/* get fin bytes */
			msg->has_fin      = 1; /* for now final message */
			msg->op_code      = 0; /* continuation frame */

			/* copy initial mask indication */
			msg->is_masked    = conn->previous_msg->is_masked;
			msg->payload_size = conn->previous_msg->remain_bytes;
			msg->unmask_desp  = conn->previous_msg->unmask_desp;

			/* copy mask */
			memcpy (msg->mask, conn->previous_msg->mask, 4);
			
			if (msg->is_masked) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Reusing mask value = %d from previous frame", nopoll_get_32bit (msg->mask));
				nopoll_show_byte (conn->ctx, msg->mask[0], "mask[0]");
				nopoll_show_byte (conn->ctx, msg->mask[1], "mask[1]");
				nopoll_show_byte (conn->ctx, msg->mask[2], "mask[2]");
				nopoll_show_byte (conn->ctx, msg->mask[3], "mask[3]");
			}

			/* release previous reference because de don't need it anymore */
			nopoll_msg_unref (conn->previous_msg);
		} else {
			/* reuse reference */
			msg = conn->previous_msg;

			/* update remaining bytes */
			msg->payload_size = msg->remain_bytes;
			nopoll_free (msg->payload);
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "reusing noPollMsg reference (%p) since last payload read was 0, remaining: %d", msg,
				    msg->payload_size);
		}

		/* nullify references */
		conn->previous_msg      = NULL;

		goto read_payload;
		
	} /* end if */

	/*
	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	  +-+-+-+-+-------+-+-------------+-------------------------------+
	  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	  |I|S|S|S|  (4)  |A|     (7)     |             (16/63)           |
	  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	  | |1|2|3|       |K|             |                               |
	  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	  |     Extended payload length continued, if payload len == 127  |
	  + - - - - - - - - - - - - - - - +-------------------------------+
	  |                               |Masking-key, if MASK set to 1  |
	  +-------------------------------+-------------------------------+
	  | Masking-key (continued)       |          Payload Data         |
	  +-------------------------------- - - - - - - - - - - - - - - - +
	  :                     Payload Data continued ...                :
	  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	  |                     Payload Data continued ...                |
	  +---------------------------------------------------------------+
	*/
	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Found data in opened connection id %d..", conn->id);*/ 

	/* get the first 2 bytes from the websocket header */
	bytes = __nopoll_conn_receive (conn, buffer, 2);
	if (bytes == 0) {
		/* connection not ready */
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Connection id=%d without data, errno=%d : %s, returning no message",
			    conn->id, errno, strerror (errno));
		return NULL;
	}

	if (bytes <= 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received connection close, finishing connection session");
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	if (bytes != 2) { 
		/* ok, store content read into the pending buffer for next call */
		memcpy (conn->pending_buf + conn->pending_buf_bytes, buffer, bytes);
		conn->pending_buf_bytes += bytes;
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG,
			    "Expected to receive complete websocket frame header but found only %d bytes over conn-id=%d, saving to reuse later",
			    bytes, conn->id);
		return NULL;
	} /* end if */

	/* record header size */
	header_size = 2;

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received %d bytes for websocket header", bytes);
	nopoll_show_byte (conn->ctx, buffer[0], "header[0]");
	nopoll_show_byte (conn->ctx, buffer[1], "header[1]");

	/* build next message */
	msg = nopoll_msg_new ();
	if (msg == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to allocate memory for received message, closing session id: %d", 
			    conn->id);
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	/* get fin bytes */
	msg->has_fin      = nopoll_get_bit (buffer[0], 7);
	msg->op_code      = buffer[0] & 0x0F;
	msg->is_masked    = nopoll_get_bit (buffer[1], 7);
	msg->payload_size = buffer[1] & 0x7F;

	/* ensure FIN = 1 in case we are listener */
	if (conn->role == NOPOLL_ROLE_LISTENER && ! msg->is_masked) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received websocket frame with mask bit set to zero, closing session id: %d", 
			    conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	/* check payload size value */
	if (msg->payload_size < 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received wrong payload size at first 7 bits, closing session id: %d", 
			    conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

 read_pending_header:
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "interim payload size received: %d", (int) msg->payload_size);

	/* read the rest */
	if (msg->payload_size < 126) {
		/* nothing to declare here */

	} else if (msg->payload_size == 126) {
		/* get extended 2 bytes length as unsigned 16 bit
		   unsigned integer */
		bytes = __nopoll_conn_receive (conn, buffer + 2, 2);
		if (bytes != 2) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Failed to get next 2 bytes to read header from the wire, but received=%d, failed to received content, shutting down id=%d the connection, errno=%d (%s)",
				    bytes, conn->id, errno, strerror (errno));
			if (errno == NOPOLL_EWOULDBLOCK || errno == 0) { 
				/* connection is not ready at this point */
				conn->previous_msg = msg;
				conn->read_pending_header = nopoll_true;

				/* check amount of bytes to reuse them */
				nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Detected broken WebSocket peer sending header content using different frames, trying to save and resume later");
				if (bytes > 0) {
					/* ok, store content read into the pending buffer for next call */
					memcpy (conn->pending_buf + conn->pending_buf_bytes, buffer + 2, bytes);
					conn->pending_buf_bytes += bytes;
				}
				
				return NULL;
			}

			nopoll_msg_unref (msg);
			nopoll_conn_shutdown (conn);
			return NULL; 	
		} /* end if */

		/* add to the header bytes read */
		header_size += bytes;
			
		msg->payload_size = nopoll_get_16bit (buffer + 2);

		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received (%d) bytes in header (size %d) for payload size indication, which finally is: %d", bytes, header_size,(int) msg->payload_size);
		
	} else if (msg->payload_size == 127) {
		/* read more content (next 8 bytes) */
		if ((bytes = __nopoll_conn_receive (conn, buffer, 8)) != 8) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, 
				    "Expected to receive next 6 bytes for websocket frame header but found only %d bytes, closing session: %d",
				    bytes, conn->id);
			nopoll_conn_shutdown (conn);
			return NULL;
		} /* end if */

                len = (unsigned char*)buffer;
		msg->payload_size = 0;
#if defined(NOPOLL_64BIT_PLATFORM)
		msg->payload_size |= ((long)(len[0]) << 56);
		msg->payload_size |= ((long)(len[1]) << 48);
		msg->payload_size |= ((long)(len[2]) << 40);
		msg->payload_size |= ((long)(len[3]) << 32);
#else
		if (len[0] || len[1] || len[2] || len[3]) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "noPoll doesn't support messages bigger than 2GB on this plataform (support for 64bit not found)");
			nopoll_msg_unref (msg);
			nopoll_conn_shutdown (conn);
			return NULL;
		}
#endif
		msg->payload_size |= ((long)(len[4]) << 24);
		msg->payload_size |= ((long)(len[5]) << 16);
		msg->payload_size |= ((long)(len[6]) << 8);
		msg->payload_size |= len[7];
	} /* end if */

	if (msg->op_code == NOPOLL_PONG_FRAME) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "PONG received over connection id=%d", conn->id);
		nopoll_msg_unref (msg);
		return NULL;
	} /* end if */

	if (msg->op_code == NOPOLL_CLOSE_FRAME) {

		/* report that a closed frame was received */
		conn->peer_close_status = 1005;

		if (msg->payload_size == 0) {
			/* nothing more to add here, close frame
			   without content received, so we have no
			   reason to keep on reading */
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Proper connection close frame received id=%d, shutting down", conn->id);
			nopoll_msg_unref (msg);
			nopoll_conn_shutdown (conn);
			return NULL;
		} /* end if */

		/* received close frame with content, try to read the content */
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Proper connection close frame received id=%d with content bytes=%d, reading reason..", 
			    conn->id, msg->payload_size);
	} /* end if */

	/* get more bytes */
	if (msg->is_masked) {
		bytes = __nopoll_conn_receive (conn, (char *) msg->mask, 4);
		if (bytes != 4) {
			/* record header read so far */
			memcpy (conn->pending_buf, buffer, header_size);
			conn->pending_buf_bytes = header_size;
			/* record mask read so far if required */
			if (bytes > 0) {
				memcpy (conn->pending_buf + header_size, msg->mask, bytes);
				conn->pending_buf_bytes += bytes;
			} /* end if */

			/* release message because it not available here */
			nopoll_msg_unref (msg);
			if (bytes >= 0 && nopoll_conn_is_ok (conn)) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG,
					    "Expected to receive incoming mask after header (4 bytes) but found %d bytes on conn-id=%d, saving %d for future operations ", 
					    bytes, conn->id, conn->pending_buf_bytes);
				return NULL;
			} /* end if */

			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Expected to receive incoming mask after header (4 bytes) but found %d bytes, shutting down id=%d the connection", 
				    bytes, conn->id);
			nopoll_conn_shutdown (conn);
			return NULL; 
		} /* end if */
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received mask value = %d", nopoll_get_32bit (msg->mask));
		nopoll_show_byte (conn->ctx, msg->mask[0], "mask[0]");
		nopoll_show_byte (conn->ctx, msg->mask[1], "mask[1]");
		nopoll_show_byte (conn->ctx, msg->mask[2], "mask[2]");
		nopoll_show_byte (conn->ctx, msg->mask[3], "mask[3]");
	} /* end if */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Detected incoming websocket frame: fin(%d), op_code(%d), is_masked(%d), payload size(%ld), mask=%d", 
		    msg->has_fin, msg->op_code, msg->is_masked, msg->payload_size, nopoll_get_32bit (msg->mask));

	/* check payload size */
	if (msg->payload_size == 0) {
		/* check for empty PING frames (RFC6455 5.5.2. Ping
		   frame may include 'Application data'. Fixes
		   https://github.com/ASPLes/nopoll/issues/31 */
		if (msg->op_code == NOPOLL_PING_FRAME) {

			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "PING received over connection id=%d, replying PONG", conn->id);
			/* call to send pong */
			nopoll_conn_send_pong (conn, nopoll_msg_get_payload_size (msg), (noPollPtr)nopoll_msg_get_payload (msg));
			nopoll_msg_unref (msg);

			/* reporting no message (but no error) */
			return NULL;
		} /* end if */

		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Found incoming frame with payload size 0, shutting down id=%d the connection", conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL; 	
	} /* end if */

	/* check here for the limit of message we are willing to accept */
	/* FIX SECURITY ISSUE */

read_payload:

	/* copy payload received */
	msg->payload = nopoll_new (char, msg->payload_size + 1);	/* allow extra byte for string terminator */
	if (msg->payload == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to read the incoming frame, dropping connection id=%d", conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;		
	} /* end if */

	bytes = __nopoll_conn_receive (conn, (char *) msg->payload, msg->payload_size);
	if (bytes < 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Connection lost during message reception, dropping connection id=%d, bytes=%d, errno=%d : %s", 
			    conn->id, bytes, errno, strerror (errno));
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;		
	} /* end if */

	/* add string terminator */
	((char *) msg->payload)[bytes] = 0;

	/* record we've got content pending to be read */
	msg->remain_bytes = msg->payload_size - bytes;	
	if (msg->remain_bytes > 0) {

		/* set connection in remaining data to read */
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received fewer bytes than expected (bytes: %d < payload size: %d)",
			    bytes, (int) msg->payload_size);
		msg->payload_size = bytes;

		/* grab a reference to previous message to reuse itsdata but only when bytes > 0 
		   because when bytes == 0, the reference is reused since it is not returned
		   to the caller (see next lines) */
		if (bytes > 0)
			nopoll_msg_ref (msg);
		conn->previous_msg = msg;

		/* flag this message as a fragment */
		msg->is_fragment = nopoll_true;

		/* flag that this message doesn't have FIN = 0 because
		 * we wasn't able to read it entirely */
		/* msg->has_fin = 0; */
	} /* end if */

	/* flag the message was being a fragment according to previous flag */
	msg->is_fragment = msg->is_fragment || conn->previous_was_fragment || msg->has_fin == 0;

	/* update was a fragment */
	conn->previous_was_fragment = msg->is_fragment && msg->has_fin == 0;

	/* do not notify any frame since no content was found */
	if (bytes == 0 && msg == conn->previous_msg) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "bytes == %d, msg (%p) == conn->previous_msg (%p)",
			    bytes, msg, conn->previous_msg);
		return NULL;
	} /* end if */

	/* now unmask content (if required) */
	if (msg->is_masked) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Unmasking (payload size %d, mask: %d, msg: %p, desp: %d)", 
			    msg->payload_size, nopoll_get_32bit (msg->mask), msg, msg->unmask_desp);
		nopoll_conn_mask_content (conn->ctx, (char*) msg->payload, msg->payload_size, (char*) msg->mask, msg->unmask_desp);

		/* flag what was unmasked */
		msg->unmask_desp += msg->payload_size;
	} /* end if */

	/* check here close frame with reason */
	if (msg->op_code == NOPOLL_CLOSE_FRAME) {

		/* report that a closed frame was received */
		conn->peer_close_status = 1005;

		/* try to read reason and report those values */
		if (msg->payload_size >= 2) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Close frame received id=%d with content bytes=%d, peer status=%d, peer reason=%s, reading reason..", 
				    conn->id, msg->payload_size, nopoll_get_16bit (msg->payload), msg->payload + 2);

			/* get values so the user can get them */
			conn->peer_close_status = nopoll_get_16bit ((const char *) msg->payload);
			conn->peer_close_reason = nopoll_strdup ((const char *) msg->payload + 2);
		} /* end if */

		/* release message, close the connection and return
		   NULL to notify caller nothing to read for the
		   application */
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;
	}

	/* Received ping frame with payload */
	if (msg->payload_size != 0 && msg->op_code == NOPOLL_PING_FRAME) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "PING received over connection id=%d and payload_size=%d, replying PONG",
			    conn->id, msg->payload_size);
		nopoll_conn_send_pong (conn, nopoll_msg_get_payload_size (msg), (noPollPtr)nopoll_msg_get_payload (msg));
		nopoll_msg_unref (msg);
		return NULL;
	} /* end if */

	return msg;
}

/** 
 * @internal Implementation to send Frames according to various
 * parameters passed in into the function. This is the core function
 * used to send frames in noPoll.
 *
 * @param conn The connection where the content will be sent.
 *
 * @param content The content that will be sent (user level content)
 *
 * @param length The length of such content to be sent.
 *
 * @param has_fin nopoll_true/nopoll_false to signal FIN header flag 
 *
 * @param sleep_in_header Optional hacking option that allows to
 * include a pause between sending the header and the rest of the
 * content.
 */
int           __nopoll_conn_send_common (noPollConn * conn, const char * content, long length, nopoll_bool has_fin, long sleep_in_header, noPollOpCode frame_type)
{
	if (conn == NULL || content == NULL || length == 0 || length < -1)
		return -1;

	if (conn->role == NOPOLL_ROLE_MAIN_LISTENER) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Trying to send content over a master listener connection");
		return -1;
	} /* end if */

	if (length == -1) {
		if (NOPOLL_BINARY_FRAME == frame_type) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received length == -1 for binary frame. Unable to guess length");
			return -1;
		} /* end if */

		length = strlen (content);
	}
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "nopoll_conn_send_text: Attempting to send %d bytes", (int) length);

	/* sending content as client */
	if (conn->role == NOPOLL_ROLE_CLIENT) {
		return nopoll_conn_send_frame (conn, /* fin */ has_fin, /* masked */ nopoll_true, 
					       frame_type, length, (noPollPtr) content, sleep_in_header);
	} /* end if */

	/* sending content as listener */
	return nopoll_conn_send_frame (conn, /* fin */ has_fin, /* masked */ nopoll_false, 
				       frame_type, length, (noPollPtr) content, sleep_in_header);	
}

/** 
 * @brief Allows to send an UTF-8 text (op code 1) message over the
 * provided connection with the provided length.
 *
 * @param conn The connection where the message will be sent.
 *
 * @param content The content to be sent (it should be utf-8 content
 * or the function will fail).
 *
 * @param length Amount of bytes to take from the content to be
 * sent. If provided -1, it is assumed you are passing in a C-like
 * string nul terminated, so, that's the content to be sent.
 *
 * @return The number of bytes written otherwise < 0 is returned in
 * case of failure. The function will fail if some parameter is NULL
 * or undefined, or the content provided is not UTF-8. In the case of
 * failure, also check errno variable to know more what went wrong.
 *
 * See \ref nopoll_manual_retrying_write_operations to know more about error codes and when it is possible to retry write operations.
 * 
 * The function returns the number of bytes sent, being @length the 
 * max amount of bytes that can be reported as sent by
 * this funciton. This means value reported by this function do not
 * includes headers.  The funciton also returns the following general indications:
 *
 *   N : number of bytes sent (user land bytes sent, without including web socket headers).
 *   0 : no bytes sent (see errno indication). See also \ref nopoll_conn_complete_pending_write
 *  -1 : failure found
 *  -2 : retry operation needed (NOPOLL_EWOULDBLOCK)
 */
int           nopoll_conn_send_text (noPollConn * conn, const char * content, long length)
{
	/* do a send common operation with FIN = 1 */
	return __nopoll_conn_send_common (conn, content, length, nopoll_true, 0, NOPOLL_TEXT_FRAME);
}

/** 
 * @brief Allows to send an UTF-8 text (op code 1) message over the
 * provided connection with the provided length but flagging the frame
 * sent as not complete (more frames to come, that is, FIN = 0).
 *
 * @param conn The connection where the message will be sent.
 *
 * @param content The content to be sent (it should be utf-8 content
 * or the function will fail).
 *
 * @param length Amount of bytes to take from the content to be
 * sent. If provided -1, it is assumed you are passing in a C-like
 * string nul terminated, so, that's the content to be sent.
 *
 * @return The number of bytes written otherwise < 0 is returned in
 * case of failure. The function will fail if some parameter is NULL
 * or undefined, or the content provided is not UTF-8. In the case of
 * failure, also check errno variable to know more what went wrong.
 *
 * See \ref nopoll_manual_retrying_write_operations to know more about
 * error codes and when it is possible to retry write operations.
 *
 * The function returns the number of bytes sent, being @length the 
 * max amount of bytes that can be reported as sent by
 * this funciton. This means value reported by this function do not
 * includes headers.  The funciton also returns the following general indications:
 *
 *   N : number of bytes sent (user land bytes sent, without including web socket headers).
 *   0 : no bytes sent (see errno indication). See also \ref nopoll_conn_complete_pending_write
 *  -1 : failure found
 *  -2 : retry operation needed (NOPOLL_EWOULDBLOCK)
 */
int           nopoll_conn_send_text_fragment (noPollConn * conn, const char * content, long length)
{
	/* do a send common operation with FIN = 0 */
	return __nopoll_conn_send_common (conn, content, length, nopoll_false, 0, NOPOLL_TEXT_FRAME);
}

/** 
 * @brief Allows to send a binary (op code 2) message over the
 * provided connection with the provided length.
 *
 * @param conn The connection where the message will be sent.
 *
 * @param content The content to be sent (it should be utf-8 content
 * or the function will fail).
 *
 * @param length Amount of bytes to take from the content to be
 * sent. Note you cannot pass in -1 (unlike \ref nopoll_conn_send_text).
 *
 * @return The number of bytes written otherwise < 0 is returned in
 * case of failure. The function will fail if some parameter is NULL
 * or undefined. In the case of failure, also check errno variable to
 * know more what went wrong.
 *
 * See \ref nopoll_manual_retrying_write_operations to know more about error codes and when it is possible to retry write operations.
 *
 * The function returns the number of bytes sent, being @length the 
 * max amount of bytes that can be reported as sent by
 * this funciton. This means value reported by this function do not
 * includes headers.  The funciton also returns the following general indications:
 *
 *   N : number of bytes sent (user land bytes sent, without including web socket headers).
 *   0 : no bytes sent (see errno indication). See also \ref nopoll_conn_complete_pending_write
 *  -1 : failure found
 *  -2 : retry operation needed (NOPOLL_EWOULDBLOCK)
 */
int           nopoll_conn_send_binary (noPollConn * conn, const char * content, long length)
{
	return __nopoll_conn_send_common (conn, content, length, nopoll_true, 0, NOPOLL_BINARY_FRAME);
}


/** 
 * @brief Allows to send a binary (op code 2) message over the
 * provided connection with the provided length but flagging the frame
 * sent as not complete (more frames to come, that is, FIN = 0).
 *
 * @param conn The connection where the message will be sent.
 *
 * @param content The content to be sent (it should be utf-8 content
 * or the function will fail).
 *
 * @param length Amount of bytes to take from the content to be
 * sent. Note you cannot pass in -1 (unlike \ref
 * nopoll_conn_send_text).
 *
 * @return The number of bytes written otherwise < 0 is returned in
 * case of failure. The function will fail if some parameter is NULL
 * or undefined. In the case of failure, also check errno variable to
 * know more what went wrong.
 *
 * See \ref nopoll_manual_retrying_write_operations to know more about
 * error codes and when it is possible to retry write operations.
 *
 * The function returns the number of bytes sent, being @length the 
 * max amount of bytes that can be reported as sent by
 * this funciton. This means value reported by this function do not
 * includes headers.  The funciton also returns the following general indications:
 *
 *   N : number of bytes sent (user land bytes sent, without including web socket headers).
 *   0 : no bytes sent (see errno indication). See also \ref nopoll_conn_complete_pending_write
 *  -1 : failure found
 *  -2 : retry operation needed (NOPOLL_EWOULDBLOCK) 
 */
int           nopoll_conn_send_binary_fragment (noPollConn * conn, const char * content, long length)
{
	return __nopoll_conn_send_common (conn, content, length, nopoll_true, 0, NOPOLL_BINARY_FRAME);
}


/** 
 * @brief Allows to read the provided amount of bytes from the
 * provided connection, leaving the content read on the buffer
 * provided.
 *
 * Optionally, the function allows blocking the caller until the
 * amount of bytes requested are satisfied. Also, the function allows
 * to timeout the operation after provided amount of time.
 *
 * @param conn The connection where the read operation will take place.
 *
 * @param buffer The buffer where the result is returned. Memory
 * buffer must be enough to hold bytes requested and must be acquired
 * by the caller. 
 *
 * @param bytes Number of bytes to be read from the connection.
 *
 * @param block If nopoll_true, the caller will be blocked until the
 * amount of bytes requested are satisfied or until the timeout is
 * reached (if enabled). If nopoll_false is provided, the function
 * won't block and will return all bytes available at this moment.
 *
 * @param timeout (milliseconds 1sec = 1000ms) If provided a value
 * higher than 0, a timeout will be enabled to complete the
 * operation. If the timeout is reached, the function will return the
 * bytes read so far. Please note that the function has a precision of
 * 10ms.
 *
 * @return Number of bytes read or -1 if it fails. The function
 * returns -1 when no content is available to be read and you pass
 * block == nopoll_false
 *
 * Note that the function doesn't clear the buffer received. Only
 * memory (bytes) notified by the value returned by this function
 * should be accessed by the caller. In the same direction you can't
 * use the buffer as a nul-terminated string because the function
 * doesn't add the final \0 to the content read.
 *
 * <b>More calls to nopoll_conn_read might be needed</b><br>
 *
 * Because \ref nopoll_conn_read calls to \ref nopoll_conn_get_msg to
 * get content to satisfy buffer read requested, it might happen that
 * the actual amount of bytes requested by this function (\ref
 * nopoll_conn_read) is smaller than the number of bytes that can be
 * satisfied (because \ref nopoll_conn_get_msg reported a \ref
 * noPollMsg with bigger content).
 *
 * In such case, these bytes will be reported on next call to
 * nopoll_conn_read. 
 *
 * However, it might also happen that the socket is being watched (by
 * select(), poll(), epoll(), similar mechanism) and because all bytes
 * have been read and are retained in a \ref noPollMsg inside the
 * provided \ref noPollConn) this might cause a block until a forced
 * nopoll_conn_read happens again (because the socket does not report
 * any content to be read and nopoll_conn_read is not called).
 *
 * In such scenarios, if you want to avoid such lockings, please use
 * \ref nopoll_conn_read_pending to do more calls to \ref nopoll_conn_read
 *
 *
 */
int           nopoll_conn_read (noPollConn * conn, char * buffer, int bytes, nopoll_bool block, long int timeout)
{
	long int           wait_slice = 0;
	noPollMsg        * msg        = NULL;
	struct  timeval    start;
	struct  timeval    stop;
	struct  timeval    diff;
	long               ellapsed   = 0;
	int                desp       = 0;
	int                amount;
	int                total_read = 0;
	int                total_pending = 0;

	/* report error value */
	if (conn == NULL || buffer == NULL || bytes <= 0)
		return -1;
	
	if (timeout > 1000)
		wait_slice = 100;
	else if (timeout > 100)
		wait_slice = 50;
	else if (timeout > 10)
		wait_slice = 10;

	if (timeout > 0)
#if defined(NOPOLL_OS_WIN32)
		nopoll_win32_gettimeofday (&start, NULL);
#else
		gettimeofday (&start, NULL);
#endif

	/* clear the buffer */
	memset (buffer, 0, bytes);

	/* check here if we have a pending message to read */
	if (conn->pending_msg)  {
		/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "nopoll_conn_read (found pending content: %d, requested %d)", conn->pending_diff, bytes); */
		/* get references to pending data */
		amount = conn->pending_diff;
		msg    = conn->pending_msg;
		if (amount > bytes) {
			/* check if bytes requested are bigger the
			 * conn->pending_diff */
			if (bytes < conn->pending_diff) {
				conn->pending_diff -= bytes;
			} else {
				/* update values */
				bytes = conn->pending_diff;
				conn->pending_diff = 0;
			} /* end if */

			amount = bytes;
		} else {
			conn->pending_diff = 0;
		}

		/* read content */
		memcpy (buffer, ((unsigned char *) nopoll_msg_get_payload (msg)) + conn->pending_desp, amount);
		total_read += amount;
		desp        = amount;
		/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "nopoll_conn_read total amount satisfied is not %d, requested %d", total_read, bytes); */
		
		/* increase pending desp */
		conn->pending_desp += amount;

		/* now release internally the content if consumed the message */
		if (conn->pending_diff == 0) {
			nopoll_msg_unref (conn->pending_msg);
			conn->pending_msg = NULL;
		} /* end if */

		/* see if we have finished */
		if (total_read == bytes || ! block) {
			if (total_read == 0 && ! block) 
				return -1;

			return total_read;
		}
		
		/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "### ====> Read %d bytes from previous pending frame, requested %d. Pending diff %d", total_read, bytes, conn->pending_diff); */
	} /* end if */


	/* for for the content */
	while (nopoll_true) {
		/* call to get next message */
		msg = nopoll_conn_get_msg (conn);
		if (msg == NULL) {
			if (! nopoll_conn_is_ok (conn)) {
			        nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received websocket conn-id=%d close during wait reply..",
					    conn->id);
				if (total_read == 0 && ! block)
					return -1;
				return total_read;
			} /* end if */

			if (! block) {
				if (total_read == 0 && ! block) {
#if defined(NOPOLL_OS_UNIX)
					errno = NOPOLL_EWOULDBLOCK; /* simulate there is no data available */
#elif defined(NOPOLL_OS_WIN32)
					WSASetLastError(NOPOLL_EWOULDBLOCK); /* simulate there is no data available */
#endif
					return -1;
                                }
				return total_read;
			} /* end if */
			
		} /* end if */

		/* get the message content into the buffer */
		if (msg) {
			/* get the amount of bytes we can read */
			amount = nopoll_msg_get_payload_size (msg);
			total_pending = bytes - total_read;
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "(New Frame) bytes received %d (requested %d, read %d, total_pending %d bytes, desp: %d)",
				    amount, bytes, total_read, total_pending, desp);
			if (amount > total_pending) {
				/* save here the difference between
				 * what we have read and remaining data */
				conn->pending_desp = total_pending;
				conn->pending_diff = amount - total_pending;
				conn->pending_msg  = msg;
				amount = total_pending;

				/* acquire a reference to the message */
				nopoll_msg_ref (msg);
			} /* end if */
			/* copy data */
			memcpy (buffer + desp, nopoll_msg_get_payload (msg), amount);
			total_read += amount;
			desp       += amount;
			/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "nopoll_conn_read total amount satisfied is not %d, requested %d, desp: %d",
			   total_read, bytes, desp); */

			/* release message */
			nopoll_msg_unref (msg);

			/* return amount read */
			if (total_read == bytes || ! block) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Finishing nopoll_conn_read because block=%d or total bytes requested=%d satisfied=%d ", 
					    block, bytes, total_read);

				if (total_read == 0 && ! block) 
					return -1;
				return total_read;
			}
		}

		/* check to stop due to timeout */
		if (timeout > 0) {
#if defined(NOPOLL_OS_WIN32)
			nopoll_win32_gettimeofday (&stop, NULL);
#else
			gettimeofday (&stop, NULL);
#endif
			nopoll_timeval_substract (&stop, &start, &diff);
			
			ellapsed = (diff.tv_sec * 1000) + (diff.tv_usec / 1000);
			if (ellapsed > (timeout)) 
				break;
		} /* end if */
		
		nopoll_sleep (wait_slice);
	} /* end while */

	/* reached this point, return that timeout was reached */
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Finishing nopoll_conn_read timeout reached=%ld ms , returning total bytes requested=%d satisfied=%d ", 
		    timeout, bytes, total_read);

	if (total_read == 0 && ! block)
		return -1;
	return total_read;
}

/** 
 * @brief Allows to check if the are pending bytes to be read on the
 * provided connection and that are retained in internal buffers so
 * the socket associated will not report pending content.
 *
 * See \ref nopoll_conn_read documentation to know more about this
 * function. This API is not useful if you are not using \ref
 * nopoll_conn_read (stream oriented APi).
 *
 * This function is designed to be used *after* you have completed a
 * \ref nopoll_conn_read so you can get a quick indication if there
 * are pending data that can be read without waiting and blocking
 * (especially when you use non blocking I/O).
 *
 * You have an working example at:
 *
 * https://github.com/ASPLes/libvortex-1.1/blob/master/web-socket/vortex_websocket.c#L404
 *
 * @param conn The connection where the operation takes place
 *
 * @return The amount of bytes pendings to be read (and a confirmation
 * that bytes are pending to be read) or 0 if nothing is pending. 
 */
int nopoll_conn_read_pending (noPollConn * conn) {
        if (conn == NULL || conn->pending_msg == NULL)
	        return 0;
	return conn->pending_diff;
}

/** 
 * @brief Allows to send a ping message over the Websocket connection
 * provided. The function will not block the caller.
 *
 * @param conn The connection where the PING operation will be sent.
 *
 * @return nopoll_true if the operation was sent without any error,
 * otherwise nopoll_false is returned.
 */
nopoll_bool      nopoll_conn_send_ping (noPollConn * conn)
{
	/* check input parameter to allow role check */
	if (conn == NULL)
		return nopoll_false;
	
	return nopoll_conn_send_frame (conn, nopoll_true, conn->role == NOPOLL_ROLE_CLIENT, NOPOLL_PING_FRAME, 0, NULL, 0) >= 0;
}

/** 
 * @brief Allows to configure an on message handler on the provided
 * connection that overrides the one configured at \ref noPollCtx.
 *
 * @param conn The connection to be configured with a particular on message handler.
 *
 * @param on_msg The on message handler configured.
 *
 * @param user_data User defined pointer to be passed in into the on message handler when it is called.
 * 
 */
void          nopoll_conn_set_on_msg (noPollConn              * conn,
				      noPollOnMessageHandler    on_msg,
				      noPollPtr                 user_data)
{
	if (conn == NULL)
		return;

	/* configure on message handler */
	conn->on_msg      = on_msg;
	conn->on_msg_data = user_data;

	return;
}

/** 
 * @brief Allows to configure a handler that is called when the
 * connection provided is ready to send and receive because all
 * WebSocket handshake protocol finished OK.
 *
 * Unlike handlers configured at \ref nopoll_ctx_set_on_open and \ref
 * nopoll_ctx_set_on_accept which get notified when the connection
 * isn't still working (because WebSocket handshake wasn't finished
 * yet), on read handlers configured here will get called just after
 * the WebSocket handshake has taken place.
 *
 * @param conn The connection to configure.
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
void           nopoll_conn_set_on_ready (noPollConn            * conn,
					 noPollActionHandler     on_ready,
					 noPollPtr               user_data)
{
	if (conn == NULL)
		return;

	/* set the handler */
	conn->on_ready = on_ready;
	if (conn->on_ready == NULL)
		conn->on_ready_data = NULL;
	else
		conn->on_ready_data = user_data;
	return;
}

/** 
 * @brief Allows to configure an OnClose handler that will be called
 * when the connection is closed.
 *
 * @param conn The connection to configure with the on close handle.
 *
 * @param on_close The handler to be configured.
 *
 * @param user_data A reference pointer to be passed in into the handler.
 */
void          nopoll_conn_set_on_close (noPollConn            * conn,
					noPollOnCloseHandler    on_close,
					noPollPtr               user_data)
{
	if (conn == NULL)
		return;

	/* configure on close handler */
	conn->on_close      = on_close;
	conn->on_close_data = user_data;

        return;
}

/** 
 * @internal Allows to send a pong message over the Websocket
 * connection provided. The function will not block the caller. This
 * function is not intended to be used by normal API consumer.
 *
 * @param conn The connection where the PING operation will be sent.
 *
 * @param nopoll_true if the operation was sent without any error,
 * otherwise nopoll_false is returned.
 */
nopoll_bool      nopoll_conn_send_pong (noPollConn * conn, long length, noPollPtr content)
{
	if (conn == NULL)
		return nopoll_false; /* do not pong if wrong reference received */

	return nopoll_conn_send_frame (conn, nopoll_true, conn->role == NOPOLL_ROLE_CLIENT, NOPOLL_PONG_FRAME, length, content, 0);
}

int __nopoll_conn_complete_pending_write_reduce_header (noPollConn * conn, int bytes_written)
{
        /* now check if the last write operation includes
	   headers added by noPoll, to remove that size from
	   reported value. Reported value should only include
	   in its counting bytes requested by the upper level
	   application not upper level application plus bytes
	   added by noPoll */
        while (conn->pending_write_added_header > 0 && bytes_written > 0) {
	        bytes_written --;
		conn->pending_write_added_header--;
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Reduced added header (bytes_written=%d, conn->pending_write_added_header=%d)", 
			    bytes_written, conn->pending_write_added_header);
	} /* end if */
	return bytes_written;
}

/** 
 * @brief Allows to call to complete last pending write process that may be
 * pending from a previous uncompleted write operation. The function
 * returns the number of bytes that were written.
 *
 * @param conn The connection where the pending write operation
 * operation will take place. In the case conn == NULL is received, 0
 * is returned. Keep in mind this.
 *
 * @return In the case no pending write is in place, the function
 * returns 0. Otherwise, the function returns the pending bytes that
 * were written. The function returns -1 in the case of failure.
 *
 * You can call this function as many times as you want until you get
 * a 0. You can view it as a flush operation.
 */
int nopoll_conn_complete_pending_write (noPollConn * conn)
{
	int    bytes_written = 0;

	if (conn == NULL || conn->pending_write == NULL)
		return 0;

	/* simple implementation */
	bytes_written = conn->send (conn, conn->pending_write + conn->pending_write_desp, conn->pending_write_bytes);
	if (bytes_written == conn->pending_write_bytes) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Completed pending write operation with bytes=%d", bytes_written);
		nopoll_free (conn->pending_write);
		conn->pending_write = NULL;

		/* reduce/remove bytes written due to header */
		return __nopoll_conn_complete_pending_write_reduce_header (conn, bytes_written);
	} /* end if */

	if (bytes_written > 0) {
		/* bytes written but not everything */
		conn->pending_write_bytes -= bytes_written;
		conn->pending_write_desp += bytes_written;

		/* reduce/remove bytes written due to header */
		return __nopoll_conn_complete_pending_write_reduce_header (conn, bytes_written);
	}

	nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Found complete write operation didn't finish well, result=%d, errno=%d, conn-id=%d",
		    bytes_written, errno, conn->id);
	return bytes_written;
}

/** 
 * @brief Allows to check if there are pending write bytes. The
 * function returns the number of pending write bytes that are waiting
 * to be flushed. To do so you must call \ref nopoll_conn_complete_pending_write.
 *
 * @param conn The connection to be checked to have pending bytes to be written.
 *
 * @return The number of bytes pending to be written. The function
 * also returns 0 when conn reference received is NULL.
 */
int           nopoll_conn_pending_write_bytes (noPollConn * conn)
{
	if (conn == NULL || conn->pending_write == NULL)
		return 0;

	return conn->pending_write_bytes;
}

/** 
 * @brief Ready to use function that checks for pending write
 * operations and flush them waiting until they are done or until the
 * timeout provided by the user is reached.
 *
 * This function uses \ref nopoll_conn_pending_write_bytes and \ref
 * nopoll_conn_complete_pending_write to check and complete pending
 * write operations. 
 *
 * Because writing pending bytes is a common operation, this function
 * is provided as a ready to use function to call after a write operation (for
 * example \ref nopoll_conn_send_text).
 *
 * @param conn The connection where pending bytes must be written. 
 *
 * @param timeout Timeout in microseconds to limit the flush operation.
 *
 * @param previous_result Optional parameter that can receive the
 * number of bytes optionally read before this call. The value
 * received on this function will be added to the result, checking
 * first it contains a value higher than 0. This is an option to
 * clarify the interface. If you don't have the value to be passed to
 * this function at the time needed, just pass 0.
 *
 * @return Bytes that were written. If no pending bytes must be
 * written, the function returns 0. The function returns bytes written
 * or bytes written plus @previous_result if previous_result is > 0.
 */
int nopoll_conn_flush_writes (noPollConn * conn, long timeout, int previous_result)
{
	int iterator = 0;
	int bytes_written;
	int total = 0;
	int multiplier = 1;
	long wait_implemented = 0;

	/* check for errno and pending write operations */
	if ((errno != NOPOLL_EWOULDBLOCK && errno != NOPOLL_EINPROGRESS) && (nopoll_conn_pending_write_bytes (conn) == 0)) {
	        nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "called flush but nothing is pending=%d or errno=%d isn't %d",
		            nopoll_conn_pending_write_bytes (conn), errno, NOPOLL_EWOULDBLOCK);
		return previous_result > 0 ? previous_result : 0;
	} 
		
	while (iterator < 100 && nopoll_conn_pending_write_bytes (conn) > 0) {

		/* stop operation if timeout reached */
		if (wait_implemented >= timeout) 
			break;

		nopoll_sleep (100000 * multiplier);
		wait_implemented += (100000 * multiplier);

		/* write content pending */
		bytes_written = nopoll_conn_complete_pending_write (conn);

		if (bytes_written > 0) 
			total += bytes_written;

		/* next position */
		iterator++;
		multiplier++;
	} /* end while */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "finishing flush operation, total written=%d, added to previous result=%d, errno=%d",
		    total, previous_result, errno); 

	/* add value received */
	if (previous_result > 0) 
		return total + previous_result;

	/* just bytes written */
	return total;
}


/** 
 * @internal Function used to send a frame over the provided
 * connection.
 *
 * @param conn The connection where the send operation will hapen.
 *
 * @param fin If the frame to be sent must be flagged as a fin frame.
 *
 * @param masked The frame to be sent is masked or not.
 *
 * @param op_code The frame op code to be configured.
 *
 * @param length The frame payload length.
 *
 * @param content Pointer to the data to be sent in the frame.
 *
 * @return The function returns the number of bytes sent, being @length the 
 * max amount of bytes that can be reported as sent by
 * this funciton. This means value reported by this function do not
 * includes headers.  The funciton also returns the following general indications:
 *
 *   N : number of bytes sent (user land bytes sent, without including web socket headers).
 *   0 : no bytes sent (see errno indication). See also \ref nopoll_conn_complete_pending_write
 *  -1 : failure found
 *  -2 : retry operation needed (NOPOLL_EWOULDBLOCK)
 *
 */
int nopoll_conn_send_frame (noPollConn * conn, nopoll_bool fin, nopoll_bool masked,
			    noPollOpCode op_code, long length, noPollPtr content, long sleep_in_header)

{
	char               header[14];
	int                header_size;
	char             * send_buffer;
	int                bytes_written = 0;
	int                bytes_sent    = 0;
	char               mask[4];
	unsigned int       mask_value = 0;
	int                desp = 0;
	int                tries;
#if defined(SHOW_DEBUG_LOG)
	noPollDebugLevel   level;
#endif

	/* check for pending send operation */
	bytes_written = nopoll_conn_complete_pending_write (conn);
	if (bytes_written < 0)
		return bytes_written;

	/* clear header */
	memset (header, 0, 14);

	/* set header codes */
	if (fin) 
		nopoll_set_bit (header, 7);
	
	if (masked) {
		nopoll_set_bit (header + 1, 7);
		
		/* define a random mask */
#if defined(NOPOLL_OS_WIN32)
		mask_value = (unsigned int) rand ();
#else
		mask_value = (unsigned int) random ();
#endif
		memset (mask, 0, 4);
		nopoll_set_32bit (mask_value, mask);
	} /* end if */

	if (op_code) {
		/* set initial 4 bits */
		header[0]   |= op_code & 0x0f;
	}

	/* set default header size */
	header_size  = 2;

	/* according to message length */
	if (length < 126) {
		header[1] |= length;
	} else if (length <= 65535) {
		/* set the next header length is at least 65535 */
		header[1] |= 126;
		header_size += 2;
		/* set length into the next bytes */
		nopoll_set_16bit (length, header + 2);
#if defined(NOPOLL_64BIT_PLATFORM)
	} else if (length < 0x8000000000000000) {
		header[2] = (length & 0xFF00000000000000) >> 56;
		header[3] = (length & 0x00FF000000000000) >> 48;
		header[4] = (length & 0x0000FF0000000000) >> 40;
		header[5] = (length & 0x000000FF00000000) >> 32;
#else
	} else if (length < 0x80000000) {
		header[2] = header[3] = header[4] = header[5] = 0;
#endif
		header[1] |= 127;
		header_size += 8;
		header[6] = (length & 0x00000000FF000000) >> 24;
		header[7] = (length & 0x0000000000FF0000) >> 16;
		header[8] = (length & 0x000000000000FF00) >> 8;
		header[9] = (length & 0x00000000000000FF);
	} else {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to send the requested message, this requested is bigger than the value that can be supported by this platform");
		return -1;
	}

	/* place mask */
	if (masked) {
		nopoll_set_32bit (mask_value, header + header_size);
		header_size += 4;
	} /* end if */

	/* allocate enough memory to send content */
	send_buffer = nopoll_new (char, length + header_size + 2);
	if (send_buffer == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to allocate memory to implement send operation");
		return -1;
	} /* end if */
	
	/* copy content to be sent */
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Copying into the buffer %d bytes of header (total memory allocated: %d)", 
		    header_size, (int) length + header_size + 1);
	memcpy (send_buffer, header, header_size);
	if (length > 0) {
		memcpy (send_buffer + header_size, content, length);

		/* mask content before sending if requested */
		if (masked) {
			nopoll_conn_mask_content (conn->ctx, send_buffer + header_size, length, mask, 0);
		}
	} /* end if */

	
	/* send content */
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Mask used for this delivery: %d (about to send %d bytes)",
		    nopoll_get_32bit (send_buffer + header_size - 2), (int) length + header_size);

	/* clear errno status before writting */
	desp  = 0;
	tries = 0;

	/***** BEGIN INTERNAL debug code for test_30, test_31, test_32, test_33, test_34, test_35 : nopoll-regression-client.c ******/
	if ((conn->__force_stop_after_header > 0) && (conn->__force_stop_after_header < (length + header_size))) {
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Sending broken header (just %d bytes) and implement a pause on purpose...", conn->__force_stop_after_header);

		/* send just 2 bytes for the header and then implement a very long pause */
		bytes_written = conn->send (conn, send_buffer, conn->__force_stop_after_header);
		desp          = conn->__force_stop_after_header;
		if (bytes_written != conn->__force_stop_after_header) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Requested to write %d bytes for the header but %d were written",
				    conn->__force_stop_after_header, bytes_written);
			desp  = 0;
		} /* end if */

		/* sleep after header ... */
		nopoll_sleep (5000000); /* 5 seconds */

	} /* end if */
	/****** END INTERNAL debug code for test_30 : nopoll-regression-client.c ******/

	while (nopoll_true) {
		/* try to write bytes */
		if (sleep_in_header == 0) {
			bytes_written = conn->send (conn, send_buffer + desp, length + header_size - desp);
		} else {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Found sleep in header indication, sending header: %d bytes (waiting %ld)", header_size, sleep_in_header);
			bytes_written = conn->send (conn, send_buffer, header_size);
			if (bytes_written == header_size) {
				/* sleep after header ... */
				nopoll_sleep (sleep_in_header);
				
				/* now send the rest of the content (without the header) */
				bytes_written = conn->send (conn, send_buffer + header_size, length);
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Rest of content written %d (header size: %d, length: %d)", 
					    bytes_written, header_size, length);
				bytes_written = length + header_size;
				nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "final bytes_written %d", bytes_written);
			} else {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Requested to write %d bytes for the header but %d were written",
					    header_size, bytes_written);
				return -1;
			} /* end if */
		} /* end if */
		
		if ((bytes_written + desp) != (length + header_size)) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, 
				    "Requested to write %d bytes but found %d written (masked? %d, mask: %u, header size: %d, length: %d), errno = %d : %s", 
				    (int) length + header_size - desp, bytes_written, masked, mask_value, header_size, (int) length, errno, strerror (errno));
		} else {
			/* accomulate bytes written to continue */
			if (bytes_written > 0)
				desp += bytes_written;

			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Bytes written to the wire %d (masked? %d, mask: %u, header size: %d, length: %d)", 
				    bytes_written, masked, mask_value, header_size, (int) length);
			break;
		} /* end if */

		/* accomulate bytes written to continue */
		if (bytes_written > 0)
			desp += bytes_written;

		/* increase tries */
		tries++;

		if ((errno != 0) || tries > 50) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Found errno=%d (%s) value while trying to bytes to the WebSocket conn-id=%d or max tries reached=%d",
				    errno, strerror (errno), conn->id, tries);
			break;
		} /* end if */

		/* wait a bit */
		nopoll_sleep (100000);

	} /* end while */

	/* record pending write bytes */
	conn->pending_write_bytes = length + header_size - desp;

	/* record the header to be accurate when reporting the amount
	   of bytes written: we have to avoid confusing two things:

	   - Bytes written by the protocol (length + headers)
	   - Bytes requested by the upper level application to be written (just length)
	   
	   By recording the following value, we try to make
	   nopoll_conn_complete_pending_write to report amount of
	   upper level bytes written (without headers, which is
	   something created by noPoll and not requested by the upper
	   level application).
	*/
	conn->pending_write_added_header = header_size;

	/* record and report useful userland payload's bytes sent  */
	bytes_sent = 0;
	if ((desp - header_size) > 0) {
	        bytes_sent = (desp - header_size);

		/* make conn->pending_write_added_header 0 because it
		   have written enought bytes including the header */
		conn->pending_write_added_header = 0;
	} /* end if */
	
#if defined(SHOW_DEBUG_LOG)
	level = NOPOLL_LEVEL_DEBUG;
	if (desp != (length + header_size))
		level = NOPOLL_LEVEL_CRITICAL;
	else if (errno == NOPOLL_EWOULDBLOCK && conn->pending_write_bytes > 0)
		level = NOPOLL_LEVEL_WARNING;

	nopoll_log (conn->ctx, level, 
		    "Write operation finished with last result=%d (bytes_written), bytes-sent=%d, desp=%d, header_size=%d, requested=%d (length), remaining=%d (conn->pending_write_bytes), errno=%d (conn-id=%d)",
		    /* report want we are going to report: result */
		    bytes_written,
		    /* bytes sent */
		    bytes_sent, desp, header_size,
		    length, conn->pending_write_bytes, errno, conn->id);
#endif

	/* check pending bytes for the next operation */
	if (conn->pending_write_bytes > 0) {
		conn->pending_write = send_buffer;
		conn->pending_write_desp = desp;
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Stored %d bytes starting from %d out of %d bytes (header size: %d)", 
			    conn->pending_write_bytes, desp, length + header_size, header_size);
	} else {
		/* release memory */
		nopoll_free (send_buffer);
	} /* end if */

	/* if no byte was sent and errno is set to non-blocking error
	   operation that indicates a retry, report -2 */
	if (bytes_sent == 0 && errno == NOPOLL_EWOULDBLOCK) 
	        return -2;

	/* report what was was written (which can be everything, part,
	   anything or error) */
	return bytes_sent;
}

/** 
 * @brief Allows to accept a new incoming WebSocket connection on the
 * provided listener.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param listener The WebSocket listener that is receiving a new incoming connection.
 *
 * @return A newly created \ref noPollConn reference or NULL if it
 * fails.
 */
noPollConn * nopoll_conn_accept (noPollCtx * ctx, noPollConn * listener)
{
	NOPOLL_SOCKET   session;

	nopoll_return_val_if_fail (ctx, ctx && listener, NULL);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Calling to accept web socket connection over master id=%d, socket=%d",
		    listener->id, listener->session);

	/* recevied a new connection: accept the
	 * connection and ask the app level to accept
	 * or not */
	session = nopoll_listener_accept (listener->session);
	if (session == NOPOLL_INVALID_SOCKET) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received invalid socket value from accept(2): %d, error code errno=: %d", 
			    session, errno);
		return NULL;
	} /* end if */

	return nopoll_conn_accept_socket (ctx, listener, session);
}


/** 
 * @brief Allows to accept a new incoming WebSocket connection on the
 * provided listener but with a socket already accepted.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param listener The WebSocket listener that is receiving a new incoming connection.
 *
 * @param session An already accepted socket from the provided
 * listener.
 *
 * @return A newly created \ref noPollConn reference or NULL if it
 * fails.
 */
noPollConn * nopoll_conn_accept_socket (noPollCtx * ctx, noPollConn * listener, NOPOLL_SOCKET session)
{
	noPollConn * conn;

	nopoll_return_val_if_fail (ctx, ctx && listener, NULL);

	/* create the connection */
	conn = nopoll_listener_from_socket (ctx, session);
	if (conn == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Received NULL pointer after calling to create listener from session..");
		return NULL;
	} /* end if */

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Accepted new WebSocket conn-id=%d, socket=%d, over master id=%d, socket=%d",
		    conn->id, conn->session, listener->id, listener->session);

	/* configure the listener reference that accepted this
	 * connection */
	conn->listener = listener;

	if (! nopoll_conn_accept_complete (ctx, listener, conn, session, listener->tls_on))
		return NULL;

	/* report listener created */
	return conn;
}

/**
 * @internal Function to support accept listener operations.
 */
nopoll_bool __nopoll_conn_accept_complete_common (noPollCtx * ctx, noPollConnOpts * options, noPollConn * listener, noPollConn * conn, NOPOLL_SOCKET session, nopoll_bool tls_on) {

	const char          * certificateFile  = NULL;
	const char          * privateKey       = NULL;
	const char          * chainCertificate = NULL;
	const char          * serverName       = NULL;

	/* check input parameters */
	if (! (ctx && listener && conn && session != NOPOLL_INVALID_SOCKET)) {
		nopoll_conn_shutdown (conn);
		nopoll_ctx_unregister_conn (ctx, conn);

		/* release connection options */
		__nopoll_conn_opts_release_if_needed (options);

		return nopoll_false;
	} /* end if */

	/* configure non blocking mode */
	nopoll_conn_set_sock_block (session, nopoll_true);
	
	/* now check for accept handler */
	if (ctx->on_accept) {
		/* call to on accept */
		if (! ctx->on_accept (ctx, conn, ctx->on_accept_data)) {
			nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "Application level denied accepting connection from %s:%s, closing", 
				    conn->host, conn->port);
			nopoll_conn_shutdown (conn);
			nopoll_ctx_unregister_conn (ctx, conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		} /* end if */
	} /* end if */

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Connection received and accepted from %s:%s (conn refs: %d, ctx refs: %d)", 
		    listener->host, listener->port, listener->refs, ctx->refs);

	if (listener->tls_on || tls_on) {
		/* reached this point, ensure tls is enabled on this
		 * session */
		conn->tls_on = nopoll_true;

		/* get here SNI to query about the serverName */

		/* 1) GET FROM OPTIONS: detect here if we have
		 * certificates provided through options */
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Starting TLS process, options=%p, listener=%p", options, listener);

		if (options) {
			certificateFile = options->certificate;
			privateKey      = options->private_key;
		} /* end if */
		if (certificateFile == NULL || privateKey == NULL) {

			/* 2) GET FROM LISTENER: get references to currently configured certificate file */
			certificateFile = listener->certificate;
			privateKey      = listener->private_key;
			if (certificateFile == NULL || privateKey == NULL) {
				/* 3) GET FROM STORE: check if the
				 * certificate is already installed */
				nopoll_ctx_find_certificate (ctx, serverName, &certificateFile, &privateKey, &chainCertificate);
			}
		} /* end if */

		/* check certificates and private key */
		if (certificateFile == NULL || privateKey == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to accept secure web socket connection, certificate file %s and/or key file %s isn't defined",
				    certificateFile ? certificateFile : "<not defined>", 
				    privateKey ? privateKey : "<not defined>");
			nopoll_conn_shutdown (conn);
			nopoll_ctx_unregister_conn (ctx, conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		} /* end if */

		/* init ssl ciphers and engines */
		if (! __nopoll_tls_was_init) {
			__nopoll_tls_was_init = nopoll_true;
			SSL_library_init ();
		} /* end if */

		/* now configure chainCertificate */
		if (listener->chain_certificate) 
			chainCertificate = listener->chain_certificate;
		else if (options && options->chain_certificate)
			chainCertificate = options->chain_certificate;

		/* create ssl context */
		conn->ssl_ctx  = __nopoll_conn_get_ssl_context (ctx, conn, listener->opts, nopoll_false);
		if (conn->ssl_ctx == NULL) {
		        nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to get Ssl Context (conn->ssl_ctx = __nopoll_conn_get_ssl_context), function returned NULL");
			return nopoll_false;
		} /* end if */

		/* Configure ca certificate in the case it is defined */
		if (options && options->ca_certificate) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting up CA certificate: %s", options->ca_certificate);
			if (SSL_CTX_load_verify_locations (conn->ssl_ctx, options->ca_certificate, NULL) != 1) {
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure CA certificate (%s), SSL_CTX_load_verify_locations () failed", options->ca_certificate);
				return nopoll_false;
			} /* end if */

		} /* end if */

		/* enable default verification paths */
		if (SSL_CTX_set_default_verify_paths (conn->ssl_ctx) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to configure default verification paths, SSL_CTX_set_default_verify_paths () failed");
			return nopoll_false;
		} /* end if */

		/* configure chain certificate */
		if (chainCertificate) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Setting up chain certificate: %s", chainCertificate);
			if (SSL_CTX_use_certificate_chain_file (conn->ssl_ctx, chainCertificate) != 1) {
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to configure chain certificate (%s), SSL_CTX_use_certificate_chain_file () failed", chainCertificate);
				return nopoll_false;
			} /* end if */
		} /* end if */

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Using certificate file: %s (with ssl context ref: %p)", certificateFile, conn->ssl_ctx);
		if (conn->ssl_ctx == NULL || SSL_CTX_use_certificate_chain_file (conn->ssl_ctx, certificateFile) != 1) {
			/* drop an error log */
			if (conn->ssl_ctx == NULL)
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to accept incoming connection, failed to create SSL context. Context creator returned NULL pointer");
			else 
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "there was an error while setting certificate file into the SSl context, unable to start TLS profile. Failure found at SSL_CTX_use_certificate_file function. Tried certificate file: %s", 
					    certificateFile);

			/* dump error stack */
			nopoll_conn_shutdown (conn);
			nopoll_ctx_unregister_conn (ctx, conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		} /* end if */

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Using certificate key: %s", privateKey);
		if (SSL_CTX_use_PrivateKey_file (conn->ssl_ctx, privateKey, SSL_FILETYPE_PEM) != 1) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, 
				    "there was an error while setting private file into the SSl context, unable to start TLS profile. Failure found at SSL_CTX_use_PrivateKey_file function. Tried private file: %s", 
				    privateKey);
			/* dump error stack */
			nopoll_conn_shutdown (conn);
			nopoll_ctx_unregister_conn (ctx, conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		}

		/* check for private key and certificate file to match. */
		if (! SSL_CTX_check_private_key (conn->ssl_ctx)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, 
				    "seems that certificate file and private key doesn't match!, unable to start TLS profile. Failure found at SSL_CTX_check_private_key function. Used certificate %s, and key: %s",
				    certificateFile, privateKey);
			/* dump error stack */
			nopoll_conn_shutdown (conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		} /* end if */

		if (options != NULL && ! options->disable_ssl_verify) {
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Enabling certificate client peer verification from server");
			/** really, really ugly hack to let
			 * __nopoll_conn_ssl_verify_callback to be able to get
			 * access to the context required to drop some logs */
			__nopoll_conn_ssl_ctx_debug = ctx;
			SSL_CTX_set_verify (conn->ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, __nopoll_conn_ssl_verify_callback); 
			SSL_CTX_set_verify_depth (conn->ssl_ctx, 5);
		} /* end if */


		/* create SSL context */
		conn->ssl = SSL_new (conn->ssl_ctx);       
		if (conn->ssl == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "error while creating TLS transport, SSL_new (%p) returned NULL", conn->ssl_ctx);
			nopoll_conn_shutdown (conn);
			nopoll_ctx_unregister_conn (ctx, conn);

			/* release connection options */
			__nopoll_conn_opts_release_if_needed (options);

			return nopoll_false;
		} /* end if */

		/* set the file descriptor */
		SSL_set_fd (conn->ssl, conn->session);

		/* don't complete here the operation but flag it as
		 * pending */
		conn->pending_ssl_accept = nopoll_true;
		nopoll_conn_set_sock_block (conn->session, nopoll_false);

		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Prepared TLS session to be activated on next reads (conn id %d)", conn->id);
		
	} /* end if */

	/* release connection options */
	__nopoll_conn_opts_release_if_needed (options);

	return nopoll_true;
}

/** 
 * @brief Allows to complete accept operation by setting up all I/O
 * handlers required to make the WebSocket connection to work.
 *
 * @param ctx The context where the operation takes place.
 *
 * @param listener The listener where the connection was accepted.
 *
 * @param conn The connection that was accepted.
 *
 * @param session The socket associated to the listener accepted.
 *
 * @param tls_on A boolean indication if the TLS interface should be
 * enabled or not.
 *
 * @return nopoll_true if the listener was accepted otherwise nopoll_false is returned.
 */
nopoll_bool nopoll_conn_accept_complete (noPollCtx * ctx, noPollConn * listener, noPollConn * conn, NOPOLL_SOCKET session, nopoll_bool tls_on) {

	if (listener->opts) {
		if (! nopoll_conn_opts_ref (listener->opts)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire a reference to the connection option at nopoll_conn_accept_complete() function nopoll_conn_opts_ref () failed..");
			return nopoll_false;
		} /* end if */
	} /* end if */

	return __nopoll_conn_accept_complete_common (ctx, listener->opts, listener, conn, session, tls_on);
}

/** 
 * @brief Allows to implement a wait operation until the provided
 * connection is ready or the provided timeout is reached.
 *
 * @param conn The connection that is being waited to be created.
 *
 * @param timeout The timeout operation to limit the wait
 * operation. Timeout is provided in seconds.
 *
 * @return The function returns when the timeout was reached or the
 * connection is ready. In the case the connection is ready when the
 * function finished nopoll_true is returned, otherwise nopoll_false.
 */
nopoll_bool      nopoll_conn_wait_until_connection_ready (noPollConn * conn,
							  int          timeout)
{
	long int total_timeout = timeout * 1000000;

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn) && total_timeout > 0) {

		/* check if the connection is ok */
		if (! nopoll_conn_is_ok (conn)) 
			return nopoll_false;

		/* wait a bit 0,5ms */
		nopoll_sleep (500);

		/* reduce the amount of time we have to wait */
		total_timeout = total_timeout - 500;
	} /* end if */

	/* report if the connection is ok */
	return nopoll_conn_is_ok (conn) && nopoll_conn_is_ready (conn);
}

/* @} */

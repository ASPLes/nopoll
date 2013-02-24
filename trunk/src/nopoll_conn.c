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
#include <nopoll_conn.h>
#include <nopoll_private.h>

#if defined(NOPOLL_OS_UNIX)
# include <netinet/tcp.h>
#endif

#if defined(NOPOLL_OS_WIN32)
int      __nopoll_win32_blocking_socket_set (NOPOLL_SOCKET socket,
                                             int           status) 
{
        unsigned long enable = status;

	return (ioctlsocket (socket, FIONBIO, &enable) == 0);
}
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
		if (!__nopoll_win32_blocking_enable (socket)) {
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
		if (!__nopoll_win32_nonblocking_enable (socket)) {
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
	if (result < 0) {
		return nopoll_false;
	}

	/* properly configured */
	return nopoll_true;
} /* end */

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
NOPOLL_SOCKET nopoll_conn_sock_connect (noPollCtx   * ctx,
					const char  * host,
					const char  * port)
{
	struct hostent     * hostent;
	struct sockaddr_in   saddr;
	NOPOLL_SOCKET        session;

	/* resolve hosting name */
	hostent = gethostbyname (host);
	if (hostent == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "unable to resolve host name %s", host);
		return -1;
	} /* end if */

	/* create the socket and check if it */
	session      = socket (AF_INET, SOCK_STREAM, 0);
	if (session == NOPOLL_INVALID_SOCKET) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to create socket");
		return -1;
	} /* end if */

	/* disable nagle */
	nopoll_conn_set_sock_tcp_nodelay (session, nopoll_true);

	/* prepare socket configuration to operate using TCP/IP
	 * socket */
        memset(&saddr, 0, sizeof(saddr));
	saddr.sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;
        saddr.sin_family    = AF_INET;
        saddr.sin_port      = htons((uint16_t) strtod (port, NULL));

	/* set non blocking status */
	nopoll_conn_set_sock_block (session, nopoll_false);
	
	/* do a tcp connect */
        if (connect (session, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		if(errno != NOPOLL_EINPROGRESS && errno != NOPOLL_EWOULDBLOCK) { 
			shutdown (session, SHUT_RDWR);
			nopoll_close_socket (session);
			nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "unable to connect to remote host %s:%s errno=%d",
				    host, port);
			return -1;
		} /* end if */
	} /* end if */

	/* return socket created */
	return session;
}




/** 
 * @internal Function that builds the client init greetings that will
 * be send to the server according to registered implementation.
 */ 
char * __nopoll_conn_get_client_init (noPollConn * conn)
{
	/* build sec-websocket-key */
	char key[50];
	int  key_size = 50;
	char nonce[17];

	/* get the nonce */
	if (! nopoll_nonce (nonce, 16)) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to get nonce, unable to produce Sec-WebSocket-Key.");
		return nopoll_false;
	} /* end if */

	/* now base 64 */
	if (! nopoll_base64_encode (nonce, 16, key, &key_size)) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to base 64 encode characters for Sec-WebSocket-Key");
		return NULL;
	}
	
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Created Sec-WebSocket-Key nonce: %s", key);

	/* create accept and store */
	conn->handshake = nopoll_new (noPollHandShake, 1);
	conn->handshake->expected_accept = strdup (key);

	/* send initial handshake */
	return nopoll_strdup_printf ("GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nOrigin: %s\r\n%s%s%s%sSec-WebSocket-Version: 13\r\n\r\n", 
				     conn->get_url, conn->host_name, 
				     /* sec-websocket-key */
				     key,
				     /* origin */
				     conn->origin, 
				     /* protocol part */
				     conn->protocols ? "Sec-WebSocket-Protocol" : "",
				     conn->protocols ? ": " : "",
				     conn->protocols ? conn->protocols : "",
				     conn->protocols ? "\r\n" : "");
}


/** 
 * @brief Creates a new Websocket connection to the provided
 * destination, physically located at host_ip and host_port.
 *
 * @param ctx The Nopoll context to which this new connection will be associated.
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
 */
noPollConn * nopoll_conn_new (noPollCtx  * ctx,
			      const char * host_ip, 
			      const char * host_port, 
			      const char * host_name,
			      const char * get_url, 
			      const char * protocols,
			      const char * origin)
{
	noPollConn     * conn;
	NOPOLL_SOCKET    session;
	char           * content;
	int              size;

	nopoll_return_val_if_fail (ctx, ctx && host_ip, NULL);

	/* set default connection port */
	if (host_port == NULL)
		host_port = "80";

	/* create socket connection in a non block manner */
	session = nopoll_conn_sock_connect (ctx, host_ip, host_port);
	if (session == -1) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to connect to remote host %s:%s", host_ip, host_port);
		return NULL;
	} /* end if */

	/* create the connection */
	conn = nopoll_new (noPollConn, 1);
	if (conn == NULL) 
		return NULL;
	conn->refs = 1;

	/* register connection into context */
	nopoll_ctx_register_conn (ctx, conn);
	
	/* configure context */
	conn->ctx     = ctx;
	conn->session = session;
	conn->role    = NOPOLL_ROLE_CLIENT;

	/* record host and port */
	conn->host    = strdup (host_ip);
	conn->port    = strdup (host_port);

	/* configure default handlers */
	conn->receive = nopoll_conn_default_receive;
	conn->send    = nopoll_conn_default_send;

	/* build host name */
	if (host_name == NULL)
		conn->host_name = strdup (host_ip);
	else
		conn->host_name = strdup (host_name);

	/* build origin */
	if (origin == NULL)
		conn->origin = nopoll_strdup_printf ("http://%s", conn->host_name);
	else
		conn->origin = strdup (origin);

	/* get url */
	if (get_url == NULL)
		conn->get_url = strdup ("/");
	else
		conn->get_url = strdup (get_url);

	/* protocols */
	if (protocols != NULL)
		conn->protocols = strdup (protocols);


	/* get client init payload */
	content = __nopoll_conn_get_client_init (conn);
	
	if (content == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to build client init message, unable to connect");
		nopoll_conn_shutdown (conn);
		return NULL;
	}
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sending websocket client init: %s", content);
	size = strlen (content);

	/* call to send content */
	if (size != conn->send (conn, content, size)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to send websocket init message, error code was: %d, closing session", errno);
		nopoll_conn_shutdown (conn);
		conn = NULL;
	} /* end if */

	/* release content */
	nopoll_free (content);

	/* return connection created */
	return conn;
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

	/* acquire here the mutex */
	if (conn->refs <= 0) {
		/* release here the mutex */
		return nopoll_false;
	}
	
	conn->refs++;

	/* release here the mutex */

	return nopoll_true;
}

/** 
 * @brief Allows to check if the provided connection is in connected
 * state. This is different to be ready to send and receive content
 * because the session needs to be first established. You can use \ref
 * nopoll_conn_is_ready to ensure the connection is ok.
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
	return conn->session > 0;
}

/** 
 * @brief Allows to check if the connection is ready to be used
 * (handshake completed).
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
	if (conn->session < 0)
		return nopoll_false;
	if (! conn->handshake_ok) {
		/* acquire here handshake mutex */

		/* complete handshake */
		nopoll_conn_complete_handshake (conn);

		/* release here handshake mutex */
	}
	return conn->handshake_ok;
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
 * @brief Call to close the connection immediately without going
 * through websocket close negotiation.
 *
 * @param conn The connection to be shutted down.
 */
void          nopoll_conn_shutdown (noPollConn * conn)
{
	if (conn == NULL)
		return;

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "shutting down connection id=%d (session %d)", conn->id, conn->session);

	/* shutdown connection here */
	nopoll_close_socket (conn->session);
	conn->session = -1;

	return;
}

/** 
 * @brief Allows to close an opened \ref noPollConn no matter its role
 * (\ref noPollRole).
 *
 * @param conn The connection to close.
 */ 
void          nopoll_conn_close  (noPollConn  * conn)
{
	/* check input data */
	if (conn == NULL)
		return;

	if (conn->session) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "requested proper connection close id=%d (session %d)", conn->id, conn->session);

		/* send close message */

		/* call to shutdown connection and release memory */
		nopoll_close_socket (conn->session);
		conn->session = -1;
	} /* end if */

	/* unregister connection from context */
	nopoll_ctx_unregister_conn (conn->ctx, conn);

	/* call to unref connection */
	nopoll_conn_unref (conn);

	return;
}

/** 
 * @brief Allows to unref connection reference acquired via \ref
 * nopoll_conn_ref.
 *
 * @param conn The connection to be unrefered.
 */
void nopoll_conn_unref (noPollConn * conn)
{
	if (conn == NULL)
		return;

	/* acquire here the mutex */
	conn->refs--;
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Releasing connection id %d reference, current ref count status is: %d", 
		    conn->id, conn->refs);
	if (conn->refs != 0) {
		/* release here the mutex */
		return;
	}
	/* release here the mutex */

	/* release message */
	if (conn->pending_msg)
		nopoll_msg_unref (conn->pending_msg);

	/* release ctx */
	nopoll_ctx_unref (conn->ctx);
	conn->ctx = NULL;

	/* free all internal strings */
	nopoll_free (conn->host);
	nopoll_free (conn->port);
	nopoll_free (conn->host_name);
	nopoll_free (conn->origin);
	nopoll_free (conn->protocols);
	nopoll_free (conn->get_url);

	/* release handshake internal data */
	if (conn->handshake) {
		nopoll_free (conn->handshake->websocket_key);
		nopoll_free (conn->handshake->websocket_version);
		nopoll_free (conn->handshake->websocket_accept);
		nopoll_free (conn->handshake->expected_accept);
		nopoll_free (conn->handshake);
	} /* end if */

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
	noPollCtx * ctx = conn->ctx;

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
						conn->pending_line = strdup (buffer);
					} /* end if */
				} /* end if */
				return (-2);
			}
			
			/* if the conn is closed, just return
			 * without logging a message */
			if (nopoll_conn_is_ok (conn)) {
				nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to read a line, error code errno: %d", errno);
			}
			return (-1);
		}
	}
	*ptr = 0;
	return (n + desp);

}

/** 
 * @internal Function used to read bytes from the wire..
 */
int         nopoll_conn_receive  (noPollConn * conn, char  * buffer, int  maxlen)
{
	int         nread;

	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Calling to get %d from connection..", maxlen); */

 keep_reading:
	/* clear buffer */
	/* memset (buffer, 0, maxlen * sizeof (char )); */
	if ((nread = conn->receive (conn, buffer, maxlen)) == NOPOLL_SOCKET_ERROR) {
		/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, " returning errno=%d (%s)", errno, strerror (errno)); */
		if (errno == NOPOLL_EAGAIN) 
			return 0;
		if (errno == NOPOLL_EWOULDBLOCK) 
			return 0;
		if (errno == NOPOLL_EINTR) 
			goto keep_reading;
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "unable to readn=%d, error code was: %d (shutting down connection)", maxlen, errno);
		nopoll_conn_shutdown (conn);
		return -1;
	}

	/* nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, " returning bytes read = %d", nread); */
	if (nread == 0) {
		nopoll_conn_shutdown (conn);
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "received connection close while reading from conn id %d, shutting down connection..", conn->id);
	} /* end if */

	/* ensure we don't access outside the array */
	if (nread < 0) 
		nread = 0;

	buffer[nread] = 0;
	return nread;
}

nopoll_bool nopoll_conn_get_http_url (noPollConn * conn, const char * buffer, int buffer_size, char * method, char ** url)
{
	int          iterator;
	int          iterator2;
	noPollCtx  * ctx = conn->ctx;

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

	/* nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Processing content: %s", buffer); */
	
	/* ok, find the first : */
	while (iterator < buffer_size && buffer[iterator] && buffer[iterator] != ':')
		iterator++;
	if (buffer[iterator] != ':') {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Expected to find mime header separator : but it wasn't found..");
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
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Expected to find mime header value end (13) but it wasn't found..");
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
	char      * static_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";	
	char      * accept_key;	
	int         accept_key_size;
	int         key_length;

	if (websocket_key == NULL)
		return NULL;

	key_length  = strlen (websocket_key);

	unsigned char   buffer[EVP_MAX_MD_SIZE];
	EVP_MD_CTX      mdctx;
	const EVP_MD  * md = NULL;
	unsigned int    md_len = EVP_MAX_MD_SIZE;

	accept_key_size = key_length + 36 + 1;
	accept_key      = nopoll_new (char, accept_key_size);

	memcpy (accept_key, websocket_key, key_length);
	memcpy (accept_key + key_length, static_guid, 36);
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "base key value: %s", accept_key);

	/* now sha-1 */
	md = EVP_sha1 ();
	EVP_DigestInit (&mdctx, md);
	EVP_DigestUpdate (&mdctx, accept_key, strlen (accept_key));
	EVP_DigestFinal (&mdctx, buffer, &md_len);

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sha-1 length is: %u", md_len);
	/* now convert into base64 */
	if (! nopoll_base64_encode ((const char *) buffer, md_len, (char *) accept_key, &accept_key_size)) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to base64 sec-websocket-key value..");
		return NULL;
	}

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Returning Sec-Websocket-Accept: %s", accept_key);
	
	return accept_key;
	
}

nopoll_bool nopoll_conn_complete_handshake_check_listener (noPollCtx * ctx, noPollConn * conn)
{
	char      * reply;
	int         reply_size;
	char      * accept_key;

	/* call to check listener handshake */
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Checking client handshake data..");

	/* ensure we have all minumum data */
	if (! conn->handshake->upgrade_websocket ||
	    ! conn->handshake->connection_upgrade ||
	    ! conn->handshake->websocket_key ||
	    ! conn->origin ||
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
	
	/* now call the user app level to accept the websocket
	   connection */
	if (ctx->on_open) {
		if (! ctx->on_open (ctx, conn, ctx->on_open_data)) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Client from %s:%s was denied by application level, clossing session", 
				    conn->host, conn->port);
			nopoll_conn_shutdown (conn);
			return nopoll_false;
		}
	} /* end if */
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Client from %s:%s was accepted, replying accept",
		    conn->host, conn->port);

	/* produce accept key */
	accept_key = nopoll_conn_produce_accept_key (ctx, conn->handshake->websocket_key);
	
	/* ok, send handshake reply */
	if (conn->protocols) {
		/* send accept header accepting protocol requested by the user */
		reply = nopoll_strdup_printf ("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nSec-WebSocket-Protocol: %s\r\n\r\n", 
					      accept_key, conn->protocols);
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

	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Sec-Websocket-Accept matches expected value..");

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
	if (nopoll_conn_check_mime_header_repeated (conn, header, value, "Sec-WebSocket-Protocol", conn->protocols)) 
		return 0;
	
	/* set the value if required */
	if (strcasecmp (header, "Sec-Websocket-Accept") == 0)
		conn->handshake->websocket_accept = value;
	else if (strcasecmp (header, "Sec-Websocket-Protocol") == 0)
		conn->protocols = value;
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
	char        buffer[1024];
	int         buffer_size;
	noPollCtx * ctx = conn->ctx;

	/* ensure we didn't complete handshake */
	if (conn->handshake_ok)
		return;

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Checking to complete connection id %d handshake, role %d", conn->id, conn->role);

	/* ensure handshake object is created */
	if (conn->handshake == NULL)
		conn->handshake = nopoll_new (noPollHandShake, 1);

	/* get lines and complete the handshake data */
	while (nopoll_true) {
		/* clear buffer for debugging functions */
		buffer[0] = 0;
		/* get next line to process */
		buffer_size = nopoll_conn_readline (conn, buffer, 1024);
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
			nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Called to handle connection handshake on a connection with an unexpected role: %d, closing session");
			nopoll_conn_shutdown (conn);
			return;
		}
	} /* end while */

	return;
}

void nopoll_conn_mask_content (noPollCtx * ctx, char * payload, int payload_size, char * mask)
{
	int iter       = 0;
	int mask_index = 0;

	while (iter < payload_size) {
		/* rotate mask and apply it */
		mask_index = iter % 4;
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
 * The function do not block.
 *
 * @param conn The connection where the read operation will take
 * place.
 * 
 * @return A reference to a noPollMsg object or NULL if there is
 * nothing available. In case the function returns NULL, check
 * connection status with \ref nopoll_conn_is_ok.
 */
noPollMsg   * nopoll_conn_get_msg (noPollConn * conn)
{
	char        buffer[20];
	int         bytes;
	noPollMsg * msg;

	if (conn == NULL)
		return NULL;
	
	/* check connection status */
	if (! conn->handshake_ok) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Connection id %d handshake is not complete, running..", conn->id);
		/* acquire here handshake mutex */

		/* complete handshake */
		nopoll_conn_complete_handshake (conn);

		/* release here handshake mutex */
		if (! conn->handshake_ok) 
			return NULL;
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

	/* get the first 4 bytes from the websocket header */
	bytes = nopoll_conn_receive (conn, buffer, 2);
	if (bytes == 0) {
		/* connection not ready */
		return NULL;
	}

	if (bytes <= 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received connection close, finishing connection session");
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	if (bytes != 2) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Expected to receive complete websocket frame header but found only %d bytes, closing session: %d",
			    bytes, conn->id);
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received %d bytes for websocket header", bytes);

	/* build next message */
	msg = nopoll_new (noPollMsg, 1);
	if (msg == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Failed to allocate memory for received message, closing session id: %d", 
			    conn->id);
		nopoll_conn_shutdown (conn);
		return NULL;
	} /* end if */

	/* set initial ref count */
	msg->refs = 1;

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

	/* get more bytes */
	if (msg->is_masked) {
		bytes = nopoll_conn_receive (conn, (noPollPtr) msg->mask, 4);
		if (bytes != 4) {
			nopoll_msg_unref (msg);
			nopoll_conn_shutdown (conn);
			return NULL; 
		} /* end if */
		
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received mask value = %d", nopoll_get_32bit (msg->mask));
	} /* end if */

	/* read the rest */
	if (msg->payload_size < 126) {
		/* nothing to declare here */

	} else if (msg->payload_size == 126) {
		/* get extended 2 bytes length as unsigned 16 bit
		   unsigned integer */
		msg->payload_size = 0;
		msg->payload_size = (buffer[2] << 8);
		msg->payload_size |= buffer[3];
		
	} else if (msg->payload_size == 127) {
		/* get extended 2 bytes length as unsigned 16 bit
		   unsigned integer */
		msg->payload_size = 0;
		msg->payload_size |= ((long)(buffer[2]) << 56);
		msg->payload_size |= ((long)(buffer[3]) << 48);

		/* read more content (next 6 bytes) */
		if ((bytes = nopoll_conn_receive (conn, buffer, 6)) != 6) {
			nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, 
				    "Expected to receive next 6 bytes for websocket frame header but found only %d bytes, closing session: %d",
				    bytes, conn->id);
			nopoll_conn_shutdown (conn);
			return NULL;
		} /* end if */

		msg->payload_size |= ((long)(buffer[0]) << 40);
		msg->payload_size |= ((long)(buffer[1]) << 32);
		msg->payload_size |= ((long)(buffer[2]) << 24);
		msg->payload_size |= ((long)(buffer[3]) << 16);
		msg->payload_size |= ((long)(buffer[4]) << 8);
		msg->payload_size |= buffer[5];
	} /* end if */

	if (msg->op_code == NOPOLL_PONG_FRAME) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "PONG received over connection id=%d", conn->id);
		nopoll_msg_unref (msg);
		return NULL;
	} /* end if */

	if (msg->op_code == NOPOLL_PING_FRAME) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "PING received over connection id=%d, replying PONG", conn->id);
		nopoll_msg_unref (msg);

		/* call to send pong */
		nopoll_conn_send_pong (conn);

		return NULL;
	} /* end if */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Detected incoming websocket frame: fin(%d), op_code(%d), is_masked(%d), payload size(%ld), mask=%d", 
		    msg->has_fin, msg->op_code, msg->is_masked, msg->payload_size, nopoll_get_32bit (msg->mask));

	/* check here for the limit of message we are willing to accept */
	/* FIX SECURITY ISSUE */

	/* copy payload received */
	msg->payload = nopoll_new (char, msg->payload_size + 1);
	if (msg->payload == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to acquire memory to read the incoming frame, dropping connection id=%d", conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;		
	} /* end if */

	
	bytes = nopoll_conn_receive (conn, msg->payload, msg->payload_size);
	if (bytes <= 0) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Connection lost during message reception, dropping connection id=%d", conn->id);
		nopoll_msg_unref (msg);
		nopoll_conn_shutdown (conn);
		return NULL;		
	} /* end if */

	if (bytes != msg->payload_size) {
		/* record we've got content pending to be read */
		msg->remain_bytes = msg->payload_size - bytes;

		/* set connection in remaining data to read */
		nopoll_log (conn->ctx, NOPOLL_LEVEL_WARNING, "Received fewer bytes than expected (%d < %d)", bytes, msg->payload_size);
	} /* end if */

	/* now unmask content (if required) */
	if (msg->is_masked) {
		nopoll_conn_mask_content (conn->ctx, msg->payload, msg->payload_size, msg->mask);
	} /* end if */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Message received: %s", (const char *) msg->payload);

	return msg;
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
 * @param length Amount of bytes to take from the content to be sent.
 *
 * @return The number of bytes written otherwise -1 is returned in
 * case of failure. The function will fail if some parameter is NULL
 * or undefined, or the content provided is not UTF-8.
 */
int           nopoll_conn_send_text (noPollConn * conn, const char * content, long length)
{
	if (conn == NULL || content == NULL || length < 0)
		return -1;

	if (conn->role == NOPOLL_ROLE_MAIN_LISTENER) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Trying to send content over a master listener connection");
		return -1;
	} /* end if */

	/* sending content as client */
	if (conn->role == NOPOLL_ROLE_CLIENT) {
		return nopoll_conn_send_frame (conn, /* fin */ nopoll_true, /* masked */ nopoll_true, 
					       NOPOLL_TEXT_FRAME, length, (noPollPtr) content);
	} /* end if */

	/* sending content as listener */
	return nopoll_conn_send_frame (conn, /* fin */ nopoll_true, /* masked */ nopoll_false, 
				       NOPOLL_TEXT_FRAME, length, (noPollPtr) content);
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
 * @param block If nopoll_true, the caller will be blocked until the
 * amount of bytes requested are satisfied or until the timeout is
 * reached (if enabled). If nopoll_false is provided, the function
 * won't block and will return all bytes available at this moment.
 *
 * @paran timeout (milliseconds 1sec = 1000ms) If provided a value
 * higher than 0, a timeout will be enabled to complete the
 * operation. If the timeout is reached, the function will return the
 * bytes read so far. Please note that the function has a precision of
 * 10ms.
 *
 * @return Number of bytes read or -1 if it fails. 
 *
 */
int           nopoll_conn_read (noPollConn * conn, char * buffer, int bytes, nopoll_bool block, long int timeout)
{
	long int           wait_slice = 0;
	noPollMsg        * msg        = NULL;
	struct  timeval    start;
	struct  timeval    stop;
	struct  timeval    diff;
	long               ellapsed;
	int                desp       = 0;
	int                amount;
	int                total_read = 0;

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
		gettimeofday (&start, NULL);

	/* clear the buffer */
	memset (buffer, 0, bytes);

	/* check here if we have a pending message to read */
	if (conn->pending_msg)  {
		/* get references to pending data */
		amount = conn->pending_diff;
		msg    = conn->pending_msg;
		if (amount > bytes) {
			conn->pending_diff -= bytes;
			amount = bytes;
		} else {
			conn->pending_diff = 0;
		}

		/* read content */
		memcpy (buffer + desp, nopoll_msg_get_payload (msg) + (nopoll_msg_get_payload_size (msg) - amount - 1), amount);
		total_read += amount;

		/* now release internally the content if consumed the message */
		if (conn->pending_diff == 0) {
			nopoll_msg_unref (conn->pending_msg);
			conn->pending_msg = NULL;
		} /* end if */

		/* see if we have finished */
		if (amount == bytes || ! block)
			return amount;

		
	} /* end if */


	/* for for the content */
	while (nopoll_true) {
		/* call to get next message */
		while ((msg = nopoll_conn_get_msg (conn)) == NULL) {
			
			if (! nopoll_conn_is_ok (conn)) {
				nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Received websocket connection close during wait reply..");
				return -1;
			} /* end if */

		} /* end if */

		/* get the message content into the buffer */
		if (msg) {
			/* get the amount of bytes we can read */
			amount = nopoll_msg_get_payload_size (msg);
			nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Received %d bytes (requested %d bytes)", amount, bytes);
			if (amount > bytes) {
				/* save here the difference between
				 * what we have read and remaining data */
				conn->pending_diff = amount - bytes;
				conn->pending_msg  = msg;
				amount = bytes;

				/* acquire a reference to the message */
				nopoll_msg_ref (msg);
			} /* end if */
			/* copy data */
			memcpy (buffer + desp, nopoll_msg_get_payload (msg), amount);
			total_read += amount;

			/* release message */
			nopoll_msg_unref (msg);

			/* return amount read */
			if (total_read == bytes || ! block)
				return total_read;
		}

		/* if timeout is still bigger continue reading data */
		if (timeout <= 0)
			break;

		/* check to stop due to timeout */
		if (timeout > 0) {
			gettimeofday (&stop, NULL);
			nopoll_timeval_substract (&stop, &start, &diff);
			
			ellapsed = (diff.tv_sec * 1000000) + diff.tv_usec;
			if (ellapsed > timeout) 
				break;
		} /* end if */
		
		nopoll_sleep (wait_slice);
	} /* end while */

	/* reached this point, return that timeout was reached */
	return total_read;
}

/** 
 * @brief Allows to send a ping message over the Websocket connection
 * provided. The function will not block the caller.
 *
 * @param conn The connection where the PING operation will be sent.
 *
 * @param nopoll_true if the operation was sent without any error,
 * otherwise nopoll_false is returned.
 */
nopoll_bool      nopoll_conn_send_ping (noPollConn * conn)
{
	return nopoll_conn_send_frame (conn, nopoll_true, nopoll_false, NOPOLL_PING_FRAME, 0, NULL);
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
nopoll_bool      nopoll_conn_send_pong (noPollConn * conn)
{
	return nopoll_conn_send_frame (conn, nopoll_true, nopoll_false, NOPOLL_PONG_FRAME, 0, NULL);
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
 */
int nopoll_conn_send_frame (noPollConn * conn, nopoll_bool fin, nopoll_bool masked,
			    noPollOpCode op_code, long length, noPollPtr content)

{
	char   header[14];
	int    header_size;
	char * send_buffer;
	int    bytes_written;
	char   mask[4];
	int    mask_value = 0;

	/* clear header */
	memset (header, 0, 14);

	/* set header codes */
	if (fin) 
		nopoll_set_bit (header, 7);
	
	if (masked) {
		nopoll_set_bit (header + 1, 7);
		
		/* define a random mask */
		mask_value = (int) random ();
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
	} else if (length < 65535) {
		/* not supported yet */
		return -1;
	} else if (length < 9223372036854775807) {
		/* not supported yet */
		return -1;
	}

	/* place mask */
	if (masked) {
		nopoll_set_32bit (mask_value, header + header_size);
		header_size += 4;
	} /* end if */

	/* allocate enough memory to send content */
	send_buffer = nopoll_new (char, length + header_size);
	if (send_buffer == NULL) {
		nopoll_log (conn->ctx, NOPOLL_LEVEL_CRITICAL, "Unable to allocate memory to implement send operation");
		return -1;
	} /* end if */
	
	/* copy content to be sent */
	memcpy (send_buffer, header, header_size);
	if (length > 0) {
		memcpy (send_buffer + header_size, content, length);

		/* mask content before sending if requested */
		if (masked) {
			nopoll_conn_mask_content (conn->ctx, send_buffer + header_size, length, mask);
		}
	} /* end if */

	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Mask used for this delivery: %d",
		    nopoll_get_32bit (send_buffer +2));
	
	/* send content */
	bytes_written = conn->send (conn, send_buffer, length + header_size);
	nopoll_log (conn->ctx, NOPOLL_LEVEL_DEBUG, "Bytes written to the wire %d (masked? %d, mask: %d, header size: %d, length: %d)", 
		    bytes_written, masked, mask_value, header_size, length);

	/* release memory */
	nopoll_free (send_buffer);
	
	/* return bytes written */
	return bytes_written - header_size;
}


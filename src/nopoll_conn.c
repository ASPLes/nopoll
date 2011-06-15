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
 * @brief Allows to get current timeout set for \ref nopollConn
 * connect operation.
 *
 * See also \ref nopoll_conn_connect_timeout.
 *
 * @return Current timeout configured. Returned value is measured in
 * microseconds (1 second = 1000000 microseconds). If a null value is
 * received, 0 is return and no timeout is implemented.
 */
long              nopoll_conn_get_connect_timeout (nopollCtx * ctx)
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
void               nopoll_conn_connect_timeout (nopollCtx * ctx,
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
NOPOLL_SOCKET nopoll_conn_sock_connect (nopollCtx   * ctx,
					const char  * host,
					const char  * port)
{
	struct hostent     * hostent;
	struct sockaddr_in   saddr;
	NOPOLL_SOCKET        session;

	/* resolve hosting name */
	hostent = gethostbyname (host);
	if (hostent != NULL) {
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
 * @param protocol Optional protocol requested to be activated for
 * this connection.
 */
nopollConn * nopoll_conn_new (nopollCtx  * ctx,
			      const char * host_ip, 
			      const char * host_port, 
			      const char * host_name,
			      const char * get_url, 
			      const char * protocol)
{
	nopollConn     * conn;
	NOPOLL_SOCKET    session;

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
	conn = nopoll_new (nopollConn, 1);
	if (conn == NULL) 
		return NULL;
	
	/* configure context */
	conn->ctx     = ctx;
	conn->session = session;

	/* return connection created */
	return conn;
}

/** 
 * @brief Allows to check if the provided connection is working at
 * this moment.
 *
 * @param conn The websocket connection to be checked.
 *
 * @return nopoll_true in the case the connection is working otherwise
 * nopoll_false is returned.
 */
nopoll_bool    no_poll_conn_is_connected (nopollConn * conn)
{
	if (conn == NULL)
		return nopoll_false;

	/* return current socket status */
	return conn->session > 0;
}

/** 
 * @brief Allows to get the socket associated to this nopoll
 * connection.
 *
 * @return The socket reference or -1 if it fails.
 */
NOPOLL_SOCKET nopoll_conn_socket (nopollConn * conn)
{
	if (conn == NULL)
		return -1;
	return conn->session;
}



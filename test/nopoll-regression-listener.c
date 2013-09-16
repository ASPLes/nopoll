/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2013 Advanced Software Production Line, S.L.
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
#include <nopoll.h>

#include <signal.h>

nopoll_bool on_connection_opened (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	if (! nopoll_conn_set_sock_block (nopoll_conn_socket (conn), nopoll_false)) {
		printf ("ERROR: failed to configure non-blocking state to connection..\n");
		return nopoll_false;
	} /* end if */

	/* check to reject */
	if (nopoll_cmp (nopoll_conn_get_origin (conn), "http://deny.aspl.es"))  {
		printf ("INFO: rejected connection from %s, with Host: %s and Origin: %s\n",
			nopoll_conn_host (conn), nopoll_conn_get_host_header (conn), nopoll_conn_get_origin (conn));
		return nopoll_false;
	} /* end if */

	/* notify connection accepted */
	/* printf ("INFO: connection received from %s, with Host: %s and Origin: %s\n",
	   nopoll_conn_host (conn), nopoll_conn_get_host_header (conn), nopoll_conn_get_origin (conn)); */
	return nopoll_true;
}

noPollMsg * previous_msg = NULL;

void write_file_handler (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr user_data)
{
	FILE * open_file_cmd = user_data;
	const char * content = (const char *) nopoll_msg_get_payload (msg);

	/* check for close operation */
	if (nopoll_ncmp (content, "close-file", 10)) {
		printf ("CLOSING FILE: opened..\n");
		fclose (open_file_cmd);
		open_file_cmd = NULL;
		return;
	} /* end if */

	if (open_file_cmd) {
		/* write content */
		fwrite (content, 1, nopoll_msg_get_payload_size (msg), open_file_cmd);

		return;
	} /* end if */
	return;
}

void listener_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr user_data)
{
	const char * content = (const char *) nopoll_msg_get_payload (msg);
	FILE       * file = NULL;
	char         buffer[1024];
	int          bytes;
	int          sent;
	char         example[100];
	int          shown;
	noPollMsg  * aux;
	nopoll_bool  dont_reply = nopoll_false;
	FILE       * open_file_cmd = NULL;
	int          iterator;

	/* check for open file commands */
	if (nopoll_ncmp (content, "open-file: ", 11)) {
#if defined(NOPOLL_OS_WIN32)
		open_file_cmd = fopen (content + 11, "ab");
#else
		open_file_cmd = fopen (content + 11, "a");
#endif
		if (open_file_cmd == NULL) {
			printf ("ERROR: unable to open file: %s\n", content + 11);
			return;
		} /* end if */

		/* set handler */
		nopoll_conn_set_on_msg (conn, write_file_handler, open_file_cmd);

		return;
	} /* end if */

	printf ("Message received: %s\n", content);
	if (nopoll_ncmp (content, "release-message", 15)) {
		printf ("Listener: RELEASING previous message..\n");
		nopoll_msg_unref (previous_msg);
		previous_msg = NULL;
		return;
	} /* end if */

	if (nopoll_ncmp (content, "1234-1) ", 8)) {
		printf ("Listener: waiting a second to force buffer flooding..\n");
		nopoll_sleep (100000);
		dont_reply = nopoll_true;
	} /* end if */

	/* get initial bytes */
	bytes = nopoll_msg_get_payload_size (msg);
	shown = bytes > 100 ? 99 : bytes;

	memset (example, 0, 100);
	/*	if (! nopoll_msg_is_fragment (msg)) */
		memcpy (example, (const char *) nopoll_msg_get_payload (msg), shown);

	printf ("Listener received (size: %d, ctx refs: %d): (first %d bytes, fragment: %d) '%s'\n", 
		nopoll_msg_get_payload_size (msg),
		nopoll_ctx_ref_count (ctx), shown, nopoll_msg_is_fragment (msg), example);

	if (nopoll_cmp (content, "ping")) {
		/* send a ping */
		nopoll_conn_send_ping (conn);
		return;
	} else if (nopoll_cmp (content, "get-file")) {
		iterator = 0;
		file     = NULL;
		while (nopoll_true) {
#if defined(NOPOLL_OS_WIN32)
			file = fopen ("nopoll-regression-client.c", "rb");
#else
			file = fopen ("nopoll-regression-client.c", "r");
#endif		
			printf ("LISTENER: file pointer (%p, errno=%d : %s)..\n", file, errno, strerror (errno));
			
			if (file)
				break;
			iterator++;
			if (iterator > 3) {
				printf ("ERROR: failed to open nopoll-regression-client.c (fopen call failed)\n");
				nopoll_conn_shutdown (conn);
				return;
			} /* end if */
		} /* end while */

		while (! feof (file)) {
			/* read content */
			bytes = fread (buffer, 1, 1024, file);
			/* send content */
			if (bytes > 0) {
				/* send content and get the result */
				/* printf ("Sending message with %d bytes..\n", bytes); */
				/* nopoll_log_enable (ctx, nopoll_true); */
				
				while (nopoll_true) {
					/* try to send content */
					sent = nopoll_conn_send_text (conn, buffer, bytes);
					/* nopoll_log_enable (ctx, nopoll_false); */
					if (sent != bytes) {
						if (errno == NOPOLL_EWOULDBLOCK) {
							nopoll_sleep (1000);
							/* printf ("   ..retrying..sending message with %d bytes..\n", bytes); */
							continue;
						} /* end if */
						printf ("ERROR: expected to send %d bytes but sent different content size (%d bytes), errno=%d (%d)..\n", 
							bytes, sent, errno, NOPOLL_EWOULDBLOCK);
					} /* end if */
					break;
				}
			} /* end if */
			/* next */
		} /* end while */

		/* now close the handle */
		fclose (file);
		return;
	} /* end if */

	/* check if we have to reply */
	if (dont_reply)
		return;

	if (nopoll_msg_is_fragment (msg)) {
		printf ("Found fragment, FIN = %d (%p)?..\n", nopoll_msg_is_final (msg), msg);
		/* call to join this message */
		aux          = previous_msg;
		previous_msg = nopoll_msg_join (previous_msg, msg);
		nopoll_msg_unref (aux);
		if (! nopoll_msg_is_final (msg)) {
			printf ("Found fragment that is not final..\n");
			printf ("Not replying because frame fragment received..\n");
			return;
		} /* end if */

		printf ("Found final fragment, replying with complete content: %s (refs: %d)..\n",
			(const char *) nopoll_msg_get_payload (previous_msg), 
			nopoll_msg_ref_count (previous_msg));

		/* ok, now found final piece, replying */
		nopoll_conn_send_text (conn, (const char *) nopoll_msg_get_payload (previous_msg), 
				       nopoll_msg_get_payload_size (previous_msg));
		/* release reference */
		nopoll_msg_unref (previous_msg);
		previous_msg = NULL;

		return;
	}

	/* send reply as received */
	nopoll_conn_send_text (conn, (const char *) nopoll_msg_get_payload (msg), 
			       nopoll_msg_get_payload_size (msg));
	return;
}

noPollCtx      * ctx = NULL;

void __terminate_listener (int value)
{
	
	/* unlock listener */
	nopoll_loop_stop (ctx);

	return;
}

int main (int argc, char ** argv)
{
	noPollConn     * listener;
	noPollConn     * listener2;
	int              iterator;

	signal (SIGTERM,  __terminate_listener);

	/* create the context */
	ctx = nopoll_ctx_new ();

	iterator = 1;
	while (iterator < argc) {
		/* check for debug */
		printf ("Checking agument: %s\n", argv[iterator]);
		if (nopoll_cmp (argv[iterator], "--debug")) {
			printf ("Activating debug..\n");
			nopoll_log_enable (ctx, nopoll_true);
#if !defined(NOPOLL_OS_WIN32)
			nopoll_log_color_enable (ctx, nopoll_true);
#endif
		} /* end if */

		/* next position */
		iterator++;
	}

	/* call to create a listener */
	listener = nopoll_listener_new (ctx, "0.0.0.0", "1234");
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: Expected to find proper listener connection status, but found..\n");
		return -1;
	}

	printf ("noPoll listener started at: %s:%s (refs: %d)..\n", nopoll_conn_host (listener), nopoll_conn_port (listener), nopoll_conn_ref_count (listener));

	/* now start a TLS version */
	listener2 = nopoll_listener_tls_new (ctx, "0.0.0.0", "1235");
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: Expected to find proper listener TLS connection status, but found..\n");
		return -1;
	} /* end if */

	/* configure certificates to be used by this listener */
	if (! nopoll_listener_set_certificate (listener2, "test-certificate.crt", "test-private.key", NULL)) {
		printf ("ERROR: unable to configure certificates for TLS websocket..\n");
		return -1;
	}

	/* register certificates at context level */
	if (! nopoll_ctx_set_certificate (ctx, NULL, "test-certificate.crt", "test-private.key", NULL)) {
		printf ("ERROR: unable to setup certificates at context level..\n");
		return -1;
	}

	/* set on message received */
	nopoll_ctx_set_on_msg (ctx, listener_on_message, NULL);

	/* set on open */
	nopoll_ctx_set_on_open (ctx, on_connection_opened, NULL);
	
	/* process events */
	nopoll_loop_wait (ctx, 0);

	/* unref connection */
	nopoll_conn_close (listener);
	nopoll_conn_close (listener2);

	/* finish */
	printf ("Listener: finishing references: %d\n", nopoll_ctx_ref_count (ctx));
	if (previous_msg) {
		printf ("..reference counting for previous msg: %d\n", nopoll_msg_ref_count (previous_msg));
		nopoll_msg_unref (previous_msg);
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* call to release all pending memory allocated as a
	 * consequence of using nopoll (especially TLS) */
	nopoll_cleanup_library ();

	return 0;
}

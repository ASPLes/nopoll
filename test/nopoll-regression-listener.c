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
#include <nopoll-regression-common.h>
#include <nopoll.h>

#include <signal.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int connection_close_count = 0;

void __nopoll_regression_on_close (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	printf ("Reg test: called connection close (TLS: %d)..\n", nopoll_conn_is_tls_on (conn));
	connection_close_count++;
	return;
}

nopoll_bool on_connection_opened (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	/* set connection close */
	nopoll_conn_set_on_close (conn, __nopoll_regression_on_close, NULL);

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

	/* get protocol to reply an especific case. This is an example
	   on how to detect protocols requested by the client and how
	   to reply with a particular value at the server. */
	printf ("Requested protocol: %s\n", nopoll_conn_get_requested_protocol (conn));
	if (nopoll_cmp (nopoll_conn_get_requested_protocol (conn), "hello-protocol")) {
		/* set hello-protocol-response */
		nopoll_conn_set_accepted_protocol (conn, "hello-protocol-response");
	} /* end if */

	/* notify connection accepted */
	/* printf ("INFO: connection received from %s, with Host: %s and Origin: %s\n",
	   nopoll_conn_host (conn), nopoll_conn_get_host_header (conn), nopoll_conn_get_origin (conn)); */
	return nopoll_true;
}

noPollMsg * previous_msg = NULL;

void write_file_handler (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr user_data)
{
	FILE       * open_file_cmd = (FILE*) user_data;
	const char * content = (const char *) nopoll_msg_get_payload (msg);
	int          value;

	/* check for close operation */
	if (nopoll_ncmp (content, "close-file", 10)) {
		printf ("CLOSING FILE: opened..\n");
		fclose (open_file_cmd);
		open_file_cmd = NULL;
		return;
	} /* end if */

	if (open_file_cmd) {
		/* write content */
	        value = fwrite (content, 1, nopoll_msg_get_payload_size (msg), open_file_cmd);
	        if (value < 0)
		        return;

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
	char       * ref;

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

	/* printf ("Message received: %s\n", content); */
	if (nopoll_ncmp (content, "close with message", 18)) {
		printf ("Listener: RELEASING connection (closing it) with reason..\n");
		nopoll_conn_close_ext (conn, 1048, "Hey, this is a very reasonable error message", 44);
		return;
	} /* end if */

	if (nopoll_ncmp (content, "release-message", 15)) {
		printf ("Listener: RELEASING previous message..\n");
		nopoll_msg_unref (previous_msg);
		previous_msg = NULL;
		return;
	} /* end if */
	if (nopoll_ncmp (content, "get-cookie", 10)) {
		printf ("Listener: reporting cookie: %s\n", nopoll_conn_get_cookie (conn));
		nopoll_conn_send_text (conn, nopoll_conn_get_cookie (conn), strlen (nopoll_conn_get_cookie (conn)));
		return;
	}

	/* printf ("Checking for set-broken socket: %s\n", content); */
	if (nopoll_ncmp (content, "set-broken-socket", 17)) {
		printf ("Listener: setting broken socket on conn: %p (socket=%d)\n",
			conn, (int) nopoll_conn_socket (conn));
		nopoll_conn_shutdown (conn);
		return;
	} /* end if */

	if (nopoll_ncmp (content, "get-connection-close-count", 26)) {
		printf ("Sending reply to report connection close...\n");
		ref = nopoll_strdup_printf ("%d", connection_close_count);
		nopoll_conn_send_text (conn, ref, strlen (ref));
		nopoll_free (ref);
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
	printf ("Sending reply... (same message size: %d)\n", nopoll_msg_get_payload_size (msg));
	nopoll_conn_send_text (conn, (const char *) nopoll_msg_get_payload (msg), 
			       nopoll_msg_get_payload_size (msg));
	return;
}

noPollCtx      * ctx = NULL;

void __terminate_listener (int value)
{
	printf ("__terminate_listener: Signal received...terminating listener..\n");
	/* unlock listener */
	nopoll_loop_stop (ctx);

	return;
}

int verify_callback (int ok, X509_STORE_CTX * store) {
	char   data[256];
	X509 * cert;
	int    depth;
	int    err;

	if (! ok) {
		cert  = X509_STORE_CTX_get_current_cert (store);
		depth = X509_STORE_CTX_get_error_depth (store);
		err   = X509_STORE_CTX_get_error (store);

		printf ("CERTIFICATE: error at depth: %d\n", depth);

		X509_NAME_oneline (X509_get_issuer_name (cert), data, 256);
		printf ("CERTIFICATE: issuer: %s\n", data);

		X509_NAME_oneline (X509_get_subject_name (cert), data, 256);
		printf ("CERTIFICATE: subject: %s\n", data);

		printf ("CERTIFICATE: error %d:%s\n", err, X509_verify_cert_error_string (err));

	}
	return ok; /* return same value */
}

noPollPtr ssl_context_creator (noPollCtx * ctx, noPollConn * conn, noPollConnOpts * opts, nopoll_bool is_client, noPollPtr user_data)
{
	SSL_CTX             * ssl_ctx;
	noPollConn          * listener;

	/* very basic context creation using default settings provided
	 * by OpenSSL */
	if (is_client) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		return SSL_CTX_new (TLSv1_client_method ());
#else
	        return SSL_CTX_new (TLS_client_method ());
#endif
	} /* end if */

	/* get the ssl context */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	ssl_ctx = SSL_CTX_new (TLSv1_server_method ());
#else
	ssl_ctx = SSL_CTX_new (TLS_server_method ());
#endif

	/* get a reference to the listener */
	listener = nopoll_conn_get_listener (conn);

	if (nopoll_cmp ("1239", nopoll_conn_port (listener))) {
		printf ("ACCEPTED ssl connection on port: %s (for conn %p)\n", nopoll_conn_port (listener), conn);

		/* ok, especiall case where we require a certain
		 * certificate from renote side */
		if (SSL_CTX_load_verify_locations (ssl_ctx, "client-side-cert-auth-cacert.crt", NULL) != 1) {
			printf ("ERROR: unable to add ca certificate...\n");
		}


		/* make server to ask for a certificate to the client
		 * .... and verify it */
		SSL_CTX_set_verify (ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
	} /* end if */
	
	printf ("RETURNING: ssl context reference %p\n", ssl_ctx);
	return ssl_ctx;
}

int main (int argc, char ** argv)
{
	noPollConn     * listener;
	noPollConn     * listener_6;
	noPollConn     * listener2;
	noPollConn     * listener_62;
#if defined(NOPOLL_HAVE_SSLv23_ENABLED)	
	noPollConn     * listener3;
#endif
#if defined(NOPOLL_HAVE_SSLv3_ENABLED)	
	noPollConn     * listener4;
#endif	
#if defined(NOPOLL_HAVE_TLSv11_ENABLED)
	noPollConn     * listener5;
	noPollConn     * listener_65;
#endif
	noPollConn     * listener6;
#if defined(NOPOLL_HAVE_TLSv12_ENABLED)
	noPollConn     * listener7;
#endif	
	int              iterator;
	noPollConnOpts * opts;

	signal (SIGTERM,  __terminate_listener);

#if defined(__NOPOLL_PTHREAD_SUPPORT__)	
	printf ("INFO: install default threading functions to check noPoll locking code..\n");
	nopoll_thread_handlers (__nopoll_regtest_mutex_create,
				__nopoll_regtest_mutex_destroy,
				__nopoll_regtest_mutex_lock,
				__nopoll_regtest_mutex_unlock);
#endif

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

	/* call to create a listener */
	listener_6 = nopoll_listener_new6 (ctx, "::1", "2234");
	if (! nopoll_conn_is_ok (listener_6)) {
		printf ("ERROR: Expected to find proper listener connection status, but found (IPv6 -- .1.1)..\n");
		return -1;
	} /* end if */

	printf ("noPoll listener started at (IPv6): %s:%s (refs: %d)..\n", nopoll_conn_host (listener_6), nopoll_conn_port (listener_6), nopoll_conn_ref_count (listener_6));

	/* now start a TLS version */
	printf ("Test: starting listener with TLS (TLSv1) at :1235\n");
	listener2 = nopoll_listener_tls_new (ctx, "0.0.0.0", "1235");
	if (! nopoll_conn_is_ok (listener2)) {
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

	/* now start a TLS version */
	printf ("Test: starting listener with TLS IPv6 (TLSv1) at :2235\n");
	listener_62 = nopoll_listener_tls_new6 (ctx, "::1", "2235");
	if (! nopoll_conn_is_ok (listener_62)) {
		printf ("ERROR: Expected to find proper listener TLS connection status, but found..\n");
		return -1;
	} /* end if */

	/* configure certificates to be used by this listener */
	if (! nopoll_listener_set_certificate (listener_62, "test-certificate.crt", "test-private.key", NULL)) {
		printf ("ERROR: unable to configure certificates for TLS websocket..\n");
		return -1;
	}

	/* register certificates at context level */
	if (! nopoll_ctx_set_certificate (ctx, NULL, "test-certificate.crt", "test-private.key", NULL)) {
		printf ("ERROR: unable to setup certificates at context level..\n");
		return -1;
	}

#if defined(NOPOLL_HAVE_SSLv23_ENABLED)	
	/* start listener with sslv23 */
	printf ("Test: starting listener with TLS (SSLv23) at :1236 (all methods)\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_SSLV23);
	listener3 = nopoll_listener_tls_new_opts (ctx, opts, "0.0.0.0", "1236");
	if (! nopoll_conn_is_ok (listener3)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (:1236, SSLv23), but found..\n");
		return -1;
	} /* end if */
#endif

#if defined(NOPOLL_HAVE_SSLv3_ENABLED)	
	printf ("Test: starting listener with TLS (SSLv3) at :1237\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_SSLV3);
	listener4 = nopoll_listener_tls_new_opts (ctx, opts, "0.0.0.0", "1237");
	if (! nopoll_conn_is_ok (listener4)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (:1237, SSLv3), but found..\n");
		return -1;
	} /* end if */
#endif	

#if defined(NOPOLL_HAVE_TLSv11_ENABLED)
	printf ("Test: starting listener with TLS (TLSv1.1) at :1238\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_1);
	listener5 = nopoll_listener_tls_new_opts (ctx, opts, "0.0.0.0", "1238");
	if (! nopoll_conn_is_ok (listener5)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (:1238, TLSv1.1), but found..\n");
		return -1;
	} /* end if */

	printf ("Test: starting listener with TLS (TLSv1.1) (IPv6) at :2238\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_1);
	listener_65 = nopoll_listener_tls_new_opts6 (ctx, opts, "::1", "2238");
	if (! nopoll_conn_is_ok (listener_65)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (::1:2238, TLSv1.1, IPv6), but found..\n");
		return -1;
	} /* end if */
	
#endif

#if defined(NOPOLL_HAVE_TLSv12_ENABLED)
	printf ("Test: starting listener with TLS (TLSv1.2) at :1240\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_2);
	listener7 = nopoll_listener_tls_new_opts (ctx, opts, "0.0.0.0", "1240");
	if (! nopoll_conn_is_ok (listener7)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (:1240, TLSv1.2), but found..\n");
		return -1;
	} /* end if */
#endif

	opts     = nopoll_conn_opts_new ();

	/* configure server certificates (server.pem) signed by the
	 * provided ca (root.pem) also configured in the last
	 * parameter */
	if (! nopoll_conn_opts_set_ssl_certs (opts, 
					      "server.pem",
					      "server.pem",
					      NULL,
					      "root.pem")) {
		printf ("ERROR: unable to setup certificates...\n");
		return -1;
	}
	/* configure peer verification */
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_true);
	    
	listener6 = nopoll_listener_tls_new_opts (ctx, opts, "0.0.0.0", "1239");
	if (! nopoll_conn_is_ok (listener6)) {
		printf ("ERROR: Expected to find proper listener TLS connection status (:1236, SSLv23), but found..\n");
		return -1;
	} /* end if */


	/* configure ssl context creator */
	/* nopoll_ctx_set_ssl_context_creator (ctx, ssl_context_creator, NULL); */

	/* set on message received */
	nopoll_ctx_set_on_msg (ctx, listener_on_message, NULL);

	/* set on open */
	nopoll_ctx_set_on_open (ctx, on_connection_opened, NULL);
	
	/* process events */
	nopoll_loop_wait (ctx, 0);

	/* unref connection */
	nopoll_conn_close (listener);
	nopoll_conn_close (listener_6);  /* ipv6 */
	
	nopoll_conn_close (listener2);
	nopoll_conn_close (listener_62); /* ipv6 */
	
#if defined(NOPOLL_HAVE_SSLv23_ENABLED)	
	nopoll_conn_close (listener3);
#endif
#if defined(NOPOLL_HAVE_SSLv3_ENABLED)	
	nopoll_conn_close (listener4);
#endif	
#if defined(NOPOLL_HAVE_TLSv12_ENABLED)
	nopoll_conn_close (listener5);
	nopoll_conn_close (listener_65);
#endif
	nopoll_conn_close (listener6);
#if defined(NOPOLL_HAVE_TLSv12_ENABLED)
	nopoll_conn_close (listener7);
#endif	

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

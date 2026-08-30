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
#include <nopoll-regression-common.h>
#include <nopoll.h>
#include <limits.h>

nopoll_bool debug = nopoll_false;
nopoll_bool show_critical_only = nopoll_false;

nopoll_bool test_sending_and_check_echo (noPollConn * conn, const char * label, const char * msg)
{
	char  buffer[1024];
	int   length = strlen (msg);
	int   bytes_read;
	int   tries;

	/* wait for the connection to be ready but limiting how much we
	 * are willing to wait: without this limit, any connection
	 * failure (for example, expired test certificates) makes the
	 * regression test to hang for ever instead of reporting a
	 * failure */
	tries = 3000; /* 3000 x 10ms = 30 seconds */
	while (tries > 0) {
		if (nopoll_conn_is_ready (conn))
			break;

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: %s: connection failure detected while waiting for it to be ready..\n", label);
			return nopoll_false;
		} /* end if */

		nopoll_sleep (10000);
		tries--;
	} /* end while */

	if (! nopoll_conn_is_ready (conn)) {
		printf ("ERROR: %s: timeout reached (30 seconds) while waiting for the connection to be ready..\n", label);
		return nopoll_false;
	} /* end if */

	/* send content text(utf-8) */
	printf ("%s: sending content..\n", label);
	if (nopoll_conn_send_text (conn, msg, length) != length) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 1024, blocking and with a 3 seconds timeout) */
	bytes_read = nopoll_conn_read (conn, buffer, length, nopoll_true, 3000);
	if (bytes_read > 0)
		buffer[bytes_read] = 0;
	
	if (bytes_read != length) {
		printf ("ERROR: expected to find 14 bytes but found %d..\n", bytes_read);
		return nopoll_false;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp (buffer, msg)) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			buffer);
		return nopoll_false;
	} /* end if */

	printf ("%s: received reply and echo matches..\n", label);

	/* return that we sent and received the echo reply */
	return nopoll_true;
}

void __report_critical (noPollCtx * ctx, noPollDebugLevel level, const char * log_msg, noPollPtr user_data)
{
        if (level == NOPOLL_LEVEL_CRITICAL) {
  	        printf ("CRITICAL: %s\n", log_msg);
	}
	return;
}

noPollCtx * create_ctx (void) {
	
	/* create a context */
	noPollCtx * ctx = nopoll_ctx_new ();
	nopoll_log_enable (ctx, debug);
	nopoll_log_color_enable (ctx, debug);

	/* configure handler */
	if (show_critical_only)
	        nopoll_log_set_handler (ctx, __report_critical, NULL);
	return ctx;
}

nopoll_bool test_01_strings (void) {
	/* check string compare functions */
	if (! nopoll_ncmp ("GET ", "GET ", 4)) {
		printf ("ERROR (1): expected to find right equal comparison..\n");
		return nopoll_false;
	}

	if (! nopoll_ncmp ("GET VALUE", "GET ", 4)) {
		printf ("ERROR (2): expected to find right equal comparison..\n");
		return nopoll_false;
	}

	return nopoll_true;
}

nopoll_bool test_01_base64 (void) {
	char buffer[1024];
	int  size = 1024;
	int  iterator = 0;

	/* call to produce base 64 (we do a loop to ensure we don't
	 * leak through openssl (220) bytes */
	while (iterator < 10) {
		size = 1024;
		if (! nopoll_base64_encode ("This is a test", 14, buffer, &size)) {
			printf ("ERROR: failed to encode this is a test, nopoll_base64_encode failed to encode 'This is a test' ..\n");
			return nopoll_false;
		} /* end if */
		
		/* check result */
		if (! nopoll_cmp (buffer, "VGhpcyBpcyBhIHRlc3Q=")) {
			printf ("ERROR: expected to find encoded base64 string %s but found %s..\n", 
				"VGhpcyBpcyBhIHRlc3Q=", buffer);
			return nopoll_false;
		}

		iterator++;
	}

	/* now decode content */
	iterator = 0;
	while (iterator < 10) {
		size = 1024;
		if (! nopoll_base64_decode ("VGhpcyBpcyBhIHRlc3Q=", 20, buffer, &size)) {
			printf ("ERROR: failed to decode base64 content..\n");
		}
		
		/* check result */
		if (! nopoll_cmp (buffer, "This is a test")) {
			printf ("ERROR: expected to find encoded base64 string %s but found %s..\n", 
				"This is a test", buffer);
			return nopoll_false;
		} /* end if */

		iterator++;
	}

	
	return nopoll_true;
}

/**
 * @internal Checks for the support functions at nopoll.c that are
 * used all over the library: the nonce generator, base64 decoding
 * failures and the integer accessors used to parse frame headers.
 */
nopoll_bool test_01_support (void) {

	char  guarded[64];
	char  value[4];
	int   size;
	int   iterator;
	int   round;

	printf ("Test 01-support: checking nopoll_nonce does not write past the size requested..\n");

	/* ask for sizes that are not a multiple of sizeof (long int)
	 * and check the rest of the area is left untouched. Several
	 * rounds are done so a random value matching the pattern by
	 * chance cannot hide the problem */
	round = 0;
	while (round < 20) {
		memset (guarded, 0xAA, 64);

		if (! nopoll_nonce (guarded, 13)) {
			printf ("ERROR: expected to be able to create a nonce of 13 bytes..\n");
			return nopoll_false;
		} /* end if */

		iterator = 13;
		while (iterator < 64) {
			if (guarded[iterator] != (char) 0xAA) {
				printf ("ERROR: nopoll_nonce wrote past the size requested (position %d was modified)..\n", iterator);
				return nopoll_false;
			} /* end if */
			iterator++;
		} /* end while */

		round++;
	} /* end while */

	printf ("Test 01-support: checking base64 decode reports failures..\n");

	/* decoding invalid content must report failure: reporting
	 * success left the caller using an undefined buffer */
	memset (guarded, 0xBB, 64);
	size = 16;
	if (nopoll_base64_decode ("!!!!", 4, guarded + 32, &size)) {
		printf ("ERROR: expected nopoll_base64_decode to fail with invalid content, but it reported success..\n");
		return nopoll_false;
	} /* end if */

	/* and it must not touch memory before the buffer provided */
	if (guarded[31] != (char) 0xBB) {
		printf ("ERROR: nopoll_base64_decode wrote before the buffer provided..\n");
		return nopoll_false;
	} /* end if */

	/* a valid decode must keep on working */
	size = 64;
	if (! nopoll_base64_decode ("VGhpcyBpcyBhIHRlc3Q=", 20, guarded, &size)) {
		printf ("ERROR: expected nopoll_base64_decode to decode valid content..\n");
		return nopoll_false;
	} /* end if */
	if (size != 14 || ! nopoll_cmp (guarded, "This is a test")) {
		printf ("ERROR: expected to decode 'This is a test' (14 bytes) but found '%s' (%d bytes)..\n", guarded, size);
		return nopoll_false;
	} /* end if */

	printf ("Test 01-support: checking integer accessors with the high bit set..\n");

	/* these values exercise the octets >= 0x80 used by frame
	 * lengths, close codes and masks */
	value[0] = (char) 0xff;
	value[1] = (char) 0xfe;
	if (nopoll_get_16bit (value) != 0xfffe) {
		printf ("ERROR: expected nopoll_get_16bit to report 0xfffe but found 0x%x..\n", nopoll_get_16bit (value));
		return nopoll_false;
	} /* end if */

	value[0] = (char) 0x80;
	value[1] = (char) 0x00;
	value[2] = (char) 0x00;
	value[3] = (char) 0x01;
	if (nopoll_get_32bit (value) != (int) 0x80000001) {
		printf ("ERROR: expected nopoll_get_32bit to report 0x80000001 but found 0x%x..\n", nopoll_get_32bit (value));
		return nopoll_false;
	} /* end if */

	return nopoll_true;
}

nopoll_bool test_01_masking (void) {

	char         mask[4];
	int          mask_value;
	char         buffer[1024];
	noPollCtx  * ctx;

	/* clear buffer */
	memset (buffer, 0, 1024);

	/* create context */
	ctx = create_ctx ();

#if defined(NOPOLL_OS_WIN32)
	mask_value = rand ();
#else
	mask_value = random ();
#endif
	printf ("Test-01 masking: using masking value %d\n", mask_value);
	nopoll_set_32bit (mask_value, mask);

	memcpy (buffer, "This is a test value", 20);
	nopoll_conn_mask_content (ctx, buffer, 20, mask, 0);

	if (nopoll_ncmp (buffer, "This is a test value", 20)) {
		printf ("ERROR: expected to find different values after masking but found the same..\n");
		return nopoll_false;
	}

	/* revert changes */
	nopoll_conn_mask_content (ctx, buffer, 20, mask, 0);

	if (! nopoll_ncmp (buffer, "This is a test value", 20)) {
		printf ("ERROR: expected to find SAME values after masking but found the same..\n");
		return nopoll_false;
	} /* end if */

	/* now check transfering these values to the mask */
	if (nopoll_get_32bit (mask) != mask_value) {
		printf ("ERROR: found failure while reading the mask from from buffer..\n");
		return nopoll_false;
	}
	printf ("Test 01 masking: found mask in the buffer %d == %d\n", 
		nopoll_get_32bit (mask), mask_value);

	nopoll_ctx_unref (ctx);
	return nopoll_true;
}

nopoll_bool test_01 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
	        printf ("ERROR: Expected to find proper client connection status, but found error (conn=%p, conn->session=%d, NOPOLL_INVALID_SOCKET=%d)..\n",
			conn, (int) nopoll_conn_socket (conn), (int) NOPOLL_INVALID_SOCKET);
		return nopoll_false;
	}

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 1) {
		printf ("ERROR: expected to find 1 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	/* ensure connection status is ok */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR (3): expected to find proper connection status, but found failure.. (conn=%p, conn->session=%d, NOPOLL_INVALID_SOCKET=%d)..\n",
			conn, (int) nopoll_conn_socket (conn), (int) NOPOLL_INVALID_SOCKET);
		return nopoll_false;
	}

	printf ("Test 01: reference counting for the connection: %d\n", nopoll_conn_ref_count (conn));

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4.1 jkd412): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
				(int) nopoll_conn_socket (conn), errno, strerror (errno));
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_02 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollMsg  * msg;
	int          iter;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error.. (conn=%p, conn->session=%d, NOPOLL_INVALID_SOCKET=%d, errno=%d, strerr=%s)..\n",
			conn, (int) nopoll_conn_socket (conn), (int) NOPOLL_INVALID_SOCKET, errno, strerror (errno));
		return nopoll_false;
	}

	printf ("Test 02: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	iter = 0;
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: received websocket connection close during wait reply..\n");
			return nopoll_false;
		}

		nopoll_sleep (10000);

		if (iter > 10)
			break;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp ((char*) nopoll_msg_get_payload (msg), "This is a test")) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			(const char *) nopoll_msg_get_payload (msg));
		return nopoll_false;
	} /* end if */

	/* unref message */
	nopoll_msg_unref (msg);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_02a (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollMsg  * msg;
	int          iter;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	printf ("Test 02a: connecting IPv6 (::1:%s)..\n", regtest_port (2234));
	conn = nopoll_conn_new6 (ctx, "::1", regtest_port (2234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error.. (conn=%p, conn->session=%d, NOPOLL_INVALID_SOCKET=%d, errno=%d, strerr=%s)..\n",
			conn, (int) nopoll_conn_socket (conn), (int) NOPOLL_INVALID_SOCKET, errno, strerror (errno));
		return nopoll_false;
	}

	printf ("Test 02a: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	iter = 0;
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: received websocket connection close during wait reply..\n");
			return nopoll_false;
		}

		nopoll_sleep (10000);

		if (iter > 10)
			break;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp ((char*) nopoll_msg_get_payload (msg), "This is a test")) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			(const char *) nopoll_msg_get_payload (msg));
		return nopoll_false;
	} /* end if */

	/* unref message */
	nopoll_msg_unref (msg);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_02b (void) {
	
	noPollCtx  * ctx;
	noPollConn * conn;

	/* create context */
	ctx = create_ctx ();

	/* call to create a connection */
	printf ("Test 02-b: creating connection localhost:%s (errno=%d)\n", regtest_port (1234), errno);
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error.. (conn=%p, conn->session=%d, NOPOLL_INVALID_SOCKET=%d, errno=%d, strerr=%s)..\n",
			conn, (int) nopoll_conn_socket (conn), (int) NOPOLL_INVALID_SOCKET, errno, strerror (errno));
		return nopoll_false;
	}

	printf ("Test 02-b: waiting until connection is ok (errno=%d)\n", errno);
	if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
		printf ("ERROR: failed to fully establish connection nopoll_conn_wait_until_connection_ready (conn, 5) failed..\n");
	}

	/* sending echo */
	if (! test_sending_and_check_echo (conn, "Test 02-b", "This is a test"))
		return nopoll_false;

	printf ("Test 02-b: connection ready, sending PING frame (errno=%d)..\n", errno);
	if (! nopoll_conn_send_ping (conn)) {
		printf ("ERROR: failed to send ping frame..\n");
		return nopoll_false;
	} /* end if */

	/* sending echo */
	if (! test_sending_and_check_echo (conn, "Test 02-b", "This is a test"))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}


nopoll_bool test_03 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;

	memset (buffer, 0, 1024);

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 03: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 1024, blocking and with a 3 seconds timeout) */
	printf ("Test 03: now reading reply..\n");
	bytes_read = nopoll_conn_read (conn, buffer, 14, nopoll_true, 3000);
	
	if (bytes_read != 14) {
		printf ("ERROR: expected to find 14 bytes but found %d..\n", bytes_read);
		return nopoll_false;
	} /* end if */

	/* check content received */
	if (! nopoll_ncmp (buffer, "This is a test", 14)) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			buffer);
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_04 (int chunk_size) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;
	FILE       * file;
	struct stat  stat_buf;
	int          total_read = 0;
	const char * cmd;
	int          retries = 0;

	/* create context */
	ctx = create_ctx ();

	printf ("Test 04: running test with chunk_size=%d\n", chunk_size);

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	printf ("Test 04: creating connection to download file..\n");

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 04: sending get-file..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "get-file", 8) != 8) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

#if defined(NOPOLL_OS_WIN32)
	file = fopen ("tmp", "wb");
#else
	file = fopen ("tmp", "w");
#endif
	if (file == NULL) {
		printf ("ERROR: unable to open file tmp for content comparision\n");
		return nopoll_false; 
	} /* end if */

	stat ("nopoll-regression-client.c", &stat_buf);

	printf ("Test 04: stat file (nopoll-regression-client.c = %d bytes)\n", (int) stat_buf.st_size);

	retries = 0;
	while (total_read < stat_buf.st_size) {
		/* wait for the reply (try to read 1024, blocking) */
		bytes_read = nopoll_conn_read (conn, buffer, chunk_size, nopoll_true, 1000);
		/* printf ("Test 04: read %d bytes over the connection %d\n", bytes_read, nopoll_conn_get_id (conn));  */

		if (bytes_read < 0) {
			printf ("ERROR: expected to find bytes from the connection but found: %d\n", bytes_read);
			return nopoll_false;
		}

		if (bytes_read == 0) {
			retries ++;
			if (retries > 100) {
				printf ("Test 04: nothing found (0 bytes), total read %d, total requested: %ld, for %d retries\n", 
					total_read, (long) stat_buf.st_size, retries); 
				return nopoll_false;
			} /* end if */
			continue;
		} /* end if */

		/* write content */
		if (fwrite (buffer, 1, bytes_read, file) != bytes_read)
		  return nopoll_false;
	
		/* count total read bytes */
		total_read += bytes_read;

	} /* end while */
	fclose (file);

	/* now check both files */
	printf ("Test 04: checking content download (chunk_size=%d)...\n", chunk_size);
	printf ("Test 04: about to run diff nopoll-regression-client.c tmp > /dev/null\n");
#if defined(NOPOLL_OS_WIN32)	
	cmd = "diff -q nopoll-regression-client.c tmp";
#else
	cmd = "diff -q nopoll-regression-client.c tmp > /dev/null";
#endif
	if (system (cmd)) {
		printf ("ERROR: failed to download file from server, content differs. Check: diff nopoll-regression-client.c tmp\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_04a (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          result;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* attempt to read without blocking */
	printf ("Test 04-a: checking non-blocking API..\n");
	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 0);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}
		
	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 300);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);

	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 1000);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);
	

	return nopoll_true;
}

nopoll_bool test_04b (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	int          iterator;
	int          length;
	int          bytes_written;
	const char * msg = "1234-1) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-2) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-3) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-4) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-5) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-6) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-7) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-8) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-9) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-10) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-11) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-12) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-13) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw 1234-14) klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw";

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 04-b: waiting until connection is ok\n");
	nopoll_conn_wait_until_connection_ready (conn, 5);

	printf ("Test 04-b: sending was quick as possible to flood local buffers..\n");
	
	/* get message length */
	length = strlen (msg);
	iterator = 0;
	while (iterator < 100) {
		/* send a message */
		if (nopoll_conn_send_text (conn, msg, length) != length) {
			if (errno == 0) {
				printf ("ERROR: expected to find errno value but found 0..\n");
			}
			printf ("Test 04-b: found expected error, checking errno=%d..\n", errno);
			break;
		} /* end if */

		/* next iterator */
		iterator ++;
	}  /* end while */

#if !defined(ETIMEDOUT)
#define ETIMEDOUT 0
#endif

	if (errno != NOPOLL_EWOULDBLOCK && errno != EINPROGRESS && errno != ETIMEDOUT) {
		printf ("ERROR: expected to find errno=%d, but found errno=%d : %s\n",
			(int)NOPOLL_EWOULDBLOCK, (int)errno, strerror (errno));
		return nopoll_false;
	} /* end if */

	/* write pending content */
	if (nopoll_conn_pending_write_bytes (conn) == 0) {
	  printf ("WARNING: expected to have pending bytes to be written.. but found %d..\n",
		  nopoll_conn_pending_write_bytes (conn));
	  /* return nopoll_false; */
	} /* end if */

	iterator = 0;
	while (iterator < 10) {
		printf ("Test 04-b: found pending write bytes=%d\n", nopoll_conn_pending_write_bytes (conn));

		/* call to flush bytes */
		nopoll_conn_complete_pending_write (conn);

		if (nopoll_conn_pending_write_bytes (conn) == 0) {
			printf ("Test 04-b: all bytes written..\n");
			break;
		} /* end if */

		/* sleep a bit */
		nopoll_sleep (1000000);

		/* next iterator */
		iterator++;
	} 

	if (nopoll_conn_pending_write_bytes (conn) != 0) {
		printf ("Test 04-b: expected to find no pending bytes waiting to be written but found: %d\n", nopoll_conn_pending_write_bytes (conn));
		return nopoll_false;
	} /* end if */

	nopoll_conn_close (conn);

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 04-b: waiting until connection is ok\n");
	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* send a cleanup message */
	bytes_written = nopoll_conn_send_text (conn, "release-message", 15);
	if (bytes_written != 15) {
		printf ("Test 04-b: unable to send release message, bytes_written=%d, but expected=%d..\n",
			bytes_written, 15);
		return nopoll_false;
	} /* end if */

	printf ("Test 04-b: waiting a second before finishing test..\n");
	nopoll_sleep (1000000);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_04c (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	int          length;
	int          bytes_written, bytes_written_orig;
	char         buffer[4096];
	FILE       * handle;
	struct stat  file_info;
	int          iterator;
	char       * cmd;
	const char * file_checked;
	const char * cmd_format;
	int          total_bytes = 0;
	nopoll_bool  flush_required = nopoll_false;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 04-c: waiting until connection is ok\n");
	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* remove local file */
	if (stat ("copy-test-04c.txt", &file_info) == 0) {
		printf ("Test 04-c: FILE exists, removing (copy-test-04c.txt)\n");
		/* remove file */
		unlink ("copy-test-04c.txt");
	} /* end if */

	/* open file descriptor */
	bytes_written = nopoll_conn_send_text (conn, "open-file: copy-test-04c.txt", 28);
	if (bytes_written != 28) {
		printf ("Test 04-c: unable to send open file command, bytes_written=%d, but expected=%d..\n",
			bytes_written, 15);
		return nopoll_false;
	} /* end if */	

	/* open the handle to send the content */
	file_checked = "/boot/vmlinuz-2.6.32-5-amd64";
#if defined(NOPOLL_OS_WIN32)
	handle = fopen (file_checked, "rb");
#else
	handle = fopen (file_checked, "r");
#endif
	if (handle == NULL) {
		/* checking file */
		file_checked = "nopoll-regression-client.c";
#if defined(NOPOLL_OS_WIN32)
		handle = fopen (file_checked, "rb");
#else
		handle = fopen (file_checked, "r");
#endif
		if (handle == NULL) {
			printf ("Test 04-c: failed to open file to be sent to the server..\n");
			return nopoll_false;
		} /* end if */
	} /* end if */
	printf ("Test 04-c: running test with file: %s\n", file_checked);

	/* send content */
	while (nopoll_true) {
		/* read content */
		length = fread (buffer, 1, 4096, handle);

		/* write content */
		if (length > 0) {
			bytes_written = nopoll_conn_send_text (conn, buffer, length);

			/* check for flush required */
			if (nopoll_conn_pending_write_bytes (conn) > 0)
				flush_required = nopoll_true;

			if (bytes_written != length) {
				/* check pending bytes plus bytes
				   written equals to requested bytes
				   (length) */
				if ((nopoll_conn_pending_write_bytes (conn) + bytes_written) != length) {
					printf ("ERROR: after bytes_written(%d) = nopoll_conn_send_text (conn, buffer, length(%d)), but nopoll_conn_pending_write_bytes (conn)=%d do not match\n",
						bytes_written, length, nopoll_conn_pending_write_bytes (conn));
					return nopoll_false;

				} /* end if */

				printf ("Test 04-c: requesting to flush (bytes_written=%d, requested=%d, pending=%d)\n", 
					bytes_written, length, nopoll_conn_pending_write_bytes (conn));
				
				/* call to flush writes */
				bytes_written_orig = bytes_written;
				bytes_written      = nopoll_conn_flush_writes (conn, 10000000, bytes_written);
			}

			if (bytes_written != length) {
				printf ("ERROR: Failed to flush bytes read from file %d, bytes written were=%d, bytes written after flushing=%d (errno=%d : %s, pending bytes: %d, total bytes: %d)..\n",
					length, bytes_written_orig, bytes_written, errno, strerror (errno), nopoll_conn_pending_write_bytes (conn), total_bytes);
				
				return nopoll_false;
			} /* end if */
		} /* end if */

		if (bytes_written > 0)
			total_bytes += bytes_written;

		if (length < 4096) {
			printf ("Test 04-c: last read operation found length=%d\n", length);
			break;
		} /* end if */
	} /* end while */

	fclose (handle);

	printf ("Test 04-c: pending bytes to be written are=%d\n", nopoll_conn_pending_write_bytes (conn));

	/* send command to close file */
	bytes_written = nopoll_conn_send_text (conn, "close-file", 10);
	if (bytes_written != 10) {
		printf ("Test 04-c: unable to send close file command\n");
		return nopoll_false;
	} /* end if */	

#if defined(NOPOLL_OS_WIN32)	
	cmd_format = "diff -q copy-test-04c.txt %s";
#else
	cmd_format = "diff -q copy-test-04c.txt %s > /dev/null";
#endif
	cmd = nopoll_strdup_printf (cmd_format, file_checked);

	iterator = 0;
	while (iterator < 50) {
		/* checking file transferred */
		printf ("Test 04-c: checking file transfered, iterator=%d..\n", iterator);
		if (system (cmd) == 0) 
			break;

		iterator++;
		nopoll_sleep (500000);
	} /* end if */

	if (system (cmd) != 0) {
		printf ("Test 04-c: file differs, test failing, run: diff copy-test-04c.txt %s\n", file_checked);
		return nopoll_false;
	} /* end if */

	/* check total size */
	if (stat ("copy-test-04c.txt", &file_info) == 0 && file_info.st_size != total_bytes) {
		printf ("Test 04-c: expected to find same total bytes written %d != %d\n",
			(int) file_info.st_size, (int) total_bytes);
		return nopoll_false;
	} /* end if */

	if (stat (file_checked, &file_info) == 0 && file_info.st_size != total_bytes) {
		printf ("Test 04-c: expected to find same total bytes written %d != %d\n",
			(int) file_info.st_size, (int) total_bytes);
		return nopoll_false;
	} /* end if */

	printf ("Test 04-c: file ok (%d bytes written)..\n", total_bytes);
	nopoll_free (cmd);

	if (! flush_required) {
		printf (" *** \n");
		printf (" *** \n");
		printf (" *** ATTENTION: !! Flush operations weren't required so this test didn't check everything (file used to check transfers was: %s)  \n",
			file_checked);
		printf (" *** \n");
		printf (" *** \n");
	}

	/* sleep half a second */
	nopoll_sleep (500000);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_05 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;
	const char * msg = " klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw";

	memset (buffer, 0, 1024);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 05: sending UTF-8 content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, msg, -1) <= 0) {
		printf ("ERROR: Expected to find proper send operation (nopoll_conn_send_test) returned less or 0..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 322, blocking and with a 3 seconds timeout) */
	bytes_read = nopoll_conn_read (conn, buffer, 322, nopoll_true, 3000);
	if (bytes_read != 322) {
		printf ("ERROR: expected to receive 322 bytes, but received %d\n", bytes_read);
		return nopoll_false;
	}

	if (! nopoll_ncmp (buffer, msg, 322)) {
		printf ("ERROR: expected to receive another content....\n");
		printf ("Expected: %s\n", msg);
		printf ("Received: %s\n", buffer);

		return nopoll_false;
	}

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_06 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4.1 jg72): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
				(int) nopoll_conn_socket (conn), errno, strerror (errno));
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	if (! nopoll_conn_is_tls_on (conn)) {
		printf ("ERROR (5): expected to find TLS enabled on the connection but found it isn't..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_06a (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new6 (ctx, opts, "::1", regtest_port (2235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4.1 jg72): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
				(int) nopoll_conn_socket (conn), errno, strerror (errno));
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	if (! nopoll_conn_is_tls_on (conn)) {
		printf ("ERROR (5): expected to find TLS enabled on the connection but found it isn't..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_07 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4.1 dk45): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
				(int) nopoll_conn_socket (conn), errno, strerror (errno));
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	printf ("Test 07: testing sending TLS content over the wire..\n");
	if (! test_sending_and_check_echo (conn, "Test 07", "This is a test"))
		return nopoll_false;

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_08 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to connect to TLS port expecting non-TLS protocol */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected a FAILING connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_09 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* setup the protocol version to see how it breaks (it should) */
	nopoll_ctx_set_protocol_version (ctx, 12);

	/* call to connect to TLS port expecting non-TLS protocol */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected a FAILING connection status due to protocol version error, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_10 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to connect from an origining that shouldn't be allowed */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, "http://deny.aspl.es");

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected a FAILING connection status due to origing denied, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_11 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* create a working connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);

	if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
		printf ("ERROR: Expected a FAILING connection status due to origing denied, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish */
	nopoll_ctx_unref (ctx);

	/* finish connection */
	nopoll_conn_close (conn);
	
	return nopoll_true;
}

nopoll_bool test_12 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;
	int          iterator;

	/* time tracking */
	struct  timeval    start;
	struct  timeval    stop;
	struct  timeval    diff;


	/* reinit again */
	ctx = create_ctx ();

	/* start */
#if defined(NOPOLL_OS_WIN32)
	nopoll_win32_gettimeofday (&start, NULL);
#else
	gettimeofday (&start, NULL);
#endif	

	printf ("Test 12: creating 4000 connections...\n");
	iterator = 0;
	while (iterator < 4000) {
		/* create a working connection */
		conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
		
		if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
			printf ("ERROR: Expected NOT to find a FAILING connection status, errno is=%d..\n", errno);
			return nopoll_false;
		} /* end if */

		/* finish connection */
		nopoll_conn_close (conn);

		iterator++;
	} /* end while */

	/* finish */
	nopoll_ctx_unref (ctx);

	/* stop */
#if defined(NOPOLL_OS_UNIX)
	gettimeofday (&stop, NULL);
#else
	nopoll_win32_gettimeofday (&stop, NULL);
#endif

	nopoll_timeval_substract (&stop, &start, &diff);

	printf ("Test 12: created %d connections in %ld.%ld secs\n", 
		iterator, (long) diff.tv_sec, (long) diff.tv_usec);
	
	
	return nopoll_true;
}

nopoll_bool test_13_test (noPollCtx * ctx, const char * serverName, const char * _certificateFile, const char * _privateKey)
{
	const char * certificateFile;
	const char * privateKey;

	if (! nopoll_ctx_find_certificate (ctx, serverName, NULL, NULL, NULL)) {
		printf ("Test 13: it SHOULD find something about found.server.com but function reported failure status..\n");
		return nopoll_false;
	}

	if (! nopoll_ctx_find_certificate (ctx, serverName, &certificateFile, &privateKey, NULL)) {
		printf ("Test 13: it SHOULD find something about found.server.com but function reported failure status..\n");
		return nopoll_false;
	}

	if (! nopoll_cmp (certificateFile, _certificateFile)) {
		printf ("Test 13: expected to find certificate %s, but found %s\n", _certificateFile, certificateFile);
		return nopoll_false;
	}
	if (! nopoll_cmp (privateKey, _privateKey)) {
		printf ("Test 13: expected to find certificate %s, but found %s\n", _privateKey, privateKey);
		return nopoll_false;
	}
	return nopoll_true;
}

nopoll_bool test_13 (void)
{
	noPollCtx * ctx;

	/* create ctx */
	ctx = nopoll_ctx_new ();

	if (nopoll_ctx_find_certificate (ctx, "not-found", NULL, NULL, NULL)) {
		printf ("Test 13: it shouldn't find anything but function reported ok status..\n");
		return nopoll_false;
	}

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "found.server.com", "test.crt", "test.key", NULL)) {
		printf ("Test 13: unable to install certificate...\n");
		return nopoll_false;
	} /* end if */

	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "another.server.com", "another.test.crt", "another.test.key", NULL)) {
		printf ("Test 13: unable to install certificate (another.server.com)...\n");
		return nopoll_false;
	} /* end if */


	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "another.server.com", "another.test.crt", "another.test.key")) 
		return nopoll_false;

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "other.server.com", "other.test.crt", "other.test.key", NULL)) {
		printf ("Test 13: unable to install certificate (another.server.com)...\n");
		return nopoll_false;
	} /* end if */

	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "another.server.com", "another.test.crt", "another.test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "other.server.com", "other.test.crt", "other.test.key"))
		return nopoll_false;

	/* NOTE: nopoll_conn.c always looks up certificates with
	 * serverName == NULL, so that path must be checked too.
	 *
	 * With only named certificates stored, the lookup must fall
	 * back to reporting the first certificate of the list */
	printf ("Test 13: checking lookup with serverName == NULL (fallback to first certificate)..\n");
	if (! test_13_test (ctx, NULL, "test.crt", "test.key"))
		return nopoll_false;

	/* now install a certificate not associated to any serverName:
	 * it must take precedence over the fallback above */
	if (! nopoll_ctx_set_certificate (ctx, NULL, "default.test.crt", "default.test.key", NULL)) {
		printf ("Test 13: unable to install certificate (no serverName)...\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 13: checking lookup with serverName == NULL (certificate without serverName)..\n");
	if (! test_13_test (ctx, NULL, "default.test.crt", "default.test.key"))
		return nopoll_false;

	/* and the named ones must keep on being found */
	if (! test_13_test (ctx, "another.server.com", "another.test.crt", "another.test.key"))
		return nopoll_false;

	/* check ctx API guards: these must not crash nor modify anything */
	printf ("Test 13: checking noPollCtx API guards with NULL references..\n");
	nopoll_ctx_set_protocol_version (NULL, 13);
	if (nopoll_ctx_conns (NULL) != -1) {
		printf ("Test 13: expected -1 when asking connections to a NULL context..\n");
		return nopoll_false;
	} /* end if */
	if (nopoll_ctx_ref_count (NULL) != -1) {
		printf ("Test 13: expected -1 when asking reference count to a NULL context..\n");
		return nopoll_false;
	} /* end if */

	/* release ctx */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_14 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollMsg  * msg;
	int          iter;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 14: sending partial frames (Hel..)..\n");
	if (nopoll_conn_send_text_fragment (conn, "Hel", 3) != 3) {
		printf ("ERROR: expected to be able to send Hel frame..\n");
		return nopoll_false;
	}
	printf ("Test 14: sending completing frame (..lo)..\n");
	if (nopoll_conn_send_text (conn, "lo", 2) != 2) {
		printf ("ERROR: expected to be able to send lo completion frame..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	iter = 0;
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: received websocket connection close during wait reply..\n");
			return nopoll_false;
		}

		nopoll_sleep (10000);

		if (iter > 10)
			break;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp ((char*) nopoll_msg_get_payload (msg), "Hello")) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			(const char *) nopoll_msg_get_payload (msg));
		return nopoll_false;
	} /* end if */

	/* unref message */
	nopoll_msg_unref (msg);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_15 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	while (nopoll_true) {
		if (nopoll_conn_is_ready (conn))
			break;
		printf ("Test 15: not ready yet..\n");
		nopoll_sleep (10000);
	} /* end if */

	printf ("Test 15: setting non-blocking state..\n");

	if (! nopoll_conn_set_sock_block (nopoll_conn_socket (conn), nopoll_false)) {
		printf ("ERROR: failed to configure non-blocking state to connection..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 15: attempting to read content..\n");

	/* wait for the reply */
	if (nopoll_conn_get_msg (conn)) {
		printf ("ERROR (1): expected to not be able to find a message..\n");
		return nopoll_false;
	}
	if (nopoll_conn_get_msg (conn)) {
		printf ("ERROR (2): expected to not be able to find a message..\n");
		return nopoll_false;
	}
	if (nopoll_conn_get_msg (conn)) {
		printf ("ERROR (3): expected to not be able to find a message..\n");
		return nopoll_false;
	}

	printf ("Test 15: reads finished..\n");

	/* check connection state */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected to find connection state ok, but failure found..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_16 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	int          iterator;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	while (nopoll_true) {
		if (nopoll_conn_is_ready (conn))
			break;
		printf ("Test 16: not ready yet..\n");
		nopoll_sleep (10000);
	} /* end if */

	iterator = 0;
	while (iterator < 10) {
		printf ("Test 16: send sleep in header content (waiting 1000 ms, iterator=%d)..\n", iterator);
		if (__nopoll_conn_send_common (conn, "This is a test", 14, nopoll_true, 400000, NOPOLL_TEXT_FRAME) != 14) {
			printf ("ERROR: failed to send content..\n");
			return nopoll_false;
		} /* end if */

		iterator++;
	} /* end while */

	printf ("Test 16: sends finished, now checking connection ..\n");

	nopoll_sleep (100000);

	/* check connection state */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected to find connection state ok, but failure found..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_17_send_and_receive_test (noPollCtx * ctx, noPollConn * conn, noPollConn * listener, 
					   const char * message, int length, nopoll_bool read_in_the_middle, 
					   nopoll_bool read_after_header, nopoll_bool read_after_mask) 
{
	char           buffer[1024];
	char           buffer2[1024];
	NOPOLL_SOCKET  _socket;
	char           mask[4];
	int            desp;
	int            value;

	memset (buffer, 0, 1024);
	memset (buffer2, 0, 1024);

	/* make it unblock */
	nopoll_conn_set_sock_block (nopoll_conn_socket (listener), nopoll_false);

	/* now send partial content */
	printf ("Test 17: 1. CLIENT >> sending normal message to test link..\n");
	if (nopoll_conn_send_text (conn, message, length) != length) {
		printf ("ERROR: expected to properly send all bytes but it wasn't possible..\n");
		return nopoll_false;
	} /* end if */

	/* read reply */
	printf ("Test 17: 2. SERVER << reading content..\n");
	if (nopoll_conn_read (listener, buffer, length, nopoll_true, 0) != length) {
		printf ("ERROR: expected read 22 bytes ...but there was a failure..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 17: 3. CLIENT >> sending partial content..\n");
	_socket = nopoll_conn_socket (conn);

	/* NOTE: the following values has to be 129 and 150. They has
	   be this way because they represent the right WebSocket
	   header initialization for this test */
	buffer[0] = 129; /*  buffer[0]) = 0 1 1 1  1 1 1 0 */
	buffer[1] = 150; /*  buffer[0]) = 0 1 1 1  1 1 1 1 */
	
	send (_socket, buffer, 2, 0);
	
	nopoll_show_byte (ctx, buffer[0], "CLIENT >> buffer[0]");
	nopoll_show_byte (ctx, buffer[1], "CLIENT >> buffer[0]");

	if (read_after_header) {
		nopoll_sleep (1000000);
		printf ("Test 17: 3.1 SERVER << Reading after header..\n");
		nopoll_conn_read (listener, buffer2, length, nopoll_false, 0);
	}

	/* send mask */
	printf ("Test 17: 4. CLIENT >> sending content..\n");
	mask[0] = 23;
	mask[1] = 24;
	mask[2] = 25;
	mask[3] = 26;
	send (_socket, mask, 4, 0);

	nopoll_show_byte (ctx, mask[0], "CLIENT >> mask[0]");
	nopoll_show_byte (ctx, mask[1], "CLIENT >> mask[1]");
	nopoll_show_byte (ctx, mask[2], "CLIENT >> mask[2]");
	nopoll_show_byte (ctx, mask[3], "CLIENT >> mask[3]");

	if (read_after_mask) {
		nopoll_sleep (1000000);
		printf ("Test 17: 4.1 SERVER << Reading after mask..\n");
		nopoll_conn_read (listener, buffer2, length, nopoll_false, 0);
	}

	memcpy (buffer, message, length);
	nopoll_conn_mask_content (ctx, buffer, length, mask, 0);

	send (_socket, buffer, 10, 0);
	printf ("Test 17: 5. CLIENT >> sent partial content...wait a bit (2 seconds)..\n");
	nopoll_sleep (2000000);

	desp = 0;
	if (read_in_the_middle) {
		printf ("Test 17: 5.1 SERVER << reading in the middle (10 bytes)\n");
		memset (buffer2, 0, 100);
		desp = nopoll_conn_read (listener, buffer2, length, nopoll_false, 0);
		if (desp != 10) {
			printf ("Test 17: failed to read some initial content (10), found %d bytes..\n", desp);
			return nopoll_false;
		} /* end if */

		printf ("Test 17: read %d bytes..\n", desp);
	} 

	printf ("Test 17: 6. CLIENT >> now send the rest..\n");
	send (_socket, buffer + 10, length - 10, 0);

	/* copy content into original buffer */
	if (read_in_the_middle) 
		memcpy (buffer, buffer2, desp);

	printf ("Test 17: 7. SERVER << now read the content received..\n");
	/* now read the content */
	value = nopoll_conn_read (listener, buffer + desp, length - desp, nopoll_true, 0);
	if (value != (length - desp)) {
		printf ("ERROR: expected to receive nopoll_conn_read (listener, buffer + desp, length - desp, nopoll_true, 0)=%d bytes but found something different (length - desp)=%d..\n",
			value, (length - desp));
		return nopoll_false;
	} /* end if */

	if (! nopoll_ncmp (buffer, message, length)) {
		printf ("ERROR: expected to receive test message but found: '%s'\n", buffer);
		return nopoll_false;
	}

	return nopoll_true;
}

nopoll_bool test_17 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConn     * listener, * master;

	/* reinit again */
	ctx = create_ctx ();

	/* create a listener */
	master = nopoll_listener_new (ctx, "0.0.0.0", "22351");
	printf ("Test 17: created master listener (conn-id=%d, status=%d)\n", 
		nopoll_conn_get_id (master), nopoll_conn_is_ok (master));
	if (! nopoll_conn_is_ok (master)) {
		printf ("ERROR: expected proper master listener at 0.0.0.0:%s creation but a failure was found..\n", regtest_port (2235));
		return nopoll_false;
	} /* end if */

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "22351", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */


	/* wait for the reply */
	printf ("Test 17: accepting listener..\n");
	listener = nopoll_conn_accept (ctx, master);

	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to find proper listener status (connection accepted), but found failure..\n");
		return nopoll_false;
	} /* end if */
	
	/** call test here **/
	printf ("Test 17: sending first 'This is a test message'..\n");
	if (! test_17_send_and_receive_test (ctx, conn, listener, "This is a test message", 22, 
					     /* read in the middle */
					     nopoll_false, 
					     /* read after the header */
					     nopoll_false, 
					     /* read after the mask */
					     nopoll_false))
		return nopoll_false;

	/** call test here **/
	printf ("Test 17: sending second 'This is a test message'..\n");
	if (! test_17_send_and_receive_test (ctx, conn, listener, "This is a test message", 22, 
					     /* read in the middle */
					     nopoll_true, 
					     /* read after the header */
					     nopoll_false, 
					     /* read after the mask */
					     nopoll_false))
		return nopoll_false;

	/** call test here **/
	printf ("Test 17: sending third 'This is a test message'..\n");
	if (! test_17_send_and_receive_test (ctx, conn, listener, "This is a test message", 22, 
					     /* read in the middle */
					     nopoll_false, 
					     /* read after the header */
					     nopoll_true, 
					     /* read after the mask */
					     nopoll_false))
		return nopoll_false;

	/** call test here **/
	printf ("Test 17: sending fourth 'This is a test message'..\n");
	if (! test_17_send_and_receive_test (ctx, conn, listener, "This is a test message", 22, 
					     /* read in the middle */
					     nopoll_false, 
					     /* read after the header */
					     nopoll_false, 
					     /* read after the mask */
					     nopoll_true))
		return nopoll_false;

	printf ("Test 17: closing connections..\n"); 
	nopoll_conn_close (listener);
	nopoll_conn_close (master);
	nopoll_conn_close (conn);

	printf ("Test 17: finishing context..\n");

	/* finish */
	nopoll_ctx_unref (ctx);

	/* report finish */
	return nopoll_true;
}

nopoll_bool test_18 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 18: waiting on nopoll_loop_wait (1 seconds => 1000000 microseconds)...\n");
	nopoll_loop_wait (ctx, 1000000);
	printf ("Test 18: waiting on nopoll_loop_wait (1 seconds => 1000000 microseconds)...\n");
	nopoll_loop_wait (ctx, 1000000);

	/* finish connection */
	nopoll_conn_close (conn);

	/* finish */
	nopoll_ctx_unref (ctx);

	/* report finish */
	return nopoll_true;
}

nopoll_bool test_19 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

#if defined (NOPOLL_HAVE_SSLv23_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	printf ("Test 19: testing SSLv23 connection...\n");

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_SSLV23);

	/* create connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1236), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: failed to start listener connection..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn, "Test 19", "This is a test...checking SSL with different values..."))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);
#endif	

#if defined (NOPOLL_HAVE_SSLv23_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	printf ("Test 19: testing SSLv23 connection with TLSv1 server...\n");

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_SSLV23);

	/* create connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("WARNING: failed to create connection (2)..unable to connect to TLSv1 server with NOPOLL_METHOD_SSLV23\n");
	} else {
		if (! test_sending_and_check_echo (conn, "Test 19", "This is a test...checking SSL with different values..."))
			return nopoll_false;
	} /* end if */

	nopoll_conn_close (conn);
#endif	

#if defined (NOPOLL_HAVE_SSLv3_ENABLED) && OPENSSL_VERSION_NUMBER < 0x10100000L
	printf ("Test 19: perfect, got it working (OPENSSL_VERSION_NUMBER=%ld)..\n", (OPENSSL_VERSION_NUMBER));

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_SSLV3);

	/* create connection */
	printf ("Test 19: checking SSLv3 with TLSv1..\n");
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);

	/* check connection */
	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected a connection failure..\n");
		return nopoll_false;
	} /* end if */
	printf ("   ... it does not work, but this is expected..\n");

	nopoll_conn_close (conn);
#endif	

	/*** 
	 * The following versions do not support the following test:
	 *
	 * - OPENSSL_VERSION_NUMBER=0x1000114fL  (jessie)
	 */
	
#if defined (NOPOLL_HAVE_TLSv10_ENABLED) && OPENSSL_VERSION_NUMBER < 0x1000114fL	
	printf ("Test 19: testing TLSv1.0 connection...(OPENSSL_VERSION_NUMBER=%ld)..\n", (OPENSSL_VERSION_NUMBER));

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1);

	/* create connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: failed to start listener connection..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn, "Test 19", "This is a test...checking SSL with different values..."))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);
#endif	

#if defined (NOPOLL_HAVE_TLSv11_ENABLED)
	printf ("Test 19: testing TLSv1.1 connection...\n");

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_1);

	/* create connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1238), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: failed to start listener connection..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn, "Test 19", "This is a test...checking SSL with different values..."))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);

	printf ("Test 19-a: testing TLSv1.1 (IPv6) connection...\n");

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_1);

	/* create connection */
	conn = nopoll_conn_tls_new6 (ctx, opts, "::1", regtest_port (2238), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: failed to start listener connection..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn, "Test 19a", "This is a test...checking SSL with different values..."))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);
	
#endif

#if defined (NOPOLL_HAVE_TLSv12_ENABLED)
	printf ("Test 19: testing TLSv1.2 connection...\n");

	/* create options */
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_protocol (opts, NOPOLL_METHOD_TLSV1_2);

	/* create connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1240), NULL, NULL, NULL, NULL);

	/* check connection */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: failed to start listener connection..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn, "Test 19", "This is a test...checking SSL with different values..."))
		return nopoll_false;
	
	/* finish connection */
	nopoll_conn_close (conn);
#endif

	/* finish */
	nopoll_ctx_unref (ctx);

	/* report finish */
	return nopoll_true;
}

#if defined(__NOPOLL_PTHREAD_SUPPORT__)
nopoll_bool test_20 (void) {

	noPollPtr  * mutex;
	int          iterator = 0;

	printf ("Test 20: checking default API mutex used by the tests..\n");
	while (iterator < 10) {
		/* call to create mutex */
		mutex = __nopoll_regtest_mutex_create ();
		if (mutex == NULL)
			return nopoll_false;
		
		/* call to lock */
		__nopoll_regtest_mutex_lock (mutex);
		__nopoll_regtest_mutex_unlock (mutex);
		
		/* call to destroy */
		__nopoll_regtest_mutex_destroy (mutex);

		/* next operation */
		iterator++;
	} /* end while */

	return nopoll_true;
}
#endif

/* 
 * Use gen-certificates-test-21.sh to rebuild certificates.
 * Reg test to check client auth certificate.
 */
nopoll_bool test_21 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	printf ("Test 21: check ssl connection (with auth certificate)..\n");
	conn = nopoll_conn_tls_new (ctx, NULL, "localhost", regtest_port (1239), NULL, NULL, NULL, NULL);
	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to FAILURE client connection status, but ok..\n");
		return nopoll_false;
	}
	nopoll_conn_close (conn);

	/* NOTE: the following test needs client.pem, server.pem and
	 * root.epm used by nopoll-regression-client.c and
	 * nopoll-regression-listener.c to be synchronized and
	 * updated. 
	 *
	 * If you have problems running this test, run it with:
	 *
	 * >> ./nopoll-regression-client --show-critical-only
	 *
	 * or with full debug enabled:
	 *
	 * >> ./nopoll-regression-client --debug
	 *
	 * There is a script that allows to generate and refresh
	 * certificates used by this test:
	 *
	 * >> ./gen-certificates-test-21.sh
	 *
	 * You can use to see how to generate a root.pem CA
	 * certificate an a couple of certificates for client.pem and
	 * server.pem
	 */
	
	/* try again configuring conection certificates*/
	printf ("Test 21: checking to connect again with client provided certificates..\n");
	opts     = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_ssl_certs (opts, 
					/* certificate */
					"client.pem",
					/* private key */
					"client.pem",
					NULL,
					/* ca certificate */
					"root.pem");
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1239), NULL, NULL, NULL, NULL);
	if (! test_sending_and_check_echo (conn, "Test 21", "This is a test")) {
		printf ("ERROR: it should WORK, client certificate isn't working..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}


nopoll_bool __test_22_on_close_signal = nopoll_false;

void __test_22_on_close (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	printf ("Test --: called on connection close for conn-id=%d\n", nopoll_conn_get_id (conn));
	__test_22_on_close_signal = nopoll_true;

	return;
}

nopoll_bool test_22 (void) {
	
	noPollCtx      * ctx;
	noPollConn     * conn;
	NOPOLL_SOCKET    _socket;
	noPollMsg      * msg;
	noPollConnOpts * opts;

	printf ("Test 22: testing connection close notification for regular connections (client side)..\n");

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* set connection close */
	nopoll_conn_set_on_close (conn, __test_22_on_close, NULL);

	/* call to close connection as we had lost the connection */
	_socket = nopoll_conn_socket (conn);
	nopoll_close_socket (_socket);

	/* call to get content (we shouldn't get anythign) */
	msg = nopoll_conn_get_msg (conn);
	if (msg) {
		printf ("ERROR: we shouldn't get a msg frame, but a well defined pointer was found..\n");
		return nopoll_false;
	} /* end if */

	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: we shouldn't get an ok value from nopoll_conn_is_ok (conn)..\n");
		return nopoll_false;
	} /* end if */

	if (! __test_22_on_close_signal) {
		printf ("ERROR: connection close should've been called but it wasn't..\n");
		return nopoll_false;
	} /* end if */

	/* close the connection */
	nopoll_conn_close (conn);

	__test_22_on_close_signal = nopoll_false;
	printf ("Test 22: test close connection close notification for WSS:// (ssl connections), (client side)..\n");

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* set connection close */
	nopoll_conn_set_on_close (conn, __test_22_on_close, NULL);

	/* call to close connection as we had lost the connection */
	_socket = nopoll_conn_socket (conn);
	nopoll_close_socket (_socket);

	/* call to get content (we shouldn't get anythign) */
	msg = nopoll_conn_get_msg (conn);
	if (msg) {
		printf ("ERROR: we shouldn't get a msg frame, but a well defined pointer was found..\n");
		return nopoll_false;
	} /* end if */

	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: we shouldn't get an ok value from nopoll_conn_is_ok (conn)..\n");
		return nopoll_false;
	} /* end if */

	if (! __test_22_on_close_signal) {
		printf ("ERROR: connection close should've been called but it wasn't..\n");
		return nopoll_false;
	} /* end if */

	/* close the connection */
	nopoll_conn_close (conn);
	

	nopoll_ctx_unref (ctx);
	return nopoll_true;
} /* end if */

int test_23_get_connection_close_count (noPollCtx * ctx, noPollConn * conn) {
	
	int         count_before_closing;
	noPollMsg * msg;

	/* wait for the reply */
	while (nopoll_true) {
		if (nopoll_conn_is_ready (conn))
			break;
		nopoll_sleep (10000);
	} /* end if */

	/* send package to get number of connection close detected */
	if (nopoll_conn_send_text (conn, "get-connection-close-count", 26) != 26) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}
	
	/* call to get content (we shouldn't get anythign) */
	while (nopoll_true) {
		msg = nopoll_conn_get_msg (conn);
		if (msg)
			break;

		nopoll_sleep (10000);
	} /* end if */


	count_before_closing = strtod ((const char *) nopoll_msg_get_payload (msg), NULL);
	printf ("Test 23: Message received: %d..\n", count_before_closing);
	/* release message */
	nopoll_msg_unref (msg);

	return count_before_closing;
}

nopoll_bool test_23 (void) {
	
	noPollCtx      * ctx;
	noPollConn     * conn;
	int              count_before_closing;
	int              count_before_closing2;
	noPollConnOpts * opts;

	printf ("Test 23: testing connection close notification for regular connections (client side)..\n");

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* get connection close before closing */
	if ((count_before_closing = test_23_get_connection_close_count (ctx, conn)) == -1)
		return nopoll_false;
	printf ("Test 23: current connection close is: %d\n", count_before_closing);

	/* close the connection cleanly and check connection close is not called  */
	nopoll_conn_close (conn);

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	if ((count_before_closing2 = test_23_get_connection_close_count (ctx, conn)) == -1)
		return nopoll_false;
	printf ("Test 23: current connection close is: %d\n", count_before_closing2);

	if (count_before_closing == count_before_closing2) {
		printf ("ERROR: expected connection close notification ...but same values were found..\n");
		return nopoll_false;
	} /* end if */

	/* close the connection cleanly and check connection close is not called  */
	nopoll_conn_close (conn);

	printf ("Test 23: now test TLS connections connection close..\n");

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* get connection close before closing */
	if ((count_before_closing = test_23_get_connection_close_count (ctx, conn)) == -1)
		return nopoll_false;

	/* close the connection */
	nopoll_conn_close (conn);

	/* call to create a connection second connection */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	if ((count_before_closing2 = test_23_get_connection_close_count (ctx, conn)) == -1)
		return nopoll_false;

	/* close the connection */
	nopoll_conn_close (conn);

	if (count_before_closing == count_before_closing2) {
		printf ("ERROR: expected connection close notification ...but same values were found..\n");
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);
	return nopoll_true;
} /* end if */

nopoll_bool test_24 (void) {
	
	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;
	noPollMsg      * msg;

	printf ("Test 24: test cookie support (set client and receive on server..)\n");

	/* init context */
	ctx = create_ctx ();

	/* configure cookie */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_cookie (opts, "theme=light; sessionToken=abc123");

	/* create connection */
	conn = nopoll_conn_new_opts (ctx, opts, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* send package to get number of connection close detected */
	if (nopoll_conn_send_text (conn, "get-cookie", 10) != 10) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}
	
	/* call to get content (we shouldn't get anythign) */
	while (nopoll_true) {
		msg = nopoll_conn_get_msg (conn);
		if (msg)
			break;

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: connection failure found during message wait..\n");
			return nopoll_false;
		}

		nopoll_sleep (10000);
	} /* end if */

	printf ("Test 24: received header set on server side: %s\n", nopoll_msg_get_payload (msg));
	if (! nopoll_cmp ((const char *) nopoll_msg_get_payload (msg), "theme=light; sessionToken=abc123")) {
		printf ("ERROR: expected to receive different header, error was: %s\n", nopoll_msg_get_payload (msg));
		return nopoll_false;
	}

	/* release message */
	nopoll_msg_unref (msg);


	/* close the connection */
	nopoll_conn_close (conn);


	nopoll_ctx_unref (ctx);
	return nopoll_true;
} /* end if */

nopoll_bool test_25_check_cookie (noPollCtx * ctx, const char * cookie) {
	noPollConn     * conn;
	noPollConnOpts * opts;

	/* configure cookie */
	opts = nopoll_conn_opts_new ();

	/* set a cookie bigger than 1044 */
	nopoll_conn_opts_set_cookie (opts, cookie);

	/* create connection */
	conn = nopoll_conn_new_opts (ctx, opts, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}


	/* close the connection */
	nopoll_conn_close (conn);	

	return nopoll_true;
}

nopoll_bool test_25 (void) {
	
	noPollCtx      * ctx;

	printf ("Test 25: test cookie support (set client and receive on server..)\n");

	/* init context */
	ctx = create_ctx ();

	if (! test_25_check_cookie (ctx, "theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b theme=light; sessionToken=abc123 lkjsadfkljasdf lkjaseflkawjet klajw glkajy240u 4234lkj y3j 3q5yñkl aegar glkejry b "))
		return nopoll_false;


	if (! test_25_check_cookie (ctx, "222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444asd35555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555affffffffff-23345"))
		return nopoll_false;

	if (! test_25_check_cookie (ctx, "222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444asd35555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555affffffffff-233"))
		return nopoll_false;

	if (! test_25_check_cookie (ctx, "222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444asd35555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555afffffff"))
		return nopoll_false;

	if (! test_25_check_cookie (ctx, "22222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444222222222222222222222211111111111111111111111133333333333333333333333334444444444444444444444444asd35555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555afffffff"))
		return nopoll_false;

	nopoll_ctx_unref (ctx);
	return nopoll_true;
} /* end if */

nopoll_bool test_26 (void) {

	noPollConn     * conn;
	noPollCtx      * ctx;
	int              tries;
	nopoll_bool      result;

	/* init context */
	ctx = create_ctx ();

	/* NOTE: this test connects to an external service
	 * (echo.websocket.org) which is out of our control: it may be
	 * unreachable, moved or replying a redirect. Because of that,
	 * the test reports SKIPPED instead of failing when the
	 * connection cannot be completed: otherwise a third party site
	 * breaks the whole regression test */
	conn = nopoll_conn_new (ctx, "echo.websocket.org", "80", NULL, NULL, NULL, NULL);

	/* wait a bit for the handshake to be completed */
	tries = 100; /* 100 x 100ms = 10 seconds */
	while (tries > 0 && nopoll_conn_is_ok (conn) && ! nopoll_conn_is_ready (conn)) {
		nopoll_sleep (100000);
		tries--;
	} /* end while */

	if (! nopoll_conn_is_ok (conn) || ! nopoll_conn_is_ready (conn)) {
		printf ("Test 26: SKIPPED: unable to reach external service echo.websocket.org (unreachable or changed)..\n");
		nopoll_conn_close (conn);
		nopoll_ctx_unref (ctx);
		return nopoll_true;
	} /* end if */

	/* check test */
	result = test_sending_and_check_echo (conn, "Test 26", "This is a test");

	/* close the connection */
	nopoll_conn_close (conn);

	/* release context */
	nopoll_ctx_unref (ctx);

	return result;
}

nopoll_bool test_27 (void) {

	noPollConn     * conn;
	noPollCtx      * ctx;

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, "/", "chat-protocol", "http://www.aspl.es");
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* check test */
	if (! test_sending_and_check_echo (conn, "Test 27", "This is a test"))
		return nopoll_false;

	/* check accepted protocol */
	if (! nopoll_cmp ("chat-protocol", nopoll_conn_get_accepted_protocol (conn))) {
		printf ("ERROR: expected to find [chat-protocol] but found something: %s\n", 
			nopoll_conn_get_accepted_protocol (conn));
		return nopoll_false;
	} /* end if */ 

	printf ("Test 27: accepted protocol by the server: %s\n", nopoll_conn_get_accepted_protocol (conn));

	/* close the connection */
	nopoll_conn_close (conn);	

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, "/", "hello-protocol", "http://www.aspl.es");
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* check test */
	if (! test_sending_and_check_echo (conn, "Test 27", "This is a test"))
		return nopoll_false;

	/* check accepted protocol */
	if (! nopoll_cmp ("hello-protocol-response", nopoll_conn_get_accepted_protocol (conn))) {
		printf ("ERROR: expected to find [chat-protocol] but found something: %s\n", 
			nopoll_conn_get_accepted_protocol (conn));
		return nopoll_false;
	} /* end if */ 

	printf ("Test 27: accepted protocol by the server: %s\n", nopoll_conn_get_accepted_protocol (conn));

	/* close the connection */
	nopoll_conn_close (conn);	


	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}


nopoll_bool test_28 (void) {

	noPollConn     * conn;
	noPollCtx      * ctx;
	noPollMsg      * msg;

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* wait until it is connected */
	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* send a message to request connection close with a particular message */
	if (nopoll_conn_send_text (conn, "close with message", 18) != 18) {
		printf ("ERROR: failed to send close with message..");
		return nopoll_false;
	} /* end while */

	/* wait for the reply */
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			/* connection was closed by remote side */
			break;
		} /* end if */

		nopoll_sleep (10000);
	} /* end if */

	printf ("Test 28: close reason received, statud=%d, message=%s\n", 
		nopoll_conn_get_close_status (conn),
		nopoll_conn_get_close_reason (conn));
	if (nopoll_conn_get_close_status (conn) != 1048) {
		printf ("ERROR: expected different error code..\n");
		return nopoll_false;
	}

	if (! nopoll_cmp (nopoll_conn_get_close_reason (conn), "Hey, this is a very reasonable error message")) {
		printf ("ERROR: expected different error message..\n");
		return nopoll_false;
	} /* end if */

	/* close connection */
	nopoll_conn_close (conn);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_29 (void) {

	noPollConn     * conn;
	noPollCtx      * ctx;
	noPollConnOpts * opts;

	/* init context */
	ctx = create_ctx ();

	/* configure extra headers */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_extra_headers (opts, "\r\nfoo: bar");

	/* create connection */
	conn = nopoll_conn_new_opts (ctx, opts, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* wait until it is connected */
	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* close connection */
	nopoll_conn_close (conn);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/**** DO NOT INCLUDE THIS HEADER IN PRODUCTION: this header is
      included in this regression test just for testing purposes. Any
      code developed using this include might failure in future
      relases ****/
#include <nopoll_private.h>

nopoll_bool test_30_common_header_stop (const char * label, int bytes_to_send_before_stop) {

	noPollConn      * conn;
	noPollCtx       * ctx;
	const char      * msg = "This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...This is content to check this regression test 30, content for a reply that send a header that is partially sent...";
	int              length;
	noPollMsg      * msg_ref;
	int              tries;

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test %s: waiting until connection is ready..\n", label);
	/* wait until it is connected */
	nopoll_conn_wait_until_connection_ready (conn, 5);
	printf ("Test %s: ok..\n", label);

	/* send a message to request connection close with a particular message */
	conn->__force_stop_after_header = bytes_to_send_before_stop;

	length = strlen (msg);

	printf ("Test %s: sending first message (of %d bytes, sending broken header of %d, pausing then, and then sending the rest..)\n",
		label, length,  bytes_to_send_before_stop);
		
	if (nopoll_conn_send_text (conn, msg, length) != length) {
		printf ("ERROR: failed to send message..");
		return nopoll_false;
	} /* end while */

	/* call to get status */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: connection failure found after send operation with broken header....\n");
		return nopoll_false;
	} /* end if */

	printf ("Test %s: getting reply to the message..\n", label);
	tries = 10;
	while (tries > 0 ) { 
		/* get message */
		msg_ref = nopoll_conn_get_msg (conn);
		if (msg_ref)
			break;

		nopoll_sleep (500000);
		tries--;
		
	}
	if (msg_ref == NULL) {
		printf ("ERROR: expected to find reply message...but NULL was received..\n");
		return nopoll_false;
	} /* end if */

	if (! nopoll_cmp ((const char *) nopoll_msg_get_payload (msg_ref), msg)) {
		printf ("ERROR: expected to find message equal content but found something different..\n");
		return nopoll_false;
	} /* end if */

	/* release message */
	nopoll_msg_unref (msg_ref);


	printf ("Test %s: sending second message (of %d bytes, sending broken header of %d, pausing then, and then sending the rest..)\n",
		label, length,  bytes_to_send_before_stop);
	
	if (nopoll_conn_send_text (conn, msg, length) != length) {
		printf ("ERROR: failed to send message..");
		return nopoll_false;
	} /* end while */

	printf ("Test %s: getting reply to the message (to the second message)..\n", label);
	tries = 10;
	while (tries > 0 ) { 
		/* get message */
		msg_ref = nopoll_conn_get_msg (conn);
		if (msg_ref)
			break;

		nopoll_sleep (500000);
		tries--;
		
	}
	if (msg_ref == NULL) {
		printf ("ERROR: expected to find reply message...but NULL was received..\n");
		return nopoll_false;
	} /* end if */

	if (! nopoll_cmp ((const char *) nopoll_msg_get_payload (msg_ref), msg)) {
		printf ("ERROR: expected to find message equal content but found something different..\n");
		return nopoll_false;
	} /* end if */

	/* release message */
	nopoll_msg_unref (msg_ref);

	/* close connection */
	nopoll_conn_close (conn);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_30 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("30", 2);
}

nopoll_bool test_31 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("31", 1);
}

nopoll_bool test_32 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("32", 3);
}

nopoll_bool test_33 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("33", 4);
}

nopoll_bool test_34 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("34", 5);
}

nopoll_bool test_35 (void) {
	/* call to test send 1 byte and stop */
	return test_30_common_header_stop ("35", 8);
}

nopoll_bool test_36 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;
	noPollConn * conn2;

	printf ("Test 36: force connection timeout artificially..\n");

	/* init again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: not expected connection error but found connection ok..\n");
		return nopoll_false;
	}

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "set-broken-socket", 17) != 17) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	nopoll_sleep (100000);
	
	printf ("Test 36: connecting again to ensure listener is working..\n");
	
	/* call to create a connection */
	conn2 = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn2)) {
		printf ("ERROR: not expected connection error but found connection ok..\n");
		return nopoll_false;
	}

	/* send content text(utf-8) */
	nopoll_conn_send_text (conn2, "This is a test", 14);

	/* finish connection */
	nopoll_conn_close (conn);
	nopoll_conn_close (conn2);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/**
 * @internal Common function to send a crafted websocket frame to the
 * listener, checking it is rejected and that the listener keeps on
 * working after that (see https://github.com/ASPLes/nopoll/issues/84).
 */
nopoll_bool test_37_common_crafted_frame (const char * label, const char * frame, int frame_size)
{
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollConn * conn2;
	int          tries;

	printf ("Test %s: sending crafted frame (%d bytes) announcing a wrong payload size..\n", label, frame_size);

	/* init context */
	ctx = create_ctx ();

	/* create connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* wait until the handshake is finished so the content sent is
	 * handled by the websocket engine at the listener */
	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* send the crafted frame directly over the socket to skip
	 * noPoll's own framing */
	if (send (nopoll_conn_socket (conn), frame, frame_size, 0) != frame_size) {
		printf ("ERROR: failed to send crafted frame to the listener..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test %s: waiting for the listener to close the connection..\n", label);
	tries = 20;
	while (tries > 0) {
		nopoll_conn_get_msg (conn);
		if (! nopoll_conn_is_ok (conn))
			break;
		nopoll_sleep (100000);
		tries--;
	} /* end while */

	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected the listener to close the connection after receiving a crafted frame, but it is still working..\n");
		return nopoll_false;
	} /* end if */

	nopoll_conn_close (conn);

	/* now check the listener is still working: without the fix
	 * the crafted frame crashes it */
	printf ("Test %s: connecting again to ensure listener is working..\n", label);
	conn2 = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn2)) {
		printf ("ERROR: expected proper connection after sending the crafted frame (listener gone?)..\n");
		return nopoll_false;
	} /* end if */

	if (! test_sending_and_check_echo (conn2, label, "This is a test to check the listener is alive"))
		return nopoll_false;

	nopoll_conn_close (conn2);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_37 (void) {

	/* masked binary frame announcing 0xFFFFFFFFFFFFFFFF as
	 * payload size: it used to produce a signed overflow that
	 * ended into a negative payload size */
	const char frame[] = {(char) 0x82, (char) 0xFF,
			      (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
			      (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
			      (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00};

	return test_37_common_crafted_frame ("37", frame, 14);
}

nopoll_bool test_38 (void) {

	/* masked binary frame announcing 0x00000000FFFFFFFF as
	 * payload size: this value is positive and representable but
	 * it gets truncated into -1 when it reaches the read
	 * function, so it must be rejected too */
	const char frame[] = {(char) 0x82, (char) 0xFF,
			      (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00,
			      (char) 0xFF, (char) 0xFF, (char) 0xFF, (char) 0xFF,
			      (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00};

	return test_37_common_crafted_frame ("38", frame, 14);
}

nopoll_bool test_39 (void) {

	/* masked binary frame announcing 32MB as payload size: it is
	 * a perfectly legal value but it is over the default limit
	 * (16MB), so the connection must be closed */
	const char frame[] = {(char) 0x82, (char) 0xFF,
			      (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00,
			      (char) 0x02, (char) 0x00, (char) 0x00, (char) 0x00,
			      (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00};

	return test_37_common_crafted_frame ("39", frame, 14);
}

nopoll_bool test_40 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;
	const char     * msg = "This is a message that is bigger than the limit configured for this connection..";
	int              tries;

	printf ("Test 40: checking max frame size configuration..\n");

	/* init context */
	ctx = create_ctx ();

	/* check default value */
	if (nopoll_ctx_get_max_frame_size (ctx) != NOPOLL_MAX_FRAME_SIZE_DEFAULT) {
		printf ("ERROR: expected to find default max frame size (%ld) but found %ld..\n",
			(long int) NOPOLL_MAX_FRAME_SIZE_DEFAULT, nopoll_ctx_get_max_frame_size (ctx));
		return nopoll_false;
	} /* end if */

	/* check wrong values are discarded */
	nopoll_ctx_set_max_frame_size (ctx, 0);
	nopoll_ctx_set_max_frame_size (ctx, -1);
	nopoll_ctx_set_max_frame_size (ctx, NOPOLL_MAX_FRAME_SIZE_LIMIT + 1);
	if (nopoll_ctx_get_max_frame_size (ctx) != NOPOLL_MAX_FRAME_SIZE_DEFAULT) {
		printf ("ERROR: expected wrong max frame size values to be discarded, but configuration changed to %ld..\n",
			nopoll_ctx_get_max_frame_size (ctx));
		return nopoll_false;
	} /* end if */

	/* configure a small limit at the context */
	nopoll_ctx_set_max_frame_size (ctx, 32);

	/* check connection options take precedence over the context */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_max_frame_size (opts, 64);

	conn = nopoll_conn_new_opts (ctx, opts, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	if (nopoll_conn_get_max_frame_size (conn) != 64) {
		printf ("ERROR: expected to find max frame size configured at the connection options (64) but found %ld..\n",
			nopoll_conn_get_max_frame_size (conn));
		return nopoll_false;
	} /* end if */

	nopoll_conn_wait_until_connection_ready (conn, 5);

	/* ask the listener to echo a message bigger than the limit
	 * configured: the reply must be rejected, closing the
	 * connection */
	printf ("Test 40: sending message to get a reply bigger than the limit configured (64)..\n");
	if (nopoll_conn_send_text (conn, msg, strlen (msg)) != (int) strlen (msg)) {
		printf ("ERROR: failed to send message..\n");
		return nopoll_false;
	} /* end if */

	tries = 20;
	while (tries > 0) {
		nopoll_conn_get_msg (conn);
		if (! nopoll_conn_is_ok (conn))
			break;
		nopoll_sleep (100000);
		tries--;
	} /* end while */

	if (nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected connection to be closed after receiving a frame bigger than the limit configured..\n");
		return nopoll_false;
	} /* end if */

	nopoll_conn_close (conn);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/**
 * @internal Same check done by test_14 but using the binary fragment
 * API: nopoll_conn_send_binary_fragment () must flag the frame with
 * FIN = 0 so the remote peer waits for the completion frame instead of
 * handling the piece as a complete message.
 */
nopoll_bool test_42 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollMsg  * msg;
	int          iter;
	nopoll_bool  result;

	/* create context */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 42: sending partial binary frame (Hel..)..\n");
	if (nopoll_conn_send_binary_fragment (conn, "Hel", 3) != 3) {
		printf ("ERROR: expected to be able to send Hel binary fragment..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 42: sending completing binary frame (..lo)..\n");
	if (nopoll_conn_send_binary (conn, "lo", 2) != 2) {
		printf ("ERROR: expected to be able to send lo completion frame..\n");
		return nopoll_false;
	} /* end if */

	/* wait for the reply */
	iter = 0;
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: received websocket connection close during wait reply..\n");
			return nopoll_false;
		} /* end if */

		nopoll_sleep (10000);
		iter++;

		if (iter > 100)
			break;
	} /* end while */

	if (msg == NULL) {
		printf ("ERROR: expected to find a reply from the listener but NULL was received..\n");
		return nopoll_false;
	} /* end if */

	/* check content received: getting "Hel" here means the
	 * fragment was sent with FIN = 1, so the listener handled it
	 * as a complete message instead of waiting for the rest */
	result = nopoll_cmp ((char *) nopoll_msg_get_payload (msg), "Hello");
	if (! result) {
		printf ("ERROR: expected to find message 'Hello' but something different was received: '%s'..\n",
			(const char *) nopoll_msg_get_payload (msg));
	} /* end if */

	/* unref message */
	nopoll_msg_unref (msg);

	/* finish connection */
	nopoll_conn_close (conn);

	/* finish */
	nopoll_ctx_unref (ctx);

	return result;
}

/**
 * @internal Checks the parameter validation paths of the listener
 * creation API, which were not covered by any test: they are the ones
 * that reject wrong host/port values before touching the network.
 */
nopoll_bool test_43 (void) {

	noPollCtx  * ctx;
	noPollConn * listener;
	noPollConn * conn;
	noPollConn * accepted;

	/* init context */
	ctx = create_ctx ();

	printf ("Test 43: checking listener creation with a NULL port..\n");
	/* NOTE: this used to evaluate strlen (NULL) and crash */
	listener = nopoll_listener_new (ctx, "0.0.0.0", NULL);
	if (listener != NULL) {
		printf ("ERROR: expected to fail creating a listener without port, but a listener was reported..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 43: checking listener creation with an empty port..\n");
	listener = nopoll_listener_new (ctx, "0.0.0.0", "");
	if (listener != NULL) {
		printf ("ERROR: expected to fail creating a listener with an empty port, but a listener was reported..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 43: checking listener creation with a NULL host..\n");
	listener = nopoll_listener_new (ctx, NULL, regtest_port (1251));
	if (listener != NULL) {
		printf ("ERROR: expected to fail creating a listener without host, but a listener was reported..\n");
		return nopoll_false;
	} /* end if */

	printf ("Test 43: checking IPv6 listener rejects an IPv4 address..\n");
	listener = nopoll_listener_new6 (ctx, "0.0.0.0", regtest_port (1251));
	if (listener != NULL) {
		printf ("ERROR: expected to fail creating an IPv6 listener over 0.0.0.0, but a listener was reported..\n");
		return nopoll_false;
	} /* end if */

	/* and after all those failures, a normal listener must keep on
	 * working (that is, nothing was left in a broken state) */
	printf ("Test 43: checking a regular listener still works after the failures..\n");
	listener = nopoll_listener_new (ctx, "0.0.0.0", regtest_port (1251));
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to create a proper listener at 0.0.0.0:%s..\n", regtest_port (1251));
		return nopoll_false;
	} /* end if */

	if (nopoll_conn_role (listener) != NOPOLL_ROLE_MAIN_LISTENER) {
		printf ("ERROR: expected the listener to report NOPOLL_ROLE_MAIN_LISTENER role..\n");
		return nopoll_false;
	} /* end if */

	nopoll_conn_close (listener);

	/* now check the remote host recorded for a connection accepted
	 * over IPv6: reading it into a sockaddr_in truncated the
	 * address, so every IPv6 connection was reporting 0.0.0.0 */
	printf ("Test 43: checking remote host reported for an IPv6 accepted connection..\n");
	listener = nopoll_listener_new6 (ctx, "::1", regtest_port (1253));
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to create a proper IPv6 listener at ::1:%s..\n", regtest_port (1253));
		return nopoll_false;
	} /* end if */

	conn = nopoll_conn_new6 (ctx, "::1", regtest_port (1253), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected to connect to the IPv6 listener at ::1:%s..\n", regtest_port (1253));
		return nopoll_false;
	} /* end if */

	accepted = nopoll_conn_accept (ctx, listener);
	if (accepted == NULL) {
		printf ("ERROR: expected to accept the incoming IPv6 connection..\n");
		return nopoll_false;
	} /* end if */

	if (! nopoll_cmp (nopoll_conn_host (accepted), "::1")) {
		printf ("ERROR: expected to find remote host '::1' for the accepted IPv6 connection but found '%s'..\n",
			nopoll_conn_host (accepted) ? nopoll_conn_host (accepted) : "<null>");
		return nopoll_false;
	} /* end if */

	printf ("Test 43: IPv6 accepted connection reports host=%s port=%s\n",
		nopoll_conn_host (accepted), nopoll_conn_port (accepted));

	nopoll_conn_close (accepted);
	nopoll_conn_close (conn);
	nopoll_conn_close (listener);

	/* release context */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/**
 * @internal Checks the reference lifecycle of a noPollConnOpts object
 * flagged for reuse and shared by every connection accepted by a
 * listener. Nothing else in the suite exercises
 * nopoll_conn_opts_set_reuse (), which is how the leak below survived.
 */
nopoll_bool test_44 (void) {

	noPollCtx      * ctx;
	noPollConnOpts * opts;
	noPollConn     * listener;
	noPollConn     * conn;
	noPollConn     * accepted;
	int              iterator;

	printf ("Test 44: checking noPollConnOpts reference lifecycle with reuse enabled..\n");

	ctx  = create_ctx ();
	opts = nopoll_conn_opts_new ();
	if (opts == NULL) {
		printf ("ERROR: expected to create a connection options object..\n");
		return nopoll_false;
	} /* end if */

	/* the documented way of reusing one options object across the
	 * connections accepted by a listener: with reuse enabled the
	 * object belongs to the caller */
	nopoll_conn_opts_set_reuse (opts, nopoll_true);
	nopoll_conn_opts_set_cookie (opts, "session=test-44");

	listener = nopoll_listener_new_opts (ctx, opts, "0.0.0.0", regtest_port (1254));
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to create a listener at 0.0.0.0:%s..\n", regtest_port (1254));
		return nopoll_false;
	} /* end if */

	/* accept a few connections: each one acquires a reference over
	 * the options object and must release it when done */
	iterator = 0;
	while (iterator < 3) {
		conn = nopoll_conn_new (ctx, "127.0.0.1", regtest_port (1254), NULL, NULL, NULL, NULL);
		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: expected to connect to the listener (iteration %d)..\n", iterator);
			return nopoll_false;
		} /* end if */

		accepted = nopoll_conn_accept (ctx, listener);
		if (accepted == NULL) {
			printf ("ERROR: expected to accept the incoming connection (iteration %d)..\n", iterator);
			return nopoll_false;
		} /* end if */

		nopoll_conn_close (accepted);
		nopoll_conn_close (conn);

		iterator++;
	} /* end while */

	/* the object must be back to the single reference held by the
	 * caller: every accepted connection acquired one and had to
	 * release it. A bigger value means those references were
	 * leaked and the object will never be released */
	if (opts->refs != 1) {
		printf ("ERROR: expected 1 reference over the options object after accepting %d connections, but found %d..\n",
			iterator, opts->refs);
		return nopoll_false;
	} /* end if */

	nopoll_conn_close (listener);

	/* reuse was enabled, so releasing it is up to us */
	nopoll_conn_opts_free (opts);

	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/* internal API used by the test below to compute the Sec-WebSocket-Accept
   value the same way the library does */
char * nopoll_conn_produce_accept_key (noPollCtx * ctx, const char * websocket_key);

/**
 * @internal Common function that creates a raw TCP listener (we need
 * to reply a handcrafted handshake, which the regression listener
 * cannot do), lets a noPoll client connect to it and replies a 101
 * with either the right or a wrong Sec-WebSocket-Accept value.
 *
 * It checks the client accepts the connection only when the value
 * replied is the one derived from the Sec-WebSocket-Key it sent
 * (RFC 6455, section 4.1).
 */
nopoll_bool test_41_common_accept_key (const char * label, nopoll_bool send_valid_accept)
{
	noPollCtx          * ctx;
	noPollConn         * conn;
	NOPOLL_SOCKET        listener_sock;
	NOPOLL_SOCKET        session;
	struct sockaddr_in   addr;
	char                 buffer[4096];
	char                 key[128];
	char               * accept_key = NULL;
	char               * reply;
	char               * key_start;
	int                  bytes;
	int                  iterator;
	int                  tries;
	int                  reuse  = 1;
	nopoll_bool          result = nopoll_false;

	printf ("Test %s: creating raw listener at 127.0.0.1:%s..\n", label, regtest_port (1250));

	/* create the raw listener */
	listener_sock = socket (AF_INET, SOCK_STREAM, 0);
	if (listener_sock == NOPOLL_INVALID_SOCKET) {
		printf ("ERROR: unable to create raw listener socket..\n");
		return nopoll_false;
	} /* end if */

	setsockopt (listener_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof (reuse));

	memset (&addr, 0, sizeof (addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
	addr.sin_port        = htons ((unsigned short) regtest_port_int (1250));

	if (bind (listener_sock, (struct sockaddr *) &addr, sizeof (addr)) != 0 || listen (listener_sock, 1) != 0) {
		printf ("ERROR: unable to bind/listen at 127.0.0.1:%s, errno=%d..\n", regtest_port (1250), errno);
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */

	/* init context and connect to our raw listener */
	ctx  = create_ctx ();
	conn = nopoll_conn_new (ctx, "127.0.0.1", regtest_port (1250), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */

	/* accept the connection and read the client handshake */
	session = accept (listener_sock, NULL, NULL);
	if (session == NOPOLL_INVALID_SOCKET) {
		printf ("ERROR: unable to accept the incoming connection, errno=%d..\n", errno);
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */

	bytes = recv (session, buffer, sizeof (buffer) - 1, 0);
	if (bytes <= 0) {
		printf ("ERROR: expected to receive the client handshake but found %d bytes..\n", bytes);
		nopoll_close_socket (session);
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */
	buffer[bytes] = 0;

	/* find the Sec-WebSocket-Key sent by the client */
	key_start = strstr (buffer, "Sec-WebSocket-Key: ");
	if (key_start == NULL) {
		printf ("ERROR: unable to find Sec-WebSocket-Key inside the client handshake..\n");
		nopoll_close_socket (session);
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */

	key_start += 19; /* strlen ("Sec-WebSocket-Key: ") */
	iterator   = 0;
	while (iterator < ((int) sizeof (key) - 1) && key_start[iterator] && key_start[iterator] != '\r' && key_start[iterator] != '\n') {
		key[iterator] = key_start[iterator];
		iterator++;
	} /* end while */
	key[iterator] = 0;

	printf ("Test %s: received Sec-WebSocket-Key: %s\n", label, key);

	/* build the reply */
	if (send_valid_accept) {
		accept_key = nopoll_conn_produce_accept_key (ctx, key);
	} else {
		/* a syntactically valid but wrong accept value */
		accept_key = nopoll_strdup ("AAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	} /* end if */

	if (accept_key == NULL) {
		printf ("ERROR: unable to produce the Sec-WebSocket-Accept value for the reply..\n");
		nopoll_close_socket (session);
		nopoll_close_socket (listener_sock);
		return nopoll_false;
	} /* end if */

	printf ("Test %s: replying Sec-WebSocket-Accept: %s (valid: %d)\n", label, accept_key, send_valid_accept);

	reply = nopoll_strdup_printf ("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",
				      accept_key);
	send (session, reply, strlen (reply), 0);

	/* let the client process the reply */
	tries = 50;
	while (tries > 0) {
		if (nopoll_conn_is_ready (conn))
			break;
		if (! nopoll_conn_is_ok (conn))
			break;
		nopoll_sleep (100000);
		tries--;
	} /* end while */

	if (send_valid_accept) {
		/* the handshake must be completed */
		result = nopoll_conn_is_ok (conn) && nopoll_conn_is_ready (conn);
		if (! result)
			printf ("ERROR: expected the connection to be accepted after replying the right Sec-WebSocket-Accept..\n");
	} else {
		/* the connection must have been closed by the client */
		result = ! nopoll_conn_is_ok (conn);
		if (! result)
			printf ("ERROR: expected the connection to be closed after replying a wrong Sec-WebSocket-Accept, but it is still working..\n");
	} /* end if */

	/* release everything */
	nopoll_free (reply);
	nopoll_free (accept_key);
	nopoll_conn_close (conn);
	nopoll_ctx_unref (ctx);
	nopoll_close_socket (session);
	nopoll_close_socket (listener_sock);

	return result;
}

nopoll_bool test_41 (void) {
	/* a listener replying the right accept value must be accepted */
	if (! test_41_common_accept_key ("41", nopoll_true))
		return nopoll_false;

	/* a listener replying a wrong accept value must be rejected */
	return test_41_common_accept_key ("41", nopoll_false);
}

/**
 * @internal Handler used by test_45 that closes the very connection it
 * is being notified about, which is the supported pattern that
 * triggered a use after free.
 */
void test_45_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr user_data)
{
	int * closed = (int *) user_data;

	/* close the connection that received the message */
	nopoll_conn_close (conn);

	(*closed)++;

	return;
}

/**
 * @internal Handler used by test_45 to configure accepted connections
 * the same way the regression listener does.
 *
 * Accepted connections are created in blocking mode: the built-in loop
 * assumes they are not, so without this the loop blocks inside recv ()
 * waiting for a websocket header that is not there yet.
 */
nopoll_bool test_45_on_open (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	if (! nopoll_conn_set_sock_block (nopoll_conn_socket (conn), nopoll_false)) {
		printf ("ERROR: failed to configure non-blocking state to the accepted connection..\n");
		return nopoll_false;
	} /* end if */

	return nopoll_true;
}

/**
 * @internal Checks that closing a connection from inside a
 * notification handler does not release the connection while the
 * library is still using it.
 *
 * nopoll_ctx_foreach_conn () acquires a reference over every
 * connection before notifying it, and nopoll_conn_close () used the
 * reference counting to find out whether the caller had a reference of
 * its own to release. As a result, closing an accepted connection from
 * the on-message handler dropped the reference held by the iterator,
 * destroying the connection before the iterator dropped it: the
 * regression listener died with SIGSEGV in the middle of the test run.
 *
 * NOTE: run this test under ASan or Valgrind to get a diagnostic. On a
 * plain build the failure shows up as a crash (or, worse, as nothing
 * at all).
 */
nopoll_bool test_45 (void) {
	noPollCtx  * ctx;
	noPollConn * listener;
	noPollConn * conn;
	int          closed = 0;
	int          tries;
	int          conns;

	printf ("Test 45: checking connection close from inside a notification handler..\n");

	ctx = create_ctx ();
	if (ctx == NULL) {
		printf ("ERROR: expected to create a context..\n");
		return nopoll_false;
	} /* end if */

	/* create the listener that will receive the message */
	listener = nopoll_listener_new (ctx, "0.0.0.0", regtest_port (1255));
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to create a listener at 0.0.0.0:%s..\n", regtest_port (1255));
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* configure accepted connections to be non-blocking */
	nopoll_ctx_set_on_open (ctx, test_45_on_open, NULL);

	/* close every connection that receives a message */
	nopoll_ctx_set_on_msg (ctx, test_45_on_message, &closed);

	/* connect to it */
	conn = nopoll_conn_new (ctx, "127.0.0.1", regtest_port (1255), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: expected to connect to the listener at 127.0.0.1:%s..\n", regtest_port (1255));
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* let the loop accept the connection and complete the
	 * handshake: both connections live in the same context */
	tries = 100; /* 100 x 100ms = 10 seconds */
	while (tries > 0 && ! nopoll_conn_is_ready (conn)) {
		nopoll_loop_wait (ctx, 100000);
		tries--;
	} /* end while */

	if (! nopoll_conn_is_ready (conn)) {
		printf ("ERROR: timeout reached while waiting for the connection to be ready..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* at this point the context holds the listener, the client
	 * connection and the connection accepted by the listener */
	conns = nopoll_ctx_conns (ctx);
	if (conns != 3) {
		printf ("ERROR: expected 3 connections registered before the close, but found %d..\n", conns);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* send the message that makes the listener side close the
	 * connection from inside the handler */
	if (nopoll_conn_send_text (conn, "close me", 8) != 8) {
		printf ("ERROR: expected to send the content that triggers the close..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* run the loop so the message is notified: this is where the
	 * connection used to be released too early */
	tries = 50; /* 50 x 100ms = 5 seconds */
	while (tries > 0 && closed == 0) {
		nopoll_loop_wait (ctx, 100000);
		tries--;
	} /* end while */

	if (closed != 1) {
		printf ("ERROR: expected the handler to be called once, but it was called %d time(s)..\n", closed);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* the accepted connection must be gone: the client connection
	 * may be gone too (it is unregistered as soon as the loop
	 * processes the close frame it received), so only the drop is
	 * checked here */
	conns = nopoll_ctx_conns (ctx);
	if (conns >= 3) {
		printf ("ERROR: expected the accepted connection to be unregistered after the close, but %d connections are still registered..\n", conns);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* release everything
	 *
	 * NOTE: an extra reference is acquired here to check that the
	 * close releases the reference owned by this function. At this
	 * point the loop already unregistered the client connection
	 * (it is not working anymore after the close received), so the
	 * context registry does not hold a reference of its own:
	 * nopoll_conn_close () used to skip the release in that case,
	 * leaking the connection */
	nopoll_conn_ref (conn);
	nopoll_conn_close (conn);

	if (nopoll_conn_ref_count (conn) != 1) {
		printf ("ERROR: expected 1 reference after closing the connection, but found %d (the reference owned by the caller wasn't released)..\n",
			nopoll_conn_ref_count (conn));
		nopoll_conn_unref (conn);
		nopoll_conn_close (listener);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* release the extra reference acquired above */
	nopoll_conn_unref (conn);

	nopoll_conn_close (listener);
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

/**
 * @internal Checks the ownership contract of noPollConnOpts on the
 * listener creation path when it fails.
 *
 * With the reuse flag disabled the API takes over the options object,
 * so a failed listener creation must release it: it used to return
 * NULL leaving the object (and the strings it holds) allocated. With
 * the reuse flag enabled the caller keeps the ownership and the
 * reference count must be left untouched.
 *
 * NOTE: the missing release is a memory leak, so run this test under
 * ASan or Valgrind to catch that half of it. The reference count check
 * below fails on any build.
 */
nopoll_bool test_46 (void) {
	noPollCtx      * ctx;
	noPollConn     * listener;
	noPollConn     * listener2;
	noPollConnOpts * opts;
	noPollConnOpts * opts_reuse;

	printf ("Test 46: checking connection options release on listener creation failure..\n");

	ctx = create_ctx ();
	if (ctx == NULL) {
		printf ("ERROR: expected to create a context..\n");
		return nopoll_false;
	} /* end if */

	/* take the port so the creations below fail */
	listener = nopoll_listener_new (ctx, "0.0.0.0", regtest_port (1256));
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: expected to create a listener at 0.0.0.0:%s..\n", regtest_port (1256));
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* a failed creation must consume the options object when it is
	 * not flagged for reuse */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_cookie (opts, "session=test-46");

	listener2 = nopoll_listener_new_opts (ctx, opts, "0.0.0.0", regtest_port (1256));
	if (listener2 != NULL) {
		printf ("ERROR: expected to fail creating a second listener over a port already in use..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* same failure but keeping the ownership: the object must
	 * survive with its reference count untouched */
	opts_reuse = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_reuse (opts_reuse, nopoll_true);
	nopoll_conn_opts_set_cookie (opts_reuse, "session=test-46");

	listener2 = nopoll_listener_new_opts (ctx, opts_reuse, "0.0.0.0", regtest_port (1256));
	if (listener2 != NULL) {
		printf ("ERROR: expected to fail creating a second listener over a port already in use (reuse)..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	if (opts_reuse->refs != 1) {
		printf ("ERROR: expected 1 reference over the reused options object after a failed listener creation, but found %d..\n",
			opts_reuse->refs);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* a listener creation rejected before touching the network
	 * must release the object too */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_set_cookie (opts, "session=test-46");

	listener2 = nopoll_listener_new_opts (ctx, opts, NULL, regtest_port (1256));
	if (listener2 != NULL) {
		printf ("ERROR: expected to fail creating a listener without host..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* release everything: only the object flagged for reuse is
	 * owned by this function, the others were consumed by the
	 * failed creations */
	nopoll_conn_opts_free (opts_reuse);
	nopoll_conn_close (listener);
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_47 (void) {
	noPollMsg * msg;
	noPollMsg * msg2;
	noPollMsg * joined;
	noPollMsg * aux;
	long int    huge_size;

	printf ("Test 47: checking nopoll_msg_join () joining and allocation failure handling..\n");

	msg  = nopoll_msg_new ();
	msg2 = nopoll_msg_new ();
	if (msg == NULL || msg2 == NULL) {
		printf ("ERROR: expected to create both message holders..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	/* first message: fragment with FIN = 0 */
	msg->op_code      = NOPOLL_TEXT_FRAME;
	msg->payload_size = 5;
	msg->payload      = nopoll_new (char, 6);
	memcpy (msg->payload, "hello", 5);

	/* second message: final continuation fragment */
	msg2->has_fin      = nopoll_true;
	msg2->op_code      = NOPOLL_CONTINUATION_FRAME;
	msg2->payload_size = 6;
	msg2->payload      = nopoll_new (char, 7);
	memcpy (msg2->payload, "-world", 6);

	if (msg->payload == NULL || msg2->payload == NULL) {
		printf ("ERROR: expected to allocate payload for both messages..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	/* check the join produces the complete content */
	joined = nopoll_msg_join (msg, msg2);
	if (joined == NULL) {
		printf ("ERROR: expected to join both messages..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	if (nopoll_msg_get_payload_size (joined) != 11 ||
	    memcmp (nopoll_msg_get_payload (joined), "hello-world", 12) != 0) {
		printf ("ERROR: expected to find 'hello-world' (11 bytes) after joining but found '%s' (%ld bytes)..\n",
			(const char *) nopoll_msg_get_payload (joined), nopoll_msg_get_payload_size (joined));
		nopoll_msg_unref (joined);
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	/* the function must not consume the references received */
	if (nopoll_msg_ref_count (msg) != 1 || nopoll_msg_ref_count (msg2) != 1 || nopoll_msg_ref_count (joined) != 1) {
		printf ("ERROR: expected 1 reference on every message after joining but found %d, %d and %d..\n",
			nopoll_msg_ref_count (msg), nopoll_msg_ref_count (msg2), nopoll_msg_ref_count (joined));
		nopoll_msg_unref (joined);
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	nopoll_msg_unref (joined);

	/* check reference handling for the single argument cases */
	if (nopoll_msg_join (NULL, NULL) != NULL) {
		printf ("ERROR: expected NULL when joining two NULL references..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	if (nopoll_msg_join (msg, NULL) != msg || nopoll_msg_ref_count (msg) != 2) {
		printf ("ERROR: expected same reference with 2 references when joining with NULL..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */
	nopoll_msg_unref (msg);

	if (nopoll_msg_join (NULL, msg2) != msg2 || nopoll_msg_ref_count (msg2) != 2) {
		printf ("ERROR: expected same reference with 2 references when joining from NULL..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */
	nopoll_msg_unref (msg2);

	/* now force the payload allocation done by nopoll_msg_join to
	 * fail by reporting a payload size that cannot be allocated:
	 * the function must report failure without dereferencing the
	 * failed allocation and without leaking the internal holder
	 * (run this test under valgrind to check the leak) */
	if (sizeof (long int) >= 8)
		huge_size = ((long int) 1) << 60;
	else
		huge_size = LONG_MAX - 16;
	msg->payload_size = huge_size;

	joined = nopoll_msg_join (msg, msg2);
	if (joined != NULL) {
		printf ("ERROR: expected to fail joining a message reporting a payload size of %ld bytes..\n", huge_size);
		nopoll_msg_unref (joined);
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	/* restore the real payload size so the message is released fine */
	msg->payload_size = 5;

	/* check the function rejects a payload size that cannot be added
	 * without overflowing (it used to be computed with a signed sum,
	 * which is undefined behaviour: run this test under
	 * -fsanitize=undefined to check it) */
	msg->payload_size = LONG_MAX - 4;
	if (nopoll_msg_join (msg, msg2) != NULL) {
		printf ("ERROR: expected to fail joining messages whose sizes cannot be added..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	/* check the function rejects a message reporting a negative
	 * payload size (it would be converted into a huge size_t) */
	msg->payload_size = -8;
	if (nopoll_msg_join (msg, msg2) != NULL) {
		printf ("ERROR: expected to fail joining a message reporting a negative payload size..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	msg->payload_size = 5;

	/* check joining a message holder with no payload at all: this is
	 * what nopoll_msg_new () reports and it must not make the
	 * function copy from a NULL reference */
	joined = nopoll_msg_new ();
	if (joined == NULL) {
		printf ("ERROR: expected to create an empty message holder..\n");
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	aux = nopoll_msg_join (joined, msg);
	if (aux == NULL || nopoll_msg_get_payload_size (aux) != 5 ||
	    memcmp (nopoll_msg_get_payload (aux), "hello", 6) != 0) {
		printf ("ERROR: expected to join an empty message holder with 'hello' (5 bytes)..\n");
		nopoll_msg_unref (aux);
		nopoll_msg_unref (joined);
		nopoll_msg_unref (msg);
		nopoll_msg_unref (msg2);
		return nopoll_false;
	} /* end if */

	nopoll_msg_unref (aux);
	nopoll_msg_unref (joined);

	nopoll_msg_unref (msg);
	nopoll_msg_unref (msg2);

	return nopoll_true;
}

/**
 * @internal Builds a complete masked websocket text frame (FIN = 1)
 * holding the content provided, writing it into the buffer received.
 *
 * @return The amount of bytes written into the buffer.
 */
int test_48_build_frame (char * buffer, const char * content)
{
	int          iterator;
	int          length = (int) strlen (content);
	unsigned char mask[4] = {0x11, 0x22, 0x33, 0x44};

	/* FIN = 1, opcode = text frame */
	buffer[0] = (char) (0x80 | NOPOLL_TEXT_FRAME);
	/* mask = 1, payload size (always < 126 for this test) */
	buffer[1] = (char) (0x80 | length);

	memcpy (buffer + 2, mask, 4);

	iterator = 0;
	while (iterator < length) {
		buffer[6 + iterator] = (char) (content[iterator] ^ mask[iterator % 4]);
		iterator++;
	} /* end while */

	return 6 + length;
}

nopoll_bool test_48 (void) {

	noPollCtx      * ctx;
	noPollConn     * conn;
	noPollConnOpts * opts;
	noPollMsg      * msg;
	char             buffer[128];
	char             content[128];
	int              size;
	int              received = 0;
	int              iterator;

	printf ("Test 48: checking several frames received inside the same TLS record..\n");

	/* reinit again */
	ctx = create_ctx ();

	/* disable verification */
	opts = nopoll_conn_opts_new ();
	nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, opts, "localhost", regtest_port (1235), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_wait_until_connection_ready (conn, 10)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		nopoll_conn_close (conn);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* build two complete frames and push them with a single
	 * SSL_write () call so both travel inside the same TLS record:
	 * the listener is driven by nopoll_loop_wait () and, after
	 * reading the first frame, the second one is already decrypted
	 * and held by OpenSSL, so no socket event can report it */
	size  = test_48_build_frame (buffer, "packed-frame-1");
	size += test_48_build_frame (buffer + size, "packed-frame-2");

	if (SSL_write (conn->ssl, buffer, size) != size) {
		printf ("ERROR: expected to write %d bytes with a single TLS record..\n", size);
		nopoll_conn_close (conn);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* now wait for both echo replies: the listener replies with the
	 * same content received */
	iterator = 0;
	while (iterator < 100 && received < 2) {
		msg = nopoll_conn_get_msg (conn);
		if (msg == NULL) {
			if (! nopoll_conn_is_ok (conn)) {
				printf ("ERROR: connection was closed while waiting for the echo replies..\n");
				nopoll_conn_close (conn);
				nopoll_ctx_unref (ctx);
				return nopoll_false;
			} /* end if */

			nopoll_sleep (50000);
			iterator++;
			continue;
		} /* end if */

		memset (content, 0, 128);
		memcpy (content, (const char *) nopoll_msg_get_payload (msg),
			nopoll_msg_get_payload_size (msg) > 127 ? 127 : (int) nopoll_msg_get_payload_size (msg));
		nopoll_msg_unref (msg);

		received++;
		printf ("Test 48: received echo reply %d: %s\n", received, content);

		if (! nopoll_cmp (content, received == 1 ? "packed-frame-1" : "packed-frame-2")) {
			printf ("ERROR: expected to receive 'packed-frame-%d' but found '%s'..\n", received, content);
			nopoll_conn_close (conn);
			nopoll_ctx_unref (ctx);
			return nopoll_false;
		} /* end if */
	} /* end while */

	if (received != 2) {
		printf ("ERROR: expected to receive 2 echo replies but received %d: the listener did not process the frame left inside the TLS buffer..\n",
			received);
		nopoll_conn_close (conn);
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);

	/* finish */
	nopoll_ctx_unref (ctx);

	/* report finish */
	return nopoll_true;
}

/* record of the logs received by __test_49_log_handler */
int          test_49_criticals = 0;
const char * test_49_last_msg  = NULL;

void __test_49_log_handler (noPollCtx * ctx, noPollDebugLevel level, const char * log_msg, noPollPtr user_data)
{
	int * counter = (int *) user_data;

	if (level != NOPOLL_LEVEL_CRITICAL)
		return;

	/* record the critical received */
	test_49_criticals++;
	test_49_last_msg = log_msg;

	if (counter)
		(*counter)++;

	return;
}

nopoll_bool test_49 (void)
{
	noPollCtx * ctx;
	int         user_data_counter = 0;

	ctx = nopoll_ctx_new ();
	if (ctx == NULL) {
		printf ("ERROR (1): expected to find proper context creation..\n");
		return nopoll_false;
	} /* end if */

	/* disable console log on purpose: the handler configured must
	 * receive the log no matter this setting */
	nopoll_log_enable (ctx, nopoll_false);
	nopoll_log_color_enable (ctx, nopoll_false);

	test_49_criticals = 0;
	test_49_last_msg  = NULL;
	nopoll_log_set_handler (ctx, __test_49_log_handler, &user_data_counter);

	/* trigger a precondition failure with a valid context: it is
	 * reported through nopoll_return_val_if_fail (), which calls
	 * __nopoll_log () directly, without going through the
	 * nopoll_log () macro */
	if (nopoll_ctx_foreach_conn (ctx, NULL, NULL) != NULL) {
		printf ("ERROR (2): expected NULL return from nopoll_ctx_foreach_conn () with a NULL handler..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	if (test_49_criticals == 0) {
		printf ("ERROR (3): expected to receive the critical reported by the precondition check at the configured log handler..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	if (user_data_counter != test_49_criticals) {
		printf ("ERROR (4): expected to receive the user data pointer configured at nopoll_log_set_handler ()..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	printf ("Test 49: received %d critical (s) at the configured log handler..\n", test_49_criticals);

	/* now remove the handler and check no additional critical is
	 * notified */
	nopoll_log_set_handler (ctx, NULL, NULL);
	test_49_criticals = 0;

	if (nopoll_ctx_foreach_conn (ctx, NULL, NULL) != NULL) {
		printf ("ERROR (5): expected NULL return from nopoll_ctx_foreach_conn () with a NULL handler..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	if (test_49_criticals != 0) {
		printf ("ERROR (6): expected to not receive any critical after removing the log handler..\n");
		nopoll_ctx_unref (ctx);
		return nopoll_false;
	} /* end if */

	/* finish */
	nopoll_ctx_unref (ctx);

	/* report finish */
	return nopoll_true;
}

/* number of simultaneous connections opened by test_50: the context
 * connection list grows 10 by 10, so this value forces the
 * reallocation path inside nopoll_ctx_register_conn () to run several
 * times */
#define TEST_50_CONNECTIONS (25)

nopoll_bool test_50 (void)
{
	noPollCtx  * ctx;
	noPollConn * conns[TEST_50_CONNECTIONS];
	noPollMsg  * msg;
	int          iterator;
	int          iter;
	nopoll_bool  result = nopoll_false;

	printf ("Test 50: checking context connection list growth with %d simultaneous connections..\n", TEST_50_CONNECTIONS);

	/* create context */
	ctx = create_ctx ();
	if (ctx == NULL) {
		printf ("ERROR (1): expected to find proper context creation..\n");
		return nopoll_false;
	} /* end if */

	/* clear the array so the cleanup code below is always safe */
	for (iterator = 0; iterator < TEST_50_CONNECTIONS; iterator++)
		conns[iterator] = NULL;

	/* open all the connections before closing any of them, so every
	 * one of them takes a different position of the list */
	for (iterator = 0; iterator < TEST_50_CONNECTIONS; iterator++) {
		conns[iterator] = nopoll_conn_new (ctx, "localhost", regtest_port (1234), NULL, NULL, NULL, NULL);
		if (! nopoll_conn_is_ok (conns[iterator])) {
			printf ("ERROR (2): expected to find proper connection status at connection %d..\n", iterator);
			goto finish;
		} /* end if */

		if (! nopoll_conn_wait_until_connection_ready (conns[iterator], 5)) {
			printf ("ERROR (3): expected to find proper connection ready at connection %d..\n", iterator);
			goto finish;
		} /* end if */
	} /* end for */

	/* check all of them are registered */
	if (nopoll_ctx_conns (ctx) != TEST_50_CONNECTIONS) {
		printf ("ERROR (4): expected to find %d registered connections but found: %d\n",
			TEST_50_CONNECTIONS, nopoll_ctx_conns (ctx));
		goto finish;
	} /* end if */

	/* check every connection still works: a wrong handling of the
	 * list reallocation would have lost or duplicated references */
	for (iterator = 0; iterator < TEST_50_CONNECTIONS; iterator++) {
		if (nopoll_conn_send_text (conns[iterator], "This is a test", 14) != 14) {
			printf ("ERROR (5): expected to find proper send operation at connection %d..\n", iterator);
			goto finish;
		} /* end if */

		/* wait for the reply */
		iter = 0;
		while ((msg = nopoll_conn_get_msg (conns[iterator])) == NULL) {
			if (! nopoll_conn_is_ok (conns[iterator])) {
				printf ("ERROR (6): received websocket connection close during wait reply at connection %d..\n", iterator);
				goto finish;
			} /* end if */

			nopoll_sleep (10000);

			if (iter > 10)
				break;
			iter++;
		} /* end while */

		if (msg == NULL || ! nopoll_cmp ((char *) nopoll_msg_get_payload (msg), "This is a test")) {
			printf ("ERROR (7): expected to find echo reply at connection %d but found: '%s'..\n",
				iterator, msg ? (const char *) nopoll_msg_get_payload (msg) : "<null>");
			nopoll_msg_unref (msg);
			goto finish;
		} /* end if */

		nopoll_msg_unref (msg);
	} /* end for */

	/* check the accepted protocol is replaced (and not leaked) when
	 * it is configured twice on the same connection */
	nopoll_conn_set_accepted_protocol (conns[0], "protocol-1");
	nopoll_conn_set_accepted_protocol (conns[0], "protocol-2");

	result = nopoll_true;

finish:
	/* close all the connections opened */
	for (iterator = 0; iterator < TEST_50_CONNECTIONS; iterator++)
		nopoll_conn_close (conns[iterator]);

	/* check the context is left with no connection registered */
	if (result && nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR (8): expected to find 0 registered connections after closing them but found: %d\n",
			nopoll_ctx_conns (ctx));
		result = nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	return result;
}

/* content sent by test_51: it is short on purpose, but the frame
 * announces its length using the 64 bit extended representation
 * (payload len = 127), which is what forces the 10 byte header path */
#define TEST_51_CONTENT "split-extended-header"

nopoll_bool test_51 (void)
{
	noPollCtx     * ctx;
	noPollConn    * master   = NULL;
	noPollConn    * conn     = NULL;
	noPollConn    * listener = NULL;
	noPollMsg     * msg;
	NOPOLL_SOCKET   _socket;
	char            header[10];
	char            mask[4];
	char            payload[64];
	char            buffer[64];
	int             length;
	int             iterator;
	nopoll_bool     result   = nopoll_false;

	printf ("Test 51: checking a frame whose 10 byte header arrives split from its mask..\n");

	ctx = create_ctx ();
	if (ctx == NULL) {
		printf ("ERROR (1): expected to find proper context creation..\n");
		return nopoll_false;
	} /* end if */

	/* create a listener and a connection to it, so both peers are
	 * driven from this same process */
	master = nopoll_listener_new (ctx, "0.0.0.0", regtest_port (1255));
	if (! nopoll_conn_is_ok (master)) {
		printf ("ERROR (2): expected to create a listener at 0.0.0.0:%s..\n", regtest_port (1255));
		goto finish;
	} /* end if */

	conn = nopoll_conn_new (ctx, "localhost", regtest_port (1255), NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR (3): expected to find proper client connection status..\n");
		goto finish;
	} /* end if */

	listener = nopoll_conn_accept (ctx, master);
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR (4): expected to accept the incoming connection..\n");
		goto finish;
	} /* end if */

	/* make the accepted connection non blocking: the test sends an
	 * incomplete frame on purpose, so the listener must report "no
	 * content available" instead of blocking waiting for the mask */
	if (! nopoll_conn_set_sock_block (nopoll_conn_socket (listener), nopoll_false)) {
		printf ("ERROR (4.1): expected to configure the accepted connection as non blocking..\n");
		goto finish;
	} /* end if */

	/* exchange a regular message first, so the handshake is
	 * completed on both sides before sending content by hand */
	if (nopoll_conn_send_text (conn, "sync", 4) != 4) {
		printf ("ERROR (5): expected to send the initial sync message..\n");
		goto finish;
	} /* end if */

	memset (buffer, 0, 64);
	if (nopoll_conn_read (listener, buffer, 4, nopoll_true, 0) != 4) {
		printf ("ERROR (6): expected to read the initial sync message at the listener..\n");
		goto finish;
	} /* end if */

	/* build the 10 byte header: FIN + text frame, masked, with the
	 * length announced through the 64 bit extended field */
	length = (int) strlen (TEST_51_CONTENT);
	memset (header, 0, 10);
	header[0] = (char) 0x81;  /* FIN = 1, op_code = text */
	header[1] = (char) 0xFF;  /* MASK = 1, payload len = 127 */
	header[9] = (char) length;

	_socket = nopoll_conn_socket (conn);

	printf ("Test 51: sending the 10 byte header alone..\n");
	if (send (_socket, header, 10, 0) != 10) {
		printf ("ERROR (7): expected to send the frame header..\n");
		goto finish;
	} /* end if */

	/* let the listener consume the header and find no mask behind
	 * it: at this point it must save what was read and report no
	 * message, keeping the connection open */
	nopoll_sleep (200000);

	iterator = 0;
	while (iterator < 5) {
		msg = nopoll_conn_get_msg (listener);
		if (msg != NULL) {
			printf ("ERROR (8): expected no message while only the header was sent..\n");
			nopoll_msg_unref (msg);
			goto finish;
		} /* end if */

		if (! nopoll_conn_is_ok (listener)) {
			printf ("ERROR (9): the listener closed the connection after receiving the header alone: the header saved to be resumed was wrong..\n");
			goto finish;
		} /* end if */

		nopoll_sleep (10000);
		iterator++;
	} /* end while */

	/* now send the mask and the masked payload */
	printf ("Test 51: sending the mask and the payload..\n");
	mask[0] = 11;
	mask[1] = 12;
	mask[2] = 13;
	mask[3] = 14;

	memset (payload, 0, 64);
	memcpy (payload, TEST_51_CONTENT, length);
	nopoll_conn_mask_content (ctx, payload, length, mask, 0);

	if (send (_socket, mask, 4, 0) != 4 || send (_socket, payload, length, 0) != length) {
		printf ("ERROR (10): expected to send the mask and the payload..\n");
		goto finish;
	} /* end if */

	/* the listener must now complete the frame it had partially
	 * read and report the original content */
	iterator = 0;
	msg      = NULL;
	while (iterator < 100 && msg == NULL) {
		msg = nopoll_conn_get_msg (listener);
		if (msg == NULL) {
			if (! nopoll_conn_is_ok (listener)) {
				printf ("ERROR (11): the listener closed the connection while completing the frame..\n");
				goto finish;
			} /* end if */

			nopoll_sleep (10000);
			iterator++;
		} /* end if */
	} /* end while */

	if (msg == NULL) {
		printf ("ERROR (12): expected to receive the frame sent with its header split from the mask..\n");
		goto finish;
	} /* end if */

	if (nopoll_msg_get_payload_size (msg) != length ||
	    ! nopoll_cmp ((const char *) nopoll_msg_get_payload (msg), TEST_51_CONTENT)) {
		printf ("ERROR (13): expected to receive '%s' (%d bytes) but found '%s' (%ld bytes)..\n",
			TEST_51_CONTENT, length, (const char *) nopoll_msg_get_payload (msg),
			nopoll_msg_get_payload_size (msg));
		nopoll_msg_unref (msg);
		goto finish;
	} /* end if */

	printf ("Test 51: received '%s' after resuming the split header..\n", (const char *) nopoll_msg_get_payload (msg));
	nopoll_msg_unref (msg);

	result = nopoll_true;

finish:
	nopoll_conn_close (conn);
	nopoll_conn_close (listener);
	nopoll_conn_close (master);
	nopoll_ctx_unref (ctx);

	return result;
}

int main (int argc, char ** argv)
{
	int iterator;

	printf ("** NoPoll: Websocket toolkit (regression test).\n");
	printf ("** Copyright (C) 2025 Advanced Software Production Line, S.L.\n**\n");
	printf ("** NoPoll regression tests: version=%s\n**\n",
		VERSION);
	printf ("** To gather information about time performance you can use:\n**\n");
	printf ("**     >> time ./nopoll-regression-client [--debug,--show-critical-only,--offset-port <value>]\n**\n");
	printf ("** Use --offset-port to add the value provided to every port used by the test,\n");
	printf ("** so a complete run (listener and client) can be started without colliding with\n");
	printf ("** another run already in progress. The same value must be provided to\n");
	printf ("** ./nopoll-regression-listener\n**\n");
	printf ("** To gather information about memory consumed (and leaks) use:\n**\n");
	printf ("**     >> libtool --mode=execute valgrind --leak-check=yes --error-limit=no ./nopoll-regression-client\n**\n");
	printf ("**\n");
	printf ("** Report bugs to:\n**\n");
	printf ("**     <nopoll@lists.aspl.es> noPoll mailing list\n**\n");
	printf ("**     <info@aspl.es> ASPL's contact \n**\n");

	/* configure port offset before anything else: every port used
	 * by the tests is derived from it */
	if (! regtest_configure_port_offset (argc, argv))
		return -1;

	iterator = 1;
	while (iterator < argc) {
		/* check for debug */
		printf ("Checking agument: %s\n", argv[iterator]);
		if (nopoll_cmp (argv[iterator], "--debug")) {
			printf ("Activating debug..\n");
			debug = nopoll_true;
		} /* end if */
		if (nopoll_cmp (argv[iterator], "--show-critical-only")) {
			printf ("Activating reporting of critical messages..\n");
			show_critical_only = nopoll_true;
		} /* end if */

		/* next position */
		iterator++;
	}

#if defined(__NOPOLL_PTHREAD_SUPPORT__)	
	printf ("INFO: install default threading functions to check noPoll locking code..\n");
	nopoll_thread_handlers (__nopoll_regtest_mutex_create,
				__nopoll_regtest_mutex_destroy,
				__nopoll_regtest_mutex_lock,
				__nopoll_regtest_mutex_unlock);
#endif

	printf ("INFO: starting tests with pid: %d\n", getpid ());
	if (test_01_strings ()) {
		printf ("Test 01-strings: Library strings support [   OK   ]\n");
	}else {
		printf ("Test 01-strings: Library strings support [ FAILED ]\n");
		return -1;
	}

	if (test_01_base64 ()) {
		printf ("Test 01-base64: Library base64 support [   OK   ]\n");
	}else {
		printf ("Test 01-base64: Library base64 support [ FAILED ]\n");
		return -1;
	}

	if (test_01_support ()) {
		printf ("Test 01-support: library support functions [   OK   ]\n");
	}else {
		printf ("Test 01-support: library support functions [ FAILED ]\n");
		return -1;
	}

	if (test_01_masking ()) {
		printf ("Test 01-masking: Library websocket content masking support [   OK   ]\n");
	}else {
		printf ("Test 01-masking: Library websocket content masking support [ FAILED ]\n");
		return -1;
	}

	if (test_01 ()) {	
		printf ("Test 01: Simple connect and disconnect [   OK   ]\n");
	}else {
		printf ("Test 01: Simple connect and disconnect [ FAILED ]\n");
		return -1;
	}

	if (test_02 ()) {	
		printf ("Test 02: Simple request/reply [   OK   ]\n");
	}else {
		printf ("Test 02: Simple request/reply [ FAILED ]\n");
		return -1;
	}

	if (test_02a ()) {	
		printf ("Test 02a: Simple request/reply (IPv6) [   OK   ]\n");
	}else {
		printf ("Test 02a: Simple request/reply (IPv6) [ FAILED ]\n");
		return -1;
	}

	/* test sending ping */
	if (test_02b ()) {	
		printf ("Test 02b: test sending ping [   OK   ]\n");
	}else {
		printf ("Test 02a: test sending ping [ FAILED ]\n");
		return -1;
	}

	/* test sending pong (without ping) */

	/* test streaming api */
	if (test_03 ()) {	
		printf ("Test 03: test streaming api [   OK   ]\n");
	}else {
		printf ("Test 03: test streaming api [ FAILED ]\n");
		return -1;
	}

	if (test_04 (1024)) {	
		printf ("Test 04: test streaming api (II) [   OK   ]\n");
	}else {
		printf ("Test 04: test streaming api (II) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (512)) {	
		printf ("Test 04-a: test streaming api (III) [   OK   ]\n");
	}else {
		printf ("Test 04-a: test streaming api (III) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (137)) {	
		printf ("Test 04-b: test streaming api (IV) [   OK   ]\n");
	}else {
		printf ("Test 04-b: test streaming api (IV) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (17)) {	
		printf ("Test 04-c: test streaming api (V) [   OK   ]\n");
	}else {
		printf ("Test 04-c: test streaming api (V) [ FAILED ]\n");
		return -1;
	}

	if (test_04a ()) {
		printf ("Test 04-a: check non-blocking streaming and message based API  [   OK   ]\n");
	} else {
		printf ("Test 04-a: check non-blocking streaming and message based API [ FAILED ]\n");
		return -1;
	}

	if (test_04b ()) {
		printf ("Test 04-b: try to overflow write access and recover from it  [   OK   ]\n");
	} else {
		printf ("Test 04-b: try to overflow write access and recover from it [ FAILED ]\n");
		return -1;
	}

	if (test_04c ()) {
		printf ("Test 04-c: send a file and try to overflow (but retry)  [   OK   ]\n");
	} else {
		printf ("Test 04-c: send a file and try to overflow (but retry) [ FAILED ]\n");
		return -1;
	}

	if (test_05 ()) {
		printf ("Test 05: sending utf-8 content [   OK   ]\n");
	} else {
		printf ("Test 05: sending utf-8 content [ FAILED ]\n");
		return -1;
	}

	if (test_06 ()) {
		printf ("Test 06: testing basic TLS connect [   OK   ]\n");
	} else {
		printf ("Test 06: testing basic TLS connect [ FAILED ]\n");
		return -1;
	}

	if (test_06a ()) {
		printf ("Test 06a: testing basic TLS connect (IPv6) [   OK   ]\n");
	} else {
		printf ("Test 06a: testing basic TLS connect (IPv6) [ FAILED ]\n");
		return -1;
	}

	if (test_07 ()) {
		printf ("Test 07: testing TLS request/reply [   OK   ]\n");
	} else {
		printf ("Test 07: testing TLS request/reply [ FAILED ]\n");
		return -1;
	}

	if (test_08 ()) {
		printf ("Test 08: test normal connect to TLS port [   OK   ]\n");
	} else {
		printf ("Test 08: test normal connect to TLS port [ FAILED ]\n");
		return -1;
	}

	if (test_09 ()) {
		printf ("Test 09: ensure we only support Sec-WebSocket-Version: 13 [   OK   ]\n");
	} else {
		printf ("Test 09: ensure we only support Sec-WebSocket-Version: 13 [ FAILED ]\n");
		return -1;
	}

	if (test_10 ()) {
		printf ("Test 10: test checking origing in on open and denying it [   OK   ]\n");
	} else {
		printf ("Test 10: test checking origing in on open and denying it [ FAILED ]\n");
		return -1;
	}

	if (test_11 ()) {
		printf ("Test 11: release context after connection [   OK   ]\n");
	} else {
		printf ("Test 11: release context after connection [ FAILED ]\n");
		return -1;
	}

	if (test_12 ()) {
		printf ("Test 12: create huge amount of connections in a short time [   OK   ]\n");
	} else {
		printf ("Test 12: create huge amount of connections in a short time [ FAILED ]\n");
		return -1;
	}

	if (test_13 ()) {
		printf ("Test 13: testing certificate storage [   OK    ]\n");
	} else {
		printf ("Test 13: testing certificate storage [ FAILED  ]\n");
		return -1;
	}

	if (test_14 ()) {
		printf ("Test 14: testing sending frame with few content as indicated by header [   OK    ]\n");
	} else {
		printf ("Test 14: testing sending frame with few content as indicated by header [ FAILED  ]\n");
		return -1;
	}

	if (test_15 ()) {
		printf ("Test 15: checking non-blocking calls to get messages when no content is available [   OK    ]\n");
	} else {
		printf ("Test 15: checking non-blocking calls to get messages when no content is available [ FAILED  ]\n");
		return -1;
	}

	if (test_16 ()) {
		printf ("Test 16: check sending frames with sleep in header [   OK    ]\n");
	} else {
		printf ("Test 16: check sending frames with sleep in header [ FAILED  ]\n");
		return -1;
	}

	if (test_17 ()) {
		printf ("Test 17: check partial frame reception [   OK    ]\n");
	} else {
		printf ("Test 17: check partial frame reception [ FAILED  ]\n");
		return -1;
	}

	if (test_18 ()) {
		printf ("Test 18: check nopoll_loop_wait (second call) [   OK    ]\n");
	} else {
		printf ("Test 18: check nopoll_loop_wait (second call) [ FAILED  ]\n");
		return -1;
	}

	if (test_19 ()) {
		printf ("Test 19: support different SSL methods (SSLv23, SSLv3, TLSv1 [   OK    ]\n");
	} else {
		printf ("Test 19: support different SSL methods (SSLv23, SSLv3, TLSv1 [ FAILED  ]\n");
		return -1;
	}

#if defined(__NOPOLL_PTHREAD_SUPPORT__)
	if (test_20 ()) {
		printf ("Test 20: check mutex support [   OK    ]\n");
	} else {
		printf ("Test 20: check mutex support [ FAILED  ]\n");
		return -1;
	}
#endif

	if (test_21 ()) {
		printf ("Test 21: client side ssl certificates verification  [   OK    ]\n");
	} else {
		printf ("Test 21: client side ssl certificates verification [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_22 ()) {
		printf ("Test 22: test connection close trigger (client side)  [   OK    ]\n");
	} else {
		printf ("Test 22: test connection close trigger (client side) [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_23 ()) {
		printf ("Test 23: test connection close trigger (server side)  [   OK    ]\n");
	} else {
		printf ("Test 23: test connection close trigger (server side) [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_24 ()) {
		printf ("Test 24: check cookie support (client and server side)  [   OK    ]\n");
	} else {
		printf ("Test 24: check cookie support (client and server side)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_25 ()) {
		printf ("Test 25: check cookie attack  [   OK    ]\n");
	} else {
		printf ("Test 25: check cookie attack  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_26 ()) {
		printf ("Test 26: checking echo.websocket.org  [   OK    ]\n");
	} else {
		printf ("Test 26: checking echo.websocket.org  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_27 ()) {
		printf ("Test 27: checking setting protocol  [   OK    ]\n");
	} else {
		printf ("Test 27: checking setting protocol  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_28 ()) {
		printf ("Test 28: checking setting protocol  [   OK    ]\n");
	} else {
		printf ("Test 28: checking setting protocol  [ FAILED  ]\n");
		return -1;
	} /* end if */
 
	if (test_29 ()) {
		printf ("Test 29: checking extra http headers  [   OK    ]\n");
	} else {
		printf ("Test 29: checking extra http headers  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_30 ()) {
		printf ("Test 30: simulate stop in the middle of the header send  [   OK    ]\n");
	} else {
		printf ("Test 30: simulate stop in the middle of the header send  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_31 ()) {
		printf ("Test 31: simulate stop in the middle of the header send (II)  [   OK    ]\n");
	} else {
		printf ("Test 31: simulate stop in the middle of the header send (II)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_32 ()) {
		printf ("Test 32: simulate stop in the middle of the header send (III)  [   OK    ]\n");
	} else {
		printf ("Test 32: simulate stop in the middle of the header send (III)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_33 ()) {
		printf ("Test 33: simulate stop in the middle of the header send (IV)  [   OK    ]\n");
	} else {
		printf ("Test 33: simulate stop in the middle of the header send (IV)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_34 ()) {
		printf ("Test 34: simulate stop in the middle of the header send (V)  [   OK    ]\n");
	} else {
		printf ("Test 34: simulate stop in the middle of the header send (V)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_35 ()) {
		printf ("Test 35: simulate stop in the middle of the header send (VI)  [   OK    ]\n");
	} else {
		printf ("Test 35: simulate stop in the middle of the header send (VI)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_36 ()) {
		printf ("Test 36: check dead-lock on connection timeout (23/02/2016)  [   OK    ]\n");
	} else {
		printf ("Test 36: check dead-lock on connection timeout (23/02/2016) [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_37 ()) {
		printf ("Test 37: reject frame with 64bit payload size overflow (issue #84)  [   OK    ]\n");
	} else {
		printf ("Test 37: reject frame with 64bit payload size overflow (issue #84)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_38 ()) {
		printf ("Test 38: reject frame with payload size truncated to -1 (issue #84)  [   OK    ]\n");
	} else {
		printf ("Test 38: reject frame with payload size truncated to -1 (issue #84)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_39 ()) {
		printf ("Test 39: reject frame bigger than default max frame size  [   OK    ]\n");
	} else {
		printf ("Test 39: reject frame bigger than default max frame size  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_40 ()) {
		printf ("Test 40: check max frame size configuration (ctx and conn opts)  [   OK    ]\n");
	} else {
		printf ("Test 40: check max frame size configuration (ctx and conn opts)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_41 ()) {
		printf ("Test 41: check client side Sec-WebSocket-Accept validation  [   OK    ]\n");
	} else {
		printf ("Test 41: check client side Sec-WebSocket-Accept validation  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_42 ()) {
		printf ("Test 42: check binary fragment is sent with FIN = 0  [   OK    ]\n");
	} else {
		printf ("Test 42: check binary fragment is sent with FIN = 0  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_43 ()) {
		printf ("Test 43: check listener creation parameter validation  [   OK    ]\n");
	} else {
		printf ("Test 43: check listener creation parameter validation  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_44 ()) {
		printf ("Test 44: check conn opts reference lifecycle (reuse)  [   OK    ]\n");
	} else {
		printf ("Test 44: check conn opts reference lifecycle (reuse)  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_45 ()) {
		printf ("Test 45: check connection close from a notification handler  [   OK    ]\n");
	} else {
		printf ("Test 45: check connection close from a notification handler  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_46 ()) {
		printf ("Test 46: check conn opts release on listener creation failure  [   OK    ]\n");
	} else {
		printf ("Test 46: check conn opts release on listener creation failure  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_47 ()) {
		printf ("Test 47: check nopoll_msg_join () allocation failure handling  [   OK    ]\n");
	} else {
		printf ("Test 47: check nopoll_msg_join () allocation failure handling  [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_48 ()) {
		printf ("Test 48: check several frames inside the same TLS record     [   OK    ]\n");
	} else {
		printf ("Test 48: check several frames inside the same TLS record     [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_49 ()) {
		printf ("Test 49: check log handler notification for critical logs    [   OK    ]\n");
	} else {
		printf ("Test 49: check log handler notification for critical logs    [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_50 ()) {
		printf ("Test 50: check context connection list growth               [   OK    ]\n");
	} else {
		printf ("Test 50: check context connection list growth               [ FAILED  ]\n");
		return -1;
	} /* end if */

	if (test_51 ()) {
		printf ("Test 51: check frame header split from its mask             [   OK    ]\n");
	} else {
		printf ("Test 51: check frame header split from its mask             [ FAILED  ]\n");
		return -1;
	} /* end if */

	/* add support to reply with redirect 301 to an opening
	 * request: page 19 and 22 */

	/* add support for basic HTTP auth before proceding with the
	 * handshake. The the possibility to use htpasswd tools. Page 19 and 22 */

	/* add support to define cookies by the server: page 20 */

	/* update the library to split message frames into smaller
	 * complete frames when bigger messages are received. */

	/* add support for proxy mode */

	/* check control files aren't flagged as fragmented */
	
	/* upload a file to the server ...*/

	/* more streaming api testing, get bigger content as a
	 * consequence of receiving several messages */

	/* test streaming API when it timeouts */

	/* test sending wrong mime headers */

	/* test sending missing mime headers */

	/* test injecting wrong bytes */

	/* test sending lot of MIME headers (really lot of
	 * information) */

	/* test checking protocols and denying it */

	/* test sending frames 126 == ( 65536) */

	/* test sending frames 127 == ( more than 65536) */

	/* test applying limits to incoming content */

	/* test splitting into several frames content bigger */

	/* test wrong UTF-8 content received on text frames */

	/* add support to sending close frames with status code and a
	 * textual indication as defined by page 36 */

	/* call to cleanup */
	nopoll_cleanup_library ();
	printf ("All tests ok!!\n");


	return 0;
}

/* end-of-file-found */

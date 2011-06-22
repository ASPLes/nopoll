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
#include <nopoll.h>

nopoll_bool debug = nopoll_false;

noPollCtx * create_ctx (void) {
	
	/* create a context */
	noPollCtx * ctx = nopoll_ctx_new ();
	nopoll_log_enable (ctx, debug);
	nopoll_log_color_enable (ctx, debug);
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
			printf ("ERROR: failed to encode this is a test..\n");
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

	/* call to create a listener */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 1) {
		printf ("ERROR: expected to find 1 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	/* ensure connection status is ok */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR (3): expected to find proper connection status, but found failure..\n");
		return nopoll_false;
	}

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4): expected to find proper connection handshake finished, but found it is still not prepared..\n");
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

	/* call to create a listener */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {
		printf ("Message still not received..\n");
		nopoll_sleep (10000);
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}



int main (int argc, char ** argv)
{
	int iterator;

	printf ("** NoPoll: Websocket toolkit (regression test).\n");
	printf ("** Copyright (C) 2011 Advanced Software Production Line, S.L.\n**\n");
	printf ("** NoPoll regression tests: version=%s\n**\n",
		VERSION);
	printf ("** To gather information about time performance you can use:\n**\n");
	printf ("**     >> time ./nopoll-regression-client\n**\n");
	printf ("** To gather information about memory consumed (and leaks) use:\n**\n");
	printf ("**     >> libtool --mode=execute valgrind --leak-check=yes --error-limit=no ./nopoll-regression-client\n**\n");
	printf ("**\n");
	printf ("** Report bugs to:\n**\n");
	printf ("**     <info@aspl.es> noPoll mailing list\n**\n");

	iterator = 1;
	while (iterator < argc) {
		/* check for debug */
		printf ("Checking agument: %s\n", argv[iterator]);
		if (nopoll_cmp (argv[iterator], "--debug")) {
			printf ("Activating debug..\n");
			debug = nopoll_true;
		} /* end if */

		/* next position */
		iterator++;
	}

	if (test_01_strings ()) {
		printf ("Test 01-strings: Library strings support [   OK   ]\n");
	}else {
		printf ("Test 01-strings: Library strings support [ FAILED ]\n");
		return -1;
	}

	if (test_01_base64 ()) {
		printf ("Test 01-bas64: Library bas64 support [   OK   ]\n");
	}else {
		printf ("Test 01-bas64: Library bas64 support [ FAILED ]\n");
		return -1;
	}

	if (test_01 ()) {	
		printf ("Test 01: Simple connect and disconnect [   OK   ]\n");
	}else {
		printf ("Test 01: Simple connect and disconnect [ FAILED ]\n");
		return -1;
	}

	if (test_02 ()) {	
		printf ("Test 01: Simple request/reply [   OK   ]\n");
	}else {
		printf ("Test 01: Simple request/reply [ FAILED ]\n");
		return -1;
	}


	return 0;
}

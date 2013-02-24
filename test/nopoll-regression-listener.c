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

void listener_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr * user_data)
{
	const char * content = (const char *) nopoll_msg_get_payload (msg);
	FILE       * file;
	char         buffer[1024];
	int          bytes;
	int          sent;

	printf ("Listener received (size: %d): '%s'\n", 
		nopoll_msg_get_payload_size (msg),
		(const char *) nopoll_msg_get_payload (msg));

	if (nopoll_cmp (content, "ping")) {
		/* send a ping */
		nopoll_conn_send_ping (conn);
		return;
	} else if (nopoll_cmp (content, "get-file")) {
		file = fopen ("nopoll-regression-client.c", "rb");
		while (! feof (file)) {
			/* read content */
			bytes = fread (buffer, 1, 1024, file);
			/* send content */
			if (bytes > 0) {
				/* send content and get the result */
				sent = nopoll_conn_send_text (conn, buffer, bytes);
				if (sent != bytes)
					printf ("ERROR: expected to send %d bytes but sent different content size (%d bytes)..\n", bytes, sent);
			} /* end if */
			/* next */
		} /* end while */

		/* now close the handle */
		fclose (file);
	} /* end if */

	/* send reply as received */
	nopoll_conn_send_text (conn, (const char *) nopoll_msg_get_payload (msg), 
			       nopoll_msg_get_payload_size (msg));

	return;
}

int main (int argc, char ** argv)
{
	noPollCtx      * ctx;
	noPollConn     * listener;
	int              iterator;

	/* create the context */
	ctx = nopoll_ctx_new ();

	iterator = 1;
	while (iterator < argc) {
		/* check for debug */
		printf ("Checking agument: %s\n", argv[iterator]);
		if (nopoll_cmp (argv[iterator], "--debug")) {
			printf ("Activating debug..\n");
			nopoll_log_enable (ctx, nopoll_true);
			nopoll_log_color_enable (ctx, nopoll_true);
		} /* end if */

		/* next position */
		iterator++;
	}

	/* call to create a listener */
	listener = nopoll_listener_new (ctx, "localhost", "1234");
	if (! nopoll_conn_is_ok (listener)) {
		printf ("ERROR: Expected to find proper listener connection status, but found..\n");
		return -1;
	}

	printf ("noPoll listener started at: %s:%s..\n", nopoll_conn_host (listener), nopoll_conn_port (listener));

	/* set on message received */
	nopoll_ctx_set_on_msg (ctx, listener_on_message, NULL);
	

	/* process events */
	nopoll_loop_wait (ctx, 0);

	return 0;
}

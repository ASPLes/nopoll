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
#include <nopoll_msg.h>
#include <nopoll_private.h>

/** 
 * @brief Allows to get a reference to the payload content inside the
 * provided websocket message.
 *
 * @param msg The websocket message to get the payload from.
 *
 * @return A reference to the payload or NULL if it fails. See \ref
 * nopoll_msg_get_payload_size to get payload size.
 */
const noPollPtr nopoll_msg_get_payload (noPollMsg * msg)
{
	if (msg == NULL)
		return NULL;
	return msg->payload;
}

/** 
 * @brief Allows to get the payload byte length stored on the provided
 * message.
 *
 * @param msg The websocket message to get the payload from.
 *
 * @return The payload size or -1 if it fails (only when msg is NULL).
 */
int          nopoll_msg_get_payload_size (noPollMsg * msg)
{
	if (msg == NULL)
		return -1;
	return msg->payload_size;
}

/** 
 * @brief Allows to acquire a reference to the provided websocket
 * message.
 *
 * @param msg The websocket message to acquire a reference.
 *
 * @return nopoll_true if the reference was acquired, otherwise
 * nopoll_false is returned.
 */
nopoll_bool  nopoll_msg_ref (noPollMsg * msg)
{
	/* check recieved reference */
	if (msg == NULL)
		return nopoll_false;

	/* acquire mutex here */
	msg->refs++;

	/* release mutex here */
	return nopoll_true;
}

/** 
 * @brief Allows to release the reference acquired, finished the
 * object if all references are terminated.
 *
 * @param msg The websocket message to be finished.
 */
void         nopoll_msg_unref (noPollMsg * msg)
{
	if (msg == NULL)
		return;
	
	/* acquire mutex here */
	msg->refs--;
	if (msg->refs != 0) {
		/* release mutex here */
		return;
	}

	/* free websocket message */
	nopoll_free (msg->payload);
	nopoll_free (msg);

	/* release mutex here */
	return;
}



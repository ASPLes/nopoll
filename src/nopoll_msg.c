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
#include <nopoll_msg.h>
#include <nopoll_private.h>

#include <limits.h>

/** 
 * \defgroup nopoll_msg noPoll Message: functions for handling and using noPoll messages (websocket messages)
 */

/** 
 * \addtogroup nopoll_msg
 * @{
 */

/** 
 * @internal function that creates an empty message holder.
 * @return A newly created reference or NULL if it fails. 
 */
noPollMsg  * nopoll_msg_new (void)
{
	noPollMsg * msg = nopoll_new (noPollMsg, 1);
	if (msg == NULL)
		return NULL;

	msg->refs = 1;
	msg->ref_mutex = nopoll_mutex_create ();

	return msg;
}

/** 
 * @brief Allows to get a reference to the payload content inside the
 * provided websocket message.
 *
 * @param msg The websocket message to get the payload from.
 *
 * @return A reference to the payload or NULL if it fails. See \ref
 * nopoll_msg_get_payload_size to get payload size.
 */
const unsigned char * nopoll_msg_get_payload (noPollMsg * msg)
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
 *
 * NOTE: this function returned an <b>int</b> up to noPoll 1.2.x. It
 * now returns a <b>long int</b> to be able to report payload sizes
 * bigger than 2GB without truncating them (the value stored inside
 * the message was already a long int). Callers storing the result
 * into an int must be updated.
 */
long int     nopoll_msg_get_payload_size (noPollMsg * msg)
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
	/* check received reference */
	if (msg == NULL)
		return nopoll_false;

	/* acquire mutex here */
	nopoll_mutex_lock (msg->ref_mutex);

	msg->refs++;

	/* release mutex here */
	nopoll_mutex_unlock (msg->ref_mutex);

	return nopoll_true;
}

/** 
 * @brief Allows to get current reference counting for the provided
 * message.
 *
 * @param msg The message for which we are requesting for the
 * reference counting.
 *
 * @return Reference counting or -1 if it fails (returned when msg
 * reference received is NULL).
 */
int          nopoll_msg_ref_count (noPollMsg * msg)
{
	int result;

	/* check received reference */
	if (msg == NULL)
		return -1;

	/* acquire mutex here */
	nopoll_mutex_lock (msg->ref_mutex);

	result = msg->refs;

	/* release mutex here */
	nopoll_mutex_unlock (msg->ref_mutex);

	return result;
}

/** 
 * @brief Allows to get if the provided message reference has FIN flag
 * on (or off) to indicate if it is a final frame.
 *
 * When a series of messages are received and they conform together a
 * single message, the last message is flagged with FIN = 1 while the
 * rest before go with FIN = 0. 
 *
 * For example, if a user level application is splitted into 4 frame
 * fragments, then the WebSocket peer will receive 3 fragments with
 * FIN = 0 and the last fragment with FIN = 1.
 *
 * You can use \ref nopoll_msg_is_fragment to know if a particular
 * message was produced due to a fragmentation found at the network
 * level. This happens when the entire frame wasn't sent or it
 * couldn't be read entirely. In the example before, the four frames
 * will be also flagged as fragments too.
 *
 * @param msg The message that is being checked for FIN flag.
 *
 * @return nopoll_true if the message is a final one, otherwise
 * nopoll_false is returned. The function returns nopoll_false when
 * message reference received is NULL.
 *
 */
nopoll_bool  nopoll_msg_is_final (noPollMsg * msg)
{
	if (msg == NULL)
		return nopoll_false;

	return msg->remain_bytes == 0 && msg->has_fin;
}

/** 
 * @brief Allows to check if the message represents a frame fragment.
 *
 * The function allows to check if the provided noPollMsg is a
 * fragment from a bigger frame or message that was splitted as a
 * consequence of not being able to read the entire frame or because
 * it wasn't sent complete from the other side. See \ref
 * nopoll_msg_is_final for more information.
 *
 * The function also returns that the message is a fragment when the frame has FIN = 0.
 *
 * @param msg The message checked to be a fragment or not.
 *
 * @return nopoll_true if the message is a fragment, otherwise
 * nopoll_false is returned.
 */
nopoll_bool  nopoll_msg_is_fragment (noPollMsg * msg)
{
	if (msg == NULL)
		return nopoll_false;
	return msg->is_fragment || msg->has_fin == 0;
}

/** 
 * @brief Get message OpCode to get the type of message that was
 * received.
 *
 * @param msg The message that is being checked for its OpCode
 *
 * @return The op code or -1 in the case NULL reference is received.
 */
noPollOpCode nopoll_msg_opcode (noPollMsg * msg)
{
	if (msg == NULL)
		return NOPOLL_UNKNOWN_OP_CODE;
	return (noPollOpCode) msg->op_code;
}

/** 
 * @brief Allows to join the provided noPollMsg references to create a
 * newly allocated message (or reusing same reference but increasing reference
 * counting) that contains both content.
 *
 * @param msg The message to be join to the next message. Headers from
 * this message will be used as reference for the headers to be
 * used. In the case this is NULL, the second argument will be used as
 * argument and reference counting will be updated.
 *
 * @param msg2 The message to be join as a second part for the first
 * argument. 
 *
 * Here are some examples showing how the function works. The notation
 * along the argument indicates the reference counting at the end of
 * the function.
 *
 * msgA (2) = nopoll_msg_join (msgA (1), NULL);
 * msgB (2) = nopoll_msg_join (NULL, msgB (1));
 * msgC (1) = nopoll_msg_join (msgA (1), msgB (1));
 * NULL     = nopoll_msg_join (NULL, NULL);
 *
 * Note the function never consumes the references received: the
 * caller still owns <b>msg</b> and <b>msg2</b> and must call to \ref
 * nopoll_msg_unref on them when they are no longer needed.
 *
 * CAVEAT about the resulting message flags: as stated before, the
 * headers used for the resulting message are taken from the first
 * argument (<b>msg</b>). Because a fragmented message carries FIN = 0
 * on every fragment but the last one, the message returned by this
 * function will report \ref nopoll_msg_is_final == nopoll_false and
 * \ref nopoll_msg_is_fragment == nopoll_true even when it already
 * holds the complete content. Do not use those functions over the
 * joined message to detect completion: track it over the messages
 * received from \ref nopoll_conn_get_msg (the last fragment received
 * is the one flagging FIN = 1) as the regression listener does.
 *
 * @return The function returns the newly allocated message (with one
 * reference) or, when only one of the arguments is defined, that same
 * reference with its reference counting increased. NULL is returned
 * if it fails, which happens when both arguments are NULL, when the
 * memory needed for the joined content cannot be allocated or when
 * any of the messages received reports an inconsistent payload size
 * (negative, without payload, or big enough to make the resulting
 * size not representable).
 */
noPollMsg  * nopoll_msg_join (noPollMsg * msg, noPollMsg * msg2)
{
	noPollMsg * result;

	/* check for basic cases */
	if (msg == NULL && msg2 == NULL)
		return NULL;
	if (msg == NULL && msg2) {
		nopoll_msg_ref (msg2);
		return msg2;
	} /* end if */
	if (msg && msg2 == NULL) {
		nopoll_msg_ref (msg);
		return msg;
	} /* end if */

	/* check both messages report a consistent payload size before
	 * using it: payload_size is signed, so a negative value would be
	 * converted into a huge size_t when allocating and would make the
	 * memcpy operations below read out of bounds */
	if (msg->payload_size < 0 || msg2->payload_size < 0)
		return NULL;
	if ((msg->payload == NULL && msg->payload_size > 0) ||
	    (msg2->payload == NULL && msg2->payload_size > 0))
		return NULL;

	/* check the resulting size can be represented: the sum is done
	 * with signed values so an overflow here is undefined behaviour
	 * (the extra byte reserved for the string terminator is also
	 * taken into account) */
	if (msg->payload_size > (LONG_MAX - 1) - msg2->payload_size)
		return NULL;

	/* now, join content */
	result            = nopoll_msg_new ();
	if (result == NULL)
		return NULL;
	result->has_fin   = msg->has_fin;
	result->op_code   = msg->op_code;
	result->is_masked = msg->is_masked;
	if (result->is_masked)
		memcpy (result->mask, msg->mask, 4);

	/* copy payload size and content */
	result->payload_size = msg->payload_size + msg2->payload_size;
	result->payload = nopoll_new (char, (size_t) (result->payload_size + 1));
	if (result->payload == NULL) {
		/* release the holder created to avoid leaking it */
		nopoll_msg_unref (result);
		return NULL;
	} /* end if */

	/* copy content from first message (skipped when the message
	 * carries no payload: passing a NULL source to memcpy is
	 * undefined even when copying 0 bytes) */
	if (msg->payload_size > 0)
		memcpy (result->payload, msg->payload, (size_t) msg->payload_size);

	/* copy content from second message */
	if (msg2->payload_size > 0)
		memcpy (((unsigned char *) result->payload) + msg->payload_size , msg2->payload, (size_t) msg2->payload_size);

	/* return joined message */
	return result;
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
	nopoll_mutex_lock (msg->ref_mutex);

	msg->refs--;
	if (msg->refs != 0) {
		/* release mutex here */
		nopoll_mutex_unlock (msg->ref_mutex);
		return;
	}
	/* release mutex */
	nopoll_mutex_unlock (msg->ref_mutex);
	nopoll_mutex_destroy (msg->ref_mutex);

	/* free websocket message */
	nopoll_free (msg->payload);
	nopoll_free (msg);

	return;
}

/**
 * @}
 */

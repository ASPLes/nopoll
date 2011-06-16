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
#include <nopoll_loop.h>
#include <nopoll_private.h>

/** 
 * @brief Performs a timeval substract leaving the result in
 * (result). Subtract the `struct timeval' values a and b, storing the
 * result in result.
 *
 * @param a First parameter to substract
 *
 * @param b Second parameter to substract
 *
 * @param result Result variable. Do no used a or b to place the
 * result.
 *
 * @return 1 if the difference is negative, otherwise 0 (operations
 * implemented is a - b).
 */ 
int     nopoll_timeval_substract                  (struct timeval * a, 
						   struct timeval * b,
						   struct timeval * result)
{
	/* Perform the carry for the later subtraction by updating
	 * y. */
	if (a->tv_usec < b->tv_usec) {
		int nsec = (b->tv_usec - a->tv_usec) / 1000000 + 1;
		b->tv_usec -= 1000000 * nsec;
		b->tv_sec += nsec;
	}

	if (a->tv_usec - b->tv_usec > 1000000) {
		int nsec = (a->tv_usec - b->tv_usec) / 1000000;
		b->tv_usec += 1000000 * nsec;
		b->tv_sec -= nsec;
	}
	
	/* get the result */
	result->tv_sec = a->tv_sec - b->tv_sec;
	result->tv_usec = a->tv_usec - b->tv_usec;
     
       /* return 1 if result is negative. */
       return a->tv_sec < b->tv_sec;	
}

/** 
 * @brief Allows to implement a wait over all connections registered
 * under the provided context during the provided timeout until
 * something is detected meaningful to the user, calling to the action
 * handler defined, optionally receving the user data pointer.
 *
 * @param ctx The context object where the wait will be implemented.
 *
 * @param timeout The timeout to wait for changes. If no changes
 * happens, the function returns.
 *
 * @return The function returns the number of connections that changed
 * its status during the wait or -1 in the case timeout was
 * reached. The function returns -2 in the case ctx is NULL or timeout
 * is negative.
 */
int nopoll_loop_wait (noPollCtx * ctx, long timeout)
{
	struct timeval start;
	struct timeval stop;
	struct timeval diff;
	long           elapsed;
	int            wait_status;

	nopoll_return_val_if_fail (ctx, ctx, -2);
	nopoll_return_val_if_fail (ctx, timeout < 0, -2);
	
	/* grab the mutex for the following check */
	if (ctx->io_engine == NULL) {
		ctx->io_engine = nopoll_io_get_engine (ctx, NOPOLL_IO_ENGINE_DEFAULT);
		if (ctx->io_engine == NULL) {
			nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to create IO wait engine, unable to implement wait call");
			return -2;
		} 
	} /* end if */
	/* release the mutex */

	/* get as reference current time */
	gettimeofday (&start, NULL);
	
	while (nopoll_true) {
		/* ok, now implement wait operation */
		ctx->io_engine->clear (ctx, ctx->io_engine->io_object);

		/* add all connections */
		
		/* implement wait operation */
		wait_status = ctx->io_engine->wait (ctx, ctx->io_engine->io_object);

		/* check and call for connections with something
		 * interesting */

		/* check to stop wait operation */
		
		/* get as reference current time */
		gettimeofday (&stop, NULL);
		nopoll_timeval_substract (&stop, &start, &diff);
		elapsed = (stop.tv_sec * 1000000) + stop.tv_usec;
		if (elapsed > timeout)
			break;
	} /* end while */

	return 0;
}



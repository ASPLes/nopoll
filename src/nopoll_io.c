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
#include <nopoll_io.h>
#include <nopoll_private.h>

typedef struct _noPollSelect {
	noPollCtx          * ctx;
	fd_set               set;
	int                  length;
	int                  max_fds;
} noPollSelect;

/** 
 * @internal nopoll implementation to create a compatible "select" IO
 * call fd set reference.
 *
 * @param ctx The context the fd set created will be associated to.
 *
 * @return A newly allocated \ref noPollSelect reference (which holds
 * the fd set) or NULL if it fails.
 */
noPollPtr nopoll_io_wait_select_create (noPollCtx * ctx) 
{
	noPollSelect * select = nopoll_new (noPollSelect, 1);

	if (select == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to allocate select fd set, unable to create io wait object");
		return NULL;
	} /* end if */

	/* set default behaviour expected for the set */
	select->ctx           = ctx;
	
	/* clear the set */
	FD_ZERO (&(select->set));

	return select;
}

/** 
 * @internal noPoll implementation to destroy the "select" IO call
 * created by the default create.
 *
 * @param ctx The context where the operation takes place.
 *
 * @param fd_group The fd group to be deallocated.
 */
void    nopoll_io_wait_select_destroy (noPollCtx * ctx, noPollPtr fd_group)
{
	noPollSelect * select = (noPollSelect *) fd_group;

	/* release memory allocated */
	nopoll_free (select);

	/* nothing more to do */
	return;
}

/** 
 * @internal noPoll implementation to clear the "select" IO call
 * created by the default create.
 *
 * @param ctx The context where the operation takes place.
 *
 * @param __fd_group The fd group to be cleared, so it can be reused
 * by the next wait operation.
 */
void    nopoll_io_wait_select_clear (noPollCtx * ctx, noPollPtr __fd_group)
{
	noPollSelect * select = (noPollSelect *) __fd_group;

	/* clear the fd set: max_fds must be reset too because it is
	 * recalculated by nopoll_io_wait_select_add_to () every time
	 * the connection set is registered again, otherwise select ()
	 * would be called with an nfds value that only grows */
	select->length  = 0;
	select->max_fds = 0;
	FD_ZERO (&(select->set));

	/* nothing more to do */
	return;
}

/**
 * @internal Default internal implementation for the wait operation:
 * blocks until at least one socket descriptor inside the fd set
 * provided changes its status, or until the internal wait period
 * (500ms) is exhausted.
 *
 * @param ctx The context where the operation takes place.
 *
 * @param __fd_group The fd set having all sockets to be watched.
 *
 * @return Number of socket descriptors that changed, 0 if the wait
 * finished without changes (wait period exhausted or the call was
 * interrupted by a signal) or -1 if it failed.
 *
 * NOTE: on return, the fd set received is modified in place to hold
 * only the descriptors that changed, which is what \ref
 * nopoll_io_wait_select_is_set reports afterwards. That is only
 * meaningful when this function returns a value greater than 0: for
 * any other result the content of the set must not be trusted.
 */
int nopoll_io_wait_select_wait (noPollCtx * ctx, noPollPtr __fd_group)
{
	int                 result = -1;
	struct timeval      tv;
	noPollSelect     * _select = (noPollSelect *) __fd_group;

	/* init wait */
	tv.tv_sec    = 0;
	tv.tv_usec   = 500000;
	result       = select (_select->max_fds + 1, &(_select->set), NULL,   NULL, &tv);

	/* check result: an interrupted wait is not a failure, just
	 * report that nothing changed so the caller keeps waiting
	 * instead of aborting the loop (see nopoll_loop_wait) */
	if ((result == NOPOLL_SOCKET_ERROR) && (errno == NOPOLL_EINTR))
		return 0;

	return result;
}

/** 
 * @internal noPoll select implementation for the "add to" on fd set
 * operation.
 * 
 * @param fds The socket descriptor to be added.
 *
 * @param ctx The context where the operation takes place.
 *
 * @param conn The connection owning the socket descriptor provided.
 *
 * @param __fd_set The fd set where the socket descriptor will be added.
 *
 * @return nopoll_true if the socket was added, otherwise nopoll_false
 * is returned.
 */
nopoll_bool  nopoll_io_wait_select_add_to (int               fds, 
					   noPollCtx       * ctx,
					   noPollConn      * conn,
					   noPollPtr         __fd_set)
{
	noPollSelect * select = (noPollSelect *) __fd_set;

	if (fds < 0 || fds >= FD_SETSIZE) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL,
			    "received a non valid socket (%d), unable to add to the set", fds);
		return nopoll_false;
	}
	if (select->length >= FD_SETSIZE) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL,
			    "Unable to add requested socket (%d), reached max FD_SETSIZE=%d (select->length=%d)", fds, FD_SETSIZE, select->length);
		return nopoll_false;
	} /* end if */

	/* set the value */
	FD_SET (fds, &(select->set));

	/* update length */
	select->length++;

	/* update max fds */
	if (fds > select->max_fds)
		select->max_fds = fds;

	return nopoll_true;
}

/** 
 * @internal
 *
 * @brief Default noPoll implementation for the "is set" on fd
 * set operation.
 * 
 * @param ctx The context where the operation takes place.
 *
 * @param fds The socket descriptor to be checked to be active on the
 * given fd group.
 *
 * @param __fd_set The fd set where the socket descriptor will be checked.
 *
 * @return nopoll_true if the socket descriptor is active on the fd
 * set, otherwise nopoll_false is returned.
 */
nopoll_bool      nopoll_io_wait_select_is_set (noPollCtx   * ctx,
					       int           fds, 
					       noPollPtr      __fd_set)
{
	noPollSelect * select = (noPollSelect *) __fd_set;
	
	if (fds < 0 || fds >= FD_SETSIZE) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL,
			    "received a non valid socket (%d), unable to test in the set", fds);
		return nopoll_false;
	}

	return FD_ISSET (fds, &(select->set));
}


/** 
 * @brief Creates an object that represents the best IO wait mechanism
 * found on the current system.
 *
 * @param ctx The context where the engine will be created/associated.
 *
 * @param engine_type Use \ref NOPOLL_IO_ENGINE_DEFAULT or the engine
 * you want to use. NOTE: only \ref NOPOLL_IO_ENGINE_SELECT is
 * implemented at this moment, so it is the mechanism returned for
 * every value requested (a warning is reported through the log when
 * asking for a different one).
 *
 * @return The selected IO wait mechanism or NULL if it fails.
 */
noPollIoEngine * nopoll_io_get_engine (noPollCtx * ctx, noPollIoEngineType engine_type)
{
	noPollIoEngine * engine = nopoll_new (noPollIoEngine, 1);
	if (engine == NULL)
		return NULL;

	/* report the caller it will not get the mechanism requested:
	 * select is the only engine implemented so far */
	if (engine_type != NOPOLL_IO_ENGINE_DEFAULT && engine_type != NOPOLL_IO_ENGINE_SELECT) {
		nopoll_log (ctx, NOPOLL_LEVEL_WARNING, "Requested io wait engine %d is not implemented, using select(2) based engine", engine_type);
	} /* end if */

	/* configure default implementation */
	engine->create  = nopoll_io_wait_select_create;
	engine->destroy = nopoll_io_wait_select_destroy;
	engine->clear   = nopoll_io_wait_select_clear;
	engine->wait    = nopoll_io_wait_select_wait;
	engine->add_to  = nopoll_io_wait_select_add_to;
	engine->is_set  = nopoll_io_wait_select_is_set;

	/* call to create the object */
	engine->ctx       = ctx;
	engine->io_object = engine->create (ctx);

	/* check the io object was created: without it every operation
	 * implemented by this engine (clear, add_to, is_set, wait)
	 * would dereference a NULL pointer */
	if (engine->io_object == NULL) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "Failed to create io wait object, unable to create io wait engine");
		nopoll_free (engine);
		return NULL;
	} /* end if */

	/* return the engine that was created */
	return engine;
}

/** 
 * @brief Release the io engine created by \ref nopoll_io_get_engine.
 *
 * @param engine The engine to be released.
 */
void             nopoll_io_release_engine (noPollIoEngine * engine)
{
	if (engine == NULL)
		return;
	engine->destroy (engine->ctx, engine->io_object);
	nopoll_free (engine);
	return;
}



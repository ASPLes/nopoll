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

#if defined(__NOPOLL_PTHREAD_SUPPORT__)
/** 
 * Needed for extended pthread API for recursive functions.
 */
#define _GNU_SOURCE 

#include <nopoll.h>

#include <pthread.h>

typedef struct _noPollMutex {
	pthread_mutex_t mutex;
} noPollMutex;

noPollPtr __nopoll_regtest_mutex_create (void) {
	pthread_mutex_t     * mutex;
	pthread_mutexattr_t   attr;
	int                   error;

	
	mutex = nopoll_new (pthread_mutex_t, 1);
	if (mutex == NULL) {
		printf ("ERROR: failed to allocate memory for mutex..\n");
		return NULL;
	}

	/* init the mutex using default values */
	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_NORMAL);
	error = pthread_mutex_init (mutex, &attr);
	if (error != 0) {
		printf ("ERROR: pthread_mutex_init () failed errno=%d %s..\n",
			error, strerror (error));
	} /* end if */

	pthread_mutexattr_destroy (&attr);

	return mutex;
}

void __nopoll_regtest_mutex_destroy (noPollPtr _mutex) {
	pthread_mutex_t * mutex = _mutex;
	if (mutex == NULL)
		return;

	pthread_mutex_destroy (mutex);
	nopoll_free (mutex);

	return;
}

void __nopoll_regtest_mutex_lock (noPollPtr _mutex) {
	pthread_mutex_t * mutex = _mutex;

	if (mutex == NULL) {
		printf ("...blocking because NULL mutex received..\n");
		nopoll_sleep (100000000);
	}

	/* lock the mutex */
	if (pthread_mutex_lock (mutex) != 0) {
		/* do some reporting */
		return;
	} /* end if */
	return;
}

void __nopoll_regtest_mutex_unlock (noPollPtr _mutex) {
	pthread_mutex_t * mutex = _mutex;

	/* unlock mutex */
	if (pthread_mutex_unlock (mutex) != 0) {
		/* do some reporting */
		return;
	} /* end if */
	return;
}
#endif

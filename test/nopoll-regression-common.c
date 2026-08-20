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

#include <nopoll-regression-common.h>

/**
 * @internal Offset added to every port used by the regression tests.
 */
int regtest_port_offset = 0;

/* number of buffers used by regtest_port () to allow calling it more
 * than once inside the same expression */
#define REGTEST_PORT_BUFFERS 8

static char __regtest_port_buffer[REGTEST_PORT_BUFFERS][12];
static int  __regtest_port_next   = 0;

/**
 * @internal Reads the --offset-port option from the arguments
 * received, storing the value found at \ref regtest_port_offset.
 *
 * Both "--offset-port 1000" and "--offset-port=1000" are accepted.
 *
 * @param argc The argument count received by main ().
 *
 * @param argv The argument values received by main ().
 *
 * @return nopoll_true if no option was found or if it was found with
 * a usable value, otherwise nopoll_false when the value provided is
 * missing or out of range (in that case the caller should abort
 * because running with the wrong ports would silently talk to a
 * different regression run).
 */
nopoll_bool regtest_configure_port_offset (int argc, char ** argv)
{
	int          iterator;
	const char * value;
	long         offset;
	char       * end;

	iterator = 1;
	while (iterator < argc) {
		value = NULL;

		if (nopoll_cmp (argv[iterator], "--offset-port")) {
			/* value provided as the next argument */
			if ((iterator + 1) >= argc) {
				printf ("ERROR: --offset-port requires a value (for example: --offset-port 1000)..\n");
				return nopoll_false;
			} /* end if */
			value = argv[iterator + 1];
			iterator++;
		} else if (! strncmp (argv[iterator], "--offset-port=", 14)) {
			/* value provided joined to the option */
			value = argv[iterator] + 14;
		} /* end if */

		if (value) {
			errno  = 0;
			end    = NULL;
			offset = strtol (value, &end, 10);

			/* reject anything that is not a complete number:
			 * strtol reports 0 for a value like "abc", which
			 * would silently run with no offset at all */
			if (errno != 0 || end == value || (end && *end != '\0')) {
				printf ("ERROR: wrong value provided to --offset-port (%s): it must be a number..\n", value);
				return nopoll_false;
			} /* end if */

			/* the biggest base port used by the regression
			 * tests is 2238, so keep the result inside the
			 * port range */
			if (offset < 0 || offset > 60000) {
				printf ("ERROR: wrong value provided to --offset-port (%ld): it must be between 0 and 60000..\n", offset);
				return nopoll_false;
			} /* end if */

			regtest_port_offset = (int) offset;
			printf ("INFO: using port offset: %d\n", regtest_port_offset);
		} /* end if */

		/* next position */
		iterator++;
	} /* end while */

	return nopoll_true;
}

/**
 * @internal Returns the port to use for the base port received, that
 * is, the base port plus the offset configured by --offset-port.
 *
 * @param base_port The port used by the test when no offset is configured.
 *
 * @return The port to use.
 */
int regtest_port_int (int base_port)
{
	return base_port + regtest_port_offset;
}

/**
 * @internal Same as \ref regtest_port_int but reporting the value as
 * the string representation required by the noPoll API.
 *
 * NOTE: the value returned points to an internal buffer that must not
 * be released. A rotating set of buffers is used so several calls can
 * be done inside the same expression, but this function is not
 * intended to be used from several threads at the same time.
 *
 * @param base_port The port used by the test when no offset is configured.
 *
 * @return The port to use, as a string.
 */
const char * regtest_port (int base_port)
{
	char * buffer;

	/* select the next buffer available */
	buffer = __regtest_port_buffer[__regtest_port_next];
	__regtest_port_next = (__regtest_port_next + 1) % REGTEST_PORT_BUFFERS;

	snprintf (buffer, sizeof (__regtest_port_buffer[0]), "%d", regtest_port_int (base_port));

	return buffer;
}

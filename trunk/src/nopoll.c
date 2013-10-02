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
#include <nopoll.h>
#include <nopoll_private.h>

/** 
 * \defgroup nopoll_support noPoll Support: core support functions used by the library
 */

/** 
 * \addtogroup nopoll_support
 * @{
 */

/** 
 * @brief Allows to check if provided strings are equal.
 *
 * @param string1 The first string to check. The string must be ended by 0.
 * @param string2 The second string to check. The string must be ended by 0.
 *
 * @return nopoll_true if both strings are equal, otherwise
 * nopoll_false is returned. 
 */
nopoll_bool nopoll_cmp (const char * string1, const char * string2)
{
	int iterator;
	if (string1 == NULL && string2 == NULL)
		return nopoll_true;
	if (string1 == NULL || string2 == NULL)
		return nopoll_false;

	/* next position */
	iterator = 0;
	while (string1[iterator] && string1[iterator]) {
		if (string1[iterator] != string2[iterator])
			return nopoll_false;
		iterator++;
	} /* end while */
	
	/* last check, ensure both ends with 0 */
	return string1[iterator] == string2[iterator];
}

/** 
 * @brief Allows to check if provided strings are equal for the first
 * bytes.
 *
 * @param string1 The first string to check. The string must be ended by 0.
 * @param string2 The second string to check. The string must be ended by 0.
 * @param bytes Number of bytes to check. The value must be > 0.
 *
 * @return nopoll_true if both strings are equal, otherwise
 * nopoll_false is returned. 
 */
nopoll_bool nopoll_ncmp (const char * string1, const char * string2, int bytes)
{
	int iterator;
	if (bytes <= 0)
		return nopoll_false;
	if (string1 == NULL && string2 == NULL)
		return nopoll_true;
	if (string1 == NULL || string2 == NULL)
		return nopoll_false;

	/* next position */
	iterator = 0;
	while (string1[iterator] && string1[iterator] && iterator < bytes) {
		if (string1[iterator] != string2[iterator])
			return nopoll_false;
		iterator++;
	} /* end while */
	
	/* last check, ensure both ends with 0 */
	return iterator == bytes;
}


/** 
 * @brief Allows to produce an newly allocated string produced by the
 * chunk received plus arguments, using the printf-like format.
 *
 * @param chunk The chunk to copy.
 * 
 * @return A newly allocated chunk.
 */
char      * nopoll_strdup_printf   (const char * chunk, ...)
{
	char    * result   = NULL;
	va_list   args;
	
	if (chunk == NULL)
		return NULL;

	/* open std args */
	va_start (args, chunk);

	/* get the string */
	result = nopoll_strdup_printfv (chunk, args);
	
	/* close std args */
	va_end (args);
	
	return result;
}

/** 
 * @internal Allows to calculate the amount of memory required to
 * store the string that will representing the construction provided
 * by the printf-like format received and its arguments.
 * 
 * @param format The printf-like format to be printed.
 *
 * @param args The set of arguments that the printf applies to.
 *
 * <i><b>NOTE:</b> not all printf specification is supported. Generally, the
 * following is supported: %s, %d, %f, %g, %ld, %lg and all
 * combinations that provides precision, number of items inside the
 * integer part, etc: %6.2f, %+2d, etc. An especial case not supported
 * is %lld, %llu and %llg.</i>
 *
 * @return Return the number of bytes that must be allocated to hold
 * the string (including the string terminator \0). If the format is
 * not correct or it is not properly formated according to the value
 * found at the argument set, the function will return -1.
 */
int nopoll_vprintf_len (const char * format, va_list args)
{
	/** IMPLEMENTATION NOTE: in the case this code is update,
	 * update exarg_vprintf_len **/

#if defined (NOPOLL_OS_WIN32) && ! defined (__GNUC__)
#   if HAVE_VSCPRINTF
	if (format == NULL)
		return 0;
	return _vscprintf (format, args) + 1;
#   else
	char buffer[8192];
	if (format == NULL)
		return 0;
	return _vsnprintf (buffer, 8191, format, args) + 1;
#   endif
#else
	/* gnu gcc case */
	if (format == NULL)
		return 0;
	return vsnprintf (NULL, 0, format, args) + 1;
#endif
}

/** 
 * @brief DEPRECATED: Allows to produce an string representing the
 * message hold by chunk with the parameters provided.
 * 
 * @param chunk The message chunk to print.
 * @param args The arguments for the chunk.
 * 
 * @return A newly allocated string.
 *
 * IMPLEMENTATION NOTE: This function may have a fundamental bug due
 * to the design of va_list arguments under amd64 platforms. In short,
 * a function receiving a va_list argument can't use it twice. In you
 * are running amd64, check your nopoll_config.h did find
 * NOPOLL_HAVE_VASPRINTF.
 */
char  * nopoll_strdup_printfv    (const char * chunk, va_list args)
{
	/** IMPLEMENTATION NOTE: place update exarg_strdup_printfv
	 * code in the case this code is updated **/
#if defined(SHOW_DEBUG_LOG) && ! defined(NOPOLL_HAVE_VASPRINTF)
	noPollCtx * ctx = NULL;
#endif

#if !defined(NOPOLL_HAVE_VASPRINTF)
	int       size;
#endif
	char    * result   = NULL;

	if (chunk == NULL)
		return NULL;

#ifdef NOPOLL_HAVE_VASPRINTF
	/* do the operation using the GNU extension */
	if (vasprintf (&result, chunk, args) == -1)
	        return NULL;
#else
	/* get the amount of memory to be allocated */
	size = nopoll_vprintf_len (chunk, args);

	/* check result */
	if (size == -1) {
		nopoll_log (ctx, NOPOLL_LEVEL_CRITICAL, "unable to calculate the amount of memory for the strdup_printf operation");
		return NULL;
	} /* end if */

	/* allocate memory */
	result   = nopoll_new (char, size + 2);
	
	/* copy current size */
#    if defined(NOPOLL_OS_WIN32) && ! defined (__GNUC__)
	size = _vsnprintf_s (result, size + 1, size, chunk, args);
#    else
	size = vsnprintf (result, size + 1, chunk, args);
#    endif
#endif
	/* return the result */
	return result;
}

/** 
 * @internal Function used by \ref nopoll_trim.
 */
nopoll_bool        nopoll_is_white_space  (char * chunk)
{
	/* do not complain about receive a null refernce chunk */
	if (chunk == NULL)
		return nopoll_false;
	
	if (chunk[0] == ' ')
		return nopoll_true;
	if (chunk[0] == '\n')
		return nopoll_true;
	if (chunk[0] == '\t')
		return nopoll_true;
	if (chunk[0] == '\r')
		return nopoll_true;

	/* no white space was found */
	return nopoll_false;
}

/** 
 * @brief Removes white spaces and new lines characters from the
 * string providing the count of bytes trimmed from the string.
 *
 * @param chunk The chunk to trim.
 *
 * @param trimmed An optional reference that returns the count of bytes
 * trimmed by the operation.
 */
void        nopoll_trim  (char * chunk, int * trimmed)
{
	int    iterator;
	int    iterator2;
	int    end;
	int    total;

	/* perform some environment check */
	if (chunk == NULL)
		return;

	/* check empty string received */
	if (strlen (chunk) == 0) {
		if (trimmed)
			*trimmed = 0;
		return;
	}

	/* check the amount of white spaces to remove from the
	 * begin */
	iterator = 0;
	while (chunk[iterator] != 0) {
		
		/* check that the iterator is not pointing to a white
		 * space */
		if (! nopoll_is_white_space (chunk + iterator))
			break;
		
		/* update the iterator */
		iterator++;
	}

	/* check for the really basic case where an empty string is found */
	if (iterator == strlen (chunk)) {
		/* an empty string, trim it all */
		chunk [0] = 0;
		if (trimmed)
			*trimmed = iterator;
		return;
	} /* end if */

	/* now get the position for the last valid character in the
	 * chunk */
	total   = strlen (chunk) -1;
	end     = total;
	while (chunk[end] != 0) {
		
		/* stop if a white space is found */
		if (! nopoll_is_white_space (chunk + end)) {
			break;
		}

		/* update the iterator to eat the next white space */
		end--;
	}

	/* the number of items trimmed */
	total -= end;
	total += iterator;
	
	/* copy the exact amount of non white spaces items */
	iterator2 = 0;
	while (iterator2 < (end - iterator + 1)) {
		/* copy the content */
		chunk [iterator2] = chunk [iterator + iterator2];

		/* update the iterator */
		iterator2++;
	}
	chunk [ end - iterator + 1] = 0;

	if (trimmed != NULL)
		*trimmed = total;

	/* return the result reference */
	return;	
}

/** 
 * @brief Portable subsecond sleep. Suspends the calling thread during
 * the provided amount of time.
 *
 * @param microseconds The amount of time to wait.
 */
void        nopoll_sleep (long microseconds)
{
#if defined(NOPOLL_OS_UNIX)
	usleep (microseconds);
	return;
#elif defined(NOPOLL_OS_WIN32)
	Sleep (microseconds / 1000);
	return;
#endif
}

noPollMutexCreate   __nopoll_mutex_create  = NULL;
noPollMutexDestroy  __nopoll_mutex_destroy = NULL;
noPollMutexLock     __nopoll_mutex_lock    = NULL;
noPollMutexUnlock   __nopoll_mutex_unlock  = NULL;

/** 
 * @brief Creates a mutex with the defined create mutex handler.
 *
 * See \ref nopoll_thread_handlers for more information.
 *
 * @return A mutex reference or NULL if it fails.
 */
noPollPtr   nopoll_mutex_create (void)
{
	if (! __nopoll_mutex_create)
		return NULL;

	/* call defined handler */
	return __nopoll_mutex_create ();
}

/** 
 * @brief Implements a mutex lock operation on the provided
 * reference. The function just skip when no mutex reference is
 * received o no lock handler was defined.
 *
 * See \ref nopoll_thread_handlers for more information.
 *
 * @param mutex The mutex where do the lock operation. 
 *
 * The function will just return if the reference isn't defined or the
 * lock handler wasn't installed.
 */
void        nopoll_mutex_lock    (noPollPtr mutex)
{
	if (! __nopoll_mutex_lock)
		return;

	/* call defined handler */
	__nopoll_mutex_lock (mutex);
	return;
}

/** 
 * @brief Implements a mutex unlock operation on the provided
 * reference. The function just skip when no mutex reference is
 * received o no unlock handler was defined.
 *
 * See \ref nopoll_thread_handlers for more information.
 *
 * @param mutex The mutex where do the unlock operation. 
 *
 */
void        nopoll_mutex_unlock  (noPollPtr mutex)
{
	if (! __nopoll_mutex_unlock)
		return;

	/* call defined handler */
	__nopoll_mutex_unlock (mutex);
	return;
}

/** 
 * @brief Implements a mutex destroy operation on the provided
 * reference. The function just skip when no mutex reference is
 * received o no destroy handler was defined.
 *
 * See \ref nopoll_thread_handlers for more information.
 *
 * @param mutex The mutex to destroy operation. 
 *
 */
void        nopoll_mutex_destroy (noPollPtr mutex)
{
	if (! __nopoll_mutex_destroy)
		return;

	/* call defined handler */
	__nopoll_mutex_destroy (mutex);
	return;
}

/** 
 * @brief Global optional mutex handlers used by noPoll library to
 * create, destroy, lock and unlock mutex.
 *
 * If you set this handlers, the library will use these functions to
 * secure sensitive code paths that mustn't be protected while working
 * with threads.
 *
 * If you don't provide these, the library will work as usual without
 * doing any locking.
 *
 * @param mutex_create The handler used to create mutexes.
 *
 * @param mutex_destroy The handler used to destroy mutexes.
 *
 * @param mutex_lock The handler used to lock a particular mutex.
 *
 * @param mutex_unlock The handler used to unlock a particular mutex.
 *
 * The function must receive all handlers defined, otherwise no
 * configuration will be done.
 */
void        nopoll_thread_handlers (noPollMutexCreate  mutex_create,
				    noPollMutexDestroy mutex_destroy,
				    noPollMutexLock    mutex_lock,
				    noPollMutexUnlock  mutex_unlock)
{
	/* check handlers before configuring anything */
	if (! mutex_create ||
	    ! mutex_destroy ||
	    ! mutex_lock ||
	    ! mutex_unlock)
		return;

	/* configured received handlers */
	__nopoll_mutex_create  = mutex_create;
	__nopoll_mutex_destroy = mutex_destroy;
	__nopoll_mutex_lock    = mutex_lock;
	__nopoll_mutex_unlock  = mutex_unlock;

	return;
}

/** 
 * @brief Allows to encode the provided content, leaving the output on
 * the buffer allocated by the caller.
 *
 * @param content The content to be encoded.
 *
 * @param length Content byte-length to encode.
 *
 * @param output Reference to the already allocated buffer where to
 * place the output.
 *
 * @param output_size The buffer size.
 *
 * @return nopoll_true if the conversion was properly done, otherwise
 * nopoll_false is returned. The function also returns nopoll_false in
 * the case some parameter is not defined.
 */
nopoll_bool nopoll_base64_encode (const char  * content, 
				  int           length, 
				  char        * output, 
				  int         * output_size)
{
	BIO     * b64;
	BIO     * bmem;
	BUF_MEM * bptr;

	if (content == NULL || output == NULL || length <= 0 || output_size == NULL)
		return nopoll_false;
	
	/* create bio */
	b64  = BIO_new (BIO_f_base64());
	bmem = BIO_new (BIO_s_mem());
	
	/* push */
	b64  = BIO_push(b64, bmem);
	
	if (BIO_write (b64, content, length) != length) {
		BIO_free_all (b64);
		return nopoll_false;
	}

	if (BIO_flush (b64) != 1) {
		BIO_free_all (b64);
		return nopoll_false;
	}

	/* now get content */
	BIO_get_mem_ptr (b64, &bptr);
	
	/* check output size */
	if ((*output_size) < bptr->length) {
		BIO_free_all (b64);

		*output_size = bptr->length;
		return nopoll_false;
	}

	memcpy(output, bptr->data, bptr->length - 1);
	output[bptr->length-1] = 0;

	BIO_free_all (b64);

	return nopoll_true;
}

/** 
 * @brief Decodes the provided base64 content into the user provided
 * buffer.
 *
 * @param content The content to be decoded.
 *
 * @param length Content byte-length to encode.
 *
 * @param output Reference to the already allocated buffer where to
 * place the output.
 *
 * @param output_size The buffer size.
 *
 * @return nopoll_true if the conversion was properly done, otherwise
 * nopoll_false is returned. The function also returns nopoll_false in
 * the case some parameter is not defined.
 */ 
nopoll_bool nopoll_base64_decode (const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size)
{
	BIO     * b64;
	BIO     * bmem;

	if (content == NULL || output == NULL || length <= 0 || output_size == NULL)
		return nopoll_false;

	/* create bio */
	bmem = BIO_new_mem_buf ((void *) content, length);
	b64  = BIO_new (BIO_f_base64());
	BIO_set_flags (b64, BIO_FLAGS_BASE64_NO_NL);
	
	/* push */
	bmem  = BIO_push(b64, bmem);
	
	*output_size = BIO_read (bmem, output, *output_size);
	output[*output_size] = 0;

	BIO_free_all (bmem);

	return nopoll_true;
}

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


/* internal reference to track if we have to randomly init seed */
nopoll_bool __nopoll_nonce_init = nopoll_false;

/** 
 * @brief Fills the buffer provided with a random nonce of the
 * requested size. The function try to read random bytes from the
 * local PRNG to complete the bytes requested on the buffer..
 *
 * @param buffer The buffer where the output is left
 *
 * @param nonce_size The size of the requested nonce to written into the caller buffer.
 *
 * @return nopoll_true if the nonce was created otherwise nopoll_false
 * is returned.
 */
nopoll_bool nopoll_nonce (char * buffer, int nonce_size)
{
	long int       random_value;
	int            iterator;
	struct timeval tv;

	if (buffer == NULL || nonce_size <= 0)
		return nopoll_false;
	if (! __nopoll_nonce_init) {
#if defined(NOPOLL_OS_WIN32)
		nopoll_win32_gettimeofday (&tv, NULL);
#else
		gettimeofday (&tv, NULL);
#endif

		srand (time(0) * tv.tv_usec);
		__nopoll_nonce_init = nopoll_true;
	} /* end if */

	/* now get the value from random */
	iterator = 0;
	while (iterator < nonce_size) {
		/* gen random value */
#if defined(NOPOLL_OS_WIN32)
		random_value = rand ();
#else
		random_value = random ();
#endif

		/* copy into the buffer */
		memcpy (buffer + iterator, &random_value, sizeof (random_value));
		iterator += sizeof (random_value);
	} /* end while */

	return nopoll_true;
}

/** 
 * @internal Allows to extract a particular bit from a byte given the
 * position.
 *
 *    +------------------------+
 *    | 7  6  5  4  3  2  1  0 | position
 *    +------------------------+
 */
int nopoll_get_bit (char byte, int position) {
	return ( ( byte & (1 << position) ) >> position);
}

/** 
 * @internal Allows to set a particular bit on the first position of
 * the buffer provided.
 *
 *    +------------------------+
 *    | 7  6  5  4  3  2  1  0 | position
 *    +------------------------+
 */
void nopoll_set_bit (char * buffer, int position) {
	buffer[0] |= (1 << position);
	return;
}

void nopoll_show_byte (noPollCtx * ctx, char byte, const char * label) {
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "  byte (%s) = %d %d %d %d  %d %d %d %d",
		    label,
		    nopoll_get_bit (byte, 7),
		    nopoll_get_bit (byte, 6),
		    nopoll_get_bit (byte, 5),
		    nopoll_get_bit (byte, 4),
		    nopoll_get_bit (byte, 3),
		    nopoll_get_bit (byte, 2),
		    nopoll_get_bit (byte, 1),
		    nopoll_get_bit (byte, 0));
	return;
}

char * nopoll_int2bin (int a, char *buffer, int buf_size) {
	int i;

	buffer += (buf_size - 1);
	
	for (i = 31; i >= 0; i--) {
		*buffer-- = (a & 1) + '0';
		
		a >>= 1;
	}
	
	return buffer;
}

#define BUF_SIZE 33

void nopoll_int2bin_print (noPollCtx * ctx, int value) {
	
	char buffer[BUF_SIZE];
	buffer[BUF_SIZE - 1] = '\0';

	nopoll_int2bin (value, buffer, BUF_SIZE - 1);
	
	nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "%d = %s", value, buffer);

	return;
}

/** 
 * @internal Allows to get the 16 bit integer located at the buffer
 * pointer.
 *
 * @param buffer The buffer pointer to extract the 16bit integer from.
 *
 * @return The 16 bit integer value found at the buffer pointer.
 */
int    nopoll_get_16bit (const char * buffer)
{
	int high_part = buffer[0] << 8;
	int low_part  = buffer[1] & 0x000000ff;

	return (high_part | low_part) & 0x000000ffff;
}

/** 
 * @internal Allows to get the 8bit integer located at the buffer
 * pointer.
 *
 * @param buffer The buffer pointer to extract the 8bit integer from.
 *
 * @erturn The 8 bit integer value found at the buffer pointer.
 */
int    nopoll_get_8bit  (const char * buffer)
{
	return buffer[0] & 0x00000000ff;
}

/** 
 * @internal Allows to set the 16 bit integer value into the 2 first
 * bytes of the provided buffer.
 *
 * @param value The value to be configured in the buffer.
 *
 * @param buffer The buffer where the content will be placed.
 */
void   nopoll_set_16bit (int value, char * buffer)
{
	buffer[0] = (value & 0x0000ff00) >> 8;
	buffer[1] = value & 0x000000ff;
	
	return;
}

/** 
 * @internal Allows to set the 32 bit integer value into the 4 first
 * bytes of the provided buffer.
 *
 * @param value The value to be configured in the buffer.
 *
 * @param buffer The buffer where the content will be placed.
 */
void   nopoll_set_32bit (int value, char * buffer)
{
	buffer[0] = (value & 0x00ff000000) >> 24;
	buffer[1] = (value & 0x0000ff0000) >> 16;
	buffer[2] = (value & 0x000000ff00) >> 8;
	buffer[3] =  value & 0x00000000ff;

	return;
}

/** 
 * @brief Allows to get a 32bits integer value from the buffer.
 *
 * @param buffer The buffer where the integer will be retreived from.
 *
 * @return The integer value reported by the buffer.
 */
int    nopoll_get_32bit (const char * buffer)
{
	int part1 = (int)(buffer[0] & 0x0ff) << 24;
	int part2 = (int)(buffer[1] & 0x0ff) << 16;
	int part3 = (int)(buffer[2] & 0x0ff) << 8;
	int part4 = (int)(buffer[3] & 0x0ff);

	return part1 | part2 | part3 | part4;
}

extern nopoll_bool __nopoll_tls_was_init;

/** 
 * @brief Optional function that can be called at the very end of the
 * noPoll usage to ensure all memory allocated by the library is
 * released so debugging operations are easier.
 *
 * It is not required to call this function to get proper operation by
 * the library. It is provided, especially due to TLS, to release all
 * internal memory that may be allocated.
 */
void nopoll_cleanup_library (void)
{
	
	if (__nopoll_tls_was_init) {
		EVP_cleanup ();
		CRYPTO_cleanup_all_ex_data ();
		ERR_free_strings ();

		/* notify the library isn't initialized */
		__nopoll_tls_was_init = nopoll_false;
	} /* end if */
	
	return;
} /* end if */

/* @} */

/** 
 * \mainpage 
 *
 * \section intro noPoll: a toolkit to add WebSocket support to your project
 *
 * <b>noPoll</b> is a clean implemetnation of the <b>RFC 6455 : The Websocket protocol</b> definition, written in <b>ANSI C</b>.
 *
 * Some of its features are:
 *
 * - Context based API design making the library stateless. Support to run several execution contexts in the same process.
 * - Support for stream based API and message based API (handler notified)
 * - Robust and well tested implementation checked by a strong regression test to ensure the library keeps on working as new features are added.
 * - Flexible design that allows its integration into a external loop or to use its own waiting loop
 * - Support for port share which allows running native protocol and WebSocket protocol on the same port.
 *
 * noPoll is been developed by <b>Advanced Software Production Line,
 * S.L.</b> (http://www.aspl.es). It is licensed under the LGPL 2.1
 * which allows open source and commercial usage.
 *
 * noPoll manual is available in the following link: 
 *
 * - \ref nopoll_core_library_manual
 *
 * Check out <b>noPoll API documentation</b> in the following links:
 *
 * 1) Common API definition:
 *
 * - \ref nopoll_support
 * - \ref nopoll_decl_module
 * - \ref nopoll_ctx
 * - \ref nopoll_handlers
 * - \ref nopoll_log
 * - \ref nopoll_loop
 *
 * 2) Function for creating connections and listeners
 *
 * - \ref nopoll_conn
 * - \ref nopoll_listener
 *
 * 3) Functions to handle messages (message based API):
 *
 * - \ref nopoll_msg
 *
 * \section contact_aspl Contact Us
 * 
 * You can reach us at the <b>noPoll mailing list:</b> at <a href="http://lists.aspl.es/cgi-bin/mailman/listinfo/nopoll">noPoll users</a>
 * for any question you may find. 
 *
 * If you are interested on getting commercial support, you can also
 * contact us at: info@aspl.es.
 */

/** 
 * \page nopoll_core_library_manual noPoll core library manual
 *
 * \section index Index
 *
 * <b>Section 1: Basic concepts to starting writing with or integrating noPoll:</b>
 * 
 * - \ref installing_nopoll
 * - \ref using_nopoll
 * - \ref thread_safety
 * - \ref creating_a_nopoll_ctx
 * - \ref creating_basic_web_socket_server
 * - \ref creating_basic_web_socket_client
 * - \ref nopoll_manual_reading_content_from_connection
 * - \ref nopoll_manual_handling_fragments_with_websocket
 *
 * <b>Section 2: Advanced concepts to consider: </b>
 *
 * - \ref nopoll_manual_retrying_write_operations
 * - \ref nopoll_implementing_port_sharing 
 *
 * \section installing_nopoll 1.1 How to install noPoll 
 *
 * Currently, noPoll has only one dependency, which is OpenSSL
 * (libssl) for all those crypto operations required by the protocol
 * itself. 
 *
 * After having that library installed in your system (check your OS
 * documentation), download lastest tar.gz noPoll library from: http://code.google.com/p/no-poll
 *
 * Then, to compile the library follow the standard autoconf voodoo:
 *
 * \code
 * >> tar xzvf nopoll-{version}.tar.gz
 * >> cd nopoll-{version}
 * >> ./configure 
 * >> make
 * \endcode
 *
 * At this point, if everything went ok, you can check the library by running in one terminal:
 *
 * \code
 * >> cd test/
 * >> ./nopoll-regression-listener
 * \endcode
 *
 * And then in another terminal the client regression test:
 *
 * \code
 * >> cd test/
 * >> ./nopoll-regression-client
 * \endcode
 *
 * If everything looks fine, you can install nopoll into your system with the standard:
 * \code
 * >> make install
 * \endcode
 *
 * \section using_nopoll 1.2. How to use noPoll library into your application
 *
 * After a successful noPoll installation, you should be able to
 * include noPoll API functions by including the following header:
 *
 * \code
 * #include <nopoll.h>
 * \endcode
 *
 * Then you can use the following to get nopoll compilation flags:
 *
 * \code
 * >> pkg-config nopoll --cflags
 * >> pkg-config nopoll --libs
 * \endcode
 *
 * Or if your project uses autoconf for the building process, just include the following into your configure.ac file:
 *
 * \code
 * dnl check for websocket support (through noPoll)
 * AC_ARG_ENABLE(websocket-support, [  --disable-websocket-support  Makes the built with WebSocket extension library], 
 *	      enable_websocket_support="$enableval", 
 *	      enable_websocket_support=yes)
 * if test "$enable_websocket_support" != "no" ; then
 *    PKG_CHECK_MODULES(NOPOLL, nopoll,	[enable_websocket_support=yes], [enable_websocket_support=no])
 *    AC_SUBST(NOPOLL_CFLAGS)
 *    AC_SUBST(NOPOLL_LIBS)
 * fi
 * AM_CONDITIONAL(ENABLE_WEBSOCKET_SUPPORT, test "x$enable_websocket_support" = "xyes")
 * \endcode
 *
 * \section thread_safety 1.3. noPoll thread safety considerations
 *
 * noPoll is designed as a thread agnostic stateless library so it can
 * fit in any project configuration (no matter if it uses threads or
 * event notification).
 *
 * In the case you are planning to use noPoll in a project that uses
 * threads and you expect to make calls to the noPoll API from
 * different threads at the same time, then you must setup four
 * callbacks that will help noPoll to create, destroy, lock and unlock
 * mutexes. For that, check documentation about \ref
 * nopoll_thread_handlers
 *
 * \section creating_a_nopoll_ctx 1.4. Creating a noPoll context
 *
 * Before working with noPoll API you must create a
 * \ref noPollCtx object, which represents a single library instance
 * state. You can create as much \ref noPollCtx objects inside the process as you
 * want. To create it you must do something like:
 *
 * \code
 * noPollCtx * ctx = nopoll_ctx_new ();
 * if (! ctx) {
 *     // error some handling code here
 * }
 *
 * // do some WebSocket operations (as client or listener)...
 *
 * // ...and once you are done and after terminating all messages and
 * // connections created you have to release the context by doing the
 * // following:
 * nopoll_ctx_unref (ctx);
 * \endcode
 *
 * 
 *
 *
 * \section creating_basic_web_socket_server 1.5. Creating a basic WebSocket server with noPoll (using noPoll own loop)
 *
 * \note Remember you can see many of the code already supported by noPoll by checking the nopoll regression listener at: https://dolphin.aspl.es/svn/publico/nopoll/trunk/test/nopoll-regression-listener.c
 *
 * Now let's see how to create a simple WebSocket server using noPoll own loop:
 * \code
 * // create a listener to receive connections on port 1234
 * noPollConn * listener = nopoll_listener_new (ctx, "0.0.0.0", "1234");
 * if (! nopoll_conn_is_ok (listener)) {
 *      // some error handling here
 * }
 * 
 * // now set a handler that will be called when a message (fragment or not) is received
 * nopoll_ctx_set_on_msg (ctx, listener_on_message, NULL);
 *
 * // now call to wait for the loop to notify events 
 * nopoll_loop_wait (ctx, 0);
 * \endcode
 *
 * Now, every time a frame is received, the handler
 * <b>listener_on_message</b> will be called. Here is an example about
 * that handler:
 *
 * \code
 * void listener_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr * user_data) {
 *         // print the message (for debugging purposes) and reply
 *         printf ("Listener received (size: %d, ctx refs: %d): (first %d bytes, fragment: %d) '%s'\n", 
 *                 nopoll_msg_get_payload_size (msg),
 *                 nopoll_ctx_ref_count (ctx), shown, nopoll_msg_is_fragment (msg), example);
 *     
 *         // reply to the message
 *         nopoll_conn_send_text (conn, "Message received", 16);
 *   
 *         return;
 * }
 * \endcode
 * 
 * \section creating_basic_web_socket_client 1.6. Creating a basic WebSocket client with noPoll
 *
 * \note Remember you can see many of the code already supported by noPoll by checking the nopoll regression client at: https://dolphin.aspl.es/svn/publico/nopoll/trunk/test/nopoll-regression-client.c
 *
 * The process of creating a WebSocket connection is really
 * simple. After creating a context (\ref noPollCtx) you connect to
 * the listener by using:
 *
 * \code
 * // call to create a connection 
 * noPollConn * conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
 * if (! nopoll_conn_is_ok (conn)) {
 *     // some error handling here
 * }
 *
 * \endcode
 *
 * After that, you can call to \ref nopoll_conn_is_ready to check if
 * the connection is ready to send and receive content. If you don't
 * have a loop to wait on, you can use the following function to wait
 * for the connection to be ready:
 *
 * \code
 * if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
 *         // some error handling
 * }
 * \endcode
 *
 * In any case, once the connection is ready, either because \ref
 * nopoll_conn_is_ready returned \ref nopoll_true or because you used
 * \ref nopoll_conn_wait_until_connection_ready, you can send a
 * message by using the following:
 *
 * \code
 * // send a message 
 * if (nopoll_conn_send_text (conn, "Hello there! this is a test", 27) != 27) {
 *         // send a message
 * }
 * \endcode
 *
 * Now, to receive the content from this connection see the following.
 * 
 * \section nopoll_manual_reading_content_from_connection 1.7. How to handle read I/O operations on noPollConn objects (or how it integrates with existing I/O loops).
 *
 * Now, to receive content from connections (or to handle master listeners requests) you can use the following methods:
 *
 * - Use noPoll own I/O loop wait (\ref nopoll_loop_wait) and set a onMessage received handler (\ref nopoll_ctx_set_on_msg and \ref nopoll_conn_set_on_msg)
 *
 * - Use your already working I/O loop to wait for changes on the
 *   socket associated to the noPollConn object (see \ref
 *   nopoll_conn_socket). In the case of detecting changes on a master listener connection use to \ref nopoll_conn_accept to accept new incoming connections. In the case of detecting changes in connections with I/O, call to \ref nopoll_conn_get_msg to receive entire messages or \ref nopoll_conn_read (if you want to use streaming API).
 *
 * \section nopoll_manual_handling_fragments_with_websocket 1.8 Will noPoll automatically integrate fragments into a single  message?.
 *  
 * Ok, in general it does, however we would have to look into the
 * particular case to give a correct answer. That's because the term
 * "fragment" is *really* broad especially for WebSocket.
 * 
 * In general, <b>you shouldn't design in a way that this is important
 * for your application to work</b>. All about WebSocket must be designed
 * as if you were working with a stream oriented socket.
 *
 * For example, WebSocket standard states that any intermediary may
 * split or join frames. And by any intermediary we mean any WebSocket
 * stack in the middle of the transmission including the sender and
 * the receiver.
 *
 * The practical implication is that if your WebSocket peer sends a
 * message, it may reach to the other peer consolidated along with
 * other smaller messages or it may be received as several "complete"
 * WebSocket frames.
 *
 * In general noPoll tries to consolidate internal frame fragments but
 * only for some particular functions like (\ref
 * nopoll_conn_get_msg). For example, if a frame fragment is received,
 * that is, just the header and part of the body, this particular
 * function will "wait" until the entire body is received. And by
 * "wait" we mean the function will return NULL, waiting you to call
 * again in the future to get the entire message.
 *
 * In general, even though the standard allows it (as we saw), noPoll
 * doesn't do any split or join operation to avoid confusion. However,
 * some API calls like \ref nopoll_conn_read, which provides an "stream
 * view" of the connection, will serve bytes as they come (so the
 * frame concept is not present in this case).
 *
 * \section nopoll_manual_retrying_write_operations 2.1. Retrying failed write operations 
 *
 * Every time you do a write operation (using for example \ref
 * nopoll_conn_send_text or \ref nopoll_conn_send_text_fragment) there
 * is a possibility that the write operation <b>failes because the socket
 * isn't capable to keep on accepting more data</b>.
 *
 * In that case, errno == 11 (or \ref NOPOLL_EWOULDBLOCK) is returned
 * so you can check this to later retry the write operation.
 *
 * Because websocket involves sending headers that already includes
 * the size of the message sent, <b>you can't just retry by calling again
 * to the send operation used</b> (like \ref nopoll_conn_send_text),
 * especially because you must "continue" the send operation where it
 * was left instead of sending more content with additional
 * headers. <b>In short, you must complete the operation.</b>
 *
 * To this end, you must use the following functions to check and
 * complete pending write operations:
 *
 * - \ref nopoll_conn_pending_write_bytes
 * - \ref nopoll_conn_complete_pending_write
 * 
 * Here is a posible complete function considering all points:
 *
 * \code
 * int websocket_write (noPollConn * conn, const char * content, int length) {
 *           // FIRST PART: normal send operation
 *           int tries = 0;
 *           int bytes_written;
 * 
 *           // do write operation and check
 *           bytes_written = nopoll_conn_send_text (conn, content, length);
 *           if (bytes_written == length) {
 *                  // operation completed, just return bytes written
 *                  return bytes_written;
 *           } 
 *
 *           // SECOND PART: retry in the case of failure
 *           // some failure found, check errno
 *           while (tries < 5 && errno == NOPOLL_EWOULDBLOCK && nopoll_conn_pending_write_bytes (conn) > 0) {
 *                  // ok, unable to write all data but that data is waiting to be flushed
 *                  // you can return here and then make your application to retry again or
 *                  // try it right now, but with a little pause before continue
 *                  nopoll_sleep (10000); // lets wait 10ns
 *
 *                  // flush and check if write operation completed
 *                  if (nopoll_conn_complete_pending_write (conn) == 0)
 *                          return length;
 *                   
 *                  // limit loop
 *                  tries++;
 *           }
 *         
 *           // failure, return error code reported by the first call or the last retry
 *           return  bytes_written;
 * }
 * \endcode
 *
 * As we can see, the example tries to first write the content and
 * then check for errors, trying to complete write in the case of
 * errno == NOPOLL_EWOULDBLOCK, but, before going ahead retrying, the
 * function sleeps a bit.
 *
 * <b>A very important note to consider is that this</b> isn't by far
 * the best way to do this. This example is just to demonstrate the
 * concept. The "ideal" implementation would be not to do any retry
 * here (second part) but let the engine looping and waiting for this
 * WebSocket to retry later, letting the overall application to keep
 * on doing other things meanwhile (like writing or handling I/O in other
 * connections) rather than locking the caller (as the example do).
 *
 * Knowing this, if you want a ready to use function that implements
 * concept (for the second part), you can directly use:
 *
 * - \ref nopoll_conn_flush_writes
 *
 * With it, a fairly complete and efficient write operation would be:
 *
 * \code
 * // do write operation 
 * bytes_written = nopoll_conn_send_text (conn, content, length);
 *
 * // complete pending write by flushing and limitting operation for 2 seconds
 * // pass to the function bytes_written as returned by nopoll_conn_send_text
 * bytes_written = nopoll_conn_flush_writes (conn, 2000000, bytes_written);
 *
 * \endcode
 * 
 * \section nopoll_implementing_port_sharing  2.2. Implementing protocol port sharing: running WebSocket and legacy protocol on the same port
 *
 * Current noPoll design allows to implement full WebSocket connection
 * accept by calling to \ref nopoll_conn_accept but also it is
 * possible to let other application to do the accept connection, try
 * to guess if what is comming is a WebSocket connection, to then let
 * noPoll to complete the accept socket operation.
 *
 * As a example, these concepts are being implemented by Vortex
 * Library (http://www.aspl.es/vortex) to allow running on the same
 * port BEEP and BEEP over WebSocket.
 *
 * The way each application implements "port sharing concept" is
 * really especific to each case, but here are the general steps:
 *
 * - 1. Let the legacy application to accept the socket by the standard call accept ()
 *
 * - 2. Then, call to recv (socket, buffer[3], 3, MSG_PEEK); to get just 3 bytes from the socket without removing them from the queue.
 *
 * - 3. Then check with something like the following to know if the incoming connection seems to be a WebSocket one or not:
 *
 *  \code
 *  // detect tls conn 
 *  nopoll_bool is_tls_conn = bytes[0] == 22 && bytes[1] == 3 && bytes[2] == 1;
 *
 *  // detect then both values (TLS WebSocket and just WebScoket)
 *  if (! axl_memcmp ("GET", bytes, 3) && ! is_tls_conn)
 *          return nopoll_false; // nothing detected here (it doesn't seems
 *			         // to be a websocket connection) continue as normal
 *  
 *  // nice, it seems we've found an incoming WebSocket connection
 *  \endcode
 *
 * - 4. In the case nothing was detected, stop, and continue with the
 *     usual accept process that was already in place. In the case you
 *     detected a posible WebSocket connection, then, you'll have to
 *     do something like:
 *
 * \code
 * // Create a noPollConn listener object that presents the legacy listener where the connection was
 * // received. In general is recommended to reuse these references to avoid creating over and over
 * // again new references as new WebSocket connections are received.
 * //
 * noPollConn * nopoll_listener = nopoll_listener_from_socket (nopoll_ctx, listener_socket);
 *
 * // Create an accepted listener connection reusing the socket received
 * // where nopoll_ctx is a reference to a noPollCtx object created and reused by all
 * // connections accepted, and _socket represents the socket file descriptor
 * //
 * noPollConn * conn = nopoll_listener_from_socket (nopoll_ctx, _socket);
 * if (! nopoll_conn_is_ok (conn)) {
 *       // ok, something failed, release and return
 *       nopoll_conn_close (conn);
 *       return;
 * }
 *
 * // Now, initiate the entire WebSocket accept process (including TLS one)
 * // where nopoll_ctx is the context we used before, nopoll_listener is a listener where
 * // the connection was received, conn is the connection accepted and is_tls_conn
 * // is an indication about what to expect about TLS process.
 * //
 * if (! nopoll_conn_accept_complete (nopoll_ctx, nopoll_listener, conn, _socket, is_tls_conn)) {
 *       // failed to accept connection, release connection
 *       nopoll_conn_close (conn);
 *       // optionally close listener reference if it is not reused
 *       nopoll_conn_close (nopoll_listener);
 *       return;
 * }
 * 
 * // now process incoming messages as configured (either because you've configured an onMessage handler)
 * // or because you are handling directly all incoming content (streaming API).
 * \endcode
 * 
 * 
 * 
 */



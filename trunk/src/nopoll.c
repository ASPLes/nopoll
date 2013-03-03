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

#ifndef NOPOLL_HAVE_VASPRINTF
	int       size;
#endif
	char    * result   = NULL;
	int       new_size = -1;

	if (chunk == NULL)
		return NULL;

#ifdef NOPOLL_HAVE_VASPRINTF
	/* do the operation using the GNU extension */
	new_size = vasprintf (&result, chunk, args);
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
	new_size = _vsnprintf_s (result, size + 1, size, chunk, args);
#    else
	new_size = vsnprintf (result, size + 1, chunk, args);
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
	Sleep (microseconds);
	return;
#endif
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
	int       bwritten;

	if (content == NULL || output == NULL || length <= 0 || output_size == NULL)
		return nopoll_false;
	
	/* create bio */
	b64  = BIO_new (BIO_f_base64());
	bmem = BIO_new (BIO_s_mem());
	
	/* push */
	b64  = BIO_push(b64, bmem);
	
	if (BIO_write (b64, content, length) != length) {
		printf ("Write values difers..%d\n", length);
		return nopoll_false;
	}
	bwritten = BIO_flush (b64);

	/* now get content */
	BIO_get_mem_ptr (b64, &bptr);
	
	/* check output size */
	if ((*output_size) < bptr->length) {
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
		gettimeofday (&tv, NULL);
		srand (time(0) * tv.tv_usec);
		__nopoll_nonce_init = nopoll_true;
	} /* end if */

	/* now get the value from random */
	iterator = 0;
	while (iterator < nonce_size) {
		/* gen random value */
		random_value = random ();

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


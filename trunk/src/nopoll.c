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

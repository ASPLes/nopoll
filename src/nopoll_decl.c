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
#include <nopoll_decl.h>

/** 
 * \addtogroup nopoll_decl_module
 * @{
 */

/**
 * @brief Calloc helper for nopoll library. The memory returned is
 * initialized to zero.
 *
 * @param count How many items to allocate.
 * @param size Size of one item.
 *
 * @return A newly allocated pointer that must be released with \ref
 * nopoll_free, or NULL if the allocation failed (no memory
 * available). The caller must check the value returned before using
 * it: this function never aborts the process on allocation failure.
 *
 * @see nopoll_free
 */
noPollPtr nopoll_calloc (size_t count, size_t size)
{
	return calloc (count, size);
}

/**
 * @brief Realloc helper for nopoll library.
 *
 * @param ref the reference to reallocate. It can be NULL, in that
 * case the call behaves like a plain allocation of <i>size</i> bytes.
 *
 * @param size Size of the new reference.
 *
 * @return The reference holding the resized memory, which may be
 * different from <i>ref</i>, or NULL if the reallocation failed. In
 * that failure case <b>the memory referenced by <i>ref</i> is still
 * valid and must still be released</b>, so the caller must not
 * overwrite its only pointer to it with the value returned (doing
 * <i>ref = nopoll_realloc (ref, size)</i> leaks the previous block on
 * error). Use a temporary variable and check it before assigning.
 *
 * @see nopoll_free
 */
noPollPtr nopoll_realloc (noPollPtr ref, size_t size)
{
	return realloc (ref, size);
}

/**
 * @brief Allows to deallocate memory referenced by <i>ref</i>.
 *
 * @param ref The reference to clear. It can be NULL, in that case the
 * function does nothing.
 */
void nopoll_free (noPollPtr ref)
{
	free (ref);
	return;
}


/** 
 * @}
 */


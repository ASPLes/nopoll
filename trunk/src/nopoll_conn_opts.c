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
#include <nopoll_conn_opts.h>
#include <nopoll_private.h>

/** 
 * \defgroup nopoll_conn_opts noPoll Connection Options: API to change default connection options.
 */

/** 
 * \addtogroup nopoll_conn_opts
 * @{
 */

/** 
 * @brief Create a new connection options object.
 *
 * @return A newly created connection options object. In general you don't have to worry about releasing this object because this is automatically done by functions using this object. However, if you call to \ref nopoll_conn_opts_set_reuse (opts, nopoll_true), then you'll have to use \ref nopoll_conn_opts_free to release the object after it is no longer used. The function may return NULL in case of memory allocation problems. Creating an object without setting anything will cause the library to provide same default behaviour as not providing it.
 */
noPollConnOpts * nopoll_conn_opts_new ()
{
	noPollConnOpts * result;

	/* create configuration object */
	result = nopoll_new (noPollConnOpts, 1);
	if (! result)
		return NULL;

	result->reuse        = nopoll_false; /* this is not needed, just to clearly state defaults */
	result->ssl_protocol = NOPOLL_METHOD_TLSV1;

	return result;
}

/** 
 * @brief Set ssl protocol method to be used on the API receiving this
 * configuration object.
 *
 * @param opts The connection options object. 
 *
 * @param ssl_protocol SSL protocol to use. See \ref noPollSslProtocol for more information.
 */
void nopoll_conn_opts_set_ssl_protocol (noPollConnOpts * opts, noPollSslProtocol ssl_protocol)
{
	if (opts == NULL)
		return;
	opts->ssl_protocol = ssl_protocol;
	return;
}

/** 
 * @brief Set reuse-flag be used on the API receiving this
 * configuration object. By setting nopoll_true will cause the API to
 * not release the object when finished. Instead, the caller will be
 * able to use this object in additional API calls but, after
 * finishing, a call to \ref nopoll_conn_opts_set_reuse function is
 * required.
 *
 * @param opts The connection options object. 
 *
 * @param reuse nopoll_true to reuse the object across calls,
 * otherwise nopoll_false to make the API function to release the
 * object when done.
 */
void nopoll_conn_opts_set_reuse        (noPollConnOpts * opts, nopoll_bool reuse)
{
	if (opts == NULL)
		return;
	opts->reuse = reuse;
	return;
}

/** 
 * @brief Allows to release a connection object reported by \ref nopoll_conn_opts_new
 *
 * IMPORTANT NOTE: do not use this function over a \ref noPollConnOpts if it is not flagged with \ref nopoll_conn_opts_set_reuse (opts, nopoll_true).
 *
 * Default behaviour provided by the API implies that every connection
 * options object created by \ref nopoll_conn_opts_new is
 * automatically released by the API consuming that object.
 */
void nopoll_conn_opts_free (noPollConnOpts * opts)
{
	if (opts == NULL)
		return;
	nopoll_free (opts);
	return;
} /* end if */

/** 
 * @internal API. Do not use it. It may change at any time without any
 * previous indication.
 */
void __nopoll_conn_opts_release_if_needed (noPollConnOpts * options)
{
	if (! options)
		return;
	if (options && options->reuse)
		return;
	nopoll_free (options);
	return;
}

/* @} */

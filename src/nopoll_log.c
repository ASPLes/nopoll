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
#include <nopoll.h>
#include <nopoll_private.h>

/** 
 * \defgroup nopoll_log noPoll Log: Console log reporting for noPoll library
 */

/** 
 * \addtogroup nopoll_log
 * @{
 */

/** 
 * @brief Allows to check if the log reporting inside the system is
 * enabled.
 *
 * @param ctx The context where the operation will take place.
 *
 * @return nopoll_true if the log is enabled or nopoll_false
 */
nopoll_bool      nopoll_log_is_enabled (noPollCtx * ctx) 
{
	if (ctx == NULL)
		return nopoll_false;

	/* return current value */
	return ctx->debug_enabled;
}

/** 
 *
 * @brief Allows to get current log configuration, to use colors.
 *
 * @param ctx The context where the operation will take place.
 *
 * @return nopoll_true if the color log is enabled or nopoll_false
 */
nopoll_bool    nopoll_log_color_is_enabled (noPollCtx * ctx)
{

	if (ctx == NULL)
		return nopoll_false;
	
	/* return current value */
	return ctx->debug_color_enabled;
}

/** 
 * @brief Allows to control how to activate the log reporting to the
 * console from the nopoll core library.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param value nopoll_true to enable log to console, otherwise
 * nopoll_false to disable it.
 */
void     nopoll_log_enable (noPollCtx * ctx, nopoll_bool value)
{
	if (ctx == NULL)
		return;

	/* activate debuging according to the variable */
	ctx->debug_enabled = value;
	return;
}

/** 
 * @brief Allows to control how to activate the color log reporting to
 * the console from the nopoll core library.
 *
 * @param ctx The context where the operation will take place.
 *
 * @param value nopoll_true to enable color log to console, otherwise
 * nopoll_false to disable it.
 */
void     nopoll_log_color_enable (noPollCtx * ctx, nopoll_bool value)
{
	if (ctx == NULL)
		return;

	/* activate color debuging according to the variable */
	ctx->debug_color_enabled = value;
	return;
}

/** 
 * @brief Allows to define a log handler that will receive all logs
 * produced under the provided context.
 *
 * @param ctx The context that is going to be configured.
 *
 * @param handler The handler to be called for each log to be
 * notified. Passing in NULL is allowed to remove any previously
 * configured handler. The handler is called no matter the value
 * configured by \ref nopoll_log_enable (which only governs the log
 * reported to the console) and no matter if the library was compiled
 * with or without --enable-nopoll-log.
 *
 * @param user_data User defined pointer to be passed in into the
 * handler configured along with the log notified.
 */
void            nopoll_log_set_handler (noPollCtx * ctx, noPollLogHandler handler, noPollPtr user_data)
{
	nopoll_return_if_fail (ctx, ctx);

	ctx->log_handler   = handler;
	ctx->log_user_data = user_data;

	return;
}

/** 
 * @internal Allows to drop a log to the console.
 *
 * This function allow to drop a log to the console using the given
 * domain, as an identification of which subsystem have reported the
 * information, and report level. This report level is used to notify
 * the consideration of the log reported.
 * 
 * The function allows to provide a printf like interface to report
 * messages. Here are some examples:
 * 
 * \code
 * // drop a log about current library initialization
 * nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "library properly initialized status=%d", status);
 * \endcode
 *
 *
 * @param ctx The context where the operation will take place. It is
 * allowed to receive a NULL reference (no log is then notified to any
 * handler and the console log is skipped).
 *
 * @param function_name The name of the function reporting the log.
 *
 * @param file The file where the log was reported.
 *
 * @param line The line, inside the file, where the log was reported.
 *
 * @param level The level that this message is classified.
 *
 * @param message The message to report. The message to report must be
 * not NULL.
 */
void __nopoll_log (noPollCtx * ctx, const char * function_name, const char * file, int line, noPollDebugLevel level, const char * message, ...)
{
	va_list      args;
	char       * log_msg;
	char       * log_msg2;

	/* not used for now: the function name is still received to
	 * allow reporting it in the future without changing the
	 * signature used by every nopoll_log () call site */
	(void) function_name;

	/* check if the application configured a log handler: in that
	 * case notify the log to it. This is done before the
	 * SHOW_DEBUG_LOG guard on purpose because __nopoll_log () is
	 * also called, unconditionally, by the
	 * nopoll_return_if_fail () macros: otherwise, a library
	 * compiled with --disable-nopoll-log would silently drop
	 * every critical condition reported by them */
	if (ctx && ctx->log_handler) {
		/* print the message */
		va_start (args, message);
		log_msg = nopoll_strdup_printfv (message, args);
		va_end (args);

		/* check the message was properly built before using
		 * it as a %s argument */
		if (log_msg == NULL)
			return;

		log_msg2 = log_msg;
		log_msg = nopoll_strdup_printf ("%s:%d %s ", file, line, log_msg);
		nopoll_free (log_msg2);

		if (log_msg == NULL)
			return;

		ctx->log_handler (ctx, level, log_msg, ctx->log_user_data);
		nopoll_free (log_msg);
		return;
	} /* end if */

#ifdef SHOW_DEBUG_LOG

	/* check if the log is enabled */
	if (! nopoll_log_is_enabled (ctx))
		return;

	/* printout the process pid */
	if (nopoll_log_color_is_enabled (ctx)) 
		printf ("\x1b[1;36m(proc %d)\x1b[0m: ", getpid ());
	else
		printf ("(proc %d): ", getpid ());

	/* drop a log according to the level */
	if (nopoll_log_color_is_enabled (ctx)) {
		switch (level) {
		case NOPOLL_LEVEL_DEBUG:
			printf ("(\x1b[1;32mdebug\x1b[0m) ");
			break;
		case NOPOLL_LEVEL_WARNING:
			printf ("(\x1b[1;33mwarning\x1b[0m) ");
			break;
		case NOPOLL_LEVEL_CRITICAL:
			printf ("(\x1b[1;31mcritical\x1b[0m) ");
			break;
		}
	} else {
		switch (level) {
		case NOPOLL_LEVEL_DEBUG:
			printf ("(debug) ");
			break;
		case NOPOLL_LEVEL_WARNING:
			printf ("(warning) ");
			break;
		case NOPOLL_LEVEL_CRITICAL:
			printf ("(critical) ");
			break;
		}
	}

	/* drop a log according to the domain */
	printf ("%s:%d ", file, line);

	/* print the message */
	va_start (args, message);
	vprintf (message, args);
	va_end (args);

	printf ("\n");

	/* ensure that the log is droped to the console */
	fflush (stdout);
#endif

	/* return */
	return;
}

/**
 * @}
 */


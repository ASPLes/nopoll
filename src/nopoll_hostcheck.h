/***************************************************************************
 *
 * Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#ifndef __NOPOLL_HOSTCHECK_H__
#define __NOPOLL_HOSTCHECK_H__

#define NOPOLL_HOST_NOMATCH 0
#define NOPOLL_HOST_MATCH   1

int nopoll_cert_hostcheck(const char *match_pattern, const char *hostname);

#endif /* __NOPOLL_HOSTCHECK_H__ */


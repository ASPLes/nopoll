/***************************************************************************
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

#ifndef __NOPOLL_INET_PTON_H__
#define __NOPOLL_INET_PTON_H__

#include <nopoll_hostname_validation.h>

#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

int nopoll_inet_pton(int, const char *, void *);

#endif /* __NOPOLL_INET_PTON_H__ */


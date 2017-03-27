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


#include <nopoll_hostname_validation.h>

/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
   its behavior is altered by the current locale. */
char nopoll_raw_toupper(char in)
{
  if(in >= 'a' && in <= 'z')
    return (char)('A' + in - 'a');
  return in;
}

int nopoll_strcasecompare(const char *first, const char *second)
{
  while(*first && *second) {
    if(nopoll_raw_toupper(*first) != nopoll_raw_toupper(*second))
      /* get out of the loop as soon as they don't match */
      break;
    first++;
    second++;
  }
  /* we do the comparison here (possibly again), just to make sure that if the
     loop above is skipped because one of the strings reached zero, we must not
     return this as a successful match */
  return (nopoll_raw_toupper(*first) == nopoll_raw_toupper(*second));
}


int nopoll_strncasecompare(const char *first, const char *second, size_t max)
{
  while(*first && *second && max) {
    if(nopoll_raw_toupper(*first) != nopoll_raw_toupper(*second)) {
      break;
    }
    max--;
    first++;
    second++;
  }
  if(0 == max)
    return 1; /* they are equal this far */

  return nopoll_raw_toupper(*first) == nopoll_raw_toupper(*second);
}


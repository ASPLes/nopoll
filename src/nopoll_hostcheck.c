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

static int hostmatch(char *hostname, char *pattern);

/*
 * Match a hostname against a wildcard pattern.
 * E.g.
 *  "foo.host.com" matches "*.host.com".
 *
 * We use the matching rule described in RFC6125, section 6.4.3.
 * https://tools.ietf.org/html/rfc6125#section-6.4.3
 *
 * In addition: ignore trailing dots in the host names and wildcards, so that
 * the names are used normalized. This is what the browsers do.
 *
 * Do not allow wildcard matching on IP numbers. There are apparently
 * certificates being used with an IP address in the CN field, thus making no
 * apparent distinction between a name and an IP. We need to detect the use of
 * an IP address and not wildcard match on such names.
 *
 * NOTE: hostmatch() gets called with copied buffers so that it can modify the
 * contents at will.
 */

static int hostmatch(char *hostname, char *pattern)
{
  const char *pattern_label_end, *pattern_wildcard, *hostname_label_end;
  int wildcard_enabled;
  size_t prefixlen, suffixlen;
  struct in_addr ignored;
  struct sockaddr_in6 si6;

  /* normalize pattern and hostname by stripping off trailing dots */
  size_t len = strlen(hostname);
  if(hostname[len-1]=='.')
    hostname[len-1]=0;
  len = strlen(pattern);
  if(pattern[len-1]=='.')
    pattern[len-1]=0;

  pattern_wildcard = strchr(pattern, '*');
  if(pattern_wildcard == NULL)
    return nopoll_strcasecompare(pattern, hostname) ?
      NOPOLL_HOST_MATCH : NOPOLL_HOST_NOMATCH;

  /* detect IP address as hostname and fail the match if so */
  if(nopoll_inet_pton(AF_INET, hostname, &ignored) > 0)
    return NOPOLL_HOST_NOMATCH;
  else if(nopoll_inet_pton(AF_INET6, hostname, &si6.sin6_addr) > 0)
    return NOPOLL_HOST_NOMATCH;

  /* We require at least 2 dots in pattern to avoid too wide wildcard
     match. */
  wildcard_enabled = 1;
  pattern_label_end = strchr(pattern, '.');
  if(pattern_label_end == NULL || strchr(pattern_label_end+1, '.') == NULL ||
     pattern_wildcard > pattern_label_end ||
     nopoll_strncasecompare(pattern, "xn--", 4)) {
    wildcard_enabled = 0;
  }
  if(!wildcard_enabled)
    return nopoll_strcasecompare(pattern, hostname) ?
      NOPOLL_HOST_MATCH : NOPOLL_HOST_NOMATCH;

  hostname_label_end = strchr(hostname, '.');
  if(hostname_label_end == NULL ||
     !nopoll_strcasecompare(pattern_label_end, hostname_label_end))
    return NOPOLL_HOST_NOMATCH;

  /* The wildcard must match at least one character, so the left-most
     label of the hostname is at least as large as the left-most label
     of the pattern. */
  if(hostname_label_end - hostname < pattern_label_end - pattern)
    return NOPOLL_HOST_NOMATCH;

  prefixlen = pattern_wildcard - pattern;
  suffixlen = pattern_label_end - (pattern_wildcard+1);
  return nopoll_strncasecompare(pattern, hostname, prefixlen) &&
    nopoll_strncasecompare(pattern_wildcard+1, hostname_label_end - suffixlen,
                    suffixlen) ?
    NOPOLL_HOST_MATCH : NOPOLL_HOST_NOMATCH;
}

int nopoll_cert_hostcheck(const char *match_pattern, const char *hostname)
{
  char *matchp;
  char *hostp;
  int res = 0;
  if(!match_pattern || !*match_pattern ||
      !hostname || !*hostname) 
    ;
  else {
    matchp = strdup(match_pattern);
    if(matchp) {
      hostp = strdup(hostname);
      if(hostp) {
        if(hostmatch(hostp, matchp) == NOPOLL_HOST_MATCH)
          res= 1;
        free(hostp);
      }
      free(matchp);
    }
  }

  return res;
}


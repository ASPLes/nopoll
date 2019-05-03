#ifndef __NOPOLL_HOSTCHECK_H__
#define __NOPOLL_HOSTCHECK_H__

typedef enum {
	MATCH_FAIL = 0,
	MATCH_SUCCESS,
} noPollHostMatchStatus;
noPollHostMatchStatus nopoll_match_hostname(const char *pattern,const char *hostname);

#endif /* __NOPOLL_HOSTCHECK_H__ */


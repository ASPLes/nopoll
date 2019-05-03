#ifndef __NOPOLL_HOSTNAME_COMPARE_H__
#define __NOPOLL_HOSTNAME_COMPARE_H__

int nopoll_hostname_compare_without_wildcard(char *pattern, char *hostname);
int nopoll_hostname_compare_with_wildcard(char *pattern, char *hostname);
int nopoll_hostname_validate_inet(int addr_fam, char *hostname);
char nopoll_to_lower(char ch);

#endif /* __NOPOLL_HOSTNAME_COMPARE_H__*/
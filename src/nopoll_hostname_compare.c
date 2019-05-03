#include <nopoll_hostname_validation.h>

static int nopoll_get_delimeter_count(char * string, char *delimeter);
static int  nopoll_validate_ipv4(char *hostname);
static int nopoll_validate_ipv6(char *hostname);

char nopoll_to_lower(char ch)
{
  if(ch >= 'A' && ch <= 'Z')
    return (char)('a' + ch - 'A');
  return ch;
}

static int nopoll_get_delimeter_count(char * string, char *delimeter) {
	int count = 0;
	while (*string != '\0') {
		if (*string == *delimeter) {
			count +=1;
		}
		string++;
	}
	return count;
}

int nopoll_hostname_compare_without_wildcard(char *pattern, char *hostname) {
  	while (*pattern != '\0' && *hostname != '\0') {
		if( nopoll_to_lower(*pattern) != nopoll_to_lower(*hostname) ){
			return 0;
		}
		pattern++;
		hostname++;
	}
	if (*pattern == '\0' && *hostname == '\0') {
		return 1;
	}
	return 0;
}

int nopoll_hostname_compare_with_wildcard(char *pattern, char *hostname) {
	int cmp_result = 0;
	int split_count = 0;
	char *p_split_first, *p_split_second, *h_split_first, *h_split_second;

	/* Get the string after two dots (.) for comparison (without wildcard)
	* as to long wildcard is not supported */
	p_split_first = strchr(pattern, '.');
	h_split_first = strchr(hostname,'.');

	if (p_split_first != NULL && h_split_first != NULL) {
		p_split_second = strchr(++p_split_first, '.');
		h_split_second = strchr(++h_split_first, '.');
	}
	else {
		return 0;
	}
	
	if (p_split_second && h_split_second) {
		cmp_result = nopoll_hostname_compare_without_wildcard(++p_split_second, ++h_split_second);
		if (!cmp_result){
			return 0;
		}
	}
	else {
		return 0;
	}
	/* Get string till two dots (.) for wild card comparison */
	int p_index = p_split_second - pattern;
	int h_index = h_split_second - hostname;

	*(pattern + (p_index)) = '\0';
	*(hostname + (h_index)) = '\0';

	while ((*pattern!= '\0') && (*hostname != '\0')) {
		/* Continue reading if both the charecters are same */
		if ( nopoll_to_lower(*pattern) == nopoll_to_lower(*hostname)) {
			pattern++;
			hostname++;
		}
		/* check if hostname contains wildcard */
		else if (*pattern == '*') {

			if(*(++pattern) == '.') {
				/* If hostname contains only * in first octet then skip all charecters in pattern
				* Then do comparison of next octet*/
				if (split_count == 0) {
 					h_index = h_split_first - hostname;
 					hostname += h_index;
 					++pattern;
 				}
 				else {
 					h_index = h_split_second - hostname;
 					hostname += h_index;
 					if (*hostname == '\0') {
 						return 1;
 					}
				}
			}
			else {
				/* Will support partial wildcard comparison */
				char *p_part = strchr(pattern, '.');
				p_index = p_part - pattern;

				/* Skip all charecters in pattern and compare the charecters remained after '*' in hostname string */
				while (*(hostname + p_index) != '.'){
					hostname++;
				}

				while (p_index) {
					if ( nopoll_to_lower(*hostname) != nopoll_to_lower(*pattern)) {
						return 0;
					}
					pattern++;
					hostname++;
					p_index -= 1;
				}
			}
			/* After first octet comparison make this on, so that wildcard comparison for next octet will get rejected */
			split_count = 1;
		}
		else if ( nopoll_to_lower(*pattern) != nopoll_to_lower(*hostname)) {
				return 0;
		}
	}
	/* Outside while, check whether we have traversed both the strings completely or not */ 
	if ((*pattern == '\0') && (*hostname == '\0') ) {
			return 1;
	}
	else {
		return 0;
	}
}

static int nopoll_validate_ipv4(char *ip_addr){
	int octets = 0;
	char delimeter[] = {"."};
	unsigned char *ip_val;

	unsigned char tmp[4];
	ip_val = tmp;
	*ip_val = 0;

	char *ip_addr_tmp = ip_addr;

	octets = nopoll_get_delimeter_count(ip_addr_tmp, delimeter);
	if (octets != 3){
		return 0;
	}

	while (*ip_addr_tmp != '\0'){

		if(!isdigit(*ip_addr_tmp)){
			return 0;
		}

		while ( (*ip_addr_tmp != *delimeter) && (*ip_addr_tmp != '\0')) {
			unsigned int val = (*ip_val) * 10 + (unsigned int)(*ip_addr_tmp - '0') ;
			*ip_val = val;
			ip_addr_tmp++;

			if(val >255){
				return 0;
			}
		}
		/* IP should not end with . */
		if((*ip_addr_tmp == *delimeter) && (*(ip_addr_tmp+1) == '\0'))
			return 0;
		if(*ip_addr_tmp != '\0'){
			*ip_val = 0;
			ip_addr_tmp++;
		}
	}
	return 1;
}

static int  nopoll_validate_ipv6(char *ip_addr){
	int octets = 0, colon_count =0, chr_count = 0, octet_count = 0;
	char *ip_addr_rem = NULL;
	nopoll_bool ipv4_valid = nopoll_false;
	static char ip_digits[] = "0123456789abcdef";

	char *ip_addr_tmp = ip_addr;
	/* If ip starts with ":"" then next chr should be ":"" only */
	if (*ip_addr_tmp == ':' && *(ip_addr_tmp+1) != ':'){
		return 0;
	}

	octets = nopoll_get_delimeter_count(ip_addr_tmp, ":");
	if (octets > 7)
		return 0;

	while (*ip_addr_tmp != '\0'){

		while((*ip_addr_tmp != ':') &&  (*ip_addr_tmp != '\0')){
			if (strchr(ip_digits, nopoll_to_lower(*ip_addr_tmp))){
				ip_addr_tmp++;
				chr_count +=1;
				/* octet length should not be more than 4 */
				if (chr_count > 4)
					return 0;
			}
			else if (*ip_addr_tmp == '.')
				break;
			else
				return 0;
		}

		if (*ip_addr_tmp == ':') {
			if (*(ip_addr_tmp+1) == '\0' && colon_count == 0){
				return 0;
			}
			/* Ipv6 should not contain "::" (consecutive zero's representation) more than one */
			if (*(ip_addr_tmp+1) == ':') {
				colon_count +=1;
				if(colon_count >1)
					return 0;
			}
			ip_addr_tmp++;
			ip_addr_rem = ip_addr_tmp;
			chr_count = 0;
			octet_count +=1;
			continue;
		}
		/* validate ipv6 dual addressing support (IPv6 + IPv4) */
		if (*ip_addr_tmp == '.'){
			if (octet_count > 6)
				return 0;
			if(colon_count ==0 && octet_count < 6)
				return 0;

			if(nopoll_validate_ipv4(ip_addr_rem) > 0) {
				ipv4_valid = nopoll_true;
				break;
			}
			return 0;
		}
	}

	if(colon_count == 0 && octet_count != 7 && !ipv4_valid)
		return 0;
	return 1;
}

int nopoll_hostname_validate_inet(int addr_fam, char *hostname) {

  switch(addr_fam) {
  case AF_INET:
    return (nopoll_validate_ipv4(hostname));
  case AF_INET6:
    return (nopoll_validate_ipv6(hostname));
  default:
    errno = EAFNOSUPPORT;
    return (-1);
  }
}

#ifndef __NOPOLL_HOSTNAME_VALIDATION_H__
#define __NOPOLL_HOSTNAME_VALIDATION_H__

#include <nopoll_conn.h>
#include <nopoll_private.h>

BEGIN_C_DECLS

#include <nopoll_hostcheck.h>
#include <nopoll_hostname_compare.h>

typedef enum {
	MATCH_FOUND,
	MATCH_NOT_FOUND,
	NO_SAN_PRESENT,
	MAL_FORMED_CERTIFICATE,
	ERROR
} noPollHostValidationStatus;

/**
* Validates the server's identity by looking for the expected hostname in the
* server's certificate. As described in RFC 6125, it first tries to find a match
* in the Subject Alternative Name extension. If the extension is not present in
* the certificate, it checks the Common Name instead.
*
* Returns MATCH_FOUND if a match was found.
* Returns MATCH_NOT_FOUND if no matches were found.
* Returns MAL_FORMED_CERTIFICATE if any of the hostnames had a NUL character embedded in it.
* Returns ERROR if there was an error.
*/
noPollHostValidationStatus nopoll_validate_hostname(const X509 *server_cert, const char *hostname);
END_C_DECLS
#endif /* __NOPOLL_HOSTNAME_VALIDATION_H__ */

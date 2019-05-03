#include <nopoll_hostname_validation.h>

static noPollHostValidationStatus nopoll_cert_common_name_host_check(const X509 *server_cert, const char *hostname);
static noPollHostValidationStatus nopoll_cert_subject_alt_name_host_check(const X509 *server_cert, const char *hostname);

/**
* Tries to find a match for hostname in the certificate's Common Name field.
*
* Returns MATCH_FOUND if a match was found.
* Returns MATCH_NOT_FOUND if no matches were found.
* Returns MAL_FORMED_CERTIFICATE if any of the hostnames had a NULL character embedded in it.
* Returns ERROR if there was an error.
*/
static noPollHostValidationStatus nopoll_cert_common_name_host_check(const X509 *server_cert, const char *hostname) 
{
	int cn_last_pos = -1;
	const char *cn_data = NULL;
	X509_NAME_ENTRY *cn_entry = NULL;
	ASN1_STRING *cn_asn1_val = NULL;

	/* Find the position of the CN field in the Subject field of the certificate*/
	cn_last_pos = X509_NAME_get_index_by_NID(X509_get_subject_name((X509 *) server_cert), NID_commonName, cn_last_pos);
	if (cn_last_pos < 0) {
		return ERROR;
	}

	/* Extract the CN field*/
	cn_entry = X509_NAME_get_entry(X509_get_subject_name((X509 *) server_cert), cn_last_pos);
	if (cn_entry == NULL) {
		return ERROR;
	}

	cn_asn1_val = X509_NAME_ENTRY_get_data(cn_entry);
	if (cn_asn1_val == NULL) {
		return ERROR;
	}

	/* Convert the CN field to a C string*/
#if OPENSSL_VERSION_NUMBER >= 0x010100000L
 				cn_data = (char *)ASN1_STRING_get0_data(cn_asn1_val);			
#else
				cn_data = (char *)ASN1_STRING_data(cn_asn1_val);
#endif			

	/* Make sure there isn't an embedded NULL character in the CN*/
	if (ASN1_STRING_length(cn_asn1_val) != strlen(cn_data)) {
		return MAL_FORMED_CERTIFICATE;
	}

	/* Compare expected hostname with the CN*/
    if (nopoll_match_hostname(cn_data, hostname)) {
            return MATCH_FOUND;
    }
    else {
            return MATCH_NOT_FOUND;
    }
}


/**
* Will find a match for hostname in the certificate's Subject Alternative Name extension field.
*
* Returns MATCH_FOUND if a match was found.
* Returns MATCH_NOT_FOUND if no matches were found.
* Returns MAL_FORMED_CERTIFICATE if any of the hostnames had a NULL character embedded in it.
* Returns NO_SAN_PRESENT if the SAN extension was not present in the certificate.
*/
static noPollHostValidationStatus nopoll_cert_subject_alt_name_host_check(const X509 *server_cert, const char *hostname)
{
    STACK_OF(GENERAL_NAME) *san_gen_names = NULL;
    GENERAL_NAME *general_name_value;
    noPollHostValidationStatus result = MATCH_NOT_FOUND;

    /* Try to extract the names within the SAN extension from the certificate*/
    san_gen_names = X509_get_ext_d2i((X509 *) server_cert, NID_subject_alt_name, NULL, NULL);
    if (san_gen_names == NULL) {
        return NO_SAN_PRESENT;
    }

    /* Check each name within the extension*/
    while (sk_GENERAL_NAME_num(san_gen_names) > 0) {
        general_name_value = sk_GENERAL_NAME_pop(san_gen_names);
        if (general_name_value->type == GEN_DNS) {
           /* Current name is a DNS name, let's check it*/
	#if OPENSSL_VERSION_NUMBER >= 0x010100000L
            char *dns_name = (char *) ASN1_STRING_get0_data(general_name_value->d.dNSName);
	#else
			char *dns_name = (char *) ASN1_STRING_data(general_name_value->d.dNSName);
	#endif

			/* Make sure there isn't an embedded NULL character in the DNS name*/
            if ((size_t)ASN1_STRING_length(general_name_value->d.dNSName) != strlen(dns_name)) {
                result = MAL_FORMED_CERTIFICATE;
                break;
            }
            else { /* Compare expected hostname with the DNS name*/
            	    if (nopoll_match_hostname(dns_name, hostname)) {
                        result = MATCH_FOUND;
                	    break;
                    }
                }
        }
    }
    sk_GENERAL_NAME_pop_free(san_gen_names, GENERAL_NAME_free);
	return result;
}


/**
* This function Validates the server's identity by looking for the expected hostname in the
* server's certificate. As described in RFC 6125, it first tries to find a match
* in the Subject Alternative Name extension. If the extension is not present in
* the certificate, it checks the Common Name instead.
*
* Returns MATCH_FOUND if a match was found.
* Returns MATCH_NOT_FOUND if no matches were found.
* Returns MAL_FORMED_CERTIFICATE if any of the hostnames had a NUL character embedded in it.
* Returns ERROR if there was an error.
*/

noPollHostValidationStatus nopoll_validate_hostname(const X509 *server_cert, const char *hostname) {
	noPollHostValidationStatus result;

	if((server_cert == NULL) || (hostname == NULL))
		return ERROR;

	/* First try the Subject Alternative Names extension*/
	result = nopoll_cert_subject_alt_name_host_check(server_cert, hostname);
	if (result == NO_SAN_PRESENT) {
		/* Extension was not found: try the Common Name*/
		result = nopoll_cert_common_name_host_check(server_cert, hostname);
	}

	return result;
}

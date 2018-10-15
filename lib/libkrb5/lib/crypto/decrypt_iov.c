/*
 * lib/crypto/encrypt_iov.c
 *
 * Copyright 2008 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#include "k5-int.h"
#include "etypes.h"
#include "aead.h"

krb5_error_code KRB5_CALLCONV
krb5_c_decrypt_iov(krb5_context context,
		   const krb5_keyblock *key,
		   krb5_keyusage usage,
		   const krb5_data *cipher_state,
		   krb5_crypto_iov *data,
		   size_t num_data)
{
    int i;
    const struct krb5_keytypes *ktp = NULL;

    for (i = 0; i < krb5_enctypes_length; i++) {
	if (krb5_enctypes_list[i].etype == key->enctype) {
	    ktp = &krb5_enctypes_list[i];
	    break;
	}
    }

    if (ktp == NULL || ktp->aead == NULL) {
	return KRB5_BAD_ENCTYPE;
    }

    if (krb5int_c_locate_iov(data, num_data, KRB5_CRYPTO_TYPE_STREAM) != NULL) {
	return krb5int_c_iov_decrypt_stream(ktp->aead, ktp->enc, ktp->hash,
					    key, usage, cipher_state, data, num_data);
    }

    return ktp->aead->decrypt_iov(ktp->aead, ktp->enc, ktp->hash,
				  key, usage, cipher_state, data, num_data);
}


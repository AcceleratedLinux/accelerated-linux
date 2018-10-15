/*
 * Copyright (C) 1998 by the FundsXpress, INC.
 * 
 * All rights reserved.
 * 
 * Export of this software from the United States of America may require
 * a specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of FundsXpress. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  FundsXpress makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "k5-int.h"
#include "dk.h"

#define K5CLENGTH 5 /* 32 bit net byte order integer + one byte seed */

static krb5_error_code
krb5_dk_decrypt_maybe_trunc_hmac(const struct krb5_enc_provider *enc,
				 const struct krb5_hash_provider *hash,
				 const krb5_keyblock *key,
				 krb5_keyusage usage,
				 const krb5_data *ivec,
				 const krb5_data *input,
				 krb5_data *output,
				 size_t hmacsize,
				 int ivec_mode);

krb5_error_code
krb5_dk_decrypt(const struct krb5_enc_provider *enc,
		const struct krb5_hash_provider *hash,
		const krb5_keyblock *key, krb5_keyusage usage,
		const krb5_data *ivec, const krb5_data *input,
		krb5_data *output)
{
    return krb5_dk_decrypt_maybe_trunc_hmac(enc, hash, key, usage,
					    ivec, input, output, 0, 0);
}

krb5_error_code
krb5int_aes_dk_decrypt(const struct krb5_enc_provider *enc,
		       const struct krb5_hash_provider *hash,
		       const krb5_keyblock *key, krb5_keyusage usage,
		       const krb5_data *ivec, const krb5_data *input,
		       krb5_data *output)
{
    return krb5_dk_decrypt_maybe_trunc_hmac(enc, hash, key, usage,
					    ivec, input, output, 96 / 8, 1);
}

static krb5_error_code
krb5_dk_decrypt_maybe_trunc_hmac(const struct krb5_enc_provider *enc,
				 const struct krb5_hash_provider *hash,
				 const krb5_keyblock *key, krb5_keyusage usage,
				 const krb5_data *ivec, const krb5_data *input,
				 krb5_data *output, size_t hmacsize,
				 int ivec_mode)
{
    krb5_error_code ret;
    size_t hashsize, blocksize, keybytes, keylength, enclen, plainlen;
    unsigned char *plaindata, *kedata, *kidata, *cksum, *cn;
    krb5_keyblock ke, ki;
    krb5_data d1, d2;
    unsigned char constantdata[K5CLENGTH];

    /* allocate and set up ciphertext and to-be-derived keys */

    hashsize = hash->hashsize;
    blocksize = enc->block_size;
    keybytes = enc->keybytes;
    keylength = enc->keylength;

    if (hmacsize == 0)
	hmacsize = hashsize;
    else if (hmacsize > hashsize)
	return KRB5KRB_AP_ERR_BAD_INTEGRITY;

    enclen = input->length - hmacsize;

    if ((kedata = (unsigned char *) malloc(keylength)) == NULL)
	return(ENOMEM);
    if ((kidata = (unsigned char *) malloc(keylength)) == NULL) {
	free(kedata);
	return(ENOMEM);
    }
    if ((plaindata = (unsigned char *) malloc(enclen)) == NULL) {
	free(kidata);
	free(kedata);
	return(ENOMEM);
    }
    if ((cksum = (unsigned char *) malloc(hashsize)) == NULL) {
	free(plaindata);
	free(kidata);
	free(kedata);
	return(ENOMEM);
    }

    ke.contents = kedata;
    ke.length = keylength;
    ki.contents = kidata;
    ki.length = keylength;

    /* derive the keys */

    d1.data = (char *) constantdata;
    d1.length = K5CLENGTH;

    store_32_be(usage, constantdata);

    d1.data[4] = (char) 0xAA;

    if ((ret = krb5_derive_key(enc, key, &ke, &d1)) != 0)
	goto cleanup;

    d1.data[4] = 0x55;

    if ((ret = krb5_derive_key(enc, key, &ki, &d1)) != 0)
	goto cleanup;

    /* decrypt the ciphertext */

    d1.length = enclen;
    d1.data = input->data;

    d2.length = enclen;
    d2.data = (char *) plaindata;

    if ((ret = ((*(enc->decrypt))(&ke, ivec, &d1, &d2))) != 0)
	goto cleanup;

    if (ivec != NULL && ivec->length == blocksize) {
	if (ivec_mode == 0)
	    cn = (unsigned char *) d1.data + d1.length - blocksize;
	else if (ivec_mode == 1) {
	    int nblocks = (d1.length + blocksize - 1) / blocksize;
	    cn = d1.data + blocksize * (nblocks - 2);
	} else
	    abort();
    } else
	cn = NULL;

    /* verify the hash */

    d1.length = hashsize;
    d1.data = (char *) cksum;

    if ((ret = krb5_hmac(hash, &ki, 1, &d2, &d1)) != 0)
	goto cleanup;

    if (memcmp(cksum, input->data+enclen, hmacsize) != 0) {
	ret = KRB5KRB_AP_ERR_BAD_INTEGRITY;
	goto cleanup;
    }

    /* because this encoding isn't self-describing wrt length, the
       best we can do here is to compute the length minus the
       confounder. */

    plainlen = enclen - blocksize;

    if (output->length < plainlen)
	return(KRB5_BAD_MSIZE);

    output->length = plainlen;

    memcpy(output->data, d2.data+blocksize, output->length);

    if (cn != NULL)
	memcpy(ivec->data, cn, blocksize);

    ret = 0;

cleanup:
    memset(kedata, 0, keylength);
    memset(kidata, 0, keylength);
    memset(plaindata, 0, enclen);
    memset(cksum, 0, hashsize);

    free(cksum);
    free(plaindata);
    free(kidata);
    free(kedata);

    return(ret);
}


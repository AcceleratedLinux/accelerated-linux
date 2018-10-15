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
#include "etypes.h"
#include "aead.h"

krb5_error_code KRB5_CALLCONV
krb5_c_encrypt(krb5_context context, const krb5_keyblock *key,
	       krb5_keyusage usage, const krb5_data *ivec,
	       const krb5_data *input, krb5_enc_data *output)
{
    int i;

    for (i=0; i<krb5_enctypes_length; i++) {
	if (krb5_enctypes_list[i].etype == key->enctype)
	    break;
    }

    if (i == krb5_enctypes_length)
	return(KRB5_BAD_ENCTYPE);

    output->magic = KV5M_ENC_DATA;
    output->kvno = 0;
    output->enctype = key->enctype;

    if (krb5_enctypes_list[i].encrypt == NULL) {
	assert(krb5_enctypes_list[i].aead != NULL);

	return krb5int_c_encrypt_aead_compat(krb5_enctypes_list[i].aead,
					     krb5_enctypes_list[i].enc,
					     krb5_enctypes_list[i].hash,
					     key, usage, ivec,
					     input, &output->ciphertext);
    }

    return((*(krb5_enctypes_list[i].encrypt))
	   (krb5_enctypes_list[i].enc, krb5_enctypes_list[i].hash,
	    key, usage, ivec, input, &output->ciphertext));
}

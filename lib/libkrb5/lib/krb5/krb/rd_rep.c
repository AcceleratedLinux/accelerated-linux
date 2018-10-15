/*
 * lib/krb5/krb/rd_rep.c
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology.
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
 * 
 *
 * krb5_rd_rep()
 */
/*
 * Copyright (c) 2006-2008, Novell, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *   * The copyright holder's name is not used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "k5-int.h"
#include "auth_con.h"

/*
 *  Parses a KRB_AP_REP message, returning its contents.
 * 
 *  repl is filled in with with a pointer to allocated memory containing
 * the fields from the encrypted response. 
 * 
 *  the key in kblock is used to decrypt the message.
 * 
 *  returns system errors, encryption errors, replay errors
 */

krb5_error_code KRB5_CALLCONV
krb5_rd_rep(krb5_context context, krb5_auth_context auth_context,
	    const krb5_data *inbuf, krb5_ap_rep_enc_part **repl)
{
    krb5_error_code 	  retval;
    krb5_ap_rep 	* reply;
    krb5_data 	 	  scratch;

    if (!krb5_is_ap_rep(inbuf))
	return KRB5KRB_AP_ERR_MSG_TYPE;

    /* decode it */

    if ((retval = decode_krb5_ap_rep(inbuf, &reply)))
	return retval;

    /* put together an eblock for this encryption */

    scratch.length = reply->enc_part.ciphertext.length;
    if (!(scratch.data = malloc(scratch.length))) {
	krb5_free_ap_rep(context, reply);
	return(ENOMEM);
    }

    if ((retval = krb5_c_decrypt(context, auth_context->keyblock,
				 KRB5_KEYUSAGE_AP_REP_ENCPART, 0,
				 &reply->enc_part, &scratch)))
	goto clean_scratch;

    /* now decode the decrypted stuff */
    retval = decode_krb5_ap_rep_enc_part(&scratch, repl);
    if (retval)
	goto clean_scratch;

    /* Check reply fields */
    if (((*repl)->ctime != auth_context->authentp->ctime) ||
      ((*repl)->cusec != auth_context->authentp->cusec)) {
	retval = KRB5_MUTUAL_FAILED;
	goto clean_scratch;
    }

    /* Set auth subkey */
    if ((*repl)->subkey) {
	if (auth_context->recv_subkey) {
	    krb5_free_keyblock(context, auth_context->recv_subkey);
	    auth_context->recv_subkey = NULL;
	}
	retval = krb5_copy_keyblock(context, (*repl)->subkey,
				    &auth_context->recv_subkey);
	if (retval)
	    goto clean_scratch;
	if (auth_context->send_subkey) {
	    krb5_free_keyblock(context, auth_context->send_subkey);
	    auth_context->send_subkey = NULL;
	}
	retval = krb5_copy_keyblock(context, (*repl)->subkey,
				    &auth_context->send_subkey);
	if (retval) {
	    krb5_free_keyblock(context, auth_context->send_subkey);
	    auth_context->send_subkey = NULL;
	}
	/* not used for anything yet */
	auth_context->negotiated_etype = (*repl)->subkey->enctype;
    }

    /* Get remote sequence number */
    auth_context->remote_seq_number = (*repl)->seq_number;

clean_scratch:
    memset(scratch.data, 0, scratch.length); 

    krb5_free_ap_rep(context, reply);
    free(scratch.data);
    return retval;
}

krb5_error_code KRB5_CALLCONV
krb5_rd_rep_dce(krb5_context context, krb5_auth_context auth_context,
		const krb5_data *inbuf, krb5_ui_4 *nonce)
{
    krb5_error_code 	  retval;
    krb5_ap_rep 	* reply;
    krb5_data 	 	  scratch;
    krb5_ap_rep_enc_part *repl = NULL;

    if (!krb5_is_ap_rep(inbuf))
	return KRB5KRB_AP_ERR_MSG_TYPE;

    /* decode it */

    if ((retval = decode_krb5_ap_rep(inbuf, &reply)))
	return retval;

    /* put together an eblock for this encryption */

    scratch.length = reply->enc_part.ciphertext.length;
    if (!(scratch.data = malloc(scratch.length))) {
	krb5_free_ap_rep(context, reply);
	return(ENOMEM);
    }

    if ((retval = krb5_c_decrypt(context, auth_context->keyblock,
				 KRB5_KEYUSAGE_AP_REP_ENCPART, 0,
				 &reply->enc_part, &scratch)))
	goto clean_scratch;

    /* now decode the decrypted stuff */
    retval = decode_krb5_ap_rep_enc_part(&scratch, &repl);
    if (retval)
	goto clean_scratch;

    *nonce = repl->seq_number;
    if (*nonce != auth_context->local_seq_number) {
	retval = KRB5_MUTUAL_FAILED;
	goto clean_scratch;
    }

    /* Must be NULL to prevent echoing for client AP-REP */
    if (repl->subkey != NULL) {
	retval = KRB5_MUTUAL_FAILED;
	goto clean_scratch;
    }

clean_scratch:
    memset(scratch.data, 0, scratch.length); 

    if (repl != NULL)
	krb5_free_ap_rep_enc_part(context, repl);
    krb5_free_ap_rep(context, reply);
    free(scratch.data);
    return retval;
}

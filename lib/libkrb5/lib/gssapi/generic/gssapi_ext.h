/*
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
 *
 */

#ifndef GSSAPI_EXT_H_
#define GSSAPI_EXT_H_

#include <gssapi/gssapi.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if 0
/*
 * Solaris extensions
 */
int KRB5_CALLCONV gssd_pname_to_uid
	(char *,
	 gss_OID,
	 gss_OID,
	 uid_t *);

int KRB5_CALLCONV __gss_userok
	(const gss_name_t /*name*/,
	 const char * /*username*/);
#endif

/*
 * GGF extensions
 */
typedef struct gss_buffer_set_desc_struct {
    size_t count;
    gss_buffer_desc *elements;
} gss_buffer_set_desc, *gss_buffer_set_t;

#define GSS_C_NO_BUFFER_SET ((gss_buffer_set_t) 0)

OM_uint32 KRB5_CALLCONV gss_create_empty_buffer_set
	(OM_uint32 * /*minor_status*/,
	 gss_buffer_set_t * /*buffer_set*/);

OM_uint32 KRB5_CALLCONV gss_add_buffer_set_member
	(OM_uint32 * /*minor_status*/,
	 const gss_buffer_t /*member_buffer*/,
	 gss_buffer_set_t * /*buffer_set*/);

OM_uint32 KRB5_CALLCONV gss_release_buffer_set
	(OM_uint32 * /*minor_status*/,
	 gss_buffer_set_t * /*buffer_set*/);

OM_uint32 KRB5_CALLCONV gss_inquire_sec_context_by_oid
	(OM_uint32 * /*minor_status*/,
	 const gss_ctx_id_t /*context_handle*/,
	 const gss_OID /*desired_object*/,
	 gss_buffer_set_t * /*data_set*/);

OM_uint32 KRB5_CALLCONV gss_inquire_cred_by_oid
	(OM_uint32 * /*minor_status*/,
	 const gss_cred_id_t /*cred_handle*/,
	 const gss_OID /*desired_object*/,
	 gss_buffer_set_t * /*data_set*/);

OM_uint32 KRB5_CALLCONV gss_set_sec_context_option
	(OM_uint32 * /*minor_status*/,
	 gss_ctx_id_t * /*cred_handle*/,
	 const gss_OID /*desired_object*/,
	 const gss_buffer_t /*value*/);

/* XXX do these really belong in this header? */
OM_uint32 KRB5_CALLCONV gssspi_set_cred_option
	(OM_uint32 * /*minor_status*/,
	 gss_cred_id_t /*cred*/,
	 const gss_OID /*desired_object*/,
	 const gss_buffer_t /*value*/);

OM_uint32 KRB5_CALLCONV gssspi_mech_invoke
	(OM_uint32 * /*minor_status*/,
	 const gss_OID /*desired_mech*/,
	 const gss_OID /*desired_object*/,
	 gss_buffer_t /*value*/);

/*
 * AEAD extensions
 */

OM_uint32 KRB5_CALLCONV gss_wrap_aead
	(OM_uint32 * /*minor_status*/,
	 gss_ctx_id_t /*context_handle*/,
	 int /*conf_req_flag*/,
	 gss_qop_t /*qop_req*/,
	 gss_buffer_t /*input_assoc_buffer*/,
	 gss_buffer_t /*input_payload_buffer*/,
	 int * /*conf_state*/,
	 gss_buffer_t /*output_message_buffer*/);

OM_uint32 KRB5_CALLCONV gss_unwrap_aead
	(OM_uint32 * /*minor_status*/,
	 gss_ctx_id_t /*context_handle*/,
	 gss_buffer_t /*input_message_buffer*/,
	 gss_buffer_t /*input_assoc_buffer*/,
	 gss_buffer_t /*output_payload_buffer*/,
	 int * /*conf_state*/,
	 gss_qop_t * /*qop_state*/);

/*
 * SSPI extensions
 */
#define GSS_C_DCE_STYLE			0x1000
#define GSS_C_IDENTIFY_FLAG		0x2000
#define GSS_C_EXTENDED_ERROR_FLAG	0x4000

/*
 * Returns a buffer set with the first member containing the
 * session key for SSPI compatibility. The optional second
 * member contains an OID identifying the session key type.
 */
GSS_DLLIMP extern gss_OID GSS_C_INQ_SSPI_SESSION_KEY;

OM_uint32 KRB5_CALLCONV gss_complete_auth_token
	(OM_uint32 *minor_status,
	 const gss_ctx_id_t context_handle,
	 gss_buffer_t input_message_buffer);

typedef struct gss_iov_buffer_desc_struct {
    OM_uint32 type;
    gss_buffer_desc buffer;
} gss_iov_buffer_desc, *gss_iov_buffer_t;

#define GSS_C_NO_IOV_BUFFER		    ((gss_iov_buffer_t)0)

#define GSS_IOV_BUFFER_TYPE_EMPTY	    0
#define GSS_IOV_BUFFER_TYPE_DATA	    1	/* Packet data */
#define GSS_IOV_BUFFER_TYPE_HEADER	    2	/* Mechanism header */
#define GSS_IOV_BUFFER_TYPE_MECH_PARAMS	    3	/* Mechanism specific parameters */
#define GSS_IOV_BUFFER_TYPE_TRAILER	    7	/* Mechanism trailer */
#define GSS_IOV_BUFFER_TYPE_PADDING	    9	/* Padding */
#define GSS_IOV_BUFFER_TYPE_STREAM	    10	/* Complete wrap token */
#define GSS_IOV_BUFFER_TYPE_SIGN_ONLY	    11	/* Sign only packet data */

#define GSS_IOV_BUFFER_FLAG_MASK	    0xFFFF0000
#define GSS_IOV_BUFFER_FLAG_ALLOCATE	    0x00010000	/* indicates GSS should allocate */
#define GSS_IOV_BUFFER_FLAG_ALLOCATED	    0x00020000	/* indicates caller should free */

#define GSS_IOV_BUFFER_TYPE(_type)	    ((_type) & ~(GSS_IOV_BUFFER_FLAG_MASK))
#define GSS_IOV_BUFFER_FLAGS(_type)	    ((_type) & GSS_IOV_BUFFER_FLAG_MASK)

/*
 * Sign and optionally encrypt a sequence of buffers. The buffers
 * shall be ordered HEADER | DATA | PADDING | TRAILER. Suitable
 * space for the header, padding and trailer should be provided
 * by calling gss_wrap_iov_length(), or the ALLOCATE flag should
 * be set on those buffers.
 *
 * Encryption is in-place. SIGN_ONLY buffers are untouched. Only
 * a single PADDING buffer should be provided. The order of the
 * buffers in memory does not matter. Buffers in the IOV should
 * be arranged in the order above, and in the case of multiple
 * DATA buffers the sender and receiver should agree on the
 * order.
 *
 * With GSS_C_DCE_STYLE it is acceptable to not provide PADDING
 * and TRAILER, but the caller must guarantee the plaintext data
 * being encrypted is correctly padded, otherwise an error will
 * be returned.
 *
 * While applications that have knowledge of the underlying
 * cryptosystem may request a specific configuration of data
 * buffers, the only generally supported configurations are:
 *
 *  HEADER | DATA | PADDING | TRAILER
 *
 * which will emit GSS_Wrap() compatible tokens, and:
 *
 *  HEADER | SIGN_ONLY | DATA | PADDING | TRAILER
 *
 * for AEAD.
 *
 * The typical (special cased) usage for DCE is as follows:
 *
 *  SIGN_ONLY_1 | DATA | SIGN_ONLY_2 | HEADER
 */
OM_uint32 KRB5_CALLCONV gss_wrap_iov
(
    OM_uint32 *,	/* minor_status */
    gss_ctx_id_t,       /* context_handle */
    int,		/* conf_req_flag */
    gss_qop_t,		/* qop_req */
    int *,		/* conf_state */
    gss_iov_buffer_desc *,    /* iov */
    int);		/* iov_count */

/*
 * Verify and optionally decrypt a sequence of buffers. To process
 * a GSS-API message without separate buffer, pass STREAM | DATA.
 * Upon return DATA will contain the decrypted or integrity
 * protected message. Only a single DATA buffer may be provided
 * with this usage. DATA by default will point into STREAM, but if
 * the ALLOCATE flag is set a copy will be returned.
 *
 * Otherwise, decryption is in-place. SIGN_ONLY buffers are
 * untouched.
 */
OM_uint32 KRB5_CALLCONV gss_unwrap_iov
(
    OM_uint32 *,	/* minor_status */
    gss_ctx_id_t,       /* context_handle */
    int *,		/* conf_state */
    gss_qop_t *,	/* qop_state */
    gss_iov_buffer_desc *,    /* iov */
    int);		/* iov_count */

/*
 * Query HEADER, PADDING and TRAILER buffer lengths. DATA buffers
 * should be provided so the correct padding length can be determined.
 */
OM_uint32 KRB5_CALLCONV gss_wrap_iov_length
(
    OM_uint32 *,	/* minor_status */
    gss_ctx_id_t,	/* context_handle */
    int,		/* conf_req_flag */
    gss_qop_t,		/* qop_req */
    int *,		/* conf_state */
    gss_iov_buffer_desc *, /* iov */
    int);		/* iov_count */

/*
 * Release buffers that have the ALLOCATED flag set.
 */
OM_uint32 KRB5_CALLCONV gss_release_iov_buffer
(
    OM_uint32 *,	/* minor_status */
    gss_iov_buffer_desc *, /* iov */
    int);		/* iov_count */

#ifdef __cplusplus
}
#endif

#endif /* GSSAPI_EXT_H_ */

/* -*- mode: c; indent-tabs-mode: nil -*- */
/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "gssapiP_krb5.h"

/*
 * $Id: sign.c 21778 2009-01-22 23:21:11Z tlyu $
 */

/* V2 interface */
OM_uint32
krb5_gss_get_mic(minor_status, context_handle, qop_req,
                 message_buffer, message_token)
    OM_uint32           *minor_status;
    gss_ctx_id_t        context_handle;
    gss_qop_t           qop_req;
    gss_buffer_t        message_buffer;
    gss_buffer_t        message_token;
{
    return(kg_seal(minor_status, context_handle, 0,
                   qop_req, message_buffer, NULL,
                   message_token, KG_TOK_MIC_MSG));
}

#if 0
OM_uint32
krb5_gss_get_mic_iov(OM_uint32 *minor_status,
                     gss_ctx_id_t context_handle,
                     gss_qop_t qop_req,
                     gss_iov_buffer_desc *iov,
                     int iov_count)
{
    OM_uint32 major_status;

    major_status = kg_seal_iov(minor_status, context_handle, FALSE,
                               qop_req, NULL,
                               iov, iov_count, KG_TOK_MIC_MSG);

    return major_status;
}

OM_uint32
krb5_gss_get_mic_iov_length(OM_uint32 *minor_status,
                            gss_ctx_id_t context_handle,
                            int conf_req_flag,
                            gss_qop_t qop_req,
                            int *conf_state,
                            gss_iov_buffer_desc *iov,
                            int iov_count)
{
    OM_uint32 major_status;

    major_status = kg_seal_iov_length(minor_status, context_handle, conf_req_flag,
                                      qop_req, conf_state, iov, iov_count);
    return major_status;
}
#endif

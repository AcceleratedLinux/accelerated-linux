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

/*
 * $Id: gssapi_krb5.c 21779 2009-01-22 23:37:35Z tlyu $
 */


/* For declaration of krb5_ser_context_init */
#include "k5-int.h"
#include "gssapiP_krb5.h"
#include "mglueP.h"

/** exported constants defined in gssapi_krb5{,_nx}.h **/

/* these are bogus, but will compile */

/*
 * The OID of the draft krb5 mechanism, assigned by IETF, is:
 *      iso(1) org(3) dod(5) internet(1) security(5)
 *      kerberosv5(2) = 1.3.5.1.5.2
 * The OID of the krb5_name type is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_name(1) = 1.2.840.113554.1.2.2.1
 * The OID of the krb5_principal type is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_principal(2) = 1.2.840.113554.1.2.2.2
 * The OID of the proposed standard krb5 mechanism is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) = 1.2.840.113554.1.2.2
 * The OID of the proposed standard krb5 v2 mechanism is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5v2(3) = 1.2.840.113554.1.2.3
 * Provisionally reserved for Kerberos session key algorithm
 * identifiers is:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_enctype(4) = 1.2.840.113554.1.2.2.4
 * Provisionally reserved for Kerberos mechanism-specific APIs:
 *      iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 *      krb5(2) krb5_gssapi_ext(5) = 1.2.840.113554.1.2.2.5
 */

/*
 * Encoding rules: The first two values are encoded in one byte as 40
 * * value1 + value2.  Subsequent values are encoded base 128, most
 * significant digit first, with the high bit (\200) set on all octets
 * except the last in each value's encoding.
 */

const gss_OID_desc krb5_gss_oid_array[] = {
    /* this is the official, rfc-specified OID */
    {GSS_MECH_KRB5_OID_LENGTH, GSS_MECH_KRB5_OID},
    /* this pre-RFC mech OID */
    {GSS_MECH_KRB5_OLD_OID_LENGTH, GSS_MECH_KRB5_OLD_OID},
    /* this is the unofficial, incorrect mech OID emitted by MS */
    {GSS_MECH_KRB5_WRONG_OID_LENGTH, GSS_MECH_KRB5_WRONG_OID},
    /* this is the v2 assigned OID */
    {9, "\052\206\110\206\367\022\001\002\003"},
    /* these two are name type OID's */

    /* 2.1.1. Kerberos Principal Name Form:  (rfc 1964)
     * This name form shall be represented by the Object Identifier {iso(1)
     * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
     * krb5(2) krb5_name(1)}.  The recommended symbolic name for this type
     * is "GSS_KRB5_NT_PRINCIPAL_NAME". */
    {10, "\052\206\110\206\367\022\001\002\002\001"},

    /* gss_nt_krb5_principal.  Object identifier for a krb5_principal. Do not use. */
    {10, "\052\206\110\206\367\022\001\002\002\002"},
    { 0, 0 }
};

const gss_OID_desc * const gss_mech_krb5              = krb5_gss_oid_array+0;
const gss_OID_desc * const gss_mech_krb5_old          = krb5_gss_oid_array+1;
const gss_OID_desc * const gss_mech_krb5_wrong        = krb5_gss_oid_array+2;
const gss_OID_desc * const gss_nt_krb5_name           = krb5_gss_oid_array+4;
const gss_OID_desc * const gss_nt_krb5_principal      = krb5_gss_oid_array+5;
const gss_OID_desc * const GSS_KRB5_NT_PRINCIPAL_NAME = krb5_gss_oid_array+4;

static const gss_OID_set_desc oidsets[] = {
    {1, (gss_OID) krb5_gss_oid_array+0},
    {1, (gss_OID) krb5_gss_oid_array+1},
    {3, (gss_OID) krb5_gss_oid_array+0},
    {1, (gss_OID) krb5_gss_oid_array+2},
    {3, (gss_OID) krb5_gss_oid_array+0},
};

const gss_OID_set_desc * const gss_mech_set_krb5 = oidsets+0;
const gss_OID_set_desc * const gss_mech_set_krb5_old = oidsets+1;
const gss_OID_set_desc * const gss_mech_set_krb5_both = oidsets+2;

g_set kg_vdb = G_SET_INIT;

/** default credential support */

/*
 * init_sec_context() will explicitly re-acquire default credentials,
 * so handling the expiration/invalidation condition here isn't needed.
 */
OM_uint32
kg_get_defcred(minor_status, cred)
    OM_uint32 *minor_status;
    gss_cred_id_t *cred;
{
    OM_uint32 major;

    if ((major = krb5_gss_acquire_cred(minor_status,
                                       (gss_name_t) NULL, GSS_C_INDEFINITE,
                                       GSS_C_NULL_OID_SET, GSS_C_INITIATE,
                                       cred, NULL, NULL)) && GSS_ERROR(major)) {
        return(major);
    }
    *minor_status = 0;
    return(GSS_S_COMPLETE);
}

OM_uint32
kg_sync_ccache_name (krb5_context context, OM_uint32 *minor_status)
{
    OM_uint32 err = 0;

    /*
     * Sync up the context ccache name with the GSSAPI ccache name.
     * If kg_ccache_name is NULL -- normal unless someone has called
     * gss_krb5_ccache_name() -- then the system default ccache will
     * be picked up and used by resetting the context default ccache.
     * This is needed for platforms which support multiple ccaches.
     */

    if (!err) {
        /* if NULL, resets the context default ccache */
        err = krb5_cc_set_default_name(context,
                                       (char *) k5_getspecific(K5_KEY_GSS_KRB5_CCACHE_NAME));
    }

    *minor_status = err;
    return (*minor_status == 0) ? GSS_S_COMPLETE : GSS_S_FAILURE;
}

/* This function returns whether or not the caller set a cccache name.  Used by
 * gss_acquire_cred to figure out if the caller wants to only look at this
 * ccache or search the cache collection for the desired name */
OM_uint32
kg_caller_provided_ccache_name (OM_uint32 *minor_status,
                                int *out_caller_provided_name)
{
    if (out_caller_provided_name) {
        *out_caller_provided_name =
            (k5_getspecific(K5_KEY_GSS_KRB5_CCACHE_NAME) != NULL);
    }

    *minor_status = 0;
    return GSS_S_COMPLETE;
}

OM_uint32
kg_get_ccache_name (OM_uint32 *minor_status, const char **out_name)
{
    const char *name = NULL;
    OM_uint32 err = 0;
    char *kg_ccache_name;

    kg_ccache_name = k5_getspecific(K5_KEY_GSS_KRB5_CCACHE_NAME);

    if (kg_ccache_name != NULL) {
        name = strdup(kg_ccache_name);
        if (name == NULL)
            err = ENOMEM;
    } else {
        krb5_context context = NULL;

        /* Reset the context default ccache (see text above), and then
           retrieve it.  */
        err = krb5_gss_init_context(&context);
        if (!err)
            err = krb5_cc_set_default_name (context, NULL);
        if (!err) {
            name = krb5_cc_default_name(context);
            if (name) {
                name = strdup(name);
                if (name == NULL)
                    err = ENOMEM;
            }
        }
        if (err && context)
            save_error_info(err, context);
        if (context)
            krb5_free_context(context);
    }

    if (!err) {
        if (out_name) {
            *out_name = name;
        }
    }

    *minor_status = err;
    return (*minor_status == 0) ? GSS_S_COMPLETE : GSS_S_FAILURE;
}

OM_uint32
kg_set_ccache_name (OM_uint32 *minor_status, const char *name)
{
    char *new_name = NULL;
    char *swap = NULL;
    char *kg_ccache_name;
    krb5_error_code kerr;

    if (name) {
        new_name = strdup(name);
        if (new_name == NULL) {
            *minor_status = ENOMEM;
            return GSS_S_FAILURE;
        }
    }

    kg_ccache_name = k5_getspecific(K5_KEY_GSS_KRB5_CCACHE_NAME);
    swap = kg_ccache_name;
    kg_ccache_name = new_name;
    new_name = swap;
    kerr = k5_setspecific(K5_KEY_GSS_KRB5_CCACHE_NAME, kg_ccache_name);
    if (kerr != 0) {
        /* Can't store, so free up the storage.  */
        free(kg_ccache_name);
        /* ??? free(new_name); */
        *minor_status = kerr;
        return GSS_S_FAILURE;
    }

    free (new_name);
    *minor_status = 0;
    return GSS_S_COMPLETE;
}

#define g_OID_prefix_equal(o1, o2) \
        (((o1)->length >= (o2)->length) && \
        (memcmp((o1)->elements, (o2)->elements, (o2)->length) == 0))

/*
 * gss_inquire_sec_context_by_oid() methods
 */
static struct {
    gss_OID_desc oid;
    OM_uint32 (*func)(OM_uint32 *, const gss_ctx_id_t, const gss_OID, gss_buffer_set_t *);
} krb5_gss_inquire_sec_context_by_oid_ops[] = {
    {
        {GSS_KRB5_GET_TKT_FLAGS_OID_LENGTH, GSS_KRB5_GET_TKT_FLAGS_OID},
        gss_krb5int_get_tkt_flags
    },
    {
        {GSS_KRB5_EXTRACT_AUTHZ_DATA_FROM_SEC_CONTEXT_OID_LENGTH, GSS_KRB5_EXTRACT_AUTHZ_DATA_FROM_SEC_CONTEXT_OID},
        gss_krb5int_extract_authz_data_from_sec_context
    },
    {
        {GSS_KRB5_INQ_SSPI_SESSION_KEY_OID_LENGTH, GSS_KRB5_INQ_SSPI_SESSION_KEY_OID},
        gss_krb5int_inq_session_key
    },
    {
        {GSS_KRB5_EXPORT_LUCID_SEC_CONTEXT_OID_LENGTH, GSS_KRB5_EXPORT_LUCID_SEC_CONTEXT_OID},
        gss_krb5int_export_lucid_sec_context
    },
    {
        {GSS_KRB5_EXTRACT_AUTHTIME_FROM_SEC_CONTEXT_OID_LENGTH, GSS_KRB5_EXTRACT_AUTHTIME_FROM_SEC_CONTEXT_OID},
        gss_krb5int_extract_authtime_from_sec_context
    }
};

static OM_uint32
krb5_gss_inquire_sec_context_by_oid (OM_uint32 *minor_status,
                                     const gss_ctx_id_t context_handle,
                                     const gss_OID desired_object,
                                     gss_buffer_set_t *data_set)
{
    krb5_gss_ctx_id_rec *ctx;
    size_t i;

    if (minor_status == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *minor_status = 0;

    if (desired_object == GSS_C_NO_OID)
        return GSS_S_CALL_INACCESSIBLE_READ;

    if (data_set == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *data_set = GSS_C_NO_BUFFER_SET;

    if (!kg_validate_ctx_id(context_handle))
        return GSS_S_NO_CONTEXT;

    ctx = (krb5_gss_ctx_id_rec *) context_handle;

    if (!ctx->established)
        return GSS_S_NO_CONTEXT;

    for (i = 0; i < sizeof(krb5_gss_inquire_sec_context_by_oid_ops)/
                    sizeof(krb5_gss_inquire_sec_context_by_oid_ops[0]); i++) {
        if (g_OID_prefix_equal(desired_object, &krb5_gss_inquire_sec_context_by_oid_ops[i].oid)) {
            return (*krb5_gss_inquire_sec_context_by_oid_ops[i].func)(minor_status,
                                                                      context_handle,
                                                                      desired_object,
                                                                      data_set);
        }
    }

    *minor_status = EINVAL;

    return GSS_S_UNAVAILABLE;
}

/*
 * gss_inquire_cred_by_oid() methods
 */
#if 0
static struct {
    gss_OID_desc oid;
    OM_uint32 (*func)(OM_uint32 *, const gss_cred_id_t, const gss_OID, gss_buffer_set_t *);
} krb5_gss_inquire_cred_by_oid_ops[] = {
};
#endif

static OM_uint32
krb5_gss_inquire_cred_by_oid(OM_uint32 *minor_status,
                             const gss_cred_id_t cred_handle,
                             const gss_OID desired_object,
                             gss_buffer_set_t *data_set)
{
    OM_uint32 major_status = GSS_S_FAILURE;
    krb5_gss_cred_id_t cred;
    size_t i;

    if (minor_status == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *minor_status = 0;

    if (desired_object == GSS_C_NO_OID)
        return GSS_S_CALL_INACCESSIBLE_READ;

    if (data_set == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *data_set = GSS_C_NO_BUFFER_SET;
    if (cred_handle == GSS_C_NO_CREDENTIAL) {
        *minor_status = (OM_uint32)KRB5_NOCREDS_SUPPLIED;
        return GSS_S_NO_CRED;
    }

    major_status = krb5_gss_validate_cred(minor_status, cred_handle);
    if (GSS_ERROR(major_status))
        return major_status;

    cred = (krb5_gss_cred_id_t) cred_handle;

#if 0
    for (i = 0; i < sizeof(krb5_gss_inquire_cred_by_oid_ops)/
                    sizeof(krb5_gss_inquire_cred_by_oid_ops[0]); i++) {
        if (g_OID_prefix_equal(desired_object, &krb5_gss_inquire_cred_by_oid_ops[i].oid)) {
            return (*krb5_gss_inquire_cred_by_oid_ops[i].func)(minor_status,
                                                               cred_handle,
                                                               desired_object,
                                                               data_set);
        }
    }
#endif

    *minor_status = EINVAL;

    return GSS_S_UNAVAILABLE;
}

/*
 * gss_set_sec_context_option() methods
 */
#if 0
static struct {
    gss_OID_desc oid;
    OM_uint32 (*func)(OM_uint32 *, gss_ctx_id_t *, const gss_OID, const gss_buffer_t);
} krb5_gss_set_sec_context_option_ops[] = {
};
#endif

static OM_uint32
krb5_gss_set_sec_context_option (OM_uint32 *minor_status,
                                 gss_ctx_id_t *context_handle,
                                 const gss_OID desired_object,
                                 const gss_buffer_t value)
{
    size_t i;

    if (minor_status == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *minor_status = 0;

    if (context_handle == NULL)
        return GSS_S_CALL_INACCESSIBLE_READ;

    if (desired_object == GSS_C_NO_OID)
        return GSS_S_CALL_INACCESSIBLE_READ;

    if (*context_handle != GSS_C_NO_CONTEXT) {
        krb5_gss_ctx_id_rec *ctx;

        if (!kg_validate_ctx_id(*context_handle))
            return GSS_S_NO_CONTEXT;

        ctx = (krb5_gss_ctx_id_rec *) context_handle;

        if (!ctx->established)
            return GSS_S_NO_CONTEXT;
    }

#if 0
    for (i = 0; i < sizeof(krb5_gss_set_sec_context_option_ops)/
                    sizeof(krb5_gss_set_sec_context_option_ops[0]); i++) {
        if (g_OID_prefix_equal(desired_object, &krb5_gss_set_sec_context_option_ops[i].oid)) {
            return (*krb5_gss_set_sec_context_option_ops[i].func)(minor_status,
                                                                  context_handle,
                                                                  desired_object,
                                                                  value);
        }
    }
#endif

    *minor_status = EINVAL;

    return GSS_S_UNAVAILABLE;
}

/*
 * gssspi_set_cred_option() methods
 */
static struct {
    gss_OID_desc oid;
    OM_uint32 (*func)(OM_uint32 *, gss_cred_id_t, const gss_OID, const gss_buffer_t);
} krb5_gssspi_set_cred_option_ops[] = {
    {
        {GSS_KRB5_COPY_CCACHE_OID_LENGTH, GSS_KRB5_COPY_CCACHE_OID},
        gss_krb5int_copy_ccache
    },
    {
        {GSS_KRB5_SET_ALLOWABLE_ENCTYPES_OID_LENGTH, GSS_KRB5_SET_ALLOWABLE_ENCTYPES_OID},
        gss_krb5int_set_allowable_enctypes
    },
    {
        {GSS_KRB5_SET_CRED_RCACHE_OID_LENGTH, GSS_KRB5_SET_CRED_RCACHE_OID},
        gss_krb5int_set_cred_rcache
    }
};

static OM_uint32
krb5_gssspi_set_cred_option(OM_uint32 *minor_status,
                            gss_cred_id_t cred_handle,
                            const gss_OID desired_object,
                            const gss_buffer_t value)
{
    OM_uint32 major_status = GSS_S_FAILURE;
    size_t i;

    if (minor_status == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *minor_status = 0;

    if (cred_handle == GSS_C_NO_CREDENTIAL) {
        *minor_status = (OM_uint32)KRB5_NOCREDS_SUPPLIED;
        return GSS_S_NO_CRED;
    }

    if (desired_object == GSS_C_NO_OID)
        return GSS_S_CALL_INACCESSIBLE_READ;

    major_status = krb5_gss_validate_cred(minor_status, cred_handle);
    if (GSS_ERROR(major_status))
        return major_status;

    for (i = 0; i < sizeof(krb5_gssspi_set_cred_option_ops)/
                    sizeof(krb5_gssspi_set_cred_option_ops[0]); i++) {
        if (g_OID_prefix_equal(desired_object, &krb5_gssspi_set_cred_option_ops[i].oid)) {
            return (*krb5_gssspi_set_cred_option_ops[i].func)(minor_status,
                                                              cred_handle,
                                                              desired_object,
                                                              value);
        }
    }

    *minor_status = EINVAL;

    return GSS_S_UNAVAILABLE;
}

/*
 * gssspi_mech_invoke() methods
 */
static struct {
    gss_OID_desc oid;
    OM_uint32 (*func)(OM_uint32 *, const gss_OID, const gss_OID, gss_buffer_t);
} krb5_gssspi_mech_invoke_ops[] = {
    {
        {GSS_KRB5_REGISTER_ACCEPTOR_IDENTITY_OID_LENGTH, GSS_KRB5_REGISTER_ACCEPTOR_IDENTITY_OID},
        gss_krb5int_register_acceptor_identity
    },
    {
        {GSS_KRB5_CCACHE_NAME_OID_LENGTH, GSS_KRB5_CCACHE_NAME_OID},
        gss_krb5int_ccache_name
    },
    {
        {GSS_KRB5_FREE_LUCID_SEC_CONTEXT_OID_LENGTH, GSS_KRB5_FREE_LUCID_SEC_CONTEXT_OID},
        gss_krb5int_free_lucid_sec_context
    },
    {
        {GSS_KRB5_USE_KDC_CONTEXT_OID_LENGTH, GSS_KRB5_USE_KDC_CONTEXT_OID},
        krb5int_gss_use_kdc_context
    }
};

static OM_uint32
krb5_gssspi_mech_invoke (OM_uint32 *minor_status,
                         const gss_OID desired_mech,
                         const gss_OID desired_object,
                         gss_buffer_t value)
{
    size_t i;

    if (minor_status == NULL)
        return GSS_S_CALL_INACCESSIBLE_WRITE;

    *minor_status = 0;

    if (desired_mech == GSS_C_NO_OID)
        return GSS_S_BAD_MECH;

    if (desired_object == GSS_C_NO_OID)
        return GSS_S_CALL_INACCESSIBLE_READ;

    for (i = 0; i < sizeof(krb5_gssspi_mech_invoke_ops)/
                    sizeof(krb5_gssspi_mech_invoke_ops[0]); i++) {
        if (g_OID_prefix_equal(desired_object, &krb5_gssspi_mech_invoke_ops[i].oid)) {
            return (*krb5_gssspi_mech_invoke_ops[i].func)(minor_status,
                                                          desired_mech,
                                                          desired_object,
                                                          value);
        }
    }

    *minor_status = EINVAL;

    return GSS_S_UNAVAILABLE;
}

static struct gss_config krb5_mechanism = {
    { GSS_MECH_KRB5_OID_LENGTH, GSS_MECH_KRB5_OID },
    NULL,
    krb5_gss_acquire_cred,
    krb5_gss_release_cred,
    krb5_gss_init_sec_context,
#ifdef LEAN_CLIENT
    NULL,
#else
    krb5_gss_accept_sec_context,
#endif
    krb5_gss_process_context_token,
    krb5_gss_delete_sec_context,
    krb5_gss_context_time,
    krb5_gss_get_mic,
    krb5_gss_verify_mic,
#ifdef IOV_SHIM_EXERCISE
    NULL,
    NULL,
#else
    krb5_gss_wrap,
    krb5_gss_unwrap,
#endif
    krb5_gss_display_status,
    krb5_gss_indicate_mechs,
    krb5_gss_compare_name,
    krb5_gss_display_name,
    krb5_gss_import_name,
    krb5_gss_release_name,
    krb5_gss_inquire_cred,
    krb5_gss_add_cred,
#ifdef LEAN_CLIENT
    NULL,
    NULL,
#else
    krb5_gss_export_sec_context,
    krb5_gss_import_sec_context,
#endif
    krb5_gss_inquire_cred_by_mech,
    krb5_gss_inquire_names_for_mech,
    krb5_gss_inquire_context,
    krb5_gss_internal_release_oid,
    krb5_gss_wrap_size_limit,
    krb5_gss_export_name,
    NULL,                        /* store_cred */
    krb5_gss_inquire_sec_context_by_oid,
    krb5_gss_inquire_cred_by_oid,
    krb5_gss_set_sec_context_option,
    krb5_gssspi_set_cred_option,
    krb5_gssspi_mech_invoke,
    NULL,                /* wrap_aead */
    NULL,                /* unwrap_aead */
    krb5_gss_wrap_iov,
    krb5_gss_unwrap_iov,
    krb5_gss_wrap_iov_length,
    NULL,               /* complete_auth_token */
};


#ifdef _GSS_STATIC_LINK
#include "mglueP.h"
static int gss_krb5mechglue_init(void)
{
    struct gss_mech_config mech_krb5;

    memset(&mech_krb5, 0, sizeof(mech_krb5));
    mech_krb5.mech = &krb5_mechanism;
    mech_krb5.mechNameStr = "kerberos_v5";
    mech_krb5.mech_type = (gss_OID)gss_mech_krb5;

    gssint_register_mechinfo(&mech_krb5);

    mech_krb5.mechNameStr = "kerberos_v5_old";
    mech_krb5.mech_type = (gss_OID)gss_mech_krb5_old;
    gssint_register_mechinfo(&mech_krb5);

    mech_krb5.mechNameStr = "mskrb";
    mech_krb5.mech_type = (gss_OID)gss_mech_krb5_wrong;
    gssint_register_mechinfo(&mech_krb5);

    return 0;
}
#else
MAKE_INIT_FUNCTION(gss_krb5int_lib_init);
MAKE_FINI_FUNCTION(gss_krb5int_lib_fini);

gss_mechanism KRB5_CALLCONV
gss_mech_initialize(void)
{
    return &krb5_mechanism;
}
#endif /* _GSS_STATIC_LINK */

int gss_krb5int_lib_init(void)
{
    int err;

#ifdef SHOW_INITFINI_FUNCS
    printf("gss_krb5int_lib_init\n");
#endif

    add_error_table(&et_ggss_error_table);

#ifndef LEAN_CLIENT
    err = k5_mutex_finish_init(&gssint_krb5_keytab_lock);
    if (err)
        return err;
#endif /* LEAN_CLIENT */
    err = k5_key_register(K5_KEY_GSS_KRB5_SET_CCACHE_OLD_NAME, free);
    if (err)
        return err;
    err = k5_key_register(K5_KEY_GSS_KRB5_CCACHE_NAME, free);
    if (err)
        return err;
    err = k5_key_register(K5_KEY_GSS_KRB5_ERROR_MESSAGE,
                          krb5_gss_delete_error_info);
    if (err)
        return err;
#ifndef _WIN32
    err = k5_mutex_finish_init(&kg_kdc_flag_mutex);
    if (err)
        return err;
    err = k5_mutex_finish_init(&kg_vdb.mutex);
    if (err)
        return err;
#endif
#ifdef _GSS_STATIC_LINK
    err = gss_krb5mechglue_init();
    if (err)
        return err;
#endif

    return 0;
}

void gss_krb5int_lib_fini(void)
{
#ifndef _GSS_STATIC_LINK
    if (!INITIALIZER_RAN(gss_krb5int_lib_init) || PROGRAM_EXITING()) {
# ifdef SHOW_INITFINI_FUNCS
        printf("gss_krb5int_lib_fini: skipping\n");
# endif
        return;
    }
#endif
#ifdef SHOW_INITFINI_FUNCS
    printf("gss_krb5int_lib_fini\n");
#endif
    remove_error_table(&et_k5g_error_table);

    k5_key_delete(K5_KEY_GSS_KRB5_SET_CCACHE_OLD_NAME);
    k5_key_delete(K5_KEY_GSS_KRB5_CCACHE_NAME);
    k5_mutex_destroy(&kg_vdb.mutex);
#ifndef _WIN32
    k5_mutex_destroy(&kg_kdc_flag_mutex);
#endif
#ifndef LEAN_CLIENT
    k5_mutex_destroy(&gssint_krb5_keytab_lock);
#endif /* LEAN_CLIENT */
}

#ifdef _GSS_STATIC_LINK
extern OM_uint32 gssint_lib_init(void);
#endif

OM_uint32 gss_krb5int_initialize_library (void)
{
#ifdef _GSS_STATIC_LINK
    return gssint_mechglue_initialize_library();
#else
    return CALL_INIT_FUNCTION(gss_krb5int_lib_init);
#endif
}

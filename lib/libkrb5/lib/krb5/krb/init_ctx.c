/*
 * lib/krb5/krb/init_ctx.c
 *
 * Copyright 1994,1999,2000, 2002, 2003, 2007, 2008, 2009  by the Massachusetts Institute of Technology.
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
 * krb5_init_contex()
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

#include "k5-int.h"
#include <ctype.h>
#include "brand.c"
/* There has to be a better way for windows... */
#if defined(unix) || TARGET_OS_MAC
#include "../krb5_libinit.h"
#endif

/* The des-mdX entries are last for now, because it's easy to
   configure KDCs to issue TGTs with des-mdX keys and then not accept
   them.  This'll be fixed, but for better compatibility, let's prefer
   des-crc for now.  */
#define DEFAULT_ETYPE_LIST	\
	"aes256-cts-hmac-sha1-96 " \
	"aes128-cts-hmac-sha1-96 " \
	"des3-cbc-sha1 arcfour-hmac-md5 " \
	"des-cbc-crc des-cbc-md5 des-cbc-md4 "

/* Not included:
	"aes128-cts-hmac-sha1-96 " \
 */

#if (defined(_WIN32))
extern krb5_error_code krb5_vercheck();
extern void krb5_win_ccdll_load(krb5_context context);
#endif

static krb5_error_code init_common (krb5_context *, krb5_boolean, krb5_boolean);

krb5_error_code KRB5_CALLCONV
krb5_init_context(krb5_context *context)
{

	return init_common (context, FALSE, FALSE);
}

krb5_error_code KRB5_CALLCONV
krb5_init_secure_context(krb5_context *context)
{

        /* This is to make gcc -Wall happy */
        if(0) krb5_brand[0] = krb5_brand[0];
	return init_common (context, TRUE, FALSE);
}

krb5_error_code
krb5int_init_context_kdc(krb5_context *context)
{
    return init_common (context, FALSE, TRUE);
}

static krb5_error_code
init_common (krb5_context *context, krb5_boolean secure, krb5_boolean kdc)
{
	krb5_context ctx = 0;
	krb5_error_code retval;
	struct {
	    krb5_int32 now, now_usec;
	    long pid;
	} seed_data;
	krb5_data seed;
	int tmp;

	/* Verify some assumptions.  If the assumptions hold and the
	   compiler is optimizing, this should result in no code being
	   executed.  If we're guessing "unsigned long long" instead
	   of using uint64_t, the possibility does exist that we're
	   wrong.  */
	{
	    krb5_ui_8 i64;
	    assert(sizeof(i64) == 8);
	    i64 = 0, i64--, i64 >>= 62;
	    assert(i64 == 3);
	    i64 = 1, i64 <<= 31, i64 <<= 31, i64 <<= 1;
	    assert(i64 != 0);
	    i64 <<= 1;
	    assert(i64 == 0);
	}

	retval = krb5int_initialize_library();
	if (retval)
	    return retval;

#if (defined(_WIN32))
	/* 
	 * Load the krbcc32.dll if necessary.  We do this here so that
	 * we know to use API: later on during initialization.
	 * The context being NULL is ok.
	 */
	krb5_win_ccdll_load(ctx);

	/*
	 * krb5_vercheck() is defined in win_glue.c, and this is
	 * where we handle the timebomb and version server checks.
	 */
	retval = krb5_vercheck();
	if (retval)
		return retval;
#endif

	*context = 0;

	ctx = calloc(1, sizeof(struct _krb5_context));
	if (!ctx)
		return ENOMEM;
	ctx->magic = KV5M_CONTEXT;

	ctx->profile_secure = secure;

	/* Set the default encryption types, possible defined in krb5/conf */
	if ((retval = krb5_set_default_in_tkt_ktypes(ctx, NULL)))
		goto cleanup;

	if ((retval = krb5_set_default_tgs_ktypes(ctx, NULL)))
		goto cleanup;

	if ((retval = krb5_os_init_context(ctx, kdc)))
		goto cleanup;

	retval = profile_get_boolean(ctx->profile, KRB5_CONF_LIBDEFAULTS,
				     KRB5_CONF_ALLOW_WEAK_CRYPTO, NULL, 1, &tmp);
	if (retval)
		goto cleanup;
	ctx->allow_weak_crypto = tmp;

	/* initialize the prng (not well, but passable) */
	if ((retval = krb5_c_random_os_entropy( ctx, 0, NULL)) !=0)
	  goto cleanup;
	if ((retval = krb5_crypto_us_timeofday(&seed_data.now, &seed_data.now_usec)))
		goto cleanup;
	seed_data.pid = getpid ();
	seed.length = sizeof(seed_data);
	seed.data = (char *) &seed_data;
	if ((retval = krb5_c_random_add_entropy(ctx, KRB5_C_RANDSOURCE_TIMING, &seed)))
		goto cleanup;

	ctx->default_realm = 0;
	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS, KRB5_CONF_CLOCKSKEW,
			    0, 5 * 60, &tmp);
	ctx->clockskew = tmp;

#if 0
	/* Default ticket lifetime is currently not supported */
	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS, "tkt_lifetime",
			    0, 10 * 60 * 60, &tmp);
	ctx->tkt_lifetime = tmp;
#endif

	/* DCE 1.1 and below only support CKSUMTYPE_RSA_MD4 (2)  */
	/* DCE add kdc_req_checksum_type = 2 to krb5.conf */
	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS,
			    KRB5_CONF_KDC_REQ_CHECKSUM_TYPE, 0, CKSUMTYPE_RSA_MD5, 
			    &tmp);
	ctx->kdc_req_sumtype = tmp;

	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS,
			    KRB5_CONF_AP_REQ_CHECKSUM_TYPE, 0, 0,
			    &tmp);
	ctx->default_ap_req_sumtype = tmp;

	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS,
			    KRB5_CONF_SAFE_CHECKSUM_TYPE, 0,
			    CKSUMTYPE_RSA_MD5_DES, &tmp);
	ctx->default_safe_sumtype = tmp;

	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS,
			    KRB5_CONF_KDC_DEFAULT_OPTIONS, 0,
			    KDC_OPT_RENEWABLE_OK, &tmp);
	ctx->kdc_default_options = tmp;
#define DEFAULT_KDC_TIMESYNC 1
	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS,
			    KRB5_CONF_KDC_TIMESYNC, 0, DEFAULT_KDC_TIMESYNC,
			    &tmp);
	ctx->library_options = tmp ? KRB5_LIBOPT_SYNC_KDCTIME : 0;

	/*
	 * We use a default file credentials cache of 3.  See
	 * lib/krb5/krb/ccache/file/fcc.h for a description of the
	 * credentials cache types.
	 *
	 * Note: DCE 1.0.3a only supports a cache type of 1
	 * 	DCE 1.1 supports a cache type of 2.
	 */
#define DEFAULT_CCACHE_TYPE 4
	profile_get_integer(ctx->profile, KRB5_CONF_LIBDEFAULTS, KRB5_CONF_CCACHE_TYPE,
			    0, DEFAULT_CCACHE_TYPE, &tmp);
	ctx->fcc_default_format = tmp + 0x0500;
	ctx->prompt_types = 0;
	ctx->use_conf_ktypes = 0;

	ctx->udp_pref_limit = -1;
	*context = ctx;
	return 0;

cleanup:
	krb5_free_context(ctx);
	return retval;
}

void KRB5_CALLCONV
krb5_free_context(krb5_context ctx)
{
     if (ctx == NULL)
	 return;
     krb5_os_free_context(ctx);

     free(ctx->in_tkt_ktypes);
     ctx->in_tkt_ktypes = 0;
     free(ctx->tgs_ktypes);
     ctx->tgs_ktypes = 0;
     free(ctx->default_realm);
     ctx->default_realm = 0;
     if (ctx->ser_ctx_count && ctx->ser_ctx) {
	  free(ctx->ser_ctx);
	  ctx->ser_ctx = 0;
     }

     krb5_clear_error_message(ctx);

     ctx->magic = 0;
     free(ctx);
}

/*
 * Set the desired default ktypes, making sure they are valid.
 */
krb5_error_code
krb5_set_default_in_tkt_ktypes(krb5_context context, const krb5_enctype *ktypes)
{
    krb5_enctype * new_ktypes;
    int i;

    if (ktypes) {
	for (i = 0; ktypes[i]; i++) {
	    if (!krb5_c_valid_enctype(ktypes[i])) 
		return KRB5_PROG_ETYPE_NOSUPP;
	    if (!context->allow_weak_crypto && krb5_c_weak_enctype(ktypes[i]))
		return KRB5_PROG_ETYPE_NOSUPP;
	}

	/* Now copy the default ktypes into the context pointer */
	if ((new_ktypes = (krb5_enctype *)malloc(sizeof(krb5_enctype) * i)))
	    memcpy(new_ktypes, ktypes, sizeof(krb5_enctype) * i);
	else
	    return ENOMEM;

    } else {
	i = 0;
	new_ktypes = 0;
    }

    if (context->in_tkt_ktypes) 
        free(context->in_tkt_ktypes);
    context->in_tkt_ktypes = new_ktypes;
    context->in_tkt_ktype_count = i;
    return 0;
}

static krb5_error_code
get_profile_etype_list(krb5_context context, krb5_enctype **ktypes, char *profstr,
		       unsigned int ctx_count, krb5_enctype *ctx_list)
{
    krb5_enctype *old_ktypes;
    krb5_enctype ktype;

    if (ctx_count) {
	/* application-set defaults */
	if ((old_ktypes = 
	     (krb5_enctype *)malloc(sizeof(krb5_enctype) *
				    (ctx_count + 1)))) {
	    memcpy(old_ktypes, ctx_list, sizeof(krb5_enctype) * ctx_count);
	    old_ktypes[ctx_count] = 0;
	} else {
	    return ENOMEM;
	}
    } else {
        /*
	   XXX - For now, we only support libdefaults
	   Perhaps this should be extended to allow for per-host / per-realm
	   session key types.
	 */

	char *retval = NULL;
	char *sp = NULL, *ep = NULL;
	int i, j, count;
	krb5_error_code code;

	code = profile_get_string(context->profile, KRB5_CONF_LIBDEFAULTS, profstr,
				  NULL, DEFAULT_ETYPE_LIST, &retval);
	if (code)
	    return code;

	count = 0;
	sp = retval;
	while (*sp) {
	    for (ep = sp; *ep && (*ep != ',') && !isspace((int) (*ep)); ep++)
		;
	    if (*ep) {
		*ep++ = '\0';
		while (isspace((int) (*ep)) || *ep == ',')
		    *ep++ = '\0';
	    }
	    count++;
	    sp = ep;
	}
	
	if ((old_ktypes =
	     (krb5_enctype *)malloc(sizeof(krb5_enctype) * (count + 1))) ==
	    (krb5_enctype *) NULL) {
	    profile_release_string(retval);
	    return ENOMEM;
	}
	
	sp = retval;
	j = 0;
	i = 1;
	while (1) {
	    if (!krb5_string_to_enctype(sp, &ktype) &&
		(context->allow_weak_crypto || !krb5_c_weak_enctype(ktype))) {
		old_ktypes[j] = ktype;
		j++;
	    }
	    if (i++ >= count)
		break;

	    /* skip to next token */
	    while (*sp) sp++;
	    while (! *sp) sp++;
	}

	old_ktypes[j] = (krb5_enctype) 0;
	profile_release_string(retval);
    }

    if (old_ktypes[0] == 0) {
	free (old_ktypes);
	*ktypes = 0;
	return KRB5_CONFIG_ETYPE_NOSUPP;
    }

    *ktypes = old_ktypes;
    return 0;
}

krb5_error_code
krb5_get_default_in_tkt_ktypes(krb5_context context, krb5_enctype **ktypes)
{
    return(get_profile_etype_list(context, ktypes, KRB5_CONF_DEFAULT_TKT_ENCTYPES,
				  context->in_tkt_ktype_count,
				  context->in_tkt_ktypes));
}

krb5_error_code KRB5_CALLCONV
krb5_set_default_tgs_enctypes (krb5_context context, const krb5_enctype *ktypes)
{
    krb5_enctype * new_ktypes;
    int i;

    if (ktypes) {
	for (i = 0; ktypes[i]; i++) {
	    if (!krb5_c_valid_enctype(ktypes[i])) 
		return KRB5_PROG_ETYPE_NOSUPP;
	    if (!context->allow_weak_crypto && krb5_c_weak_enctype(ktypes[i]))
		return KRB5_PROG_ETYPE_NOSUPP;
	}

	/* Now copy the default ktypes into the context pointer */
	if ((new_ktypes = (krb5_enctype *)malloc(sizeof(krb5_enctype) * i)))
	    memcpy(new_ktypes, ktypes, sizeof(krb5_enctype) * i);
	else
	    return ENOMEM;

    } else {
	i = 0;
	new_ktypes = (krb5_enctype *)NULL;
    }

    if (context->tgs_ktypes) 
        krb5_free_ktypes(context, context->tgs_ktypes);
    context->tgs_ktypes = new_ktypes;
    context->tgs_ktype_count = i;
    return 0;
}

krb5_error_code krb5_set_default_tgs_ktypes
(krb5_context context, const krb5_enctype *etypes)
{
  return (krb5_set_default_tgs_enctypes (context, etypes));
}


void
KRB5_CALLCONV
krb5_free_ktypes (krb5_context context, krb5_enctype *val)
{
    free (val);
}

krb5_error_code
KRB5_CALLCONV
krb5_get_tgs_ktypes(krb5_context context, krb5_const_principal princ, krb5_enctype **ktypes)
{
    if (context->use_conf_ktypes)
	/* This one is set *only* by reading the config file; it's not
	   set by the application.  */
	return(get_profile_etype_list(context, ktypes, KRB5_CONF_DEFAULT_TKT_ENCTYPES,
				      0, NULL));
    else
	return(get_profile_etype_list(context, ktypes, KRB5_CONF_DEFAULT_TGS_ENCTYPES,
				      context->tgs_ktype_count,
				      context->tgs_ktypes));
}

krb5_error_code KRB5_CALLCONV
krb5_get_permitted_enctypes(krb5_context context, krb5_enctype **ktypes)
{
    return(get_profile_etype_list(context, ktypes, KRB5_CONF_PERMITTED_ENCTYPES,
				  context->tgs_ktype_count,
				  context->tgs_ktypes));
}

krb5_boolean
krb5_is_permitted_enctype(krb5_context context, krb5_enctype etype)
{
    krb5_enctype *list, *ptr;
    krb5_boolean ret;

    if (krb5_get_permitted_enctypes(context, &list))
	return(0);

    
    ret = 0;

    for (ptr = list; *ptr; ptr++)
	if (*ptr == etype)
	    ret = 1;

    krb5_free_ktypes (context, list);

    return(ret);
}

/* The same as krb5_is_permitted_enctype, but verifies multiple etype's
 * Returns 0 is either the list of the permitted enc types is not available
 * or all requested etypes are not permitted. Otherwise returns 1.
 */

krb5_boolean
krb5_is_permitted_enctype_ext ( krb5_context context,
                                krb5_etypes_permitted *etypes)
{
    krb5_enctype *list, *ptr;
    krb5_boolean ret = 0;
    int i = 0;

    if (krb5_get_permitted_enctypes(context, &list))
        return(0);

    for ( i=0; i< etypes->etype_count; i++ )
    {
        for (ptr = list; *ptr; ptr++)
        {
            if (*ptr == etypes->etype[i])
            {
                etypes->etype_ok[i] =  TRUE;
                ret = 1;
            }
        }
    }
    krb5_free_ktypes (context, list);

    return(ret);
}

static krb5_error_code
copy_ktypes(krb5_context ctx,
	    unsigned int nktypes,
	    krb5_enctype *oldktypes,
	    krb5_enctype **newktypes)
{
    unsigned int i;

    *newktypes = NULL;
    if (!nktypes)
	return 0;

    *newktypes = malloc(nktypes * sizeof(krb5_enctype));
    if (*newktypes == NULL)
	return ENOMEM;
    for (i = 0; i < nktypes; i++)
	(*newktypes)[i] = oldktypes[i];
    return 0;
}

krb5_error_code KRB5_CALLCONV
krb5_copy_context(krb5_context ctx, krb5_context *nctx_out)
{
    krb5_error_code ret;
    krb5_context nctx;

    *nctx_out = NULL;
    if (ctx == NULL)
	return EINVAL;		/* XXX */

    nctx = malloc(sizeof(*nctx));
    if (nctx == NULL)
	return ENOMEM;

    *nctx = *ctx;

    nctx->in_tkt_ktypes = NULL;
    nctx->in_tkt_ktype_count = 0;
    nctx->tgs_ktypes = NULL;
    nctx->tgs_ktype_count = 0;
    nctx->default_realm = NULL;
    nctx->profile = NULL;
    nctx->dal_handle = NULL;
    nctx->ser_ctx_count = 0;
    nctx->ser_ctx = NULL;
    nctx->prompt_types = NULL;
    nctx->os_context.default_ccname = NULL;

    memset(&nctx->preauth_plugins, 0, sizeof(nctx->preauth_plugins));
    nctx->preauth_context = NULL;

    memset(&nctx->libkrb5_plugins, 0, sizeof(nctx->libkrb5_plugins));
    nctx->vtbl = NULL;
    nctx->locate_fptrs = NULL;

    memset(&nctx->err, 0, sizeof(nctx->err));

    ret = copy_ktypes(nctx, ctx->in_tkt_ktype_count,
		      ctx->in_tkt_ktypes, &nctx->in_tkt_ktypes);
    if (ret)
	goto errout;
    nctx->in_tkt_ktype_count = ctx->in_tkt_ktype_count;

    ret = copy_ktypes(nctx, ctx->tgs_ktype_count,
		      ctx->tgs_ktypes, &nctx->in_tkt_ktypes);
    if (ret)
	goto errout;
    nctx->tgs_ktype_count = ctx->tgs_ktype_count;

    if (ctx->os_context.default_ccname != NULL) {
	nctx->os_context.default_ccname =
	    strdup(ctx->os_context.default_ccname);
	if (nctx->os_context.default_ccname == NULL) {
	    ret = ENOMEM;
	    goto errout;
	}
    }
    ret = krb5_get_profile(ctx, &nctx->profile);
    if (ret)
	goto errout;

errout:
    if (ret) {
	krb5_free_context(nctx);
    } else {
	*nctx_out = nctx;
    }
    return ret;
}

/*
 * include/k5-err.h
 *
 * Copyright 2006, 2007 Massachusetts Institute of Technology.
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
 * permission.	Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Error-message handling
 */

#ifndef K5_ERR_H
#define K5_ERR_H

#ifndef _
#define _(X) (X)
#endif

#if defined(_MSDOS) || defined(_WIN32)
#include <win-mac.h>
#endif
#ifndef KRB5_CALLCONV
#define KRB5_CALLCONV
#define KRB5_CALLCONV_C
#endif

#include <stdarg.h>

struct errinfo {
    long code;
    const char *msg;
    char scratch_buf[1024];
};

void
krb5int_set_error (struct errinfo *ep,
		   long code,
		   const char *fmt, ...)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 3, 4)))
#endif
    ;
void
krb5int_vset_error (struct errinfo *ep, long code,
		    const char *fmt, va_list args)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 3, 0)))
#endif
    ;
void
krb5int_set_error_fl (struct errinfo *ep, long code,
		      const char *file, int line,
		      const char *fmt, ...)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 5, 6)))
#endif
    ;
void
krb5int_vset_error_fl (struct errinfo *ep, long code,
		       const char *file, int line,
		       const char *fmt, va_list args)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 5, 0)))
#endif
    ;
const char *
krb5int_get_error (struct errinfo *ep, long code);
void
krb5int_free_error (struct errinfo *ep, const char *msg);
void
krb5int_clear_error (struct errinfo *ep);
void
krb5int_set_error_info_callout_fn (const char *(KRB5_CALLCONV *f)(long));

#ifdef DEBUG_ERROR_LOCATIONS
#define krb5int_set_error(ep, code, ...) \
    krb5int_set_error_fl(ep, code, __FILE__, __LINE__, __VA_ARGS__)
#endif

#endif /* K5_ERR_H */

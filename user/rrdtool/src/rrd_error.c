/*****************************************************************************
 * RRDtool 1.2.10  Copyright by Tobi Oetiker, 1997-2005
 *****************************************************************************
 * rrd_error.c   Common Header File
 *****************************************************************************
 * $Id: rrd_error.c 642 2005-06-17 09:05:17Z oetiker $
 * $Log$
 * Revision 1.4  2003/02/22 21:57:03  oetiker
 * a patch to avoid a memory leak and a Makefile.am patch to
 * distribute all required source files -- Peter Stamfest <peter@stamfest.at>
 *
 * Revision 1.3  2003/02/13 07:05:27  oetiker
 * Find attached the patch I promised to send to you. Please note that there
 * are three new source files (src/rrd_is_thread_safe.h, src/rrd_thread_safe.c
 * and src/rrd_not_thread_safe.c) and the introduction of librrd_th. This
 * library is identical to librrd, but it contains support code for per-thread
 * global variables currently used for error information only. This is similar
 * to how errno per-thread variables are implemented.  librrd_th must be linked
 * alongside of libpthred
 *
 * There is also a new file "THREADS", holding some documentation.
 *
 * -- Peter Stamfest <peter@stamfest.at>
 *
 * Revision 1.2  2002/02/01 20:34:49  oetiker
 * fixed version number and date/time
 *
 * Revision 1.1.1.1  2001/02/25 22:25:05  oetiker
 * checkin
 *
 *************************************************************************** */

#include "rrd_tool.h"
#include <stdarg.h>

#define MAXLEN 4096
#define ERRBUFLEN 256
#define CTX (rrd_get_context())

void
rrd_set_error(char *fmt, ...)
{
    va_list argp;
    rrd_clear_error();
    va_start(argp, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(CTX->rrd_error, CTX->len, fmt, argp);
#else
    vsprintf(CTX->rrd_error, fmt, argp);
#endif
    va_end(argp);
}

int
rrd_test_error(void) {
    return CTX->rrd_error[0] != '\0';
}

void
rrd_clear_error(void){
    CTX->rrd_error[0] = '\0';
}

char *
rrd_get_error(void){
    return CTX->rrd_error;
}

#if 0
/* PS: Keep this stuff around, maybe we want it again if we use
   rrd_contexts to really associate them with single RRD files and
   operations on them... Then a single thread may use more than one
   context. Using these functions would require to change each and
   every function containing any of the non _r versions... */
void
rrd_set_error_r(struct rrd_context *rrd_ctx, char *fmt, ...)
{
    va_list argp;
    rrd_clear_error_r(rrd_ctx);
    va_start(argp, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf((char *)rrd_ctx->rrd_error, rrd_ctx->len, fmt, argp);
#else
    vsprintf((char *)rrd_ctx->rrd_error, fmt, argp);
#endif
    va_end(argp);
}

int
rrd_test_error_r(struct rrd_context *rrd_ctx) {
    return rrd_ctx->rrd_error[0] != '\0';
}

void
rrd_clear_error_r(struct rrd_context *rrd_ctx) {
    rrd_ctx->rrd_error[0] = '\0';
}

char *
rrd_get_error_r(struct rrd_context *rrd_ctx) {
    return (char *)rrd_ctx->rrd_error;
}
#endif

/* PS: Should we move this to some other file? It is not really error
   related. */
struct rrd_context *
rrd_new_context(void) {
    struct rrd_context *rrd_ctx = 
	(struct rrd_context *) malloc(sizeof(struct rrd_context));

    if (rrd_ctx) {
	rrd_ctx->len = 0;
	rrd_ctx->rrd_error = malloc(MAXLEN);
	rrd_ctx->lib_errstr = malloc(ERRBUFLEN);
	if (rrd_ctx->rrd_error && rrd_ctx->lib_errstr) {
	    *rrd_ctx->rrd_error = 0;
	    *rrd_ctx->lib_errstr = 0;
	    rrd_ctx->len = MAXLEN;
	    rrd_ctx->errlen = ERRBUFLEN;
	    return rrd_ctx;
	}
	if (rrd_ctx->rrd_error) free(rrd_ctx->rrd_error);
	if (rrd_ctx->lib_errstr) free(rrd_ctx->lib_errstr);
	free(rrd_ctx);
    }
    return NULL;
}

void
rrd_free_context(struct rrd_context *rrd_ctx) {
    if (rrd_ctx) {
	if (rrd_ctx->rrd_error) free(rrd_ctx->rrd_error);
	if (rrd_ctx->lib_errstr) free(rrd_ctx->lib_errstr);
	free(rrd_ctx);
    }
}

#if 0
void rrd_globalize_error(struct rrd_context *rrd_ctx) {
    if (rrd_ctx) {
	rrd_set_error(rrd_ctx->rrd_error);
    }
}
#endif

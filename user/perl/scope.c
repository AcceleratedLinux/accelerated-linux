/*    scope.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 *    2000, 2001, 2002, 2003, 2004, 2005, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * "For the fashion of Minas Tirith was such that it was built on seven
 * levels..."
 */

/* This file contains functions to manipulate several of Perl's stacks;
 * in particular it contains code to push various types of things onto
 * the savestack, then to pop them off and perform the correct restorative
 * action for each one. This corresponds to the cleanup Perl does at
 * each scope exit.
 */

#include "EXTERN.h"
#define PERL_IN_SCOPE_C
#include "perl.h"

#if defined(PERL_FLEXIBLE_EXCEPTIONS)
void *
Perl_default_protect(pTHX_ volatile JMPENV *pcur_env, int *excpt,
		     protect_body_t body, ...)
{
    void *ret;
    va_list args;
    va_start(args, body);
    ret = vdefault_protect(pcur_env, excpt, body, &args);
    va_end(args);
    return ret;
}

void *
Perl_vdefault_protect(pTHX_ volatile JMPENV *pcur_env, int *excpt,
		      protect_body_t body, va_list *args)
{
    int ex;
    void *ret;

    JMPENV_PUSH(ex);
    if (ex)
	ret = NULL;
    else
	ret = CALL_FPTR(body)(aTHX_ *args);
    *excpt = ex;
    JMPENV_POP;
    return ret;
}
#endif

SV**
Perl_stack_grow(pTHX_ SV **sp, SV **p, int n)
{
    PL_stack_sp = sp;
#ifndef STRESS_REALLOC
    av_extend(PL_curstack, (p - PL_stack_base) + (n) + 128);
#else
    av_extend(PL_curstack, (p - PL_stack_base) + (n) + 1);
#endif
    return PL_stack_sp;
}

#ifndef STRESS_REALLOC
#define GROW(old) ((old) * 3 / 2)
#else
#define GROW(old) ((old) + 1)
#endif

PERL_SI *
Perl_new_stackinfo(pTHX_ I32 stitems, I32 cxitems)
{
    PERL_SI *si;
    Newx(si, 1, PERL_SI);
    si->si_stack = newAV();
    AvREAL_off(si->si_stack);
    av_extend(si->si_stack, stitems > 0 ? stitems-1 : 0);
    AvALLOC(si->si_stack)[0] = &PL_sv_undef;
    AvFILLp(si->si_stack) = 0;
    si->si_prev = 0;
    si->si_next = 0;
    si->si_cxmax = cxitems - 1;
    si->si_cxix = -1;
    si->si_type = PERLSI_UNDEF;
    Newx(si->si_cxstack, cxitems, PERL_CONTEXT);
    /* Without any kind of initialising PUSHSUBST()
     * in pp_subst() will read uninitialised heap. */
    Poison(si->si_cxstack, cxitems, PERL_CONTEXT);
    return si;
}

I32
Perl_cxinc(pTHX)
{
    const IV old_max = cxstack_max;
    cxstack_max = GROW(cxstack_max);
    Renew(cxstack, cxstack_max + 1, PERL_CONTEXT);	/* XXX should fix CXINC macro */
    /* Without any kind of initialising deep enough recursion
     * will end up reading uninitialised PERL_CONTEXTs. */
    Poison(cxstack + old_max + 1, cxstack_max - old_max, PERL_CONTEXT);
    return cxstack_ix + 1;
}

void
Perl_push_return(pTHX_ OP *retop)
{
    if (PL_retstack_ix == PL_retstack_max) {
	PL_retstack_max = GROW(PL_retstack_max);
	Renew(PL_retstack, PL_retstack_max, OP*);
    }
    PL_retstack[PL_retstack_ix++] = retop;
}

OP *
Perl_pop_return(pTHX)
{
    if (PL_retstack_ix > 0)
	return PL_retstack[--PL_retstack_ix];
    else
	return Nullop;
}

void
Perl_push_scope(pTHX)
{
    if (PL_scopestack_ix == PL_scopestack_max) {
	PL_scopestack_max = GROW(PL_scopestack_max);
	Renew(PL_scopestack, PL_scopestack_max, I32);
    }
    PL_scopestack[PL_scopestack_ix++] = PL_savestack_ix;

}

void
Perl_pop_scope(pTHX)
{
    const I32 oldsave = PL_scopestack[--PL_scopestack_ix];
    LEAVE_SCOPE(oldsave);
}

void
Perl_markstack_grow(pTHX)
{
    const I32 oldmax = PL_markstack_max - PL_markstack;
    const I32 newmax = GROW(oldmax);

    Renew(PL_markstack, newmax, I32);
    PL_markstack_ptr = PL_markstack + oldmax;
    PL_markstack_max = PL_markstack + newmax;
}

void
Perl_savestack_grow(pTHX)
{
    PL_savestack_max = GROW(PL_savestack_max) + 4;
    Renew(PL_savestack, PL_savestack_max, ANY);
}

void
Perl_savestack_grow_cnt(pTHX_ I32 need)
{
    PL_savestack_max = PL_savestack_ix + need;
    Renew(PL_savestack, PL_savestack_max, ANY);
}

#undef GROW

void
Perl_tmps_grow(pTHX_ I32 n)
{
#ifndef STRESS_REALLOC
    if (n < 128)
	n = (PL_tmps_max < 512) ? 128 : 512;
#endif
    PL_tmps_max = PL_tmps_ix + n + 1;
    Renew(PL_tmps_stack, PL_tmps_max, SV*);
}


void
Perl_free_tmps(pTHX)
{
    /* XXX should tmps_floor live in cxstack? */
    const I32 myfloor = PL_tmps_floor;
    while (PL_tmps_ix > myfloor) {      /* clean up after last statement */
	SV* const sv = PL_tmps_stack[PL_tmps_ix];
	PL_tmps_stack[PL_tmps_ix--] = Nullsv;
	if (sv && sv != &PL_sv_undef) {
	    SvTEMP_off(sv);
	    SvREFCNT_dec(sv);		/* note, can modify tmps_ix!!! */
	}
    }
}

STATIC SV *
S_save_scalar_at(pTHX_ SV **sptr)
{
    SV * const osv = *sptr;
    register SV * const sv = *sptr = NEWSV(0,0);

    if (SvTYPE(osv) >= SVt_PVMG && SvMAGIC(osv) && SvTYPE(osv) != SVt_PVGV) {
	sv_upgrade(sv, SvTYPE(osv));
	if (SvGMAGICAL(osv)) {
	    MAGIC* mg;
	    const bool oldtainted = PL_tainted;
	    mg_get(osv);		/* note, can croak! */
	    if (PL_tainting && PL_tainted &&
			(mg = mg_find(osv, PERL_MAGIC_taint))) {
		SAVESPTR(mg->mg_obj);
		mg->mg_obj = osv;
	    }
	    SvFLAGS(osv) |= (SvFLAGS(osv) &
	       (SVp_NOK|SVp_POK)) >> PRIVSHIFT;
	    PL_tainted = oldtainted;
	}
	SvMAGIC_set(sv, SvMAGIC(osv));
	SvFLAGS(sv) |= SvMAGICAL(osv);
	/* XXX SvMAGIC() is *shared* between osv and sv.  This can
	 * lead to coredumps when both SVs are destroyed without one
	 * of their SvMAGIC() slots being NULLed. */
	PL_localizing = 1;
	SvSETMAGIC(sv);
	PL_localizing = 0;
    }
    return sv;
}

SV *
Perl_save_scalar(pTHX_ GV *gv)
{
    SV **sptr = &GvSV(gv);
    SSCHECK(3);
    SSPUSHPTR(SvREFCNT_inc(gv));
    SSPUSHPTR(SvREFCNT_inc(*sptr));
    SSPUSHINT(SAVEt_SV);
    return save_scalar_at(sptr);
}

SV*
Perl_save_svref(pTHX_ SV **sptr)
{
    SSCHECK(3);
    SSPUSHPTR(sptr);
    SSPUSHPTR(SvREFCNT_inc(*sptr));
    SSPUSHINT(SAVEt_SVREF);
    return save_scalar_at(sptr);
}

/* Like save_sptr(), but also SvREFCNT_dec()s the new value.  Can be used to
 * restore a global SV to its prior contents, freeing new value. */
void
Perl_save_generic_svref(pTHX_ SV **sptr)
{
    SSCHECK(3);
    SSPUSHPTR(sptr);
    SSPUSHPTR(SvREFCNT_inc(*sptr));
    SSPUSHINT(SAVEt_GENERIC_SVREF);
}

/* Like save_pptr(), but also Safefree()s the new value if it is different
 * from the old one.  Can be used to restore a global char* to its prior
 * contents, freeing new value. */
void
Perl_save_generic_pvref(pTHX_ char **str)
{
    SSCHECK(3);
    SSPUSHPTR(str);
    SSPUSHPTR(*str);
    SSPUSHINT(SAVEt_GENERIC_PVREF);
}

/* Like save_generic_pvref(), but uses PerlMemShared_free() rather than Safefree().
 * Can be used to restore a shared global char* to its prior
 * contents, freeing new value. */
void
Perl_save_shared_pvref(pTHX_ char **str)
{
    SSCHECK(3);
    SSPUSHPTR(str);
    SSPUSHPTR(*str);
    SSPUSHINT(SAVEt_SHARED_PVREF);
}

void
Perl_save_gp(pTHX_ GV *gv, I32 empty)
{
    SSGROW(6);
    SSPUSHIV((IV)SvLEN(gv));
    SvLEN_set(gv, 0); /* forget that anything was allocated here */
    SSPUSHIV((IV)SvCUR(gv));
    SSPUSHPTR(SvPVX_const(gv));
    SvPOK_off(gv);
    SSPUSHPTR(SvREFCNT_inc(gv));
    SSPUSHPTR(GvGP(gv));
    SSPUSHINT(SAVEt_GP);

    if (empty) {
	register GP *gp;

	Newxz(gp, 1, GP);

	if (GvCVu(gv))
	    PL_sub_generation++;	/* taking a method out of circulation */
	if (GvIOp(gv) && (IoFLAGS(GvIOp(gv)) & IOf_ARGV)) {
	    gp->gp_io = newIO();
	    IoFLAGS(gp->gp_io) |= IOf_ARGV|IOf_START;
	}
	GvGP(gv) = gp_ref(gp);
	GvSV(gv) = NEWSV(72,0);
	GvLINE(gv) = CopLINE(PL_curcop);
	/* XXX Ideally this cast would be replaced with a change to const char*
	   in the struct.  */
	GvFILE(gv) = CopFILE(PL_curcop) ? CopFILE(PL_curcop) : (char *) "";
	GvEGV(gv) = gv;
    }
    else {
	gp_ref(GvGP(gv));
	GvINTRO_on(gv);
    }
}

AV *
Perl_save_ary(pTHX_ GV *gv)
{
    AV * const oav = GvAVn(gv);
    AV *av;

    if (!AvREAL(oav) && AvREIFY(oav))
	av_reify(oav);
    SSCHECK(3);
    SSPUSHPTR(gv);
    SSPUSHPTR(oav);
    SSPUSHINT(SAVEt_AV);

    GvAV(gv) = Null(AV*);
    av = GvAVn(gv);
    if (SvMAGIC(oav)) {
	SvMAGIC_set(av, SvMAGIC(oav));
	SvFLAGS((SV*)av) |= SvMAGICAL(oav);
	SvMAGICAL_off(oav);
	SvMAGIC_set(oav, NULL);
	PL_localizing = 1;
	SvSETMAGIC((SV*)av);
	PL_localizing = 0;
    }
    return av;
}

HV *
Perl_save_hash(pTHX_ GV *gv)
{
    HV *ohv, *hv;

    SSCHECK(3);
    SSPUSHPTR(gv);
    SSPUSHPTR(ohv = GvHVn(gv));
    SSPUSHINT(SAVEt_HV);

    GvHV(gv) = Null(HV*);
    hv = GvHVn(gv);
    if (SvMAGIC(ohv)) {
	SvMAGIC_set(hv, SvMAGIC(ohv));
	SvFLAGS((SV*)hv) |= SvMAGICAL(ohv);
	SvMAGICAL_off(ohv);
	SvMAGIC_set(ohv, NULL);
	PL_localizing = 1;
	SvSETMAGIC((SV*)hv);
	PL_localizing = 0;
    }
    return hv;
}

void
Perl_save_item(pTHX_ register SV *item)
{
    register SV * const sv = newSVsv(item);

    SSCHECK(3);
    SSPUSHPTR(item);		/* remember the pointer */
    SSPUSHPTR(sv);		/* remember the value */
    SSPUSHINT(SAVEt_ITEM);
}

void
Perl_save_int(pTHX_ int *intp)
{
    SSCHECK(3);
    SSPUSHINT(*intp);
    SSPUSHPTR(intp);
    SSPUSHINT(SAVEt_INT);
}

void
Perl_save_long(pTHX_ long int *longp)
{
    SSCHECK(3);
    SSPUSHLONG(*longp);
    SSPUSHPTR(longp);
    SSPUSHINT(SAVEt_LONG);
}

void
Perl_save_bool(pTHX_ bool *boolp)
{
    SSCHECK(3);
    SSPUSHBOOL(*boolp);
    SSPUSHPTR(boolp);
    SSPUSHINT(SAVEt_BOOL);
}

void
Perl_save_I32(pTHX_ I32 *intp)
{
    SSCHECK(3);
    SSPUSHINT(*intp);
    SSPUSHPTR(intp);
    SSPUSHINT(SAVEt_I32);
}

void
Perl_save_I16(pTHX_ I16 *intp)
{
    SSCHECK(3);
    SSPUSHINT(*intp);
    SSPUSHPTR(intp);
    SSPUSHINT(SAVEt_I16);
}

void
Perl_save_I8(pTHX_ I8 *bytep)
{
    SSCHECK(3);
    SSPUSHINT(*bytep);
    SSPUSHPTR(bytep);
    SSPUSHINT(SAVEt_I8);
}

void
Perl_save_iv(pTHX_ IV *ivp)
{
    SSCHECK(3);
    SSPUSHIV(*ivp);
    SSPUSHPTR(ivp);
    SSPUSHINT(SAVEt_IV);
}

/* Cannot use save_sptr() to store a char* since the SV** cast will
 * force word-alignment and we'll miss the pointer.
 */
void
Perl_save_pptr(pTHX_ char **pptr)
{
    SSCHECK(3);
    SSPUSHPTR(*pptr);
    SSPUSHPTR(pptr);
    SSPUSHINT(SAVEt_PPTR);
}

void
Perl_save_vptr(pTHX_ void *ptr)
{
    SSCHECK(3);
    SSPUSHPTR(*(char**)ptr);
    SSPUSHPTR(ptr);
    SSPUSHINT(SAVEt_VPTR);
}

void
Perl_save_sptr(pTHX_ SV **sptr)
{
    SSCHECK(3);
    SSPUSHPTR(*sptr);
    SSPUSHPTR(sptr);
    SSPUSHINT(SAVEt_SPTR);
}

void
Perl_save_padsv(pTHX_ PADOFFSET off)
{
    SSCHECK(4);
    ASSERT_CURPAD_ACTIVE("save_padsv");
    SSPUSHPTR(PL_curpad[off]);
    SSPUSHPTR(PL_comppad);
    SSPUSHLONG((long)off);
    SSPUSHINT(SAVEt_PADSV);
}

SV **
Perl_save_threadsv(pTHX_ PADOFFSET i)
{
#ifdef USE_5005THREADS
    SV **svp = &THREADSV(i);	/* XXX Change to save by offset */
    DEBUG_S(PerlIO_printf(Perl_debug_log, "save_threadsv %"UVuf": %p %p:%s\n",
			  (UV)i, svp, *svp, SvPEEK(*svp)));
    save_svref(svp);
    return svp;
#else
    Perl_croak(aTHX_ "panic: save_threadsv called in non-threaded perl");
    PERL_UNUSED_ARG(i);
    NORETURN_FUNCTION_END;
#endif /* USE_5005THREADS */
}

void
Perl_save_nogv(pTHX_ GV *gv)
{
    SSCHECK(2);
    SSPUSHPTR(gv);
    SSPUSHINT(SAVEt_NSTAB);
}

void
Perl_save_hptr(pTHX_ HV **hptr)
{
    SSCHECK(3);
    SSPUSHPTR(*hptr);
    SSPUSHPTR(hptr);
    SSPUSHINT(SAVEt_HPTR);
}

void
Perl_save_aptr(pTHX_ AV **aptr)
{
    SSCHECK(3);
    SSPUSHPTR(*aptr);
    SSPUSHPTR(aptr);
    SSPUSHINT(SAVEt_APTR);
}

void
Perl_save_freesv(pTHX_ SV *sv)
{
    SSCHECK(2);
    SSPUSHPTR(sv);
    SSPUSHINT(SAVEt_FREESV);
}

void
Perl_save_mortalizesv(pTHX_ SV *sv)
{
    SSCHECK(2);
    SSPUSHPTR(sv);
    SSPUSHINT(SAVEt_MORTALIZESV);
}

void
Perl_save_freeop(pTHX_ OP *o)
{
    SSCHECK(2);
    SSPUSHPTR(o);
    SSPUSHINT(SAVEt_FREEOP);
}

void
Perl_save_freepv(pTHX_ char *pv)
{
    SSCHECK(2);
    SSPUSHPTR(pv);
    SSPUSHINT(SAVEt_FREEPV);
}

void
Perl_save_clearsv(pTHX_ SV **svp)
{
    ASSERT_CURPAD_ACTIVE("save_clearsv");
    SSCHECK(2);
    SSPUSHLONG((long)(svp-PL_curpad));
    SSPUSHINT(SAVEt_CLEARSV);
}

void
Perl_save_delete(pTHX_ HV *hv, char *key, I32 klen)
{
    SSCHECK(4);
    SSPUSHINT(klen);
    SSPUSHPTR(key);
    SSPUSHPTR(SvREFCNT_inc(hv));
    SSPUSHINT(SAVEt_DELETE);
}

void
Perl_save_list(pTHX_ register SV **sarg, I32 maxsarg)
{
    register I32 i;

    for (i = 1; i <= maxsarg; i++) {
	register SV * const sv = NEWSV(0,0);
	sv_setsv(sv,sarg[i]);
	SSCHECK(3);
	SSPUSHPTR(sarg[i]);		/* remember the pointer */
	SSPUSHPTR(sv);			/* remember the value */
	SSPUSHINT(SAVEt_ITEM);
    }
}

void
Perl_save_destructor(pTHX_ DESTRUCTORFUNC_NOCONTEXT_t f, void* p)
{
    SSCHECK(3);
    SSPUSHDPTR(f);
    SSPUSHPTR(p);
    SSPUSHINT(SAVEt_DESTRUCTOR);
}

void
Perl_save_destructor_x(pTHX_ DESTRUCTORFUNC_t f, void* p)
{
    SSCHECK(3);
    SSPUSHDXPTR(f);
    SSPUSHPTR(p);
    SSPUSHINT(SAVEt_DESTRUCTOR_X);
}

void
Perl_save_aelem(pTHX_ AV *av, I32 idx, SV **sptr)
{
    SV *sv;
    SSCHECK(4);
    SSPUSHPTR(SvREFCNT_inc(av));
    SSPUSHINT(idx);
    SSPUSHPTR(SvREFCNT_inc(*sptr));
    SSPUSHINT(SAVEt_AELEM);
    /* if it gets reified later, the restore will have the wrong refcnt */
    if (!AvREAL(av) && AvREIFY(av))
        (void)SvREFCNT_inc(*sptr);
    save_scalar_at(sptr);
    sv = *sptr;
    /* If we're localizing a tied array element, this new sv
     * won't actually be stored in the array - so it won't get
     * reaped when the localize ends. Ensure it gets reaped by
     * mortifying it instead. DAPM */
    if (SvTIED_mg(sv, PERL_MAGIC_tiedelem))
	sv_2mortal(sv);
}

void
Perl_save_helem(pTHX_ HV *hv, SV *key, SV **sptr)
{
    SV *sv;
    SSCHECK(4);
    SSPUSHPTR(SvREFCNT_inc(hv));
    SSPUSHPTR(SvREFCNT_inc(key));
    SSPUSHPTR(SvREFCNT_inc(*sptr));
    SSPUSHINT(SAVEt_HELEM);
    save_scalar_at(sptr);
    sv = *sptr;
    /* If we're localizing a tied hash element, this new sv
     * won't actually be stored in the hash - so it won't get
     * reaped when the localize ends. Ensure it gets reaped by
     * mortifying it instead. DAPM */
    if (SvTIED_mg(sv, PERL_MAGIC_tiedelem))
	sv_2mortal(sv);
}

void
Perl_save_op(pTHX)
{
    SSCHECK(2);
    SSPUSHPTR(PL_op);
    SSPUSHINT(SAVEt_OP);
}

I32
Perl_save_alloc(pTHX_ I32 size, I32 pad)
{
    register const I32 start = pad + ((char*)&PL_savestack[PL_savestack_ix]
				- (char*)PL_savestack);
    register const I32 elems = 1 + ((size + pad - 1) / sizeof(*PL_savestack));

    /* SSCHECK may not be good enough */
    while (PL_savestack_ix + elems + 2 > PL_savestack_max)
	savestack_grow();

    PL_savestack_ix += elems;
    SSPUSHINT(elems);
    SSPUSHINT(SAVEt_ALLOC);
    return start;
}

void
Perl_leave_scope(pTHX_ I32 base)
{
    register SV *sv;
    register SV *value;
    register GV *gv;
    register AV *av;
    register HV *hv;
    register void* ptr;
    register char* str;
    I32 i;

    if (base < -1)
	Perl_croak(aTHX_ "panic: corrupt saved stack index");
    while (PL_savestack_ix > base) {
	switch (SSPOPINT) {
	case SAVEt_ITEM:			/* normal string */
	    value = (SV*)SSPOPPTR;
	    sv = (SV*)SSPOPPTR;
	    sv_replace(sv,value);
	    PL_localizing = 2;
	    SvSETMAGIC(sv);
	    PL_localizing = 0;
	    break;
	case SAVEt_SV:				/* scalar reference */
	    value = (SV*)SSPOPPTR;
	    gv = (GV*)SSPOPPTR;
	    ptr = &GvSV(gv);
	    av = (AV*)gv; /* what to refcnt_dec */
	    goto restore_sv;
	case SAVEt_GENERIC_PVREF:		/* generic pv */
	    str = (char*)SSPOPPTR;
	    ptr = SSPOPPTR;
	    if (*(char**)ptr != str) {
		Safefree(*(char**)ptr);
		*(char**)ptr = str;
	    }
	    break;
	case SAVEt_SHARED_PVREF:		/* shared pv */
	    str = (char*)SSPOPPTR;
	    ptr = SSPOPPTR;
	    if (*(char**)ptr != str) {
#ifdef NETWARE
		PerlMem_free(*(char**)ptr);
#else
		PerlMemShared_free(*(char**)ptr);
#endif
		*(char**)ptr = str;
	    }
	    break;
	case SAVEt_GENERIC_SVREF:		/* generic sv */
	    value = (SV*)SSPOPPTR;
	    ptr = SSPOPPTR;
	    sv = *(SV**)ptr;
	    *(SV**)ptr = value;
	    SvREFCNT_dec(sv);
	    SvREFCNT_dec(value);
	    break;
	case SAVEt_SVREF:			/* scalar reference */
	    value = (SV*)SSPOPPTR;
	    ptr = SSPOPPTR;
	    av = Nullav; /* what to refcnt_dec */
	restore_sv:
	    sv = *(SV**)ptr;
	    DEBUG_S(PerlIO_printf(Perl_debug_log,
				  "restore svref: %p %p:%s -> %p:%s\n",
				  ptr, sv, SvPEEK(sv), value, SvPEEK(value)));
	    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv) &&
		SvTYPE(sv) != SVt_PVGV)
	    {
		(void)SvUPGRADE(value, SvTYPE(sv));
		SvMAGIC_set(value, SvMAGIC(sv));
		SvFLAGS(value) |= SvMAGICAL(sv);
		SvMAGICAL_off(sv);
		SvMAGIC_set(sv, 0);
	    }
	    /* XXX This branch is pretty bogus.  This code irretrievably
	     * clears(!) the magic on the SV (either to avoid further
	     * croaking that might ensue when the SvSETMAGIC() below is
	     * called, or to avoid two different SVs pointing at the same
	     * SvMAGIC()).  This needs a total rethink.  --GSAR */
	    else if (SvTYPE(value) >= SVt_PVMG && SvMAGIC(value) &&
		     SvTYPE(value) != SVt_PVGV)
	    {
		SvFLAGS(value) |= (SvFLAGS(value) &
				  (SVp_NOK|SVp_POK)) >> PRIVSHIFT;
		SvMAGICAL_off(value);
		/* XXX this is a leak when we get here because the
		 * mg_get() in save_scalar_at() croaked */
		SvMAGIC_set(value, NULL);
	    }
	    *(SV**)ptr = value;
	    SvREFCNT_dec(sv);
	    PL_localizing = 2;
	    SvSETMAGIC(value);
	    PL_localizing = 0;
	    SvREFCNT_dec(value);
	    if (av) /* actually an av, hv or gv */
		SvREFCNT_dec(av);
	    break;
	case SAVEt_AV:				/* array reference */
	    av = (AV*)SSPOPPTR;
	    gv = (GV*)SSPOPPTR;
	    if (GvAV(gv)) {
		AV * const goner = GvAV(gv);
		SvMAGIC_set(av, SvMAGIC(goner));
		SvFLAGS((SV*)av) |= SvMAGICAL(goner);
		SvMAGICAL_off(goner);
		SvMAGIC_set(goner, NULL);
		SvREFCNT_dec(goner);
	    }
	    GvAV(gv) = av;
	    if (SvMAGICAL(av)) {
		PL_localizing = 2;
		SvSETMAGIC((SV*)av);
		PL_localizing = 0;
	    }
	    break;
	case SAVEt_HV:				/* hash reference */
	    hv = (HV*)SSPOPPTR;
	    gv = (GV*)SSPOPPTR;
	    if (GvHV(gv)) {
		HV * const goner = GvHV(gv);
		SvMAGIC_set(hv, SvMAGIC(goner));
		SvFLAGS(hv) |= SvMAGICAL(goner);
		SvMAGICAL_off(goner);
		SvMAGIC_set(goner, NULL);
		SvREFCNT_dec(goner);
	    }
	    GvHV(gv) = hv;
	    if (SvMAGICAL(hv)) {
		PL_localizing = 2;
		SvSETMAGIC((SV*)hv);
		PL_localizing = 0;
	    }
	    break;
	case SAVEt_INT:				/* int reference */
	    ptr = SSPOPPTR;
	    *(int*)ptr = (int)SSPOPINT;
	    break;
	case SAVEt_LONG:			/* long reference */
	    ptr = SSPOPPTR;
	    *(long*)ptr = (long)SSPOPLONG;
	    break;
	case SAVEt_BOOL:			/* bool reference */
	    ptr = SSPOPPTR;
	    *(bool*)ptr = (bool)SSPOPBOOL;
	    break;
	case SAVEt_I32:				/* I32 reference */
	    ptr = SSPOPPTR;
	    *(I32*)ptr = (I32)SSPOPINT;
	    break;
	case SAVEt_I16:				/* I16 reference */
	    ptr = SSPOPPTR;
	    *(I16*)ptr = (I16)SSPOPINT;
	    break;
	case SAVEt_I8:				/* I8 reference */
	    ptr = SSPOPPTR;
	    *(I8*)ptr = (I8)SSPOPINT;
	    break;
	case SAVEt_IV:				/* IV reference */
	    ptr = SSPOPPTR;
	    *(IV*)ptr = (IV)SSPOPIV;
	    break;
	case SAVEt_SPTR:			/* SV* reference */
	    ptr = SSPOPPTR;
	    *(SV**)ptr = (SV*)SSPOPPTR;
	    break;
	case SAVEt_VPTR:			/* random* reference */
	case SAVEt_PPTR:			/* char* reference */
	    ptr = SSPOPPTR;
	    *(char**)ptr = (char*)SSPOPPTR;
	    break;
	case SAVEt_HPTR:			/* HV* reference */
	    ptr = SSPOPPTR;
	    *(HV**)ptr = (HV*)SSPOPPTR;
	    break;
	case SAVEt_APTR:			/* AV* reference */
	    ptr = SSPOPPTR;
	    *(AV**)ptr = (AV*)SSPOPPTR;
	    break;
	case SAVEt_NSTAB:
	    gv = (GV*)SSPOPPTR;
	    (void)sv_clear((SV*)gv);
	    break;
	case SAVEt_GP:				/* scalar reference */
	    ptr = SSPOPPTR;
	    gv = (GV*)SSPOPPTR;
	    if (SvPVX_const(gv) && SvLEN(gv) > 0) {
		Safefree(SvPVX_mutable(gv));
	    }
	    SvPV_set(gv, (char *)SSPOPPTR);
	    SvCUR_set(gv, (STRLEN)SSPOPIV);
	    SvLEN_set(gv, (STRLEN)SSPOPIV);
	    gp_free(gv);
	    GvGP(gv) = (GP*)ptr;
	    if (GvCVu(gv))
		PL_sub_generation++;  /* putting a method back into circulation */
	    SvREFCNT_dec(gv);
	    break;
	case SAVEt_FREESV:
	    ptr = SSPOPPTR;
	    SvREFCNT_dec((SV*)ptr);
	    break;
	case SAVEt_MORTALIZESV:
	    ptr = SSPOPPTR;
	    sv_2mortal((SV*)ptr);
	    break;
	case SAVEt_FREEOP:
	    ptr = SSPOPPTR;
	    ASSERT_CURPAD_LEGAL("SAVEt_FREEOP"); /* XXX DAPM tmp */
	    op_free((OP*)ptr);
	    break;
	case SAVEt_FREEPV:
	    ptr = SSPOPPTR;
	    Safefree(ptr);
	    break;
	case SAVEt_CLEARSV:
	    ptr = (void*)&PL_curpad[SSPOPLONG];
	    sv = *(SV**)ptr;

	    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
	     "Pad 0x%"UVxf"[0x%"UVxf"] clearsv: %ld sv=0x%"UVxf"<%"IVdf"> %s\n",
		PTR2UV(PL_comppad), PTR2UV(PL_curpad),
		(long)((SV **)ptr-PL_curpad), PTR2UV(sv), (IV)SvREFCNT(sv),
		(SvREFCNT(sv) <= 1 && !SvOBJECT(sv)) ? "clear" : "abandon"
	    ));

	    /* Can clear pad variable in place? */
	    if (SvREFCNT(sv) <= 1 && !SvOBJECT(sv)) {
		/*
		 * if a my variable that was made readonly is going out of
		 * scope, we want to remove the readonlyness so that it can
		 * go out of scope quietly
		 */
		if (SvPADMY(sv) && !SvFAKE(sv))
		    SvREADONLY_off(sv);

		if (SvTHINKFIRST(sv))
		    sv_force_normal_flags(sv, SV_IMMEDIATE_UNREF);
		if (SvMAGICAL(sv))
		    mg_free(sv);

		switch (SvTYPE(sv)) {
		case SVt_NULL:
		    break;
		case SVt_PVAV:
		    av_clear((AV*)sv);
		    /* Need to detach $#array from @array that has just gone
		       out of scope. Otherwise the first $#array controls the
		       size of the array "newly" created the next time this
		       scope is entered.
		    */
		    if (AvARYLEN(sv)) {
			MAGIC *mg = mg_find (AvARYLEN(sv), PERL_MAGIC_arylen);

			if (mg) {
			    mg->mg_obj = 0;
			}

			SvREFCNT_dec(AvARYLEN(sv));
			AvARYLEN(sv) = 0;
		    }
		    break;
		case SVt_PVHV:
		    hv_clear((HV*)sv);
		    break;
		case SVt_PVCV:
		    Perl_croak(aTHX_ "panic: leave_scope pad code");
		default:
		    SvOK_off(sv);
		    break;
		}
	    }
	    else {	/* Someone has a claim on this, so abandon it. */
		const U32 padflags
		  = SvFLAGS(sv) & (SVs_PADBUSY|SVs_PADMY|SVs_PADTMP);
		switch (SvTYPE(sv)) {	/* Console ourselves with a new value */
		case SVt_PVAV:	*(SV**)ptr = (SV*)newAV();	break;
		case SVt_PVHV:	*(SV**)ptr = (SV*)newHV();	break;
		default:	*(SV**)ptr = NEWSV(0,0);	break;
		}
		SvREFCNT_dec(sv);	/* Cast current value to the winds. */
		SvFLAGS(*(SV**)ptr) |= padflags; /* preserve pad nature */
	    }
	    break;
	case SAVEt_DELETE:
	    ptr = SSPOPPTR;
	    hv = (HV*)ptr;
	    ptr = SSPOPPTR;
	    (void)hv_delete(hv, (char*)ptr, (U32)SSPOPINT, G_DISCARD);
	    SvREFCNT_dec(hv);
	    Safefree(ptr);
	    break;
	case SAVEt_DESTRUCTOR:
	    ptr = SSPOPPTR;
	    (*SSPOPDPTR)(ptr);
	    break;
	case SAVEt_DESTRUCTOR_X:
	    ptr = SSPOPPTR;
	    (*SSPOPDXPTR)(aTHX_ ptr);
	    break;
	case SAVEt_REGCONTEXT:
	case SAVEt_ALLOC:
	    i = SSPOPINT;
	    PL_savestack_ix -= i;  	/* regexp must have croaked */
	    break;
	case SAVEt_STACK_POS:		/* Position on Perl stack */
	    i = SSPOPINT;
	    PL_stack_sp = PL_stack_base + i;
	    break;
	case SAVEt_AELEM:		/* array element */
	    value = (SV*)SSPOPPTR;
	    i = SSPOPINT;
	    av = (AV*)SSPOPPTR;
	    if (!AvREAL(av) && AvREIFY(av)) /* undo reify guard */
		SvREFCNT_dec(value);
	    ptr = av_fetch(av,i,1);
	    if (ptr) {
		sv = *(SV**)ptr;
		if (sv && sv != &PL_sv_undef) {
		    if (SvTIED_mg((SV*)av, PERL_MAGIC_tied))
			(void)SvREFCNT_inc(sv);
		    goto restore_sv;
		}
	    }
	    SvREFCNT_dec(av);
	    SvREFCNT_dec(value);
	    break;
	case SAVEt_HELEM:		/* hash element */
	    value = (SV*)SSPOPPTR;
	    sv = (SV*)SSPOPPTR;
	    hv = (HV*)SSPOPPTR;
	    ptr = hv_fetch_ent(hv, sv, 1, 0);
	    if (ptr) {
		const SV * const oval = HeVAL((HE*)ptr);
		if (oval && oval != &PL_sv_undef) {
		    ptr = &HeVAL((HE*)ptr);
		    if (SvTIED_mg((SV*)hv, PERL_MAGIC_tied))
			(void)SvREFCNT_inc(*(SV**)ptr);
		    SvREFCNT_dec(sv);
		    av = (AV*)hv; /* what to refcnt_dec */
		    goto restore_sv;
		}
	    }
	    SvREFCNT_dec(hv);
	    SvREFCNT_dec(sv);
	    SvREFCNT_dec(value);
	    break;
	case SAVEt_OP:
	    PL_op = (OP*)SSPOPPTR;
	    break;
	case SAVEt_HINTS:
	    if ((PL_hints & HINT_LOCALIZE_HH) && GvHV(PL_hintgv)) {
		SvREFCNT_dec((SV*)GvHV(PL_hintgv));
		GvHV(PL_hintgv) = NULL;
	    }
	    *(I32*)&PL_hints = (I32)SSPOPINT;
	    if (PL_hints & HINT_LOCALIZE_HH) {
		SvREFCNT_dec((SV*)GvHV(PL_hintgv));
		GvHV(PL_hintgv) = (HV*)SSPOPPTR;
	    }
		    
	    break;
	case SAVEt_COMPPAD:
	    PL_comppad = (PAD*)SSPOPPTR;
	    if (PL_comppad)
		PL_curpad = AvARRAY(PL_comppad);
	    else
		PL_curpad = Null(SV**);
	    break;
	case SAVEt_PADSV:
	    {
		const PADOFFSET off = (PADOFFSET)SSPOPLONG;
		ptr = SSPOPPTR;
		if (ptr)
		    AvARRAY((PAD*)ptr)[off] = (SV*)SSPOPPTR;
	    }
	    break;
	case SAVEt_SAVESWITCHSTACK:
	    {
		dSP;
		AV* t = (AV*)SSPOPPTR;
		AV* f = (AV*)SSPOPPTR;
		SWITCHSTACK(t,f);
		PL_curstackinfo->si_stack = f;
	    }
	    break;
	default:
	    Perl_croak(aTHX_ "panic: leave_scope inconsistency");
	}
    }
}

void
Perl_cx_dump(pTHX_ PERL_CONTEXT *cx)
{
#ifdef DEBUGGING
    PerlIO_printf(Perl_debug_log, "CX %ld = %s\n", (long)(cx - cxstack), PL_block_type[CxTYPE(cx)]);
    if (CxTYPE(cx) != CXt_SUBST) {
	PerlIO_printf(Perl_debug_log, "BLK_OLDSP = %ld\n", (long)cx->blk_oldsp);
	PerlIO_printf(Perl_debug_log, "BLK_OLDCOP = 0x%"UVxf"\n",
		      PTR2UV(cx->blk_oldcop));
	PerlIO_printf(Perl_debug_log, "BLK_OLDMARKSP = %ld\n", (long)cx->blk_oldmarksp);
	PerlIO_printf(Perl_debug_log, "BLK_OLDSCOPESP = %ld\n", (long)cx->blk_oldscopesp);
	PerlIO_printf(Perl_debug_log, "BLK_OLDRETSP = %ld\n", (long)cx->blk_oldretsp);
	PerlIO_printf(Perl_debug_log, "BLK_OLDPM = 0x%"UVxf"\n",
		      PTR2UV(cx->blk_oldpm));
	PerlIO_printf(Perl_debug_log, "BLK_GIMME = %s\n", cx->blk_gimme ? "LIST" : "SCALAR");
    }
    switch (CxTYPE(cx)) {
    case CXt_NULL:
    case CXt_BLOCK:
	break;
    case CXt_FORMAT:
	PerlIO_printf(Perl_debug_log, "BLK_SUB.CV = 0x%"UVxf"\n",
		PTR2UV(cx->blk_sub.cv));
	PerlIO_printf(Perl_debug_log, "BLK_SUB.GV = 0x%"UVxf"\n",
		PTR2UV(cx->blk_sub.gv));
	PerlIO_printf(Perl_debug_log, "BLK_SUB.DFOUTGV = 0x%"UVxf"\n",
		PTR2UV(cx->blk_sub.dfoutgv));
	PerlIO_printf(Perl_debug_log, "BLK_SUB.HASARGS = %d\n",
		(int)cx->blk_sub.hasargs);
	break;
    case CXt_SUB:
	PerlIO_printf(Perl_debug_log, "BLK_SUB.CV = 0x%"UVxf"\n",
		PTR2UV(cx->blk_sub.cv));
	PerlIO_printf(Perl_debug_log, "BLK_SUB.OLDDEPTH = %ld\n",
		(long)cx->blk_sub.olddepth);
	PerlIO_printf(Perl_debug_log, "BLK_SUB.HASARGS = %d\n",
		(int)cx->blk_sub.hasargs);
	PerlIO_printf(Perl_debug_log, "BLK_SUB.LVAL = %d\n",
		(int)cx->blk_sub.lval);
	break;
    case CXt_EVAL:
	PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_IN_EVAL = %ld\n",
		(long)cx->blk_eval.old_in_eval);
	PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_OP_TYPE = %s (%s)\n",
		PL_op_name[cx->blk_eval.old_op_type],
		PL_op_desc[cx->blk_eval.old_op_type]);
	if (cx->blk_eval.old_namesv)
	    PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_NAME = %s\n",
			  SvPVX_const(cx->blk_eval.old_namesv));
	PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_EVAL_ROOT = 0x%"UVxf"\n",
		PTR2UV(cx->blk_eval.old_eval_root));
	break;

    case CXt_LOOP:
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.LABEL = %s\n",
		cx->blk_loop.label);
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.RESETSP = %ld\n",
		(long)cx->blk_loop.resetsp);
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.REDO_OP = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.redo_op));
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.NEXT_OP = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.next_op));
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.LAST_OP = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.last_op));
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERIX = %ld\n",
		(long)cx->blk_loop.iterix);
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERARY = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.iterary));
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERVAR = 0x%"UVxf"\n",
		PTR2UV(CxITERVAR(cx)));
	if (CxITERVAR(cx))
	    PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERSAVE = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.itersave));
	PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERLVAL = 0x%"UVxf"\n",
		PTR2UV(cx->blk_loop.iterlval));
	break;

    case CXt_SUBST:
	PerlIO_printf(Perl_debug_log, "SB_ITERS = %ld\n",
		(long)cx->sb_iters);
	PerlIO_printf(Perl_debug_log, "SB_MAXITERS = %ld\n",
		(long)cx->sb_maxiters);
	PerlIO_printf(Perl_debug_log, "SB_RFLAGS = %ld\n",
		(long)cx->sb_rflags);
	PerlIO_printf(Perl_debug_log, "SB_ONCE = %ld\n",
		(long)cx->sb_once);
	PerlIO_printf(Perl_debug_log, "SB_ORIG = %s\n",
		cx->sb_orig);
	PerlIO_printf(Perl_debug_log, "SB_DSTR = 0x%"UVxf"\n",
		PTR2UV(cx->sb_dstr));
	PerlIO_printf(Perl_debug_log, "SB_TARG = 0x%"UVxf"\n",
		PTR2UV(cx->sb_targ));
	PerlIO_printf(Perl_debug_log, "SB_S = 0x%"UVxf"\n",
		PTR2UV(cx->sb_s));
	PerlIO_printf(Perl_debug_log, "SB_M = 0x%"UVxf"\n",
		PTR2UV(cx->sb_m));
	PerlIO_printf(Perl_debug_log, "SB_STREND = 0x%"UVxf"\n",
		PTR2UV(cx->sb_strend));
	PerlIO_printf(Perl_debug_log, "SB_RXRES = 0x%"UVxf"\n",
		PTR2UV(cx->sb_rxres));
	break;
    }
#endif	/* DEBUGGING */
}

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */

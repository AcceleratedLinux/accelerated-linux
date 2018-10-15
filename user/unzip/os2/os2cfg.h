/*
  Copyright (c) 1990-2001 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------
    OS/2 specific configuration section:
  ---------------------------------------------------------------------------*/

#ifndef __os2cfg_h
#define __os2cfg_h

#ifdef MSDOS
#  include <dos.h>           /* for REGS macro (TC) or _dos_setftime (MSC) */
#  ifdef __TURBOC__          /* includes Power C */
#    include <sys/timeb.h>   /* for structure ftime */
#    ifndef __BORLANDC__     /* there appears to be a bug (?) in Borland's */
#      include <mem.h>       /*  MEM.H related to __STDC__ and far poin-   */
#    endif                   /*  ters. (dpk)  [mem.h included for memcpy]  */
#  endif
#endif /* MSDOS */

#ifdef __IBMC__
#  define S_IFMT 0xF000
#  define timezone _timezone            /* (underscore names work with    */
#  define tzset _tzset                  /*  all versions of C Set)        */
#  define PIPE_ERROR (errno == EERRSET || errno == EOS2ERR)
#endif /* __IBMC__ */

#ifdef __WATCOMC__
#  ifdef __386__
#    ifndef WATCOMC_386
#      define WATCOMC_386
#    endif
#    define __32BIT__
#    undef far
#    define far
#    undef near
#    define near

/* Get asm routines to link properly without using "__cdecl": */
#    ifndef USE_ZLIB
#      pragma aux crc32         "_*" parm caller [] value [eax] modify [eax]
#      pragma aux get_crc_table "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif /* !USE_ZLIB */
#  else /* !__386__ */
#    ifndef USE_ZLIB
#      pragma aux crc32         "_*" parm caller [] value [ax dx] \
                                      modify [ax cx dx bx]
#      pragma aux get_crc_table "_*" parm caller [] value [ax] \
                                      modify [ax cx dx bx]
#    endif /* !USE_ZLIB */
#  endif /* ?__386__ */

#  ifndef EPIPE
#    define EPIPE -1
#  endif
#  define PIPE_ERROR (errno == EPIPE)
#endif /* __WATCOMC__ */

#ifdef __EMX__
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#  define far
#endif

#ifndef __32BIT__
#  define __16BIT__
#endif

#ifdef MSDOS
#  undef MSDOS
#endif

#if defined(M_I86CM) || defined(M_I86LM)
#  define MED_MEM
#endif
#if (defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__))
#  define MED_MEM
#endif
#ifdef __16BIT__
#  ifndef MED_MEM
#    define SMALL_MEM
#  endif
#endif

#ifdef __16BIT__
# if defined(MSC) || defined(__WATCOMC__)
#   include <malloc.h>
#   define nearmalloc _nmalloc
#   define nearfree _nfree
# endif
# if defined(__TURBOC__) && defined(DYNALLOC_CRCTAB)
#   if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
#     undef DYNALLOC_CRCTAB
#   endif
# endif
# ifndef nearmalloc
#   define nearmalloc malloc
#   define nearfree free
# endif
# ifdef USE_DEFLATE64
#   if (defined(M_I86TM) || defined(M_I86SM) || defined(M_I86MM))
#     error Deflate64(tm) requires compact or large memory model
#   endif
#   if (defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__))
#     error Deflate64(tm) requires compact or large memory model
#   endif
    /* the 64k history buffer for Deflate64 must be allocated specially */
#   define MALLOC_WORK
#   define MY_ZCALLOC
# endif
#endif

/* TIMESTAMP is now supported on OS/2, so enable it by default */
#if (!defined(NOTIMESTAMP) && !defined(TIMESTAMP))
#  define TIMESTAMP
#endif

/* check that TZ environment variable is defined before using UTC times */
#if (!defined(NO_IZ_CHECK_TZ) && !defined(IZ_CHECK_TZ))
#  define IZ_CHECK_TZ
#endif

#ifndef RESTORE_ACL
#  define RESTORE_ACL
#endif

#ifndef OS2_EAS
#  define OS2_EAS    /* for -l and -v listings (list.c) */
#endif

#ifdef isupper
#  undef isupper
#endif
#ifdef tolower
#  undef tolower
#endif
#define isupper(x)   IsUpperNLS((unsigned char)(x))
#define tolower(x)   ToLowerNLS((unsigned char)(x))
#ifndef NO_STRNICMP     /* use UnZip's zstrnicmp(), because some compilers  */
#  define NO_STRNICMP   /*  don't provide a NLS-aware strnicmp() function  */
#endif

#define USETHREADID

/* handlers for OEM <--> ANSI string conversions */
#ifndef _OS2_ISO_ANSI
   /* use home-brewed conversion functions; internal charset is OEM */
#  ifdef CRTL_CP_IS_ISO
#    undef CRTL_CP_IS_ISO
#  endif
#  ifndef CRTL_CP_IS_OEM
#    define CRTL_CP_IS_OEM
#  endif
#endif

/* screen size detection */
#define SCREENWIDTH 80
#define SCREENSIZE(scrrows, scrcols)  screensize(scrrows, scrcols)
int screensize(int *tt_rows, int *tt_cols);

/* on the OS/2 console screen, line-wraps are always enabled */
#define SCREENLWRAP 1

#endif /* !__os2cfg_h */

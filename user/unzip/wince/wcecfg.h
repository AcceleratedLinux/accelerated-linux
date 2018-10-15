/*
  Copyright (c) 1990-2004 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------
    WinCE "command line tool" specific configuration section:
  ---------------------------------------------------------------------------*/

#ifndef __wcecfg_h
#define __wcecfg_h

/*---------------------------------------------------------------------------
    Standard Win32 project flags
  ---------------------------------------------------------------------------*/

#ifdef _WIN32_WCE   /* for native Windows CE, force UNICODE mode */
#ifndef UNICODE
#define UNICODE
#endif
#endif /* _WIN32_WCE */

// for UnZip, the "basic" part of the win32 api is sufficient
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG
#endif

#if defined(_NDEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

#if defined(NDEBUG) && !defined(_NDEBUG)
#define _NDEBUG
#endif

//******************************************************************************
//***** Common Info-ZIP UnZip feature/configuration flags
//******************************************************************************

// Windows CE environment allows support for UTC time stamps
#ifndef USE_EF_UT_TIME
#define USE_EF_UT_TIME
#endif

// NTFS extended attributes support is not available on Windows CE
#ifndef NO_NTSD_EAS
#define NO_NTSD_EAS
#endif

#ifdef NTSD_EAS
#undef NTSD_EAS
#endif

#ifdef POCKET_UNZIP
// The PUnZip GUI interface does not make use of ZipInfo style archive
// listings.
// For the command-line tool, it may be considered to disable ZipInfo, too,
// to save memory and resources.
# ifndef NO_ZIPINFO
# define NO_ZIPINFO
# endif
#endif

#ifndef POCKET_UNZIP
// CE does not support exception handling model
# define NO_EXCEPT_SIGNALS
# define MAIN main_stub
#endif

//******************************************************************************
//***** Global defines, constants, and macros
//******************************************************************************

#if (defined(_MSC_VER) && !defined(MSC))
#define MSC
#endif

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

// WinCE uses ISO 8859-1 as codepage for 8-bit chars
#define CRTL_CP_IS_ISO
// The functionality of ISO <--> OEM conversion IS needed on WinCE, too!!
// (Otherwise, extended ASCII filenames and passwords in archives coming
// from other platforms may not be handled properly.)
// Since WinCE does not supply ISO <--> OEM conversion, we try to fall
// back to the hardcoded tables in the generic UnZip code.

#define ASCII2OEM(c) (((c) & 0x80) ? iso2oem[(c) & 0x7f] : (c))
#if !defined(IZ_ISO2OEM_ARRAY)
#  define IZ_ISO2OEM_ARRAY
#endif
#if !defined(CRYP_USES_ISO2OEM)
#  define CRYP_USES_ISO2OEM
#endif

#define OEM_TO_INTERN(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#define INTERN_TO_ISO(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#define INTERN_TO_OEM(src, dst)  {register uch c;\
    register uch *dstp = (uch *)(dst), *srcp = (uch *)(src);\
    do {\
        c = (uch)foreign(*srcp++);\
        *dstp++ = (uch)ASCII2OEM(c);\
    } while (c != '\0');}

#if defined(UNICODE)
#  define MBSTOTSTR mbstowcs
#  define TSTRTOMBS wcstombs
#else
#  define MBSTOTSTR strncpy
#  define TSTRTOMBS strncpy
#endif


#if (defined(_MBCS) && !defined(_WIN32_WCE))
   /* MSC-specific version, _mbsinc() may not be available for other systems */
#  define PREINCSTR(ptr) (ptr = (char *)_mbsinc((const UCHAR *)(ptr)))
#  define MBSCHR(str, c) (char *)_mbschr((const UCHAR *)(str), (c))
#  define MBSRCHR(str, c) (char *)_mbsrchr((const UCHAR *)(str), (c))
#endif

/* Up to now, all versions of Microsoft C runtime libraries lack the support
 * for customized (non-US) switching rules between daylight saving time and
 * standard time in the TZ environment variable string.
 * But non-US timezone rules are correctly supported when timezone information
 * is read from the OS system settings in the Win32 registry.
 * The following work-around deletes any TZ environment setting from
 * the process environment.  This results in a fallback of the RTL time
 * handling code to the (correctly interpretable) OS system settings, read
 * from the registry.
 */
#ifdef USE_EF_UT_TIME
# if (defined(_WIN32_WCE) || defined(W32_USE_IZ_TIMEZONE))
#   define iz_w32_prepareTZenv()
# else
#   define iz_w32_prepareTZenv()        putenv("TZ=")
# endif
#endif

#ifdef DIR_END
#undef DIR_END
#define DIR_END   '\\' // ZipInfo with VC++ 4.0 requires this
#endif

// We are defining a few new error types for our code.
#define PK_EXCEPTION  500
#define PK_ABORTED    501

#ifndef DATE_FORMAT
#define DATE_FORMAT   DF_MDY
#endif
#if ((_WIN32_WCE >= 211) && !defined(POCKET_UNZIP))
# ifndef USE_FWRITE
# define USE_FWRITE
# endif
#endif
#define lenEOL        2
#define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}

#define countof(a) (sizeof(a)/sizeof(*(a)))

// The max number of retries (not including the first attempt) for entering
// a password for and encrypted file before giving up on that file.
#define MAX_PASSWORD_RETRIES 2

#ifdef POCKET_UNZIP

// Variables that we want to add to our Globals struct.
#define SYSTEM_SPECIFIC_GLOBALS  \
   char  matchname[FILNAMSIZ];   \
   int   notfirstcall;           \
   char *zipfnPtr;               \
   char *wildzipfnPtr;

#else /* !POCKET_UNZIP */

/* Static variables that we have to add to Uz_Globs: */
#define SYSTEM_SPECIFIC_GLOBALS \
    char rootpath[_MAX_PATH];\
    char *wildname, *matchname;\
    int rootlen, notfirstcall;

/* This replacement for C-RTL-supplied getch() (or similar) functionality
 * avoids leaving unabsorbed LFs in the keyboard buffer under Windows95,
 * and supports the <ALT>+[0]<digit><digit><digit> feature.
 */
int getch_win32  OF((void));

#endif /* ?POCKET_UNZIP */

//******************************************************************************
//***** Global headers
//******************************************************************************

#include <windows.h>
#ifndef TIME_ZONE_ID_INVALID
#  define TIME_ZONE_ID_INVALID  (DWORD)0xFFFFFFFFL
#endif
#include <setjmp.h>
#include <stdlib.h>
#if defined(_MBCS) && !defined(_WIN32_WCE)
#include <mbstring.h>
#endif
#include <excpt.h>
#ifndef Cdecl
#define Cdecl __cdecl
#endif
#include "wince/wince.h"     // Our WinCE specific code and our debug function.
#ifdef POCKET_UNZIP
#include "wince/resource.h"  // Our resource constants
#include "wince/punzip.rcv"  // Our version information.
#endif

#endif /* !__w32cfg_h */

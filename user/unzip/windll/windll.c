/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/* Windows Info-ZIP Unzip DLL module
 *
 * Author: Mike White
 *
 * Original: 1996
 *
 * This module has the entry points for "unzipping" a zip file.
 */

/*---------------------------------------------------------------------------

  This file is the WINDLL replacement for the generic ``main program source
  file'' unzip.c.

  See the general comments in the header part of unzip.c.

  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.
               (This software is free but NOT IN THE PUBLIC DOMAIN.  There
               are some restrictions on commercial use.)

  ---------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef __RSXNT__
#  include "../win32/rsxntwin.h"
#endif
#ifdef __BORLANDC__
#include <dir.h>
#endif
#define UNZIP_INTERNAL
#include "../unzip.h"
#include "../crypt.h"
#include "../unzvers.h"
#include "../windll/windll.h"
#include "../windll/structs.h"
#include "../consts.h"

/* Added type casts to prevent potential "type mismatch" error messages. */
#ifdef REENTRANT
#  undef __G
#  undef __G__
#  define __G                 (Uz_Globs *)pG
#  define __G__               (Uz_Globs *)pG,
#endif

HANDLE hwildZipFN;
HANDLE hInst;               /* current instance */
HANDLE hDCL;
int fNoPrinting = 0;
extern jmp_buf dll_error_return;

/* Helper function to release memory allocated by Wiz_SetOpts() */
static void FreeDllMem(__GPRO);

/* For displaying status messages and error messages */
static int UZ_EXP DllMessagePrint(zvoid *pG, uch *buf, ulg size, int flag);

#if 0 /* currently unused */
/* For displaying files extracted to the display window */
int DllDisplayPrint(zvoid *pG, uch *buf, ulg size, int flag);
#endif /* never */

/* Callback function for status report and/or user interception */
static int UZ_EXP Wiz_StatReportCB(zvoid *pG, int fnflag, ZCONST char *zfn,
                                   ZCONST char *efn, ZCONST zvoid *details);

/* Dummy sound function for those applications that don't use sound */
static void WINAPI DummySound(void);

#ifndef UNZIPLIB
/*  DLL Entry Point */

#ifdef __BORLANDC__
#pragma argsused
/* Borland seems to want DllEntryPoint instead of DllMain like MSVC */
#define DllMain DllEntryPoint
#endif
#ifdef WIN32
BOOL WINAPI DllMain( HINSTANCE hInstance,
                     DWORD dwReason,
                     LPVOID plvReserved)
#else
int FAR PASCAL LibMain( HINSTANCE hInstance,
                        WORD wDataSegment,
                        WORD wHeapSize,
                        LPSTR lpszCmdLine )
#endif
{
#ifndef WIN32
/* The startup code for the DLL initializes the local heap(if there is one)
 * with a call to LocalInit which locks the data segment.
 */

if ( wHeapSize != 0 )
   {
   UnlockData( 0 );
   }
hInst = hInstance;
return 1;   /* Indicate that the DLL was initialized successfully. */
#else
BOOL rc = TRUE;
switch( dwReason )
   {
   case DLL_PROCESS_ATTACH:
      // DLL is loaded. Do your initialization here.
      // If cannot init, set rc to FALSE.
      hInst = hInstance;
      break;

   case DLL_PROCESS_DETACH:
      // DLL is unloaded. Do your cleanup here.
      break;
   default:
      break;
   }
return rc;
#endif
}

#ifdef __BORLANDC__
#pragma argsused
#endif
int FAR PASCAL WEP ( int bSystemExit )
{
return 1;
}
#endif /* !UNZIPLIB */

/* DLL calls */

BOOL WINAPI Wiz_Init(pG, lpUserFunc)
zvoid *pG;
LPUSERFUNCTIONS lpUserFunc;
{
G.message = DllMessagePrint;
G.statreportcb = Wiz_StatReportCB;
if (lpUserFunc->sound == NULL)
   lpUserFunc->sound = DummySound;
G.lpUserFunctions = lpUserFunc;

SETLOCALE(LC_CTYPE, "");

if (!G.lpUserFunctions->print ||
    !G.lpUserFunctions->sound ||
    !G.lpUserFunctions->replace)
    return FALSE;

return TRUE;
}

/*
    ExtractOnlyNewer  = true for "update" without interaction
                        (extract only newer/new files, without queries)
    SpaceToUnderscore = true if convert space to underscore
    PromptToOverwrite = true if prompt to overwrite is wanted
    fQuiet            = quiet flag:
                         0 = all messages, 1 = few messages, 2 = no messages
    ncflag            = write to stdout if true
    ntflag            = test zip file
    nvflag            = verbose listing
    nfflag            = "freshen" (replace existing files by newer versions)
    nzflag            = display zip file comment
    ndflag            = controls (sub)directory recreation during extraction
                        0 = junk paths from filenames
                        1 = "safe" usage of paths in filenames (skip "../")
                        2 = allow also unsafe path components (dir traversal)
    noflag            = true if you are to always overwrite existing files
    naflag            = do end-of-line translation
    nZIflag           = get ZipInfo if TRUE
    C_flag            = be case insensitive if TRUE
    fPrivilege        = 1 => restore ACLs in user mode,
                        2 => try to use privileges for restoring ACLs
    lpszZipFN         = zip file name
    lpszExtractDir    = directory to extract to. This should be NULL if you
                        are extracting to the current directory.
 */

BOOL WINAPI Wiz_SetOpts(pG, lpDCL)
zvoid *pG;
LPDCL lpDCL;
{
    uO.qflag = lpDCL->fQuiet;  /* Quiet flag */
    G.pfnames = (char **)&fnames[0];    /* assign default file name vector */
    G.pxnames = (char **)&fnames[1];

    uO.jflag = (lpDCL->ndflag == 0);
    uO.ddotflag = (lpDCL->ndflag >= 2);
    uO.cflag = lpDCL->ncflag;
    uO.tflag = lpDCL->ntflag;
    uO.vflag = lpDCL->nvflag;
    uO.zflag = lpDCL->nzflag;
    uO.aflag = lpDCL->naflag;
    uO.C_flag = lpDCL->C_flag;
    uO.overwrite_all = lpDCL->noflag;
    uO.overwrite_none = !(lpDCL->noflag || lpDCL->PromptToOverwrite);
    uO.uflag = lpDCL->ExtractOnlyNewer || lpDCL->nfflag;
    uO.fflag = lpDCL->nfflag;
#ifdef WIN32
    uO.X_flag = lpDCL->fPrivilege;
#endif
    uO.sflag = lpDCL->SpaceToUnderscore; /* Translate spaces to underscores? */
    if (lpDCL->nZIflag)
      {
      uO.zipinfo_mode = TRUE;
      uO.hflag = TRUE;
      uO.lflag = 10;
      uO.qflag = 2;
      }
    else
      {
      uO.zipinfo_mode = FALSE;
      }

    G.extract_flag = (!uO.zipinfo_mode &&
                      !uO.cflag && !uO.tflag && !uO.vflag && !uO.zflag
#ifdef TIMESTAMP
                      && !uO.T_flag
#endif
                     );

    if (lpDCL->lpszExtractDir != NULL && G.extract_flag)
       {
#ifndef CRTL_CP_IS_ISO
       char *pExDirRoot = (char *)malloc(strlen(lpDCL->lpszExtractDir)+1);

       if (pExDirRoot == NULL)
           return FALSE;
       ISO_TO_INTERN(lpDCL->lpszExtractDir, pExDirRoot);
#else
#  define pExDirRoot lpDCL->lpszExtractDir
#endif
       uO.exdir = pExDirRoot;
       }
    else
       {
       uO.exdir = (char *)NULL;
       }

/* G.wildzipfn needs to be initialized so that do_wild does not wind
   up clearing out the zip file name when it returns in process.c
*/
    hwildZipFN = GlobalAlloc(GPTR, FILNAMSIZ);
    if (hwildZipFN == (HGLOBAL) NULL)
       return FALSE;

    G.wildzipfn = GlobalLock(hwildZipFN);
    lstrcpy(G.wildzipfn, lpDCL->lpszZipFN);
    _ISO_INTERN(G.wildzipfn);

    return TRUE;    /* set up was OK */
}

static void FreeDllMem(__GPRO)
{
    if (G.wildzipfn) {
        GlobalUnlock(hwildZipFN);
        G.wildzipfn = NULL;
    }
    if (hwildZipFN)
        hwildZipFN = GlobalFree(hwildZipFN);

    uO.zipinfo_mode = FALSE;
}

int WINAPI Wiz_Unzip(pG, ifnc, ifnv, xfnc, xfnv)
zvoid *pG;
int ifnc;
char **ifnv;
int xfnc;
char **xfnv;
{
int retcode, f_cnt;
#ifndef CRTL_CP_IS_ISO
char **intern_ifv = NULL, **intern_xfv = NULL;
#endif

if (ifnv == (char **)NULL && ifnc != 0)
    ifnc = 0;
else
    for (f_cnt = 0; f_cnt < ifnc; f_cnt++)
        if (ifnv[f_cnt] == (char *)NULL) {
            ifnc = f_cnt;
            break;
        }
if (xfnv == (char **)NULL && xfnc != 0)
    xfnc = 0;
else
    for (f_cnt = 0; f_cnt < xfnc; f_cnt++)
        if (xfnv[f_cnt] == (char *)NULL) {
            xfnc = f_cnt;
            break;
        }

G.process_all_files = (ifnc == 0 && xfnc == 0);         /* for speed */
G.filespecs = ifnc;
G.xfilespecs = xfnc;

if (ifnc > 0) {
#ifdef CRTL_CP_IS_ISO
    G.pfnames = ifnv;
#else /* !CRTL_CP_IS_ISO */
    unsigned bufsize = 0;

    intern_ifv = (char **)malloc((ifnc+1)*sizeof(char **));
    if (intern_ifv == (char **)NULL)
        {
        FreeDllMem(__G);
        return PK_BADERR;
        }

    for (f_cnt = ifnc; --f_cnt >= 0;)
        bufsize += strlen(ifnv[f_cnt]) + 1;
    intern_ifv[0] = (char *)malloc(bufsize);
    if (intern_ifv[0] == (char *)NULL)
        {
        free(intern_ifv);
        FreeDllMem(__G);
        return PK_BADERR;
        }

    for (f_cnt = 0; ; f_cnt++)
        {
        ISO_TO_INTERN(ifnv[f_cnt], intern_ifv[f_cnt]);
        if ((f_cnt+1) >= ifnc)
            break;
        intern_ifv[f_cnt+1] = intern_ifv[f_cnt] +
                              (strlen(intern_ifv[f_cnt]) + 1);
        }
    intern_ifv[ifnc] = (char *)NULL;
    G.pfnames = intern_ifv;
#endif /* ?CRTL_CP_IS_ISO */
    }

if (xfnc > 0) {
#ifdef CRTL_CP_IS_ISO
    G.pxnames = xfnv;
#else /* !CRTL_CP_IS_ISO */
    unsigned bufsize = 0;

    intern_xfv = (char **)malloc((xfnc+1)*sizeof(char **));
    if (intern_xfv == (char **)NULL)
        {
        if (ifnc > 0)
            {
            free(intern_ifv[0]);
            free(intern_ifv);
            }
        FreeDllMem(__G);
        return PK_BADERR;
        }

    for (f_cnt = xfnc; --f_cnt >= 0;)
        bufsize += strlen(xfnv[f_cnt]) + 1;
    intern_xfv[0] = (char *)malloc(bufsize);
    if (intern_xfv[0] == (char *)NULL)
        {
        free(intern_xfv);
        if (ifnc > 0)
            {
            free(intern_ifv[0]);
            free(intern_ifv);
            }
        FreeDllMem(__G);
        return PK_BADERR;
        }

    for (f_cnt = 0; ; f_cnt++)
        {
        ISO_TO_INTERN(xfnv[f_cnt], intern_xfv[f_cnt]);
        if ((f_cnt+1) >= xfnc)
            break;
        intern_xfv[f_cnt+1] = intern_xfv[f_cnt] +
                              (strlen(intern_xfv[f_cnt]) + 1);
        }
    intern_xfv[xfnc] = (char *)NULL;
    G.pxnames = intern_xfv;
#endif /* ?CRTL_CP_IS_ISO */
    }

/*---------------------------------------------------------------------------
    Okey dokey, we have everything we need to get started.  Let's roll.
  ---------------------------------------------------------------------------*/

retcode = setjmp(dll_error_return);
if (retcode)
   {
#ifndef CRTL_CP_IS_ISO
   if (xfnc > 0)
      {
      free(intern_xfv[0]);
      free(intern_xfv);
      }
   if (ifnc > 0)
      {
      free(intern_ifv[0]);
      free(intern_ifv);
      }
#endif
   FreeDllMem(__G);
   return PK_BADERR;
   }

retcode = process_zipfiles(__G);
#ifndef CRTL_CP_IS_ISO
if (xfnc > 0)
   {
   free(intern_xfv[0]);
   free(intern_xfv);
   }
if (ifnc > 0)
   {
   free(intern_ifv[0]);
   free(intern_ifv);
   }
#endif
FreeDllMem(__G);
return retcode;
}


int WINAPI Wiz_SingleEntryUnzip(int ifnc, char **ifnv, int xfnc, char **xfnv,
   LPDCL lpDCL, LPUSERFUNCTIONS lpUserFunc)
{
int retcode;
CONSTRUCTGLOBALS();

if (!Wiz_Init((zvoid *)&G, lpUserFunc))
   {
   DESTROYGLOBALS();
   return PK_BADERR;
   }

if (lpDCL->lpszZipFN == NULL)
   {
   /* Something has screwed up, we don't have a filename */
   DESTROYGLOBALS();
   return PK_NOZIP;
   }

if (!Wiz_SetOpts((zvoid *)&G, lpDCL))
   {
   DESTROYGLOBALS();
   return PK_MEM;
   }

#ifdef SFX
G.zipfn = lpDCL->lpszZipFN;
G.argv0 = lpDCL->lpszZipFN;
#endif

/* Here is the actual call to "unzip" the files (or whatever else you
 * are doing.)
 */
retcode = Wiz_Unzip((zvoid *)&G, ifnc, ifnv, xfnc, xfnv);

DESTROYGLOBALS();
return retcode;
}


int win_fprintf(zvoid *pG, FILE *file, unsigned int size, char far *buffer)
{
if ((file != stderr) && (file != stdout))
   {
   return write(fileno(file),(char far *)(buffer),size);
   }
if (!fNoPrinting)
   return G.lpUserFunctions->print((LPSTR)buffer, size);
return (int)size;
}

/**********************************
 * Function DllMessagePrint()     *
 *                                *
 * Send messages to status window *
 **********************************/
#ifdef __BORLANDC__
#pragma argsused
#endif
static int UZ_EXP DllMessagePrint(pG, buf, size, flag)
    zvoid *pG;      /* globals struct:  always passed */
    uch *buf;       /* preformatted string to be printed */
    ulg size;       /* length of string (may include nulls) */
    int flag;       /* flag bits */
{
if (!fNoPrinting)
   return G.lpUserFunctions->print((LPSTR)buf, size);
else
   return (int)size;
}

#if 0 /* currently unused */
/********************************
 * Function DllDisplayPrint()   *
 *                              *
 * Send files to display window *
 ********************************/
#ifdef __BORLANDC__
#pragma argsused
#endif
int DllDisplayPrint(pG, buf, size, flag)
    zvoid *pG;      /* globals struct:  always passed */
    uch *buf;       /* preformatted string to be printed */
    ulg size;       /* length of string (may include nulls) */
    int flag;       /* flag bits */
{
return (!fNoPrinting ? G.lpUserFunctions->print((LPSTR)buf, size) : (int)size);
}
#endif /* never */


/**********************************
 * Function UzpPassword()         *
 *                                *
 * Prompt for decryption password *
 **********************************/
#ifdef __BORLANDC__
#pragma argsused
#endif
int UZ_EXP UzpPassword(pG, rcnt, pwbuf, size, zfn, efn)
    zvoid *pG;          /* globals struct: always passed */
    int *rcnt;          /* retry counter */
    char *pwbuf;        /* buffer for password */
    int size;           /* size of password buffer */
    ZCONST char *zfn;   /* name of zip archiv */
    ZCONST char *efn;   /* name of archiv entry being processed */
{
#if CRYPT
    LPSTR m;

    if (*rcnt == 0) {
        *rcnt = 2;
        m = "Enter password for: ";
    } else {
        (*rcnt)--;
        m = "Password incorrect--reenter: ";
    }

    return (*G.lpUserFunctions->password)((LPSTR)pwbuf, size, m, (LPSTR)efn);
#else /* !CRYPT */
    return IZ_PW_ERROR; /* internal error, function should never get called */
#endif /* ?CRYPT */
} /* end function UzpPassword() */

/* Turn off all messages to the calling application */
void WINAPI Wiz_NoPrinting(int f)
{
fNoPrinting = f;
}

/* Dummy sound function for those applications that don't use sound */
static void WINAPI DummySound(void)
{
}

/* Interface between WINDLL specific service callback functions and the
   generic DLL's "status report & user interception" callback */
#ifdef __BORLANDC__
#pragma argsused
#endif
static int WINAPI Wiz_StatReportCB(zvoid *pG, int fnflag, ZCONST char *zfn,
                    ZCONST char *efn, ZCONST zvoid *details)
{
    int rval = UZ_ST_CONTINUE;

    switch (fnflag) {
      case UZ_ST_START_EXTRACT:
        if (G.lpUserFunctions->sound != NULL)
            (*G.lpUserFunctions->sound)();
        break;
      case UZ_ST_FINISH_MEMBER:
        if ((G.lpUserFunctions->ServCallBk != NULL) &&
            (*G.lpUserFunctions->ServCallBk)(efn,
                                             (details == NULL ? 0L :
                                              *((unsigned long *)details))))
            rval = UZ_ST_BREAK;
        break;
      case UZ_ST_IN_PROGRESS:
        break;
      default:
        break;
    }
    return rval;
}


#ifndef SFX
#ifndef __16BIT__

int WINAPI Wiz_UnzipToMemory(LPSTR zip, LPSTR file,
    LPUSERFUNCTIONS lpUserFunctions, UzpBuffer *retstr)
{
    int r;
#ifndef CRTL_CP_IS_ISO
    char *intern_zip, *intern_file;
#endif

    CONSTRUCTGLOBALS();
#ifndef CRTL_CP_IS_ISO
    intern_zip = (char *)malloc(strlen(zip)+1);
    if (intern_zip == NULL) {
       DESTROYGLOBALS();
       return PK_MEM;
    }
    intern_file = (char *)malloc(strlen(file)+1);
    if (intern_file == NULL) {
       DESTROYGLOBALS();
       free(intern_zip);
       return PK_MEM;
    }
    ISO_TO_INTERN(zip, intern_zip);
    ISO_TO_INTERN(file, intern_file);
#   define zip intern_zip
#   define file intern_file
#endif
    if (!Wiz_Init((zvoid *)&G, lpUserFunctions)) {
       DESTROYGLOBALS();
       return PK_BADERR;
    }
    G.redirect_data = 1;

    r = (unzipToMemory(__G__ zip, file, retstr) == PK_COOL);

    DESTROYGLOBALS();
#ifndef CRTL_CP_IS_ISO
#  undef file
#  undef zip
    free(intern_file);
    free(intern_zip);
#endif
    if (!r && retstr->strlength) {
       free(retstr->strptr);
       retstr->strptr = NULL;
    }
    return r;
}




/* Purpose: Determine if file in archive contains the string szSearch

   Parameters: archive  = archive name
               file     = file contained in the archive. This cannot be
                          a wild card to be meaningful
               pattern  = string to search for
               cmd      = 0 - case-insensitive search
                          1 - case-sensitve search
                          2 - case-insensitive, whole words only
                          3 - case-sensitive, whole words only
               SkipBin  = if true, skip any files that have control
                          characters other than CR, LF, or tab in the first
                          100 characters.

   Returns:    TRUE if a match is found
               FALSE if no match is found
               -1 on error

   Comments: This does not pretend to be as useful as the standard
             Unix grep, which returns the strings associated with a
             particular pattern, nor does it search past the first
             matching occurrence of the pattern.
 */

int WINAPI Wiz_Grep(LPSTR archive, LPSTR file, LPSTR pattern, int cmd,
                   int SkipBin, LPUSERFUNCTIONS lpUserFunctions)
{
    int retcode = FALSE, compare;
    ulg i, j, patternLen, buflen;
    char * sz, *p;
    UzpBuffer retstr;

    /* Turn off any windows printing functions, as they may not have been
     * identified yet. There is no requirement that we initialize the
     * dll with printing stuff for this. */
    Wiz_NoPrinting(TRUE);

    if (!Wiz_UnzipToMemory(archive, file, lpUserFunctions, &retstr)) {
       Wiz_NoPrinting(FALSE);
       return -1;   /* not enough memory, file not found, or other error */
    }

    if (SkipBin) {
        if (retstr.strlength < 100)
            buflen = retstr.strlength;
        else
            buflen = 100;
        for (i = 0; i < buflen; i++) {
            if (iscntrl(retstr.strptr[i])) {
                if ((retstr.strptr[i] != 0x0A) &&
                    (retstr.strptr[i] != 0x0D) &&
                    (retstr.strptr[i] != 0x09))
                {
                    /* OK, we now think we have a binary file of some sort */
                    free(retstr.strptr);
                    Wiz_NoPrinting(FALSE);
                    return FALSE;
                }
            }
        }
    }

    patternLen = strlen(pattern);

    if (retstr.strlength < patternLen) {
        free(retstr.strptr);
        Wiz_NoPrinting(FALSE);
        return FALSE;
    }

    sz = malloc(patternLen + 3); /* add two in case doing whole words only */
    if (cmd > 1) {
        strcpy(sz, " ");
        strcat(sz, pattern);
        strcat(sz, " ");
    } else
        strcpy(sz, pattern);

    if ((cmd == 0) || (cmd == 2)) {
        for (i = 0; i < strlen(sz); i++)
            sz[i] = toupper(sz[i]);
        for (i = 0; i < retstr.strlength; i++)
            retstr.strptr[i] = toupper(retstr.strptr[i]);
    }

    for (i = 0; i < (retstr.strlength - patternLen); i++) {
        p = &retstr.strptr[i];
        compare = TRUE;
        for (j = 0; j < patternLen; j++) {
            /* We cannot do strncmp here, as we may be dealing with a
             * "binary" file, such as a word processing file, or perhaps
             * even a true executable of some sort. */
            if (p[j] != sz[j]) {
                compare = FALSE;
                break;
            }
        }
        if (compare == TRUE) {
            retcode = TRUE;
            break;
        }
    }

    free(sz);
    free(retstr.strptr);
    Wiz_NoPrinting(FALSE); /* Turn printing back on */
    return retcode;
}
#endif /* !__16BIT__ */

int WINAPI Wiz_Validate(LPSTR archive, int AllCodes)
{
    return UzpValidate((char *)archive, AllCodes);
}

#endif /* !SFX */

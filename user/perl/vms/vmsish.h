/*  vmsish.h
 *
 * VMS-specific C header file for perl5.
 *
 * Last revised: 16-Sep-1998 by Charles Bailey  bailey@newman.upenn.edu
 * Version: 5.5.2
 */

#ifndef __vmsish_h_included
#define __vmsish_h_included

#include <descrip.h> /* for dirent struct definitions */
#include <libdef.h>  /* status codes for various places */
#include <rmsdef.h>  /* at which errno and vaxc$errno are */
#include <ssdef.h>   /* explicitly set in the perl source code */
#include <stsdef.h>  /* bitmasks for exit status testing */

/* Suppress compiler warnings from DECC for VMS-specific extensions:
 * ADDRCONSTEXT,NEEDCONSTEXT: initialization of data with non-constant values
 *                            (e.g. pointer fields of descriptors)
 */
#if defined(__DECC) || defined(__DECCXX)
#  pragma message disable (ADDRCONSTEXT,NEEDCONSTEXT)
#endif

/* DEC's C compilers and gcc use incompatible definitions of _to(upp|low)er() */
#ifdef _toupper
#  undef _toupper
#endif
#define _toupper(c) (((c) < 'a' || (c) > 'z') ? (c) : (c) & ~040)
#ifdef _tolower
#  undef _tolower
#endif
#define _tolower(c) (((c) < 'A' || (c) > 'Z') ? (c) : (c) | 040)
/* DECC 1.3 has a funny definition of abs; it's fixed in DECC 4.0, so this
 * can go away once DECC 1.3 isn't in use any more. */
#if defined(__ALPHA) && (defined(__DECC) || defined(__DECCXX))
#undef abs
#define abs(__x)        __ABS(__x)
#undef labs
#define labs(__x)        __LABS(__x)
#endif /* __ALPHA && __DECC */

/* Assorted things to look like Unix */
#ifdef __GNUC__
#ifndef _IOLBF /* gcc's stdio.h doesn't define this */
#define _IOLBF 1
#endif
#endif
#include <processes.h> /* for vfork() */
#include <unixio.h>
#include <unixlib.h>
#include <file.h>  /* it's not <sys/file.h>, so don't use I_SYS_FILE */
#if (defined(__DECC) && defined(__DECC_VER) && __DECC_VER > 20000000) || defined(__DECCXX)
#  include <unistd.h> /* DECC has this; gcc doesn't */
#endif

#ifdef NO_PERL_TYPEDEFS /* a2p; we don't want Perl's special routines */
#  define DONT_MASK_RTL_CALLS
#endif

/* Note that we do, in fact, have this */
#define HAS_GETENV_SV
#define HAS_GETENV_LEN

/* All this stiff is for the x2P programs. Hopefully they'll still work */
#if defined(PERL_FOR_X2P)
#ifndef aTHX_
#define aTHX_
#endif
#ifndef pTHX_
#define pTHX_
#endif
#ifndef pTHX
#define pTHX
#endif
#endif

#ifndef DONT_MASK_RTL_CALLS
#  ifdef getenv
#    undef getenv
#  endif
  /* getenv used for regular logical names */
#  define getenv(v) Perl_my_getenv(aTHX_ v,TRUE)
#endif
#ifdef getenv_len
#  undef getenv_len
#endif
#define getenv_len(v,l) Perl_my_getenv_len(aTHX_ v,l,TRUE)

/* DECC introduces this routine in the RTL as of VMS 7.0; for now,
 * we'll use ours, since it gives us the full VMS exit status. */
#define waitpid my_waitpid

/* Don't redeclare standard RTL routines in Perl's header files;
 * VMS history or extensions makes some of the formal protoypes
 * differ from the common Unix forms.
 */
#define DONT_DECLARE_STD 1

/* Our own contribution to PerlShr's global symbols . . . */
#define prime_env_iter	Perl_prime_env_iter
#define vms_image_init	Perl_vms_image_init
#define my_tmpfile		Perl_my_tmpfile
#define vmstrnenv           	Perl_vmstrnenv            
#if !defined(PERL_IMPLICIT_CONTEXT)
#define my_getenv_len		Perl_my_getenv_len
#define vmssetenv		Perl_vmssetenv
#define my_trnlnm		Perl_my_trnlnm
#define my_setenv		Perl_my_setenv
#define my_getenv		Perl_my_getenv
#define tounixspec		Perl_tounixspec
#define tounixspec_ts		Perl_tounixspec_ts
#define tovmsspec		Perl_tovmsspec
#define tovmsspec_ts		Perl_tovmsspec_ts
#define tounixpath		Perl_tounixpath
#define tounixpath_ts		Perl_tounixpath_ts
#define tovmspath		Perl_tovmspath
#define tovmspath_ts		Perl_tovmspath_ts
#define do_rmdir		Perl_do_rmdir
#define fileify_dirspec		Perl_fileify_dirspec
#define fileify_dirspec_ts	Perl_fileify_dirspec_ts
#define pathify_dirspec		Perl_pathify_dirspec
#define pathify_dirspec_ts	Perl_pathify_dirspec_ts
#define trim_unixpath		Perl_trim_unixpath
#define opendir			Perl_opendir
#define rmscopy			Perl_rmscopy
#define my_mkdir		Perl_my_mkdir
#define vms_do_aexec		Perl_vms_do_aexec
#define vms_do_exec		Perl_vms_do_exec
#define my_waitpid		Perl_my_waitpid
#define my_crypt		Perl_my_crypt
#define kill_file		Perl_kill_file
#define my_utime		Perl_my_utime
#define my_chdir		Perl_my_chdir
#define do_aspawn		Perl_do_aspawn
#define seekdir		Perl_seekdir
#define my_gmtime		Perl_my_gmtime
#define my_localtime		Perl_my_localtime
#define my_time		Perl_my_time
#define do_spawn		Perl_do_spawn
#define flex_fstat		Perl_flex_fstat
#define flex_stat		Perl_flex_stat
#define cando_by_name		Perl_cando_by_name
#define my_getpwnam		Perl_my_getpwnam
#define my_getpwuid		Perl_my_getpwuid
#define my_flush		Perl_my_flush
#define readdir			Perl_readdir
#define readdir_r		Perl_readdir_r
#else
#define my_getenv_len(a,b,c)	Perl_my_getenv_len(aTHX_ a,b,c)
#define vmssetenv(a,b,c)	Perl_vmssetenv(aTHX_ a,b,c)
#define my_trnlnm(a,b,c)	Perl_my_trnlnm(aTHX_ a,b,c)
#define my_setenv(a,b)		Perl_my_setenv(aTHX_ a,b)
#define my_getenv(a,b)		Perl_my_getenv(aTHX_ a,b)
#define tounixspec(a,b)		Perl_tounixspec(aTHX_ a,b)
#define tounixspec_ts(a,b)	Perl_tounixspec_ts(aTHX_ a,b)
#define tovmsspec(a,b)		Perl_tovmsspec(aTHX_ a,b)
#define tovmsspec_t(a,b)	Perl_tovmsspec_ts(aTHX_ a,b)
#define tounixpath(a,b)		Perl_tounixpath(aTHX_ a,b)
#define tounixpath_ts(a,b)	Perl_tounixpath_ts(aTHX_ a,b)
#define tovmspath(a,b)		Perl_tovmspath(aTHX_ a,b)
#define tovmspath_ts(a,b)	Perl_tovmspath_ts(aTHX_ a,b)
#define do_rmdir(a)		Perl_do_rmdir(aTHX_ a)
#define fileify_dirspec(a,b)	Perl_fileify_dirspec(aTHX_ a,b)
#define fileify_dirspec_ts(a,b)	Perl_fileify_dirspec_ts(aTHX_ a,b)
#define pathify_dirspec		Perl_pathify_dirspec
#define pathify_dirspec_ts	Perl_pathify_dirspec_ts
#define rmsexpand(a,b,c,d)	Perl_rmsexpand(aTHX_ a,b,c,d)
#define rmsexpand_ts(a,b,c,d)	Perl_rmsexpand_ts(aTHX_ a,b,c,d)
#define trim_unixpath(a,b,c)	Perl_trim_unixpath(aTHX_ a,b,c)
#define opendir(a)		Perl_opendir(aTHX_ a)
#define rmscopy(a,b,c)		Perl_rmscopy(aTHX_ a,b,c)
#define my_mkdir(a,b)		Perl_my_mkdir(aTHX_ a,b)
#define vms_do_aexec(a,b,c)	Perl_vms_do_aexec(aTHX_ a,b,c)
#define vms_do_exec(a)		Perl_vms_do_exec(aTHX_ a)
#define my_waitpid(a,b,c)	Perl_my_waitpid(aTHX_ a,b,c)
#define my_crypt(a,b)		Perl_my_crypt(aTHX_ a,b)
#define kill_file(a)		Perl_kill_file(aTHX_ a)
#define my_utime(a,b)		Perl_my_utime(aTHX_ a,b)
#define my_chdir(a)		Perl_my_chdir(aTHX_ a)
#define do_aspawn(a,b,c)	Perl_do_aspawn(aTHX_ a,b,c)
#define seekdir(a,b)		Perl_seekdir(aTHX_ a,b)
#define my_gmtime(a)		Perl_my_gmtime(aTHX_ a)
#define my_localtime(a)		Perl_my_localtime(aTHX_ a)
#define my_time(a)		Perl_my_time(aTHX_ a)
#define do_spawn(a)		Perl_do_spawn(aTHX_ a)
#define flex_fstat(a,b)		Perl_flex_fstat(aTHX_ a,b)
#define cando_by_name(a,b,c)	Perl_cando_by_name(aTHX_ a,b,c)
#define flex_stat(a,b)		Perl_flex_stat(aTHX_ a,b)
#define my_getpwnam(a)		Perl_my_getpwnam(aTHX_ a)
#define my_getpwuid(a)		Perl_my_getpwuid(aTHX_ a)
#define my_flush(a)		Perl_my_flush(aTHX_ a)
#define readdir(a)		Perl_readdir(aTHX_ a)
#define readdir_r(a,b,c)	Perl_readdir_r(aTHX_ a,b,c)
#endif
#define my_gconvert		Perl_my_gconvert
#define telldir		Perl_telldir
#define closedir		Perl_closedir
#define vmsreaddirversions	Perl_vmsreaddirversions
#define my_sigemptyset        Perl_my_sigemptyset
#define my_sigfillset         Perl_my_sigfillset
#define my_sigaddset          Perl_my_sigaddset
#define my_sigdelset          Perl_my_sigdelset
#define my_sigismember        Perl_my_sigismember
#define my_sigprocmask        Perl_my_sigprocmask
#define my_vfork		Perl_my_vfork
#define my_fdopen               Perl_my_fdopen
#define my_fclose               Perl_my_fclose
#define my_fwrite		Perl_my_fwrite
#define my_getpwent()		Perl_my_getpwent(aTHX)
#define my_endpwent()		Perl_my_endpwent(aTHX)
#define my_getlogin		Perl_my_getlogin
#define init_os_extras	Perl_init_os_extras

/* Delete if at all possible, changing protections if necessary. */
#define unlink kill_file

/* 
 * Intercept calls to fork, so we know whether subsequent calls to
 * exec should be handled in VMSish or Unixish style.
 */
#define fork my_vfork
#ifndef DONT_MASK_RTL_CALLS     /* #defined in vms.c so we see real vfork */
#  ifdef vfork
#    undef vfork
#  endif
#  define vfork my_vfork
#endif

/*
 * Toss in a shim to tmpfile which creates a plain temp file if the
 * RMS tmp mechanism won't work (e.g. if someone is relying on ACLs
 * from a specific directory to permit creation of files).
 */
#ifndef DONT_MASK_RTL_CALLS
#  define tmpfile Perl_my_tmpfile
#endif


/* BIG_TIME:
 *	This symbol is defined if Time_t is an unsigned type on this system.
 */
#define BIG_TIME

/* ACME_MESS:
 *	This symbol, if defined, indicates that error messages should be 
 *	should be generated in a format that allows the use of the Acme
 *	GUI/editor's autofind feature.
 */
#undef ACME_MESS	/**/

/* ALTERNATE_SHEBANG:
 *	This symbol, if defined, contains a "magic" string which may be used
 *	as the first line of a Perl program designed to be executed directly
 *	by name, instead of the standard Unix #!.  If ALTERNATE_SHEBANG
 *	begins with a character other then #, then Perl will only treat
 *	it as a command line if if finds the string "perl" in the first
 *	word; otherwise it's treated as the first line of code in the script.
 *	(IOW, Perl won't hand off to another interpreter via an alternate
 *	shebang sequence that might be legal Perl code.)
 */
#define ALTERNATE_SHEBANG "$"

/* Lower case entry points for these are missing in some earlier RTLs 
 * so we borrow the defines and declares from errno.h and upcase them.
 */
#if defined(VMS_WE_ARE_CASE_SENSITIVE) && (__DECC_VER < 50500000)
#  define errno      (*CMA$TIS_ERRNO_GET_ADDR())
#  define vaxc$errno (*CMA$TIS_VMSERRNO_GET_ADDR())
   int *CMA$TIS_ERRNO_GET_ADDR     (void);   /* UNIX style error code        */
   int *CMA$TIS_VMSERRNO_GET_ADDR  (void);   /* VMS error (errno == EVMSERR) */
#endif

/* Macros to set errno using the VAX thread-safe calls, if present */
#if (defined(__DECC) || defined(__DECCXX)) && !defined(__ALPHA)
#  define set_errno(v)      (cma$tis_errno_set_value(v))
   void cma$tis_errno_set_value(int __value);  /* missing in some errno.h */
#  define set_vaxc_errno(v) (vaxc$errno = (v))
#else
#  define set_errno(v)      (errno = (v))
#  define set_vaxc_errno(v) (vaxc$errno = (v))
#endif

/* Support for 'vmsish' behaviors enabled with C<use vmsish> pragma */

#define COMPLEX_STATUS	1	/* We track both "POSIX" and VMS values */

#define HINT_V_VMSISH		24
#define HINT_M_VMSISH_STATUS	0x40000000 /* system, $? return VMS status */
#define HINT_M_VMSISH_TIME	0x80000000 /* times are local, not UTC */
#define NATIVE_HINTS		(PL_hints >> HINT_V_VMSISH)  /* used in op.c */

#define TEST_VMSISH(h)	(PL_curcop->op_private & ((h) >> HINT_V_VMSISH))
#define VMSISH_STATUS	TEST_VMSISH(HINT_M_VMSISH_STATUS)
#define VMSISH_TIME	TEST_VMSISH(HINT_M_VMSISH_TIME)

/* VMS-specific data storage */

#define HAVE_INTERP_INTERN
struct interp_intern {
    int    hushed;
    double inv_rand_max;
};
#define VMSISH_HUSHED     (PL_sys_intern.hushed)
#define MY_INV_RAND_MAX   (PL_sys_intern.inv_rand_max)

/* Flags for vmstrnenv() */
#define PERL__TRNENV_SECURE 0x01
#define PERL__TRNENV_JOIN_SEARCHLIST 0x02

/* Handy way to vet calls to VMS system services and RTL routines. */
#define _ckvmssts(call) STMT_START { register unsigned long int __ckvms_sts; \
  if (!((__ckvms_sts=(call))&1)) { \
  set_errno(EVMSERR); set_vaxc_errno(__ckvms_sts); \
  Perl_croak(aTHX_ "Fatal VMS error (status=%d) at %s, line %d", \
  __ckvms_sts,__FILE__,__LINE__); } } STMT_END

/* Same thing, but don't call back to Perl's croak(); useful for errors
 * occurring during startup, before Perl's state is initialized */
#define _ckvmssts_noperl(call) STMT_START { register unsigned long int __ckvms_sts; \
  if (!((__ckvms_sts=(call))&1)) { \
  set_errno(EVMSERR); set_vaxc_errno(__ckvms_sts); \
  fprintf(stderr,"Fatal VMS error (status=%d) at %s, line %d", \
  __ckvms_sts,__FILE__,__LINE__); lib$signal(__ckvms_sts); } } STMT_END

#ifdef VMS_DO_SOCKETS
#include "sockadapt.h"
#define PERL_SOCK_SYSREAD_IS_RECV
#define PERL_SOCK_SYSWRITE_IS_SEND
#endif

#define BIT_BUCKET "_NLA0:"
#define PERL_SYS_INIT(c,v)	MALLOC_CHECK_TAINT2(*c,*v) vms_image_init((c),(v)); MALLOC_INIT
#define PERL_SYS_TERM()		OP_REFCNT_TERM; MALLOC_TERM
#define dXSUB_SYS
#define HAS_KILL
#define HAS_WAIT

#define PERL_FS_VER_FMT		"%d_%d_%d"
/* Temporary; we need to add support for this to Configure.Com */
#ifdef PERL_INC_VERSION_LIST
#  undef PERL_INC_VERSION_LIST
#endif

/* VMS:
 *	This symbol, if defined, indicates that the program is running under
 *	VMS.  It's a symbol automagically defined by all VMS C compilers I've seen.
 * Just in case, however . . . */
#ifndef VMS
#define VMS		/**/
#endif

/* HAS_IOCTL:
 *	This symbol, if defined, indicates that the ioctl() routine is
 *	available to set I/O characteristics
 */
#if defined(__CRTL_VER) && __CRTL_VER >= 70000000
#define	HAS_IOCTL		/**/
#else
#undef	HAS_IOCTL		/**/
#endif
 
/* HAS_UTIME:
 *	This symbol, if defined, indicates that the routine utime() is
 *	available to update the access and modification times of files.
 */
#define HAS_UTIME		/**/

/* HAS_GROUP
 *	This symbol, if defined, indicates that the getgrnam() and
 *	getgrgid() routines are available to get group entries.
 *	The getgrent() has a separate definition, HAS_GETGRENT.
 */
#undef HAS_GROUP		/**/

/* HAS_PASSWD
 *	This symbol, if defined, indicates that the getpwnam() and
 *	getpwuid() routines are available to get password entries.
 *	The getpwent() has a separate definition, HAS_GETPWENT.
 */
#define HAS_PASSWD		/**/

#define HAS_KILL
#define HAS_WAIT
  
/* USEMYBINMODE
 *	This symbol, if defined, indicates that the program should
 *	use the routine my_binmode(FILE *fp, char iotype, int mode) to insure
 *	that a file is in "binary" mode -- that is, that no translation
 *	of bytes occurs on read or write operations.
 */
#undef USEMYBINMODE

/* Stat_t:
 *	This symbol holds the type used to declare buffers for information
 *	returned by stat().  It's usually just struct stat.  It may be necessary
 *	to include <sys/stat.h> and <sys/types.h> to get any typedef'ed
 *	information.
 */
/* VMS:
 * We need this typedef to point to the new type even if DONT_MASK_RTL_CALLS
 * is in effect, since Perl's thread.h embeds one of these structs in its
 * thread data struct, and our struct mystat is a different size from the
 * regular struct stat (cf. note above about having to pad struct to work
 * around bug in compiler.)
 * It's OK to pass one of these to the RTL's stat(), though, since the
 * fields it fills are the same in each struct.
 */
#define Stat_t struct mystat

/* USE_STAT_RDEV:
*	This symbol is defined if this system has a stat structure declaring
*	st_rdev
*	VMS: Field exists in POSIXish version of struct stat(), but is not used.
*/
#undef USE_STAT_RDEV		/**/

/*
 * fwrite1() should be a routine with the same calling sequence as fwrite(),
 * but which outputs all of the bytes requested as a single stream (unlike
 * fwrite() itself, which on some systems outputs several distinct records
 * if the number_of_items parameter is >1).
 */
#define fwrite1 my_fwrite


#ifndef DONT_MASK_RTL_CALLS
#  define fwrite my_fwrite     /* for PerlSIO_fwrite */
#  define fdopen my_fdopen
#  define fclose my_fclose
#endif


/* By default, flush data all the way to disk, not just to RMS buffers */
#define Fflush(fp) my_flush(fp)

/* Use our own rmdir() */
#define rmdir(name) do_rmdir(name)

/* Assorted fiddling with sigs . . . */
# include <signal.h>
#define ABORT() abort()

/* Used with our my_utime() routine in vms.c */
struct utimbuf {
  time_t actime;
  time_t modtime;
};
#define utime my_utime

/* This is what times() returns, but <times.h> calls it tbuffer_t on VMS
 * prior to v7.0.  We check the DECC manifest to see whether it's already
 * done this for us, relying on the fact that perl.h #includes <time.h>
 * before it #includes "vmsish.h".
 */

#ifndef __TMS
  struct tms {
    clock_t tms_utime;    /* user time */
    clock_t tms_stime;    /* system time - always 0 on VMS */
    clock_t tms_cutime;   /* user time, children */
    clock_t tms_cstime;   /* system time, children - always 0 on VMS */
  };
#else
   /* The new headers change the times() prototype to tms from tbuffer */
#  define tbuffer_t struct tms
#endif

/* Substitute our own routines for gmtime(), localtime(), and time(),
 * which allow us to implement the vmsish 'time' pragma, and work
 * around absence of system-level UTC support on old versions of VMS.
 */
#define gmtime(t) my_gmtime(t)
#define localtime(t) my_localtime(t)
#define time(t) my_time(t)

/* If we're using an older version of VMS whose Unix signal emulation
 * isn't very POSIXish, then roll our own.
 */
#if __VMS_VER < 70000000 || __DECC_VER < 50200000
#  define HOMEGROWN_POSIX_SIGNALS
#endif
#ifdef HOMEGROWN_POSIX_SIGNALS
#  define sigemptyset(t) my_sigemptyset(t)
#  define sigfillset(t) my_sigfillset(t)
#  define sigaddset(t, u) my_sigaddset(t, u)
#  define sigdelset(t, u) my_sigdelset(t, u)
#  define sigismember(t, u) my_sigismember(t, u)
#  define sigprocmask(t, u, v) my_sigprocmask(t, u, v)
#  ifndef _SIGSET_T
   typedef int sigset_t;
#  endif
   /* The tools for sigprocmask() are there, just not the routine itself */
#  ifndef SIG_UNBLOCK
#    define SIG_UNBLOCK 1
#  endif
#  ifndef SIG_BLOCK
#    define SIG_BLOCK 2
#  endif
#  ifndef SIG_SETMASK
#    define SIG_SETMASK 3
#  endif
#  define sigaction sigvec
#  define sa_flags sv_onstack
#  define sa_handler sv_handler
#  define sa_mask sv_mask
#  define sigsuspend(set) sigpause(*set)
#  define sigpending(a) (not_here("sigpending"),0)
#else
/*
 * The C RTL's sigaction fails to check for invalid signal numbers so we 
 * help it out a bit.
 */
#  ifndef DONT_MASK_RTL_CALLS
#    define sigaction(a,b,c) Perl_my_sigaction(aTHX_ a,b,c)
#  endif
#endif
#ifdef KILL_BY_SIGPRC
#  define kill  Perl_my_kill
#endif


/* VMS doesn't use a real sys_nerr, but we need this when scanning for error
 * messages in text strings . . .
 */

#define sys_nerr EVMSERR  /* EVMSERR is as high as we can go. */

/* Look up new %ENV values on the fly */
#define DYNAMIC_ENV_FETCH 1
  /* Special getenv function for retrieving %ENV elements. */
#define ENVgetenv(v) my_getenv(v,FALSE)
#define ENVgetenv_len(v,l) my_getenv_len(v,l,FALSE)


/* Thin jacket around cuserid() to match Unix' calling sequence */
#define getlogin my_getlogin

/* Ditto for sys$hash_password() . . . */
#define crypt(a,b)  Perl_my_crypt(aTHX_ a,b)

/* Tweak arg to mkdir & chdir first, so we can tolerate trailing /. */
#define Mkdir(dir,mode) Perl_my_mkdir(aTHX_ (dir),(mode))
#define Chdir(dir) my_chdir((dir))

/* Use our own stat() clones, which handle Unix-style directory names */
#define Stat(name,bufptr) flex_stat(name,bufptr)
#define Fstat(fd,bufptr) Perl_flex_fstat(aTHX_ fd,bufptr)

/* Setup for the dirent routines:
 * opendir(), closedir(), readdir(), seekdir(), telldir(), and
 * vmsreaddirversions(), and preprocessor stuff on which these depend:
 *    Written by Rich $alz, <rsalz@bbn.com> in August, 1990.
 */
    /* Data structure returned by READDIR(). */
struct dirent {
    char	d_name[256];		/* File name		*/
    int		d_namlen;			/* Length of d_name */
    int		vms_verscount;		/* Number of versions	*/
    int		vms_versions[20];	/* Version numbers	*/
};

    /* Handle returned by opendir(), used by the other routines.  You
     * are not supposed to care what's inside this structure. */
typedef struct _dirdesc {
    long			context;
    int				vms_wantversions;
    unsigned long int           count;
    char			*pattern;
    struct dirent		entry;
    struct dsc$descriptor_s	pat;
    void			*mutex;
} DIR;

#define rewinddir(dirp)		seekdir((dirp), 0)

/* used for our emulation of getpw* */
struct passwd {
        char    *pw_name;    /* Username */
        char    *pw_passwd;
        Uid_t   pw_uid;      /* UIC member number */
        Gid_t   pw_gid;      /* UIC group  number */
        char    *pw_comment; /* Default device/directory (Unix-style) */
        char    *pw_gecos;   /* Owner */
        char    *pw_dir;     /* Default device/directory (VMS-style) */
        char    *pw_shell;   /* Default CLI name (eg. DCL) */
};
#define pw_unixdir pw_comment  /* Default device/directory (Unix-style) */
#define getpwnam my_getpwnam
#define getpwuid my_getpwuid
#define getpwent my_getpwent
#define endpwent my_endpwent
#define setpwent my_endpwent

/* Our own stat_t substitute, since we play with st_dev and st_ino -
 * we want atomic types so Unix-bound code which compares these fields
 * for two files will work most of the time under VMS.
 * N.B. 1. The st_ino hack assumes that sizeof(unsigned short[3]) ==
 * sizeof(unsigned) + sizeof(unsigned short).  We can't use a union type
 * to map the unsigned int we want and the unsigned short[3] the CRTL
 * returns into the same member, since gcc has different ideas than DECC
 * and VAXC about sizing union types.
 * N.B. 2. The routine cando() in vms.c assumes that &stat.st_ino is the
 * address of a FID.
 */
/* First, grab the system types, so we don't clobber them later */
#include <stat.h>
/* Since we've got to match the size of the CRTL's stat_t, we need
 * to mimic DECC's alignment settings.
 */
#ifdef USE_LARGE_FILES
/* Mimic the new stat structure, filler fields, and alignment. */
#if defined(__DECC) || defined(__DECCXX)
#  pragma __member_alignment __save
#  pragma member_alignment
#endif

struct mystat
{
        char *st_devnam;       /* pointer to device name */
        char *st_fill_dev;
        unsigned st_ino;        /* hack - CRTL uses unsigned short[3] for */
        unsigned short rvn;     /* FID (num,seq,rvn) */
        unsigned short st_fill_ino;
        unsigned short st_mode; /* file "mode" i.e. prot, dir, reg, etc. */
        unsigned short st_fill_mode;
        int     st_nlink;       /* for compatibility - not really used */
        unsigned st_uid;        /* from ACP - QIO uic field */
        unsigned short st_gid;  /* group number extracted from st_uid */
        unsigned short st_fill_gid;
        dev_t   st_rdev;        /* for compatibility - always zero */
        off_t   st_size;        /* file size in bytes */
        unsigned st_atime;      /* file access time; always same as st_mtime */
        unsigned st_fill_atime;
        unsigned st_mtime;      /* last modification time */
        unsigned st_fill_mtime;
        unsigned st_ctime;      /* file creation time */
        unsigned st_fill_ctime;
        char    st_fab_rfm;     /* record format */
        char    st_fab_rat;     /* record attributes */
        char    st_fab_fsz;     /* fixed header size */
        char    st_fab_fill;
        unsigned st_fab_mrs;    /* record size */
        int st_fill_expand[7];  /* will probably fill from beginning, so put our st_dev at end */
        unsigned st_dev;        /* encoded device name */
};

#else /* !defined(USE_LARGE_FILES) */

#if defined(__DECC) || defined(__DECCXX)
#  pragma __member_alignment __save
#  pragma __nomember_alignment
#endif
#if defined(__DECC) 
#  pragma __message __save
#  pragma __message disable (__MISALGNDSTRCT)
#  pragma __message disable (__MISALGNDMEM)
#endif

struct mystat
{
        char *st_devnam;  /* pointer to device name */
        unsigned st_ino;    /* hack - CRTL uses unsigned short[3] for */
        unsigned short rvn; /* FID (num,seq,rvn) */
        unsigned short st_mode;	/* file "mode" i.e. prot, dir, reg, etc. */
        int	st_nlink;	/* for compatibility - not really used */
        unsigned st_uid;	/* from ACP - QIO uic field */
        unsigned short st_gid;	/* group number extracted from st_uid */
        dev_t   st_rdev;	/* for compatibility - always zero */
        off_t   st_size;	/* file size in bytes */
        unsigned st_atime;	/* file access time; always same as st_mtime */
        unsigned st_mtime;	/* last modification time */
        unsigned st_ctime;	/* file creation time */
        char	st_fab_rfm;	/* record format */
        char	st_fab_rat;	/* record attributes */
        char	st_fab_fsz;	/* fixed header size */
        unsigned st_dev;	/* encoded device name */
        /* Pad struct out to integral number of longwords, since DECC 5.6/VAX
         * has a bug in dealing with offsets in structs in which are embedded
         * other structs whose size is an odd number of bytes.  (An even
         * number of bytes is enough to make it happy, but we go for natural
         * alignment anyhow.)
         */
        char	st_fill1[sizeof(void *) - (3*sizeof(unsigned short) + 3*sizeof(char))%sizeof(void *)];
};

#if defined(__DECC) 
#  pragma __message __restore
#endif

#endif /* defined(USE_LARGE_FILES) */

#if defined(__DECC) || defined(__DECCXX)
#  pragma __member_alignment __restore
#endif

typedef unsigned mydev_t;
typedef unsigned myino_t;

/*
 * DEC C previous to 6.0 corrupts the behavior of the /prefix
 * qualifier with the extern prefix pragma.  This provisional
 * hack circumvents this prefix pragma problem in previous 
 * precompilers.
 */
#if defined(__VMS_VER) && __VMS_VER >= 70000000
#  if defined(VMS_WE_ARE_CASE_SENSITIVE) && (__DECC_VER < 60000000)
#    pragma __extern_prefix save
#    pragma __extern_prefix ""  /* set to empty to prevent prefixing */
#    define geteuid decc$__unix_geteuid
#    define getuid decc$__unix_getuid
#    define stat(__p1,__p2)   decc$__utc_stat(__p1,__p2)
#    define fstat(__p1,__p2)  decc$__utc_fstat(__p1,__p2)
#    pragma __extern_prefix restore
#  endif
#endif

#ifndef DONT_MASK_RTL_CALLS  /* defined for vms.c so we can see RTL calls */
#  ifdef stat
#    undef stat
#  endif
#  define stat mystat
#  define dev_t mydev_t
#  define ino_t myino_t
#endif
/* Cons up a 'delete' bit for testing access */
#define S_IDUSR (S_IWUSR | S_IXUSR)
#define S_IDGRP (S_IWGRP | S_IXGRP)
#define S_IDOTH (S_IWOTH | S_IXOTH)


/* Prototypes for functions unique to vms.c.  Don't include replacements
 * for routines in the mainline source files excluded by #ifndef VMS;
 * their prototypes are already in proto.h.
 *
 * In order to keep Gen_ShrFls.Pl happy, functions which are to be made
 * available to images linked to PerlShr.Exe must be declared between the
 * __VMS_PROTOTYPES__ and __VMS_SEPYTOTORP__ lines, and must be in the form
 *    <data type><TAB>name<WHITESPACE>(<prototype args>);
 */

#ifdef NO_PERL_TYPEDEFS
  /* We don't have Perl typedefs available (e.g. when building a2p), so
     we fake them here.  N.B.  There is *no* guarantee that the faked
     prototypes will actually match the real routines.  If you want to
     call Perl routines, include perl.h to get the real typedefs.  */
#  ifndef bool
#    define bool int
#    define __MY_BOOL_TYPE_FAKE
#  endif
#  ifndef I32
#    define I32  int
#    define __MY_I32_TYPE_FAKE
#  endif
#  ifndef SV
#    define SV   void   /* Since we only see SV * in prototypes */
#    define __MY_SV_TYPE_FAKE
#  endif
#endif

void	prime_env_iter (void);
void	init_os_extras ();
/* prototype section start marker; `typedef' passes through cpp */
typedef char  __VMS_PROTOTYPES__;
int	Perl_vmstrnenv (const char *, char *, unsigned long int, struct dsc$descriptor_s **, unsigned long int);
#if !defined(PERL_IMPLICIT_CONTEXT)
char *	Perl_my_getenv (const char *, bool);
int	Perl_my_trnlnm (const char *, char *, unsigned long int);
char *	Perl_tounixspec (char *, char *);
char *	Perl_tounixspec_ts (char *, char *);
char *	Perl_tovmsspec (char *, char *);
char *	Perl_tovmsspec_ts (char *, char *);
char *	Perl_tounixpath (char *, char *);
char *	Perl_tounixpath_ts (char *, char *);
char *	Perl_tovmspath (char *, char *);
char *	Perl_tovmspath_ts (char *, char *);
int	Perl_do_rmdir (char *);
char *	Perl_fileify_dirspec (char *, char *);
char *	Perl_fileify_dirspec_ts (char *, char *);
char *	Perl_pathify_dirspec (char *, char *);
char *	Perl_pathify_dirspec_ts (char *, char *);
char *	Perl_rmsexpand (char *, char *, char *, unsigned);
char *	Perl_rmsexpand_ts (char *, char *, char *, unsigned);
int	Perl_trim_unixpath (char *, char*, int);
DIR *	Perl_opendir (char *);
int	Perl_rmscopy (char *, char *, int);
int	Perl_my_mkdir (char *, Mode_t);
bool	Perl_vms_do_aexec (SV *, SV **, SV **);
#else
char *	Perl_my_getenv (pTHX_ const char *, bool);
int	Perl_my_trnlnm (pTHX_ const char *, char *, unsigned long int);
char *	Perl_tounixspec (pTHX_ char *, char *);
char *	Perl_tounixspec_ts (pTHX_ char *, char *);
char *	Perl_tovmsspec (pTHX_ char *, char *);
char *	Perl_tovmsspec_ts (pTHX_ char *, char *);
char *	Perl_tounixpath (pTHX_ char *, char *);
char *	Perl_tounixpath_ts (pTHX_ char *, char *);
char *	Perl_tovmspath (pTHX_ char *, char *);
char *	Perl_tovmspath_ts (pTHX_ char *, char *);
int	Perl_do_rmdir (pTHX_ char *);
char *	Perl_fileify_dirspec (pTHX_ char *, char *);
char *	Perl_fileify_dirspec_ts (pTHX_ char *, char *);
char *	Perl_pathify_dirspec (pTHX_ char *, char *);
char *	Perl_pathify_dirspec_ts (pTHX_ char *, char *);
char *	Perl_rmsexpand (pTHX_ char *, char *, char *, unsigned);
char *	Perl_rmsexpand_ts (pTHX_ char *, char *, char *, unsigned);
int	Perl_trim_unixpath (pTHX_ char *, char*, int);
DIR *	Perl_opendir (pTHX_ char *);
int	Perl_rmscopy (pTHX_ char *, char *, int);
int	Perl_my_mkdir (pTHX_ char *, Mode_t);
bool	Perl_vms_do_aexec (pTHX_ SV *, SV **, SV **);
#endif
char *	Perl_my_getenv_len (pTHX_ const char *, unsigned long *, bool);
int	Perl_vmssetenv (pTHX_ char *, char *, struct dsc$descriptor_s **);
void	Perl_vmssetuserlnm(pTHX_ char *name, char *eqv);
char *	Perl_my_crypt (pTHX_ const char *, const char *);
Pid_t	Perl_my_waitpid (pTHX_ Pid_t, int *, int);
char *	my_gconvert (double, int, int, char *);
int	Perl_kill_file (pTHX_ char *);
int	Perl_my_chdir (pTHX_ char *);
FILE *	Perl_my_tmpfile ();
#ifndef HOMEGROWN_POSIX_SIGNALS
int	Perl_my_sigaction (pTHX_ int, const struct sigaction*, struct sigaction*);
#endif
#ifdef KILL_BY_SIGPRC
unsigned int	Perl_sig_to_vmscondition (int);
int	Perl_my_kill (int, int);
void	Perl_csighandler_init (void);
#endif
int	Perl_my_utime (pTHX_ char *, struct utimbuf *);
void	Perl_vms_image_init (int *, char ***);
struct dirent *	Perl_readdir (pTHX_ DIR *);
int	Perl_readdir_r(pTHX_ DIR *, struct dirent *, struct dirent **);
long	telldir (DIR *);
void	Perl_seekdir (pTHX_ DIR *, long);
void	closedir (DIR *);
void	vmsreaddirversions (DIR *, int);
struct tm *	Perl_my_gmtime (pTHX_ const time_t *);
struct tm *	Perl_my_localtime (pTHX_ const time_t *);
time_t	Perl_my_time (pTHX_ time_t *);
#ifdef HOMEGROWN_POSIX_SIGNALS
int     my_sigemptyset (sigset_t *);
int     my_sigfillset  (sigset_t *);
int     my_sigaddset   (sigset_t *, int);
int     my_sigdelset   (sigset_t *, int);
int     my_sigismember (sigset_t *, int);
int     my_sigprocmask (int, sigset_t *, sigset_t *);
#endif
I32	Perl_cando_by_name (pTHX_ I32, Uid_t, char *);
int	Perl_flex_fstat (pTHX_ int, Stat_t *);
int	Perl_flex_stat (pTHX_ const char *, Stat_t *);
int	my_vfork ();
bool	Perl_vms_do_exec (pTHX_ char *);
unsigned long int	Perl_do_aspawn (pTHX_ void *, void **, void **);
unsigned long int	Perl_do_spawn (pTHX_ char *);
FILE *  my_fdopen (int, const char *);
int     my_fclose (FILE *);
int    my_fwrite (const void *, size_t, size_t, FILE *);
int	Perl_my_flush (pTHX_ FILE *);
struct passwd *	Perl_my_getpwnam (pTHX_ char *name);
struct passwd *	Perl_my_getpwuid (pTHX_ Uid_t uid);
void	Perl_my_endpwent (pTHX);
char *	my_getlogin (void);
typedef char __VMS_SEPYTOTORP__;
/* prototype section end marker; `typedef' passes through cpp */

#ifdef NO_PERL_TYPEDEFS  /* We'll try not to scramble later files */
#  ifdef __MY_BOOL_TYPE_FAKE
#    undef bool
#    undef __MY_BOOL_TYPE_FAKE
#  endif
#  ifdef __MY_I32_TYPE_FAKE
#    undef I32
#    undef __MY_I32_TYPE_FAKE
#  endif
#  ifdef __MY_SV_TYPE_FAKE
#    undef SV
#    undef __MY_SV_TYPE_FAKE
#  endif
#endif

#ifndef VMS_DO_SOCKETS
/* This relies on tricks in perl.h to pick up that these manifest constants
 * are undefined and set up conversion routines.  It will then redefine
 * these manifest constants, so the actual values will match config.h
 */
#undef HAS_HTONS
#undef HAS_NTOHS
#undef HAS_HTONL
#undef HAS_NTOHL
#endif

/* The C RTL manual says to undef the macro for DEC C 5.2 and lower. */
#if defined(fileno) && defined(__DECC_VER) && __DECC_VER < 50300000
#  undef fileno 
#endif 

#define NO_ENVIRON_ARRAY

#endif  /* __vmsish_h_included */

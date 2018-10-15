dnl Helper Functions for the RRDtool configure.ac script
dnl 
dnl this file gets included into aclocal.m4 when runnning aclocal
dnl
dnl
dnl
dnl Check for the presence of a particular library and its header files
dnl if this check fails set the environment variable EX_CHECK_ALL_ERR to YES
dnl and prints out a helful message
dnl
dnl
dnl EX_CHECK_ALL(library, function, header, pkgconf name, tested-version, homepage, cppflags)
dnl              $1       $2        $3      $4            $5              $6        $7
dnl
dnl
AC_DEFUN([EX_CHECK_ALL],
[
 AC_LANG_PUSH(C)
 EX_CHECK_STATE=NO
 ex_check_save_LIBS=${LIBS}
 ex_check_save_CPPFLAGS=${CPPFLAGS}
 ex_check_save_LDFLAGS=${LDFLAGS}
 if test "x$7" != "x"; then
   CPPFLAGS="$CPPFLAGS -I$7"
 fi
 dnl try compiling naked first
 AC_CHECK_LIB($1,$2, [
    AC_CHECK_HEADER($3,[LIBS="-l$1 ${LIBS}";EX_CHECK_STATE=YES],[])],[])
 if test $EX_CHECK_STATE = NO; then
    dnl now asking pkg-config for help
    AC_CHECK_PROGS(PKGCONFIG,[pkg-config],no)
    if test "$PKGCONFIG" != "no"; then
          if $PKGCONFIG --exists $4; then
             CPPFLAGS=${CPPFLAGS}" "`$PKGCONFIG --cflags $4`
             LDFLAGS=${LDFLAGS}" "`$PKGCONFIG --libs-only-L $4`
             LDFLAGS=${LDFLAGS}" "`$PKGCONFIG --libs-only-other $4`
             LIBS=${LIBS}" "`$PKGCONFIG --libs-only-l $4`
	     dnl remove the cached value and test again
	     unset ac_cv_lib_$1_$2
             AC_CHECK_LIB($1,$2,[
	         unset ac_cv_header_`echo $3 | sed ['s/[^a-zA-Z0-9_]/_/g;s/^[0-9]/_/']`
		 AC_CHECK_HEADER($3,[EX_CHECK_STATE=YES],[])
	     ],[])
          else
             AC_MSG_WARN([
----------------------------------------------------------------------------
* I found a copy of pkgconfig, but there is no $4.pc file around.
  You may want to set the PKG_CONFIG_PATH variable to point to its
  location.
----------------------------------------------------------------------------
			])
           fi
     fi
  fi  

  if test ${EX_CHECK_STATE} = NO; then
     AC_MSG_WARN([
----------------------------------------------------------------------------
* I could not find a working copy of $4. Check config.log for hints on why
  this is the case. Maybe you need to set LDFLAGS and CPPFLAGS appropriately
  so that compiler and the linker can find lib$1 and its header files. If
  you have not installed $4, you can get it either from its original home on

     $6

  You can find also find an archive copy on

     http://people.ee.ethz.ch/~oetiker/webtools/rrdtool/pub/libs

  The last tested version of $4 is $5.

       LIBS=$LIBS
   LDFLAGS=$LDFLAGS
  CPPFLAGS=$CPPFLAGS

----------------------------------------------------------------------------
                ])
       EX_CHECK_ALL_ERR=YES
       LIBS="${ex_check_save_LIBS}"
       CPPFLAGS="${ex_check_save_CPPFLAGS}"
       LDFLAGS="${ex_check_save_LDFLAGS}"
    fi
    AC_LANG_POP(C)
]
)

dnl
dnl  Ptherad check from http://autoconf-archive.cryp.to/acx_pthread.m4
dnl
dnl @synopsis ACX_PTHREAD([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro figures out how to build C programs using POSIX threads.
dnl It sets the PTHREAD_LIBS output variable to the threads library and
dnl linker flags, and the PTHREAD_CFLAGS output variable to any special
dnl C compiler flags that are needed. (The user can also force certain
dnl compiler flags/libs to be tested by setting these environment
dnl variables.)
dnl
dnl Also sets PTHREAD_CC to any special C compiler that is needed for
dnl multi-threaded programs (defaults to the value of CC otherwise).
dnl (This is necessary on AIX to use the special cc_r compiler alias.)
dnl
dnl NOTE: You are assumed to not only compile your program with these
dnl flags, but also link it with them as well. e.g. you should link
dnl with $PTHREAD_CC $CFLAGS $PTHREAD_CFLAGS $LDFLAGS ... $PTHREAD_LIBS
dnl $LIBS
dnl
dnl If you are only building threads programs, you may wish to use
dnl these variables in your default LIBS, CFLAGS, and CC:
dnl
dnl        LIBS="$PTHREAD_LIBS $LIBS"
dnl        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
dnl        CC="$PTHREAD_CC"
dnl
dnl In addition, if the PTHREAD_CREATE_JOINABLE thread-attribute
dnl constant has a nonstandard name, defines PTHREAD_CREATE_JOINABLE to
dnl that name (e.g. PTHREAD_CREATE_UNDETACHED on AIX).
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a threads
dnl library is found, and ACTION-IF-NOT-FOUND is a list of commands to
dnl run it if it is not found. If ACTION-IF-FOUND is not specified, the
dnl default action will define HAVE_PTHREAD.
dnl
dnl Please let the authors know if this macro fails on any platform, or
dnl if you have any other suggestions or comments. This macro was based
dnl on work by SGJ on autoconf scripts for FFTW (www.fftw.org) (with
dnl help from M. Frigo), as well as ac_pthread and hb_pthread macros
dnl posted by Alejandro Forero Cuervo to the autoconf macro repository.
dnl We are also grateful for the helpful feedback of numerous users.
dnl
dnl @category InstalledPackages
dnl @author Steven G. Johnson <stevenj@alum.mit.edu>
dnl @version 2005-01-14
dnl @license GPLWithACException

AC_DEFUN([ACX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_PUSH(C)
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++
# pthread-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

		pthread-config)
		AC_CHECK_PROG(acx_pthread_config, pthread-config, yes, no)
		if test x"$acx_pthread_config" = xno; then continue; fi
		PTHREAD_CFLAGS="`pthread-config --cflags`"
		PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
		;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ]])],[acx_pthread_ok=yes],[])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
	AC_MSG_CHECKING([for joinable pthread attribute])
	attr_name=unknown
	for attr in PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED; do
	    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[int attr=$attr;]])],[attr_name=$attr; break],[])
	done
        AC_MSG_RESULT($attr_name)
        if test "$attr_name" != PTHREAD_CREATE_JOINABLE; then
            AC_DEFINE_UNQUOTED(PTHREAD_CREATE_JOINABLE, $attr_name,
                               [Define to necessary symbol if this constant
                                uses a non-standard name on your system.])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        x_rflag=no
        case "${host_cpu}-${host_os}" in
            *-aix* | *-freebsd* | *-darwin*) x_rflag="-D_THREAD_SAFE";;
            *solaris* | *-osf* | *-hpux*) x_rflag="-D_REENTRANT";;
            *-linux*)
            if test x"$PTHREAD_CFLAGS" = "x-pthread"; then
                # For Linux/gcc "-pthread" implies "-lpthread". We need, however, to make this explicit
                # in PTHREAD_LIBS such that a shared library to be built properly depends on libpthread.
                PTHREAD_LIBS="-lpthread $PTHREAD_LIBS"
            fi;;
        esac
        AC_MSG_RESULT(${x_rflag})
        if test "x$x_rflag" != xno; then
            PTHREAD_CFLAGS="$x_rflag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC})
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi
AC_LANG_POP(C)
])dnl ACX_PTHREAD


dnl
dnl determine how to get IEEE math working
dnl AC_IEEE(MESSAGE, set rd_cv_ieee_[var] variable, INCLUDES,
dnl   FUNCTION-BODY, [ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]])
dnl

dnl substitute them in all the files listed in AC_OUTPUT
AC_SUBST(PERLFLAGS)

AC_DEFUN([AC_IEEE], [
AC_MSG_CHECKING([if IEEE math works $1])
AC_CACHE_VAL([rd_cv_ieee_$2],
[AC_RUN_IFELSE([AC_LANG_SOURCE([[$3

#if HAVE_MATH_H
#  include <math.h>
#endif

#if HAVE_FLOAT_H
#  include <float.h>  
#endif

#if HAVE_IEEEFP_H
#  include <ieeefp.h>
#endif

#if HAVE_FP_CLASS_H
#  include <fp_class.h>
#endif

/* Solaris */
#if (! defined(HAVE_ISINF) && defined(HAVE_FPCLASS))
#  define HAVE_ISINF 1
#  define isinf(a) (fpclass(a) == FP_NINF || fpclass(a) == FP_PINF)
#endif

/* Digital UNIX */
#if (! defined(HAVE_ISINF) && defined(HAVE_FP_CLASS) && defined(HAVE_FP_CLASS_H))
#  define HAVE_ISINF 1
#  define isinf(a) (fp_class(a) == FP_NEG_INF || fp_class(a) == FP_POS_INF)
#endif 

/* AIX */
#if (! defined(HAVE_ISINF) && defined(HAVE_CLASS))
#  define HAVE_ISINF 1
#  define isinf(a) (class(a) == FP_MINUS_INF || class(a) == FP_PLUS_INF)
#endif

#if (! defined(HAVE_ISINF) && defined(HAVE_FPCLASSIFY) && defined(FP_PLUS_INF) && defined(FP_MINUS_INF))
#  define HAVE_ISINF 1
#  define isinf(a) (fpclassify(a) == FP_MINUS_INF || fpclassify(a) == FP_PLUS_INF)
#endif

#if (! defined(HAVE_ISINF) && defined(HAVE_FPCLASSIFY) && defined(FP_INFINITE))
#  define HAVE_ISINF 1
#  define isinf(a) (fpclassify(a) == FP_INFINITE)
#endif

#include <stdio.h>
int main(void){
    double nan,inf,c,zero;
    $4;
    /* some math to see if we get a floating point exception */
    zero=sin(0.0); /* don't let the compiler optimize us away */
    nan=0.0/zero; /* especially here */
    inf=1.0/zero; /* and here. I want to know if it can do the magic */
		  /* at run time without sig fpe */
    c = inf + nan;
    c = inf / nan;
    if (! isnan(nan)) {printf ("not isnan(NaN) ... "); return 1;}
    if (nan == nan) {printf ("nan == nan ... "); return 1;}
    if (! isinf(inf)) {printf ("not isinf(oo) ... "); return 1;}
    if (! isinf(-inf)) {printf ("not isinf(-oo) ... "); return 1;}
    if (! inf > 0) {printf ("not inf > 0 ... "); return 1;}
    if (! -inf < 0) {printf ("not -inf < 0 ... "); return 1;}
    return 0;
 }]])],[rd_cv_ieee_$2=yes],[rd_cv_ieee_$2=no],[:])])
dnl these we run regardles is cached or not
if test x${rd_cv_ieee_$2} = "xyes"; then
 AC_MSG_RESULT(yes)
 $5
else
 AC_MSG_RESULT(no)
 $6
fi

])

AC_DEFUN([AC_FULL_IEEE],[
AC_LANG_PUSH(C)
_cflags=${CFLAGS}
AC_IEEE([out of the box], works, , , ,
  [CFLAGS="$_cflags -ieee"
  AC_IEEE([with the -ieee switch], switch, , , ,
    [CFLAGS="$_cflags -qfloat=nofold"
    AC_IEEE([with the -qfloat=nofold switch], nofold, , , ,
      [CFLAGS="$_cflags -w -qflttrap=enable:zerodivide"
      AC_IEEE([with the -w -qflttrap=enable:zerodivide], flttrap, , , ,
       [CFLAGS="$_cflags -mieee"
       AC_IEEE([with the -mieee switch], mswitch, , , ,
         [CFLAGS="$_cflags -q float=rndsngl"
         AC_IEEE([with the -q float=rndsngl switch], qswitch, , , ,
           [CFLAGS="$_cflags -OPT:IEEE_NaN_inf=ON"
           AC_IEEE([with the -OPT:IEEE_NaN_inf=ON switch], ieeenaninfswitch, , , ,
             [CFLAGS="$_cflags -OPT:IEEE_comparisons=ON"
             AC_IEEE([with the -OPT:IEEE_comparisons=ON switch], ieeecmpswitch, , , ,
               [CFLAGS=$_cflags
               AC_IEEE([with fpsetmask(0)], mask,
                 [#include <floatingpoint.h>], [fpsetmask(0)],
                 [AC_DEFINE(MUST_DISABLE_FPMASK)
	         PERLFLAGS="CCFLAGS=-DMUST_DISABLE_FPMASK"],
                 [AC_IEEE([with signal(SIGFPE,SIG_IGN)], sigfpe,
                   [#include <signal.h>], [signal(SIGFPE,SIG_IGN)],
                   [AC_DEFINE(MUST_DISABLE_SIGFPE)
                   PERLFLAGS="CCFLAGS=-DMUST_DISABLE_SIGFPE"],		
                   AC_MSG_ERROR([
Your Compiler does not do propper IEEE math ... Please find out how to
make IEEE math work with your compiler and let me know (oetiker@ee.ethz.ch).
Check config.log to see what went wrong ...
]))])])])])])])])])])

AC_LANG_POP(C)

])


dnl a macro to check for ability to create python extensions
dnl  AM_CHECK_PYTHON_HEADERS([ACTION-IF-POSSIBLE], [ACTION-IF-NOT-POSSIBLE])
dnl function also defines PYTHON_INCLUDES
AC_DEFUN([AM_CHECK_PYTHON_HEADERS],
[AC_REQUIRE([AM_PATH_PYTHON])
AC_MSG_CHECKING(for headers required to compile python extensions)
dnl deduce PYTHON_INCLUDES
py_prefix=`$PYTHON -c "import sys; print sys.prefix"`
py_exec_prefix=`$PYTHON -c "import sys; print sys.exec_prefix"`
PYTHON_INCLUDES="-I${py_prefix}/include/python${PYTHON_VERSION}"
if test "$py_prefix" != "$py_exec_prefix"; then
  PYTHON_INCLUDES="$PYTHON_INCLUDES -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
fi
AC_SUBST(PYTHON_INCLUDES)
dnl check if the headers exist:
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_TRY_CPP([#include <Python.h>],dnl
[AC_MSG_RESULT(found)
$1],dnl
[AC_MSG_RESULT(not found)
$2])
CPPFLAGS="$save_CPPFLAGS"
])



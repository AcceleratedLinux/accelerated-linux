/* stdc.h -- macros to make source compile on both ANSI C and K&R C
   compilers. */

/* Copyright (C) 1993 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#if !defined (_RL_STDC_H_)
#define _RL_STDC_H_

/* Adapted from BSD /usr/include/sys/cdefs.h. */

/* A function can be defined using prototypes and compile on both ANSI C
   and traditional C compilers with something like this:
	extern char *func __P((char *, char *, int)); */

#if defined (__STDC__) || defined(__cplusplus)

#  if !defined (__P)
#    define __P(protos) protos
#  endif
#  define __STRING(x) #x

#  if !defined (__GNUC__) && !defined(__cplusplus) && !defined(inline)
#    define inline
#  endif

#else /* !__STDC__ */

#  if !defined (__P)
#    define __P(protos) ()
#  endif
#  define __STRING(x) "x"

#if defined (__GNUC__)		/* gcc with -traditional */
#  if !defined (const)
#    define const  __const
#  endif
#  if !defined (inline)
#    define inline __inline
#  endif
#  if !defined (signed)
#    define signed __signed
#  endif
#  if !defined (volatile)
#    define volatile __volatile
#  endif
#else /* !__GNUC__ */
#  if !defined (const)
#    define const
#  endif
#  if !defined (inline)
#    define inline
#  endif
#  if !defined (signed)
#    define signed
#  endif
#  if !defined (volatile)
#    define volatile
#  endif
#endif /* !__GNUC__ */

#endif /* !__STDC__ */

#if !defined(__attribute__) && (defined(__cplusplus) || !defined(__GNUC__)  || __GNUC__ == 2 && __GNUC_MINOR__ < 8)
#define __attribute__(A)
#endif

#endif /* !_RL_STDC_H_ */

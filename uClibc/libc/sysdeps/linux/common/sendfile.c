/* vi: set sw=4 ts=4: */
/*
 * sendfile() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* need to hide the 64bit prototype or the strong_alias()
 *  * will fail when __NR_sendfile64 doesnt exist */
#define sendfile64 __hidesendfile64

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/sendfile.h>

#ifdef __NR_sendfile

#undef sendfile64

_syscall4(ssize_t, sendfile, int, out_fd, int, in_fd, __off_t *, offset,
		  size_t, count)

#if ! defined __NR_sendfile64 && defined __UCLIBC_HAS_LFS__
strong_alias(sendfile,sendfile64)
#endif

#endif /* __NR_sendfile */

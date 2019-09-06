/* vi: set sw=4 ts=4: */
/*
 * fanotify interface for uClibc
 *
 * Copyright (C) 2015 by Bartosz Golaszewski <bartekgola at gmail.com>
 *
 * Licensed under the LGPL v2.1+, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/fanotify.h>

#ifdef __NR_fanotify_init
_syscall2(int, fanotify_init, unsigned int, flags, unsigned int, event_f_flags)
#endif

#ifdef __NR_fanotify_mark
# include <bits/wordsize.h>
# include <fcntl.h>

# if __WORDSIZE == 64
_syscall5(int, fanotify_mark, int, fanotify_fd, unsigned int, flags,
	  uint64_t, mask, int, dirfd, const char *, pathname)
# else
int fanotify_mark(int fanotify_fd, unsigned int flags,
		  uint64_t mask, int dirfd, const char *pathname)
{
	return INLINE_SYSCALL(fanotify_mark, 6, fanotify_fd, flags,
			      __LONG_LONG_PAIR((uint32_t)(mask >> 32), (uint32_t)(mask &  0xffffffff)),
			      dirfd, pathname);
}
# endif
#endif

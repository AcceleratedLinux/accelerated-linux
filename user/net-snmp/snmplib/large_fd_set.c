/**
 * @file large_fd_set.c
 *
 * @brief Macro's and functions for manipulation of large file descriptor sets.
 */


#include <net-snmp/net-snmp-config.h>

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/library/snmp_assert.h>
#include <net-snmp/library/large_fd_set.h>


#if ! defined(cygwin) && defined(HAVE_WINSOCK_H)

void
netsnmp_large_fd_setfd(SOCKET fd, netsnmp_large_fd_set * fdset)
{
    unsigned        i;

    netsnmp_assert(fd != INVALID_SOCKET);

    if (fdset->lfs_set.fd_count == fdset->lfs_setsize)
        netsnmp_large_fd_set_resize(fdset, 2 * (fdset->lfs_setsize + 1));

    for (i = 0; i < fdset->lfs_set.fd_count; i++) {
        if (fdset->lfs_set.fd_array[i] == (SOCKET) (fd))
            break;
    }

    if (i == fdset->lfs_set.fd_count
        && fdset->lfs_set.fd_count < fdset->lfs_setsize) {
        fdset->lfs_set.fd_count++;
        fdset->lfs_set.fd_array[i] = (fd);
    }
}

void
netsnmp_large_fd_clr(SOCKET fd, netsnmp_large_fd_set * fdset)
{
    unsigned        i;

    netsnmp_assert(fd != INVALID_SOCKET);

    for (i = 0; i < fdset->lfs_set.fd_count; i++) {
        if (fdset->lfs_set.fd_array[i] == (fd)) {
            while (i < fdset->lfs_set.fd_count - 1) {
                fdset->lfs_set.fd_array[i] =
                    fdset->lfs_set.fd_array[i + 1];
                i++;
            }
            fdset->lfs_set.fd_count--;
            break;
        }
    }
}

int
netsnmp_large_fd_is_set(SOCKET fd, netsnmp_large_fd_set * fdset)
{
    unsigned int    i;

    netsnmp_assert(fd != INVALID_SOCKET);

    for (i = 0; i < fdset->lfs_set.fd_count; i++) {
        if (fdset->lfs_set.fd_array[i] == fd)
            return 1;
    }
    return 0;
}

#else

void
netsnmp_large_fd_setfd(int fd, netsnmp_large_fd_set * fdset)
{
    netsnmp_assert(fd >= 0);

    if (fd >= fdset->lfs_setsize)
        netsnmp_large_fd_set_resize(fdset, 2 * (fdset->lfs_setsize + 1));

    FD_SET(fd, fdset->lfs_setptr);
}

void
netsnmp_large_fd_clr(int fd, netsnmp_large_fd_set * fdset)
{
    netsnmp_assert(fd >= 0);

    if (fd < fdset->lfs_setsize)
        FD_CLR(fd, fdset->lfs_setptr);
}

int
netsnmp_large_fd_is_set(int fd, netsnmp_large_fd_set * fdset)
{
    netsnmp_assert(fd >= 0);

    return fd < fdset->lfs_setsize && FD_ISSET(fd, fdset->lfs_setptr);
}

#endif

void
netsnmp_large_fd_set_init(netsnmp_large_fd_set * fdset, int setsize)
{
    fdset->lfs_setsize = 0;
    fdset->lfs_setptr  = 0;
    netsnmp_large_fd_set_resize(fdset, setsize);
}

void
netsnmp_large_fd_set_resize(netsnmp_large_fd_set * fdset, int setsize)
{
    int             fd_set_bytes;

    if (setsize > FD_SETSIZE) {
        fd_set_bytes = NETSNMP_FD_SET_BYTES(setsize);
        if (fdset->lfs_setsize > FD_SETSIZE)
            fdset->lfs_setptr = realloc(fdset->lfs_setptr, fd_set_bytes);
        else {
            fdset->lfs_setptr = malloc(fd_set_bytes);
            *fdset->lfs_setptr = fdset->lfs_set;
        }
    } else {
        if (fdset->lfs_setsize > FD_SETSIZE) {
            fdset->lfs_set = *fdset->lfs_setptr;
            free(fdset->lfs_setptr);
        }
        fdset->lfs_setptr = &fdset->lfs_set;
    }

#if ! (! defined(cygwin) && defined(HAVE_WINSOCK_H))
    {
        int             i;

	/*
	 * Unix: clear the file descriptors defined in the resized *fdset
	 * but that were not defined in the original *fdset.
	 */
	for (i = fdset->lfs_setsize + 1; i < setsize; i++)
	    FD_CLR(i, fdset->lfs_setptr);
    }
#endif

    fdset->lfs_setsize = setsize;
}

void
netsnmp_large_fd_set_cleanup(netsnmp_large_fd_set * fdset)
{
    netsnmp_large_fd_set_resize(fdset, 0);
    fdset->lfs_setsize = 0;
    fdset->lfs_setptr  = 0;
}

void
netsnmp_copy_fd_set_to_large_fd_set(netsnmp_large_fd_set * dst,
                                    const fd_set * src)
{
    netsnmp_large_fd_set_resize(dst, FD_SETSIZE);
    *dst->lfs_setptr = *src;
}

int
netsnmp_copy_large_fd_set_to_fd_set(fd_set * dst,
                                    const netsnmp_large_fd_set * src)
{
    /* Report failure if *src is larger than FD_SETSIZE. */
    if (src->lfs_setsize > FD_SETSIZE) {
        FD_ZERO(dst);
        return -1;
    }

    *dst = *src->lfs_setptr;

#if ! (! defined(cygwin) && defined(HAVE_WINSOCK_H))
    {
        int             i;

	/* Unix: clear any file descriptors defined in *dst but not in *src. */
	for (i = src->lfs_setsize; i < FD_SETSIZE; ++i)
	  FD_CLR(i, dst);
    }
#endif

    return 0;
}

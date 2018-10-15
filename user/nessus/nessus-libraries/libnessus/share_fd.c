#include <includes.h>
/*
 * send_fd() and recv_fd() come from OpenSSH-23/openssh/monitor_fdpass.c.
 * These two functions have been written by Niels Provos :
 *
 * Copyright 2001 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


int send_fd(int socket, int fd)
{
#if defined(HAVE_SENDMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR))
	struct msghdr msg;
	struct iovec vec;
	char ch = '\0';
	int n;
#ifndef HAVE_ACCRIGHTS_IN_MSGHDR
	char tmp[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;
#endif

	memset(&msg, 0, sizeof(msg));
#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof(fd);
#else
	msg.msg_control = (caddr_t)tmp;
	msg.msg_controllen = CMSG_LEN(sizeof(int));
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(cmsg) = fd;
#endif

	vec.iov_base = &ch;
	vec.iov_len = 1;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;

	if ((n = sendmsg(socket, &msg, 0)) == -1)
		fprintf(stderr, "send_fd(): sendmsg(%d): %s", fd,
		    strerror(errno));
	if (n != 1)
		fprintf(stderr, "send_fd(): sendmsg: expected sent 1 got %d", n);
 return 0;
#else
 return -1;
#endif
}

int
recv_fd(int socket)
{
#if defined(HAVE_RECVMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR))
	struct msghdr msg;
	struct iovec vec;
	char ch;
	int fd, n;
#ifndef HAVE_ACCRIGHTS_IN_MSGHDR
	char tmp[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;
#endif

	memset(&msg, 0, sizeof(msg));
	vec.iov_base = &ch;
	vec.iov_len = 1;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof(fd);
#else
	msg.msg_control = tmp;
	msg.msg_controllen = sizeof(tmp);
#endif

	if ((n = recvmsg(socket, &msg, 0)) == -1)
		printf("%s: recvmsg: %s", __func__, strerror(errno));
	if (n != 1)
		printf("%s: recvmsg: expected received 1 got %d",
		    __func__, n);

#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	if (msg.msg_accrightslen != sizeof(fd))
		printf("recv_fd(): no fd\n");
#else
	cmsg = CMSG_FIRSTHDR(&msg);
	if (cmsg->cmsg_type != SCM_RIGHTS)
		printf("recv_fd():  expected type %d got %d",
		    SCM_RIGHTS, cmsg->cmsg_type);
	fd = (*(int *)CMSG_DATA(cmsg));
#endif
	return fd;
#else
	return -1;
#endif
}

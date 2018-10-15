/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001, 2002, 2003, 2004 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* General options
 * $Id: proftpd.h,v 1.53 2005/02/26 17:28:58 castaglia Exp $
 */

#ifndef PR_PROFTPD_H
#define PR_PROFTPD_H

#include "class.h"

#ifndef TRUE
# define TRUE				1
#endif

#ifndef FALSE
# define FALSE				0
#endif

/* Definition of root user/group IDs (non-Unix platforms may have these as
 * different from 0/0).
 */
#ifdef __CYGWIN__
# define PR_ROOT_UID    18
# define PR_ROOT_GID    544
#else
# define PR_ROOT_UID    0
# define PR_ROOT_GID    0
#endif /* __CYGWIN__ */

#define CALLBACK_FRAME	\
  unsigned long p1, unsigned long p2, unsigned long p3, void *data

#define ASSERT(x)			assert(x)
#include <assert.h>

typedef int (*callback_t)(CALLBACK_FRAME);

struct conn_struc;
struct cmd_struc;
struct config_struc;

typedef struct {
  pool *pool;

  volatile int sf_flags;		/* Session/State flags */
  volatile int sp_flags;		/* Session/Protection flags */

  pr_netaddr_t data_addr;		/* Remote data address */
  unsigned short data_port;		/* Remote data port */

  unsigned char ident_lookups;		/* Is RFC931 (ident) protocol used? */
  char *ident_user;			/* User identified by ident protocol */

  const char *auth_mech;		/* Name of the authentication
                                         * module/mechanism that successfully
                                         * authenticated the client
					 */

  const char *rfc2228_mech;		/* RFC2228 authentication mechanism
					 * used
					 */

  char cwd[PR_TUNABLE_PATH_MAX];	/* Current working directory */
  char vwd[PR_TUNABLE_PATH_MAX];	/* Current virtual working directory */

  struct config_struc *dir_config;	/* Closest matching configuration
                                         * for current operation
                                         */

  /* The uid/gids are manipulated by the PRIVS macros in
   * privs.h
   */

  int disable_id_switching;		/* Disable UID/GID switching */
  uid_t uid, ouid;                      /* Current and original UIDs */
  gid_t gid;                            /* Current GID */

  array_header *gids;
  array_header *groups;

  /* fsuid/fsgid are used for automagic chown after creation or upload.
   * They are initially -1, meaning no chown/chgrp.
   */
  uid_t fsuid;				/* Saved file UID */
  gid_t fsgid;				/* Saved file GID */

  char *user,*group;			/* Username/groupname after login */
  uid_t login_uid;                      /* UID after login, but before
                                         * session.uid is changed
                                         */
  gid_t login_gid;                      /* GID after login, but before
                                         * session.gid is changed
                                         */

  pr_table_t *notes;			/* Session notes table */

  pr_class_t *class;			/* Session class */
  char *proc_prefix;			/* The "prefix" of our process name */

  int wtmp_log;				/* Are we logging to wtmp? */
  struct conn_struc *c;			/* Control connection */
  struct conn_struc *d;			/* Data connection */

  int hide_password;			/* Hide password in logs/ps listing */

  char *chroot_path;			/* Chroot directory */

  struct config_struc *anon_config;	/* Anonymous FTP configuration */
  char *anon_user;			/* E-mail address sent to us */

  char *curr_cmd;                       /* Current FTP command */
  int curr_phase;                       /* Current handler phase */

  off_t restart_pos;			/* Restart marked position */

  struct {
    struct pool *p;

    int xfer_type;     /* xfer session attributes, default/append/hidden */
    int direction;
    char *filename;			/* As shown to user */
    char *path;				/* As used in transfer */
    char *path_hidden;			/* As used in hidden stor */

    unsigned int bufsize,buflen;

    struct timeval start_time;		/* Time current transfer started */
    off_t file_size;			/* Total size of file (if known) */
    off_t total_bytes;			/* Total bytes transfered */

    char *bufstart,*buf;
  } xfer;

  /* Total number of bytes uploaded in this session. */
  off_t total_bytes_in;

  /* Total number of bytes downloaded in this session. */
  off_t total_bytes_out;

  /* Total number of bytes transfered in this session. */
  off_t total_bytes;

  /* Total number of files uploaded in this session. */
  unsigned int total_files_in;

  /* Total number of files downloaded in this session. */
  unsigned int total_files_out;

  /* Total number of files transfered (both uploaded and downloaded) in
   * this session.
   */
  unsigned int total_files_xfer;

} session_t;

/* Daemon identity values, defined in main.c */
extern uid_t daemon_uid;
extern gid_t daemon_gid;
extern array_header *daemon_gids;

/* Possible values for xfer.xfer_type, mutually exclusive */
#define STOR_DEFAULT	0
#define STOR_APPEND	1
#define STOR_HIDDEN	2
#define STOR_UNIQUE	3

extern session_t	session;
extern char AddressCollisionCheck;
extern char ServerType;
extern char MultilineRFC2228;
extern const char	*pwdfname,*grpfname;

/* Session/State flags */

#define SF_PASSIVE		0x0001	/* Data connection is in passive mode */
#define SF_ABORT		0x0002	/* Abort in progess */
#define SF_XFER			0x0004	/* Transfer in progress */
#define SF_ASCII		0x0010	/* ASCII mode transfer */
#define SF_ASCII_OVERRIDE 	0x0020	/* ASCII override this xfer only */
#define SF_ANON			0x0040	/* Anonymous (chroot) login */
#define SF_POST_ABORT		0x0100	/* After abort has occured */
#define SF_PORT			0x0200	/* Port command given */
#define	SF_EPSV_ALL		0x0400	/* EPSV ALL in effect */

#define SF_ALL		(SF_PASSIVE|SF_ABORT|SF_XFER|SF_ASCII| \
                        SF_ASCII_OVERRIDE|SF_ANON|SF_POST_ABORT|SF_PORT| \
                        SF_EPSV_ALL)

/* Session/Protection flags (RFC 2228) */

#define SP_CCC		(1 << 0)	/* Clear command channel */
#define SP_ENC		(1 << 1)	/* Privacy protected command */
#define SP_MIC		(1 << 2)	/* Integrity protected command */
#define SP_CONF		(1 << 3)	/* Confidentiality protected command */

/* Macro to test global abort flag */
#define XFER_ABORTED	(session.sf_flags & SF_ABORT)

/* Server Types */
#define SERVER_INETD		0
#define SERVER_STANDALONE	1

/* Signals */
#define RECEIVED_SIG_REHASH	0x0001
#define RECEIVED_SIG_EXIT	0x0002
#define RECEIVED_SIG_SHUTDOWN	0x0004
#define RECEIVED_SIG_SEGV	0x0008
#define RECEIVED_SIG_TERMINATE	0x0010
#define RECEIVED_SIG_XCPU	0x0020
#define RECEIVED_SIG_TERM_OTHER	0x0040
#define RECEIVED_SIG_ABORT	0x0080
#define RECEIVED_SIG_EVENT	0x0100
#define RECEIVED_SIG_CHLD	0x0200
#define RECEIVED_SIG_ALRM	0x0400

/* Timers */
#define TIMER_LOGIN		1
#define TIMER_IDLE		2
#define TIMER_NOXFER		3
#define TIMER_STALLED		4
#define TIMER_SESSION		5

/* Misc Prototypes */

void end_login(int);
void pr_signals_handle(void);
void session_exit(int, void *, int, void *);
void session_set_idle(void);
void pr_rehash_register_handler(void *, void(*)(void *));
void set_daemon_rlimits(void);
void set_session_rlimits(void);

#endif /* PR_PROFTPD_H */

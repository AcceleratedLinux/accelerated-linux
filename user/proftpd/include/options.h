/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001, 2002, 2003 The ProFTPD Project team
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

/* User configurable defaults and tunable parameters.
 *
 * $Id: options.h,v 1.21 2004/10/31 19:09:35 castaglia Exp $
 */

#ifndef PR_OPTIONS_H
#define PR_OPTIONS_H

/* Define the next option if your libc needs persistant /etc/passwd
 * and /etc/group functions.  Some libcs occasionally close these files
 * which can not be re-opened after a chroot().  Symptoms of this
 * include the inability to see user/group names when doing a 'ls -l' from
 * an anon. ftp login (you see only uid/gid numbers).
 */

/* If we have setpassent(), NEED_PERSISTENT_PASSWD is not enabled
 * by default.  This option controls the DEFAULT value of the
 * PersistentPasswd directive.  You can always override this in
 * the configuration file.
 */

#if ! (defined (HAVE_SETPASSENT) || defined (HAVE__PW_STAYOPEN))
# define NEED_PERSISTENT_PASSWD
#endif

/* Tunable parameters */

/* This defines the timeout for the main select() loop, defines the number
 * of seconds to wait for a session request before checking for things such
 * as shutdown requests, perform signal dispatching, etc before waitinng
 * for requests again.
 */

#define PR_TUNABLE_SELECT_TIMEOUT	30

/* Hash table size is the number of items in the module hash tables.
 */

#define PR_TUNABLE_HASH_TABLE_SIZE 40

/* "Backlog" is the number of connections that can be received at one
 * burst before the kernel rejects.  This can be configured by the
 * "tcpBackLog" configuration directive, this value is just the default.
 */

#define PR_TUNABLE_DEFAULT_BACKLOG	5

/* The next two define the default receive/send TCP windows.  These can be
 * configured on a per-vhost basis via the SocketOptions directive.
 */

#ifndef PR_TUNABLE_RCVBUFSZ
# define PR_TUNABLE_RCVBUFSZ		8192
#endif

#ifndef PR_TUNABLE_SNDBUFSZ
# define PR_TUNABLE_SNDBUFSZ		8192
#endif

/* Default internal buffer size used for data transfers and other
 * miscellaneous tasks.
 */
#ifndef PR_TUNABLE_BUFFER_SIZE
# define PR_TUNABLE_BUFFER_SIZE	1024
#endif

/* This one is specifically for data transfers.
 */
#ifndef PR_TUNABLE_XFER_BUFFER_SIZE
# define PR_TUNABLE_XFER_BUFFER_SIZE	PR_TUNABLE_BUFFER_SIZE
#endif

/* Maximum path length.  GNU HURD (and some others) do not define
 * MAXPATHLEN.  POSIX' PATH_MAX is mandated to be at least 256 
 * (according to some), so 1K, in the absense of MAXPATHLEN, should be
 * a reasonable default.
 */

#ifndef PR_TUNABLE_PATH_MAX
# ifdef MAXPATHLEN
#  define PR_TUNABLE_PATH_MAX           MAXPATHLEN
# else
#  define PR_TUNABLE_PATH_MAX           1024
# endif
#endif

/* Default timeouts, if not explicitly configured via
 * the TimeoutLogin, TimeoutIdle, etc directives.
 */

#ifndef PR_TUNABLE_TIMEOUTIDENT
# define PR_TUNABLE_TIMEOUTIDENT	10
#endif

#ifndef PR_TUNABLE_TIMEOUTIDLE
# define PR_TUNABLE_TIMEOUTIDLE		600
#endif

#ifndef PR_TUNABLE_TIMEOUTLINGER
# define PR_TUNABLE_TIMEOUTLINGER	180
#endif

#ifndef PR_TUNABLE_TIMEOUTLOGIN
# define PR_TUNABLE_TIMEOUTLOGIN	300
#endif

#ifndef PR_TUNABLE_TIMEOUTNOXFER
# define PR_TUNABLE_TIMEOUTNOXFER	300
#endif

#ifndef PR_TUNABLE_TIMEOUTSTALLED
# define PR_TUNABLE_TIMEOUTSTALLED	3600
#endif

/* Number of bytes in a new memory pool.  During file transfers,
 * quite a few pools can be created, which eat up a lot of memory.
 * Tune this if ProFTPD seems too memory hungry (warning! too low
 * can negatively impact performance)
 */

#ifndef PR_TUNABLE_NEW_POOL_SIZE
# define PR_TUNABLE_NEW_POOL_SIZE	512
#endif

/* Number of bytes in certain scoreboard fields, usually for reporting
 * the full command received from the connected client, or the current
 * working directory for the session.
 */

#ifndef PR_TUNABLE_SCOREBOARD_BUFFER_SIZE
# define PR_TUNABLE_SCOREBOARD_BUFFER_SIZE	80
#endif

/* Number of seconds between scoreboard scrubs, where the scoreboard is
 * scanned for slots containing invalid PIDs.  Defaults to 30 seconds.
 */

#ifndef PR_TUNABLE_SCOREBOARD_SCRUB_TIMER
# define PR_TUNABLE_SCOREBOARD_SCRUB_TIMER	30
#endif

/* Maximum number of attempted updates to the scoreboard during a
 * file transfer before an actual write is done.  This is to allow
 * an optimization where the scoreboard is not updated on every loop
 * through the transfer buffer.
 */

#ifndef PR_TUNABLE_XFER_SCOREBOARD_UPDATES
# define PR_TUNABLE_XFER_SCOREBOARD_UPDATES	10
#endif

/* Max number of recursion/directory levels to support when globbing.
 */

#ifndef PR_TUNABLE_GLOBBING_MAX	
# define PR_TUNABLE_GLOBBING_MAX	8
#endif

/* Maximum length of login name.
 */

#ifndef PR_TUNABLE_LOGIN_MAX
/* Ideally, we'd use _POSIX_LOGIN_NAME_MAX here, if it was defined.  However,
 * doing so would cause trouble for those sites that use databases for
 * storing user information; such sites often use email addresses as
 * login names.  Given that, let's use 256 as a login name size.
 */
# define PR_TUNABLE_LOGIN_MAX		256
#endif

/* Loopback network, this should generally not need to be changed,
 * although you can set a specific address by setting it to say
 * "127.0.0.1" and the netmask to "255.255.255.255".
 */

#define LOOPBACK_NET            "127.0.0.0"
#define LOOPBACK_MASK           "255.255.255.0"

#endif /* PR_OPTIONS_H */

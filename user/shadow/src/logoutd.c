/*
 * Copyright 1991 - 1993, Julianne Frances Haugh
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
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config.h>

#ident "$Id: logoutd.c,v 1.30 2005/09/07 15:00:45 kloczek Exp $"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "defines.h"
#include "prototypes.h"
/*
 * Global variables
 */
static char *Prog;

#ifndef DEFAULT_HUP_MESG
#define DEFAULT_HUP_MESG _("login time exceeded\n\n")
#endif

#ifndef HUP_MESG_FILE
#define HUP_MESG_FILE "/etc/logoutd.mesg"
#endif

#if HAVE_UTMPX_H
static int check_login (const struct utmpx *);
#else
static int check_login (const struct utmp *);
#endif

/*
 * check_login - check if user (struct utmpx/utmp) allowed to stay logged in
 */
#if HAVE_UTMPX_H
static int check_login (const struct utmpx *ut)
#else
static int check_login (const struct utmp *ut)
#endif
{
	char user[sizeof (ut->ut_user) + 1];
	time_t now;

	/*
	 * ut_user may not have the terminating NUL.
	 */
	strncpy (user, ut->ut_user, sizeof (ut->ut_user));
	user[sizeof (ut->ut_user)] = '\0';

	time (&now);

	/*
	 * Check if they are allowed to be logged in right now.
	 */
	if (!isttytime (user, ut->ut_line, now))
		return 0;
	return 1;
}


static void send_mesg_to_tty (int tty_fd)
{
	TERMIO oldt, newt;
	FILE *mesg_file, *tty_file;
	int c, is_tty;

	tty_file = fdopen (tty_fd, "w");
	if (!tty_file)
		return;

	is_tty = (GTTY (tty_fd, &oldt) == 0);
	if (is_tty) {
		/* Suggested by Ivan Nejgebauar <ian@unsux.ns.ac.yu>:
		   set OPOST before writing the message. */
		newt = oldt;
		newt.c_oflag |= OPOST;
		STTY (tty_fd, &newt);
	}

	mesg_file = fopen (HUP_MESG_FILE, "r");
	if (mesg_file) {
		while ((c = getc (mesg_file)) != EOF) {
			if (c == '\n')
				putc ('\r', tty_file);
			putc (c, tty_file);
		}
		fclose (mesg_file);
	} else {
		fputs (DEFAULT_HUP_MESG, tty_file);
	}
	fflush (tty_file);
	fclose (tty_file);

	if (is_tty) {
		STTY (tty_fd, &oldt);
	}
}


/*
 * logoutd - logout daemon to enforce /etc/porttime file policy
 *
 *	logoutd is started at system boot time and enforces the login
 *	time and port restrictions specified in /etc/porttime. The
 *	utmpx/utmp file is periodically scanned and offending users are logged
 *	off from the system.
 */
int main (int argc, char **argv)
{
	int i;
	int status;
	pid_t pid;

#if HAVE_UTMPX_H
	struct utmpx *ut;
#else
	struct utmp *ut;
#endif
	char user[sizeof (ut->ut_user) + 1];	/* terminating NUL */
	char tty_name[sizeof (ut->ut_line) + 6];	/* /dev/ + NUL */
	int tty_fd;

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

#ifndef DEBUG
	for (i = 0; close (i) == 0; i++);

	setpgrp ();

	/*
	 * Put this process in the background.
	 */
	pid = fork ();
	if (pid > 0) {
		/* parent */
		exit (0);
	} else if (pid < 0) {
		/* error */
		perror ("fork");
		exit (1);
	}
#endif				/* !DEBUG */

	/*
	 * Start syslogging everything
	 */
	Prog = Basename (argv[0]);

	OPENLOG ("logoutd");

	/*
	 * Scan the utmpx/utmp file once per minute looking for users that
	 * are not supposed to still be logged in.
	 */
	while (1) {

		/* 
		 * Attempt to re-open the utmpx/utmp file. The file is only
		 * open while it is being used.
		 */
#if HAVE_UTMPX_H
		setutxent ();
#else
		setutent ();
#endif

		/*
		 * Read all of the entries in the utmpx/utmp file. The entries
		 * for login sessions will be checked to see if the user
		 * is permitted to be signed on at this time.
		 */
#if HAVE_UTMPX_H
		while ((ut = getutxent ())) {
#else
		while ((ut = getutent ())) {
#endif
#ifdef USER_PROCESS
			if (ut->ut_type != USER_PROCESS)
				continue;
#endif
			if (ut->ut_user[0] == '\0')
				continue;
			if (check_login (ut))
				continue;

			/*
			 * Put the rest of this in a child process. This
			 * keeps the scan from waiting on other ports to die.
			 */

			pid = fork ();
			if (pid > 0) {
				/* parent */
				continue;
			} else if (pid < 0) {
				/* failed - give up until the next scan */
				break;
			}
			/* child */

			if (strncmp (ut->ut_line, "/dev/", 5) != 0)
				strcpy (tty_name, "/dev/");
			else
				tty_name[0] = '\0';

			strcat (tty_name, ut->ut_line);
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif
			tty_fd =
			    open (tty_name, O_WRONLY | O_NDELAY | O_NOCTTY);
			if (tty_fd != -1) {
				send_mesg_to_tty (tty_fd);
				close (tty_fd);
				sleep (10);
			}
#ifdef USER_PROCESS		/* USG_UTMP */
			if (ut->ut_pid > 1) {
				kill (-ut->ut_pid, SIGHUP);
				sleep (10);
				kill (-ut->ut_pid, SIGKILL);
			}
#else				/* BSD || SUN || SUN4 */
			/*
			 * vhangup() the line to kill try and kill
			 * whatever is out there using it.
			 */
			if ((tty_fd =
			     open (tty_name, O_RDONLY | O_NDELAY)) == -1)
				continue;

			vhangup (tty_fd);
			close (tty_fd);
#endif				/* BSD || SUN || SUN4 */

			strncpy (user, ut->ut_user, sizeof (user) - 1);
			user[sizeof (user) - 1] = '\0';

			SYSLOG ((LOG_NOTICE,
				 "logged off user `%s' on `%s'", user,
				 tty_name));

			/*
			 * This child has done all it can, drop dead.
			 */
			exit (0);
		}

#if HAVE_UTMPX_H
		endutxent ();
#else
		endutent ();
#endif

#ifndef DEBUG
		sleep (60);
#endif
		/*
		 * Reap any dead babies ...
		 */
		while (wait (&status) != -1);
	}
	return 1;
	/* NOT REACHED */
}

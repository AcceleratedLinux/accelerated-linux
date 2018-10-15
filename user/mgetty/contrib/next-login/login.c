/*
 * Copyright (c) 1980, 1987, 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)login.c	5.40 (Berkeley) 5/9/89";
#endif /* not lint */

/*
 * login [ name ]
 * login -h hostname	(for telnetd, etc.)
 * login -f name	(for pre-authenticated login: datakit, xterm, etc.)
 */

#include <sys/param.h>
#ifdef NeXT
#include <sys/quota.h>
#else
#include <ufs/quota.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <utmp.h>
#ifdef NeXT
#include <lastlog.h>
#ifndef UT_NAMESIZE
#define UT_NAMESIZE 8
#endif
#endif
#include <signal.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <grp.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdio.h>
#include <strings.h>
#include <tzfile.h>
#include "pathnames.h"

#ifdef	KERBEROS
#include <kerberos/krb.h>
#include <sys/termios.h>
char	realm[REALM_SZ];
int	kerror = KSUCCESS, notickets = 1;
#endif

#define	TTYGRPNAME	"tty"		/* name of group to own ttys */

/*
 * This bounds the time given to login.  Not a define so it can
 * be patched on machines where it's too small.
 */
int	timeout = 300;

struct	passwd *pwd;
int	failures;
char	term[64], *hostname, *username, *tty;

struct	sgttyb sgttyb;
struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

#ifndef NeXT
/* NeXT doesn't ifdef this out for some reason */
char *months[] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
	  "Sep", "Oct", "Nov", "Dec" };
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno, optind;
	extern char *optarg, **environ;
	struct timeval tp;
	struct tm *ttp;
	struct group *gr;
	register int ch;
	register char *p;
	int ask, fflag, hflag, pflag, cnt;
	int quietlog, passwd_req, ioctlval, timedout();
	char *domain, *salt, *envinit[1], *ttyn, *pp;
	char tbuf[MAXPATHLEN + 2], tname[sizeof(_PATH_TTY) + 10];
	char *ctime(), *ttyname(), *stypeof(), *crypt(), *getpass();
	time_t time();
	off_t lseek();

	(void)signal(SIGALRM, timedout);
	(void)alarm((u_int)timeout);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)setpriority(PRIO_PROCESS, 0, 0);
#ifndef NeXT
	(void)quota(Q_SETUID, 0, 0, 0);
#endif

	/*
	 * -p is used by getty to tell login not to destroy the environment
 	 * -f is used to skip a second login authentication
	 * -h is used by other servers to pass the name of the remote
	 *    host to login so that it may be placed in utmp and wtmp
	 */
	(void)gethostname(tbuf, sizeof(tbuf));
	domain = index(tbuf, '.');

	fflag = hflag = pflag = 0;
	passwd_req = 1;
	while ((ch = getopt(argc, argv, "fh:p")) != EOF)
		switch (ch) {
		case 'f':
			fflag = 1;
			break;
		case 'h':
			if (getuid()) {
				(void)fprintf(stderr,
				    "login: -h for super-user only.\n");
				exit(1);
			}
			hflag = 1;
			if (domain && (p = index(optarg, '.')) &&
			    strcasecmp(p, domain) == 0)
				*p = 0;
			hostname = optarg;
			break;
		case 'p':
			pflag = 1;
			break;
		case '?':
		default:
			(void)fprintf(stderr,
			    "usage: login [-fp] [username]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;
	if (*argv) {
		username = *argv;
		ask = 0;
	} else
		ask = 1;

	ioctlval = 0;
	(void)ioctl(0, TIOCLSET, &ioctlval);
	(void)ioctl(0, TIOCNXCL, 0);
	(void)fcntl(0, F_SETFL, ioctlval);
	(void)ioctl(0, TIOCGETP, &sgttyb);
	sgttyb.sg_erase = CERASE;
	sgttyb.sg_kill = CKILL;
	(void)ioctl(0, TIOCSLTC, &ltc);
	(void)ioctl(0, TIOCSETC, &tc);
	(void)ioctl(0, TIOCSETP, &sgttyb);

#if defined(NeXT) && defined(FIX8BIT)
       NeXT_repair_line(0);
#endif

	for (cnt = getdtablesize(); cnt > 2; cnt--)
		close(cnt);

	ttyn = ttyname(0);
	if (ttyn == NULL || *ttyn == '\0') {
		(void)sprintf(tname, "%s??", _PATH_TTY);
		ttyn = tname;
	}
	if (tty = rindex(ttyn, '/'))
		++tty;
	else
		tty = ttyn;

	openlog("login", LOG_ODELAY, LOG_AUTH);

	for (cnt = 0;; ask = 1) {
		ioctlval = 0;
		(void)ioctl(0, TIOCSETD, &ioctlval);

		if (ask) {
			fflag = 0;
			getloginname();
		}
		/*
		 * Note if trying multiple user names;
		 * log failures for previous user name,
		 * but don't bother logging one failure
		 * for nonexistent name (mistyped username).
		 */
		if (failures && strcmp(tbuf, username)) {
			if (failures > (pwd ? 0 : 1))
				badlogin(tbuf);
			failures = 0;
		}
		(void)strcpy(tbuf, username);
		if (pwd = getpwnam(username))
			salt = pwd->pw_passwd;
		else
			salt = "xx";

		/* if user not super-user, check for disabled logins */
		if (pwd == NULL || pwd->pw_uid)
			checknologin();

		/*
		 * Disallow automatic login to root; if not invoked by
		 * root, disallow if the uid's differ.
		 */
		if (fflag && pwd) {
			int uid = getuid();

			passwd_req = pwd->pw_uid == 0 ||
			    (uid && uid != pwd->pw_uid);
		}
#ifdef NeXT
		if (pwd->pw_uid == 0 && !rootterm(tty)) {
			fprintf(stderr, "%s login refused on this terminal.\n",
				pwd->pw_name);
			if (hostname)
				syslog(LOG_NOTICE,
					"LOGIN %s REFUSED FROM %s ON TTY %s",
					pwd->pw_name, hostname, tty);
			else
				syslog(LOG_NOTICE,
					"LOGIN %s REFUSED ON TTY %s",
					pwd->pw_name, tty);
			continue;
		}
#endif


		/*
		 * If no pre-authentication and a password exists
		 * for this user, prompt for one and verify it.
		 */
		if (!passwd_req || (pwd && !*pwd->pw_passwd))
			break;

		setpriority(PRIO_PROCESS, 0, -4);
		pp = getpass("Password:");
		p = crypt(pp, salt);
		setpriority(PRIO_PROCESS, 0, 0);

#ifdef	KERBEROS

		/*
		 * If not present in pw file, act as we normally would.
		 * If we aren't Kerberos-authenticated, try the normal
		 * pw file for a password.  If that's ok, log the user
		 * in without issueing any tickets.
		 */

		if (pwd && !krb_get_lrealm(realm,1)) {
			/*
			 * get TGT for local realm; be careful about uid's
			 * here for ticket file ownership
			 */
			(void)setreuid(geteuid(),pwd->pw_uid);
			kerror = krb_get_pw_in_tkt(pwd->pw_name, "", realm,
				"krbtgt", realm, DEFAULT_TKT_LIFE, pp);
			(void)setuid(0);
			if (kerror == INTK_OK) {
				bzero(pp, strlen(pp));
				notickets = 0;	/* user got ticket */
				break;
			}
		}
#endif
		(void) bzero(pp, strlen(pp));
		if (pwd && !strcmp(p, pwd->pw_passwd))
			break;

		(void)printf("Login incorrect\n");
		failures++;
		/* we allow 10 tries, but after 3 we start backing off */
		if (++cnt > 3) {
			if (cnt >= 10) {
				badlogin(username);
				(void)ioctl(0, TIOCHPCL, (struct sgttyb *)NULL);
				sleepexit(1);
			}
			sleep((u_int)((cnt - 3) * 5));
		}
	}

	/* committed to login -- turn off timeout */
	(void)alarm((u_int)0);

	/* paranoia... */
	endpwent();

#ifndef NeXT
	/*
	 * If valid so far and root is logging in, see if root logins on
	 * this terminal are permitted.
	 */
	if (pwd->pw_uid == 0 && !rootterm(tty)) {
		if (hostname)
			syslog(LOG_NOTICE, "ROOT LOGIN REFUSED FROM %s",
			    hostname);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN REFUSED ON %s", tty);
		(void)printf("Login incorrect\n");
		sleepexit(1);
	}

	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		switch(errno) {
		case EUSERS:
			(void)fprintf(stderr,
		"Too many users logged on already.\nTry again later.\n");
			break;
		case EPROCLIM:
			(void)fprintf(stderr,
			    "You have too many processes running.\n");
			break;
		default:
			perror("quota (Q_SETUID)");
		}
		sleepexit(0);
	}
#endif

	if (chdir(pwd->pw_dir) < 0) {
		(void)printf("No directory %s!\n", pwd->pw_dir);
		if (chdir("/"))
			exit(0);
		pwd->pw_dir = "/";
		(void)printf("Logging in with home = \"/\".\n");
	}

	quietlog = access(_PATH_HUSHLOGIN, F_OK) == 0;

#ifdef KERBEROS
	if (notickets && !quietlog)
		(void)printf("Warning: no Kerberos tickets issued\n");
#endif

#ifndef NeXT
#define	TWOWEEKS	(14*24*60*60)
	if (pwd->pw_change || pwd->pw_expire)
		(void)gettimeofday(&tp, (struct timezone *)NULL);
	if (pwd->pw_change)
		if (tp.tv_sec >= pwd->pw_change) {
			(void)printf("Sorry -- your password has expired.\n");
			sleepexit(1);
		}
		else if (tp.tv_sec - pwd->pw_change < TWOWEEKS && !quietlog) {
			ttp = localtime(&pwd->pw_change);
			(void)printf("Warning: your password expires on %s %d, %d\n",
			    months[ttp->tm_mon], ttp->tm_mday, TM_YEAR_BASE + ttp->tm_year);
		}
	if (pwd->pw_expire)
		if (tp.tv_sec >= pwd->pw_expire) {
			(void)printf("Sorry -- your account has expired.\n");
			sleepexit(1);
		}
		else if (tp.tv_sec - pwd->pw_expire < TWOWEEKS && !quietlog) {
			ttp = localtime(&pwd->pw_expire);
			(void)printf("Warning: your account expires on %s %d, %d\n",
			    months[ttp->tm_mon], ttp->tm_mday, TM_YEAR_BASE + ttp->tm_year);
		}
#endif

	/* nothing else left to fail -- really log in */
	{
#ifdef NeXT
		void login();
#endif
		struct utmp utmp;

		bzero((char *)&utmp, sizeof(utmp));
		(void)time(&utmp.ut_time);
		strncpy(utmp.ut_name, username, sizeof(utmp.ut_name));
		if (hostname)
			strncpy(utmp.ut_host, hostname, sizeof(utmp.ut_host));
		strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
		login(&utmp);
	}

	dolastlog(quietlog);

	if (!hflag) {					/* XXX */
		static struct winsize win = { 0, 0, 0, 0 };

		(void)ioctl(0, TIOCSWINSZ, &win);
	}

	(void)chown(ttyn, pwd->pw_uid,
	    (gr = getgrnam(TTYGRPNAME)) ? gr->gr_gid : pwd->pw_gid);
	(void)chmod(ttyn, 0620);
	(void)setgid(pwd->pw_gid);

	initgroups(username, pwd->pw_gid);

#ifndef NeXT
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
#endif

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = _PATH_BSHELL;
	/* turn on new line discipline for the csh */
	else if (!strcmp(pwd->pw_shell, _PATH_CSHELL)) {
		ioctlval = NTTYDISC;
		(void)ioctl(0, TIOCSETD, &ioctlval);
	}
	
/* real NeXT source diverges here */
	/* destroy environment unless user has requested preservation */
#ifdef NeXT
	if (!pflag){
		envinit[0]=(char *)NULL;
		environ = envinit;
	}
#else
	if (!pflag)
		environ = envinit;
#endif
	(void)setenv("HOME", pwd->pw_dir, 1);
	(void)setenv("SHELL", pwd->pw_shell, 1);
	if (term[0] == '\0')
		strncpy(term, stypeof(tty), sizeof(term));
	(void)setenv("TERM", term, 0);
	(void)setenv("USER", pwd->pw_name, 1);
	(void)setenv("PATH", _PATH_DEFPATH, 0);
/* real NeXT source merges here */

	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
	if (pwd->pw_uid == 0)
		if (hostname)
			syslog(LOG_NOTICE, "ROOT LOGIN ON %s FROM %s",
			    tty, hostname);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN ON %s", tty);

	if (!quietlog) {
		struct stat st;

		motd();
		(void)sprintf(tbuf, "%s/%s", _PATH_MAILDIR, pwd->pw_name);
		if (stat(tbuf, &st) == 0 && st.st_size != 0)
			(void)printf("You have %smail.\n",
			    (st.st_mtime > st.st_atime) ? "new " : "");
	}

	(void)signal(SIGALRM, SIG_DFL);
	(void)signal(SIGQUIT, SIG_DFL);
	(void)signal(SIGINT, SIG_DFL);
	(void)signal(SIGTSTP, SIG_IGN);

	tbuf[0] = '-';
	strcpy(tbuf + 1, (p = rindex(pwd->pw_shell, '/')) ?
	    p + 1 : pwd->pw_shell);

#ifndef NeXT
	if (setlogname(pwd->pw_name, strlen(pwd->pw_name)) < 0)
		syslog(LOG_ERR, "setlogname() failure: %m");
#endif

	/* discard permissions last so can't get killed and drop core */
	(void)setuid(pwd->pw_uid);

#if defined(NeXT) && !defined(ORIGINAL)
	execlp(pwd->pw_shell, tbuf,	/* enable job control! */
		strcmp(pwd->pw_shell, _PATH_BSHELL) ? 0 : "-J", 0);
#else
	execlp(pwd->pw_shell, tbuf, 0);
#endif
	(void)fprintf(stderr, "login: no shell: %s.\n", strerror(errno));
	exit(0);
}

getloginname()
{
	register int ch;
	register char *p;
	static char nbuf[UT_NAMESIZE + 1];

	for (;;) {
		(void)printf("login: ");
		for (p = nbuf; (ch = getchar()) != '\n'; ) {
			if (ch == EOF) {
				badlogin(username);
				exit(0);
			}
			if (p < nbuf + UT_NAMESIZE)
				*p++ = ch;
		}
		if (p > nbuf)
			if (nbuf[0] == '-')
				(void)fprintf(stderr,
				    "login names may not start with '-'.\n");
			else {
				*p = '\0';
				username = nbuf;
				break;
			}
	}
}

timedout()
{
	(void)fprintf(stderr, "Login timed out after %d seconds\n", timeout);
	exit(0);
}

rootterm(ttyn)
	char *ttyn;
{
	struct ttyent *t;

	return((t = getttynam(ttyn)) && t->ty_status&TTY_SECURE);
}

jmp_buf motdinterrupt;

motd()
{
	register int fd, nchars;
	int (*oldint)(), sigint();
	char tbuf[8192];

	if ((fd = open(_PATH_MOTDFILE, O_RDONLY, 0)) < 0)
		return;
	oldint = signal(SIGINT, sigint);
	if (setjmp(motdinterrupt) == 0)
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
	(void)signal(SIGINT, oldint);
	(void)close(fd);
}

sigint()
{
	longjmp(motdinterrupt, 1);
}

checknologin()
{
	register int fd, nchars;
	char tbuf[8192];

	if ((fd = open(_PATH_NOLOGIN, O_RDONLY, 0)) >= 0) {
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
		sleepexit(0);
	}
}

dolastlog(quiet)
	int quiet;
{
	struct lastlog ll;
	int fd;
	char *ctime();

	if ((fd = open(_PATH_LASTLOG, O_RDWR, 0)) >= 0) {
		(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		if (!quiet) {
			if (read(fd, (char *)&ll, sizeof(ll)) == sizeof(ll) &&
			    ll.ll_time != 0) {
				(void)printf("Last login: %.*s ",
				    24-5, (char *)ctime(&ll.ll_time));
				if (*ll.ll_host != '\0')
					(void)printf("from %.*s\n",
					    sizeof(ll.ll_host), ll.ll_host);
				else
					(void)printf("on %.*s\n",
					    sizeof(ll.ll_line), ll.ll_line);
			}
			(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		}
		bzero((char *)&ll, sizeof(ll));
		(void)time(&ll.ll_time);
		strncpy(ll.ll_line, tty, sizeof(ll.ll_line));
		if (hostname)
			strncpy(ll.ll_host, hostname, sizeof(ll.ll_host));
		(void)write(fd, (char *)&ll, sizeof(ll));
		(void)close(fd);
	}
}

badlogin(name)
	char *name;
{
	if (failures == 0)
		return;
	if (hostname)
		syslog(LOG_NOTICE, "%d LOGIN FAILURE%s FROM %s, %s",
		    failures, failures > 1 ? "S" : "", hostname, name);
	else
		syslog(LOG_NOTICE, "%d LOGIN FAILURE%s ON %s, %s",
		    failures, failures > 1 ? "S" : "", tty, name);
}

#undef	UNKNOWN
#define	UNKNOWN	"su"

char *
stypeof(ttyid)
	char *ttyid;
{
	struct ttyent *t;

	return(ttyid && (t = getttynam(ttyid)) ? t->ty_type : UNKNOWN);
}

/* NeXT has their version of setenv here */

getstr(buf, cnt, err)
	char *buf, *err;
	int cnt;
{
	char ch;

	do {
		if (read(0, &ch, sizeof(ch)) != sizeof(ch))
			exit(1);
		if (--cnt < 0) {
			(void)fprintf(stderr, "%s too long\r\n", err);
			sleepexit(1);
		}
		*buf++ = ch;
	} while (ch);
}

sleepexit(eval)
	int eval;
{
	sleep((u_int)5);
	exit(eval);
}

#ifdef NeXT
/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)login.c	5.1 (Berkeley) 9/27/88";
#endif /* LIBC_SCCS and not lint */

#ifdef NOTDEF
/* included above */
#include <sys/types.h>
#include <sys/file.h>
#include <utmp.h>
#include <stdio.h>
#endif

#define	UTMPFILE	"/etc/utmp"
#define	WTMPFILE	"/usr/adm/wtmp"

void
login(ut)
	struct utmp *ut;
{
	register int fd;
	int tty;
	off_t lseek();

	tty = ttyslot();
	if (tty > 0 && (fd = open(UTMPFILE, O_WRONLY, 0)) >= 0) {
		(void)lseek(fd, (long)(tty * sizeof(struct utmp)), L_SET);
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
	if ((fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
}
#endif

#if defined(NeXT) && defined(FIX8BIT)
NeXT_repair_line(int fd)
{
    int             bitset = LPASS8 | LPASS8OUT;
    int             bitclr = LNOHANG;

    ioctl(fd, TIOCLBIS, &bitset);
    ioctl(fd, TIOCLBIC, &bitclr);
}
#endif

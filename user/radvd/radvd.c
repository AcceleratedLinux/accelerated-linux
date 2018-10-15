/*
 *   $Id: radvd.c,v 1.12 2001/12/28 08:39:54 psavola Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <pathnames.h>

struct Interface *IfaceList = NULL;

char usage_str[] =
	"[-vhn] [-d level] [-C config_file] [-m log_method] [-l log_file]\n"
	"\t[-f facility] [-p pid_file] [-u username] [-t chrootdir]";

#ifdef HAVE_GETOPT_LONG
struct option prog_opt[] = {
	{"debug", 1, 0, 'd'},
	{"config", 1, 0, 'C'},
	{"pidfile", 1, 0, 'p'},
	{"logfile", 1, 0, 'l'},
	{"logmethod", 1, 0, 'm'},
	{"facility", 1, 0, 'f'},
	{"username", 1, 0, 'u'},
	{"chrootdir", 1, 0, 't'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{NULL, 0, 0, 0}
};
#endif

extern FILE *yyin;

char *conf_file = NULL;
char *pidfile;
char *pname;
int sock = -1;

volatile int sighup_received = 0;
volatile int sigterm_received = 0;
volatile int sigint_received = 0;

void sighup_handler(int sig);
void sigterm_handler(int sig);
void sigint_handler(int sig);
void timer_handler(void *data);
void kickoff_adverts(void);
void reload_config(void);
void version(void);
void usage(void);
int drop_root_privileges(const char *);
int readin_config(char *);
int check_ip6_forwarding();
int check_conffile_perm(const char *, const char *);

int
main(int argc, char *argv[])
{
	unsigned char msg[MSG_SIZE];
	char pidstr[16];
	int c, log_method;
	char *logfile;
	int facility, fd;
	char *username = NULL;
	char *chrootdir = NULL;
	int dodaemon = 1;
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
#endif

	pname = ((pname=strrchr(argv[0],'/')) != NULL)?pname+1:argv[0];

	srand((unsigned int)time(NULL));

	log_method = L_SYSLOG;
	logfile = PATH_RADVD_LOG;
	conf_file = PATH_RADVD_CONF;
	facility = LOG_FACILITY;
	pidfile = PATH_RADVD_PID;

	/* parse args */
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "d:C:l:m:np:t:u:vh", prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, "d:C:l:m:np:t:u:vh")) > 0)
#endif
	{
		switch (c) {
		case 'C':
			conf_file = optarg;
			break;
		case 'd':
			set_debuglevel(atoi(optarg));
			break;
		case 'f':
			facility = atoi(optarg);
			break;
		case 'l':
			logfile = optarg;
			break;
		case 'p':
			pidfile = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog"))
			{
				log_method = L_SYSLOG;
			}
			else if (!strcmp(optarg, "stderr"))
			{
				log_method = L_STDERR;
			}
			else if (!strcmp(optarg, "logfile"))
			{
				log_method = L_LOGFILE;
			}
			else if (!strcmp(optarg, "none"))
			{
				log_method = L_NONE;
			}
			else
			{
				fprintf(stderr, "%s: unknown log method: %s\n", pname, optarg);
				exit(1);
			}
			break;
		case 'n':
			dodaemon = 0;
			break;
		case 't':
			chrootdir = strdup(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
		case 'v':
			version();
			break;
		case 'h':
			usage();
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname,
				prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}

	if (chrootdir) {
		if (!username) {
			fprintf(stderr, "Chroot as root is not safe, exiting\n");
			exit(1);
		}
		
		if (chroot(chrootdir) == -1) {
			perror("chroot");
			exit (1);
		}
		
		chdir("/");
	
		/* username will be switched later */
	}
	
	if (log_open(log_method, pname, logfile, facility) < 0)
		exit(1);

	log(LOG_INFO, "version %s started", VERSION);

	/* get a raw socket for sending and receiving ICMPv6 messages */
	sock = open_icmpv6_socket();
	if (sock < 0)
		exit(1);

	/* drop root privileges if requested. */
	if (username) {
		if (drop_root_privileges(username) < 0)
			exit(1);
	}

	/* check that 'other' cannot write the file
         * for non-root, also that self/own group can't either
         */
	if (check_conffile_perm(username, conf_file) < 0) {
		if (get_debuglevel() == 0)
			exit(1);
		else
			log(LOG_WARNING, "Insecure file permissions, but continuing anyway");
	}
	
	/* if we know how to do it, check whether forwarding is enabled */
	if (dodaemon && check_ip6_forwarding()) {
		if (get_debuglevel() == 0) {
			log(LOG_ERR, "IPv6 forwarding seems to be disabled, exiting");
			exit(1);
		}
		else
			log(LOG_WARNING, "IPv6 forwarding seems to be disabled, but continuing anyway.");
	}

	/* parse config file */
	if (readin_config(conf_file) < 0)
		exit(1);

	/* FIXME: not atomic if pidfile is on an NFS mounted volume */	
	if ((fd = open(pidfile, O_CREAT|O_EXCL|O_WRONLY, 0644)) < 0)
	{
		log(LOG_ERR, "another radvd seems to be already running, terminating");
		exit(1);
	}
	
	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */

	if (get_debuglevel() == 0 && dodaemon) {

		/* Detach from controlling terminal */
		if (daemon(0, 0) < 0)
			perror("daemon");

		/*
		 * reopen logfile, so that we get the process id right in the syslog
		 */
		if (log_reopen() < 0) {
			unlink(pidfile);
			exit(1);
		}

	}

	/*
	 *	config signal handlers
	 */

	signal(SIGHUP, sighup_handler);
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigint_handler);

	snprintf(pidstr, sizeof(pidstr), "%d\n", getpid());
	
	write(fd, pidstr, strlen(pidstr));
	
	close(fd);

	kickoff_adverts();

	/* enter loop */

	for (;;)
	{
		int len, hoplimit;
		struct sockaddr_in6 rcv_addr;
		struct in6_pktinfo *pkt_info = NULL;
		
		len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit);
		if (len > 0)
			process(sock, IfaceList, msg, len, 
				&rcv_addr, pkt_info, hoplimit);

		if (sigterm_received || sigint_received)
			break;

		if (sighup_received)
		{
			reload_config();		
			sighup_received = 0;
		}
	}
	
	unlink(pidfile);
	exit(0);
}

void
timer_handler(void *data)
{
	struct Interface *iface = (struct Interface *) data;
	double next;

	dlog(LOG_DEBUG, 4, "timer_handler called for %s", iface->Name);

	send_ra(sock, iface, NULL);

	next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval); 
	set_timer(&iface->tm, next);
}

void
kickoff_adverts(void)
{
	struct Interface *iface;

	/*
	 *	send initial advertisement and set timers
	 */

	for(iface=IfaceList; iface; iface=iface->next)
	{
		if( ! iface->UnicastOnly )
		{
			init_timer(&iface->tm, timer_handler, (void *) iface);
			if (iface->AdvSendAdvert)
			{
				/* send an initial advertisement */
				send_ra(sock, iface, NULL);

				set_timer(&iface->tm, iface->MaxRtrAdvInterval);
			}
		}
	}
}

void reload_config(void)
{
	struct Interface *iface;

	log(LOG_INFO, "attempting to reread config file");

	dlog(LOG_DEBUG, 4, "reopening log");
	if (log_reopen() < 0) {
		unlink(pidfile);
		exit(1);
	}

	/* disable timers, free interface and prefix structures */
	for(iface=IfaceList; iface; iface=iface->next)
	{
		dlog(LOG_DEBUG, 4, "disabling timer for %s", iface->Name);
		clear_timer(&iface->tm);
	}

	iface=IfaceList; 
	while(iface)
	{
		struct Interface *next_iface = iface->next;
		struct AdvPrefix *prefix;

		dlog(LOG_DEBUG, 4, "freeing interface %s", iface->Name);
		
		prefix = iface->AdvPrefixList;
		while (prefix)
		{
			struct AdvPrefix *next_prefix = prefix->next;
			
			free(prefix);
			prefix = next_prefix;
		}
		
		free(iface);
		iface = next_iface;
	}

	IfaceList = NULL;

	/* reread config file */
	if (readin_config(conf_file) < 0) {
		unlink(pidfile);
		exit(1);
	}

	kickoff_adverts();

	log(LOG_INFO, "resuming normal operation");
}

void
sighup_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGHUP, sighup_handler);

	dlog(LOG_DEBUG, 4, "sighup_handler called");

	sighup_received = 1;
}

void
sigterm_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGTERM, sigterm_handler);

	dlog(LOG_DEBUG, 4, "sigterm_handler called");

	sigterm_received = 1;
}

void
sigint_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGINT, sigint_handler);

	dlog(LOG_DEBUG, 4, "sigint_handler called");

	sigint_received = 1;
}

int
drop_root_privileges(const char *username)
{
	struct passwd *pw = NULL;
	pw = getpwnam(username);
	if (pw) {
		if (initgroups(username, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
			log(LOG_ERR, "Couldn't change to '%.32s' uid=%d gid=%d\n", 
					username, pw->pw_uid, pw->pw_gid);
			return (-1);
		}
	}
	else {
		log(LOG_ERR, "Couldn't find user '%.32s'\n", username);
		return (-1);
	}
	return 0;
}

int
check_conffile_perm(const char *username, const char *conf_file)
{
	struct stat *st = NULL;
	struct passwd *pw = NULL;
	FILE *fp = fopen(conf_file, "r");

	if (fp == NULL) {
		log(LOG_ERR, "can't open %s: %s", conf_file, strerror(errno));
		return (-1);
	}
	fclose(fp);

	st = malloc(sizeof(struct stat));
	if (st == NULL)
		goto errorout;

	if (!username)
		username = "root";
	
	pw = getpwnam(username);

	if (stat(conf_file, st) || pw == NULL)
		goto errorout;

	if (st->st_mode & S_IWOTH) {
                log(LOG_ERR, "Insecure file permissions (writable by others): %s", conf_file);
		goto errorout;
        }

	/* for non-root: must not be writable by self/own group */
	if (strncmp(username, "root", 5) != 0 &&
	    ((st->st_mode & S_IWGRP && pw->pw_gid == st->st_gid) ||
	     (st->st_mode & S_IWUSR && pw->pw_uid == st->st_uid))) {
                log(LOG_ERR, "Insecure file permissions (writable by self/group): %s", conf_file);
		goto errorout;
        }

	free(st);
        return 0;

errorout:
	free(st);
	return(-1);
}

int
check_ip6_forwarding(void)
{
	int forw_sysctl[] = { SYSCTL_IP6_FORWARDING };
	int value;
	size_t size = sizeof(value);

	if (sysctl(forw_sysctl, sizeof(forw_sysctl)/sizeof(forw_sysctl[0]),
	    &value, &size, NULL, 0) < 0) {
		log(LOG_DEBUG, "Correct IPv6 forwarding sysctl branch not found, "
			"perhaps the kernel interface has changed?");
		return(0);	/* this is of advisory value only */
	}
	
	if (value != 1) {
		log(LOG_DEBUG, "IPv6 forwarding setting is: %u, should be 1", value);
		return(-1);
	}
		
	return(0);
}

int
readin_config(char *fname)
{
	if ((yyin = fopen(fname, "r")) == NULL)
	{
		log(LOG_ERR, "can't open %s: %s", fname, strerror(errno));
		return (-1);
	}

	if (yyparse() != 0)
	{
		log(LOG_ERR, "syntax error in config file: %s", fname);
		return (-1);
	}
	
	fclose(yyin);
	return 0;
}

void
version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default pidfile		\"%s\"\n", PATH_RADVD_PID);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facililty	%d\n", LOG_FACILITY);
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n",
		CONTACT_EMAIL);

	exit(1);	
}

void
usage(void)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);	
}




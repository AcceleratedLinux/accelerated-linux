/* -*- c-file-style: "linux"; -*-
 *
 * Copyright (C) 1998-2001 by Andrew Tridgell <tridge@samba.org>
 * Copyright (C) 2001-2002 by Martin Pool <mbp@samba.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @file
 *
 * The socket based protocol for setting up a connection with
 * rsyncd.
 **/

#include "rsync.h"

extern int verbose;
extern int list_only;
extern int am_sender;
extern int am_server;
extern int am_daemon;
extern int am_root;
extern int rsync_port;
extern int kluge_around_eof;
extern int daemon_over_rsh;
extern int sanitize_paths;
extern int filesfrom_fd;
extern int remote_protocol;
extern int protocol_version;
extern int io_timeout;
extern int orig_umask;
extern int no_detach;
extern int default_af_hint;
extern char *bind_address;
extern struct filter_list_struct server_filter_list;
extern char *config_file;
extern char *files_from;

char *auth_user;
int read_only = 0;
int daemon_log_format_has_i = 0;
int daemon_log_format_has_o_or_i = 0;
int module_id = -1;

/* Length of lp_path() string when in daemon mode & not chrooted, else 0. */
unsigned int module_dirlen = 0;

/**
 * Run a client connected to an rsyncd.  The alternative to this
 * function for remote-shell connections is do_cmd().
 *
 * After negotiating which module to use and reading the server's
 * motd, this hands over to client_run().  Telling the server the
 * module will cause it to chroot/setuid/etc.
 *
 * Instead of doing a transfer, the client may at this stage instead
 * get a listing of remote modules and exit.
 *
 * @return -1 for error in startup, or the result of client_run().
 * Either way, it eventually gets passed to exit_cleanup().
 **/
int start_socket_client(char *host, char *path, int argc, char *argv[])
{
	int fd, ret;
	char *p, *user = NULL;

	/* This is redundant with code in start_inband_exchange(), but this
	 * short-circuits a problem in the client before we open a socket,
	 * and the extra check won't hurt. */
	if (*path == '/') {
		rprintf(FERROR,
			"ERROR: The remote path must start with a module name not a /\n");
		return -1;
	}

	if ((p = strrchr(host, '@')) != NULL) {
		user = host;
		host = p+1;
		*p = '\0';
	}

	fd = open_socket_out_wrapped(host, rsync_port, bind_address,
				     default_af_hint);
	if (fd == -1)
		exit_cleanup(RERR_SOCKETIO);

	ret = start_inband_exchange(user, path, fd, fd, argc);

	return ret ? ret : client_run(fd, fd, -1, argc, argv);
}

int start_inband_exchange(char *user, char *path, int f_in, int f_out, 
			  int argc)
{
	int i;
	char *sargs[MAX_ARGS];
	int sargc = 0;
	char line[MAXPATHLEN];
	char *p;

	if (argc == 0 && !am_sender)
		list_only |= 1;

	if (*path == '/') {
		rprintf(FERROR,
			"ERROR: The remote path must start with a module name\n");
		return -1;
	}

	if (!user)
		user = getenv("USER");
	if (!user)
		user = getenv("LOGNAME");

	io_printf(f_out, "@RSYNCD: %d\n", protocol_version);

	if (!read_line(f_in, line, sizeof line - 1)) {
		rprintf(FERROR, "rsync: did not see server greeting\n");
		return -1;
	}

	if (sscanf(line,"@RSYNCD: %d", &remote_protocol) != 1) {
		/* note that read_line strips of \n or \r */
		rprintf(FERROR, "rsync: server sent \"%s\" rather than greeting\n",
			line);
		return -1;
	}
	if (protocol_version > remote_protocol)
		protocol_version = remote_protocol;

	if (list_only && protocol_version >= 29)
		list_only |= 2;

	/* set daemon_over_rsh to false since we need to build the
	 * true set of args passed through the rsh/ssh connection;
	 * this is a no-op for direct-socket-connection mode */
	daemon_over_rsh = 0;
	server_options(sargs, &sargc);

	sargs[sargc++] = ".";

	if (path && *path)
		sargs[sargc++] = path;

	sargs[sargc] = NULL;

	if (verbose > 1)
		print_child_argv(sargs);

	p = strchr(path,'/');
	if (p) *p = 0;
	io_printf(f_out, "%s\n", path);
	if (p) *p = '/';

	/* Old servers may just drop the connection here,
	 rather than sending a proper EXIT command.  Yuck. */
	kluge_around_eof = list_only && protocol_version < 25 ? 1 : 0;

	while (1) {
		if (!read_line(f_in, line, sizeof line - 1)) {
			rprintf(FERROR, "rsync: didn't get server startup line\n");
			return -1;
		}

		if (strncmp(line,"@RSYNCD: AUTHREQD ",18) == 0) {
			auth_client(f_out, user, line+18);
			continue;
		}

		if (strcmp(line,"@RSYNCD: OK") == 0)
			break;

		if (strcmp(line,"@RSYNCD: EXIT") == 0) {
			/* This is sent by recent versions of the
			 * server to terminate the listing of modules.
			 * We don't want to go on and transfer
			 * anything; just exit. */
			exit(0);
		}

		if (strncmp(line, "@ERROR", 6) == 0) {
			rprintf(FERROR, "%s\n", line);
			/* This is always fatal; the server will now
			 * close the socket. */
			return -1;
		}

		rprintf(FINFO, "%s\n", line);
	}
	kluge_around_eof = 0;

	for (i = 0; i < sargc; i++) {
		io_printf(f_out, "%s\n", sargs[i]);
	}
	io_printf(f_out, "\n");

	if (protocol_version < 23) {
		if (protocol_version == 22 || !am_sender)
			io_start_multiplex_in();
	}

	return 0;
}



static int rsync_module(int f_in, int f_out, int i)
{
	int argc = 0;
	int maxargs;
	char **argv;
	char **argp;
	char line[MAXPATHLEN];
	uid_t uid = (uid_t)-2;  /* canonically "nobody" */
	gid_t gid = (gid_t)-2;
	char *p;
	char *addr = client_addr(f_in);
	char *host = client_name(f_in);
	char *name = lp_name(i);
	int use_chroot = lp_use_chroot(i);
	int start_glob = 0;
	int ret;
	char *request = NULL;

	if (!allow_access(addr, host, lp_hosts_allow(i), lp_hosts_deny(i))) {
		rprintf(FLOG, "rsync denied on module %s from %s (%s)\n",
			name, host, addr);
		if (!lp_list(i))
			io_printf(f_out, "@ERROR: Unknown module '%s'\n", name);
		else {
			io_printf(f_out,
				  "@ERROR: access denied to %s from %s (%s)\n",
				  name, host, addr);
		}
		return -1;
	}

	if (am_daemon && am_server) {
		rprintf(FLOG, "rsync allowed access on module %s from %s (%s)\n",
			name, host, addr);
	}

	if (!claim_connection(lp_lock_file(i), lp_max_connections(i))) {
		if (errno) {
			rsyserr(FLOG, errno, "failed to open lock file %s",
				safe_fname(lp_lock_file(i)));
			io_printf(f_out, "@ERROR: failed to open lock file\n");
		} else {
			rprintf(FLOG, "max connections (%d) reached\n",
				lp_max_connections(i));
			io_printf(f_out, "@ERROR: max connections (%d) reached -- try again later\n",
				lp_max_connections(i));
		}
		return -1;
	}

	auth_user = auth_server(f_in, f_out, i, host, addr, "@RSYNCD: AUTHREQD ");

	if (!auth_user) {
		io_printf(f_out, "@ERROR: auth failed on module %s\n", name);
		return -1;
	}

	module_id = i;

	if (lp_read_only(i))
		read_only = 1;

	if (lp_transfer_logging(i)) {
		if (log_format_has(lp_log_format(i), 'i'))
			daemon_log_format_has_i = 1;
		if (daemon_log_format_has_i
		    || log_format_has(lp_log_format(i), 'o'))
			daemon_log_format_has_o_or_i = 1;
	}

	am_root = (MY_UID() == 0);

	if (am_root) {
		p = lp_uid(i);
		if (!name_to_uid(p, &uid)) {
			if (!isdigit(*(unsigned char *)p)) {
				rprintf(FLOG, "Invalid uid %s\n", p);
				io_printf(f_out, "@ERROR: invalid uid %s\n", p);
				return -1;
			}
			uid = atoi(p);
		}

		p = lp_gid(i);
		if (!name_to_gid(p, &gid)) {
			if (!isdigit(*(unsigned char *)p)) {
				rprintf(FLOG, "Invalid gid %s\n", p);
				io_printf(f_out, "@ERROR: invalid gid %s\n", p);
				return -1;
			}
			gid = atoi(p);
		}
	}

	/* TODO: If we're not root, but the configuration requests
	 * that we change to some uid other than the current one, then
	 * log a warning. */

	/* TODO: Perhaps take a list of gids, and make them into the
	 * supplementary groups. */

	if (use_chroot || (module_dirlen = strlen(lp_path(i))) == 1) {
		module_dirlen = 0;
		set_filter_dir("/", 1);
	} else
		set_filter_dir(lp_path(i), module_dirlen);

	p = lp_filter(i);
	parse_rule(&server_filter_list, p, MATCHFLG_WORD_SPLIT,
		   XFLG_ANCHORED2ABS);

	p = lp_include_from(i);
	parse_filter_file(&server_filter_list, p, MATCHFLG_INCLUDE,
	    XFLG_ANCHORED2ABS | XFLG_OLD_PREFIXES | XFLG_FATAL_ERRORS);

	p = lp_include(i);
	parse_rule(&server_filter_list, p,
		   MATCHFLG_INCLUDE | MATCHFLG_WORD_SPLIT,
		   XFLG_ANCHORED2ABS | XFLG_OLD_PREFIXES);

	p = lp_exclude_from(i);
	parse_filter_file(&server_filter_list, p, 0,
	    XFLG_ANCHORED2ABS | XFLG_OLD_PREFIXES | XFLG_FATAL_ERRORS);

	p = lp_exclude(i);
	parse_rule(&server_filter_list, p, MATCHFLG_WORD_SPLIT,
		   XFLG_ANCHORED2ABS | XFLG_OLD_PREFIXES);

	log_init();

	if (use_chroot) {
		/*
		 * XXX: The 'use chroot' flag is a fairly reliable
		 * source of confusion, because it fails under two
		 * important circumstances: running as non-root,
		 * running on Win32 (or possibly others).  On the
		 * other hand, if you are running as root, then it
		 * might be better to always use chroot.
		 *
		 * So, perhaps if we can't chroot we should just issue
		 * a warning, unless a "require chroot" flag is set,
		 * in which case we fail.
		 */
		if (chroot(lp_path(i))) {
			rsyserr(FLOG, errno, "chroot %s failed",
				safe_fname(lp_path(i)));
			io_printf(f_out, "@ERROR: chroot failed\n");
			return -1;
		}

		if (!push_dir("/")) {
			rsyserr(FLOG, errno, "chdir %s failed\n",
				safe_fname(lp_path(i)));
			io_printf(f_out, "@ERROR: chdir failed\n");
			return -1;
		}

	} else {
		if (!push_dir(lp_path(i))) {
			rsyserr(FLOG, errno, "chdir %s failed\n",
				safe_fname(lp_path(i)));
			io_printf(f_out, "@ERROR: chdir failed\n");
			return -1;
		}
		sanitize_paths = 1;
	}

	if (am_root) {
		/* XXXX: You could argue that if the daemon is started
		 * by a non-root user and they explicitly specify a
		 * gid, then we should try to change to that gid --
		 * this could be possible if it's already in their
		 * supplementary groups. */

		/* TODO: Perhaps we need to document that if rsyncd is
		 * started by somebody other than root it will inherit
		 * all their supplementary groups. */

		if (setgid(gid)) {
			rsyserr(FLOG, errno, "setgid %d failed", (int)gid);
			io_printf(f_out, "@ERROR: setgid failed\n");
			return -1;
		}
#ifdef HAVE_SETGROUPS
		/* Get rid of any supplementary groups this process
		 * might have inheristed. */
		if (setgroups(1, &gid)) {
			rsyserr(FLOG, errno, "setgroups failed");
			io_printf(f_out, "@ERROR: setgroups failed\n");
			return -1;
		}
#endif

		if (setuid(uid)) {
			rsyserr(FLOG, errno, "setuid %d failed", (int)uid);
			io_printf(f_out, "@ERROR: setuid failed\n");
			return -1;
		}

		am_root = (MY_UID() == 0);
	}

	io_printf(f_out, "@RSYNCD: OK\n");

	maxargs = MAX_ARGS;
	if (!(argv = new_array(char *, maxargs)))
		out_of_memory("rsync_module");
	argv[argc++] = "rsyncd";

	while (1) {
		if (!read_line(f_in, line, sizeof line - 1))
			return -1;

		if (!*line)
			break;

		p = line;

		if (argc == maxargs) {
			maxargs += MAX_ARGS;
			if (!(argv = realloc_array(argv, char *, maxargs)))
				out_of_memory("rsync_module");
		}
		if (!(argv[argc] = strdup(p)))
			out_of_memory("rsync_module");

		if (start_glob) {
			if (start_glob == 1) {
				request = strdup(p);
				start_glob++;
			}
			glob_expand(name, &argv, &argc, &maxargs);
		} else
			argc++;

		if (strcmp(line, ".") == 0)
			start_glob = 1;
	}

	verbose = 0; /* future verbosity is controlled by client options */
	argp = argv;
	ret = parse_arguments(&argc, (const char ***) &argp, 0);

	if (filesfrom_fd == 0)
		filesfrom_fd = f_in;

	if (request) {
		if (*auth_user) {
			rprintf(FLOG, "rsync %s %s from %s@%s (%s)\n",
				am_sender ? "on" : "to",
				request, auth_user, host, addr);
		} else {
			rprintf(FLOG, "rsync %s %s from %s (%s)\n",
				am_sender ? "on" : "to",
				request, host, addr);
		}
		free(request);
	}

#ifndef DEBUG
	/* don't allow the logs to be flooded too fast */
	if (verbose > lp_max_verbosity(i))
		verbose = lp_max_verbosity(i);
#endif

	if (protocol_version < 23
	    && (protocol_version == 22 || am_sender))
		io_start_multiplex_out();
	else if (!ret) {
		/* We have to get I/O multiplexing started so that we can
		 * get the error back to the client.  This means getting
		 * the protocol setup finished first in later versions. */
		setup_protocol(f_out, f_in);
		if (!am_sender) {
			/* Since we failed in our option parsing, we may not
			 * have finished parsing that the client sent us a
			 * --files-from option, so look for it manually.
			 * Without this, the socket would be in the wrong
			 * state for the upcoming error message. */
			if (!files_from) {
				int i;
				for (i = 0; i < argc; i++) {
					if (strncmp(argv[i], "--files-from", 12) == 0) {
						files_from = "";
						break;
					}
				}
			}
			if (files_from)
				write_byte(f_out, 0);
		}
		io_start_multiplex_out();
	}

	if (!ret) {
		option_error();
		msleep(400);
		exit_cleanup(RERR_UNSUPPORTED);
	}

	if (lp_timeout(i) && lp_timeout(i) > io_timeout)
		set_io_timeout(lp_timeout(i));

	start_server(f_in, f_out, argc, argp);

	return 0;
}

/* send a list of available modules to the client. Don't list those
   with "list = False". */
static void send_listing(int fd)
{
	int n = lp_numservices();
	int i;

	for (i = 0; i < n; i++) {
		if (lp_list(i))
			io_printf(fd, "%-15s\t%s\n", lp_name(i), lp_comment(i));
	}

	if (protocol_version >= 25)
		io_printf(fd,"@RSYNCD: EXIT\n");
}

/* this is called when a connection is established to a client
   and we want to start talking. The setup of the system is done from
   here */
int start_daemon(int f_in, int f_out)
{
	char line[200];
	char *motd;
	int i;

	io_set_sock_fds(f_in, f_out);

	if (!lp_load(config_file, 0))
		exit_cleanup(RERR_SYNTAX);

	log_init();

	if (!am_server) {
		set_socket_options(f_in, "SO_KEEPALIVE");
		set_socket_options(f_in, lp_socket_options());
		set_nonblocking(f_in);
	}

	io_printf(f_out, "@RSYNCD: %d\n", protocol_version);

	motd = lp_motd_file();
	if (motd && *motd) {
		FILE *f = fopen(motd,"r");
		while (f && !feof(f)) {
			int len = fread(line, 1, sizeof line - 1, f);
			if (len > 0) {
				line[len] = 0;
				io_printf(f_out, "%s", line);
			}
		}
		if (f)
			fclose(f);
		io_printf(f_out, "\n");
	}

	if (!read_line(f_in, line, sizeof line - 1))
		return -1;

	if (sscanf(line,"@RSYNCD: %d", &remote_protocol) != 1) {
		io_printf(f_out, "@ERROR: protocol startup error\n");
		return -1;
	}
	if (protocol_version > remote_protocol)
		protocol_version = remote_protocol;

	line[0] = 0;
	if (!read_line(f_in, line, sizeof line - 1))
		return -1;

	if (!*line || strcmp(line, "#list") == 0) {
		send_listing(f_out);
		return -1;
	}

	if (*line == '#') {
		/* it's some sort of command that I don't understand */
		io_printf(f_out, "@ERROR: Unknown command '%s'\n", line);
		return -1;
	}

	if ((i = lp_number(line)) < 0) {
		char *addr = client_addr(f_in);
		char *host = client_name(f_in);
		rprintf(FLOG, "unknown module '%s' tried from %s (%s)\n",
			line, host, addr);
		io_printf(f_out, "@ERROR: Unknown module '%s'\n", line);
		return -1;
	}

	return rsync_module(f_in, f_out, i);
}


int daemon_main(void)
{
	char *pid_file;

	if (is_a_socket(STDIN_FILENO)) {
		int i;

		/* we are running via inetd - close off stdout and
		 * stderr so that library functions (and getopt) don't
		 * try to use them. Redirect them to /dev/null */
		for (i = 1; i < 3; i++) {
			close(i);
			open("/dev/null", O_RDWR);
		}

		return start_daemon(STDIN_FILENO, STDIN_FILENO);
	}

	if (!no_detach)
		become_daemon();

	if (!lp_load(config_file, 1))
		exit_cleanup(RERR_SYNTAX);

	if (rsync_port == 0 && (rsync_port = lp_rsync_port()) == 0)
		rsync_port = RSYNC_PORT;
	if (bind_address == NULL && *lp_bind_address())
		bind_address = lp_bind_address();

	log_init();

	rprintf(FLOG, "rsyncd version %s starting, listening on port %d\n",
		RSYNC_VERSION, rsync_port);
	/* TODO: If listening on a particular address, then show that
	 * address too.  In fact, why not just do inet_ntop on the
	 * local address??? */

	if (((pid_file = lp_pid_file()) != NULL) && (*pid_file != '\0')) {
		char pidbuf[16];
		int fd;
		pid_t pid = getpid();
		cleanup_set_pid(pid);
		if ((fd = do_open(lp_pid_file(), O_WRONLY|O_CREAT|O_TRUNC,
					0666 & ~orig_umask)) == -1) {
			cleanup_set_pid(0);
			rsyserr(FLOG, errno, "failed to create pid file %s",
				safe_fname(pid_file));
			exit_cleanup(RERR_FILEIO);
		}
		snprintf(pidbuf, sizeof pidbuf, "%ld\n", (long)pid);
		write(fd, pidbuf, strlen(pidbuf));
		close(fd);
	}

	start_accept_loop(rsync_port, start_daemon);
	return -1;
}

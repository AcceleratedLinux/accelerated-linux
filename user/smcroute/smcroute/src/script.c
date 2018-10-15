/* Run script when a (*,G) group is matched and installed in the kernel
 *
 * Copyright (c) 2011-2017  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <signal.h>		/* sigemptyset(), sigaction() */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>		/* AF_INET, AF_INET6 */

#include "log.h"
#include "script.h"

static char *exec   = NULL;
static pid_t script = 0;

static void handler(int signo)
{
	int status;
	pid_t pid = 1;

	(void)signo;

	while (pid > 0) {
		pid = waitpid(-1, &status, WNOHANG);
		if (pid == script) {
			script = 0;

			/* Script exit OK. */
			if (WIFEXITED(status))
				continue;

			/* Script exit status ... */
			status = WEXITSTATUS(status);
			if (status)
				smclog(LOG_WARNING, "Script %s returned error: %d", exec, status);
		}
	}
}

int script_init(char *script)
{
	struct sigaction sa;

	if (script && access(script, X_OK)) {
		smclog(LOG_ERR, "%s is not executable.", script);
		return -1;
	}
	exec = script;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	return 0;
}

int script_exec(struct mroute *mroute)
{
	pid_t pid;

	char *argv[] = {
		exec,
		"reload",
		NULL,
	};

	if (!exec)
		return 0;

	if (mroute) {
		char source[INET6_ADDRSTRLEN], group[INET6_ADDRSTRLEN];

		if (mroute->version == 4) {
			inet_ntop(AF_INET, &mroute->u.mroute4.source.s_addr, source, INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &mroute->u.mroute4.group.s_addr, group, INET_ADDRSTRLEN);
		} else {
			inet_ntop(AF_INET6, &mroute->u.mroute6.source.sin6_addr, source, INET_ADDRSTRLEN);
			inet_ntop(AF_INET6, &mroute->u.mroute6.group.sin6_addr, group, INET_ADDRSTRLEN);
		}

		setenv("source", source, 1);
		setenv("group", group, 1);
		argv[1] = "install";
	} else {
		unsetenv("source");
		unsetenv("group");
	}

	pid = fork();
	if (!pid)
		_exit(execv(argv[0], argv));
	if (pid < 0) {
		smclog(LOG_WARNING, "Cannot start script %s: %s", exec, strerror(errno));
		return -1;
	}

	script = pid;
	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */


/*
 * Copyright (c) 2017 Digi International Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include <unistd.h>

#include "subprocess.h"

/**
 * Convert a command string to an array of pointers to null-terminated strings
 *
 * @param command the command to tokenize
 * @return strings array on sucess, NULL on error
 */
static char **tokenize_command(char *command)
{
	const char *delim = " ";
	char **argv;
	char *token, *buf;
	int c = 1;

	argv = calloc(c, sizeof(char *));
	if (!argv)
		goto done;

	/* 'strtok' changes the original string, so use a copy we can free */
	buf = strdup(command);

	token = strtok(buf, delim);
	argv[c - 1] = token ? strdup(token) : NULL;
	while (argv[c - 1]) {
		void *tmp;
		c++;
		tmp = realloc(argv, c * sizeof(char *));
		if (!tmp)
			free(argv);
		argv = tmp;
		if (!argv)
			break;
		token = strtok(NULL, delim);
		argv[c - 1] = token ? strdup(token) : NULL;
	}
	free(buf);

done:
	return argv;
}

/**
 * Safely fork a child process and execute the given command with PATH lookup.
 *
 * @param command the command to execute
 * @param child_status return child status of executable
 * @return 0 on success, -1 on error with errno set
 */
int safe_execute(char *command, int *const child_status)
{
	int status = 0, i;
	int sub_status = 0;
	pid_t pid = 0;
	pid_t sub_pid = 0;
	struct sigaction const action_ignore = {.sa_handler = SIG_IGN };

	char **argv = tokenize_command(command);

	if ((NULL == command) || (NULL == argv) || (NULL == child_status)) {
		status = EINVAL;
		goto done;
	}

	/* Initialize child_status */
	*child_status = -1;

	/*
	 * Fork a child process to do the execvp() call because that call will not
	 * return if it works.  We're doing it this way to try to make this
	 * function as "nice" to the calling application as possible by not
	 * inhibiting interrupts as can happen if the system() call is used (plus
	 * that utility can have weird side-effects if the calling application has
	 * suid or sgid privileges set).
	 */
	pid = fork();
	if (pid < 0) {
		status = errno;
		syslog(LOG_ERR, "[%s:%d] could not fork process: %s (%d)\n",
		       __func__, __LINE__, strerror(errno), errno);
		goto done;
	}

	/*
	 * If we're now the child process, do the execvp(), replacing our process
	 * with the called process. SIGINT and SIGTERM are ignored in child process
	 * as we don't have any good ways of cleaning up the subprocess after a
	 * signal.
	 */
	if (0 == pid) {
		if (sigaction(SIGINT, &action_ignore, NULL) < 0) {
			status = errno;
			syslog
			    (LOG_ERR, "[%s:%d] error ignoring SIGINT: %s (%d)\n\n",
			       __func__, __LINE__, strerror(errno), errno);
		}

		if (sigaction(SIGTERM, &action_ignore, NULL) < 0) {
			status = errno;
			syslog
			    (LOG_ERR, "[%s:%d] error ignoring SIGTERM: %s (%d)\n\n",
			     __func__, __LINE__, strerror(errno), errno);
		}

		if (execvp(argv[0], (char *const *)argv) < 0) {
			status = errno;
			syslog
			    (LOG_ERR, "[%s:%d] error executing child process: %s (%d)\n",
			     __func__, __LINE__, strerror(errno), errno);
		}

		_exit(127);
	}

	/*
	 * Otherwise, if we're the parent, wait for the child process to terminate.
	 * If waitpid returns due to a signal (EINTR), ignore the error and
	 * continue to wait.
	 */
	do {
		errno = 0;
		sub_pid = waitpid(pid, &sub_status, 0);
		if ((sub_pid < 0) && (EINTR != errno)) {
			status = errno;
			syslog
			    (LOG_ERR, "[%s:%d] error returned by waitpid: %s (%d)\n",
			     __func__, __LINE__, strerror(errno), errno);
			goto done;
		}

		if (WIFEXITED(sub_status)) {
			*child_status = WEXITSTATUS(sub_status);
		}
	} while ((errno == EINTR) || (!WIFEXITED(sub_status)));

done:
	/* Free resources */
	i = 0;
	while (argv && argv[i]) {
		free(argv[i]);
		i++;
	}
	free(argv);

	errno = status;

	return (status == 0) ? 0 : -1;
}

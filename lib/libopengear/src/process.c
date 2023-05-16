#define _BSD_SOURCE		/* for sigset_t and DT_DIR */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>

#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>

#include <opengear/process.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif


int
opengear_proc_exec(
	const char *path, const char * const argv[], int *exitcode, uint32_t flags)
{
	pid_t pid = -1;
	int status = 0;

	if (exitcode != NULL) {
		*exitcode = 0;
	}

	pid = opengear_proc_exec_nowait(path, argv, flags);
	if (pid == -1) {
		fprintf(stderr,
			"opengear_proc_exec_nowait(%s): failed: %s\n",
			path, strerror(errno));
		return (-1);
	}

	if (waitpid(pid, &status, 0) < 0) {
		fprintf(stderr, "waitpid(%d) failed: %s\n",
				pid, strerror(errno));
		return (-1);
	}

	if (WEXITSTATUS(status) != 0 && exitcode != NULL) {
		errno = 0;
		*exitcode = WEXITSTATUS(status);
		return (-1);
	}
	return (0);
}

int
opengear_proc_redir_std_null()
{
	int fd, fd_max;

	fd = open("/dev/null", O_RDWR, 0);
	if (fd == -1) {
		return (-1);
	}

	(void) dup2(fd, STDIN_FILENO);
	(void) dup2(fd, STDOUT_FILENO);
	(void) dup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO) {
		(void) close(fd);
	}

	/* Close all open file descriptors, see cvstrac ticket 1484 */
	fd_max = sysconf(_SC_OPEN_MAX);
	fd = STDERR_FILENO + 1;
	while (fd < fd_max) {
		(void) close(fd++);
	}

	return (0);
}

int
opengear_proc_exec_nowait(
	const char *path, const char * const argv[], uint32_t flags)
{
	pid_t pid;

	switch ((pid = fork())) {
	case 0:
		/* Daemonise */
		if ((flags & OG_PROC_DAEMON)) {
			(void) opengear_proc_redir_std_null();
		}

		/* New process group */
		setsid();

		/* Run */
		execv(path, (char **)argv);

		/* The cast from (const char **) to (char **) is safe here and
		 * below because execv() and friends do not alter their
		 * argument. The signature of exec() functions is the way it
		 * is because of historic code (predating const) and a
		 * limitation in ISO C's type system that means pointers to
		 * differently-const types are not assignment-compatible.
		 * Please refer to the discussion and table of sadness at:
		 * http://pubs.opengroup.org/onlinepubs/009695399/functions/exec.html */

		/* Terminate */
		_exit(1);
	case -1:
		return (-1);
	default:
		break;
	}

	return pid;
}

int
opengear_proc_exec_env_nowait(
	const char *path, const char * const argv[], const char * const envp[], uint32_t flags)
{
	pid_t pid;

	switch ((pid = fork())) {
	case 0:
		/* Daemonise */
		if ((flags & OG_PROC_DAEMON)) {
			(void) opengear_proc_redir_std_null();
		}

		/* New process group */
		setsid();
		if (envp != NULL) {
			/* Run */
			execve(path, (char **)argv, (char **)envp);
		} else {
			execv(path, (char **)argv);
		}
		syslog(LOG_ERR, "EXECVE FAILED");
		/* Terminate */
		_exit(1);
	case -1:
		return (-1);
	default:
		break;
	}

	return pid;
}

int
opengear_system(const char *cmd, int *exitcode, int success)
{
	int rv = opengear_system_execonly(cmd, exitcode);
	switch (rv) {
		case -1:
			syslog(LOG_ERR, "Failed to execute: %s: %d", cmd, rv);
			break;

		case 0:
			if (*exitcode != success) {
				syslog(LOG_WARNING, "Command not successful: %s: %s", cmd, strerror(*exitcode));
			}
			break;

		default:
			syslog(LOG_WARNING, "Unexpected result from opengear_system call: %d (%s)", rv, cmd);
			break;
	}

	return rv;
}

int
opengear_system_execonly(const char *cmd, int *exitcode)
{
	int status = -1;
	int rv = -1;

	rv = system(cmd);
	if (rv == -1) {
		return -1;
	}

	status = WEXITSTATUS(rv);
	*exitcode = status;
	return 0;
}

int opengear_proc_execp_nowait_fds(const char *path, const char * const argv[], int *infd, int *outfd, int *errfd) {

	pid_t pid;
	int p_stdin[2], p_stdout[2], p_stderr[2];

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0 || pipe(p_stderr) != 0) {
		return -1;
	}

	switch ((pid = fork())) {
	case 0:
		/* Connect the pipes */
		close(p_stdin[1]);
		dup2(p_stdin[0], 0);
		close(p_stdout[0]);
		dup2(p_stdout[1], 1);
		close(p_stderr[0]);
		dup2(p_stderr[1], 2);

		/* unblock signals */
		sigset_t sigs;
		sigfillset(&sigs);
		sigprocmask(SIG_UNBLOCK, &sigs, 0);

		/* New process group */
		setsid();

		/* Run */
		execvp(path, (char **)argv);
		syslog(LOG_ERR, "Exec failed");
		/* Terminate */
		_exit(1);
	case -1:
		return (-1);
	default:
		break;
	}

	/* on parent, always close the read on the child's stdin pipe */
	close(p_stdin[0]);
	if (infd == NULL) {
		close(p_stdin[1]);
	} else {
		*infd = p_stdin[1];
	}

	close(p_stdout[1]);
	if (outfd == NULL) {
		close(p_stdout[0]);
	} else {
		*outfd = p_stdout[0];
	}

	close(p_stderr[1]);
	if (errfd == NULL) {
		close(p_stderr[0]);
	} else {
		*errfd = p_stderr[0];
	}

	return pid;
}

int opengear_proc_exec_nowait_fds(const char *path, const char * const argv[], int *infd, int *outfd) {

	pid_t pid;
	int p_stdin[2], p_stdout[2];

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0) {
		return -1;
	}

	switch ((pid = fork())) {
	case 0:
		/* Connect the pipes */
		close(p_stdin[1]);
		dup2(p_stdin[0], 0);
		close(p_stdout[0]);
		dup2(p_stdout[1], 1);

		/* New process group */
		setsid();

		/* Run */
		execv(path, (char **)argv);
		syslog(LOG_ERR, "Exec failed");
		/* Terminate */
		_exit(1);
	case -1:
		return (-1);
	default:
		break;
	}

	close(p_stdin[0]);
	if (infd == NULL) {
		close(p_stdin[1]);
	} else {
		*infd = p_stdin[1];
	}

	close(p_stdout[1]);
	if (outfd == NULL) {
		close(p_stdout[0]);
	} else {
		*outfd = p_stdout[0];
	}

	return pid;
}


/*
 * An iterator over all the /proc/<pid> directories.
 * Whenever proc_iterator_next() returns true, the .path and .pid
 * fields will be set to the next process directory.
 * (Although, due to race conditions, it may be gone by the time
 * you try to use it!)
 *
 * Example usage:
 *	struct proc_iterator it;
 *	if (proc_iterator_begin(&it) == -1) goto error;
 *	while (proc_iterator_next(&it)) {
 *	     use(it.pid);
 *      }
 *      proc_iterator_end(&it);
 */
struct proc_iterator {
	DIR *procdir;
	pid_t pid;
	char path[PATH_MAX]; /* "/proc/<pid>/" intended for strcat */
};

static int
proc_iterator_begin(struct proc_iterator *it)
{
	it->procdir = opendir("/proc/");
	if (!it->procdir) {
		syslog(LOG_EMERG, "Could not open /proc/ dir: %m");
		return -1;
	}
	return 0;
}

static void
proc_iterator_end(struct proc_iterator *it)
{
	int errno_save = errno;
	(void) closedir(it->procdir);
	errno = errno_save;
}

static int
proc_iterator_next(struct proc_iterator *it)
{
	struct dirent *de;

	while ((de = readdir(it->procdir))) {
		if (de->d_type != DT_DIR)
			continue;
		if (!isdigit(de->d_name[0]))
			continue;
		snprintf(it->path, sizeof it->path, "/proc/%s/", de->d_name);
		it->path[sizeof it->path - 1] = '\0'; /* for strncat */
		it->pid = atoi(de->d_name);
		return 1;
	}
	return 0;
}


int
opengear_proc_get_pids(
	const char * const argv[], int argc, pid_t *pidlist, size_t maxpids,
	size_t *npids_return)
{
	struct proc_iterator it;
	size_t command_size;
	int i;
	size_t *argv_size;
	size_t npids;

	/* Each /proc/<pid>/command file contains a binary concatenation
	 * of nul-terminated C strings, one for each argv. */

	command_size = 0;
	argv_size = alloca(argc * sizeof *argv_size);
	for (i = 0; i < argc; i++) {
		argv_size[i] = strlen(argv[i]) + 1;
		command_size += argv_size[i];
	}

	if (proc_iterator_begin(&it) == -1) {
		return -1;
	}
	npids = 0;
	while (npids < maxpids && proc_iterator_next(&it)) {
		char buffer[command_size + 1], *p;
		ssize_t rlen;
		int fd;

		strncat(it.path, "command", sizeof it.path - 1);
		fd = open(it.path, O_RDONLY);
		if (fd == -1) {
			/* The proc probably disappeared */
			continue;
		}
		/* Over-read by one will reveal if the file
		 * is bigger than we want. */
		rlen = read(fd, buffer, command_size + 1);
		if (rlen == command_size) {
			/* Walk p over the buffer, comparing consecutively
			 * against each argv[] */
			p = buffer;
			for (i = 0; i < argc; i++) {
			    if (memcmp(argv[i], p, argv_size[i]) != 0)
				break;
			    p += argv_size[i];
			}
			if (i == argc) {
			    pidlist[npids++] = it.pid;
			}
		}
		close(fd);
	}
	proc_iterator_end(&it);

	*npids_return = npids;

	return 0;
}

int
opengear_proc_find_comms(const char *comm, pid_t *pids, int maxpids)
{
	struct proc_iterator it;
	size_t comm_len = strlen(comm);
	int npids;
	char line[1024], *lparen;
	int fd, linelen;

	if (proc_iterator_begin(&it) == -1) {
		return -1;
	}
	npids = 0;
	while (npids < maxpids && proc_iterator_next(&it)) {
		strncat(it.path, "stat", sizeof it.path - 1);
		fd = open(it.path, O_RDONLY);
		if (fd < 0) {
			continue;
		}
		linelen = read(fd, line, sizeof line - 1);
		close(fd);
		if (linelen < 0) {
			continue;
		}
		line[linelen] = '\0';
		lparen = strchr(line, '(');
		if (lparen &&
		    lparen + 1 + comm_len < line + linelen &&
		    lparen[1 + comm_len] == ')' &&
		    memcmp(comm, lparen + 1, comm_len) == 0)
		{
			pids[npids++] = it.pid;
		}
	}
	proc_iterator_end(&it);
	return npids;
}


/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */

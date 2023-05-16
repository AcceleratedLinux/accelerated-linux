#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "vfile.h"

/* Adapts a subprocess's stdout via a unix pipe. */
struct vfile_proc {
	struct vfile vfile;
	pid_t pid;
	int fd;
};

static struct vfile_proc *
to_proc(struct vfile *f)
{
	/* assert(f->ops = &proc_ops); */
        return (struct vfile_proc *)f;
}

static void
reap(struct vfile_proc *proc, int flags)
{
	if (proc->pid != -1) {
		int wstatus = 0;
		if (waitpid(proc->pid, &wstatus, flags) == proc->pid) {
#if 0
			if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus))
				fprintf(stderr, "exit %d\n",
					WEXITSTATUS(wstatus));
			if (WIFSIGNALED(wstatus))
				fprintf(stderr, "signal %d\n",
					WTERMSIG(wstatus));
#endif
			proc->pid = -1;
		}
	}
}

static void
proc_close(struct vfile *f)
{
	struct vfile_proc *proc = to_proc(f);

	if (proc->fd != -1)
		close(proc->fd);
	if (proc->pid != -1) {
		kill(SIGKILL, proc->pid);
		reap(proc, 0);
	}
	free(proc);
}

static int
proc_read(struct vfile *f, void *dst, int sz)
{
	struct vfile_proc *proc = to_proc(f);
	char *d = dst;
	int ret = 0;

	while (sz > 0 && proc->fd != -1) {
		int n = read(proc->fd, d, sz);
		if (n == 0 || n == -1) {
			if (n == -1)
				ret = -1;
			close(proc->fd);
			proc->fd = -1;
			reap(proc, WNOHANG);
			break;
		}
		sz -= n;
		ret += n;
		d += n;
	}
	return ret;
}

static int
proc_getsize(struct vfile *f)
{
	errno = ENOTSUP;
	return -1;
}

static int
proc_seek(struct vfile *f, int offset)
{
	errno = ENOTSUP;
	return -1;
}

static int
proc_tell(struct vfile *f)
{
	errno = ENOTSUP;
	return -1;
}

static const struct vfileops proc_ops = {
	.close = proc_close,
	.read = proc_read,
	.tell = proc_tell,
	.getsize = proc_getsize,
	.seek = proc_seek,
};

struct vfile *
vfile_proc(const char *file, char *const argv[])
{
	struct vfile_proc *proc = calloc(1, sizeof *proc);
	int p[2];

	if (!proc)
		return NULL;
	VFILE_INIT(&proc->vfile, &proc_ops);

	if (pipe(p) == -1) {
		free(proc);
		return NULL;
	}

	proc->pid = fork();
	if (proc->pid == -1) {
		close(p[0]);
		close(p[1]);
		free(proc);
		return NULL;
	}
	if (proc->pid == 0) {
		/* In child */
		close(p[0]);
		if (p[1] != 1) {
			dup2(p[1], 1);
			close(p[1]);
		}
		execvp(file, argv);
		perror(file);
		_exit(1);
	}
	close(p[1]);
	proc->fd = p[0];
	return &proc->vfile;
}

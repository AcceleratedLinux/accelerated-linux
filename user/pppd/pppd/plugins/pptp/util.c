/* util.c ....... error message utilities.
 *                C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: util.c,v 1.11 2005/08/22 00:49:48 quozl Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

#define MAKE_STRING(label) 				\
va_list ap;						\
char buf[256], string[256];				\
va_start(ap, format);					\
vsnprintf(buf, sizeof(buf), format, ap);		\
snprintf(string, sizeof(string), "%s %s[%s:%s:%d]: %s",	\
	 log_string, label, func, file, line, buf);	\
va_end(ap)

/*** connect a file to a file descriptor **************************************/
int file2fd(const char *path, const char *mode, int fd)
{
    int ok = 0;
    FILE *file = NULL;
    file = fopen(path, mode);
    if (file != NULL && dup2(fileno(file), fd) != -1)
        ok = 1;
    if (file) fclose(file);
    return ok;
}

/* signal to pipe delivery implementation */
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

/* pipe private to process */
static int sigpipe[2];

/* create a signal pipe, returns 0 for success, -1 with errno for failure */
int sigpipe_create()
{
  int rc;
  
  rc = pipe(sigpipe);
  if (rc < 0) return rc;
  
  fcntl(sigpipe[0], F_SETFD, FD_CLOEXEC);
  fcntl(sigpipe[1], F_SETFD, FD_CLOEXEC);
  
#ifdef O_NONBLOCK
#define FLAG_TO_SET O_NONBLOCK
#else
#ifdef SYSV
#define FLAG_TO_SET O_NDELAY
#else /* BSD */
#define FLAG_TO_SET FNDELAY
#endif
#endif
  
  rc = fcntl(sigpipe[1], F_GETFL);
  if (rc != -1)
    rc = fcntl(sigpipe[1], F_SETFL, rc | FLAG_TO_SET);
  if (rc < 0) return rc;
  return 0;
#undef FLAG_TO_SET
}

/* generic handler for signals, writes signal number to pipe */
void sigpipe_handler(int signum)
{
  write(sigpipe[1], &signum, sizeof(signum));
  signal(signum, sigpipe_handler);
}

/* assign a signal number to the pipe */
void sigpipe_assign(int signum)
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigpipe_handler;
  sigaction(signum, &sa, NULL);
}

/* return the signal pipe read file descriptor for select(2) */
int sigpipe_fd()
{
  return sigpipe[0];
}

/* read and return the pending signal from the pipe */
int sigpipe_read()
{
  int signum;
  read(sigpipe[0], &signum, sizeof(signum));
  return signum;
}

void sigpipe_close()
{
  close(sigpipe[0]);
  close(sigpipe[1]);
}


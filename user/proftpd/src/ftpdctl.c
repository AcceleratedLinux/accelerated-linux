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

/* ProFTPD Controls command-line client
 *
 * $Id: ftpdctl.c,v 1.2 2004/11/02 18:18:59 castaglia Exp $
 */

#include "conf.h"

#include <signal.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

static const char *program = "ftpdctl";

/* NOTE: these empty stubs are needed for proper linking.  What a mess. */

session_t session;
server_rec *main_server = NULL;

void *get_param_ptr(xaset_t *set, const char *name, int recurse) {
  return NULL;
}

char *pr_inet_validate(char *buf) {
  return NULL;
}

void pr_alarms_block(void) {
}

void pr_alarms_unblock(void) {
}

void pr_signals_block(void) {
}

void pr_signals_unblock(void) {
}

char *sstrcat(char *dest, const char *src, size_t n) {
  register char *d;

  for (d = dest; *d && n > 1; d++, n--) ;

  while(n-- > 1 && *src)
    *d++ = *src++;

  *d = 0;
  return dest;
}

char *sstrncpy(char *dest, const char *src, size_t n) {
  register char *d = dest;

  if (!dest)
    return NULL;

  if (src && *src) {
    for (; *src && n > 1; n--)
      *d++ = *src++;
  }

  *d = '\0';
  return dest;
}

#ifdef PR_USE_CTRLS

/* need a SIGPIPE handler */
static RETSIGTYPE sig_pipe(int sig) {
  signal(SIGPIPE, sig_pipe);
}

static void usage(void) {
  fprintf(stdout, "usage: %s [options]\n", program);
  fprintf(stdout, "  -h\tdisplays this message\n");
  fprintf(stdout, "  -s\tspecify an alternate local socket\n");
  fprintf(stdout, "  -v\tdisplays more verbose information\n");
  fprintf(stdout, "\n");
  return;
}

int main(int argc, char *argv[]) {
  unsigned char verbose = FALSE;
  char **respargv = NULL;
  const char *cmdopts = "hs:v";

  register int i = 0;
  char *socket_file = PR_RUN_DIR "/proftpd.sock";
  int sockfd = -1, optc = 0, status = 0, respargc = 0;
  unsigned int reqargc = 0;
  pool *ctl_pool = NULL;
  array_header *reqargv = NULL;

  /* Make sure we were called with at least one argument. */
  if (argc-1 < 1) {
    fprintf(stdout, "%s: missing required arguments\n", program);
    exit(1);
  }

  /* Set the POSIXLY_CORRECT environment variable, so that control handlers
   * can themselves have optional flags.
   */
  if (putenv("POSIXLY_CORRECT=1") < 0) {
    fprintf(stderr, "%s: unable to set POSIXLY_CORRECT: %s\n", program,
      strerror(errno));
    exit(1);
  }

  opterr = 0;
  while ((optc = getopt(argc, argv, cmdopts)) != -1) {
    switch (optc) {
      case 'h':
        usage();
        return 0;

      case 's':
        if (*optarg != '/') {
          fprintf(stderr, "%s: alternate socket path must be an absolute "
            "path\n", program);
          return 1;
        }

        socket_file = optarg;
        break;

      case 'v':
        verbose = TRUE;
        break;

      case '?':
        fprintf(stdout, "%s: unknown option: %c\n", program, (char) optopt);
        break;
    }
  }

  signal(SIGPIPE, sig_pipe);

  /* Allocate some memory for proftpd objects. */
  ctl_pool = make_sub_pool(NULL);

  reqargv = make_array(ctl_pool, 0, sizeof(char *));

  /* Process the command-line args into an array_header. */
  for (i = optind; i < argc; i++) {
    if (verbose)
      fprintf(stdout, "%s: adding \"%s\" to reqargv\n", program, argv[i]);
    *((char **) push_array(reqargv)) = pstrdup(ctl_pool, argv[i]);
    reqargc++;
  }

  /* Don't forget to NULL-terminate the array. */
  *((char **) push_array(reqargv)) = NULL;

  /* Open a connection to the socket maintained by mod_ftpdctl. */
  if (verbose)
    fprintf(stdout, "%s: contacting server\n", program);

  if ((sockfd = pr_ctrls_connect(socket_file)) < 0) {
    fprintf(stderr, "%s: error contacting server: %s\n", program,
      strerror(errno));
    exit(1);
  }

  if (verbose)
    fprintf(stdout, "%s: sending control request\n", program);

  if (pr_ctrls_send_msg(sockfd, 0, reqargc, (char **) reqargv->elts) < 0) {
    fprintf(stderr, "%s: error sending request: %s\n", program,
      strerror(errno));
    exit(1);
  }

  /* Read and display the responses. */

  if (verbose)
    fprintf(stdout, "%s: receiving control response\n", program);

  /* Manually set errno to this value, so that if an error (like a segfault)
   * occurs, the error string displayed to the user is more indicative of
   * the cause of the lack of responses.
   */
  errno = EPERM;

  if ((respargc = pr_ctrls_recv_response(ctl_pool, sockfd, &status,
      &respargv)) < 0) {
    fprintf(stdout, "%s: error receiving response: %s\n", program,
      strerror(errno));
    exit(1);
  }

  if (respargv != NULL) {
    for (i = 0; i < respargc; i++)
      fprintf(stdout, "%s: %s\n", program, respargv[i]);

  } else
    fprintf(stdout, "%s: no response from server\n", program);

  destroy_pool(ctl_pool);
  ctl_pool = NULL;

  return 0;
}

#else

int main(int argc, char *argv[]) {
  printf("%s:\n", program);
  printf("  Controls support disabled.\n");
  printf("  Please recompile proftpd using --enable-ctrls\n");
  return 1;
}

#endif /* PR_USE_CTRLS */

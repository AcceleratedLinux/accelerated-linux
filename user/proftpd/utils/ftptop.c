/*
 * ProFTPD - ftptop: a utility for monitoring proftpd sessions
 * Copyright (c) 2000-2002 TJ Saunders <tj@castaglia.org>
 * Copyright (c) 2003 The ProFTPD Project team
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

/* Shows who is online via proftpd, in a manner similar to top.  Uses the
 * scoreboard files.
 *
 * $Id: ftptop.c,v 1.31 2004/11/02 18:18:59 castaglia Exp $
 */

#define FTPTOP_VERSION "ftptop/0.9"

#include "utils.h"

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

static const char *program = "ftptop";

/* ncurses is preferred...*/

#if defined(HAVE_NCURSES_H) && defined(HAVE_LIBNCURSES) && \
    defined(PR_USE_NCURSES)
# define HAVE_NCURSES 1
# include <ncurses.h>
#elif defined(HAVE_CURSES_H) && defined(HAVE_LIBCURSES) && \
    defined(PR_USE_CURSES)
# define HAVE_CURSES 1
/* Sigh...portability.  It seems that Solaris' curses.h (at least for 2.8)
 * steps on wide-character macros, generating compiler warnings.  This, then
 * is just a hack to silence the compiler.
 */
# ifdef SOLARIS2
#  define __lint
# endif
# include <curses.h>
#endif

#if defined(HAVE_NCURSES) || defined(HAVE_CURSES)

/* Display options */

/* These are for displaying "PID S USER CLIENT SERVER TIME COMMAND" */
#define FTPTOP_REG_HEADER_FMT	"%-5s %s %-8s %-20s %-15s %-4s %-*s\n"
#define FTPTOP_REG_DISPLAY_FMT	"%-5u %s %-8.8s %-20.20s %-15s %-6.6s %4s %-*.*s\n"

/* These are for displaying tranfer data: "PID S USER CLIENT KB/s %DONE" */
#define FTPTOP_XFER_HEADER_FMT	"%-5s %s %-8s %-44s %-10s %-*s\n"
#define FTPTOP_XFER_DISPLAY_FMT	"%-5u %s %-8.8s %-44.44s %-10.2f %-*s\n"

#define FTPTOP_REG_ARG_MIN_SIZE		20
#define FTPTOP_XFER_DONE_MIN_SIZE	6
#define FTPTOP_REG_ARG_SIZE	\
  (COLS - (80 - FTPTOP_REG_ARG_MIN_SIZE) < FTPTOP_REG_ARG_MIN_SIZE ? \
  FTPTOP_REG_ARG_MIN_SIZE : COLS - (80 - FTPTOP_REG_ARG_MIN_SIZE))
#define FTPTOP_XFER_DONE_SIZE 	\
  (COLS - (80 - FTPTOP_XFER_DONE_MIN_SIZE) < FTPTOP_XFER_DONE_MIN_SIZE ? \
  FTPTOP_XFER_DONE_MIN_SIZE : COLS - (80 - FTPTOP_XFER_DONE_MIN_SIZE))

#define FTPTOP_SHOW_DOWNLOAD		0x0001
#define FTPTOP_SHOW_UPLOAD		0x0002
#define FTPTOP_SHOW_IDLE		0x0004
#define	FTPTOP_SHOW_REG \
  (FTPTOP_SHOW_DOWNLOAD|FTPTOP_SHOW_UPLOAD|FTPTOP_SHOW_IDLE)
#define FTPTOP_SHOW_RATES		0x0010

static int delay = 2;
static unsigned int display_mode = FTPTOP_SHOW_REG;

static char *config_filename = PR_CONFIG_FILE_PATH;

/* Scoreboard variables */
static time_t ftp_uptime = 0;
static unsigned int ftp_nsessions = 0;
static unsigned int ftp_nuploads = 0;
static unsigned int ftp_ndownloads = 0;
static unsigned int ftp_nidles = 0;
static char *server_name = NULL;
static char **ftp_sessions = NULL;
static unsigned int chunklen = 3;

/* necessary prototypes */
static void scoreboard_close(void);
static int scoreboard_open(void);

static void show_version(void);
static const char *show_ftpd_uptime(void);
static void usage(void);

static void clear_counters(void) {

  if (ftp_sessions && ftp_nsessions) {
    register unsigned int i = 0;

    for (i = 0; i < ftp_nsessions; i++)
      free(ftp_sessions[i]);
    free(ftp_sessions);
    ftp_sessions = NULL;
  }

  /* Reset the session counters. */
  ftp_nsessions = 0;
  ftp_nuploads = 0;
  ftp_ndownloads = 0;
  ftp_nidles = 0;
}

static void finish(int signo) {
  endwin();
  exit(0);
}

static char *calc_percent_done(off_t size, off_t done) {
  static char sbuf[32];

  memset(sbuf, '\0', sizeof(sbuf));

  if (done == 0) {
    util_sstrncpy(sbuf, "0", sizeof(sbuf));

  } else if (size == 0) {
    util_sstrncpy(sbuf, "Inf", sizeof(sbuf));

  } else if (done >= size) {
    util_sstrncpy(sbuf, "100", sizeof(sbuf));

  } else {
    snprintf(sbuf, sizeof(sbuf), "%.0f",
      ((double) done / (double) size) * 100.0);
    sbuf[sizeof(sbuf)-1] = '\0';
  }

  return sbuf;
}

/* Borrowed from ftpwho.c */
static const char *show_time(time_t *i) {
  time_t now = time(NULL);
  unsigned long l;
  static char sbuf[7];

  if (!i || !*i)
    return "-";

  memset(sbuf, '\0', sizeof(sbuf));
  l = now - *i;

  if (l < 3600)
    snprintf(sbuf, sizeof(sbuf), "%lum%lus",(l / 60),(l % 60));
  else
    snprintf(sbuf, sizeof(sbuf), "%luh%lum",(l / 3600),
    ((l - (l / 3600) * 3600) / 60));

  return sbuf;
}

static int check_scoreboard_file(void) {
  struct stat sbuf;

  if (stat(util_get_scoreboard(), &sbuf) < 0)
    return -1;

  return 0;
}

static const char *show_ftpd_uptime(void) {
  static char buf[128] = {'\0'};
  time_t uptime_secs = time(NULL) - ftp_uptime;
  int upminutes, uphours, updays;
  int pos = 0;

  if (!ftp_uptime)
    return "";

  memset(buf, '\0', sizeof(buf));

  strcat(buf, ", up for ");
  pos += strlen(buf); 

  updays = (int) uptime_secs / (60 * 60 * 24);

  if (updays)
    pos += sprintf(buf + pos, "%d day%s, ", updays, (updays != 1) ? "s" : "");

  upminutes = (int) uptime_secs / 60;

  uphours = upminutes / 60;
  uphours = uphours % 24;

  upminutes = upminutes % 60;

  if (uphours)
    pos += sprintf(buf + pos, "%2d hr%s %02d min", uphours,
      (uphours != 1) ? "s" : "", upminutes);
  else
    pos += sprintf(buf + pos, "%d min", upminutes);

  return buf;
}

/* scan_config_file() is a kludge for 1.2 which does a very simplistic attempt
 * at determining what the "ScoreboardFile" directive is set to.  It will be
 * replaced in 1.3 with the abstracted configure system (hopefully).
 */
static void scan_config_file(void) {
  FILE *fp = NULL;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  char *cp, *file = NULL;

  if (!config_filename || (fp = fopen(config_filename,"r")) == NULL)
    return;

  while (!file && fgets(buf, sizeof(buf) - 1, fp)) {
    int i = strlen(buf);

    if (i && buf[i - 1] == '\n')
      buf[i-1] = '\0';

    for (cp = buf; *cp && isspace((int) *cp); cp++);

    if (*cp == '#' || !*cp)
      continue;

    i = strlen("ScoreboardFile");

    if (strncasecmp(cp, "ScoreboardFile", i) != 0)
      continue;

    /* Found it! */
    cp += i;

    /* strip whitespace */
    while (*cp && isspace((int) *cp))
      cp++;

    file = cp;

    /* If the scoreboard file argument is quoted, dequote */
    if (*cp == '"') {
      char *src = cp;

      cp++;
      file++;

      while (*++src) {
        switch (*src) {
          case '\\':
            if (*++src)
              *cp++ = *src;
            break;

          case '"':
            src++;
            break;

          default:
            *cp++ = *src;
        }
      }

      *cp = '\0';
    }
  }

  fclose(fp);

  /* If we got something out of all this, go ahead and set it. */
  if (file)
    util_set_scoreboard(file);
}

static void process_opts(int argc, char *argv[]) {
  int optc = 0;
  const char *prgopts = "DS:d:f:hIiUV";

  while ((optc = getopt(argc, argv, prgopts)) != -1) {
    switch (optc) {
      case 'D':
        display_mode = 0U;
        display_mode |= FTPTOP_SHOW_DOWNLOAD;
        break;

      case 'S':
        server_name = strdup(optarg);
        break;

      case 'd':
        delay = atoi(optarg);

        if (delay < 0) {
          fprintf(stderr, "%s: negative delay illegal: %d\n", program,
            delay);
          exit(1);
        }

        break;

      case 'f':
        util_set_scoreboard(optarg);
        break;

      case 'h':
        usage();
        break;

      case 'I':
        display_mode = 0U;
        display_mode |= FTPTOP_SHOW_IDLE;
        break;

      case 'i':
        display_mode &= ~FTPTOP_SHOW_IDLE;
        break;

      case 'U':
        display_mode = 0U;
        display_mode |= FTPTOP_SHOW_UPLOAD;
        break;

      case 'V':
        show_version();
        break;

      case '?':
        break;

     default:
        break;
    }
  }

  /* First attempt to check the supplied/default scoreboard path.  If this is
   * incorrect, try the config file kludge.
   */
  if (check_scoreboard_file() < 0) {
    scan_config_file();

    if (check_scoreboard_file() < 0) {
      fprintf(stderr, "%s: %s\n", util_get_scoreboard(), strerror(errno));
      fprintf(stderr, "(Perhaps you need to specify the ScoreboardFile with -f, or change\n");
      fprintf(stderr," the compile-time default directory?)\n");
      exit(1);
    }
  }
}

static void read_scoreboard(void) {

  /* NOTE: this buffer should probably be limited to the maximum window
   * width, as it is used for display purposes.
   */
  static char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  pr_scoreboard_entry_t *score = NULL;

  if ((ftp_sessions = calloc(chunklen, sizeof(char *))) == NULL)
    exit(1);

  if (scoreboard_open() < 0)
    return;

  /* Iterate through the scoreboard. */
  while ((score = util_scoreboard_read_entry()) != NULL) {

    /* Default status: "A" for "authenticating" */
    char *status = "A";

    /* If a ServerName was given, skip unless the scoreboard entry matches. */
    if (server_name && strcmp(server_name, score->sce_server_label) != 0)
      continue;

    /* Clear the buffer for this run. */
    memset(buf, '\0', sizeof(buf));

    /* Determine the status symbol to display. */
    if (strcmp(score->sce_cmd, "idle") == 0) {
      status = "I";
      ftp_nidles++;

      if (display_mode != FTPTOP_SHOW_RATES &&
          !(display_mode & FTPTOP_SHOW_IDLE))
        continue;

    } else if (strcmp(score->sce_cmd, "RETR") == 0) {
      status = "D";
      ftp_ndownloads++;

      if (display_mode != FTPTOP_SHOW_RATES &&
          !(display_mode & FTPTOP_SHOW_DOWNLOAD))
        continue;

    } else if (strcmp(score->sce_cmd, "STOR") == 0 ||
        strcmp(score->sce_cmd, "APPE") == 0 ||
        strcmp(score->sce_cmd, "STOU") == 0) {
      status = "U";
      ftp_nuploads++;

      if (display_mode != FTPTOP_SHOW_RATES &&
          !(display_mode & FTPTOP_SHOW_UPLOAD))
        continue;

    } else if (strcmp(score->sce_cmd, "LIST") == 0 ||
        strcmp(score->sce_cmd, "NLST") == 0)
      status = "L";

    if (display_mode != FTPTOP_SHOW_RATES) {
      snprintf(buf, sizeof(buf), FTPTOP_REG_DISPLAY_FMT,
        (unsigned int) score->sce_pid, status, score->sce_user,
        score->sce_client_name, score->sce_server_addr,
        show_time(&score->sce_begin_session), score->sce_cmd,
       FTPTOP_REG_ARG_SIZE, FTPTOP_REG_ARG_SIZE, score->sce_cmd_arg);
      buf[sizeof(buf)-1] = '\0';

    } else {

      /* Skip sessions unless they are actually transferring data */
      if (*status != 'U' && *status != 'D')
        continue;

      snprintf(buf, sizeof(buf), FTPTOP_XFER_DISPLAY_FMT,
        (unsigned int) score->sce_pid, status, score->sce_user,
        score->sce_client_name,
        (score->sce_xfer_len / 1024.0) / (score->sce_xfer_elapsed / 1000),
        FTPTOP_XFER_DONE_SIZE,
        *status == 'D' ?
          calc_percent_done(score->sce_xfer_size, score->sce_xfer_done) :
          "(n/a)");
      buf[sizeof(buf)-1] = '\0';
    }

    /* Make sure there is enough memory allocated in the session list.
     * Allocate more if needed.
     */
    if (ftp_nsessions && ftp_nsessions % chunklen == 0) {
      if ((ftp_sessions = realloc(ftp_sessions,
          (ftp_nsessions + chunklen) * sizeof(char *))) == NULL)
        exit(1);
    }

    if ((ftp_sessions[ftp_nsessions] = calloc(1, strlen(buf) + 1)) == NULL)
      exit(1);
    strncpy(ftp_sessions[ftp_nsessions++], buf, strlen(buf) + 1);
  }

  scoreboard_close();
}

static void scoreboard_close(void) {
  util_close_scoreboard();
}

static int scoreboard_open(void) {
  int res = 0;

  if ((res = util_open_scoreboard(O_RDONLY)) < 0) {
    switch (res) {
      case UTIL_SCORE_ERR_BAD_MAGIC:
        fprintf(stderr, "%s: error opening scoreboard: bad/corrupted file\n",
          program);
        return res;

      case UTIL_SCORE_ERR_OLDER_VERSION:
        fprintf(stderr, "%s: error opening scoreboard: bad version (too old)\n",
          program);
        return res;

      case UTIL_SCORE_ERR_NEWER_VERSION:
        fprintf(stderr, "%s: error opening scoreboard: bad version (too new)\n",
          program);
        return res;

      default:
        fprintf(stderr, "%s: error opening scoreboard: %s\n",
          program, strerror(errno));
        return res;
    }
  }

  ftp_uptime = util_scoreboard_get_daemon_uptime();

  return 0;
}

static void show_sessions(void) {
  time_t now;
  char *now_str = NULL;
  const char *uptime_str = NULL;

  clear_counters();
  read_scoreboard();

  time(&now);

  /* Trim ctime(3)'s trailing newline. */
  now_str = ctime(&now);
  now_str[strlen(now_str)-1] = '\0';

  uptime_str = show_ftpd_uptime();

  wclear(stdscr);
  move(0, 0);

  attron(A_BOLD);
  printw(FTPTOP_VERSION ": %s%s\n", now_str, uptime_str);
  printw("%u Total FTP Sessions: %u downloading, %u uploading, %u idle\n",
    ftp_nsessions, ftp_ndownloads, ftp_nuploads, ftp_nidles);
  attroff(A_BOLD);

  printw("\n");

  attron(A_REVERSE);

  if (display_mode != FTPTOP_SHOW_RATES)
    printw(FTPTOP_REG_HEADER_FMT, "PID", "S", "USER", "CLIENT", "SERVER",
      "TIME", FTPTOP_REG_ARG_SIZE, "COMMAND");
  else
    printw(FTPTOP_XFER_HEADER_FMT, "PID", "S", "USER", "CLIENT", "KB/s", FTPTOP_XFER_DONE_SIZE, "%DONE");

  attroff(A_REVERSE);

  /* Write out the scoreboard entries. */
  if (ftp_sessions && ftp_nsessions) {
    register unsigned int i = 0;

    for (i = 0; i < ftp_nsessions; i++)
      printw("%s", ftp_sessions[i]);
  }

  wrefresh(stdscr);
}

static void toggle_mode(void) {
  static unsigned int cached_mode = 0;

  if (cached_mode == 0)
    cached_mode = display_mode;

  if (display_mode != FTPTOP_SHOW_RATES)
    display_mode = FTPTOP_SHOW_RATES;
  else
    display_mode = cached_mode;
}

static void show_version(void) {
  fprintf(stdout, FTPTOP_VERSION "\n");
  exit(0);
}

static void usage(void) {
  fprintf(stdout, "usage: ftptop [options]\n\n");
  fprintf(stdout, "\t-D      \t\tshow only downloading sessions\n");
  fprintf(stdout, "\t-d <num>\t\trefresh delay in seconds\n");
  fprintf(stdout, "\t-f      \t\tconfigures the ScoreboardFile to use\n");
  fprintf(stdout, "\t-h      \t\tdisplays this message\n");
  fprintf(stdout, "\t-i      \t\tignores idle connections when listing\n");
  fprintf(stdout, "\t-U      \t\tshow only uploading sessions\n");
  fprintf(stdout, "\t-V      \t\tshows version\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  Use the 't' key to toggle between \"regular\" and \"transfer speed\"\n");
  fprintf(stdout, "  display modes. Use the 'q' key to quit.\n\n");
  exit(0);
}

static void verify_scoreboard_file(void) {
  struct stat sbuf;

  if (stat(util_get_scoreboard(), &sbuf) < 0) {
    fprintf(stderr, "%s: unable to stat '%s': %s\n", program,
      util_get_scoreboard(), strerror(errno));
    exit(1);
  }
}

int main(int argc, char *argv[]) {

  /* Process command line options. */
  process_opts(argc, argv);

  /* Verify that the scoreboard file is useable. */
  verify_scoreboard_file();

  /* Install signal handlers. */
  signal(SIGINT, finish);
  signal(SIGTERM, finish);

  /* Initialize the display. */
  initscr();
  cbreak();
  noecho();
#ifndef HAVE_NCURSES
  nodelay(stdscr, TRUE);
#endif
  curs_set(0);

  /* Paint the initial display. */
  show_sessions();

  /* Loop endlessly. */
  for (;;) {
    int c = -1;

#ifdef HAVE_NCURSES
    if (halfdelay(delay * 10) != ERR)
      c = getch();
#else
    sleep(delay);
    c = getch();
#endif

    if (c != -1) {
      if (tolower(c) == 'q')
        finish(0);

      if (tolower(c) == 't')
        toggle_mode();
    }

    show_sessions();
  }

  /* done */
  finish(0);
}

#else /* defined(HAVE_CURSES) || defined(HAVE_NCURSES) */

#include <stdio.h>

int main(int argc, char *argv[]) {
  fprintf(stdout, "%s: no curses or ncurses library on this system\n", program);
  return 1;
}

#endif /* defined(HAVE_CURSES) || defined(HAVE_NCURSES) */

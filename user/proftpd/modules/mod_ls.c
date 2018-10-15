/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2004 The ProFTPD Project team
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

/* Directory listing module for ProFTPD.
 * $Id: mod_ls.c,v 1.125 2005/03/16 16:16:44 castaglia Exp $
 */

#include "conf.h"

#ifndef GLOB_ABORTED
#define GLOB_ABORTED GLOB_ABEND
#endif

#define MAP_UID(x) \
  (fakeuser ? fakeuser : pr_auth_uid2name(cmd->tmp_pool, (x)))

#define MAP_GID(x) \
  (fakegroup ? fakegroup : pr_auth_gid2name(cmd->tmp_pool, (x)))

static void addfile(cmd_rec *, const char *, const char *, time_t, off_t);
static int outputfiles(cmd_rec *);

static int listfile(cmd_rec *, pool *, const char *);
static int listdir(cmd_rec *, pool *, const char *);

static unsigned char list_strict_opts = FALSE;
static char *list_options = NULL;
static unsigned char list_show_symlinks = TRUE, list_times_gmt = TRUE;
static unsigned char show_symlinks_hold;
static char *fakeuser = NULL, *fakegroup = NULL;
static mode_t fakemode;
static unsigned char have_fake_mode = FALSE;
static int ls_errno = 0;
static time_t ls_curtime = 0;

static unsigned char use_globbing = TRUE;

/* Directory listing limits */
struct list_limit_rec {
  unsigned int curr, max;
  unsigned char logged;
};

static struct list_limit_rec list_ndepth;
static struct list_limit_rec list_ndirs;
static struct list_limit_rec list_nfiles;

/* ls options */
static int
    opt_a = 0,
    opt_A = 0,
    opt_C = 0,
    opt_d = 0,
    opt_F = 0,
    opt_h = 0,
    opt_l = 0,
    opt_L = 0,
    opt_n = 0,
    opt_R = 0,
    opt_r = 0,
    opt_S = 0,
    opt_t = 0,
    opt_STAT = 0;

static char cwd[PR_TUNABLE_PATH_MAX+1] = "";

/* Find a <Limit> block that limits the given command (which will probably
 * be LIST).  This code borrowed for src/dirtree.c's _dir_check_limit().
 * Note that this function is targeted specifically for ls commands (eg
 * LIST, NLST, DIRS, and ALL) that might be <Limit>'ed.
 */
static config_rec *_find_ls_limit(char *ftp_cmd) {
  config_rec *c = NULL, *limit_c = NULL;

  if (!ftp_cmd)
    return NULL;

  if (!session.dir_config)
    return NULL;

  /* Determine whether this command is <Limit>'ed. */
  for (c = session.dir_config; c; c = c->parent) {

    if (c->subset) {

      for (limit_c = (config_rec *) (c->subset->xas_list); limit_c;
          limit_c = limit_c->next) {

        if (limit_c->config_type == CONF_LIMIT) {
          register unsigned int i = 0;

          for (i = 0; i < limit_c->argc; i++) {

            /* match any of the appropriate <Limit> arguments
             */
            if (!strcasecmp(ftp_cmd, (char *) (limit_c->argv[i])) ||
                !strcasecmp("DIRS", (char *) (limit_c->argv[i])) ||
                !strcasecmp("ALL", (char *) (limit_c->argv[i])))
              break;
          }

          if (i == limit_c->argc)
            continue;

          /* Found a <Limit> directive associated with the current command
           */
          return limit_c;
        }
      }
    }
  }

  return NULL;
}

static void push_cwd(char *_cwd, unsigned char *symhold) {
  if (!_cwd)
    _cwd = cwd;

  if (!symhold)
    *symhold = show_symlinks_hold;

  sstrncpy(_cwd, pr_fs_getcwd(), PR_TUNABLE_PATH_MAX + 1);
  *symhold = list_show_symlinks;
}

static void pop_cwd(char *_cwd, unsigned char *symhold) {
  if (!_cwd)
    _cwd = cwd;

  if (!symhold)
    *symhold = show_symlinks_hold;

  pr_fsio_chdir(_cwd, *symhold);
  list_show_symlinks = *symhold;
}

static int ls_perms_full(pool *p, cmd_rec *cmd, const char *path, int *hidden) {
  int res, canon = 0;
  char *fullpath;
  mode_t *fake_mode = NULL;

  fullpath = dir_realpath(p, path);

  if (!fullpath) {
    fullpath = dir_canonical_path(p, path);
    canon = 1;
  }

  if (!fullpath)
    fullpath = pstrdup(p, path);

  if (canon)
    res = dir_check_canon(p, cmd->argv[0], cmd->group, fullpath, hidden);
  else
    res = dir_check(p, cmd->argv[0], cmd->group, fullpath, hidden);

  if (session.dir_config) {
    unsigned char *tmp = get_param_ptr(session.dir_config->subset,
      "ShowSymlinks", FALSE);

    if (tmp)
      list_show_symlinks = *tmp;
  }

  if ((fake_mode = get_param_ptr(CURRENT_CONF, "DirFakeMode", FALSE))) {
    fakemode = *fake_mode;
    have_fake_mode = TRUE;

  } else
    have_fake_mode = FALSE;

  return res;
}

static int ls_perms(pool *p, cmd_rec *cmd, const char *path,int *hidden) {
  int res = 0;
  char fullpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  mode_t *fake_mode = NULL;

  /* No need to process dotdirs. */
  if (is_dotdir(path))
    return 1;

  if (*path == '~')
    return ls_perms_full(p, cmd, path, hidden);

  if (*path != '/')
    pr_fs_clean_path(pdircat(p, pr_fs_getcwd(), path, NULL), fullpath,
      PR_TUNABLE_PATH_MAX);
  else
    pr_fs_clean_path(path, fullpath, PR_TUNABLE_PATH_MAX);

  res = dir_check(p, cmd->argv[0], cmd->group, fullpath, hidden);

  if (session.dir_config) {
    unsigned char *tmp = get_param_ptr(session.dir_config->subset,
      "ShowSymlinks",FALSE);

    if (tmp)
      list_show_symlinks = *tmp;
  }

  if ((fake_mode = get_param_ptr(CURRENT_CONF, "DirFakeMode", FALSE))) {
    fakemode = *fake_mode;
    have_fake_mode = TRUE;

  } else
    have_fake_mode = FALSE;

  return res;
}

/* sendline() now has an internal buffer, to help speed up LIST output. */
static int sendline(char *fmt, ...) {
  static char listbuf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  va_list msg;
  char buf[PR_TUNABLE_BUFFER_SIZE+1] = {'\0'};
  int res = 0;

  /* A NULL fmt argument is the signal to flush the buffer */
  if (!fmt) {
    if ((res = pr_data_xfer(listbuf, strlen(listbuf))) < 0)
      pr_log_debug(DEBUG3, "pr_data_xfer returned %d, error = %s.", res,
        strerror(PR_NETIO_ERRNO(session.d->outstrm)));

    memset(listbuf, '\0', sizeof(listbuf));
    return res;
  }

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);

  buf[sizeof(buf)-1] = '\0';

  /* If buf won't fit completely into listbuf, flush listbuf */
  if (strlen(buf) >= (sizeof(listbuf) - strlen(listbuf))) {
    if ((res = pr_data_xfer(listbuf, strlen(listbuf))) < 0)
      pr_log_debug(DEBUG3, "pr_data_xfer returned %d, error = %s.", res,
        strerror(PR_NETIO_ERRNO(session.d->outstrm)));

    memset(listbuf, '\0', sizeof(listbuf));
  }

  sstrcat(listbuf, buf, sizeof(listbuf));
  return res;
}

static void ls_done(cmd_rec *cmd) {
  pr_data_close(FALSE);
}

static char units[6][2] = 
  { "", "k", "M", "G", "T", "P" };

static void ls_fmt_filesize(char *buf, size_t buflen, off_t sz) {
  if (!opt_h || sz < 1000) {
    snprintf(buf, buflen, "%8" PR_LU, (pr_off_t) sz);

  } else {
    register unsigned int i = 0;
    float size = sz;

    /* Determine the appropriate units label to use. */
    while (size >= 1024.0) {
      size /= 1024.0;
      i++;
    }

    snprintf(buf, buflen, "%7.1f%s", size, units[i]);
  }
}

static char months[12][4] =
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static int listfile(cmd_rec *cmd, pool *p, const char *name) {
  int rval = 0, len;
  time_t mtime;
  char m[1024] = {'\0'}, l[1024] = {'\0'}, s[16] = {'\0'};
  struct stat st;
  struct tm *t = NULL;
  char suffix[2];
  int hidden = 0;

  if (list_nfiles.curr && list_nfiles.max &&
      list_nfiles.curr >= list_nfiles.max) {

    if (!list_nfiles.logged) {
      pr_log_debug(DEBUG8, "ListOptions maxfiles (%u) reached",
        list_nfiles.max);
      list_nfiles.logged = TRUE;
    }
 
    return 2;
  }
  list_nfiles.curr++;

  if (!p)
    p = cmd->tmp_pool;

  if (pr_fsio_lstat(name, &st) == 0) {
    suffix[0] = suffix[1] = '\0';

    if (S_ISLNK(st.st_mode) && (opt_L || !list_show_symlinks)) {
      /* Attempt to fully dereference symlink */
      struct stat l_st;

      pr_fs_clear_cache();
      if (pr_fsio_stat(name, &l_st) != -1) {
        memcpy(&st, &l_st, sizeof(struct stat));

        len = pr_fsio_readlink(name, m, sizeof(m));
        if (len < 0)
          return 0;

        m[len] = '\0';

        if (!ls_perms_full(p, cmd, m, NULL))
          return 0;

      } else
        return 0;

    } else if (S_ISLNK(st.st_mode)) {
      len = pr_fsio_readlink(name, l, sizeof(l));
      if (len < 0)
        return 0;

      l[len] = '\0';

      if (!ls_perms_full(p, cmd, l, &hidden))
        return 0;

    } else if (!ls_perms(p, cmd, name, &hidden))
      return 0;

    /* Skip dotfiles, unless requested not to via -a or -A. */
    if (*name == '.' &&
        (!opt_a && (!opt_A || is_dotdir(name))))
      return 0;

    if (hidden)
      return 0;

    mtime = st.st_mtime;

    if (list_times_gmt)
      t = pr_gmtime(p, (time_t *) &mtime);
    else
      t = pr_localtime(p, (time_t *) &mtime);

    if (!t) {
      pr_response_add_err(R_421, "Fatal error (localtime() returned NULL?!?)");
      return -1;
    }

    if (opt_F) {
      if (S_ISLNK(st.st_mode))
        suffix[0] = '@';

      else if (S_ISDIR(st.st_mode)) {
        suffix[0] = '/';
        rval = 1;

      } else if (st.st_mode & 0111)
        suffix[0] = '*';
    }

    if (opt_l) {
      sstrncpy(m, " ---------", sizeof(m));
      switch (st.st_mode & S_IFMT) {
        case S_IFREG:
          m[0] = '-';
          break;

        case S_IFLNK:
          m[0] = 'l';
          break;

#ifdef S_IFSOCK
        case S_IFSOCK:
          m[0] = 's';
          break;
#endif /* S_IFSOCK */

        case S_IFBLK:
          m[0] = 'b';
          break;

        case S_IFCHR:
          m[0] = 'c';
          break;

        case S_IFIFO:
          m[0] = 'p';
          break;

        case S_IFDIR:
          m[0] = 'd';
          rval = 1;
          break;
      }

      if (m[0] != ' ') {
        char nameline[(PR_TUNABLE_PATH_MAX * 2) + 128] = {'\0'};
        char timeline[6] = {'\0'};
        mode_t mode = st.st_mode;

        if (have_fake_mode) {
          mode = fakemode;

          if (S_ISDIR(st.st_mode)) {
            if (mode & S_IROTH) mode |= S_IXOTH;
            if (mode & S_IRGRP) mode |= S_IXGRP;
            if (mode & S_IRUSR) mode |= S_IXUSR;
          }
        }

        m[9] = (mode & S_IXOTH)
                ? ((mode & S_ISVTX) ? 't' : 'x')
                : ((mode & S_ISVTX) ? 'T' : '-');
        m[8] = (mode & S_IWOTH) ? 'w' : '-';
        m[7] = (mode & S_IROTH) ? 'r' : '-';
        m[6] = (mode & S_IXGRP)
                ? ((mode & S_ISGID) ? 's' : 'x')
                : ((mode & S_ISGID) ? 'S' : '-');
        m[5] = (mode & S_IWGRP) ? 'w' : '-';
        m[4] = (mode & S_IRGRP) ? 'r' : '-';
        m[3] = (mode & S_IXUSR) ? ((mode & S_ISUID)
                ? 's' : 'x')
                :  ((mode & S_ISUID) ? 'S' : '-');
        m[2] = (mode & S_IWUSR) ? 'w' : '-';
        m[1] = (mode & S_IRUSR) ? 'r' : '-';

        if (ls_curtime - mtime > 180 * 24 * 60 * 60)
          snprintf(timeline, sizeof(timeline), "%5d", t->tm_year+1900);

        else
          snprintf(timeline, sizeof(timeline), "%02d:%02d", t->tm_hour,
            t->tm_min);

        ls_fmt_filesize(s, sizeof(s), st.st_size);

        if (!opt_n) {

          /* Format nameline using user/group names. */
          snprintf(nameline, sizeof(nameline)-1,
            "%s %3d %-8s %-8s %s %s %2d %s %s", m, (int) st.st_nlink,
            MAP_UID(st.st_uid), MAP_GID(st.st_gid), s,
            months[t->tm_mon], t->tm_mday, timeline, name);

        } else {

          /* Format nameline using user/group IDs. */
          snprintf(nameline, sizeof(nameline)-1,
            "%s %3d %-8u %-8u %s %s %2d %s %s", m, (int) st.st_nlink,
            (unsigned) st.st_uid, (unsigned) st.st_gid, s,
            months[t->tm_mon], t->tm_mday, timeline, name);
        }

        nameline[sizeof(nameline)-1] = '\0';

        if (S_ISLNK(st.st_mode)) {
          char *buf = nameline + strlen(nameline);

          suffix[0] = '\0';
          if (opt_F && pr_fsio_stat(name, &st) == 0) {
            if (S_ISLNK(st.st_mode))
              suffix[0] = '@';

            else if (S_ISDIR(st.st_mode))
              suffix[0] = '/';

            else if (st.st_mode & 0111)
              suffix[0] = '*';
          }

          if (!opt_L && list_show_symlinks) {
            if (sizeof(nameline) - strlen(nameline) > 4)
              snprintf(buf, sizeof(nameline) - strlen(nameline) - 4,
                " -> %s", l);
            else
              pr_log_pri(PR_LOG_NOTICE, "notice: symlink '%s' yields an "
                "excessive string, ignoring", name);
          }

          nameline[sizeof(nameline)-1] = '\0';
        }

        if (opt_STAT)
          pr_response_add(R_211, "%s%s", nameline, suffix);
        else
          addfile(cmd, nameline, suffix, mtime, st.st_size);
      }

    } else {
      if (S_ISREG(st.st_mode) ||
          S_ISDIR(st.st_mode) ||
          S_ISLNK(st.st_mode))
           addfile(cmd, name, suffix, mtime, st.st_size);
    }
  }

  return rval;
}

static int colwidth = 0;
static int filenames = 0;

struct filename {
  struct filename *down;
  struct filename *right;
  char *line;
  int top;
};

struct sort_filename {
  time_t mtime;
  off_t size;
  char *name;
  char *suffix;
};

static struct filename *head = NULL;
static struct filename *tail = NULL;
static array_header *sort_arr = NULL;
static pool *fpool = NULL;

static void addfile(cmd_rec *cmd, const char *name, const char *suffix,
    time_t mtime, off_t size) {
  struct filename *p;
  size_t l;

  if (!name || !suffix)
    return;

  if (!fpool) {
    fpool = make_sub_pool(cmd->tmp_pool);
    pr_pool_tag(fpool, "mod_ls: addfile() fpool");
  }

  if (opt_S || opt_t) {
    struct sort_filename *s;

    if (!sort_arr)
      sort_arr = make_array(fpool, 50, sizeof(struct sort_filename));

    s = (struct sort_filename *) push_array(sort_arr);
    s->mtime = mtime;
    s->size = size;
    s->name = pstrdup(fpool, name);
    s->suffix = pstrdup(fpool, suffix);

    return;
  }

  l = strlen(name) + strlen(suffix);
  if (l > colwidth)
    colwidth = l;

  p = (struct filename *) pcalloc(fpool, sizeof(struct filename));
  p->line = pcalloc(fpool, l + 2);
  snprintf(p->line, l + 1, "%s%s", name, suffix);

  if (tail)
    tail->down = p;

  else
    head = p;

  tail = p;
  filenames++;
}

static int file_mtime_cmp(const struct sort_filename *f1,
    const struct sort_filename *f2) {

  if (f1->mtime > f2->mtime)
    return -1;

  else if (f1->mtime < f2->mtime)
    return 1;

  return 0;
}

static int file_mtime_reverse_cmp(const struct sort_filename *f1,
    const struct sort_filename *f2) {
  return -file_mtime_cmp(f1, f2);
}

static int file_size_cmp(const struct sort_filename *f1,
    const struct sort_filename *f2) {

  if (f1->size > f2->size)
    return -1;

  else if (f1->size < f2->size)
    return 1;

  return 0;
}

static int file_size_reverse_cmp(const struct sort_filename *f1,
    const struct sort_filename *f2) {
  return -file_size_cmp(f1, f2);
}

static void sortfiles(cmd_rec *cmd) {

  if (sort_arr) {

    /* Sort by modification time? */
    if (opt_t) {
      register unsigned int i = 0;
      int setting = opt_S;
      struct sort_filename *elts = sort_arr->elts;

      qsort(sort_arr->elts, sort_arr->nelts, sizeof(struct sort_filename),
        (int (*)(const void *, const void *))
          (opt_r ? file_mtime_reverse_cmp : file_mtime_cmp));

      opt_S = opt_t = 0;

      for (i = 0; i < sort_arr->nelts; i++)
        addfile(cmd, elts[i].name, elts[i].suffix, elts[i].mtime, elts[i].size);

      opt_S = setting;
      opt_t = 1;

    /* Sort by file size? */
    } else if (opt_S) {
      register unsigned int i = 0;
      int setting = opt_t;
      struct sort_filename *elts = sort_arr->elts;

      qsort(sort_arr->elts, sort_arr->nelts, sizeof(struct sort_filename),
        (int (*)(const void *, const void *))
          (opt_r ? file_size_reverse_cmp : file_size_cmp));

      opt_S = opt_t = 0;

      for (i = 0; i < sort_arr->nelts; i++)
        addfile(cmd, elts[i].name, elts[i].suffix, elts[i].mtime, elts[i].size);

      opt_S = 1;
      opt_t = setting;
    }
  }

  sort_arr = NULL;
}

static int outputfiles(cmd_rec *cmd) {
  int n;
  struct filename *p = NULL, *q = NULL;

  if (opt_S || opt_t)
    sortfiles(cmd);

  if (!head)		/* nothing to display */
    return 0;

  tail->down = NULL;
  tail = NULL;
  colwidth = ( colwidth | 7 ) + 1;
  if (opt_l || !opt_C)
    colwidth = 75;

  /* avoid division by 0 if colwidth > 75 */
  if (colwidth > 75)
    colwidth = 75;

  p = head;
  p->top = 1;
  n = (filenames + (75 / colwidth)-1) / (75 / colwidth);
  while (n && p) {
    p = p->down;
    if (p)
      p->top = 0;
    n--;
  }

  q = head;
  while (p) {
    p->top = q->top;
    q->right = p;
    q = q->down;
    p = p->down;
  }

  while (q) {
    q->right = NULL;
    q = q->down;
  }

  p = head;
  while (p && p->down && !p->down->top)
    p = p->down;
  if (p && p->down)
    p->down = NULL;

#if 0
  if (opt_l)
    if (sendline("total 0\n") < 0)
      return -1;
#endif

  p = head;
  while (p) {
    q = p;
    p = p->down;
    while (q) {
      char pad[6] = {'\0'};

      if (q->right) {
        sstrncpy(pad, "\t\t\t\t\t", sizeof(pad));
        pad[(colwidth + 7 - strlen(q->line)) / 8] = '\0';

      } else {
        sstrncpy(pad, "\n", sizeof(pad));
      }

      if (sendline("%s%s", q->line, pad) < 0)
        return -1;

      q = q->right;
    }
  }

  destroy_pool(fpool);
  fpool = NULL;
  sort_arr = NULL;
  head = tail = NULL;
  colwidth = 0;
  filenames = 0;

  /* flush the buffer */
  if (sendline(NULL) < 0)
    return -1;

  return 0;
}

static void discard_output(void) {
  if (fpool)
    destroy_pool(fpool);
  fpool = NULL;

  head = tail = NULL;
  colwidth = 0;
  filenames = 0;
}

static int dircmp(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

static char **sreaddir(const char *dirname, const int sort) {
  DIR 		*d;
  struct	dirent *de;
  struct	stat st;
  int		i;
  char		**p;
  int		dsize, ssize;
  int		dir_fd;

  if (pr_fsio_stat(dirname, &st) < 0)
    return NULL;

  if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return NULL;
  }

  if ((d = pr_fsio_opendir(dirname)) == NULL)
    return NULL;

  /* It doesn't matter if the following guesses are wrong, but it slows
   * the system a bit and wastes some memory if they are wrong, so
   * don't guess *too* naively!
   *
   * 'dsize' must be greater than zero or we loop forever.
   * 'ssize' must be at least big enough to hold a maximum-length name.
   */
  dsize = (st.st_size / 4) + 10;	 /* Guess number of entries in dir */

  /*
  ** The directory has been opened already, but portably accessing the file
  ** descriptor inside the DIR struct isn't easy.  Some systems use "dd_fd" or
  ** "__dd_fd" rather than "d_fd".  Still others work really hard at opacity.
  */
#if defined(HAVE_STRUCT_DIR_D_FD)
  dir_fd = d->d_fd;
#elif defined(HAVE_STRUCT_DIR_DD_FD)
  dir_fd = d->dd_fd;
#elif defined(HAVE_STRUCT_DIR___DD_FD)
  dir_fd = d->__dd_fd;
#else
  dir_fd = 0;
#endif
  if ((ssize = get_name_max((char *) dirname, dir_fd)) < 1 ) {
    pr_log_debug(DEBUG1, "get_name_max(%s, %d) = %d, using %d", dirname,
      dir_fd, ssize, NAME_MAX_GUESS);
    ssize = NAME_MAX_GUESS;
  }

  ssize *= ((dsize / 4) + 1);

  /* Allocate first block for holding filenames.  Yes, we are explicitly using
   * malloc (and realloc, and calloc, later) rather than the memory pools.
   * Recursive directory listings would eat up a lot of pool memory that is
   * only freed when the _entire_ directory structure has been parsed.  Also,
   * this helps to keep the memory footprint a little smaller.
   */
  if ((p = (char **) malloc(dsize * sizeof(char *))) == NULL) {
    pr_log_pri(PR_LOG_ERR, "fatal: memory exhausted");
    exit(1);
  }

  i = 0;

  while ((de = pr_fsio_readdir(d)) != NULL) {

    if (i >= dsize - 1) {
      char **newp;

      /* The test above goes off one item early in case this is the last item
       * in the directory and thus next time we will want to NULL-terminate
       * the array.
       */
      pr_log_debug(DEBUG0, "Reallocating sreaddir buffer from %d entries to %d "
        "entries", dsize, dsize * 2);

      /* Allocate bigger array for pointers to filenames */
      if ((newp = (char **) realloc(p, 2 * dsize * sizeof(char *))) == NULL) {
        pr_log_pri(PR_LOG_ERR, "fatal: memory exhausted");
        exit(1);
      }
      p = newp;
      dsize *= 2;
    }

    /* Append the filename to the block. */
    if ((p[i] = (char *) calloc(strlen(de->d_name) + 1,
        sizeof(char))) == NULL) {
      pr_log_pri(PR_LOG_ERR, "fatal: memory exhausted");
      exit(1);
    }
    sstrncpy(p[i++], de->d_name, strlen(de->d_name) + 1);
  }

  pr_fsio_closedir(d);

  /* This is correct, since the above is off by one element.
   */
  p[i] = NULL;

  if (sort)
    qsort(p, i, sizeof(char *), dircmp);

  return p;
}

/* This listdir() requires a chdir() first. */
static int listdir(cmd_rec *cmd, pool *workp, const char *name) {
  char **dir;
  int dest_workp = 0;
  config_rec *c = NULL;
  unsigned char ignore_hidden = FALSE;
  register unsigned int i = 0;

  if (list_ndepth.curr && list_ndepth.max &&
      list_ndepth.curr >= list_ndepth.max) {

    if (!list_ndepth.logged) {
      /* Don't forget to take away the one we add to maxdepth internally. */
      pr_log_debug(DEBUG8, "ListOptions maxdepth (%u) reached",
        list_ndepth.max - 1);
      list_ndepth.logged = TRUE;
    }
 
    return 1;
  }

  if (list_ndirs.curr && list_ndirs.max &&
      list_ndirs.curr >= list_ndirs.max) {

    if (!list_ndirs.logged) {
      pr_log_debug(DEBUG8, "ListOptions maxdirs (%u) reached", list_ndirs.max);
      list_ndirs.logged = TRUE;
    }

    return 1;
  }
  list_ndirs.curr++;

  if (XFER_ABORTED)
    return -1;

  if (!workp) {
    workp = make_sub_pool(cmd->tmp_pool);
    pr_pool_tag(workp, "mod_ls: listdir(): workp (from cmd->tmp_pool)");
    dest_workp++;

  } else {
    workp = make_sub_pool(workp);
    pr_pool_tag(workp, "mod_ls: listdir(): workp (from workp)");
    dest_workp++;
  }

  dir = sreaddir(".", TRUE);

  /* Search for relevant <Limit>'s to this LIST command.  If found,
   * check to see whether hidden files should be ignored.
   */
  if ((c = _find_ls_limit(cmd->argv[0])) != NULL) {
    unsigned char *ignore = get_param_ptr(c->subset, "IgnoreHidden", FALSE);

    if (ignore && *ignore == TRUE)
      ignore_hidden = TRUE;
  }

  if (dir) {
    char **s;
    char **r;

    int d = 0;

#if 0
    if (opt_l) {
      if (opt_STAT)
        pr_response_add(R_211, "total 0");
      else if (sendline("total 0\n") < 0)
        return -1;
    }
#endif

    s = dir;
    while (*s) {
      if (**s == '.') {
        if (!opt_a && (!opt_A || is_dotdir(*s))) {
          d = 0;

        } else {

          /* Make sure IgnoreHidden is properly honored. "." and ".." are
           * not to be treated as hidden files, though.
           */
          d = listfile(cmd, workp, *s);
        }

      } else {
        d = listfile(cmd, workp, *s);
      }

      if (opt_R && d == 0) {

        /* This is a nasty hack.  If listfile() returns a zero, and we
         * will be recursing (-R option), make sure we don't try to list
         * this file again by changing the first character of the path
         * to ".".  Such files are skipped later.
         */
        **s = '.';
        *(*s + 1) = '\0';

      } else if (d == 2)
        break;

      s++;
    }

    if (outputfiles(cmd) < 0) {
      if (dest_workp)
        destroy_pool(workp);

      /* Explicitly free the memory allocated for containing the list of
       * filenames.
       */
      i = 0;
      while (dir[i] != NULL)
        free(dir[i++]);
      free(dir);

      return -1;
    }

    r = dir;
    while (opt_R && r != s) {
      char cwd_buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
      unsigned char symhold;

      if (*r && (strcmp(*r, ".") == 0 || strcmp(*r, "..") == 0)) {
        r++;
        continue;
      }

      /* Add some signal processing to this while loop, as it can
       * potentially recurse deeply.
       */
      pr_signals_handle();

      if (list_ndirs.curr && list_ndirs.max &&
          list_ndirs.curr >= list_ndirs.max) {

        if (!list_ndirs.logged) {
          pr_log_debug(DEBUG8, "ListOptions maxdirs (%u) reached",
            list_ndirs.max);
          list_ndirs.logged = TRUE;
        }

        break;
      }

      if (list_nfiles.curr && list_nfiles.max &&
          list_nfiles.curr >= list_nfiles.max) {

        if (!list_nfiles.logged) {
          pr_log_debug(DEBUG8, "ListOptions maxfiles (%u) reached",
            list_nfiles.max);
          list_nfiles.logged = TRUE;
        }

        break;
      }

      push_cwd(cwd_buf, &symhold);

      if (*r && ls_perms_full(workp, cmd, (char *) *r, NULL) &&
          !pr_fsio_chdir_canon(*r, !opt_L && list_show_symlinks)) {
        char *subdir;
        int res = 0;

        if (strcmp(name, ".") == 0)
          subdir = *r;
        else
          subdir = pdircat(workp, name, *r, NULL);

        if (opt_STAT) {
          pr_response_add(R_211, "%s", "");
          pr_response_add(R_211, "%s:", subdir);

        } else if (sendline("\n%s:\n", subdir) < 0 ||
            sendline(NULL) < 0) {
          pop_cwd(cwd_buf, &symhold);

          if (dest_workp)
            destroy_pool(workp);

          return -1;
        }

        list_ndepth.curr++;
        res = listdir(cmd, workp, subdir);
        list_ndepth.curr--;
        pop_cwd(cwd_buf, &symhold);

        if (res > 0) {
          break;

        } else if (res < 0) {
          if (dest_workp)
            destroy_pool(workp);

          /* Explicitly free the memory allocated for containing the list of
           * filenames.
           */
          i = 0;
          while (dir[i] != NULL)
            free(dir[i++]);
          free(dir);

          return -1;
        }
      }
      r++;
    }
  }

  if (dest_workp)
    destroy_pool(workp);

  /* Explicitly free the memory allocated for containing the list of
   * filenames.
   */
  if (dir) {
    i = 0;
    while (dir[i] != NULL)
      free(dir[i++]);
    free(dir);
  }

  return 0;
}

static void ls_terminate(void) {
  if (!opt_STAT) {
    discard_output();

    if (!XFER_ABORTED) {
      /* An error has occured, other than client ABOR */
      if (ls_errno)
        pr_data_abort(ls_errno,FALSE);
      else
        pr_data_abort((session.d && session.d->outstrm ?
                   PR_NETIO_ERRNO(session.d->outstrm) : errno),FALSE);
    }
    ls_errno = 0;

  } else if (ls_errno) {
    pr_response_add(R_211, "ERROR: %s", strerror(ls_errno));
    ls_errno = 0;
  }
}

static void parse_list_opts(char **opt, int *glob_flags, int handle_plus_opts) {
  while (isspace((int) **opt))
    (*opt)++;

  /* Check for standard /bin/ls options */
  while (*opt && **opt == '-') {
    while ((*opt)++ && isalnum((int) **opt)) {
      switch (**opt) {
        case '1':
          opt_l = opt_C = 0;
          break;

        case 'A':
          opt_A = 1;
          break;

        case 'a':
          opt_a = 1;
          break;

        case 'C':
          if (strcmp(session.curr_cmd, C_NLST) != 0) {
            opt_l = 0;
            opt_C = 1;
          }
          break;

        case 'd':
          opt_d = 1;
          break;

        case 'F':
          if (strcmp(session.curr_cmd, C_NLST) != 0) {
            opt_F = 1;
          }
          break;

        case 'h':
          if (strcmp(session.curr_cmd, C_NLST) != 0) {
            opt_h = 1;
          }
          break;

        case 'L':
          opt_L = 1;
          break;

        case 'l':
          if (strcmp(session.curr_cmd, C_NLST) != 0) {
            opt_l = 1;
            opt_C = 0;
          }
          break;

        case 'n':
          if (strcmp(session.curr_cmd, C_NLST) != 0) {
            opt_n = 1;
          }
          break;

        case 'R':
          opt_R = 1;
          break;

        case 'r':
          opt_r = 1;
          break;

        case 'S':
          opt_S = 1;
          break;

        case 't':
          opt_t = 1;
          if (glob_flags)
            *glob_flags |= GLOB_NOSORT;
          break;
      }
    }

    while (isspace((int) **opt))
      (*opt)++;
  }

  if (!handle_plus_opts)
    return;

  /* Check for non-standard options */
  while (*opt && **opt == '+') {
    while ((*opt)++ && isalnum((int) **opt)) {
      switch (**opt) {
        case '1':
          opt_l = opt_C = 0;
          break;

        case 'A':
          opt_A = 0;
          break;

        case 'a':
          opt_a = 0;
          break;

        case 'C':
          opt_l = opt_C = 0;
          break;

        case 'd':
          opt_d = 0;
          break;

        case 'F':
          opt_F = 0;
          break;

        case 'h':
          opt_h = 0;
          break;

        case 'L':
          opt_L = 0;
          break;

        case 'l':
          opt_l = opt_C = 0;
          break;

        case 'n':
          opt_n = 0;
          break;

        case 'R':
          opt_R = 0;
          break;

        case 'r':
          opt_r = 0;
          break;

        case 'S':
          opt_S = 0;
          break;

        case 't':
          opt_t = 0;
          if (glob_flags)
            *glob_flags &= GLOB_NOSORT;
          break;
      }
    }

    while (isspace((int) **opt))
      (*opt)++;
  }
}

/* The main work for LIST and STAT (not NLST).  Returns -1 on error, 0 if
 * successful.
 */
static int dolist(cmd_rec *cmd, const char *opt, int clearflags) {
  int skiparg = 0;
  int glob_flags = GLOB_PERIOD;
  char *arg = (char*) opt;

  ls_curtime = time(NULL);

  if (clearflags)
    opt_a = opt_C = opt_d = opt_F = opt_h = opt_n = opt_r = opt_R =
      opt_S = opt_t = opt_STAT = opt_L = 0;

  if (!list_strict_opts) {
    parse_list_opts(&arg, &glob_flags, FALSE);

  } else {

    /* Even if the user-given options are ignored, they still need to
     * "processed" (ie skip past options) in order to get to the paths.
     */
    while (*arg && isspace((int) *arg))
      arg++;

    while (arg && *arg == '-') {

      /* Advance to the next whitespace */
      while (*arg != '\0' && !isspace((int) *arg))
        arg++;

      while (isspace((int) *arg))
        arg++;
    }

    while (isspace((int) *arg))
      arg++;
  }

  if (list_options)
    parse_list_opts(&list_options, &glob_flags, TRUE);

  if (arg && *arg) {
    int justone = 1;
    glob_t g;
    int globbed = FALSE;
    int a;
    char   pbuffer[PR_TUNABLE_PATH_MAX + 1] = "";
    char *target;

    /* Make sure the glob_t is initialized. */
    memset(&g, '\0', sizeof(g));

    if (*arg == '~') {
      struct passwd *pw;
      int i;
      const char *p;

      for (i = 0, p = arg + 1;
          (i < sizeof(pbuffer) - 1) && p && *p && *p != '/';
          pbuffer[i++] = *p++);

      pbuffer[i] = '\0';

      pw = pr_auth_getpwnam(cmd->tmp_pool, i ? pbuffer : session.user);
      if (pw) {
        snprintf(pbuffer, sizeof(pbuffer), "%s%s", pw->pw_dir, p);

      } else
        pbuffer[0] = '\0';
    }

    target = *pbuffer ? pbuffer : arg;

    /* If there are no globbing characters in the given target,
     * we can check to see if it even exists.
     */
    if (strpbrk(target, "{[*?") == NULL) {
      struct stat st;

      pr_fs_clear_cache();
      if (pr_fsio_stat(target, &st) < 0) {
        pr_response_add_err(R_450, "%s: %s", target, strerror(errno));
        return -1;
      }
    }

    /* Open data connection */
    if (!opt_STAT) {
      session.sf_flags |= SF_ASCII_OVERRIDE;
      if (pr_data_open(NULL, "file list", PR_NETIO_IO_WR, 0) < 0)
        return -1;
    }

    /* Check perms on the directory/file we are about to scan. */
    if (!ls_perms_full(cmd->tmp_pool, cmd, target, NULL)) {
      a = -1;
      skiparg = TRUE;

    } else {

      skiparg = FALSE;

      if (use_globbing &&
          strpbrk(target, "{[*?") != NULL) {
        a = pr_fs_glob(target, glob_flags, NULL, &g);
        globbed = TRUE;

      } else {

        /* Trick the following code into using the non-glob() processed path */
        a = 0;
        g.gl_pathv = (char **) pcalloc(cmd->tmp_pool, 2 * sizeof(char *));
        g.gl_pathv[0] = (char *) pstrdup(cmd->tmp_pool, target);
        g.gl_pathv[1] = NULL;
      }
    }

    if (!a) {
      int list_dir_as_file = FALSE;
      char **path;

      /* If glob characters are present, and if recursion has not been
       * explicitly requested, then do not recurse.  Do this by treating
       * directories as files for listing purposes.
       */
      if (use_globbing &&
          strpbrk(target, "{[*?") != NULL &&
          !opt_R)
        list_dir_as_file = TRUE;

      path = g.gl_pathv;

      if (path && path[0] && path[1])
        justone = 0;

      while (path && *path) {
        struct stat st;

        if (pr_fsio_lstat(*path, &st) == 0) {
          mode_t target_mode, lmode;
          target_mode = st.st_mode;

          if (S_ISLNK(st.st_mode) && (lmode = file_mode((char*)*path)) != 0) {
            if (opt_L || !list_show_symlinks)
              st.st_mode = lmode;
            target_mode = lmode;
          }

          if (opt_d ||
              !(S_ISDIR(target_mode)) ||
              (S_ISDIR(target_mode) && list_dir_as_file)) {
            if (listfile(cmd, NULL, *path) < 0) {
              ls_terminate();
              if (use_globbing && globbed)
                pr_fs_globfree(&g);
              return -1;
            }

            **path = '\0';
          }

        } else 
          **path = '\0';

        path++;
      }

      if (outputfiles(cmd) < 0) {
        ls_terminate();
        if (use_globbing && globbed) {
          pr_fs_globfree(&g);
        }
        return -1;
      }

      path = g.gl_pathv;
      while (path && *path) {
        if (**path && ls_perms_full(cmd->tmp_pool, cmd, *path, NULL)) {
          char cwd_buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
          unsigned char symhold;

          if (!justone) {
            if (opt_STAT) {
              pr_response_add(R_211, "%s", "");
              pr_response_add(R_211, "%s:", *path);

            } else {
              sendline("\n%s:\n", *path);
              sendline(NULL);
            }
          }

          push_cwd(cwd_buf, &symhold);

          if (!pr_fsio_chdir_canon(*path, !opt_L && list_show_symlinks)) {
            int res = 0;

            list_ndepth.curr++;
            res = listdir(cmd, NULL, *path);
            list_ndepth.curr--;

            pop_cwd(cwd_buf, &symhold);

            if (res > 0) {
              break;

            } else if (res < 0) {
              ls_terminate();
              if (use_globbing && globbed)
                pr_fs_globfree(&g);
              return -1;
            }
          }
        }

        if (XFER_ABORTED) {
          discard_output();
          if (use_globbing && globbed)
            pr_fs_globfree(&g);
          return -1;
        }

        path++;
      }

    } else if (!skiparg) {
      if (a == GLOB_NOSPACE)
        pr_response_add(R_226, "Out of memory during globbing of %s", arg);

      else if (a == GLOB_ABORTED)
        pr_response_add(R_226, "Read error during globbing of %s", arg);

      else if (a != GLOB_NOMATCH)
        pr_response_add(R_226, "Unknown error during globbing of %s", arg);
    }

    if (!skiparg && use_globbing && globbed)
      pr_fs_globfree(&g);

    if (XFER_ABORTED) {
      discard_output();
      return -1;
    }

  } else {

    /* Open data connection */
    if (!opt_STAT) {
      session.sf_flags |= SF_ASCII_OVERRIDE;
      if (pr_data_open(NULL, "file list", PR_NETIO_IO_WR, 0) < 0)
        return -1;
    }

    if (ls_perms_full(cmd->tmp_pool, cmd, ".", NULL)) {

      if (opt_d) {
        if (listfile(cmd, NULL, ".") < 0) {
          ls_terminate();
          return -1;
        }

      } else {
        list_ndepth.curr++;
        if (listdir(cmd, NULL, ".") < 0) {
          ls_terminate();
          return -1;
        }

        list_ndepth.curr--;
      }
    }

    if (outputfiles(cmd) < 0) {
      ls_terminate();
      return -1;
    }
  }

  return 0;
}

/* Display listing of a single file, no permission checking is done.
 * An error is only returned if the data connection cannot be opened or is
 * aborted.
 */
static int nlstfile(cmd_rec *cmd, const char *file) {
  int res = 0;

  /* If the data connection isn't open, open it now. */
  if ((session.sf_flags & SF_XFER) == 0) {
    if (pr_data_open(NULL, "file list", PR_NETIO_IO_WR, 0) < 0) {
      pr_data_reset();
      return -1;
    }

    session.sf_flags |= SF_ASCII_OVERRIDE;
  }

  if (dir_hide_file(file))
    return 1;

  /* Be sure to flush the output */
  if ((res = sendline("%s\n", file)) < 0 ||
      (res = sendline(NULL)) < 0)
    return res;

  return 1;
}

/* Display listing of a directory, ACL checks performed on each entry,
 * sent in NLST fashion.  Files which are inaccessible via ACL are skipped,
 * error returned if data conn cannot be opened or is aborted.
 */
static int nlstdir(cmd_rec *cmd, const char *dir) {
  char **list, *p, *f,
       file[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  char cwd_buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  pool *workp;
  unsigned char symhold;
  int curdir = 0, i, j, count = 0, hidden = 0;
  mode_t mode;
  config_rec *c = NULL;
  unsigned char ignore_hidden = FALSE;

  if (list_ndepth.curr && list_ndepth.max &&
      list_ndepth.curr >= list_ndepth.max) {

    if (!list_ndepth.logged) {
      /* Don't forget to take away the one we add to maxdepth internally. */
      pr_log_debug(DEBUG8, "ListOptions maxdepth (%u) reached",
        list_ndepth.max - 1);
      list_ndepth.logged = TRUE;
    }

    return 0;
  }

  if (list_ndirs.curr && list_ndirs.max &&
      list_ndirs.curr >= list_ndirs.max) {

    if (!list_ndirs.logged) {
      pr_log_debug(DEBUG8, "ListOptions maxdirs (%u) reached", list_ndirs.max);
      list_ndirs.logged = TRUE;
    }

    return 0;
  }
  list_ndirs.curr++;

  workp = make_sub_pool(cmd->tmp_pool);
  pr_pool_tag(workp, "mod_ls: nlstdir(): workp (from cmd->tmp_pool)");

  if (!*dir || (*dir == '.' && !dir[1]) || strcmp(dir, "./") == 0) {
    curdir = 1;
    dir = "";

  } else
    push_cwd(cwd_buf, &symhold);

  if (pr_fsio_chdir_canon(dir, !opt_L && list_show_symlinks)) {
    destroy_pool(workp);
    return 0;
  }

  if ((list = sreaddir(".", FALSE)) == NULL) {
    if (!curdir)
      pop_cwd(cwd_buf, &symhold);
    destroy_pool(workp);
    return 0;
  }

  /* Search for relevant <Limit>'s to this NLST command.  If found,
   * check to see whether hidden files should be ignored.
   */
  if ((c = _find_ls_limit(cmd->argv[0])) != NULL) {
    unsigned char *ignore = get_param_ptr(c->subset, "IgnoreHidden", FALSE);

    if (ignore && *ignore == TRUE)
      ignore_hidden = TRUE;
  }

  j = 0;
  while (list[j] && count >= 0) {
    p = list[j++];

    if (*p == '.') {
      if (!opt_a && (!opt_A || is_dotdir(p)))
        continue;

      /* Make sure IgnoreHidden is properly honored. */
      else if (ignore_hidden)
        continue;
    }

    if ((i = pr_fsio_readlink(p, file, sizeof(file))) > 0) {
      file[i] = '\0';
      f = file;

    } else {
      f = p;
    }

    if (ls_perms(workp, cmd, f, &hidden)) {
      if (hidden)
        continue;

      /* If the data connection isn't open, open it now. */
      if ((session.sf_flags & SF_XFER) == 0) {
        if (pr_data_open(NULL, "file list", PR_NETIO_IO_WR, 0) < 0) {
          pr_data_reset();
          count = -1;
          continue;
        }

        session.sf_flags |= SF_ASCII_OVERRIDE;
      }

      if ((mode = file_mode(f)) == 0)
        continue;

      if (!curdir) {
        if (sendline("%s/%s\n", dir, p) < 0 || sendline(NULL) < 0)
          count = -1;

        else {
          count++;

          if (list_nfiles.curr && list_nfiles.max &&
              list_nfiles.curr >= list_nfiles.max) {

            if (!list_nfiles.logged) {
              pr_log_debug(DEBUG8, "ListOptions maxfiles (%u) reached",
                list_nfiles.max);
              list_nfiles.logged = TRUE;
            }

            break;
          }
          list_nfiles.curr++;
        }

      } else {
        if (sendline("%s\n", p) < 0 || sendline(NULL) < 0)
          count = -1;

        else {
          count++;

          if (list_nfiles.curr && list_nfiles.max &&
              list_nfiles.curr >= list_nfiles.max) {

            if (!list_nfiles.logged) {
              pr_log_debug(DEBUG8, "ListOptions maxfiles (%u) reached",
                list_nfiles.max);
              list_nfiles.logged = TRUE;
            }

            break;
          }
          list_nfiles.curr++;
        }
      }
    }
  }

  if (!curdir)
    pop_cwd(cwd_buf, &symhold);

  destroy_pool(workp);

  /* Explicitly free the memory allocated for containing the list of
   * filenames.
   */
  i = 0;
  while (list[i] != NULL)
    free(list[i++]);
  free(list);

  return count;
}

/* The LIST command.  */
MODRET genericlist(cmd_rec *cmd) {
  int res = 0;
  unsigned char *tmp = NULL;
  mode_t *fake_mode = NULL;
  config_rec *c = NULL;

  tmp = get_param_ptr(TOPLEVEL_CONF, "ShowSymlinks", FALSE);
  if (tmp != NULL)
    list_show_symlinks = *tmp;

  list_strict_opts = FALSE;

  list_nfiles.max = list_ndirs.max = list_ndepth.max = 0;

  c = find_config(CURRENT_CONF, CONF_PARAM, "ListOptions", FALSE);
  if (c != NULL) {
    list_options = c->argv[0];
    list_strict_opts = *((unsigned char *) c->argv[1]);

    list_ndepth.max = *((unsigned char *) c->argv[2]);

    /* We add one to the configured maxdepth in order to allow it to
     * function properly: if one configures a maxdepth of 2, one should
     * allowed to list the current directory, and all subdirectories one
     * layer deeper.  For the checks to work, the maxdepth of 2 needs to
     * handled internally as a maxdepth of 3.
     */
    if (list_ndepth.max)
      list_ndepth.max += 1;

    list_nfiles.max = *((unsigned char *) c->argv[3]);
    list_ndirs.max = *((unsigned char *) c->argv[4]);
  }

  fakeuser = get_param_ptr(CURRENT_CONF, "DirFakeUser", FALSE);

  /* Check for a configured "logged in user" DirFakeUser. */
  if (fakeuser && strcmp(fakeuser, "~") == 0)
    fakeuser = session.user;

  fakegroup = get_param_ptr(CURRENT_CONF, "DirFakeGroup", FALSE);

  /* Check for a configured "logged in user" DirFakeGroup. */
  if (fakegroup && strcmp(fakegroup, "~") == 0)
    fakegroup = session.group;

  if ((fake_mode = get_param_ptr(CURRENT_CONF, "DirFakeMode", FALSE))) {
    fakemode = *fake_mode;
    have_fake_mode = TRUE;

  } else
    have_fake_mode = FALSE;

  tmp = get_param_ptr(TOPLEVEL_CONF, "TimesGMT", FALSE);
  if (tmp != NULL)
    list_times_gmt = *tmp;

  res = dolist(cmd, cmd->arg, TRUE);

  if (XFER_ABORTED) {
    pr_data_abort(0, 0);
    res = -1;

  } else if (session.sf_flags & SF_XFER)
    ls_done(cmd);

  opt_l = 0;

  return (res == -1 ? ERROR(cmd) : HANDLED(cmd));
}

MODRET ls_log_nlst(cmd_rec *cmd) {
  pr_data_cleanup();
  return DECLINED(cmd);
}

MODRET ls_err_nlst(cmd_rec *cmd) {
  pr_data_cleanup();
  return DECLINED(cmd);
}

MODRET ls_stat(cmd_rec *cmd) {
  char *arg = cmd->arg;
  unsigned char *tmp = NULL;
  mode_t *fake_mode = NULL;
  config_rec *c = NULL;

  if (cmd->argc == 1) {

    /* In this case, the client is requesting the current session
     * status.
     */

    if (!dir_check(cmd->tmp_pool, cmd->argv[0], cmd->group, session.cwd,
        NULL)) {
      pr_response_add_err(R_500, "%s: %s", cmd->argv[0], strerror(EPERM));
      return ERROR(cmd);
    }

    pr_response_add(R_211, "Status of '%s'", main_server->ServerName);
    pr_response_add(R_DUP, "Connected from %s (%s)", session.c->remote_name,
      pr_netaddr_get_ipstr(session.c->remote_addr));
    pr_response_add(R_DUP, "Logged in as %s", session.user);
    pr_response_add(R_DUP, "TYPE: %s, STRUcture: File, Mode: Stream",
      (session.sf_flags & SF_ASCII) ? "ASCII" : "BINARY");

    if (session.total_bytes)
      pr_response_add(R_DUP, "Total bytes transferred for session: %" PR_LU,
        (pr_off_t) session.total_bytes);

    if (session.sf_flags & SF_XFER) {

      /* Report on the data transfer attributes.
       */

      pr_response_add(R_DUP, "%s from %s port %u",
        (session.sf_flags & SF_PASSIVE) ?
          "Passive data transfer from" : "Active data transfer to",
        pr_netaddr_get_ipstr(session.d->remote_addr), session.d->remote_port);

      if (session.xfer.file_size)
        pr_response_add(R_DUP, "%s %s (%" PR_LU "/%" PR_LU ")",
          session.xfer.direction == PR_NETIO_IO_RD ? C_STOR : C_RETR,
          session.xfer.path, (pr_off_t) session.xfer.file_size,
          (pr_off_t) session.xfer.total_bytes);

      else
        pr_response_add(R_DUP, "%s %s (%" PR_LU ")",
          session.xfer.direction == PR_NETIO_IO_RD ? C_STOR : C_RETR,
          session.xfer.path, (pr_off_t) session.xfer.total_bytes);

    } else
      pr_response_add(R_DUP, "No data connection");

    pr_response_add(R_DUP, "End of status");

    return HANDLED(cmd);
  }

  list_nfiles.curr = list_ndirs.curr = list_ndepth.curr = 0;
  list_nfiles.logged = list_ndirs.logged = list_ndepth.logged = FALSE;

  /* Get to the actual argument. */
  if (*arg == '-')
    while (arg && *arg && !isspace((int) *arg)) arg++;

  while (arg && *arg && isspace((int) *arg)) arg++;

  if ((tmp = get_param_ptr(TOPLEVEL_CONF, "ShowSymlinks", FALSE)) != NULL)
    list_show_symlinks = *tmp;

  list_strict_opts = FALSE;
  list_ndepth.max = list_nfiles.max = list_ndirs.max = 0;

  c = find_config(CURRENT_CONF, CONF_PARAM, "ListOptions", FALSE);
  if (c != NULL) {
    list_options = c->argv[0];
    list_strict_opts = *((unsigned char *) c->argv[1]);

    list_ndepth.max = *((unsigned char *) c->argv[2]);

    /* We add one to the configured maxdepth in order to allow it to
     * function properly: if one configures a maxdepth of 2, one should
     * allowed to list the current directory, and all subdirectories one
     * layer deeper.  For the checks to work, the maxdepth of 2 needs to
     * handled internally as a maxdepth of 3.
     */
    if (list_ndepth.max)
      list_ndepth.max += 1;

    list_nfiles.max = *((unsigned char *) c->argv[3]);
    list_ndirs.max = *((unsigned char *) c->argv[4]);
  }

  fakeuser = get_param_ptr(CURRENT_CONF, "DirFakeUser", FALSE);

  /* Check for a configured "logged in user" DirFakeUser. */
  if (fakeuser && strcmp(fakeuser, "~") == 0)
    fakeuser = session.user;

  fakegroup = get_param_ptr(CURRENT_CONF, "DirFakeGroup", FALSE);

  /* Check for a configured "logged in user" DirFakeGroup. */
  if (fakegroup && strcmp(fakegroup, "~") == 0)
    fakegroup = session.group;

  if ((fake_mode = get_param_ptr(CURRENT_CONF, "DirFakeMode", FALSE))) {
    fakemode = *fake_mode;
    have_fake_mode = TRUE;

  } else
    have_fake_mode = FALSE;

  if ((tmp = get_param_ptr(TOPLEVEL_CONF, "TimesGMT", FALSE)) != NULL)
    list_times_gmt = *tmp;

  opt_C = opt_d = opt_F = opt_R = 0;
  opt_a = opt_l = opt_STAT = 1;

  pr_response_add(R_211, "Status of %s:", arg && *arg ? arg : ".");
  dolist(cmd, cmd->arg, FALSE);
  pr_response_add(R_211, "End of Status");
  return HANDLED(cmd);
}

MODRET ls_list(cmd_rec *cmd) {
  list_nfiles.curr = list_ndirs.curr = list_ndepth.curr = 0;
  list_nfiles.logged = list_ndirs.logged = list_ndepth.logged = FALSE;

  opt_l = 1;
  return genericlist(cmd);
}

/* NLST is a very simplistic directory listing, unlike LIST (which
 * emulates ls), it only sends a list of all files/directories
 * matching the glob(s).
 */

MODRET ls_nlst(cmd_rec *cmd) {
  char *target, buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  config_rec *c = NULL;
  int count = 0, res = 0, hidden = 0;
  int glob_flags = GLOB_PERIOD;
  unsigned char *tmp = NULL;

  list_nfiles.curr = list_ndirs.curr = list_ndepth.curr = 0;
  list_nfiles.logged = list_ndirs.logged = list_ndepth.logged = FALSE;

  tmp = get_param_ptr(TOPLEVEL_CONF, "ShowSymlinks", FALSE);
  if (tmp != NULL)
    list_show_symlinks = *tmp;

  target = cmd->argc == 1 ? "." : cmd->arg;

  c = find_config(CURRENT_CONF, CONF_PARAM, "ListOptions", FALSE);
  if (c != NULL) {
    list_options = c->argv[0];
    list_strict_opts = *((unsigned char *) c->argv[1]);

    list_ndepth.max = *((unsigned char *) c->argv[2]);

    /* We add one to the configured maxdepth in order to allow it to
     * function properly: if one configures a maxdepth of 2, one should
     * allowed to list the current directory, and all subdirectories one
     * layer deeper.  For the checks to work, the maxdepth of 2 needs to
     * handled internally as a maxdepth of 3.
     */
    if (list_ndepth.max)
      list_ndepth.max += 1;

    list_nfiles.max = *((unsigned char *) c->argv[3]);
    list_ndirs.max = *((unsigned char *) c->argv[4]);
  }

  /* Clear the listing option flags. */
  opt_a = opt_C = opt_d = opt_F = opt_n = opt_r = opt_R = opt_S = opt_t =
    opt_STAT = opt_L = 0;

  if (!list_strict_opts) {
    parse_list_opts(&target, &glob_flags, FALSE);

  } else {

    /* Even if the user-given options are ignored, they still need to
     * "processed" (ie skip past options) in order to get to the paths.
     */
    while (*target && isspace((int) *target))
      target++;

    while (target && *target == '-') {

      /* Advance to the next whitespace */
      while (*target != '\0' && !isspace((int) *target))
        target++;

      while (isspace((int) *target))
        target++;
    }

    while (isspace((int) *target))
      target++;
  }

  if (list_options)
    parse_list_opts(&list_options, &glob_flags, TRUE);

  /* If the target starts with '~' ... */
  if (*target == '~') {
    char pb[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
    struct passwd *pw = NULL;
    int i = 0;
    const char *p = target;

    p++;

    while (*p && *p != '/' && i < PR_TUNABLE_PATH_MAX)
      pb[i++] = *p++;
    pb[i] = '\0';

    pw = pr_auth_getpwnam(cmd->tmp_pool, i ? pb : session.user);
    if (pw) {
      snprintf(pb, sizeof(pb), "%s%s", pw->pw_dir, p);
      sstrncpy(buf, pb, sizeof(buf));
      target = buf;
    }
  }

  /* Clean the path. */
  if (*target != '/') {
    size_t cwdlen = strlen(pr_fs_getcwd());

    pr_fs_clean_path(pdircat(cmd->tmp_pool, pr_fs_getcwd(), target, NULL),
      buf, sizeof(buf));

    target = buf;

    /* If the given target was not an absolute path, advance past the
     * current working directory prefix in the cleaned up target path.
     */
    target += cwdlen;

    /* If the length of the current working directory (cwdlen) is one,
     * it means that the current working directory is the root ('/'),
     * and so we don't want to advance past that into the file name
     * portion of the path.
     */
    if (cwdlen > 1)
      target += 1;

  } else {
    pr_fs_clean_path(target, buf, sizeof(buf));
    target = buf;
  }

  /* Remove any trailing separators. */
  while (target[strlen(target)-1] == '/')
    target[strlen(target)-1] = '\0';

  /* If the target is a glob, get the listing of files/dirs to send. */
  if (use_globbing && strpbrk(target, "{[*?") != NULL) {
    glob_t g;
    char **path,*p;

    /* Make sure the glob_t is initialized */
    memset(&g, '\0', sizeof(glob_t));

    if (pr_fs_glob(target, GLOB_PERIOD, NULL, &g) != 0) {
      pr_response_add_err(R_450, "No files found");
      return ERROR(cmd);
    }

    /* Iterate through each matching entry */
    path = g.gl_pathv;
    while (path && *path && res >= 0) {
      struct stat st;

      p = *path;
      path++;

      if (*p == '.' && (!opt_A || is_dotdir(p)))
        continue;

      if (pr_fsio_stat(p, &st) == 0) {
        /* If it's a directory, hand off to nlstdir */
        if (S_ISDIR(st.st_mode))
          res = nlstdir(cmd, p);

        else if (S_ISREG(st.st_mode) &&
            ls_perms(cmd->tmp_pool, cmd, p, &hidden)) {
          /* Don't display hidden files */
          if (hidden)
            continue;

          res = nlstfile(cmd, p);
        }

        if (res > 0)
          count += res;
      }
    }

    pr_fs_globfree(&g);

  } else {

    /* A single target. If it's a directory, list the contents; if it's a
     * file, just list the file.
     */
    struct stat st;

    if (!ls_perms_full(cmd->tmp_pool, cmd, target, &hidden)) {
      pr_response_add_err(R_450, "%s: %s", *cmd->arg ? cmd->arg : session.vwd,
        strerror(errno));
      return ERROR(cmd);
    }

    /* Don't display hidden files */
    if (hidden) {
      c = _find_ls_limit(target);

      if (c) {
        unsigned char *ignore_hidden = get_param_ptr(c->subset,
          "IgnoreHidden", FALSE);

        if (ignore_hidden && *ignore_hidden == TRUE)
          pr_response_add_err(R_450, "%s: %s", target, strerror(ENOENT));
        else
          pr_response_add_err(R_450, "%s: %s", target, strerror(EACCES));

        return ERROR(cmd);
      }
    }

    /* Make sure the target is a file or directory, and that we have access
     * to it.
     */
    pr_fs_clear_cache();
    if (pr_fsio_stat(target, &st) < 0) {
      pr_response_add_err(R_450, "%s: %s", cmd->arg, strerror(errno));
      return ERROR(cmd);
    }

    if (S_ISREG(st.st_mode))
      res = nlstfile(cmd, target);

    else if (S_ISDIR(st.st_mode)) {
      if (pr_fsio_access(target, R_OK, session.uid, session.gid,
          session.gids) != 0) {
        pr_response_add_err(R_450, "%s: %s", cmd->arg, strerror(errno));
        return ERROR(cmd);
      }

      res = nlstdir(cmd, target);

    } else {
      pr_response_add_err(R_450, "%s: Not a regular file", cmd->arg);
      return ERROR(cmd);
    }

    if (res > 0)
      count += res;
  }

  if (XFER_ABORTED) {
    pr_data_abort(0, 0);
    res = -1;

  } else {
    /* Note that the data connection is NOT cleared here, as an error in
     * NLST still leaves data ready for another command.
     */
    ls_done(cmd);
  }

  return (res < 0 ? ERROR(cmd) : HANDLED(cmd));
}

/* Check for the UseGlobbing setting, if any, after the PASS command has
 * been successfully handled.
 */
MODRET ls_post_pass(cmd_rec *cmd) {
  unsigned char *globbing = NULL;

  if ((globbing = get_param_ptr(TOPLEVEL_CONF, "UseGlobbing",
      FALSE)) != NULL && *globbing == FALSE) {
    pr_log_debug(DEBUG3, "UseGlobbing: disabling globbing functionality");
    use_globbing = FALSE;
  }

  return DECLINED(cmd);
}

/* Configuration handlers
 */

MODRET set_dirfakeusergroup(cmd_rec *cmd) {
  int bool = -1;
  char *as = "ftp";
  config_rec *c = NULL;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_ANON|CONF_GLOBAL|
    CONF_DIR|CONF_DYNDIR);

  if (cmd->argc < 2 || cmd->argc > 3)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "syntax: ", cmd->argv[0],
      " on|off [<id to display>]", NULL));

  if ((bool = get_boolean(cmd,1)) == -1)
     CONF_ERROR(cmd, "expected boolean argument");

  if (bool == TRUE) {
    /* Use the configured ID to display rather than the default "ftp". */
    if (cmd->argc > 2)
      as = cmd->argv[2];

    c = add_config_param_str(cmd->argv[0], 1, as);

  } else {
    /* Still need to add a config_rec to turn off the display of fake IDs. */
    c = add_config_param_str(cmd->argv[0], 0);
  }

  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

MODRET set_dirfakemode(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *endp = NULL;
  mode_t fake_mode;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR|
    CONF_DYNDIR);

  fake_mode = (mode_t) strtol(cmd->argv[1], &endp, 8);

  if (endp && *endp)
    CONF_ERROR(cmd, "parameter must be an octal number");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(mode_t));
  *((mode_t *) c->argv[0]) = fake_mode;
  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

MODRET set_listoptions(cmd_rec *cmd) {
  config_rec *c = NULL;

  if (cmd->argc-1 < 1)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  c = add_config_param(cmd->argv[0], 5, NULL, NULL, NULL, NULL, NULL);
  c->flags |= CF_MERGEDOWN;
  
  c->argv[0] = pstrdup(c->pool, cmd->argv[1]);

  /* The default "strict" setting. */
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[1]) = FALSE;

  /* The default "maxdepth" setting. */
  c->argv[2] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[2]) = 0;

  /* The default "maxfiles" setting. */
  c->argv[3] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[3]) = 0;

  /* The default "maxdirs" setting. */
  c->argv[4] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[4]) = 0;

  /* Check for, and handle, optional arguments. */
  if (cmd->argc-1 >= 2) {
    register unsigned int i = 0;

    for (i = 2; i < cmd->argc; i++) {

      if (strcasecmp(cmd->argv[i], "strict") == 0) {
        *((unsigned int *) c->argv[1]) = TRUE;

      } else if (strcasecmp(cmd->argv[i], "maxdepth") == 0) {
        int maxdepth = atoi(cmd->argv[++i]);

        if (maxdepth < 1)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
            ": maxdepth must be greater than 0: '", cmd->argv[i],
            "'", NULL));

        *((unsigned int *) c->argv[2]) = maxdepth;

      } else if (strcasecmp(cmd->argv[i], "maxfiles") == 0) {
        int maxfiles = atoi(cmd->argv[++i]);

        if (maxfiles < 1)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
            ": maxfiles must be greater than 0: '", cmd->argv[i],
            "'", NULL));

          *((unsigned int *) c->argv[3]) = maxfiles;

      } else if (strcasecmp(cmd->argv[i], "maxdirs") == 0) {
        int maxdirs = atoi(cmd->argv[++i]);

        if (maxdirs < 1)
          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
            ": maxdirs must be greater than 0: '", cmd->argv[i],
            "'", NULL));

          *((unsigned int *) c->argv[4]) = maxdirs;

      } else {
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown keyword: '",
          cmd->argv[i], "'", NULL));
      }
    }
  }

  return HANDLED(cmd);
}

MODRET set_showsymlinks(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

MODRET set_useglobbing(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

/* Initialization routines
 */

static int ls_init(void) {

  /* Add the commands handled by this module to the HELP list. */
  pr_help_add(C_LIST, "[<sp> pathname]", TRUE);
  pr_help_add(C_NLST, "[<sp> (pathname)]", TRUE);
  pr_help_add(C_STAT, "[<sp> pathname]", TRUE);

  return 0;
}

/* Module API tables
 */

static conftable ls_conftab[] = {
  { "DirFakeUser",	set_dirfakeusergroup,			NULL },
  { "DirFakeGroup",	set_dirfakeusergroup,			NULL },
  { "DirFakeMode",	set_dirfakemode,			NULL },
  { "ListOptions",	set_listoptions,			NULL },
  { "ShowSymlinks",	set_showsymlinks,			NULL },
  { "UseGlobbing",	set_useglobbing,			NULL },
  { NULL,		NULL,					NULL }
};

static cmdtable ls_cmdtab[] = {
  { CMD,  	C_NLST,	G_DIRS,	ls_nlst,	TRUE, FALSE, CL_DIRS },
  { CMD,	C_LIST,	G_DIRS,	ls_list,	TRUE, FALSE, CL_DIRS },
  { CMD, 	C_STAT,	G_DIRS,	ls_stat,	TRUE, FALSE, CL_INFO },
  { POST_CMD,	C_PASS,	G_NONE,	ls_post_pass,	FALSE, FALSE },
  { LOG_CMD,	C_LIST,	G_NONE,	ls_log_nlst,	FALSE, FALSE },
  { LOG_CMD,	C_NLST, G_NONE,	ls_log_nlst,	FALSE, FALSE },
  { LOG_CMD_ERR,C_LIST, G_NONE, ls_err_nlst,   FALSE, FALSE },
  { LOG_CMD_ERR,C_NLST, G_NONE, ls_err_nlst,   FALSE, FALSE },
  { 0, NULL }
};

module ls_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "ls",

  /* Module configuration handler table */
  ls_conftab,

  /* Module command handler table */
  ls_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization */
  ls_init,

  /* Session initialization */
  NULL
};

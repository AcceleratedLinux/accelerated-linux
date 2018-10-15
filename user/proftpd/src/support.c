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

/* Various basic support routines for ProFTPD, used by all modules
 * and not specific to one or another.
 *
 * $Id: support.c,v 1.77 2005/03/18 18:25:55 castaglia Exp $
 */

#include "conf.h"

#include <signal.h>

#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#elif defined(HAVE_SYS_VFS_H)
# include <sys/vfs.h>
#elif defined(HAVE_SYS_MOUNT_H)
# include <sys/mount.h>
#endif

#ifdef AIX3
# include <sys/statfs.h>
#endif

typedef struct sched_obj {
  struct sched_obj *next, *prev;

  pool *pool;
  void (*f)(void*,void*,void*,void*);
  int loops;
  void *a1,*a2,*a3,*a4;
} sched_t;

static xaset_t *scheds = NULL;

/* Masks/unmasks all important signals (as opposed to blocking alarms)
 */
static void mask_signals(unsigned char block) {
  static sigset_t mask_sigset;

  if (block) {
    sigemptyset(&mask_sigset);

    sigaddset(&mask_sigset, SIGTERM);
    sigaddset(&mask_sigset, SIGCHLD);
    sigaddset(&mask_sigset, SIGUSR1);
    sigaddset(&mask_sigset, SIGINT);
    sigaddset(&mask_sigset, SIGQUIT);
    sigaddset(&mask_sigset, SIGALRM);
#ifdef SIGIO
    sigaddset(&mask_sigset, SIGIO);
#endif
#ifdef SIGBUS
    sigaddset(&mask_sigset, SIGBUS);
#endif
    sigaddset(&mask_sigset, SIGHUP);

    sigprocmask(SIG_BLOCK, &mask_sigset, NULL);

  } else
    sigprocmask(SIG_UNBLOCK, &mask_sigset, NULL);
}

void pr_signals_block(void) {
  mask_signals(TRUE);
}

void pr_signals_unblock(void) {
  mask_signals(FALSE);
}

void schedule(void (*f)(void*,void*,void*,void*),int nloops, void *a1,
    void *a2, void *a3, void *a4) {
  pool *p, *sub_pool;
  sched_t *s;

  if (!scheds) {
   p = make_sub_pool(permanent_pool);
   pr_pool_tag(p, "Schedules Pool");
   scheds = xaset_create(p, NULL);

  } else
   p = scheds->pool;

  sub_pool = make_sub_pool(p);

  s = pcalloc(sub_pool, sizeof(sched_t));
  s->pool = sub_pool;
  s->f = f;
  s->a1 = a1;
  s->a2 = a2;
  s->a3 = a3;
  s->a4 = a4;
  s->loops = nloops;
  xaset_insert(scheds, (xasetmember_t*)s);
}

void run_schedule(void) {
  sched_t *s,*snext;

  if (!scheds || !scheds->xas_list)
    return;

  for (s = (sched_t*)scheds->xas_list; s; s=snext) {
    snext = s->next;

    if (s->loops-- <= 0) {
      s->f(s->a1,s->a2,s->a3,s->a4);
      xaset_remove(scheds, (xasetmember_t*)s);
      destroy_pool(s->pool);
    }
  }
}

/* Get the maximum size of a file name (pathname component).
 * If a directory file descriptor, e.g. the d_fd DIR structure element,
 * is not available, the second argument should be 0.
 *
 * Note: a POSIX compliant system typically should NOT define NAME_MAX,
 * since the value almost certainly varies across different file system types.
 * Refer to POSIX 1003.1a, Section 2.9.5, Table 2-5.
 * Alas, current (Jul 2000) Linux systems define NAME_MAX anyway.
 * NB: NAME_MAX_GUESS is defined in support.h.
 */
int get_name_max(char *dirname, int dir_fd) {
  int name_max = 0;
#if defined(HAVE_FPATHCONF) || defined(HAVE_PATHCONF)
  char *msgfmt = "";

# if defined(HAVE_FPATHCONF)
  if (dir_fd > 0) {
    name_max = fpathconf(dir_fd, _PC_NAME_MAX);
    msgfmt = "fpathconf(%s, _PC_NAME_MAX) = %d, errno = %d";
  } else
# endif
# if defined(HAVE_PATHCONF)
  if (dirname != NULL) {
    name_max = pathconf(dirname, _PC_NAME_MAX);
    msgfmt = "pathconf(%s, _PC_NAME_MAX) = %d, errno = %d";
  } else
# endif
  /* No data provided to use either pathconf() or fpathconf() */
  return -1;

  if (name_max < 0) {
    /* NB: errno may not be set if the failure is due to a limit or option
     * not being supported.
     */
    pr_log_debug(DEBUG1, msgfmt, dirname ? dirname : "(NULL)", name_max, errno);
  }

#else
  name_max = NAME_MAX_GUESS;
#endif /* HAVE_FPATHCONF or HAVE_PATHCONF */

  return name_max;
}


/* Interpolates a pathname, expanding ~ notation if necessary
 */
char *dir_interpolate(pool *p, const char *path) {
  struct passwd *pw;
  char *user,*tmp;
  char *ret = (char *)path;

  if (!ret)
    return NULL;

  if (*ret == '~') {
    user = pstrdup(p, ret+1);
    tmp = strchr(user, '/');

    if (tmp)
      *tmp++ = '\0';

    if (!*user)
      user = session.user;

    pw = pr_auth_getpwnam(p, user);

    if (!pw) {
      errno = ENOENT;
      return NULL;
    }

    ret = pdircat(p, pw->pw_dir, tmp, NULL);
  }

  return ret;
}

/* dir_best_path() creates the "most" fully canonicalized path possible
 * (i.e. if path components at the end don't exist, they are ignored
 */
char *dir_best_path(pool *p, const char *path) {
  char workpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  char realpath_buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  char *target = NULL, *ntarget;
  int fini = 0;

  if (*path == '~') {
    if (pr_fs_interpolate(path, workpath, sizeof(workpath)-1) != 1) {
      if (pr_fs_dircat(workpath, sizeof(workpath), pr_fs_getcwd(), path) < 0)
        return NULL;
    }

  } else {
    if (pr_fs_dircat(workpath, sizeof(workpath), pr_fs_getcwd(), path) < 0)
      return NULL;
  }

  pr_fs_clean_path(pstrdup(p, workpath), workpath, sizeof(workpath)-1);

  while (!fini && *workpath) {
    if (pr_fs_resolve_path(workpath, realpath_buf,
        sizeof(realpath_buf)-1, 0) != -1)
      break;

    ntarget = strrchr(workpath, '/');
    if (ntarget) {
      if (target) {
        if (pr_fs_dircat(workpath, sizeof(workpath), workpath, target) < 0)
          return NULL;
      }

      target = ntarget;
      *target++ = '\0';

    } else
      fini++;
  }

  if (!fini && *workpath) {
    if (target) {
      if (pr_fs_dircat(workpath, sizeof(workpath), realpath_buf, target) < 0)
        return NULL;

    } else
      sstrncpy(workpath, realpath_buf, sizeof(workpath));

  } else {
    if (pr_fs_dircat(workpath, sizeof(workpath), "/", target) < 0)
      return NULL;
  }

  return pstrdup(p, workpath);
}

char *dir_canonical_path(pool *p, const char *path) {
  char buf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'};
  char work[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

  if (*path == '~') {
    if (pr_fs_interpolate(path, work, sizeof(work)-1) != 1) {
      if (pr_fs_dircat(work, sizeof(work), pr_fs_getcwd(), path) < 0)
        return NULL;
    }

  } else {
    if (pr_fs_dircat(work, sizeof(work), pr_fs_getcwd(), path) < 0)
      return NULL;
  }

  pr_fs_clean_path(work, buf, sizeof(buf)-1);
  return pstrdup(p, buf);
}

char *dir_canonical_vpath(pool *p, const char *path) {
  char buf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'};
  char work[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

  if (*path == '~') {
    if (pr_fs_interpolate(path, work, sizeof(work)-1) != 1) {
      if (pr_fs_dircat(work, sizeof(work), pr_fs_getvwd(), path) < 0)
        return NULL;
    }

  } else {
    if (pr_fs_dircat(work, sizeof(work), pr_fs_getvwd(), path) < 0)
      return NULL;
  }

  pr_fs_clean_path(work, buf, sizeof(buf)-1);
  return pstrdup(p, buf);
}

/* dir_realpath() is needed to properly dereference symlinks (getcwd() may
 * not work if permissions cause problems somewhere up the tree).
 */
char *dir_realpath(pool *p, const char *path) {
  char buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

  if (pr_fs_resolve_partial(path, buf, sizeof(buf)-1, 0) == -1)
    return NULL;

  return pstrdup(p, buf);
}

/* Takes a directory and returns it's absolute version.  ~username
 * references are appropriately interpolated.  "Absolute" includes
 * a *full* reference based on the root directory, not upon a chrooted
 * dir.
 */
char *dir_abs_path(pool *p, const char *path, int interpolate) {
  char *res = NULL;

  if (interpolate)
    path = dir_interpolate(p, path);

  if (!path)
    return NULL;

  if (*path != '/') {
    if (session.chroot_path)
      res = pdircat(p, session.chroot_path, pr_fs_getcwd(), path, NULL);
    else
      res = pdircat(p, pr_fs_getcwd(), path, NULL);

  } else if (session.chroot_path)
    res = pdircat(p, session.chroot_path, path, NULL);

  else
    res = pstrdup(p, path);

  return res;
}

/* Return the mode (including the file type) of the file pointed to by symlink
 * PATH, or 0 if it doesn't exist. Catch symlink loops using LAST_INODE and
 * RCOUNT.
 */
static mode_t _symlink(char *path, ino_t last_inode, int rcount) {
  char buf[PR_TUNABLE_PATH_MAX + 1];
  struct stat sbuf;
  int i;

  if (++rcount >= 32) {
    errno = ELOOP;
    return 0;
  }

  memset(buf, '\0', sizeof(buf));

  i = pr_fsio_readlink(path, buf, sizeof(buf) - 1);
  if (i == -1)
    return (mode_t)0;
  buf[i] = '\0';

  if (pr_fsio_lstat(buf, &sbuf) != -1) {
    if (sbuf.st_ino && (ino_t) sbuf.st_ino == last_inode) {
      errno = ELOOP;
      return 0;
    }

    if (S_ISLNK(sbuf.st_mode))
      return _symlink(buf, (ino_t) sbuf.st_ino, rcount);
    return sbuf.st_mode;
  }

  return 0;
}

mode_t file_mode(char *path) {
  struct stat sbuf;
  mode_t res = 0;

  pr_fs_clear_cache();
  if (pr_fsio_lstat(path, &sbuf) != -1) {
    if (S_ISLNK(sbuf.st_mode)) {
      res = _symlink(path, (ino_t) 0, 0);

      if (res == 0)
	/* a dangling symlink, but it exists to rename or delete. */
	res = sbuf.st_mode;

    } else
      res = sbuf.st_mode;
  }

  return res;
}

/* If DIRP == 1, fail unless PATH is an existing directory.
 * If DIRP == 0, fail unless PATH is an existing non-directory.
 * If DIRP == -1, fail unless PATH exists; the caller doesn't care whether
 * PATH is a file or a directory.
 */
static int _exists(char *path, int dirp) {
  mode_t fmode;

  if ((fmode = file_mode(path)) != 0) {
    if (dirp == 1 && !S_ISDIR(fmode))
      return FALSE;

    else if (dirp == 0 && S_ISDIR(fmode))
      return FALSE;

    return TRUE;
  }

  return FALSE;
}

int file_exists(char *path) {
  return _exists(path, 0);
}

int dir_exists(char *path) {
  return _exists(path, 1);
}

int exists(char *path) {
  return _exists(path, -1);
}

char *pr_str_strip(pool *p, char *str) {
  char c, *dupstr, *start, *finish;

  if (!p || !str) {
    errno = EINVAL;
    return NULL;
  }

  /* First, find the non-whitespace start of the given string */
  for (start = str; isspace((int) *start); start++);

  /* Now, find the non-whitespace end of the given string */
  for (finish = &str[strlen(str)-1]; isspace((int) *finish); finish--);

  /* finish is now pointing to a non-whitespace character.  So advance one
   * character forward, and set that to NUL.
   */
  c = *++finish;
  *finish = '\0';

  /* The space-stripped string is, then, everything from start to finish. */
  dupstr = pstrdup(p, start);
 
  /* Restore the given string buffer contents. */
  *finish = c;

  return dupstr;
}

char *strip_end(char *s, char *ch) {
  int i = strlen(s);

  while (i && strchr(ch,*(s+i-1))) {
    *(s+i-1) = '\0';
    i--;
  }

  return s;
}

/* get_token tokenizes a string, increments the src pointer to
 * the next non-separator in the string.  If the src string is
 * empty or NULL, the next token returned is NULL.
 */
char *get_token(char **s, char *sep) {
  char *res;

  if (!s || !*s || !**s)
    return NULL;

  res = *s;

  while (**s && !strchr(sep,**s))
    (*s)++;

  if (**s)
    *(*s)++ = '\0';

  return res;
}

/* safe_token tokenizes a string, and increments the pointer to
 * the next non-white space character.  It's "safe" because it
 * never returns NULL, only an empty string if no token remains
 * in the source string.
 */
char *safe_token(char **s) {
  char *res = "";

  if (!s || !*s)
    return res;

  while (isspace((int) **s) && **s)
    (*s)++;

  if (**s) {
    res = *s;

    while (!isspace((int) **s) && **s)
      (*s)++;

    if (**s)
      *(*s)++ = '\0';

    while (isspace((int) **s) && **s)
      (*s)++;
  }

  return res;
}

/* Checks for the existence of PR_SHUTMSG_PATH.  deny and disc are
 * filled with the times to deny new connections and disconnect
 * existing ones.
 */
int check_shutmsg(time_t *shut, time_t *deny, time_t *disc, char *msg,
                  size_t msg_size) {
  FILE *fp;
  char *deny_str,*disc_str,*cp, buf[PR_TUNABLE_BUFFER_SIZE+1] = {'\0'};
  char hr[3] = {'\0'}, mn[3] = {'\0'};
  time_t now,shuttime = (time_t)0;
  struct tm tm;

  if (file_exists(PR_SHUTMSG_PATH) && (fp = fopen(PR_SHUTMSG_PATH, "r"))) {
    if ((cp = fgets(buf, sizeof(buf),fp)) != NULL) {
      buf[sizeof(buf)-1] = '\0'; CHOP(cp);

      /* We use this to fill in dst, timezone, etc */
      time(&now);
      tm = *(localtime(&now));

      tm.tm_year = atoi(safe_token(&cp)) - 1900;
      tm.tm_mon = atoi(safe_token(&cp)) - 1;
      tm.tm_mday = atoi(safe_token(&cp));
      tm.tm_hour = atoi(safe_token(&cp));
      tm.tm_min = atoi(safe_token(&cp));
      tm.tm_sec = atoi(safe_token(&cp));

      deny_str = safe_token(&cp);
      disc_str = safe_token(&cp);

      if ((shuttime = mktime(&tm)) == (time_t) - 1) {
        fclose(fp);
        return 0;
      }

      if (deny) {
        if (strlen(deny_str) == 4) {
          sstrncpy(hr,deny_str,sizeof(hr)); hr[2] = '\0'; deny_str += 2;
          sstrncpy(mn,deny_str,sizeof(mn)); mn[2] = '\0';

          *deny = shuttime - ((atoi(hr) * 3600) + (atoi(mn) * 60));
        } else
          *deny = shuttime;
      }

      if (disc) {
        if (strlen(disc_str) == 4) {
          sstrncpy(hr,disc_str,sizeof(hr)); hr[2] = '\0'; disc_str += 2;
          sstrncpy(mn,disc_str,sizeof(mn)); mn[2] = '\0';

          *disc = shuttime - ((atoi(hr) * 3600) + (atoi(mn) * 60));
        } else
          *disc = shuttime;
      }

      if (fgets(buf, sizeof(buf),fp) && msg) {
        buf[sizeof(buf)-1] = '\0';
	CHOP(buf);
        sstrncpy(msg, buf, msg_size-1);
      }
    }

    fclose(fp);
    if (shut)
      *shut = shuttime;
    return 1;
  }

  return 0;
}

/* Make sure we don't display any sensitive information via argstr. Note:
 * make this a separate function in the future (get_full_cmd() or somesuch),
 * and have that function deal with creating a displayable string.  Once
 * RFC2228 support is added, PASS won't be the only command whose parameters
 * should not be displayed.
 */
char *make_arg_str(pool *p, int argc, char **argv) {
  char *res = "";

  /* Check for "sensitive" commands. */
  if (!strcmp(argv[0], C_PASS) ||
      !strcmp(argv[0], C_ADAT)) {
    argc = 2;
    argv[1] = "(hidden)";
  }

  while (argc--) {
    if (*res)
      res = pstrcat(p, res, " ", *argv++, NULL);
    else
      res = pstrcat(p, res, *argv++, NULL);
  }

  return res;
}

char *sreplace(pool *p, char *s, ...) {
  va_list args;
  char *m,*r,*src = s,*cp;
  char **mptr,**rptr;
  char *marr[33],*rarr[33];
  char buf[PR_TUNABLE_PATH_MAX] = {'\0'}, *pbuf = NULL;
  size_t mlen = 0, rlen = 0, blen;
  int dyn = TRUE;

  cp = buf;
  *cp = '\0';

  memset(marr, '\0', sizeof(marr));
  memset(rarr, '\0', sizeof(rarr));
  blen = strlen(src) + 1;

  va_start(args, s);

  while ((m = va_arg(args, char *)) != NULL && mlen < sizeof(marr)-1) {
    char *tmp = NULL;
    size_t count = 0;

    if ((r = va_arg(args, char *)) == NULL)
      break;

    /* Increase the length of the needed buffer by the difference between
     * the given match and replacement strings, multiplied by the number
     * of times the match string occurs in the source string.
     */
    tmp = strstr(s, m);
    while (tmp) {
      pr_signals_handle();
      count++;

      /* Be sure to increment the pointer returned by strstr(3), to
       * advance past the beginning of the substring for which we are
       * looking.  Otherwise, we just loop endlessly, seeing the same
       * value for tmp over and over.
       */
      tmp += strlen(m);
      tmp = strstr(tmp, m);
    }

    /* We are only concerned about match/replacement strings that actually
     * occur in the given string.
     */
    if (count) {
      blen += count * (strlen(r) - strlen(m));
      marr[mlen] = m;
      rarr[mlen++] = r;
    }
  }

  va_end(args);

  /* Try to handle large buffer situations (i.e. escaping of PR_TUNABLE_PATH_MAX
   * (>2048) correctly, but do not allow very big buffer sizes, that may
   * be dangerous (BUFSIZ may be defined in stdio.h) in some library
   * functions.
   */
#ifndef BUFSIZ
# define BUFSIZ 8192
#endif

  if (blen < BUFSIZ)
    cp = pbuf = (char *) pcalloc(p, ++blen);

  if (!pbuf) {
    cp = pbuf = buf;
    dyn = FALSE;
    blen = sizeof(buf);
  }

  while (*src) {
    for (mptr = marr, rptr = rarr; *mptr; mptr++, rptr++) {
      mlen = strlen(*mptr);
      rlen = strlen(*rptr);

      if (strncmp(src, *mptr, mlen) == 0) {
        sstrncpy(cp, *rptr, blen - strlen(pbuf));
	if (((cp + rlen) - pbuf + 1) > blen) {
	  pr_log_pri(PR_LOG_ERR,
		  "WARNING: attempt to overflow internal ProFTPD buffers");
	  cp = pbuf + blen - 1;
	  goto done;

	} else {
	  cp += rlen;
	}
	
        src += mlen;
        break;
      }
    }

    if (!*mptr) {
      if ((cp - pbuf + 1) > blen) {
	pr_log_pri(PR_LOG_ERR,
		"WARNING: attempt to overflow internal ProFTPD buffers");
	cp = pbuf + blen - 1;
      }
      *cp++ = *src++;
    }
  }

 done:
  *cp = '\0';

  if (dyn)
    return pbuf;

  return pstrdup(p, buf);
}

/* "safe" memset() (code borrowed from OpenSSL).  This function should be
 * used to clear/scrub sensitive memory areas instead of memset() for the
 * reasons mentioned in this BugTraq thread:
 *
 *  http://online.securityfocus.com/archive/1/298598
 */

unsigned char memscrub_ctr = 0;

void pr_memscrub(void *ptr, size_t ptrlen) {
  unsigned char *p = ptr;
  size_t loop = ptrlen;

  while (loop--) {
    *(p++) = memscrub_ctr++;
    memscrub_ctr += (17 + (unsigned char)((intptr_t) p & 0xF));
  }

  if (memchr(ptr, memscrub_ctr, ptrlen))
    memscrub_ctr += 63;
}

/* "safe" strcat, saves room for \0 at end of dest, and refuses to copy
 * more than "n" bytes.
 */
char *sstrcat(char *dest, const char *src, size_t n) {
  register char *d;

  for (d = dest; *d && n > 1; d++, n--) ;

  while (n-- > 1 && *src)
    *d++ = *src++;

  *d = 0;
  return dest;
}

struct tm *pr_gmtime(pool *p, const time_t *t) {
  struct tm *sys_tm, *dup_tm;

  sys_tm = gmtime(t);
  dup_tm = pcalloc(p, sizeof(struct tm));
  memcpy(dup_tm, sys_tm, sizeof(struct tm));

  return dup_tm;
}

struct tm *pr_localtime(pool *p, const time_t *t) {
  struct tm *sys_tm, *dup_tm;

  sys_tm = localtime(t);
  dup_tm = pcalloc(p, sizeof(struct tm));
  memcpy(dup_tm, sys_tm, sizeof(struct tm));

  return dup_tm;
}

const char *pr_strtime(time_t t) {
  static char buf[30];
  static char *mons[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec" };
  static char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  struct tm *tr;

  memset(buf, '\0', sizeof(buf));

  tr = localtime(&t);
  if (tr != NULL) {
    snprintf(buf, sizeof(buf), "%s %s %2d %02d:%02d:%02d %d",
      days[tr->tm_wday], mons[tr->tm_mon], tr->tm_mday, tr->tm_hour,
      tr->tm_min, tr->tm_sec, tr->tm_year + 1900);

  } else
    buf[0] = '\0';

  buf[sizeof(buf)-1] = '\0';

  return buf;
}


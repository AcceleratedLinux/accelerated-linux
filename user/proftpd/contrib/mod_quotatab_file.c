/*
 * ProFTPD: mod_quotatab_file -- a mod_quotatab sub-module for managing quota
 *                          data via file-based tables
 *
 * Copyright (c) 2002-2003 TJ Saunders
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
 * As a special exemption, TJ Saunders gives permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * $Id: mod_quotatab_file.c,v 1.2 2004/12/16 22:55:46 castaglia Exp $
 */

#include "mod_quotatab.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

/* bloody lack of consistency... */
#if defined(FREEBSD4)
#  define QUOTATAB_IOV_BASE_TYPE        (char *)
#elif defined(LINUX)
#  define QUOTATAB_IOV_BASE_TYPE        (__ptr_t)
#elif defined(SOLARIS2)
#  define QUOTATAB_IOV_BASE_TYPE        (caddr_t)
#else
#  define QUOTATAB_IOV_BASE_TYPE        (void *)
#endif

module quotatab_file_module;

static int filetab_close(quota_table_t *filetab) {
  int res = close(filetab->tab_handle);
  filetab->tab_handle = -1;
  return res;
}

static int filetab_create(quota_table_t *filetab) {
  int res = -1;
  struct iovec quotav[8];
  off_t current_pos = 0;

  /* Use writev() to make this more efficient.  It is done piecewise, rather
   * than doing a normal write(2) directly from the struct pointer, to avoid
   * alignment/padding issues.
   */

  quotav[0].iov_base = QUOTATAB_IOV_BASE_TYPE quotatab_tally.name;
  quotav[0].iov_len = sizeof(quotatab_tally.name);

  quotav[1].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.quota_type;
  quotav[1].iov_len = sizeof(quotatab_tally.quota_type);

  quotav[2].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_in_used;
  quotav[2].iov_len = sizeof(quotatab_tally.bytes_in_used);

  quotav[3].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_out_used;
  quotav[3].iov_len = sizeof(quotatab_tally.bytes_out_used);

  quotav[4].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_xfer_used;
  quotav[4].iov_len = sizeof(quotatab_tally.bytes_xfer_used);

  quotav[5].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_in_used;
  quotav[5].iov_len = sizeof(quotatab_tally.files_in_used);

  quotav[6].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_out_used;
  quotav[6].iov_len = sizeof(quotatab_tally.files_out_used);

  quotav[7].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_xfer_used;
  quotav[7].iov_len = sizeof(quotatab_tally.files_xfer_used);

  /* Seek to the end of the table */
  current_pos = lseek(filetab->tab_handle, 0, SEEK_END);

  if ((res = writev(filetab->tab_handle, quotav, 8)) > 0)

    /* Rewind to the start of the entry. */
    lseek(filetab->tab_handle, current_pos, SEEK_SET);

  else if (res == 0)
    /* If no bytes were written, it's an error. */
    res = -1;

  return res;
}

static unsigned char filetab_lookup(quota_table_t *filetab, const char *name,
    quota_type_t quota_type) {

  /* Make sure the table pointer is positioned at the start of the table,
   * skipping the magic header value of the table.
   */
  lseek(filetab->tab_handle, sizeof(unsigned int), SEEK_SET);

  /* Handle tally and limit tables differently... */

  if (filetab->tab_type == TYPE_TALLY) {
    while (filetab->tab_read(filetab) >= 0) {

      /* Compare quota types.  If the quota type is ALL_QUOTA, don't
       * worry about the name.
       */
      if (quota_type == quotatab_tally.quota_type) {

        /* Match names if need be */
        if (name && !strcmp(name, quotatab_tally.name))
          return TRUE;

        if (quota_type == ALL_QUOTA)
          return TRUE;
      }

      /* Undo the auto-rewind of a single record's length done by
       * filetab_read(), so that the while loop actually does iterate through
       * all the available records.
       */
      lseek(filetab->tab_handle, filetab->tab_quotalen, SEEK_CUR);
    }

  } else if (filetab->tab_type == TYPE_LIMIT) {
    while (filetab->tab_read(filetab) >= 0) {
      if (quota_type == quotatab_limit.quota_type) {
        if (name && !strcmp(name, quotatab_limit.name))
          return TRUE;

        if (quota_type == ALL_QUOTA)
          return TRUE;
      }

      lseek(filetab->tab_handle, filetab->tab_quotalen, SEEK_CUR);
    }
  }

  /* Default return value */
  return FALSE;
}

static int filetab_read(quota_table_t *filetab) {
  int res = -1;
  struct iovec quotav[10];

  /* Mark the current file position. */
  off_t current_pos = lseek(filetab->tab_handle, 0, SEEK_CUR);

  /* Use readv() to make this more efficient.  It is done piecewise, rather
   * than doing a normal read(2) directly into the struct pointer, to avoid
   * alignment/padding issues.
   */

  /* Handle the limit and tally tables differently. */

  if (filetab->tab_type == TYPE_TALLY) {

    quotav[0].iov_base = QUOTATAB_IOV_BASE_TYPE quotatab_tally.name;
    quotav[0].iov_len = sizeof(quotatab_tally.name);

    quotav[1].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.quota_type;
    quotav[1].iov_len = sizeof(quotatab_tally.quota_type);

    quotav[2].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_in_used;
    quotav[2].iov_len = sizeof(quotatab_tally.bytes_in_used);

    quotav[3].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_out_used;
    quotav[3].iov_len = sizeof(quotatab_tally.bytes_out_used);

    quotav[4].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_xfer_used;
    quotav[4].iov_len = sizeof(quotatab_tally.bytes_xfer_used);

    quotav[5].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_in_used;
    quotav[5].iov_len = sizeof(quotatab_tally.files_in_used);

    quotav[6].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_out_used;
    quotav[6].iov_len = sizeof(quotatab_tally.files_out_used);

    quotav[7].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_xfer_used;
    quotav[7].iov_len = sizeof(quotatab_tally.files_xfer_used);

    if ((res = readv(filetab->tab_handle, quotav, 8)) > 0)

      /* Always rewind after reading a record. */
      lseek(filetab->tab_handle, current_pos, SEEK_SET);

    else if (res == 0) {

      /* Assume end-of-file. */
      errno = EOF;
      res = -1;
    }

    return res;

  } else if (filetab->tab_type == TYPE_LIMIT) {

    quotav[0].iov_base = QUOTATAB_IOV_BASE_TYPE quotatab_limit.name;
    quotav[0].iov_len = sizeof(quotatab_limit.name);

    quotav[1].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_limit.quota_type;
    quotav[1].iov_len = sizeof(quotatab_limit.quota_type);

    quotav[2].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.quota_per_session;
    quotav[2].iov_len = sizeof(quotatab_limit.quota_per_session);

    quotav[3].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.quota_limit_type;
    quotav[3].iov_len = sizeof(quotatab_limit.quota_limit_type);

    quotav[4].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.bytes_in_avail;
    quotav[4].iov_len = sizeof(quotatab_limit.bytes_in_avail);

    quotav[5].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.bytes_out_avail;
    quotav[5].iov_len = sizeof(quotatab_limit.bytes_out_avail);

    quotav[6].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.bytes_xfer_avail;
    quotav[6].iov_len = sizeof(quotatab_limit.bytes_xfer_avail);

    quotav[7].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.files_in_avail;
    quotav[7].iov_len = sizeof(quotatab_limit.files_in_avail);

    quotav[8].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.files_out_avail;
    quotav[8].iov_len = sizeof(quotatab_limit.files_out_avail);

    quotav[9].iov_base = QUOTATAB_IOV_BASE_TYPE
      &quotatab_limit.files_xfer_avail;
    quotav[9].iov_len = sizeof(quotatab_limit.files_xfer_avail);

    if ((res = readv(filetab->tab_handle, quotav, 10)) > 0)

      /* Always rewind after reading a record. */
      lseek(filetab->tab_handle, current_pos, SEEK_SET);

    else if (res == 0) {

      /* Assume end-of-file. */
      errno = EOF;
      res = -1;
    }

    return res;
  }

  /* default */
  errno = EINVAL;
  return -1;
}

static unsigned char filetab_verify(quota_table_t *filetab) {
  unsigned int magic = 0L;

  /* Make sure we are positioned at the start of the table. */
  lseek(filetab->tab_handle, 0, SEEK_SET);

  /* Check the header of this table, to make sure it's valid. */
  if (read(filetab->tab_handle, &magic, sizeof(magic)) != sizeof(magic))
    return FALSE;

  if (magic == filetab->tab_magic)
    return TRUE;

  return FALSE;
}

static int filetab_write(quota_table_t *filetab) {
  int res = -1;
  struct iovec quotav[8];

  /* Mark the current file position. */
  off_t current_pos = lseek(filetab->tab_handle, 0, SEEK_CUR);

  /* Use writev() to make this more efficient.  It is done piecewise, rather
   * than doing a normal write(2) directly from the struct pointer, to avoid
   * alignment/padding issues.
   */

  quotav[0].iov_base = QUOTATAB_IOV_BASE_TYPE quotatab_tally.name;
  quotav[0].iov_len = sizeof(quotatab_tally.name);

  quotav[1].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.quota_type;
  quotav[1].iov_len = sizeof(quotatab_tally.quota_type);

  quotav[2].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_in_used;
  quotav[2].iov_len = sizeof(quotatab_tally.bytes_in_used);

  quotav[3].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_out_used;
  quotav[3].iov_len = sizeof(quotatab_tally.bytes_out_used);

  quotav[4].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.bytes_xfer_used;
  quotav[4].iov_len = sizeof(quotatab_tally.bytes_xfer_used);

  quotav[5].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_in_used;
  quotav[5].iov_len = sizeof(quotatab_tally.files_in_used);

  quotav[6].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_out_used;
  quotav[6].iov_len = sizeof(quotatab_tally.files_out_used);

  quotav[7].iov_base = QUOTATAB_IOV_BASE_TYPE &quotatab_tally.files_xfer_used;
  quotav[7].iov_len = sizeof(quotatab_tally.files_xfer_used);

  if ((res = writev(filetab->tab_handle, quotav, 8)) >= 0)

    /* Always rewind after writing a record. */
    lseek(filetab->tab_handle, current_pos, SEEK_SET);

  else if (res == 0)

    /* It's an error if no bytes are written. */
    res = -1;

  return res;
}

static int filetab_rlock(quota_table_t *filetab) {
  filetab->tab_lock.l_type = F_RDLCK;
  return fcntl(filetab->tab_handle, F_SETLK, &filetab->tab_lock);
}

static int filetab_unlock(quota_table_t *filetab) {
  filetab->tab_lock.l_type = F_UNLCK;
  return fcntl(filetab->tab_handle, F_SETLK, &filetab->tab_lock);
}

static int filetab_wlock(quota_table_t *filetab) {
  filetab->tab_lock.l_type = F_WRLCK;
  return fcntl(filetab->tab_handle, F_SETLK, &filetab->tab_lock);
}

static quota_table_t *filetab_open(pool *parent_pool,
    quota_tabtype_t tab_type, const char *srcinfo) {
  quota_table_t *tab = NULL;
  pool *tab_pool = make_sub_pool(parent_pool);

  tab = (quota_table_t *) pcalloc(tab_pool, sizeof(quota_table_t));
  tab->tab_pool = tab_pool;
  tab->tab_type = tab_type;

  if (tab->tab_type == TYPE_TALLY) {

    /* File-based tally table magic number */
    tab->tab_magic = 0x07644;

    /* File-based tally record length, manually defined to avoid alignment/
     * padding issues when using sizeof().
     */
    tab->tab_quotalen = 121;

    tab->tab_lock.l_whence = SEEK_CUR;
    tab->tab_lock.l_start = 0;
    tab->tab_lock.l_len = tab->tab_quotalen;

    /* Open the table handle */
    if ((tab->tab_handle = open(srcinfo, O_RDWR)) < 0) {
      destroy_pool(tab->tab_pool);
      return NULL;
    }

  } else if (tab->tab_type == TYPE_LIMIT) {

    /* File-based limit table magic number */
    tab->tab_magic = 0x07626;

    /* File-based limit record length, manually defined to avoid alignment/
     * padding issues when using sizeof().
     */
    tab->tab_quotalen = 126;
    tab->tab_lock.l_whence = SEEK_CUR;
    tab->tab_lock.l_start = 0;
    tab->tab_lock.l_len = tab->tab_quotalen;

    /* Open the table handle */
    if ((tab->tab_handle = open(srcinfo, O_RDONLY)) < 0) {
      destroy_pool(tab->tab_pool);
      return NULL;
    }
  }

  /* Set all the necessary function pointers. */
  tab->tab_close = filetab_close;
  tab->tab_create = filetab_create;
  tab->tab_lookup = filetab_lookup;
  tab->tab_read = filetab_read;
  tab->tab_verify = filetab_verify;
  tab->tab_write = filetab_write;

  tab->tab_rlock = filetab_rlock;
  tab->tab_unlock = filetab_unlock;
  tab->tab_wlock = filetab_wlock;

  return tab;
}

/* Event handlers
 */

#if defined(PR_SHARED_MODULE)
static void filetab_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_quotatab_file.c", (const char *) event_data) == 0) {
    pr_event_unregister(&quotatab_file_module, NULL, NULL);
    quotatab_unregister_backend("file", QUOTATAB_LIMIT_SRC|QUOTATAB_TALLY_SRC);
  }
}
#endif /* PR_SHARED_MODULE */

/* Initialization routines
 */

static int filetab_init(void) {

  /* Initialize the quota source objects for type "file".
   */
  quotatab_register_backend("file", filetab_open,
    QUOTATAB_LIMIT_SRC|QUOTATAB_TALLY_SRC);

#if defined(PR_SHARED_MODULE)
  pr_event_register(&quotatab_file_module, "core.module-unload",
    filetab_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */

  return 0;
}

module quotatab_file_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "quotatab_file",

  /* Module configuration handler table */
  NULL,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  filetab_init,

  /* Module child initialization function */
  NULL
};

/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004 The ProFTPD Project team
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/*
 * HELP management code
 * $Id: help.c,v 1.1 2004/02/17 02:16:00 castaglia Exp $
 */

#include "conf.h"

struct help_rec {
  const char *cmd;
  const char *syntax;
  int impl;
};

static pool *help_pool = NULL;
static array_header *help_list = NULL;

void pr_help_add(const char *cmd, const char *syntax, int impl) {
  struct help_rec *help;

  if (!cmd || !syntax)
    return;

  /* If no list has been allocated, create one. */
  if (!help_pool) {
    help_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(help_pool, "HELP Pool");
    help_list = make_array(help_pool, 0, sizeof(struct help_rec));
  }

  /* Make sure that the command being added isn't already in the list.
   * However, if it _is_ already in the list, but it's marked as not
   * implemented, _and_ the given impl flag is TRUE, then handle it
   * accordingly.
   */
  if (help_list->nelts > 0) {
    register unsigned int i = 0;
    struct help_rec *helps = help_list->elts;

    for (i = 0; i < help_list->nelts; i++)
      if (strcmp(helps[i].cmd, cmd) == 0) {
        if (helps[i].impl == FALSE)
          helps[i].impl = impl;

        return;
      }
  }

  help = push_array(help_list);
  help->cmd = pstrdup(help_pool, cmd);
  help->syntax = pstrdup(help_pool, syntax);
  help->impl = impl;
}

int pr_help_add_response(cmd_rec *cmd, const char *target) {
  if (help_list) {
    register unsigned int i;
    struct help_rec *helps = help_list->elts;
    char *outa[8], *outstr;
    char buf[9] = {'\0'};
    int col = 0;

    if (!target) {
      pr_response_add(R_214,
        "The following commands are recognized (* =>'s unimplemented):");

      memset(outa, '\0', sizeof(outa));

      for (i = 0; i < help_list->nelts; i++) {
        outstr = "";

        if (helps[i].impl)
          outa[col++] = (char *) helps[i].cmd;
        else
          outa[col++] = pstrcat(cmd->tmp_pool, helps[i].cmd, "*", NULL);

        /* 8 rows */
        if ((i + 1) % 8 == 0) {
          register unsigned int j;

          for (j = 0; j < 8; j++) {
            if (outa[j]) {
              snprintf(buf, sizeof(buf), "%-8s", outa[j]);
              buf[sizeof(buf)-1] = '\0';
              outstr = pstrcat(cmd->tmp_pool, outstr, buf, NULL);

            } else
              break;
          }

          if (*outstr)
            pr_response_add(R_DUP, "%s", outstr);

          memset(outa, '\0', sizeof(outa));
          col = 0;
          outstr = "";
        }
      }

      pr_response_add(R_DUP, "Direct comments to %s",
        cmd->server->ServerAdmin ? cmd->server->ServerAdmin : "ftp-admin");

    } else {

      /* List the syntax for the given target command. */
      for (i = 0; i < help_list->nelts; i++) {
        if (strcasecmp(helps[i].cmd, target) == 0) {
          pr_response_add(R_214, "Syntax: %s %s", helps[i].cmd,
            helps[i].syntax);
          return 0;
        }
      }
    }

    errno = ENOENT;
    return -1;
  }

  errno = ENOENT;
  return -1;
}

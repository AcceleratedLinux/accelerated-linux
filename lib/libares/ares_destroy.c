/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: ares_destroy.c,v 1.2 1998/09/22 01:46:10 ghudson Exp $";

#include <stdlib.h>
#include "ares.h"
#include "ares_private.h"

void ares_destroy(ares_channel channel)
{
  int i;
  struct query *query;

  for (i = 0; i < channel->nservers; i++)
    ares__close_sockets(&channel->servers[i]);
  free(channel->servers);
  for (i = 0; i < channel->ndomains; i++)
    free(channel->domains[i]);
  free(channel->domains);
  free(channel->sortlist);
  free(channel->lookups);
  while (channel->queries)
    {
      query = channel->queries;
      channel->queries = query->next;
      query->callback(query->arg, ARES_EDESTRUCTION, NULL, 0);
      free(query->tcpbuf);
      free(query->skip_server);
      free(query);
    }
  free(channel);
}

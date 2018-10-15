/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* Written by Sergei A. Golubchik, who has a shared copyright to this code
   added support for long options (my_getopt) 22.5.2002 by Jani Tolonen */

#include "ftdefs.h"
#include <my_getopt.h>

static void get_options(int *argc,char **argv[]);
static void usage();
static void complain(int val);

static int count=0, stats=0, dump=0, lstats=0;
static my_bool verbose;
static char *query=NULL;
static uint lengths[256];

#define MAX_LEN (HA_FT_MAXLEN+10)
#define HOW_OFTEN_TO_WRITE 10000

static struct my_option my_long_options[] =
{
  {"dump", 'd', "Dump index (incl. data offsets and word weights)",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"stats", 's', "Report global stats",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"verbose", 'v', "Be verbose",
   (gptr*) &verbose, (gptr*) &verbose, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"count", 'c', "Calculate per-word stats (counts and global weights)",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"length", 'l', "Report length distribution",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"execute", 'e', "Execute given query", (gptr*) &query, (gptr*) &query, 0,
   GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"help", 'h', "Display help and exit",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"help", '?', "Synonym for -h",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};


int main(int argc,char *argv[])
{
  int error=0;
  uint keylen, keylen2=0, inx, doc_cnt=0;
  float weight;
  double gws, min_gws=0, avg_gws=0;
  MI_INFO *info;
  char buf[MAX_LEN], buf2[MAX_LEN], buf_maxlen[MAX_LEN], buf_min_gws[MAX_LEN];
  ulong total=0, maxlen=0, uniq=0, max_doc_cnt=0;
  struct { MI_INFO *info; } aio0, *aio=&aio0; /* for GWS_IN_USE */

  MY_INIT(argv[0]);
  get_options(&argc, &argv);
  if (count || dump)
    verbose=0;
  if (!count && !dump && !lstats && !query)
    stats=1;

  if (verbose)
    setbuf(stdout,NULL);

  if (argc < 2)
    usage();

  if (!(info=mi_open(argv[0],2,HA_OPEN_ABORT_IF_LOCKED)))
    goto err;

  inx=atoi(argv[1]);
  *buf2=0;
  aio->info=info;

  if ((inx >= info->s->base.keys) ||
      !(info->s->keyinfo[inx].flag & HA_FULLTEXT))
  {
    printf("Key %d in table %s is not a FULLTEXT key\n", inx, info->filename);
    goto err;
  }

  if (query)
  {
#if 0
    FT_DOCLIST *result;
    int i;

    ft_init_stopwords(ft_precompiled_stopwords);

    result=ft_nlq_init_search(info,inx,query,strlen(query),1);
    if(!result)
      goto err;

    if (verbose)
      printf("%d rows matched\n",result->ndocs);

    for(i=0 ; i<result->ndocs ; i++)
      printf("%9qx %20.7f\n",result->doc[i].dpos,result->doc[i].weight);

    ft_nlq_close_search(result);
#else
    printf("-e option is disabled\n");
#endif
  }
  else
  {
    info->lastpos= HA_OFFSET_ERROR;
    info->update|= HA_STATE_PREV_FOUND;

    while (!(error=mi_rnext(info,NULL,inx)))
    {
      keylen=*(info->lastkey);

#if HA_FT_WTYPE == HA_KEYTYPE_FLOAT
      mi_float4get(weight,info->lastkey+keylen+1);
#else
#error
#endif

#ifdef HAVE_SNPRINTF
      snprintf(buf,MAX_LEN,"%.*s",(int) keylen,info->lastkey+1);
#else
      sprintf(buf,"%.*s",(int) keylen,info->lastkey+1);
#endif
      casedn_str(buf);
      total++;
      lengths[keylen]++;

      if (count || stats)
      {
        doc_cnt++;
        if (strcmp(buf, buf2))
        {
          if (*buf2)
          {
            uniq++;
            avg_gws+=gws=GWS_IN_USE;
            if (count)
              printf("%9u %20.7f %s\n",doc_cnt,gws,buf2);
            if (maxlen<keylen2)
            {
              maxlen=keylen2;
              strmov(buf_maxlen, buf2);
            }
            if (max_doc_cnt < doc_cnt)
            {
              max_doc_cnt=doc_cnt;
              strmov(buf_min_gws, buf2);
              min_gws=gws;
            }
          }
          strmov(buf2, buf);
          keylen2=keylen;
          doc_cnt=0;
        }
      }
      if (dump)
        printf("%9qx %20.7f %s\n",info->lastpos,weight,buf);

      if(verbose && (total%HOW_OFTEN_TO_WRITE)==0)
        printf("%10ld\r",total);
    }

    if (stats)
    {
      count=0;
      for (inx=0;inx<256;inx++)
      {
        count+=lengths[inx];
        if ((ulong) count >= total/2)
          break;
      }
      printf("Total rows: %qu\nTotal words: %lu\n"
             "Unique words: %lu\nLongest word: %lu chars (%s)\n"
             "Median length: %u\n"
             "Average global weight: %f\n"
             "Most common word: %lu times, weight: %f (%s)\n",
             (ulonglong)info->state->records, total, uniq, maxlen, buf_maxlen,
             inx, avg_gws/uniq, max_doc_cnt, min_gws, buf_min_gws);
    }
    if (lstats)
    {
      count=0;
      for (inx=0; inx<256; inx++)
      {
        count+=lengths[inx];
        if (count && lengths[inx])
          printf("%3u: %10lu %5.2f%% %20lu %4.1f%%\n", inx,
		 (ulong) lengths[inx],100.0*lengths[inx]/total,(ulong) count,
		 100.0*count/total);
      }
    }
  }

err:
  if (error && error != HA_ERR_END_OF_FILE)
    printf("got error %d\n",my_errno);
  if (info)
    mi_close(info);
  return 0;
}


static my_bool
get_one_option(int optid, const struct my_option *opt __attribute__((unused)),
	       char *argument __attribute__((unused)))
{
  switch(optid) {
  case 'd':
    dump=1; 
    complain(count || query);
    break;
  case 's': 
    stats=1; 
    complain(query!=0);
    break;
  case 'c': 
    count= 1;
    complain(dump || query);
    break;
  case 'l': 
    lstats=1;
    complain(query!=0);
    break;
  case 'e':
    complain(dump || count || stats);
    break;
  case '?':
  case 'h':
    usage();
  }
  return 0;
}


static void get_options(int *argc, char **argv[])
{
  int ho_error;

  if ((ho_error=handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);
} /* get options */


static void usage()
{
  printf("Use: ft_dump <table_name> <index_no>\n");
  my_print_help(my_long_options);
  my_print_variables(my_long_options);
  exit(1);
}


static void complain(int val) /* Kinda assert :-)  */
{
  if (val)
  {
    printf("You cannot use these options together!\n");
    exit(1);
  }
}

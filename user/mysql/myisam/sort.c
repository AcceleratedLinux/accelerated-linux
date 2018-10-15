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

/*
  Creates a index for a database by reading keys, sorting them and outputing
  them in sorted order through SORT_INFO functions.
*/

#include "fulltext.h"
#if defined(MSDOS) || defined(__WIN__)
#include <fcntl.h>
#else
#include <stddef.h>
#endif
#include <queues.h>

/* static variables */

#undef MIN_SORT_MEMORY
#undef MYF_RW
#undef DISK_BUFFER_SIZE

#define MERGEBUFF 15
#define MERGEBUFF2 31
#define MIN_SORT_MEMORY (4096-MALLOC_OVERHEAD)
#define MYF_RW  MYF(MY_NABP | MY_WME | MY_WAIT_IF_FULL)
#define DISK_BUFFER_SIZE (IO_SIZE*16)

typedef struct st_buffpek {
  my_off_t file_pos;                    /* Where we are in the sort file */
  uchar *base,*key;                     /* Key pointers */
  ha_rows count;                        /* Number of rows in table */
  ulong mem_count;                      /* numbers of keys in memory */
  ulong max_keys;                       /* Max keys in buffert */
} BUFFPEK;

extern void print_error _VARARGS((const char *fmt,...));

/* Functions defined in this file */

static ha_rows NEAR_F find_all_keys(MI_SORT_PARAM *info,uint keys,
                                    uchar **sort_keys,
                                    DYNAMIC_ARRAY *buffpek,int *maxbuffer,
                                    IO_CACHE *tempfile,
                                    IO_CACHE *tempfile_for_exceptions);
static int NEAR_F write_keys(MI_SORT_PARAM *info,uchar * *sort_keys,
                             uint count, BUFFPEK *buffpek,IO_CACHE *tempfile);
static int NEAR_F write_key(MI_SORT_PARAM *info, uchar *key,
			    IO_CACHE *tempfile);
static int NEAR_F write_index(MI_SORT_PARAM *info,uchar * *sort_keys,
                              uint count);
static int NEAR_F merge_many_buff(MI_SORT_PARAM *info,uint keys,
                                  uchar * *sort_keys,
                                  BUFFPEK *buffpek,int *maxbuffer,
                                  IO_CACHE *t_file);
static uint NEAR_F read_to_buffer(IO_CACHE *fromfile,BUFFPEK *buffpek,
                                  uint sort_length);
static int NEAR_F merge_buffers(MI_SORT_PARAM *info,uint keys,
                                IO_CACHE *from_file, IO_CACHE *to_file,
                                uchar * *sort_keys, BUFFPEK *lastbuff,
                                BUFFPEK *Fb, BUFFPEK *Tb);
static int NEAR_F merge_index(MI_SORT_PARAM *,uint,uchar **,BUFFPEK *, int,
                              IO_CACHE *);


/*
  Creates a index of sorted keys

  SYNOPSIS
    _create_index_by_sort()
    info		Sort parameters
    no_messages		Set to 1 if no output
    sortbuff_size	Size if sortbuffer to allocate

  RESULT
    0	ok
   <> 0 Error
*/

int _create_index_by_sort(MI_SORT_PARAM *info,my_bool no_messages,
			  ulong sortbuff_size)
{
  int error,maxbuffer,skr;
  uint memavl,old_memavl,keys,sort_length;
  DYNAMIC_ARRAY buffpek;
  ha_rows records;
  uchar **sort_keys;
  IO_CACHE tempfile, tempfile_for_exceptions;
  DBUG_ENTER("_create_index_by_sort");
  DBUG_PRINT("enter",("sort_length: %d", info->key_length));

  my_b_clear(&tempfile);
  my_b_clear(&tempfile_for_exceptions);
  bzero((char*) &buffpek,sizeof(buffpek));
  sort_keys= (uchar **) NULL; error= 1;
  maxbuffer=1;

  memavl=max(sortbuff_size,MIN_SORT_MEMORY);
  records=	info->sort_info->max_records;
  sort_length=	info->key_length;
  LINT_INIT(keys);

  while (memavl >= MIN_SORT_MEMORY)
  {
    if ((my_off_t) (records+1)*(sort_length+sizeof(char*)) <=
	(my_off_t) memavl)
      keys= records+1;
    else
      do
      {
	skr=maxbuffer;
	if (memavl < sizeof(BUFFPEK)*(uint) maxbuffer ||
	    (keys=(memavl-sizeof(BUFFPEK)*(uint) maxbuffer)/
	     (sort_length+sizeof(char*))) <= 1)
	{
	  mi_check_print_error(info->sort_info->param,
			       "sort_buffer_size is to small");
	  goto err;
	}
      }
      while ((maxbuffer= (int) (records/(keys-1)+1)) != skr);

    if ((sort_keys=(uchar **)my_malloc(keys*(sort_length+sizeof(char*))+
				       HA_FT_MAXLEN, MYF(0))))
    {
      if (my_init_dynamic_array(&buffpek, sizeof(BUFFPEK), maxbuffer,
			     maxbuffer/2))
	my_free((gptr) sort_keys,MYF(0));
      else
	break;
    }
    old_memavl=memavl;
    if ((memavl=memavl/4*3) < MIN_SORT_MEMORY && old_memavl > MIN_SORT_MEMORY)
      memavl=MIN_SORT_MEMORY;
  }
  if (memavl < MIN_SORT_MEMORY)
  {
    mi_check_print_error(info->sort_info->param,"Sort buffer to small"); /* purecov: tested */
    goto err; /* purecov: tested */
  }
  (*info->lock_in_memory)(info->sort_info->param);/* Everything is allocated */

  if (!no_messages)
    printf("  - Searching for keys, allocating buffer for %d keys\n",keys);

  if ((records=find_all_keys(info,keys,sort_keys,&buffpek,&maxbuffer,
                                  &tempfile,&tempfile_for_exceptions))
      == HA_POS_ERROR)
    goto err; /* purecov: tested */
  if (maxbuffer == 0)
  {
    if (!no_messages)
      printf("  - Dumping %lu keys\n", (ulong) records);
    if (write_index(info,sort_keys, (uint) records))
      goto err; /* purecov: inspected */
  }
  else
  {
    keys=(keys*(sort_length+sizeof(char*)))/sort_length;
    if (maxbuffer >= MERGEBUFF2)
    {
      if (!no_messages)
	printf("  - Merging %lu keys\n", (ulong) records); /* purecov: tested */
      if (merge_many_buff(info,keys,sort_keys,
                  dynamic_element(&buffpek,0,BUFFPEK *),&maxbuffer,&tempfile))
	goto err;				/* purecov: inspected */
    }
    if (flush_io_cache(&tempfile) ||
	reinit_io_cache(&tempfile,READ_CACHE,0L,0,0))
      goto err;					/* purecov: inspected */
    if (!no_messages)
      puts("  - Last merge and dumping keys\n"); /* purecov: tested */
    if (merge_index(info,keys,sort_keys,dynamic_element(&buffpek,0,BUFFPEK *),
                    maxbuffer,&tempfile))
      goto err;					/* purecov: inspected */
  }

  if (flush_pending_blocks(info))
    goto err;

  if (my_b_inited(&tempfile_for_exceptions))
  {
    MI_INFO *index=info->sort_info->info;
    uint     keyno=info->key;
    uint     key_length, ref_length=index->s->rec_reflength;

    if (flush_io_cache(&tempfile_for_exceptions) ||
	reinit_io_cache(&tempfile_for_exceptions,READ_CACHE,0L,0,0))
      goto err;

    while (!my_b_read(&tempfile_for_exceptions,(byte*)&key_length,
		      sizeof(key_length))
        && !my_b_read(&tempfile_for_exceptions,(byte*)sort_keys,
		      (uint) key_length))
    {
	if (_mi_ck_write(index,keyno,(uchar*) sort_keys,key_length-ref_length))
	  goto err;
    }
  }

  error =0;

err:
  if (sort_keys)
    my_free((gptr) sort_keys,MYF(0));
  delete_dynamic(&buffpek);
  close_cached_file(&tempfile);
  close_cached_file(&tempfile_for_exceptions);

  DBUG_RETURN(error ? -1 : 0);
} /* _create_index_by_sort */


/* Search after all keys and place them in a temp. file */

static ha_rows NEAR_F find_all_keys(MI_SORT_PARAM *info, uint keys,
				    uchar **sort_keys, DYNAMIC_ARRAY *buffpek,
				    int *maxbuffer, IO_CACHE *tempfile,
				    IO_CACHE *tempfile_for_exceptions)
{
  int error;
  uint idx;
  DBUG_ENTER("find_all_keys");

  idx=error=0;
  sort_keys[0]=(uchar*) (sort_keys+keys);

  while (!(error=(*info->key_read)(info,sort_keys[idx])))
  {
    if (info->real_key_length > info->key_length)
    {
      if (write_key(info,sort_keys[idx],tempfile_for_exceptions))
        DBUG_RETURN(HA_POS_ERROR);		/* purecov: inspected */
      continue;
    }

    if (++idx == keys)
    {
      if (write_keys(info,sort_keys,idx-1,(BUFFPEK *)alloc_dynamic(buffpek),
		     tempfile))
      DBUG_RETURN(HA_POS_ERROR);		/* purecov: inspected */

      sort_keys[0]=(uchar*) (sort_keys+keys);
      memcpy(sort_keys[0],sort_keys[idx-1],(size_t) info->key_length);
      idx=1;
    }
    sort_keys[idx]=sort_keys[idx-1]+info->key_length;
  }
  if (error > 0)
    DBUG_RETURN(HA_POS_ERROR);		/* Aborted by get_key */ /* purecov: inspected */
  if (buffpek->elements)
  {
    if (write_keys(info,sort_keys,idx,(BUFFPEK *)alloc_dynamic(buffpek),
		   tempfile))
      DBUG_RETURN(HA_POS_ERROR);		/* purecov: inspected */
    *maxbuffer=buffpek->elements-1;
  }
  else
    *maxbuffer=0;

  DBUG_RETURN((*maxbuffer)*(keys-1)+idx);
} /* find_all_keys */


#ifdef THREAD
/* Search after all keys and place them in a temp. file */

pthread_handler_decl(thr_find_all_keys,arg)
{
  MI_SORT_PARAM *info= (MI_SORT_PARAM*) arg;
  int error;
  uint memavl,old_memavl,keys,sort_length;
  uint idx, maxbuffer;
  uchar **sort_keys=0;

  LINT_INIT(keys);

  error=1;

  if (my_thread_init())
    goto err;
  if (info->sort_info->got_error)
    goto err;

  my_b_clear(&info->tempfile);
  my_b_clear(&info->tempfile_for_exceptions);
  bzero((char*) &info->buffpek,sizeof(info->buffpek));
  bzero((char*) &info->unique, sizeof(info->unique));
  sort_keys= (uchar **) NULL;

  memavl=max(info->sortbuff_size, MIN_SORT_MEMORY);
  idx=      info->sort_info->max_records;
  sort_length=  info->key_length;
  maxbuffer= 1;

  while (memavl >= MIN_SORT_MEMORY)
  {
    if ((my_off_t) (idx+1)*(sort_length+sizeof(char*)) <=
        (my_off_t) memavl)
      keys= idx+1;
    else
    {
      uint skr;
      do
      {
        skr=maxbuffer;
        if (memavl < sizeof(BUFFPEK)*maxbuffer ||
            (keys=(memavl-sizeof(BUFFPEK)*maxbuffer)/
             (sort_length+sizeof(char*))) <= 1)
        {
          mi_check_print_error(info->sort_info->param,
                               "sort_buffer_size is to small");
          goto err;
        }
      }
      while ((maxbuffer= (int) (idx/(keys-1)+1)) != skr);
    }
    if ((sort_keys=(uchar **)my_malloc(keys*(sort_length+sizeof(char*))+
				       ((info->keyinfo->flag & HA_FULLTEXT) ?
					HA_FT_MAXLEN : 0), MYF(0))))
    {
      if (my_init_dynamic_array(&info->buffpek, sizeof(BUFFPEK),
				maxbuffer, maxbuffer/2))
        my_free((gptr) sort_keys,MYF(0));
      else
        break;
    }
    old_memavl=memavl;
    if ((memavl=memavl/4*3) < MIN_SORT_MEMORY && old_memavl > MIN_SORT_MEMORY)
      memavl=MIN_SORT_MEMORY;
  }
  if (memavl < MIN_SORT_MEMORY)
  {
    mi_check_print_error(info->sort_info->param,"Sort buffer to small"); /* purecov: tested */
    goto err; /* purecov: tested */
  }

  if (info->sort_info->param->testflag & T_VERBOSE)
    printf("Key %d - Allocating buffer for %d keys\n",info->key+1,keys);
  info->sort_keys=sort_keys;

  idx=error=0;
  sort_keys[0]=(uchar*) (sort_keys+keys);

  while (!(error=info->sort_info->got_error) &&
         !(error=(*info->key_read)(info,sort_keys[idx])))
  {
    if (info->real_key_length > info->key_length)
    {
      if (write_key(info,sort_keys[idx], &info->tempfile_for_exceptions))
        goto err;
      continue;
    }

    if (++idx == keys)
    {
      if (write_keys(info,sort_keys,idx-1,
		     (BUFFPEK *)alloc_dynamic(&info->buffpek),
		     &info->tempfile))
        goto err;
      sort_keys[0]=(uchar*) (sort_keys+keys);
      memcpy(sort_keys[0],sort_keys[idx-1],(size_t) info->key_length);
      idx=1;
    }
    sort_keys[idx]=sort_keys[idx-1]+info->key_length;
  }
  if (error > 0)
    goto err;
  if (info->buffpek.elements)
  {
    if (write_keys(info,sort_keys, idx,
		   (BUFFPEK *) alloc_dynamic(&info->buffpek), &info->tempfile))
      goto err;
    info->keys=(info->buffpek.elements-1)*(keys-1)+idx;
  }
  else
    info->keys=idx;

  info->sort_keys_length=keys;
  goto ok;

err:
  info->sort_info->got_error=1; /* no need to protect this with a mutex */
  if (sort_keys)
    my_free((gptr) sort_keys,MYF(0));
  info->sort_keys=0;
  delete_dynamic(& info->buffpek);
  close_cached_file(&info->tempfile);
  close_cached_file(&info->tempfile_for_exceptions);

ok:
  remove_io_thread(&info->read_cache);
  pthread_mutex_lock(&info->sort_info->mutex);
  info->sort_info->threads_running--;
  pthread_cond_signal(&info->sort_info->cond);
  pthread_mutex_unlock(&info->sort_info->mutex);
  my_thread_end();
  return NULL;
}


int thr_write_keys(MI_SORT_PARAM *sort_param)
{
  SORT_INFO *sort_info=sort_param->sort_info;
  MI_CHECK *param=sort_info->param;
  ulong length, keys;
  ulong *rec_per_key_part=param->rec_per_key_part;
  int got_error=sort_info->got_error;
  uint i;
  MI_INFO *info=sort_info->info;
  MYISAM_SHARE *share=info->s;
  MI_SORT_PARAM *sinfo;
  byte *mergebuf=0;
  LINT_INIT(length);

  for (i= 0, sinfo= sort_param ;
       i < sort_info->total_keys ;
       i++, rec_per_key_part+=sinfo->keyinfo->keysegs, sinfo++)
  {
    if (!sinfo->sort_keys)
    {
      got_error=1;
      continue;
    }
    if (!got_error)
    {
      share->state.key_map|=(ulonglong) 1 << sinfo->key;
      if (param->testflag & T_STATISTICS)
        update_key_parts(sinfo->keyinfo, rec_per_key_part,
                         sinfo->unique, (ulonglong) info->state->records);
      if (!sinfo->buffpek.elements)
      {
        if (param->testflag & T_VERBOSE)
        {
          printf("Key %d  - Dumping %u keys\n",sinfo->key+1, sinfo->keys);
          fflush(stdout);
        }
        if (write_index(sinfo, sinfo->sort_keys, sinfo->keys) ||
            flush_pending_blocks(sinfo))
          got_error=1;
      }
    }
    my_free((gptr) sinfo->sort_keys,MYF(0));
    my_free(mi_get_rec_buff_ptr(info, sinfo->rec_buff),
	    MYF(MY_ALLOW_ZERO_PTR));
    sinfo->sort_keys=0;
  }

  for (i= 0, sinfo= sort_param ;
       i < sort_info->total_keys ;
       i++,
	 delete_dynamic(&sinfo->buffpek),
	 close_cached_file(&sinfo->tempfile),
	 close_cached_file(&sinfo->tempfile_for_exceptions),
	 sinfo++)
  {
    if (got_error)
      continue;
    if (sinfo->buffpek.elements)
    {
      uint maxbuffer=sinfo->buffpek.elements-1;
      if (!mergebuf)
      {
        length=param->sort_buffer_length;
        while (length >= MIN_SORT_MEMORY && !mergebuf)
        {
          mergebuf=my_malloc(length, MYF(0));
          length=length*3/4;
        }
        if (!mergebuf)
        {
          got_error=1;
          continue;
        }
      }
      keys=length/sinfo->key_length;
      if (maxbuffer >= MERGEBUFF2)
      {
        if (param->testflag & T_VERBOSE)
          printf("Key %d  - Merging %u keys\n",sinfo->key+1, sinfo->keys);
        if (merge_many_buff(sinfo, keys, (uchar **)mergebuf,
			    dynamic_element(&sinfo->buffpek, 0, BUFFPEK *),
			    (int*) &maxbuffer, &sinfo->tempfile))
        {
          got_error=1;
          continue;
        }
      }
      if (flush_io_cache(&sinfo->tempfile) ||
          reinit_io_cache(&sinfo->tempfile,READ_CACHE,0L,0,0))
      {
        got_error=1;
        continue;
      }
      if (param->testflag & T_VERBOSE)
        printf("Key %d  - Last merge and dumping keys\n", sinfo->key+1);
      if (merge_index(sinfo, keys, (uchar **)mergebuf,
                      dynamic_element(&sinfo->buffpek,0,BUFFPEK *),
                      maxbuffer,&sinfo->tempfile) ||
	  flush_pending_blocks(sinfo))
      {
        got_error=1;
        continue;
      }
    }
    if (my_b_inited(&sinfo->tempfile_for_exceptions))
    {
      uint key_length;

      if (param->testflag & T_VERBOSE)
        printf("Key %d  - Dumping 'long' keys\n", sinfo->key+1);

      if (flush_io_cache(&sinfo->tempfile_for_exceptions) ||
          reinit_io_cache(&sinfo->tempfile_for_exceptions,READ_CACHE,0L,0,0))
      {
        got_error=1;
        continue;
      }

      while (!got_error &&
	     !my_b_read(&sinfo->tempfile_for_exceptions,(byte*)&key_length,
			sizeof(key_length)) &&
	     !my_b_read(&sinfo->tempfile_for_exceptions,(byte*)mergebuf,
			(uint) key_length))
      {
	if (_mi_ck_write(info,sinfo->key,(uchar*) mergebuf,
			 key_length - info->s->rec_reflength))
	  got_error=1;
      }
    }
  }
  my_free((gptr) mergebuf,MYF(MY_ALLOW_ZERO_PTR));
  return got_error;
}
#endif /* THREAD */

        /* Write all keys in memory to file for later merge */

static int NEAR_F write_keys(MI_SORT_PARAM *info, register uchar **sort_keys,
                             uint count, BUFFPEK *buffpek, IO_CACHE *tempfile)
{
  uchar **end;
  uint sort_length=info->key_length;
  DBUG_ENTER("write_keys");

  qsort2((byte*) sort_keys,count,sizeof(byte*),(qsort2_cmp) info->key_cmp,
         info);
  if (!my_b_inited(tempfile) &&
      open_cached_file(tempfile, info->tmpdir, "ST", DISK_BUFFER_SIZE,
                       info->sort_info->param->myf_rw))
    DBUG_RETURN(1); /* purecov: inspected */

  buffpek->file_pos=my_b_tell(tempfile);
  buffpek->count=count;

  for (end=sort_keys+count ; sort_keys != end ; sort_keys++)
  {
    if (my_b_write(tempfile,(byte*) *sort_keys,(uint) sort_length))
      DBUG_RETURN(1); /* purecov: inspected */
  }
  DBUG_RETURN(0);
} /* write_keys */


static int NEAR_F write_key(MI_SORT_PARAM *info, uchar *key,
			    IO_CACHE *tempfile)
{
  uint key_length=info->real_key_length;
  DBUG_ENTER("write_key");

  if (!my_b_inited(tempfile) &&
      open_cached_file(tempfile, info->tmpdir, "ST", DISK_BUFFER_SIZE,
                       info->sort_info->param->myf_rw))
    DBUG_RETURN(1);

  if (my_b_write(tempfile,(byte*)&key_length,sizeof(key_length)) ||
      my_b_write(tempfile,(byte*)key,(uint) key_length))
    DBUG_RETURN(1);
  DBUG_RETURN(0);
} /* write_key */


/* Write index */

static int NEAR_F write_index(MI_SORT_PARAM *info, register uchar **sort_keys,
                              register uint count)
{
  DBUG_ENTER("write_index");

  qsort2((gptr) sort_keys,(size_t) count,sizeof(byte*),
        (qsort2_cmp) info->key_cmp,info);
  while (count--)
  {
    if ((*info->key_write)(info,*sort_keys++))
      DBUG_RETURN(-1); /* purecov: inspected */
  }
  DBUG_RETURN(0);
} /* write_index */


        /* Merge buffers to make < MERGEBUFF2 buffers */

static int NEAR_F merge_many_buff(MI_SORT_PARAM *info, uint keys,
                                  uchar **sort_keys, BUFFPEK *buffpek,
                                  int *maxbuffer, IO_CACHE *t_file)
{
  register int i;
  IO_CACHE t_file2, *from_file, *to_file, *temp;
  BUFFPEK *lastbuff;
  DBUG_ENTER("merge_many_buff");

  if (*maxbuffer < MERGEBUFF2)
    DBUG_RETURN(0);                             /* purecov: inspected */
  if (flush_io_cache(t_file) ||
      open_cached_file(&t_file2,info->tmpdir,"ST",DISK_BUFFER_SIZE,
                       info->sort_info->param->myf_rw))
    DBUG_RETURN(1);                             /* purecov: inspected */

  from_file= t_file ; to_file= &t_file2;
  while (*maxbuffer >= MERGEBUFF2)
  {
    reinit_io_cache(from_file,READ_CACHE,0L,0,0);
    reinit_io_cache(to_file,WRITE_CACHE,0L,0,0);
    lastbuff=buffpek;
    for (i=0 ; i <= *maxbuffer-MERGEBUFF*3/2 ; i+=MERGEBUFF)
    {
      if (merge_buffers(info,keys,from_file,to_file,sort_keys,lastbuff++,
                        buffpek+i,buffpek+i+MERGEBUFF-1))
        break; /* purecov: inspected */
    }
    if (merge_buffers(info,keys,from_file,to_file,sort_keys,lastbuff++,
                      buffpek+i,buffpek+ *maxbuffer))
      break; /* purecov: inspected */
    if (flush_io_cache(to_file))
      break;                                    /* purecov: inspected */
    temp=from_file; from_file=to_file; to_file=temp;
    *maxbuffer= (int) (lastbuff-buffpek)-1;
  }
  close_cached_file(to_file);                   /* This holds old result */
  if (to_file == t_file)
    *t_file=t_file2;                            /* Copy result file */

  DBUG_RETURN(*maxbuffer >= MERGEBUFF2);        /* Return 1 if interrupted */
} /* merge_many_buff */


/*
   Read data to buffer

  SYNOPSIS
    read_to_buffer()
    fromfile		File to read from
    buffpek		Where to read from
    sort_length		max length to read
  RESULT
    > 0	Ammount of bytes read
    -1	Error
*/

static uint NEAR_F read_to_buffer(IO_CACHE *fromfile, BUFFPEK *buffpek,
                                  uint sort_length)
{
  register uint count;
  uint length;

  if ((count=(uint) min((ha_rows) buffpek->max_keys,buffpek->count)))
  {
    if (my_pread(fromfile->file,(byte*) buffpek->base,
                 (length= sort_length*count),buffpek->file_pos,MYF_RW))
      return((uint) -1);                        /* purecov: inspected */
    buffpek->key=buffpek->base;
    buffpek->file_pos+= length;                 /* New filepos */
    buffpek->count-=    count;
    buffpek->mem_count= count;
  }
  return (count*sort_length);
} /* read_to_buffer */


/*
  Merge buffers to one buffer
  If to_file == 0 then use info->key_write
*/

static int NEAR_F
merge_buffers(MI_SORT_PARAM *info, uint keys, IO_CACHE *from_file,
              IO_CACHE *to_file, uchar **sort_keys, BUFFPEK *lastbuff,
              BUFFPEK *Fb, BUFFPEK *Tb)
{
  int error;
  uint sort_length,maxcount;
  ha_rows count;
  my_off_t to_start_filepos;
  uchar *strpos;
  BUFFPEK *buffpek,**refpek;
  QUEUE queue;
  DBUG_ENTER("merge_buffers");

  count=error=0;
  maxcount=keys/((uint) (Tb-Fb) +1);
  LINT_INIT(to_start_filepos);
  if (to_file)
    to_start_filepos=my_b_tell(to_file);
  strpos=(uchar*) sort_keys;
  sort_length=info->key_length;

  if (init_queue(&queue,(uint) (Tb-Fb)+1,offsetof(BUFFPEK,key),0,
                 (int (*)(void*, byte *,byte*)) info->key_cmp,
                 (void*) info))
    DBUG_RETURN(1); /* purecov: inspected */

  for (buffpek= Fb ; buffpek <= Tb ; buffpek++)
  {
    count+= buffpek->count;
    buffpek->base= strpos;
    buffpek->max_keys=maxcount;
    strpos+= (uint) (error=(int) read_to_buffer(from_file,buffpek,
                                                sort_length));
    if (error == -1)
      goto err; /* purecov: inspected */
    queue_insert(&queue,(char*) buffpek);
  }

  while (queue.elements > 1)
  {
    for (;;)
    {
      buffpek=(BUFFPEK*) queue_top(&queue);
      if (to_file)
      {
        if (my_b_write(to_file,(byte*) buffpek->key,(uint) sort_length))
        {
          error=1; goto err; /* purecov: inspected */
        }
      }
      else
      {
        if ((*info->key_write)(info,(void*) buffpek->key))
        {
          error=1; goto err; /* purecov: inspected */
        }
      }
      buffpek->key+=sort_length;
      if (! --buffpek->mem_count)
      {
        if (!(error=(int) read_to_buffer(from_file,buffpek,sort_length)))
        {
          uchar *base=buffpek->base;
          uint max_keys=buffpek->max_keys;

          VOID(queue_remove(&queue,0));

          /* Put room used by buffer to use in other buffer */
          for (refpek= (BUFFPEK**) &queue_top(&queue);
               refpek <= (BUFFPEK**) &queue_end(&queue);
               refpek++)
          {
            buffpek= *refpek;
            if (buffpek->base+buffpek->max_keys*sort_length == base)
            {
              buffpek->max_keys+=max_keys;
              break;
            }
            else if (base+max_keys*sort_length == buffpek->base)
            {
              buffpek->base=base;
              buffpek->max_keys+=max_keys;
              break;
            }
          }
          break;                /* One buffer have been removed */
        }
      }
      else if (error == -1)
        goto err;               /* purecov: inspected */
      queue_replaced(&queue);   /* Top element has been replaced */
    }
  }
  buffpek=(BUFFPEK*) queue_top(&queue);
  buffpek->base=(uchar *) sort_keys;
  buffpek->max_keys=keys;
  do
  {
    if (to_file)
    {
      if (my_b_write(to_file,(byte*) buffpek->key,
                     (sort_length*buffpek->mem_count)))
      {
        error=1; goto err; /* purecov: inspected */
      }
    }
    else
    {
      register uchar *end;
      strpos= buffpek->key;
      for (end=strpos+buffpek->mem_count*sort_length;
           strpos != end ;
           strpos+=sort_length)
      {
        if ((*info->key_write)(info,(void*) strpos))
        {
          error=1; goto err; /* purecov: inspected */
        }
      }
    }
  }
  while ((error=(int) read_to_buffer(from_file,buffpek,sort_length)) != -1 &&
         error != 0);

  lastbuff->count=count;
  if (to_file)
    lastbuff->file_pos=to_start_filepos;
err:
  delete_queue(&queue);
  DBUG_RETURN(error);
} /* merge_buffers */


        /* Do a merge to output-file (save only positions) */

static int NEAR_F
merge_index(MI_SORT_PARAM *info, uint keys, uchar **sort_keys,
            BUFFPEK *buffpek, int maxbuffer, IO_CACHE *tempfile)
{
  DBUG_ENTER("merge_index");
  if (merge_buffers(info,keys,tempfile,(IO_CACHE*) 0,sort_keys,buffpek,buffpek,
                    buffpek+maxbuffer))
    DBUG_RETURN(1); /* purecov: inspected */
  DBUG_RETURN(0);
} /* merge_index */


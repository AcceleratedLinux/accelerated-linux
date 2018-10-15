/* Copyright (C) 2000 MySQL AB

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
  This functions is handle keyblock cacheing for NISAM, MISAM and PISAM
  databases.
  One cache can handle many files. Every different blocksize has it owns
  set of buffers that are allocated from block_mem.
  init_key_cache() should be used to init cache handler.
 */

#include "mysys_priv.h"
#include "my_static.h"
#include <m_string.h>
#include <errno.h>

#if defined(MSDOS) && !defined(M_IC80386)
	/* We nead much memory */
#undef my_malloc_lock
#undef my_free_lock
#define my_malloc_lock(A,B)	halloc((long) (A/IO_SIZE),IO_SIZE)
#define my_free_lock(A,B)	hfree(A)
#endif

/* size of map to be used to find changed files */

#define CHANGED_BLOCKS_HASH	128	/* Must be power of 2 */
#define CHANGED_BLOCKS_MASK	(CHANGED_BLOCKS_HASH-1)
#define FLUSH_CACHE 		2000	/* Sort this many blocks at once */

typedef struct sec_link {
  struct sec_link *next_hash,**prev_hash;/* Blocks linked acc. to hash-value */
  struct sec_link *next_used,*prev_used;
  struct sec_link *next_changed,**prev_changed;
  File file;
  my_off_t diskpos;
  byte *buffer;
  my_bool changed;
} SEC_LINK;


static SEC_LINK *find_key_block(int file,my_off_t filepos,int *error);
static int flush_all_key_blocks();

	/* static variables in this file */
static SEC_LINK *_my_block_root,**_my_hash_root,
		*_my_used_first,*_my_used_last;
static int	_my_disk_blocks;
static uint	_my_disk_blocks_used, _my_hash_blocks;
static uint	key_cache_shift;
ulong		_my_blocks_used,_my_blocks_changed;
ulong		_my_cache_w_requests,_my_cache_write,_my_cache_r_requests,
		_my_cache_read;
uint		key_cache_block_size=DEFAULT_KEYCACHE_BLOCK_SIZE;
static byte	HUGE_PTR *_my_block_mem;
static SEC_LINK *changed_blocks[CHANGED_BLOCKS_HASH];
static SEC_LINK *file_blocks[CHANGED_BLOCKS_HASH];
#ifndef DBUG_OFF
static my_bool	_my_printed;
#endif


	/* Init of disk_buffert */
	/* Returns blocks in use */
	/* ARGSUSED */

int init_key_cache(ulong use_mem)
{
  uint blocks,length;
  DBUG_ENTER("init_key_cache");

  if (key_cache_inited && _my_disk_blocks > 0)
  {
    DBUG_PRINT("warning",("key cache already in use")); /* purecov: inspected */
    DBUG_RETURN(0); /* purecov: inspected */
  }
  if (! key_cache_inited)
  {
    key_cache_inited=TRUE;
    _my_disk_blocks= -1;
    key_cache_shift=my_bit_log2(key_cache_block_size);
    DBUG_PRINT("info",("key_cache_block_size: %u  key_cache_shift: %u",
		       key_cache_block_size, key_cache_shift));
#ifndef DBUG_OFF
    _my_printed=0;
#endif
  }

  blocks= (uint) (use_mem/(sizeof(SEC_LINK)+sizeof(SEC_LINK*)*5/4+
			   key_cache_block_size));
  /* No use to have very few blocks */
  if (blocks >= 8 && _my_disk_blocks < 0)
  {
    for (;;)
    {
      /* Set my_hash_blocks to the next bigger 2 power */
      _my_hash_blocks=(uint) 1 << (my_bit_log2(blocks*5/4)+1);
      while ((length=(uint) blocks*sizeof(SEC_LINK)+
	      sizeof(SEC_LINK*)*_my_hash_blocks)+
	     ((ulong) blocks << key_cache_shift) >
	     use_mem)
	blocks--;
      if ((_my_block_mem=my_malloc_lock((ulong) blocks << key_cache_shift,
					MYF(0))))
      {
	if ((_my_block_root=(SEC_LINK*) my_malloc((uint) length,MYF(0))) != 0)
	  break;
	my_free_lock(_my_block_mem,MYF(0));
      }
      if (blocks < 8)
	goto err;
      blocks=blocks/4*3;
    }
    _my_disk_blocks=(int) blocks;
    _my_hash_root= (SEC_LINK**) (_my_block_root+blocks);
    bzero((byte*) _my_hash_root,_my_hash_blocks*sizeof(SEC_LINK*));
    _my_used_first=_my_used_last=0;
    _my_blocks_used=_my_disk_blocks_used=_my_blocks_changed=0;
    _my_cache_w_requests=_my_cache_r_requests=_my_cache_read=_my_cache_write=0;
    DBUG_PRINT("exit",("disk_blocks: %d  block_root: %lx  _my_hash_blocks: %d  hash_root: %lx",
		       _my_disk_blocks,_my_block_root,_my_hash_blocks,
		       _my_hash_root));
  }
  bzero((gptr) changed_blocks,sizeof(changed_blocks[0])*CHANGED_BLOCKS_HASH);
  bzero((gptr) file_blocks,sizeof(file_blocks[0])*CHANGED_BLOCKS_HASH);
  DBUG_RETURN((int) blocks);

err:
  my_errno=ENOMEM;
  DBUG_RETURN(0);
} /* init_key_cache */


/*
  Resize the key cache

  SYNOPSIS
    resize_key_cache()
    use_mem		Bytes to use for new key cache

  RETURN VALUES
    0	Error
    #	number of blocks in key cache
*/


int resize_key_cache(ulong use_mem)
{
  int block;
  pthread_mutex_lock(&THR_LOCK_keycache);
  if (flush_all_key_blocks())
  {
    /* TODO: If this happens, we should write a warning in the log file ! */
    pthread_mutex_unlock(&THR_LOCK_keycache);
    return 0;
  }
  end_key_cache();
  /* The following will work even if memory is 0 */
  block=init_key_cache(use_mem);
  pthread_mutex_unlock(&THR_LOCK_keycache);  
  return block;
}


	/* Remove key_cache from memory */

void end_key_cache(void)
{
  DBUG_ENTER("end_key_cache");
  if (! _my_blocks_changed)
  {
    if (_my_disk_blocks > 0)
    {
      my_free_lock((gptr) _my_block_mem,MYF(0));
      my_free((gptr) _my_block_root,MYF(0));
      _my_disk_blocks= -1;
    }
  }
  key_cache_inited=0;
  _my_hash_blocks=_my_blocks_used=0;
  DBUG_PRINT("status",
	     ("used: %d  changed: %d  w_requests: %ld  writes: %ld  r_requests: %ld  reads: %ld",
	      _my_blocks_used,_my_blocks_changed,_my_cache_w_requests,
	      _my_cache_write,_my_cache_r_requests,_my_cache_read));
  DBUG_VOID_RETURN;
} /* end_key_cache */


static inline void link_into_file_blocks(SEC_LINK *next, int file)
{
  reg1 SEC_LINK **ptr= &file_blocks[(uint) file & CHANGED_BLOCKS_MASK];
  next->prev_changed= ptr;
  if ((next->next_changed= *ptr))
    (*ptr)->prev_changed= &next->next_changed;
  *ptr=next;
}


static inline void relink_into_file_blocks(SEC_LINK *next, int file)
{
  reg1 SEC_LINK **ptr= &file_blocks[(uint) file & CHANGED_BLOCKS_MASK];
  if (next->next_changed)
    next->next_changed->prev_changed=next->prev_changed;
  *next->prev_changed=next->next_changed;
  next->prev_changed= ptr;
  if ((next->next_changed= *ptr))
    (*ptr)->prev_changed= &next->next_changed;
  *ptr=next;
}

static inline void link_changed_to_file(SEC_LINK *next,int file)
{
  reg1 SEC_LINK **ptr= &file_blocks[(uint) file & CHANGED_BLOCKS_MASK];
  if (next->next_changed)
    next->next_changed->prev_changed=next->prev_changed;
  *next->prev_changed=next->next_changed;
  next->prev_changed= ptr;
  if ((next->next_changed= *ptr))
    (*ptr)->prev_changed= &next->next_changed;
  *ptr=next;
  next->changed=0;
  _my_blocks_changed--;
}

static inline void link_file_to_changed(SEC_LINK *next)
{
  reg1 SEC_LINK **ptr= &changed_blocks[(uint) next->file & CHANGED_BLOCKS_MASK];
  if (next->next_changed)
    next->next_changed->prev_changed=next->prev_changed;
  *next->prev_changed=next->next_changed;
  next->prev_changed= ptr;
  if ((next->next_changed= *ptr))
    (*ptr)->prev_changed= &next->next_changed;
  *ptr=next;
  next->changed=1;
  _my_blocks_changed++;
}


#if !defined(DBUG_OFF) && !defined(EXTRA_DEBUG)
#define DBUG_OFF				/* This should work */
#endif

#ifndef DBUG_OFF
static void test_key_cache(const char *where, my_bool lock);
#endif


	/*
	** read a key_buffer
	** filepos must point at a even key_cache_block_size block
	** if return_buffer is set then the intern buffer is returned if
	** it can be used
	** Returns adress to where data is read
	*/

byte *key_cache_read(File file, my_off_t filepos, byte *buff, uint length,
		     uint block_length __attribute__((unused)),
		     int return_buffer __attribute__((unused)))
{
  reg1 SEC_LINK *next;
  int error=0;
  DBUG_ENTER("key_cache_read");
  DBUG_PRINT("enter", ("file %u, filepos %lu, length %u",
		       (uint) file, (ulong) filepos, length));

#ifndef THREAD
  if (block_length > key_cache_block_size)
    return_buffer=0;
#endif
  if (_my_disk_blocks > 0)
  {						/* We have key_cacheing */
    byte *start=buff;
    uint read_length;
    pthread_mutex_lock(&THR_LOCK_keycache);
    if (_my_disk_blocks <= 0)			/* Resize failed */
    {
      pthread_mutex_unlock(&THR_LOCK_keycache);
      goto no_key_cache;
    }
    do
    {
      _my_cache_r_requests++;
      read_length= (length > key_cache_block_size ? key_cache_block_size :
		    length);
      if (!(next=find_key_block(file,filepos,&error)))
      {
	pthread_mutex_unlock(&THR_LOCK_keycache);
	DBUG_RETURN ((byte*) 0);       		/* Got a fatal error */
      }
      if (error)
      {					/* Didn't find it in cache */
	if (my_pread(file,next->buffer,read_length,filepos,MYF(MY_NABP)))
	{
	  pthread_mutex_unlock(&THR_LOCK_keycache);
	  DBUG_RETURN((byte*) 0);
	}
	_my_cache_read++;
      }
#ifndef THREAD				/* buffer may be used a long time */
      if (return_buffer)
      {
	pthread_mutex_unlock(&THR_LOCK_keycache);
	DBUG_RETURN (next->buffer);
      }
#endif
      if (! (read_length & 511))
	bmove512(buff,next->buffer,read_length);
      else
	memcpy(buff,next->buffer,(size_t) read_length);
      buff+=read_length;
      filepos+=read_length;
    } while ((length-= read_length));
    pthread_mutex_unlock(&THR_LOCK_keycache);
    DBUG_RETURN(start);
  }

no_key_cache:
  _my_cache_r_requests++;
  _my_cache_read++;
  if (my_pread(file,(byte*) buff,length,filepos,MYF(MY_NABP)))
    error=1;
  DBUG_RETURN(error ? (byte*) 0 : buff);
} /* key_cache_read */


	/* write a key_buffer */
	/* We don't have to use pwrite because of write locking */
	/* buff must point at a even key_cache_block_size block */

int key_cache_write(File file, my_off_t filepos, byte *buff, uint length,
		    uint block_length  __attribute__((unused)),
		    int dont_write)
{
  reg1 SEC_LINK *next;
  int error=0;
  DBUG_ENTER("key_cache_write");
  DBUG_PRINT("enter", ("file %u, filepos %lu, length %u",
		       (uint) file, (ulong) filepos, length));

  if (!dont_write)
  {						/* Forced write of buffer */
    _my_cache_write++;
    if (my_pwrite(file,buff,length,filepos,MYF(MY_NABP | MY_WAIT_IF_FULL)))
      DBUG_RETURN(1);
  }

#if !defined(DBUG_OFF) && defined(EXTRA_DEBUG)
  DBUG_EXECUTE("check_keycache",test_key_cache("start of key_cache_write",1););
#endif
  if (_my_disk_blocks > 0)
  {						/* We have key_cacheing */
    uint read_length;
    pthread_mutex_lock(&THR_LOCK_keycache);
    if (_my_disk_blocks <= 0)			/* If resize failed */
    {
      pthread_mutex_unlock(&THR_LOCK_keycache);
      goto no_key_cache;
    }

    _my_cache_w_requests++;
    do
    {
      read_length= length > key_cache_block_size ? key_cache_block_size : length;
      if (!(next=find_key_block(file,filepos,&error)))
	goto end;				/* Fatal error */
      if (!dont_write)				/* If we wrote buff at start */
      {
	if (next->changed)			/* Unlink from changed list */
	  link_changed_to_file(next,next->file);
      }
      else if (!next->changed)
	link_file_to_changed(next);		/* Add to changed list */

      if (!(read_length & 511))
	bmove512(next->buffer,buff,read_length);
      else
	memcpy(next->buffer,buff,(size_t) read_length);
      buff+=read_length;
      filepos+=read_length;
    } while ((length-= read_length));
    error=0;
    pthread_mutex_unlock(&THR_LOCK_keycache);
    goto end;
  }

no_key_cache:
  if (dont_write)
  {						/* We must write, no cache */
    _my_cache_w_requests++;
    _my_cache_write++;
    if (my_pwrite(file,(byte*) buff,length,filepos,
		  MYF(MY_NABP | MY_WAIT_IF_FULL)))
      error=1;
  }

end:
#if !defined(DBUG_OFF) && defined(EXTRA_DEBUG)
  DBUG_EXECUTE("check_keycache",test_key_cache("end of key_cache_write",1););
#endif
  DBUG_RETURN(error);
} /* key_cache_write */


	/* Find block in cache */
	/* IF found sector and error is set then next->changed is cleared */

static SEC_LINK *find_key_block(int file, my_off_t filepos, int *error)
{
  reg1 SEC_LINK *next,**start;
  DBUG_ENTER("find_key_block");
  DBUG_PRINT("enter", ("file %u, filepos %lu",
		       (uint) file, (ulong) filepos));

#if !defined(DBUG_OFF) && defined(EXTRA_DEBUG)
  DBUG_EXECUTE("check_keycache2",test_key_cache("start of find_key_block",0););
#endif

  *error=0;
  next= *(start= &_my_hash_root[((ulong) (filepos >> key_cache_shift)+(ulong) file) &
				(_my_hash_blocks-1)]);
  while (next && (next->diskpos != filepos || next->file != file))
    next= next->next_hash;

  if (next)
  {						/* Found block */
    if (next != _my_used_last)
    {						/* Relink used-chain */
      if (next == _my_used_first)
	_my_used_first=next->next_used;
      else
      {
	next->prev_used->next_used = next->next_used;
	next->next_used->prev_used = next->prev_used;
      }
      next->prev_used=_my_used_last;
      _my_used_last->next_used=next;
    }
  }
  else
  {						/* New block */
    if (_my_disk_blocks_used+1 <= (uint) _my_disk_blocks)
    {						/* There are unused blocks */
      next= &_my_block_root[_my_blocks_used++]; /* Link in hash-chain */
      next->buffer=ADD_TO_PTR(_my_block_mem,
			      ((ulong) _my_disk_blocks_used << key_cache_shift),
			      byte*);
      /* link first in file_blocks */
      next->changed=0;
      link_into_file_blocks(next,file);
      _my_disk_blocks_used++;
      if (!_my_used_first)
	_my_used_first=next;
      if (_my_used_last)
	_my_used_last->next_used=next; /* Last in used-chain */
    }
    else
    {						/* Reuse old block */
      next= _my_used_first;
      if (next->changed)
      {
	if (my_pwrite(next->file,next->buffer,key_cache_block_size,
		      next->diskpos,
		      MYF(MY_NABP | MY_WAIT_IF_FULL)))
	{
	  *error=1;
	  return((SEC_LINK*) 0);
	}
	_my_cache_write++;
	link_changed_to_file(next,file);
      }
      else
      {
	if (next->file == -1)
	  link_into_file_blocks(next,file);
	else
	  relink_into_file_blocks(next,file);
      }
      if (next->prev_hash)			/* If in hash-link */
	if ((*next->prev_hash=next->next_hash) != 0) /* Remove from link */
	  next->next_hash->prev_hash= next->prev_hash;

      _my_used_last->next_used=next;
      _my_used_first=next->next_used;
    }
    if (*start)					/* Link in first in h.-chain */
      (*start)->prev_hash= &next->next_hash;
    next->next_hash= *start; next->prev_hash=start; *start=next;
    next->prev_used=_my_used_last;
    next->file=file;
    next->diskpos=filepos;
    *error=1;					/* Block wasn't in memory */
  }
  _my_used_last=next;
#if !defined(DBUG_OFF) && defined(EXTRA_DEBUG)
  DBUG_EXECUTE("check_keycache2",test_key_cache("end of find_key_block",0););
#endif
  DBUG_RETURN(next);
} /* find_key_block */


static void free_block(SEC_LINK *used)
{
  used->file= -1;
  used->changed=0;
  if (used != _my_used_first)			/* Relink used-chain */
  {
    if (used == _my_used_last)
      _my_used_last=used->prev_used;
    else
    {
      used->prev_used->next_used = used->next_used;
      used->next_used->prev_used = used->prev_used;
    }
    used->next_used=_my_used_first;
    used->next_used->prev_used=used;
    _my_used_first=used;
  }
  if ((*used->prev_hash=used->next_hash))	/* Relink hash-chain */
    used->next_hash->prev_hash= used->prev_hash;
  if (used->next_changed)			/* Relink changed/file list */
    used->next_changed->prev_changed=used->prev_changed;
  *used->prev_changed=used->next_changed;
  used->prev_hash=0; used->next_hash=0;		/* Safety */
}


	/* Flush all changed blocks to disk. Free used blocks if requested */

static int cmp_sec_link(SEC_LINK **a, SEC_LINK **b)
{
  return (((*a)->diskpos < (*b)->diskpos) ? -1 :
	  ((*a)->diskpos > (*b)->diskpos) ? 1 : 0);
}


static int flush_cached_blocks(File file, SEC_LINK **cache, uint count)
{
  uint last_errno=0;
  qsort((byte*) cache, count, sizeof(*cache), (qsort_cmp) cmp_sec_link);
  for ( ; count-- ; cache++)
  {
    if (my_pwrite(file,(*cache)->buffer,key_cache_block_size,
		  (*cache)->diskpos,
		  MYF(MY_NABP | MY_WAIT_IF_FULL)))
    {
      if (!last_errno)
	last_errno=errno ? errno : -1;
    }
  }
  return last_errno;
}


static int flush_key_blocks_int(File file, enum flush_type type)
{
  int error=0,last_errno=0;
  uint count=0;
  SEC_LINK *cache_buff[FLUSH_CACHE],**cache,**pos,**end;
  SEC_LINK *used,*next;
  DBUG_ENTER("flush_key_blocks_int");
  DBUG_PRINT("enter",("file: %d  blocks_used: %d  blocks_changed: %d",
		      file,_my_blocks_used,_my_blocks_changed));

  cache=cache_buff;				/* If no key cache */
  if (_my_disk_blocks > 0 &&
      (!my_disable_flush_key_blocks || type != FLUSH_KEEP))
  {
#if !defined(DBUG_OFF) && defined(EXTRA_DEBUG)
    DBUG_EXECUTE("check_keycache",test_key_cache("start of flush_key_blocks",0););
#endif
    if (type != FLUSH_IGNORE_CHANGED)
    {
      /* Count how many key blocks we have to cache to be able to
	 write everything with so few seeks as possible */

      for (used=changed_blocks[(uint) file & CHANGED_BLOCKS_MASK];
	   used ;
	   used=used->next_changed)
      {
	if (used->file == file)
	  count++;
      }
      /* Only allocate a new buffer if its bigger than the one we have */
      if (count > FLUSH_CACHE)
      {
	if (!(cache=(SEC_LINK**) my_malloc(sizeof(SEC_LINK*)*count,MYF(0))))
        {
	  cache=cache_buff;		/* Fall back to safe buffer */
	  count=FLUSH_CACHE;
        }
      }
    }

    /* Go through the keys and write them to buffer to be flushed */
    end=(pos=cache)+count;
    for (used=changed_blocks[(uint) file & CHANGED_BLOCKS_MASK];
	 used ;
	 used=next)
    {
      next=used->next_changed;
      if (used->file == file)
      {
	if (type != FLUSH_IGNORE_CHANGED)
	{
	  if (pos == end)
	  {
	    if ((error=flush_cached_blocks(file, cache, count)))
	      last_errno=error;
	    pos=cache;
	  }
	  *pos++=used;
	  _my_cache_write++;
	}
	if (type != FLUSH_KEEP && type != FLUSH_FORCE_WRITE)
	{
	  /* This will not destroy position or data */
	  _my_blocks_changed--;
	  free_block(used);
	}
	else
	  link_changed_to_file(used,file);
      }
    }
    if (pos != cache)
    {
      if ((error=flush_cached_blocks(file, cache, (uint) (pos-cache))))
	last_errno=error;
    }
    /* The following happens very seldom */
    if (type != FLUSH_KEEP && type != FLUSH_FORCE_WRITE)
    {
      for (used=file_blocks[(uint) file & CHANGED_BLOCKS_MASK];
	   used ;
	   used=next)
      {
	next=used->next_changed;
	if (used->file == file && (!used->changed ||
				   type == FLUSH_IGNORE_CHANGED))
	  free_block(used);
      }
    }
#ifndef DBUG_OFF
    DBUG_EXECUTE("check_keycache",test_key_cache("end of flush_key_blocks",0););
#endif
  }
  if (cache != cache_buff)
    my_free((gptr) cache,MYF(0));
  if (last_errno)
    errno=last_errno;				/* Return first error */
  DBUG_RETURN(last_errno != 0);
}


/*
  Flush all blocks for a specific file to disk

  SYNOPSIS
    flush_all_key_blocks()
    file	File descriptor
    type	Type of flush operation

  RETURN VALUES
    0		Ok
    1		Error
*/

int flush_key_blocks(File file, enum flush_type type)
{
  int res;
  pthread_mutex_lock(&THR_LOCK_keycache);
  res=flush_key_blocks_int(file, type);
  pthread_mutex_unlock(&THR_LOCK_keycache);
  return res;
}


/*
  Flush all blocks in the key cache to disk

  SYNOPSIS
    flush_all_key_blocks()

  NOTE
    We must have a lock on THR_LOCK_keycache before calling this function

  RETURN VALUES
    0		Ok
    1		Error
*/


static int flush_all_key_blocks()
{
  SEC_LINK **block, **end;
  for (block= changed_blocks, end= block+CHANGED_BLOCKS_HASH;
       block < end;
       block++
       )
  {
    while (*block)
    {
      if (flush_key_blocks_int((*block)->file, FLUSH_RELEASE))
	return 1;
    }
  }
  return 0;
}


#ifndef DBUG_OFF

	/* Test if disk-cache is ok */

static void test_key_cache(const char *where, my_bool lock)
{
  reg1 uint i,error;
  ulong found,changed;
  SEC_LINK *pos,**prev;

  if (lock)
  {
    pthread_mutex_lock(&THR_LOCK_keycache);
    if (_my_disk_blocks <= 0)			/* No active key cache */
    {
      pthread_mutex_unlock(&THR_LOCK_keycache);
      return;
    }
  }
  found=error=0;
  for (i= 0 ; i < _my_hash_blocks ; i++)
  {

    for (pos= *(prev= &_my_hash_root[i]) ;
	 pos && found < _my_blocks_used+2 ;
	 found++, pos= *(prev= &pos->next_hash))
    {
      if (prev != pos->prev_hash)
      {
	error=1;
	DBUG_PRINT("error",
		   ("hash: %d  pos: %lx  : prev: %lx  !=  pos->prev: %lx",
		    i,(ulong) pos,(ulong) prev,(ulong) pos->prev_hash));
      }

      if (((pos->diskpos >> key_cache_shift)+pos->file) % _my_hash_blocks != i)
      {
	DBUG_PRINT("error",("hash: %d  pos: %lx  : Wrong disk_buffer %ld",
			    i,(ulong) pos,(ulong) pos->diskpos));
	error=1;
      }
    }
  }
  if (found > _my_blocks_used)
  {
    DBUG_PRINT("error",("Found too many hash_pointers"));
    error=1;
  }
  if (error && !_my_printed)
  {						/* Write all hash-pointers */
    _my_printed=1;
    for (i=0 ; i < _my_hash_blocks ; i++)
    {
      DBUG_PRINT("loop",("hash: %d  _my_hash_root: %lx",i,&_my_hash_root[i]));
      pos= _my_hash_root[i]; found=0;
      while (pos && found < 10)
      {
	DBUG_PRINT("loop",("pos: %lx  prev: %lx  next: %lx  file: %d  disk_buffer: %ld", (ulong) pos, (ulong) pos->prev_hash, (ulong) pos->next_hash, (ulong) pos->file, (ulong) pos->diskpos));
	found++; pos= pos->next_hash;
      }
    }
  }

  found=changed=0;

  if ((pos=_my_used_first))
  {
    while (pos != _my_used_last && found < _my_blocks_used+2)
    {
      found++;
      if (pos->changed)
	changed++;
      if (pos->next_used->prev_used != pos)
      {
	DBUG_PRINT("error",("pos: %lx  next_used: %lx  next_used->prev: %lx",
			    (ulong) pos,
			    (ulong) pos->next_used,
			    (ulong) pos->next_used->prev_hash));
	error=1;
      }
      pos=pos->next_used;
    }
    found++;
    if (pos->changed)
      changed++;
  }
  if (found != _my_blocks_used)
  {
    DBUG_PRINT("error",("Found %lu of %lu keyblocks",found,_my_blocks_used));
    error=1;
  }

  for (i= 0 ; i < CHANGED_BLOCKS_HASH ; i++)
  {
    found=0;
    prev= &changed_blocks[i];
    for (pos= *prev ;  pos && found < _my_blocks_used+2; pos=pos->next_changed)
    {
      found++;
      if (pos->prev_changed != prev)
      {
	DBUG_PRINT("error",("changed_block list %d doesn't point backwards properly",i));
	error=1;
      }
      prev= &pos->next_changed;
      if (((uint) pos->file & CHANGED_BLOCKS_MASK) != i)
      {
	DBUG_PRINT("error",("Wrong file %d in changed blocks: %d",pos->file,i));
	error=1;
      }
      changed--;
    }
    if (pos)
    {
      DBUG_PRINT("error",("changed_blocks %d has recursive link",i));
      error=1;
    }

    found=0;
    prev= &file_blocks[i];
    for (pos= *prev ;  pos && found < _my_blocks_used+2; pos=pos->next_changed)
    {
      found++;
      if (pos->prev_changed != prev)
      {
	DBUG_PRINT("error",("file_block list %d doesn't point backwards properly",i));
	error=1;
      }
      prev= &pos->next_changed;
      if (((uint) pos->file & CHANGED_BLOCKS_MASK) != i)
      {
	DBUG_PRINT("error",("Wrong file %d in file_blocks: %d",pos->file,i));
	error=1;
      }
    }
    if (pos)
    {
      DBUG_PRINT("error",("File_blocks %d has recursive link",i));
      error=1;
    }
  }
  if (changed != 0)
  {
    DBUG_PRINT("error",("Found %lu blocks that wasn't in changed blocks",
			changed));
    error=1;
  }
  if (error)
    DBUG_PRINT("error",("Found error at %s",where));
  if (lock)
    pthread_mutex_unlock(&THR_LOCK_keycache);
  return;
} /* test_key_cache */
#endif

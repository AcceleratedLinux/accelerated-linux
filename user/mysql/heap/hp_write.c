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

/* Write a record to heap-databas */

#include "heapdef.h"
#ifdef __WIN__
#include <fcntl.h>
#endif

#define LOWFIND 1
#define LOWUSED 2
#define HIGHFIND 4
#define HIGHUSED 8

static byte *next_free_record_pos(HP_SHARE *info);
static HASH_INFO *_hp_find_free_hash(HP_SHARE *info, HP_BLOCK *block,
				     ulong records);

int heap_write(HP_INFO *info, const byte *record)
{
  uint key;
  byte *pos;
  HP_SHARE *share=info->s;
  DBUG_ENTER("heap_write");

#ifndef DBUG_OFF
  if (info->mode & O_RDONLY)
  {
    DBUG_RETURN(my_errno=EACCES);
  }
#endif
  if (!(pos=next_free_record_pos(share)))
    DBUG_RETURN(my_errno);
  share->changed=1;

  for (key=0 ; key < share->keys ; key++)
  {
    if (_hp_write_key(share,share->keydef+key,record,pos))
      goto err;
  }

  memcpy(pos,record,(size_t) share->reclength);
  pos[share->reclength]=1;		/* Mark record as not deleted */
  if (++share->records == share->blength)
    share->blength+= share->blength;
  info->current_ptr=pos;
  info->current_hash_ptr=0;
  info->update|=HA_STATE_AKTIV;
#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
  DBUG_EXECUTE("check_heap",heap_check_heap(info, 0););
#endif
  DBUG_RETURN(0);

err:
  DBUG_PRINT("info",("Duplicate key: %d",key));
  info->errkey= key;
  do
  {
    if (_hp_delete_key(info,share->keydef+key,record,pos,0))
      break;
  } while (key-- > 0);

  share->deleted++;
  *((byte**) pos)=share->del_link;
  share->del_link=pos;
  pos[share->reclength]=0;			/* Record deleted */

  DBUG_RETURN(my_errno);
} /* heap_write */


	/* Find where to place new record */

static byte *next_free_record_pos(HP_SHARE *info)
{
  int block_pos;
  byte *pos;
  ulong length;
  DBUG_ENTER("next_free_record_pos");

  if (info->del_link)
  {
    pos=info->del_link;
    info->del_link= *((byte**) pos);
    info->deleted--;
    DBUG_PRINT("exit",("Used old position: %lx",pos));
    DBUG_RETURN(pos);
  }
  if (!(block_pos=(info->records % info->block.records_in_block)))
  {
    if (info->records > info->max_records && info->max_records)
    {
      my_errno=HA_ERR_RECORD_FILE_FULL;
      DBUG_RETURN(NULL);
    }
    if (_hp_get_new_block(&info->block,&length))
      DBUG_RETURN(NULL);
    info->data_length+=length;
  }
  DBUG_PRINT("exit",("Used new position: %lx",
		     (byte*) info->block.level_info[0].last_blocks+block_pos*
		     info->block.recbuffer));
  DBUG_RETURN((byte*) info->block.level_info[0].last_blocks+
	      block_pos*info->block.recbuffer);
}


	/* Write a hash-key to the hash-index */

int _hp_write_key(register HP_SHARE *info, HP_KEYDEF *keyinfo,
		  const byte *record, byte *recpos)
{
  int flag;
  ulong halfbuff,hashnr,first_index;
  byte *ptr_to_rec,*ptr_to_rec2;
  HASH_INFO *empty,*gpos,*gpos2,*pos;
  DBUG_ENTER("hp_write_key");

  LINT_INIT(gpos); LINT_INIT(gpos2);
  LINT_INIT(ptr_to_rec); LINT_INIT(ptr_to_rec2);

  flag=0;
  if (!(empty= _hp_find_free_hash(info,&keyinfo->block,info->records)))
    DBUG_RETURN(-1);				/* No more memory */
  halfbuff= (long) info->blength >> 1;
  pos=	 hp_find_hash(&keyinfo->block,(first_index=info->records-halfbuff));

  if (pos != empty)				/* If some records */
  {
    do
    {
      hashnr=_hp_rec_hashnr(keyinfo,pos->ptr_to_rec);
      if (flag == 0)				/* First loop; Check if ok */
	if (_hp_mask(hashnr,info->blength,info->records) != first_index)
	  break;
      if (!(hashnr & halfbuff))
      {						/* Key will not move */
	if (!(flag & LOWFIND))
	{
	  if (flag & HIGHFIND)
	  {
	    flag=LOWFIND | HIGHFIND;
	    /* key shall be moved to the current empty position */
	    gpos=empty;
	    ptr_to_rec=pos->ptr_to_rec;
	    empty=pos;				/* This place is now free */
	  }
	  else
	  {
	    flag=LOWFIND | LOWUSED;		/* key isn't changed */
	    gpos=pos;
	    ptr_to_rec=pos->ptr_to_rec;
	  }
	}
	else
	{
	  if (!(flag & LOWUSED))
	  {
	    /* Change link of previous LOW-key */
	    gpos->ptr_to_rec=ptr_to_rec;
	    gpos->next_key=pos;
	    flag= (flag & HIGHFIND) | (LOWFIND | LOWUSED);
	  }
	  gpos=pos;
	  ptr_to_rec=pos->ptr_to_rec;
	}
      }
      else
      {						/* key will be moved */
	if (!(flag & HIGHFIND))
	{
	  flag= (flag & LOWFIND) | HIGHFIND;
	  /* key shall be moved to the last (empty) position */
	  gpos2 = empty; empty=pos;
	  ptr_to_rec2=pos->ptr_to_rec;
	}
	else
	{
	  if (!(flag & HIGHUSED))
	  {
	    /* Change link of previous hash-key and save */
	    gpos2->ptr_to_rec=ptr_to_rec2;
	    gpos2->next_key=pos;
	    flag= (flag & LOWFIND) | (HIGHFIND | HIGHUSED);
	  }
	  gpos2=pos;
	  ptr_to_rec2=pos->ptr_to_rec;
	}
      }
    }
    while ((pos=pos->next_key));

    if ((flag & (LOWFIND | LOWUSED)) == LOWFIND)
    {
      gpos->ptr_to_rec=ptr_to_rec;
      gpos->next_key=0;
    }
    if ((flag & (HIGHFIND | HIGHUSED)) == HIGHFIND)
    {
      gpos2->ptr_to_rec=ptr_to_rec2;
      gpos2->next_key=0;
    }
  }
  /* Check if we are at the empty position */

  pos=hp_find_hash(&keyinfo->block,_hp_mask(_hp_rec_hashnr(keyinfo,record),
					    info->blength,info->records+1));
  if (pos == empty)
  {
    pos->ptr_to_rec=recpos;
    pos->next_key=0;
  }
  else
  {
    /* Check if more records in same hash-nr family */
    empty[0]=pos[0];
    gpos=hp_find_hash(&keyinfo->block,
		      _hp_mask(_hp_rec_hashnr(keyinfo,pos->ptr_to_rec),
			       info->blength,info->records+1));
    if (pos == gpos)
    {
      pos->ptr_to_rec=recpos;
      pos->next_key=empty;
    }
    else
    {
      pos->ptr_to_rec=recpos;
      pos->next_key=0;
      _hp_movelink(pos,gpos,empty);
    }

    /* Check if duplicated keys */
    if ((keyinfo->flag & HA_NOSAME) && pos == gpos &&
	(!(keyinfo->flag & HA_NULL_PART_KEY) ||
	 !hp_if_null_in_key(keyinfo, record)))
    {
      pos=empty;
      do
      {
	if (! _hp_rec_key_cmp(keyinfo,record,pos->ptr_to_rec))
	{
	  DBUG_RETURN(my_errno=HA_ERR_FOUND_DUPP_KEY);
	}
      } while ((pos=pos->next_key));
    }
  }
  DBUG_RETURN(0);
}

	/* Returns ptr to block, and allocates block if neaded */

static HASH_INFO *_hp_find_free_hash(HP_SHARE *info,
				     HP_BLOCK *block, ulong records)
{
  uint block_pos;
  ulong length;

  if (records < block->last_allocated)
    return hp_find_hash(block,records);
  if (!(block_pos=(records % block->records_in_block)))
  {
    if (_hp_get_new_block(block,&length))
      return(NULL);
    info->index_length+=length;
  }
  block->last_allocated=records+1;
  return((HASH_INFO*) ((byte*) block->level_info[0].last_blocks+
		       block_pos*block->recbuffer));
}

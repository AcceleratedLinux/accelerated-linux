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

/* functions on blocks; Keys and records are saved in blocks */

#include "heapdef.h"

	/* Find record according to record-position */

byte *_hp_find_block(HP_BLOCK *block, ulong pos)
{
  reg1 int i;
  reg3 HP_PTRS *ptr;

  for (i=block->levels-1, ptr=block->root ; i > 0 ; i--)
  {
    ptr=(HP_PTRS*)ptr->blocks[pos/block->level_info[i].records_under_level];
    pos%=block->level_info[i].records_under_level;
  }
  return (byte*) ptr+ pos*block->recbuffer;
}


	/* get one new block-of-records. Alloc ptr to block if neaded */
	/* Interrupts are stopped to allow ha_panic in interrupts */

int _hp_get_new_block(HP_BLOCK *block, ulong *alloc_length)
{
  reg1 uint i,j;
  HP_PTRS *root;

  for (i=0 ; i < block->levels ; i++)
    if (block->level_info[i].free_ptrs_in_block)
      break;

  *alloc_length=sizeof(HP_PTRS)*i+block->records_in_block* block->recbuffer;
  if (!(root=(HP_PTRS*) my_malloc(*alloc_length,MYF(0))))
    return 1;

  if (i == 0)
  {
    block->levels=1;
    block->root=block->level_info[0].last_blocks=root;
  }
  else
  {
    dont_break();		/* Dont allow SIGHUP or SIGINT */
    if ((uint) i == block->levels)
    {
      block->levels=i+1;
      block->level_info[i].free_ptrs_in_block=HP_PTRS_IN_NOD-1;
      ((HP_PTRS**) root)[0]= block->root;
      block->root=block->level_info[i].last_blocks= root++;
    }
    block->level_info[i].last_blocks->
      blocks[HP_PTRS_IN_NOD - block->level_info[i].free_ptrs_in_block--]=
	(byte*) root;

    for (j=i-1 ; j >0 ; j--)
    {
      block->level_info[j].last_blocks= root++;
      block->level_info[j].last_blocks->blocks[0]=(byte*) root;
      block->level_info[j].free_ptrs_in_block=HP_PTRS_IN_NOD-1;
    }
    block->level_info[0].last_blocks= root;
    allow_break();		/* Allow SIGHUP & SIGINT */
  }
  return 0;
}


	/* free all blocks under level */

byte *_hp_free_level(HP_BLOCK *block, uint level, HP_PTRS *pos, byte *last_pos)
{
  int i,max_pos;
  byte *next_ptr;

  if (level == 1)
    next_ptr=(byte*) pos+block->recbuffer;
  else
  {
    max_pos= (block->level_info[level-1].last_blocks == pos) ?
      HP_PTRS_IN_NOD - block->level_info[level-1].free_ptrs_in_block :
    HP_PTRS_IN_NOD;

    next_ptr=(byte*) (pos+1);
    for (i=0 ; i < max_pos ; i++)
      next_ptr=_hp_free_level(block,level-1,
			      (HP_PTRS*) pos->blocks[i],next_ptr);
  }
  if ((byte*) pos != last_pos)
  {
    my_free((gptr) pos,MYF(0));
    return last_pos;
  }
  return next_ptr;			/* next memory position */
}

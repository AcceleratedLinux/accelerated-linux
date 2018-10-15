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

/* Read a record with random-access. The position to the record must
   get by N_INFO. The next record can be read with pos= -1 */


#include "isamdef.h"

/*
	   If filepos == NI_POS_ERROR, read next
	   Returns:
	   0 = Ok.
	   1 = Row was deleted
	  -1 = EOF (check errno to verify)
*/

int nisam_rrnd(N_INFO *info, byte *buf, register ulong filepos)
{
  int skipp_deleted_blocks;
  DBUG_ENTER("nisam_rrnd");

  skipp_deleted_blocks=0;

  if (filepos == NI_POS_ERROR)
  {
    skipp_deleted_blocks=1;
    if (info->lastpos == NI_POS_ERROR)	/* First read ? */
      filepos= info->s->pack.header_length;	/* Read first record */
    else
      filepos= info->nextpos;
  }

  info->lastinx= -1;				/* Can't forward or backward */
  /* Init all but update-flag */
  info->update&= (HA_STATE_CHANGED | HA_STATE_ROW_CHANGED);

  if (info->opt_flag & WRITE_CACHE_USED && flush_io_cache(&info->rec_cache))
    DBUG_RETURN(my_errno);

  DBUG_RETURN ((*info->s->read_rnd)(info,buf,filepos,skipp_deleted_blocks));
}

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

/* open a MyISAM MERGE table */

#include "myrg_def.h"
#include <stddef.h>
#include <errno.h>
#ifdef VMS
#include "mrg_static.c"
#endif

/*
	open a MyISAM MERGE table
	if handle_locking is 0 then exit with error if some table is locked
	if handle_locking is 1 then wait if table is locked
*/


MYRG_INFO *myrg_open(const char *name, int mode, int handle_locking)
{
  int save_errno,i,errpos;
  uint files,dir_length,length,key_parts;
  ulonglong file_offset;
  char name_buff[FN_REFLEN*2],buff[FN_REFLEN],*end;
  MYRG_INFO info,*m_info;
  File fd;
  IO_CACHE file;
  MI_INFO *isam,*last_isam;
  DBUG_ENTER("myrg_open");

  LINT_INIT(last_isam);
  LINT_INIT(m_info);
  isam=0;
  errpos=files=0;
  bzero((gptr) &info,sizeof(info));
  bzero((char*) &file,sizeof(file));
  if ((fd=my_open(fn_format(name_buff,name,"",MYRG_NAME_EXT,4),
		  O_RDONLY | O_SHARE,MYF(0))) < 0 ||
      init_io_cache(&file, fd, IO_SIZE, READ_CACHE, 0, 0,
		    MYF(MY_WME | MY_NABP)))
    goto err;
  errpos=1;
  dir_length=dirname_part(name_buff,name);
  info.reclength=0;
  while ((length=my_b_gets(&file,buff,FN_REFLEN-1)))
  {
    if ((end=buff+length)[-1] == '\n')
      end[-1]='\0';
    if (!buff[0])
      continue;		/* Skip empty lines */
    if (buff[0] == '#')
    {
      if( !strncmp(buff+1,"INSERT_METHOD=",14))
      {			/* Lookup insert method */
	int tmp=find_type(buff+15,&merge_insert_method,2);
	info.merge_insert_method = (uint) (tmp >= 0 ? tmp : 0);
      }
      continue;		/* Skip comments */
    }

    if (!test_if_hard_path(buff))
    {
      VOID(strmake(name_buff+dir_length,buff,
                   sizeof(name_buff)-1-dir_length));
      VOID(cleanup_dirname(buff,name_buff));
    }
    if (!(isam=mi_open(buff,mode,(handle_locking?HA_OPEN_WAIT_IF_LOCKED:0))))
	goto err;
    files++;
    last_isam=isam;
    if (info.reclength && info.reclength != isam->s->base.reclength)
    {
      my_errno=HA_ERR_WRONG_MRG_TABLE_DEF;
      goto err;
    }
    info.reclength=isam->s->base.reclength;
  }
  key_parts=(isam ? isam->s->base.key_parts : 0);
  if (!(m_info= (MYRG_INFO*) my_malloc(sizeof(MYRG_INFO)+
                                       files*sizeof(MYRG_TABLE)+
                                       sizeof(long)*key_parts,
				       MYF(MY_WME))))
    goto err;
  *m_info=info;
  m_info->tables=files;
  if (files)
  {
    m_info->open_tables=(MYRG_TABLE *) (m_info+1);
    m_info->rec_per_key_part=(ulong *) (m_info->open_tables+files);
    bzero((char*) m_info->rec_per_key_part,sizeof(long)*key_parts);
  }
  else
  {
    m_info->open_tables=0;
    m_info->rec_per_key_part=0;
  }
  errpos=2;

  for (i=files ; i-- > 0 ; )
  {
    uint j;
    m_info->open_tables[i].table=isam;
    m_info->options|=isam->s->options;
    m_info->records+=isam->state->records;
    m_info->del+=isam->state->del;
    m_info->data_file_length+=isam->state->data_file_length;
    for (j=0; j < key_parts; j++)
      m_info->rec_per_key_part[j]+=isam->s->state.rec_per_key_part[j] / files;
    if (i)
      isam=(MI_INFO*) (isam->open_list.next->data);
  }
  /* Don't mark table readonly, for ALTER TABLE ... UNION=(...) to work */
  m_info->options&= ~(HA_OPTION_COMPRESS_RECORD | HA_OPTION_READ_ONLY_DATA);

  /* Fix fileinfo for easyer debugging (actually set by rrnd) */
  file_offset=0;
  for (i=0 ; (uint) i < files ; i++)
  {
    m_info->open_tables[i].file_offset=(my_off_t) file_offset;
    file_offset+=m_info->open_tables[i].table->state->data_file_length;
  }
  if (sizeof(my_off_t) == 4 && file_offset > (ulonglong) (ulong) ~0L)
  {
    my_errno=HA_ERR_RECORD_FILE_FULL;
    goto err;
  }
  m_info->keys=(files) ? m_info->open_tables->table->s->base.keys : 0;
  bzero((char*) &m_info->by_key,sizeof(m_info->by_key));

  /* this works ok if the table list is empty */
  m_info->end_table=m_info->open_tables+files;
  m_info->last_used_table=m_info->open_tables;

  VOID(my_close(fd,MYF(0)));
  end_io_cache(&file);
  m_info->open_list.data=(void*) m_info;
  pthread_mutex_lock(&THR_LOCK_open);
  myrg_open_list=list_add(myrg_open_list,&m_info->open_list);
  pthread_mutex_unlock(&THR_LOCK_open);
  DBUG_RETURN(m_info);

err:
  save_errno=my_errno;
  switch (errpos) {
  case 2:
    my_free((char*) m_info,MYF(0));
    /* Fall through */
  case 1:
    VOID(my_close(fd,MYF(0)));
    end_io_cache(&file);
    for (i=files ; i-- > 0 ; )
    {
      isam=last_isam;
      if (i)
	last_isam=(MI_INFO*) (isam->open_list.next->data);
      mi_close(isam);
    }
  }
  my_errno=save_errno;
  DBUG_RETURN (NULL);
}

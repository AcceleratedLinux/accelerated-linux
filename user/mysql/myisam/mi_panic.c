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

#include "fulltext.h"

	/* if flag == HA_PANIC_CLOSE then all misam files are closed */
	/* if flag == HA_PANIC_WRITE then all misam files are unlocked and
	   all changed data in single user misam is written to file */
	/* if flag == HA_PANIC_READ then all misam files that was locked when
	   mi_panic(HA_PANIC_WRITE) was done is locked. A mi_readinfo() is
	   done for all single user files to get changes in database */


int mi_panic(enum ha_panic_function flag)
{
  int error=0;
  LIST *list_element,*next_open;
  MI_INFO *info;
  DBUG_ENTER("mi_panic");

  pthread_mutex_lock(&THR_LOCK_myisam);
  for (list_element=myisam_open_list ; list_element ; list_element=next_open)
  {
    next_open=list_element->next;		/* Save if close */
    info=(MI_INFO*) list_element->data;
    switch (flag) {
    case HA_PANIC_CLOSE:
      pthread_mutex_unlock(&THR_LOCK_myisam);	/* Not exactly right... */
      if (mi_close(info))
	error=my_errno;
      pthread_mutex_lock(&THR_LOCK_myisam);
      break;
    case HA_PANIC_WRITE:		/* Do this to free databases */
#ifdef CANT_OPEN_FILES_TWICE
      if (info->s->options & HA_OPTION_READ_ONLY_DATA)
	break;
#endif
      if (flush_key_blocks(info->s->kfile,FLUSH_RELEASE))
	error=my_errno;
      if (info->opt_flag & WRITE_CACHE_USED)
	if (flush_io_cache(&info->rec_cache))
	  error=my_errno;
      if (info->opt_flag & READ_CACHE_USED)
      {
	if (flush_io_cache(&info->rec_cache))
	  error=my_errno;
	reinit_io_cache(&info->rec_cache,READ_CACHE,0,
		       (pbool) (info->lock_type != F_UNLCK),1);
      }
      if (info->lock_type != F_UNLCK && ! info->was_locked)
      {
	info->was_locked=info->lock_type;
	if (mi_lock_database(info,F_UNLCK))
	  error=my_errno;
      }
#ifdef CANT_OPEN_FILES_TWICE
      if (info->s->kfile >= 0 && my_close(info->s->kfile,MYF(0)))
	error = my_errno;
      if (info->dfile >= 0 && my_close(info->dfile,MYF(0)))
	error = my_errno;
      info->s->kfile=info->dfile= -1;	/* Files aren't open anymore */
      break;
#endif
    case HA_PANIC_READ:			/* Restore to before WRITE */
#ifdef CANT_OPEN_FILES_TWICE
      {					/* Open closed files */
	char name_buff[FN_REFLEN];
	if (info->s->kfile < 0)
	  if ((info->s->kfile= my_open(fn_format(name_buff,info->filename,"",
					      N_NAME_IEXT,4),info->mode,
				    MYF(MY_WME))) < 0)
	    error = my_errno;
	if (info->dfile < 0)
	{
	  if ((info->dfile= my_open(fn_format(name_buff,info->filename,"",
					      N_NAME_DEXT,4),info->mode,
				    MYF(MY_WME))) < 0)
	    error = my_errno;
	  info->rec_cache.file=info->dfile;
	}
      }
#endif
      if (info->was_locked)
      {
	if (mi_lock_database(info, info->was_locked))
	  error=my_errno;
	info->was_locked=0;
      }
      break;
    }
  }
  if (flag == HA_PANIC_CLOSE)
  {
    VOID(mi_log(0));				/* Close log if neaded */
    ft_free_stopwords();
  }
  pthread_mutex_unlock(&THR_LOCK_myisam);
  if (!error)
    DBUG_RETURN(0);
  DBUG_RETURN(my_errno=error);
} /* mi_panic */

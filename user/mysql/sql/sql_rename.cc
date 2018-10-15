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
  Atomic rename of table;  RENAME TABLE t1 to t2, tmp to t1 [,...]
*/

#include "mysql_priv.h"


static TABLE_LIST *rename_tables(THD *thd, TABLE_LIST *table_list,
				 bool skip_error);

/*
  Every second entry in the table_list is the original name and every
  second entry is the new name.
*/

bool mysql_rename_tables(THD *thd, TABLE_LIST *table_list)
{
  bool error= 1;
  TABLE_LIST *ren_table= 0;
  DBUG_ENTER("mysql_rename_tables");

  /*
    Avoid problems with a rename on a table that we have locked or
    if the user is trying to to do this in a transcation context
  */

  if (thd->locked_tables || thd->active_transaction())
  {
    my_error(ER_LOCK_OR_ACTIVE_TRANSACTION,MYF(0));
    DBUG_RETURN(1);
  }

  VOID(pthread_mutex_lock(&LOCK_open));
  if (lock_table_names(thd, table_list))
    goto err;
  
  error=0;
  if ((ren_table=rename_tables(thd,table_list,0)))
  {
    /* Rename didn't succeed;  rename back the tables in reverse order */
    TABLE_LIST *prev=0,*table;
    /* Reverse the table list */

    while (table_list)
    {
      TABLE_LIST *next=table_list->next;
      table_list->next=prev;
      prev=table_list;
      table_list=next;
    }
    table_list=prev;

    /* Find the last renamed table */
    for (table=table_list ;
	 table->next != ren_table ;
	 table=table->next->next) ;
    table=table->next->next;			// Skip error table
    /* Revert to old names */
    rename_tables(thd, table, 1);
    error= 1;
  }

  /* Lets hope this doesn't fail as the result will be messy */ 
  if (!error)
  {
    mysql_update_log.write(thd,thd->query,thd->query_length);
    if (mysql_bin_log.is_open())
    {
      Query_log_event qinfo(thd, thd->query, thd->query_length, 0);
      mysql_bin_log.write(&qinfo);
    }
    send_ok(&thd->net);
  }

  unlock_table_names(thd,table_list);

err:
  pthread_mutex_unlock(&LOCK_open);
  DBUG_RETURN(error);
}


/*
  Rename all tables in list; Return pointer to wrong entry if something goes
  wrong.  Note that the table_list may be empty!
*/

static TABLE_LIST *
rename_tables(THD *thd, TABLE_LIST *table_list, bool skip_error)
{
  TABLE_LIST *ren_table,*new_table;
  DBUG_ENTER("rename_tables");

  for (ren_table=table_list ; ren_table ; ren_table=new_table->next)
  {
    db_type table_type;
    char name[FN_REFLEN];
    new_table=ren_table->next;

    sprintf(name,"%s/%s/%s%s",mysql_data_home,
	    new_table->db,new_table->real_name,
	    reg_ext);
    if (!access(name,F_OK))
    {
      my_error(ER_TABLE_EXISTS_ERROR,MYF(0),name);
      DBUG_RETURN(ren_table);			// This can't be skipped
    }
    sprintf(name,"%s/%s/%s%s",mysql_data_home,
	    ren_table->db,ren_table->real_name,
	    reg_ext);
    if ((table_type=get_table_type(name)) == DB_TYPE_UNKNOWN)
    {
      my_error(ER_FILE_NOT_FOUND, MYF(0), name, my_errno);
      if (!skip_error)
	DBUG_RETURN(ren_table);
    }
    else if (mysql_rename_table(table_type,
				ren_table->db, ren_table->real_name,
				new_table->db, new_table->real_name))
    {
      if (!skip_error)
	DBUG_RETURN(ren_table);
    }
  }
  DBUG_RETURN(0);
}

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


/* Function with list databases, tables or fields */

#include "mysql_priv.h"
#include "sql_select.h"                         // For select_describe
#include "sql_acl.h"
#include "repl_failsafe.h"
#include <my_dir.h>

#ifdef HAVE_BERKELEY_DB
#include "ha_berkeley.h"			// For berkeley_show_logs
#endif

static const char *grant_names[]={
  "select","insert","update","delete","create","drop","reload","shutdown",
  "process","file","grant","references","index","alter"};

static TYPELIB grant_types = { sizeof(grant_names)/sizeof(char **),
                               "grant_types",
                               grant_names};

static int mysql_find_files(THD *thd,List<char> *files, const char *db,
                            const char *path, const char *wild, bool dir);

static int
store_create_info(THD *thd, TABLE *table, String *packet);

static void
append_identifier(THD *thd, String *packet, const char *name);

extern struct st_VioSSLAcceptorFd * ssl_acceptor_fd;

/****************************************************************************
** Send list of databases
** A database is a directory in the mysql_data_home directory
****************************************************************************/


int
mysqld_show_dbs(THD *thd,const char *wild)
{
  Item_string *field=new Item_string("",0);
  List<Item> field_list;
  char *end;
  List<char> files;
  char *file_name;
  DBUG_ENTER("mysqld_show_dbs");

  field->name=(char*) thd->alloc(20+ (wild ? (uint) strlen(wild)+4: 0));
  field->max_length=NAME_LEN;
  end=strmov(field->name,"Database");
  if (wild && wild[0])
    strxmov(end," (",wild,")",NullS);
  field_list.push_back(field);

  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);
  if (mysql_find_files(thd,&files,NullS,mysql_data_home,wild,1))
    DBUG_RETURN(1);
  List_iterator_fast<char> it(files);
  while ((file_name=it++))
  {
    if (thd->master_access & (DB_ACLS | SHOW_DB_ACL) ||
	acl_get(thd->host, thd->ip, (char*) &thd->remote.sin_addr,
		thd->priv_user, file_name) ||
	(grant_option && !check_grant_db(thd, file_name)))
    {
      thd->packet.length(0);
      net_store_data(&thd->packet, thd->variables.convert_set, file_name);
      if (my_net_write(&thd->net, (char*) thd->packet.ptr(),
		       thd->packet.length()))
	DBUG_RETURN(-1);
    }
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}

/***************************************************************************
** List all open tables in a database
***************************************************************************/

int mysqld_show_open_tables(THD *thd,const char *wild)
{
  List<Item> field_list;
  OPEN_TABLE_LIST *open_list;
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_show_open_tables");

  field_list.push_back(new Item_empty_string("Database",NAME_LEN));
  field_list.push_back(new Item_empty_string("Table",NAME_LEN));
  field_list.push_back(new Item_int("In_use",0, 4));
  field_list.push_back(new Item_int("Name_locked",0, 4));

  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);

  if (!(open_list=list_open_tables(thd,wild)) && thd->fatal_error)
    DBUG_RETURN(-1);

  for (; open_list ; open_list=open_list->next)
  {
    thd->packet.length(0);
    net_store_data(&thd->packet,convert, open_list->db);
    net_store_data(&thd->packet,convert, open_list->table);
    net_store_data(&thd->packet,open_list->in_use);
    net_store_data(&thd->packet,open_list->locked);
    if (my_net_write(&thd->net,(char*) thd->packet.ptr(),thd->packet.length()))
    {
      DBUG_RETURN(-1);
    }
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}


/***************************************************************************
** List all tables in a database (fast version)
** A table is a .frm file in the current databasedir
***************************************************************************/

int mysqld_show_tables(THD *thd,const char *db,const char *wild)
{
  Item_string *field=new Item_string("",0);
  List<Item> field_list;
  char path[FN_LEN],*end;
  List<char> files;
  char *file_name;
  DBUG_ENTER("mysqld_show_tables");

  field->name=(char*) thd->alloc(20+(uint) strlen(db)+(wild ? (uint) strlen(wild)+4:0));
  end=strxmov(field->name,"Tables_in_",db,NullS);
  if (wild && wild[0])
    strxmov(end," (",wild,")",NullS);
  field->max_length=NAME_LEN;
  (void) sprintf(path,"%s/%s",mysql_data_home,db);
  (void) unpack_dirname(path,path);
  field_list.push_back(field);
  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);
  if (mysql_find_files(thd,&files,db,path,wild,0))
    DBUG_RETURN(-1);
  List_iterator_fast<char> it(files);
  while ((file_name=it++))
  {
    thd->packet.length(0);
    net_store_data(&thd->packet, thd->variables.convert_set, file_name);
    if (my_net_write(&thd->net,(char*) thd->packet.ptr(),thd->packet.length()))
      DBUG_RETURN(-1);
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}


static int
mysql_find_files(THD *thd,List<char> *files, const char *db,const char *path,
                 const char *wild, bool dir)
{
  uint i;
  char *ext;
  MY_DIR *dirp;
  FILEINFO *file;
  uint col_access=thd->col_access;
  TABLE_LIST table_list;
  DBUG_ENTER("mysql_find_files");

  if (wild && !wild[0])
    wild=0;
  bzero((char*) &table_list,sizeof(table_list));

  if (!(dirp = my_dir(path,MYF(MY_WME | (dir ? MY_WANT_STAT : 0)))))
    DBUG_RETURN(-1);

  for (i=0 ; i < (uint) dirp->number_off_files  ; i++)
  {
    file=dirp->dir_entry+i;
    if (dir)
    {                                           /* Return databases */
#ifdef USE_SYMDIR
      char *ext;
      if (my_use_symdir && !strcmp(ext=fn_ext(file->name), ".sym"))
        *ext=0;                                 /* Remove extension */
      else
#endif
      {
        if (file->name[0] == '.' || !MY_S_ISDIR(file->mystat.st_mode) ||
            (wild && wild_compare(file->name,wild)))
          continue;
      }
    }
    else
    {
        // Return only .frm files which aren't temp files.
      if (my_strcasecmp(ext=fn_ext(file->name),reg_ext) ||
          is_prefix(file->name,tmp_file_prefix))
        continue;
      *ext=0;
      if (wild)
      {
	if (lower_case_table_names)
	{
	  if (wild_case_compare(file->name,wild))
	    continue;
	}
	else if (wild_compare(file->name,wild))
	  continue;
      }
    }
    /* Don't show tables where we don't have any privileges */
    if (db && !(col_access & TABLE_ACLS))
    {
      table_list.db= (char*) db;
      table_list.real_name=file->name;
      table_list.grant.privilege=col_access;
      if (check_grant(thd,TABLE_ACLS,&table_list,1,1))
        continue;
    }
    if (files->push_back(thd->strdup(file->name)))
    {
      my_dirend(dirp);
      DBUG_RETURN(-1);
    }
  }
  DBUG_PRINT("info",("found: %d files", files->elements));
  my_dirend(dirp);
  DBUG_RETURN(0);
}

/***************************************************************************
** Extended version of mysqld_show_tables
***************************************************************************/

int mysqld_extend_show_tables(THD *thd,const char *db,const char *wild)
{
  Item *item;
  List<char> files;
  List<Item> field_list;
  char path[FN_LEN];
  char *file_name;
  TABLE *table;
  String *packet= &thd->packet;
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_extend_show_tables");

  (void) sprintf(path,"%s/%s",mysql_data_home,db);
  (void) unpack_dirname(path,path);

  field_list.push_back(item=new Item_empty_string("Name",NAME_LEN));
  item->maybe_null=1;
  field_list.push_back(item=new Item_empty_string("Type",10));
  item->maybe_null=1;
  field_list.push_back(item=new Item_empty_string("Row_format",10));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Rows",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Avg_row_length",(int32) 0,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Data_length",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Max_data_length",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Index_length",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Data_free",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Auto_increment",(longlong) 1,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_datetime("Create_time"));
  item->maybe_null=1;
  field_list.push_back(item=new Item_datetime("Update_time"));
  item->maybe_null=1;
  field_list.push_back(item=new Item_datetime("Check_time"));
  item->maybe_null=1;
  field_list.push_back(item=new Item_empty_string("Create_options",255));
  item->maybe_null=1;
  field_list.push_back(item=new Item_empty_string("Comment",80));
  item->maybe_null=1;
  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);

  if (mysql_find_files(thd,&files,db,path,wild,0))
    DBUG_RETURN(-1);
  List_iterator_fast<char> it(files);
  while ((file_name=it++))
  {
    TABLE_LIST table_list;
    bzero((char*) &table_list,sizeof(table_list));
    packet->length(0);
    net_store_data(packet,convert, file_name);
    table_list.db=(char*) db;
    table_list.real_name= table_list.alias= file_name;
    if (lower_case_table_names)
      casedn_str(file_name);
    if (!(table = open_ltable(thd, &table_list, TL_READ)))
    {
      for (uint i=2 ; i < field_list.elements ; i++)
        net_store_null(packet);
      net_store_data(packet,convert, thd->net.last_error);
      thd->net.last_error[0]=0;
    }
    else
    {
      struct tm tm_tmp;
      handler *file=table->file;
      file->info(HA_STATUS_VARIABLE | HA_STATUS_TIME | HA_STATUS_NO_LOCK);
      net_store_data(packet, convert, file->table_type());
      net_store_data(packet, convert,
                     (table->db_options_in_use & HA_OPTION_COMPRESS_RECORD) ?
		     "Compressed" :
                     (table->db_options_in_use & HA_OPTION_PACK_RECORD) ?
                     "Dynamic" : "Fixed");
      net_store_data(packet, (longlong) file->records);
      net_store_data(packet, (uint32) file->mean_rec_length);
      net_store_data(packet, (longlong) file->data_file_length);
      if (file->max_data_file_length)
        net_store_data(packet, (longlong) file->max_data_file_length);
      else
        net_store_null(packet);
      net_store_data(packet, (longlong) file->index_file_length);
      net_store_data(packet, (longlong) file->delete_length);
      if (table->found_next_number_field)
      {
        table->next_number_field=table->found_next_number_field;
        table->next_number_field->reset();
        file->update_auto_increment();
        net_store_data(packet, table->next_number_field->val_int());
        table->next_number_field=0;
      }
      else
        net_store_null(packet);
      if (!file->create_time)
        net_store_null(packet);
      else
      {
        localtime_r(&file->create_time,&tm_tmp);
        net_store_data(packet, &tm_tmp);
      }
      if (!file->update_time)
        net_store_null(packet);
      else
      {
        localtime_r(&file->update_time,&tm_tmp);
        net_store_data(packet, &tm_tmp);
      }
      if (!file->check_time)
        net_store_null(packet);
      else
      {
        localtime_r(&file->check_time,&tm_tmp);
        net_store_data(packet, &tm_tmp);
      }
      {
        char option_buff[350],*ptr;
        ptr=option_buff;
        if (table->min_rows)
        {
          ptr=strmov(ptr," min_rows=");
          ptr=longlong10_to_str(table->min_rows,ptr,10);
        }
        if (table->max_rows)
        {
          ptr=strmov(ptr," max_rows=");
          ptr=longlong10_to_str(table->max_rows,ptr,10);
        }
        if (table->avg_row_length)
        {
          ptr=strmov(ptr," avg_row_length=");
          ptr=longlong10_to_str(table->avg_row_length,ptr,10);
        }
        if (table->db_create_options & HA_OPTION_PACK_KEYS)
          ptr=strmov(ptr," pack_keys=1");
        if (table->db_create_options & HA_OPTION_NO_PACK_KEYS)
          ptr=strmov(ptr," pack_keys=0");
        if (table->db_create_options & HA_OPTION_CHECKSUM)
          ptr=strmov(ptr," checksum=1");
        if (table->db_create_options & HA_OPTION_DELAY_KEY_WRITE)
          ptr=strmov(ptr," delay_key_write=1");
        if (table->row_type != ROW_TYPE_DEFAULT)
          ptr=strxmov(ptr, " row_format=", ha_row_type[(uint) table->row_type],
                      NullS);
        if (file->raid_type)
        {
          char buff[100];
          sprintf(buff," raid_type=%s raid_chunks=%d raid_chunksize=%ld",
                  my_raid_type(file->raid_type), file->raid_chunks, file->raid_chunksize/RAID_BLOCK_SIZE);
          ptr=strmov(ptr,buff);
        }
        net_store_data(packet, convert, option_buff+1,
                       (ptr == option_buff ? 0 : (uint) (ptr-option_buff)-1));
      }
      {
	char *comment=table->file->update_table_comment(table->comment);
	net_store_data(packet, comment);
	if (comment != table->comment)
	  my_free(comment,MYF(0));
      }
      close_thread_tables(thd,0);
    }
    if (my_net_write(&thd->net,(char*) packet->ptr(),
                     packet->length()))
      DBUG_RETURN(-1);
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}



/***************************************************************************
** List all columns in a table_list->real_name
***************************************************************************/

int
mysqld_show_fields(THD *thd, TABLE_LIST *table_list,const char *wild,
		   bool verbose)
{
  TABLE *table;
  handler *file;
  char tmp[MAX_FIELD_WIDTH];
  Item *item;
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_show_fields");
  DBUG_PRINT("enter",("db: %s  table: %s",table_list->db,
                      table_list->real_name));

  if (!(table = open_ltable(thd, table_list, TL_UNLOCK)))
  {
    send_error(&thd->net);
    DBUG_RETURN(1);
  }
  file=table->file;
  file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
  (void) get_table_grant(thd, table_list);

  List<Item> field_list;
  field_list.push_back(new Item_empty_string("Field",NAME_LEN));
  field_list.push_back(new Item_empty_string("Type",40));
  field_list.push_back(new Item_empty_string("Null",1));
  field_list.push_back(new Item_empty_string("Key",3));
  field_list.push_back(item=new Item_empty_string("Default",NAME_LEN));
  item->maybe_null=1;
  field_list.push_back(new Item_empty_string("Extra",20));
  if (verbose)
    field_list.push_back(new Item_empty_string("Privileges",80));

        // Send first number of fields and records
  {
    char *pos;
    pos=net_store_length(tmp, (uint) field_list.elements);
    pos=net_store_length(pos,(ulonglong) file->records);
    (void) my_net_write(&thd->net,tmp,(uint) (pos-tmp));
  }

  if (send_fields(thd,field_list,0))
    DBUG_RETURN(1);
  restore_record(table,2);      // Get empty record

  Field **ptr,*field;
  String *packet= &thd->packet;
  for (ptr=table->field; (field= *ptr) ; ptr++)
  {
    if (!wild || !wild[0] || !wild_case_compare(field->field_name,wild))
    {
#ifdef NOT_USED
      if (thd->col_access & TABLE_ACLS ||
          ! check_grant_column(thd,table,field->field_name,
                               (uint) strlen(field->field_name),1))
#endif
      {
        byte *pos;
        uint flags=field->flags;
        String type(tmp,sizeof(tmp));
        uint col_access;
        bool null_default_value=0;

        packet->length(0);
        net_store_data(packet,convert,field->field_name);
        field->sql_type(type);
        net_store_data(packet,convert,type.ptr(),type.length());

        pos=(byte*) ((flags & NOT_NULL_FLAG) &&
                     field->type() != FIELD_TYPE_TIMESTAMP ?
                     "" : "YES");
        net_store_data(packet,convert,(const char*) pos);
        pos=(byte*) ((field->flags & PRI_KEY_FLAG) ? "PRI" :
                     (field->flags & UNIQUE_KEY_FLAG) ? "UNI" :
                     (field->flags & MULTIPLE_KEY_FLAG) ? "MUL":"");
        net_store_data(packet,convert,(char*) pos);

        if (field->type() == FIELD_TYPE_TIMESTAMP ||
            field->unireg_check == Field::NEXT_NUMBER)
          null_default_value=1;
        if (!null_default_value && !field->is_null())
        {                                               // Not null by default
          type.set(tmp,sizeof(tmp));
          field->val_str(&type,&type);
          net_store_data(packet,convert,type.ptr(),type.length());
        }
        else if (field->maybe_null() || null_default_value)
          net_store_null(packet);                       // Null as default
        else
          net_store_data(packet,convert,tmp,0);

        char *end=tmp;
        if (field->unireg_check == Field::NEXT_NUMBER)
          end=strmov(tmp,"auto_increment");
        net_store_data(packet,convert,tmp,(uint) (end-tmp));

	if (verbose)
	{
	  /* Add grant options */
	  col_access= get_column_grant(thd,table_list,field) & COL_ACLS;
	  end=tmp;
	  for (uint bitnr=0; col_access ; col_access>>=1,bitnr++)
	  {
	    if (col_access & 1)
	    {
	      *end++=',';
	      end=strmov(end,grant_types.type_names[bitnr]);
	    }
	  }
	  net_store_data(packet,convert, tmp+1,end == tmp ? 0 : (uint) (end-tmp-1));
	}
        if (my_net_write(&thd->net,(char*) packet->ptr(),packet->length()))
          DBUG_RETURN(1);
      }
    }
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}

int
mysqld_show_create(THD *thd, TABLE_LIST *table_list)
{
  TABLE *table;
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_show_create");
  DBUG_PRINT("enter",("db: %s  table: %s",table_list->db,
                      table_list->real_name));

  /* Only one table for now */
  if (!(table = open_ltable(thd, table_list, TL_UNLOCK)))
  {
    send_error(&thd->net);
    DBUG_RETURN(1);
  }

  char buff[1024];
  String packet(buff,sizeof(buff));
  packet.length(0);
  net_store_data(&packet,convert, table->table_name);
  /*
    A hack - we need to reserve some space for the length before
    we know what it is - let's assume that the length of create table
    statement will fit into 3 bytes ( 16 MB max :-) )
  */
  ulong store_len_offset = packet.length();
  packet.length(store_len_offset + 4);
  if (store_create_info(thd, table, &packet))
    DBUG_RETURN(-1);
  ulong create_len = packet.length() - store_len_offset - 4;
  if (create_len > 0x00ffffff) // better readable in HEX ...
  {
    /*
      Just in case somebody manages to create a table
      with *that* much stuff in the definition
    */
    DBUG_RETURN(1);
  }

  /*
    Now we have to store the length in three bytes, even if it would fit
    into fewer bytes, so we cannot use net_store_data() anymore,
    and do it ourselves
  */
  char* p = (char*)packet.ptr() + store_len_offset;
  *p++ = (char) 253; // The client the length is stored using 3-bytes
  int3store(p, create_len);

  List<Item> field_list;
  field_list.push_back(new Item_empty_string("Table",NAME_LEN));
  field_list.push_back(new Item_empty_string("Create Table",
        max(packet.length(),1024))); // 1024 is for not to confuse old clients

  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);

  if (my_net_write(&thd->net, (char*)packet.ptr(), packet.length()))
    DBUG_RETURN(1);

  send_eof(&thd->net);
  DBUG_RETURN(0);
}


int
mysqld_show_logs(THD *thd)
{
  DBUG_ENTER("mysqld_show_logs");

  List<Item> field_list;
  field_list.push_back(new Item_empty_string("File",FN_REFLEN));
  field_list.push_back(new Item_empty_string("Type",10));
  field_list.push_back(new Item_empty_string("Status",10));

  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);

#ifdef HAVE_BERKELEY_DB
  if (!berkeley_skip && berkeley_show_logs(thd))
    DBUG_RETURN(-1);
#endif

  send_eof(&thd->net);
  DBUG_RETURN(0);
}


int
mysqld_show_keys(THD *thd, TABLE_LIST *table_list)
{
  TABLE *table;
  char buff[256];
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_show_keys");
  DBUG_PRINT("enter",("db: %s  table: %s",table_list->db,
                      table_list->real_name));

  if (!(table = open_ltable(thd, table_list, TL_UNLOCK)))
  {
    send_error(&thd->net);
    DBUG_RETURN(1);
  }

  List<Item> field_list;
  Item *item;
  field_list.push_back(new Item_empty_string("Table",NAME_LEN));
  field_list.push_back(new Item_int("Non_unique",0,1));
  field_list.push_back(new Item_empty_string("Key_name",NAME_LEN));
  field_list.push_back(new Item_int("Seq_in_index",0,2));
  field_list.push_back(new Item_empty_string("Column_name",NAME_LEN));
  field_list.push_back(item=new Item_empty_string("Collation",1));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Cardinality",0,21));
  item->maybe_null=1;
  field_list.push_back(item=new Item_int("Sub_part",0,3));
  item->maybe_null=1;
  field_list.push_back(item=new Item_empty_string("Packed",10));
  item->maybe_null=1;
  field_list.push_back(new Item_empty_string("Null",3));
  field_list.push_back(new Item_empty_string("Index_type",16));
  field_list.push_back(new Item_empty_string("Comment",255));
  item->maybe_null=1;

  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1);

  String *packet= &thd->packet;
  KEY *key_info=table->key_info;
  table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK | HA_STATUS_TIME);
  for (uint i=0 ; i < table->keys ; i++,key_info++)
  {
    KEY_PART_INFO *key_part= key_info->key_part;
    char *end;
    for (uint j=0 ; j < key_info->key_parts ; j++,key_part++)
    {
      packet->length(0);
      net_store_data(packet,convert,table->table_name);
      net_store_data(packet,convert,((key_info->flags & HA_NOSAME) ? "0" :"1"), 1);
      net_store_data(packet,convert,key_info->name);
      end=int10_to_str((long) (j+1),(char*) buff,10);
      net_store_data(packet,convert,buff,(uint) (end-buff));
      net_store_data(packet,convert,
		     key_part->field ? key_part->field->field_name :
                     "?unknown field?");
      if (table->file->index_flags(i) & HA_READ_ORDER)
        net_store_data(packet,convert,
		       ((key_part->key_part_flag & HA_REVERSE_SORT) ?
			"D" : "A"), 1);
      else
        net_store_null(packet); /* purecov: inspected */
      KEY *key=table->key_info+i;
      if (key->rec_per_key[j])
      {
        ha_rows records=(table->file->records / key->rec_per_key[j]);
        end=longlong10_to_str((longlong) records, buff, 10);
        net_store_data(packet,convert,buff,(uint) (end-buff));
      }
      else
        net_store_null(packet);

      /* Check if we have a key part that only uses part of the field */
      if (!key_part->field ||
          key_part->length !=
          table->field[key_part->fieldnr-1]->key_length())
      {
        end=int10_to_str((long) key_part->length, buff,10); /* purecov: inspected */
        net_store_data(packet,convert,buff,(uint) (end-buff)); /* purecov: inspected */
      }
      else
        net_store_null(packet);
      net_store_null(packet);                   // No pack_information yet

      /* Null flag */
      uint flags= key_part->field ? key_part->field->flags : 0;
      char *pos=(char*) ((flags & NOT_NULL_FLAG) ? "" : "YES");
      net_store_data(packet,convert,(const char*) pos);
      net_store_data(packet,convert,table->file->index_type(i));
      /* Comment */
      if (!(table->keys_in_use & ((key_map) 1 << i)))
	net_store_data(packet,convert,"disabled",8);
      else
	net_store_data(packet,convert,"");
      if (my_net_write(&thd->net,(char*) packet->ptr(),packet->length()))
        DBUG_RETURN(1); /* purecov: inspected */
    }
  }
  send_eof(&thd->net);
  DBUG_RETURN(0);
}


/****************************************************************************
** Return only fields for API mysql_list_fields
** Use "show table wildcard" in mysql instead of this
****************************************************************************/

void
mysqld_list_fields(THD *thd, TABLE_LIST *table_list, const char *wild)
{
  TABLE *table;
  DBUG_ENTER("mysqld_list_fields");
  DBUG_PRINT("enter",("table: %s",table_list->real_name));

  if (!(table = open_ltable(thd, table_list, TL_UNLOCK)))
  {
    send_error(&thd->net);
    DBUG_VOID_RETURN;
  }
  List<Item> field_list;

  Field **ptr,*field;
  for (ptr=table->field ; (field= *ptr); ptr++)
  {
    if (!wild || !wild[0] || !wild_case_compare(field->field_name,wild))
      field_list.push_back(new Item_field(field));
  }
  restore_record(table,2);              // Get empty record
  if (send_fields(thd,field_list,2))
    DBUG_VOID_RETURN;
  VOID(net_flush(&thd->net));
  DBUG_VOID_RETURN;
}

int
mysqld_dump_create_info(THD *thd, TABLE *table, int fd)
{
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_dump_create_info");
  DBUG_PRINT("enter",("table: %s",table->real_name));

  String* packet = &thd->packet;
  packet->length(0);
  if (store_create_info(thd,table,packet))
    DBUG_RETURN(-1);

  if (convert)
    convert->convert((char*) packet->ptr(), packet->length());
  if (fd < 0)
  {
    if (my_net_write(&thd->net, (char*)packet->ptr(), packet->length()))
      DBUG_RETURN(-1);
    VOID(net_flush(&thd->net));
  }
  else
  {
    if (my_write(fd, (const byte*) packet->ptr(), packet->length(),
		 MYF(MY_WME)))
      DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}

static void
append_identifier(THD *thd, String *packet, const char *name)
{
  if (thd->options & OPTION_QUOTE_SHOW_CREATE)
  {
    packet->append("`", 1);
    packet->append(name);
    packet->append("`", 1);
  }
  else
  {
    packet->append(name);
  }
}


/* Append directory name (if exists) to CREATE INFO */

static void append_directory(THD *thd, String *packet, const char *dir_type,
			     const char *filename)
{
  uint length;
  if (filename && !(thd->sql_mode & MODE_NO_DIR_IN_CREATE))
  {
    length= dirname_length(filename);
    packet->append(' ');
    packet->append(dir_type);
    packet->append(" DIRECTORY='", 12);
    packet->append(filename, length);
    packet->append('\'');
  }
}


static int
store_create_info(THD *thd, TABLE *table, String *packet)
{
  List<Item> field_list;
  char tmp[MAX_FIELD_WIDTH], *for_str, buff[128], *end;
  String type(tmp, sizeof(tmp));
  Field **ptr,*field;
  uint primary_key;
  KEY *key_info;
  handler *file= table->file;
  HA_CREATE_INFO create_info;
  DBUG_ENTER("store_create_info");
  DBUG_PRINT("enter",("table: %s",table->real_name));

  restore_record(table,2); // Get empty record
  if (table->tmp_table)
    packet->append("CREATE TEMPORARY TABLE ", 23);
  else
    packet->append("CREATE TABLE ", 13);
  append_identifier(thd,packet,table->real_name);
  packet->append(" (\n", 3);

  for (ptr=table->field ; (field= *ptr); ptr++)
  {
    bool has_default;
    uint flags = field->flags;

    if (ptr != table->field)
      packet->append(",\n", 2);
    packet->append("  ", 2);
    append_identifier(thd,packet,field->field_name);
    packet->append(' ');
    // check for surprises from the previous call to Field::sql_type()
    if (type.ptr() != tmp)
      type.set(tmp, sizeof(tmp));

    field->sql_type(type);
    packet->append(type.ptr(),type.length());

    has_default= (field->type() != FIELD_TYPE_BLOB &&
		  field->type() != FIELD_TYPE_TIMESTAMP &&
		  field->unireg_check != Field::NEXT_NUMBER);
    if (flags & NOT_NULL_FLAG)
      packet->append(" NOT NULL", 9);

    if (has_default)
    {
      packet->append(" default ", 9);
      if (!field->is_null())
      {                                             // Not null by default
        type.set(tmp,sizeof(tmp));
        field->val_str(&type,&type);
        packet->append('\'');
	if (type.length())
          append_unescaped(packet, type.c_ptr());
        packet->append('\'');
      }
      else if (field->maybe_null())
        packet->append("NULL", 4);                    // Null as default
      else
        packet->append(tmp,0);
    }

    if (field->unireg_check == Field::NEXT_NUMBER)
          packet->append(" auto_increment", 15 );
  }

  key_info= table->key_info;
  file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK | HA_STATUS_TIME);
  bzero((char*) &create_info, sizeof(create_info));
  file->update_create_info(&create_info);
  primary_key= table->primary_key;

  for (uint i=0 ; i < table->keys ; i++,key_info++)
  {
    KEY_PART_INFO *key_part= key_info->key_part;
    bool found_primary=0;
    packet->append(",\n  ", 4);

    if (i == primary_key && !strcmp(key_info->name,"PRIMARY"))
    {
      found_primary=1;
      packet->append("PRIMARY ", 8);
    }
    else if (key_info->flags & HA_NOSAME)
      packet->append("UNIQUE ", 7);
    else if (key_info->flags & HA_FULLTEXT)
      packet->append("FULLTEXT ", 9);
    packet->append("KEY ", 4);

    if (!found_primary)
     append_identifier(thd,packet,key_info->name);

    packet->append(" (", 2);

    for (uint j=0 ; j < key_info->key_parts ; j++,key_part++)
    {
      if (j)
        packet->append(',');

      if (key_part->field)
        append_identifier(thd,packet,key_part->field->field_name);
      if (!key_part->field ||
          (key_part->length !=
           table->field[key_part->fieldnr-1]->key_length() &&
           !(key_info->flags & HA_FULLTEXT)))
      {
        buff[0] = '(';
        char* end=int10_to_str((long) key_part->length, buff + 1,10);
        *end++ = ')';
        packet->append(buff,(uint) (end-buff));
      }
    }
    packet->append(')');
  }

  /*
    Get possible foreign key definitions stored in InnoDB and append them
    to the CREATE TABLE statement
  */

  if ((for_str= file->get_foreign_key_create_info()))
  {
    packet->append(for_str, strlen(for_str));
    file->free_foreign_key_create_info(for_str);
  }

  packet->append("\n)", 2);
  packet->append(" TYPE=", 6);
  packet->append(file->table_type());

  if (table->min_rows)
  {
    packet->append(" MIN_ROWS=");
    end= longlong10_to_str(table->min_rows, buff, 10);
    packet->append(buff, (uint) (end- buff));
  }
  if (table->max_rows)
  {
    packet->append(" MAX_ROWS=");
    end= longlong10_to_str(table->max_rows, buff, 10);
    packet->append(buff, (uint) (end - buff));
  }
  if (table->avg_row_length)
  {
    packet->append(" AVG_ROW_LENGTH=");
    end= longlong10_to_str(table->avg_row_length, buff,10);
    packet->append(buff, (uint) (end - buff));
  }

  if (table->db_create_options & HA_OPTION_PACK_KEYS)
    packet->append(" PACK_KEYS=1", 12);
  if (table->db_create_options & HA_OPTION_NO_PACK_KEYS)
    packet->append(" PACK_KEYS=0", 12);
  if (table->db_create_options & HA_OPTION_CHECKSUM)
    packet->append(" CHECKSUM=1", 11);
  if (table->db_create_options & HA_OPTION_DELAY_KEY_WRITE)
    packet->append(" DELAY_KEY_WRITE=1",18);
  if (table->row_type != ROW_TYPE_DEFAULT)
  {
    packet->append(" ROW_FORMAT=",12);
    packet->append(ha_row_type[(uint) table->row_type]);
  }
  table->file->append_create_info(packet);
  if (table->comment && table->comment[0])
  {
    packet->append(" COMMENT='", 10);
    append_unescaped(packet, table->comment);
    packet->append('\'');
  }
  if (file->raid_type)
  {
    uint length;
    length= my_snprintf(buff,sizeof(buff),
			" RAID_TYPE=%s RAID_CHUNKS=%d RAID_CHUNKSIZE=%ld",
			my_raid_type(file->raid_type), file->raid_chunks,
			file->raid_chunksize/RAID_BLOCK_SIZE);
    packet->append(buff, length);
  }
  append_directory(thd, packet, "DATA",  create_info.data_file_name);
  append_directory(thd, packet, "INDEX", create_info.index_file_name);
  DBUG_RETURN(0);
}


/****************************************************************************
** Return info about all processes
** returns for each thread: thread id, user, host, db, command, info
****************************************************************************/

class thread_info :public ilink {
public:
  static void *operator new(size_t size) {return (void*) sql_alloc((uint) size); }
  static void operator delete(void *ptr __attribute__((unused)),
                              size_t size __attribute__((unused))) {} /*lint -e715 */

  ulong thread_id;
  time_t start_time;
  uint   command;
  const char *user,*host,*db,*proc_info,*state_info;
  char *query;
};

#ifdef __GNUC__
template class I_List<thread_info>;
#endif

#define LIST_PROCESS_HOST_LEN 64

void mysqld_list_processes(THD *thd,const char *user, bool verbose)
{
  Item *field;
  List<Item> field_list;
  I_List<thread_info> thread_infos;
  ulong max_query_length= (verbose ? thd->variables.max_allowed_packet :
			   PROCESS_LIST_WIDTH);
  CONVERT *convert=thd->variables.convert_set;
  DBUG_ENTER("mysqld_list_processes");

  field_list.push_back(new Item_int("Id",0,7));
  field_list.push_back(new Item_empty_string("User",16));
  field_list.push_back(new Item_empty_string("Host",LIST_PROCESS_HOST_LEN));
  field_list.push_back(field=new Item_empty_string("db",NAME_LEN));
  field->maybe_null=1;
  field_list.push_back(new Item_empty_string("Command",16));
  field_list.push_back(new Item_empty_string("Time",7));
  field_list.push_back(field=new Item_empty_string("State",30));
  field->maybe_null=1;
  field_list.push_back(field=new Item_empty_string("Info",max_query_length));
  field->maybe_null=1;
  if (send_fields(thd,field_list,1))
    DBUG_VOID_RETURN;

  VOID(pthread_mutex_lock(&LOCK_thread_count)); // For unlink from list
  if (!thd->killed)
  {
    I_List_iterator<THD> it(threads);
    THD *tmp;
    while ((tmp=it++))
    {
      struct st_my_thread_var *mysys_var;
      if ((tmp->net.vio || tmp->system_thread) &&
          (!user || (tmp->user && !strcmp(tmp->user,user))))
      {
        thread_info *thd_info=new thread_info;

        thd_info->thread_id=tmp->thread_id;
        thd_info->user=thd->strdup(tmp->user ? tmp->user :
				   (tmp->system_thread ?
				    "system user" : "unauthenticated user"));
	if (tmp->peer_port && (tmp->host || tmp->ip) && thd->host_or_ip[0])
	{
	  if ((thd_info->host= thd->alloc(LIST_PROCESS_HOST_LEN+1)))
	    my_snprintf((char *) thd_info->host, LIST_PROCESS_HOST_LEN,
			"%s:%u", tmp->host_or_ip, tmp->peer_port);
	}
	else
	  thd_info->host= thd->strdup(tmp->host_or_ip);
        if ((thd_info->db=tmp->db))             // Safe test
          thd_info->db=thd->strdup(thd_info->db);
        thd_info->command=(int) tmp->command;
        if ((mysys_var= tmp->mysys_var))
          pthread_mutex_lock(&mysys_var->mutex);
        thd_info->proc_info= (char*) (tmp->killed ? "Killed" : 0);
        thd_info->state_info= (char*) (tmp->locked ? "Locked" :
                                       tmp->net.reading_or_writing ?
                                       (tmp->net.reading_or_writing == 2 ?
                                        "Writing to net" :
                                        thd_info->command == COM_SLEEP ? "" :
                                        "Reading from net") :
                                       tmp->proc_info ? tmp->proc_info :
                                       tmp->mysys_var &&
                                       tmp->mysys_var->current_cond ?
                                       "Waiting on cond" : NullS);
        if (mysys_var)
          pthread_mutex_unlock(&mysys_var->mutex);

#if !defined(DONT_USE_THR_ALARM) && ! defined(SCO)
        if (pthread_kill(tmp->real_id,0))
          tmp->proc_info="*** DEAD ***";        // This shouldn't happen
#endif
#ifdef EXTRA_DEBUG
        thd_info->start_time= tmp->time_after_lock;
#else
        thd_info->start_time= tmp->start_time;
#endif
        thd_info->query=0;
        if (tmp->query)
        {
	  /* query_length is always set before tmp->query */
          uint length= min(max_query_length, tmp->query_length);
          thd_info->query=(char*) thd->memdup(tmp->query,length+1);
          thd_info->query[length]=0;
        }
        thread_infos.append(thd_info);
      }
    }
  }
  VOID(pthread_mutex_unlock(&LOCK_thread_count));

  thread_info *thd_info;
  String *packet= &thd->packet;
  while ((thd_info=thread_infos.get()))
  {
    char buff[20],*end;
    packet->length(0);
    end=int10_to_str((long) thd_info->thread_id, buff,10);
    net_store_data(packet,convert,buff,(uint) (end-buff));
    net_store_data(packet,convert,thd_info->user);
    net_store_data(packet,convert,thd_info->host);
    if (thd_info->db)
      net_store_data(packet,convert,thd_info->db);
    else
      net_store_null(packet);
    if (thd_info->proc_info)
      net_store_data(packet,convert,thd_info->proc_info);
    else
      net_store_data(packet,convert,command_name[thd_info->command]);
    if (thd_info->start_time)
      net_store_data(packet,
		     (uint32) (time((time_t*) 0) - thd_info->start_time));
    else
      net_store_null(packet);
    if (thd_info->state_info)
      net_store_data(packet,convert,thd_info->state_info);
    else
      net_store_null(packet);
    if (thd_info->query)
      net_store_data(packet,convert,thd_info->query);
    else
      net_store_null(packet);
    if (my_net_write(&thd->net,(char*) packet->ptr(),packet->length()))
      break; /* purecov: inspected */
  }
  send_eof(&thd->net);
  DBUG_VOID_RETURN;
}


/*****************************************************************************
** Status functions
*****************************************************************************/


int mysqld_show(THD *thd, const char *wild, show_var_st *variables,
		enum enum_var_type value_type,
		pthread_mutex_t *mutex)
{
  char buff[8192];
  String packet2(buff,sizeof(buff));
  List<Item> field_list;
  CONVERT *convert=thd->variables.convert_set;

  DBUG_ENTER("mysqld_show");
  field_list.push_back(new Item_empty_string("Variable_name",30));
  field_list.push_back(new Item_empty_string("Value",256));
  if (send_fields(thd,field_list,1))
    DBUG_RETURN(1); /* purecov: inspected */

  pthread_mutex_lock(mutex);
  for (; variables->name; variables++)
  {
    if (!(wild && wild[0] && wild_case_compare(variables->name,wild)))
    {
      packet2.length(0);
      net_store_data(&packet2,convert,variables->name);
      SHOW_TYPE show_type=variables->type;
      char *value=variables->value;
      if (show_type == SHOW_SYS)
      {
	show_type= ((sys_var*) value)->type();
	value=     (char*) ((sys_var*) value)->value_ptr(thd, value_type);
      }

      switch (show_type) {
      case SHOW_LONG:
      case SHOW_LONG_CONST:
        net_store_data(&packet2,(uint32) *(ulong*) value);
        break;
      case SHOW_LONGLONG:
        net_store_data(&packet2,(longlong) *(longlong*) value);
        break;
      case SHOW_HA_ROWS:
        net_store_data(&packet2,(longlong) *(ha_rows*) value);
        break;
      case SHOW_BOOL:
        net_store_data(&packet2,(ulong) *(bool*) value ? "ON" : "OFF");
        break;
      case SHOW_MY_BOOL:
        net_store_data(&packet2,(ulong) *(my_bool*) value ? "ON" : "OFF");
        break;
      case SHOW_INT_CONST:
      case SHOW_INT:
        net_store_data(&packet2,(uint32) *(int*) value);
        break;
      case SHOW_HAVE:
      {
	SHOW_COMP_OPTION tmp= *(SHOW_COMP_OPTION*) value;
        net_store_data(&packet2, (tmp == SHOW_OPTION_NO ? "NO" :
				  tmp == SHOW_OPTION_YES ? "YES" :
				  "DISABLED"));
        break;
      }
      case SHOW_CHAR:
        net_store_data(&packet2,convert, value);
        break;
      case SHOW_STARTTIME:
        net_store_data(&packet2,(uint32) (thd->query_start() - start_time));
        break;
      case SHOW_QUESTION:
        net_store_data(&packet2,(uint32) thd->query_id);
        break;
      case SHOW_RPL_STATUS:
	net_store_data(&packet2, rpl_status_type[(int)rpl_status]);
	break;
#ifdef HAVE_REPLICATION
      case SHOW_SLAVE_RUNNING:
      {
	LOCK_ACTIVE_MI;
	net_store_data(&packet2, (active_mi->slave_running &&
				  active_mi->rli.slave_running)
		        ? "ON" : "OFF");
	UNLOCK_ACTIVE_MI;
	break;
      }
#endif
      case SHOW_OPENTABLES:
        net_store_data(&packet2,(uint32) cached_tables());
        break;
      case SHOW_CHAR_PTR:
      {
	value= *(char**) value;
	net_store_data(&packet2,convert, value ? value : "");
	break;
      }
#ifdef HAVE_OPENSSL
	/* First group - functions relying on CTX */
      case SHOW_SSL_CTX_SESS_ACCEPT:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_accept(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_ACCEPT_GOOD:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_accept_good(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_CONNECT_GOOD:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_connect_good(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_ACCEPT_RENEGOTIATE:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_accept_renegotiate(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_CONNECT_RENEGOTIATE:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_connect_renegotiate(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_CB_HITS:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_cb_hits(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_HITS:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_hits(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_CACHE_FULL:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_cache_full(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_MISSES:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_misses(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_TIMEOUTS:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_timeouts(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_NUMBER:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_number(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_CONNECT:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_connect(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_SESS_GET_CACHE_SIZE:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_sess_get_cache_size(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_GET_VERIFY_MODE:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_get_verify_mode(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_GET_VERIFY_DEPTH:
	net_store_data(&packet2,(uint32) 
		       (!ssl_acceptor_fd ? 0 :
			SSL_CTX_get_verify_depth(ssl_acceptor_fd->ssl_context)));
        break;
      case SHOW_SSL_CTX_GET_SESSION_CACHE_MODE:
	if (!ssl_acceptor_fd)
	{
	  net_store_data(&packet2,"NONE" );
	  break;
	}
	switch (SSL_CTX_get_session_cache_mode(ssl_acceptor_fd->ssl_context))
	{
          case SSL_SESS_CACHE_OFF:
            net_store_data(&packet2,"OFF" );
	    break;
          case SSL_SESS_CACHE_CLIENT:
            net_store_data(&packet2,"CLIENT" );
	    break;
          case SSL_SESS_CACHE_SERVER:
            net_store_data(&packet2,"SERVER" );
	    break;
          case SSL_SESS_CACHE_BOTH:
            net_store_data(&packet2,"BOTH" );
	    break;
          case SSL_SESS_CACHE_NO_AUTO_CLEAR:
            net_store_data(&packet2,"NO_AUTO_CLEAR" );
	    break;
          case SSL_SESS_CACHE_NO_INTERNAL_LOOKUP:
            net_store_data(&packet2,"NO_INTERNAL_LOOKUP" );
	    break;
	  default:
            net_store_data(&packet2,"Unknown");
	    break;
	}
        break;
	/* First group - functions relying on SSL */
      case SHOW_SSL_GET_VERSION:
	net_store_data(&packet2, thd->net.vio->ssl_arg ? 
			SSL_get_version((SSL*) thd->net.vio->ssl_arg) : "");
        break;
      case SHOW_SSL_SESSION_REUSED:
	net_store_data(&packet2,(uint32) (thd->net.vio->ssl_arg ? 
			SSL_session_reused((SSL*) thd->net.vio->ssl_arg) : 0));
        break;
      case SHOW_SSL_GET_DEFAULT_TIMEOUT:
	net_store_data(&packet2,(uint32) (thd->net.vio->ssl_arg ?
			SSL_get_default_timeout((SSL*) thd->net.vio->ssl_arg) :
					  0));
        break;
      case SHOW_SSL_GET_VERIFY_MODE:
	net_store_data(&packet2,(uint32) (thd->net.vio->ssl_arg ?
			SSL_get_verify_mode((SSL*) thd->net.vio->ssl_arg):0));
        break;
      case SHOW_SSL_GET_VERIFY_DEPTH:
	net_store_data(&packet2,(uint32) (thd->net.vio->ssl_arg ?
			SSL_get_verify_depth((SSL*) thd->net.vio->ssl_arg):0));
        break;
      case SHOW_SSL_GET_CIPHER:
	net_store_data(&packet2, thd->net.vio->ssl_arg ?
		       SSL_get_cipher((SSL*) thd->net.vio->ssl_arg) : "");
	break;
      case SHOW_SSL_GET_CIPHER_LIST:
	if (thd->net.vio->ssl_arg)
	{
	  char buf[1024], *pos;
	  pos=buf;
	  for (int i=0 ; i++ ;)
	  {
	    const char *p=SSL_get_cipher_list((SSL*) thd->net.vio->ssl_arg,i);
	    if (p == NULL) 
	      break;
	    pos=strmov(pos, p);
	    *pos++= ':';
	  }
	  if (pos != buf)
	    pos--;				// Remove last ':'
	  *pos=0;
 	  net_store_data(&packet2, buf);
        }
	else
 	  net_store_data(&packet2, "");
        break;

#endif /* HAVE_OPENSSL */
      case SHOW_UNDEF:				// Show never happen
      case SHOW_SYS:
	net_store_data(&packet2, "");		// Safety
	break;
      }
      if (my_net_write(&thd->net, (char*) packet2.ptr(),packet2.length()))
        goto err;                               /* purecov: inspected */
    }
  }
  pthread_mutex_unlock(mutex);
  send_eof(&thd->net);
  DBUG_RETURN(0);

 err:
  pthread_mutex_unlock(mutex);
  DBUG_RETURN(1);
}

#ifdef __GNUC__
template class List_iterator_fast<char>;
template class List<char>;
#endif

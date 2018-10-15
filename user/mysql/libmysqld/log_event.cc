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


#ifndef MYSQL_CLIENT
#ifdef __GNUC__
#pragma implementation				// gcc: Class implementation
#endif
#include  "mysql_priv.h"
#include "slave.h"
#include <my_dir.h>
#endif /* MYSQL_CLIENT */

#include <assert.h>

#ifdef MYSQL_CLIENT
static void pretty_print_str(FILE* file, char* str, int len)
{
  char* end = str + len;
  fputc('\'', file);
  while (str < end)
  {
    char c;
    switch ((c=*str++)) {
    case '\n': fprintf(file, "\\n"); break;
    case '\r': fprintf(file, "\\r"); break;
    case '\\': fprintf(file, "\\\\"); break;
    case '\b': fprintf(file, "\\b"); break;
    case '\t': fprintf(file, "\\t"); break;
    case '\'': fprintf(file, "\\'"); break;
    case 0   : fprintf(file, "\\0"); break;
    default:
      fputc(c, file);
      break;
    }
  }
  fputc('\'', file);
}
#endif

#ifndef MYSQL_CLIENT

inline int ignored_error_code(int err_code)
{
  return ((err_code == ER_SLAVE_IGNORED_TABLE) ||
          (use_slave_mask && bitmap_is_set(&slave_error_mask, err_code)));
}


static void pretty_print_str(String* packet, char* str, int len)
{
  char* end = str + len;
  packet->append('\'');
  while (str < end)
  {
    char c;
    switch ((c=*str++)) {
    case '\n': packet->append( "\\n"); break;
    case '\r': packet->append( "\\r"); break;
    case '\\': packet->append( "\\\\"); break;
    case '\b': packet->append( "\\b"); break;
    case '\t': packet->append( "\\t"); break;
    case '\'': packet->append( "\\'"); break;
    case 0   : packet->append( "\\0"); break;
    default:
      packet->append((char)c);
      break;
    }
  }
  packet->append('\'');
}


static inline char* slave_load_file_stem(char*buf, uint file_id,
					 int event_server_id)
{
  fn_format(buf,"SQL_LOAD-",slave_load_tmpdir, "", MY_UNPACK_FILENAME);
  buf = strend(buf);
  buf = int10_to_str(::server_id, buf, 10);
  *buf++ = '-';
  buf = int10_to_str(event_server_id, buf, 10);
  *buf++ = '-';
  return int10_to_str(file_id, buf, 10);
}

#endif

const char* Log_event::get_type_str()
{
  switch(get_type_code()) {
  case START_EVENT:  return "Start";
  case STOP_EVENT:   return "Stop";
  case QUERY_EVENT:  return "Query";
  case ROTATE_EVENT: return "Rotate";
  case INTVAR_EVENT: return "Intvar";
  case LOAD_EVENT:   return "Load";
  case NEW_LOAD_EVENT:   return "New_load";
  case SLAVE_EVENT:  return "Slave";
  case CREATE_FILE_EVENT: return "Create_file";
  case APPEND_BLOCK_EVENT: return "Append_block";
  case DELETE_FILE_EVENT: return "Delete_file";
  case EXEC_LOAD_EVENT: return "Exec_load";
  default: /* impossible */ return "Unknown";
  }
}

#ifndef MYSQL_CLIENT
Log_event::Log_event(THD* thd_arg, uint16 flags_arg, bool using_trans)
  :log_pos(0), temp_buf(0), exec_time(0), cached_event_len(0), 
   flags(flags_arg), thd(thd_arg)
{
  server_id = thd->server_id;
  when = thd->start_time;
  cache_stmt= (using_trans &&
	       (thd->options & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN)));
}


Log_event::Log_event()
  :temp_buf(0), exec_time(0), cached_event_len(0), flags(0), cache_stmt(0),
   thd(0)
{
  server_id = ::server_id;
  when = time(NULL);
  log_pos=0;
}

/*
  Delete all temporary files used for SQL_LOAD.
*/

static void cleanup_load_tmpdir()
{
  MY_DIR *dirp;
  FILEINFO *file;
  uint i;
  char fname[FN_REFLEN];
  char prefbuf[31];
  char *p;
  
  if (!(dirp=my_dir(slave_load_tmpdir,MYF(MY_WME))))
    return;
  
  /* 
     When we are deleting temporary files, we should only remove
     the files associated with the server id of our server.
     We don't use event_server_id here because since we've disabled
     direct binlogging of Create_file/Append_file/Exec_load events
     we cannot meet Start_log event in the middle of events from one 
     LOAD DATA.
  */
  p= strmake(prefbuf,"SQL_LOAD-",9);
  p= int10_to_str(::server_id, p, 10);
  *(p++)= '-';
  *p= 0;

  for (i=0 ; i < (uint)dirp->number_off_files; i++)
  {
    file=dirp->dir_entry+i;
    if (is_prefix(file->name, prefbuf))
    {
      fn_format(fname,file->name,slave_load_tmpdir,"",MY_UNPACK_FILENAME);
      my_delete(fname, MYF(0));
    }
  }

  my_dirend(dirp);
}
#endif


Log_event::Log_event(const char* buf, bool old_format)
  :temp_buf(0), cached_event_len(0), cache_stmt(0)
{
  when = uint4korr(buf);
  server_id = uint4korr(buf + SERVER_ID_OFFSET);
  if (old_format)
  {
    log_pos=0;
    flags=0;
  }
  else
  {
    log_pos = uint4korr(buf + LOG_POS_OFFSET);
    flags = uint2korr(buf + FLAGS_OFFSET);
  }
#ifndef MYSQL_CLIENT
  thd = 0;
#endif  
}


#ifndef MYSQL_CLIENT

int Log_event::exec_event(struct st_relay_log_info* rli)
{
  /*
    rli is null when (as far as I (Guilhem) know)
    the caller is
    Load_log_event::exec_event *and* that one is called from
    Execute_load_log_event::exec_event. 
    In this case, we don't do anything here ;
    Execute_load_log_event::exec_event will call Log_event::exec_event
    again later with the proper rli.
    Strictly speaking, if we were sure that rli is null
    only in the case discussed above, 'if (rli)' is useless here.
    But as we are not 100% sure, keep it for now.
  */
  if (rli)  
  {
    if (rli->inside_transaction)
      rli->inc_pending(get_event_len());
    else
    {
      rli->inc_pos(get_event_len(),log_pos);
      flush_relay_log_info(rli);
    }
  }
  return 0;
}

void Log_event::pack_info(String* packet)
{
  net_store_data(packet, "", 0);
}

void Query_log_event::pack_info(String* packet)
{
  char buf[256];
  String tmp(buf, sizeof(buf));
  tmp.length(0);
  if (db && db_len)
  {
   tmp.append("use `",5);
   tmp.append(db, db_len);
   tmp.append("`; ", 3);
  }

  if (query && q_len)
    tmp.append(query, q_len);
  net_store_data(packet, (char*)tmp.ptr(), tmp.length());
}

void Start_log_event::pack_info(String* packet)
{
  char buf1[256];
  String tmp(buf1, sizeof(buf1));
  tmp.length(0);
  char buf[22];

  tmp.append("Server ver: ");
  tmp.append(server_version);
  tmp.append(", Binlog ver: ");
  tmp.append(llstr(binlog_version, buf));
  net_store_data(packet, tmp.ptr(), tmp.length());
}

void Load_log_event::pack_info(String* packet)
{
  char buf[256];
  String tmp(buf, sizeof(buf));
  tmp.length(0);
  if (db && db_len)
  {
   tmp.append("use ");
   tmp.append(db, db_len);
   tmp.append("; ", 2);
  }

  tmp.append("LOAD DATA INFILE '");
  tmp.append(fname, fname_len);
  tmp.append("' ", 2);
  if (sql_ex.opt_flags & REPLACE_FLAG)
    tmp.append(" REPLACE ");
  else if (sql_ex.opt_flags & IGNORE_FLAG)
    tmp.append(" IGNORE ");
  
  tmp.append("INTO TABLE ");
  tmp.append(table_name);
  if (sql_ex.field_term_len)
  {
    tmp.append(" FIELDS TERMINATED BY ");
    pretty_print_str(&tmp, sql_ex.field_term, sql_ex.field_term_len);
  }

  if (sql_ex.enclosed_len)
  {
    if (sql_ex.opt_flags & OPT_ENCLOSED_FLAG )
      tmp.append(" OPTIONALLY ");
    tmp.append( " ENCLOSED BY ");
    pretty_print_str(&tmp, sql_ex.enclosed, sql_ex.enclosed_len);
  }
     
  if (sql_ex.escaped_len)
  {
    tmp.append( " ESCAPED BY ");
    pretty_print_str(&tmp, sql_ex.escaped, sql_ex.escaped_len);
  }
     
  bool line_lexem_added= false;
  if (sql_ex.line_term_len)
  {
    tmp.append(" LINES TERMINATED BY ");
    pretty_print_str(&tmp, sql_ex.line_term, sql_ex.line_term_len);
    line_lexem_added= true;
  }

  if (sql_ex.line_start_len)
  {
    if (!line_lexem_added)
      tmp.append(" LINES");
    tmp.append(" STARTING BY ");
    pretty_print_str(&tmp, sql_ex.line_start, sql_ex.line_start_len);
  }
     
  if ((long) skip_lines > 0)
  {
    char nr_buff[32], *end;
    tmp.append( " IGNORE ");
    end= longlong10_to_str((longlong) skip_lines, nr_buff, 10);
    tmp.append(nr_buff, (uint) (end-nr_buff));
    tmp.append( " LINES");
  }

  if (num_fields)
  {
    uint i;
    const char* field = fields;
    tmp.append(" (");
    for (i = 0; i < num_fields; i++)
    {
      if (i)
	tmp.append(" ,");
      tmp.append( field);
	  
      field += field_lens[i]  + 1;
    }
    tmp.append(')');
  }

  net_store_data(packet, tmp.ptr(), tmp.length());
}

void Rotate_log_event::pack_info(String* packet)
{
  char buf1[256], buf[22];
  String tmp(buf1, sizeof(buf1));
  tmp.length(0);
  tmp.append(new_log_ident, ident_len);
  tmp.append(";pos=");
  tmp.append(llstr(pos,buf));
  if (flags & LOG_EVENT_FORCED_ROTATE_F)
    tmp.append("; forced by master");
  net_store_data(packet, tmp.ptr(), tmp.length());
}

void Intvar_log_event::pack_info(String* packet)
{
  char buf1[256], buf[22];
  String tmp(buf1, sizeof(buf1));
  tmp.length(0);
  tmp.append(get_var_type_name());
  tmp.append('=');
  tmp.append(llstr(val, buf));
  net_store_data(packet, tmp.ptr(), tmp.length());
}

void Rand_log_event::pack_info(String* packet)
{
  char buf1[256], *pos;
  pos=strmov(buf1,"rand_seed1=");
  pos=int10_to_str((long) seed1, pos, 10);
  pos=strmov(pos, ",rand_seed2=");
  pos=int10_to_str((long) seed2, pos, 10);
  net_store_data(packet, buf1, (uint) (pos-buf1));
}

void Slave_log_event::pack_info(String* packet)
{
  char buf1[256], buf[22], *end;
  String tmp(buf1, sizeof(buf1));
  tmp.length(0);
  tmp.append("host=");
  tmp.append(master_host);
  tmp.append(",port=");
  end= int10_to_str((long) master_port, buf, 10);
  tmp.append(buf, (uint32) (end-buf));
  tmp.append(",log=");
  tmp.append(master_log);
  tmp.append(",pos=");
  tmp.append(llstr(master_pos,buf));
  net_store_data(packet, tmp.ptr(), tmp.length());
}


void Log_event::init_show_field_list(List<Item>* field_list)
{
  field_list->push_back(new Item_empty_string("Log_name", 20));
  field_list->push_back(new Item_empty_string("Pos", 20));
  field_list->push_back(new Item_empty_string("Event_type", 20));
  field_list->push_back(new Item_empty_string("Server_id", 20));
  field_list->push_back(new Item_empty_string("Orig_log_pos", 20));
  field_list->push_back(new Item_empty_string("Info", 20));
}

/*
 * only called by SHOW BINLOG EVENTS
 */
int Log_event::net_send(THD* thd_arg, const char* log_name, my_off_t pos)
{
  String* packet = &thd_arg->packet;
  const char* p = strrchr(log_name, FN_LIBCHAR);
  const char* event_type;
  if (p)
    log_name = p + 1;
  
  packet->length(0);
  net_store_data(packet, log_name, strlen(log_name));
  net_store_data(packet, (longlong) pos);
  event_type = get_type_str();
  net_store_data(packet, event_type, strlen(event_type));
  net_store_data(packet, server_id);
  net_store_data(packet, (longlong) log_pos);
  pack_info(packet);
  return my_net_write(&thd_arg->net, (char*) packet->ptr(), packet->length());
}

#endif /* MYSQL_CLIENT */


int Query_log_event::write(IO_CACHE* file)
{
  return query ? Log_event::write(file) : -1; 
}


int Log_event::write(IO_CACHE* file)
{
  return (write_header(file) || write_data(file)) ? -1 : 0;
}


int Log_event::write_header(IO_CACHE* file)
{
  char buf[LOG_EVENT_HEADER_LEN];
  char* pos = buf;
  int4store(pos, (ulong) when); // timestamp
  pos += 4;
  *pos++ = get_type_code(); // event type code
  int4store(pos, server_id);
  pos += 4;
  long tmp=get_data_size() + LOG_EVENT_HEADER_LEN;
  int4store(pos, tmp);
  pos += 4;
  int4store(pos, log_pos);
  pos += 4;
  int2store(pos, flags);
  pos += 2;
  return (my_b_safe_write(file, (byte*) buf, (uint) (pos - buf)));
}

#ifndef MYSQL_CLIENT

int Log_event::read_log_event(IO_CACHE* file, String* packet,
			      pthread_mutex_t* log_lock)
{
  ulong data_len;
  int result=0;
  char buf[LOG_EVENT_HEADER_LEN];
  DBUG_ENTER("read_log_event");

  if (log_lock)
    pthread_mutex_lock(log_lock);
  if (my_b_read(file, (byte*) buf, sizeof(buf)))
  {
    /*
      If the read hits eof, we must report it as eof so the caller
      will know it can go into cond_wait to be woken up on the next
      update to the log.
    */
    DBUG_PRINT("error",("file->error: %d", file->error));
    if (!file->error)
      result= LOG_READ_EOF;
    else
      result= (file->error > 0 ? LOG_READ_TRUNC : LOG_READ_IO);
    goto end;
  }
  data_len= uint4korr(buf + EVENT_LEN_OFFSET);
  if (data_len < LOG_EVENT_HEADER_LEN ||
      data_len > current_thd->variables.max_allowed_packet)
  {
    DBUG_PRINT("error",("data_len: %ld", data_len));
    result= ((data_len < LOG_EVENT_HEADER_LEN) ? LOG_READ_BOGUS :
	     LOG_READ_TOO_LARGE);
    goto end;
  }
  packet->append(buf, sizeof(buf));
  data_len-= LOG_EVENT_HEADER_LEN;
  if (data_len)
  {
    if (packet->append(file, data_len))
    {
      /*
	Here we should never hit EOF in a non-error condition.
	EOF means we are reading the event partially, which should
	never happen.
      */
      result= file->error >= 0 ? LOG_READ_TRUNC: LOG_READ_IO;
      /* Implicit goto end; */
    }
  }

end:
  if (log_lock)
    pthread_mutex_unlock(log_lock);
  DBUG_RETURN(result);
}

#endif // MYSQL_CLIENT

#ifndef MYSQL_CLIENT
#define UNLOCK_MUTEX if (log_lock) pthread_mutex_unlock(log_lock);
#define LOCK_MUTEX if (log_lock) pthread_mutex_lock(log_lock);
#define max_allowed_packet current_thd->variables.max_allowed_packet
#else
#define UNLOCK_MUTEX
#define LOCK_MUTEX
#endif

// allocates memory - the caller is responsible for clean-up
#ifndef MYSQL_CLIENT
Log_event* Log_event::read_log_event(IO_CACHE* file,
				     pthread_mutex_t* log_lock,
				     bool old_format)
#else
Log_event* Log_event::read_log_event(IO_CACHE* file, bool old_format)
#endif  
{
  char head[LOG_EVENT_HEADER_LEN];
  uint header_size= old_format ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;

  LOCK_MUTEX;
  if (my_b_read(file, (byte *) head, header_size))
  {
    UNLOCK_MUTEX;
    return 0;
  }

  uint data_len = uint4korr(head + EVENT_LEN_OFFSET);
  char *buf= 0;
  const char *error= 0;
  Log_event *res=  0;

  if (data_len > max_allowed_packet)
  {
    error = "Event too big";
    goto err;
  }

  if (data_len < header_size)
  {
    error = "Event too small";
    goto err;
  }

  // some events use the extra byte to null-terminate strings
  if (!(buf = my_malloc(data_len+1, MYF(MY_WME))))
  {
    error = "Out of memory";
    goto err;
  }
  buf[data_len] = 0;
  memcpy(buf, head, header_size);
  if (my_b_read(file, (byte*) buf + header_size, data_len - header_size))
  {
    error = "read error";
    goto err;
  }
  if ((res = read_log_event(buf, data_len, &error, old_format)))
    res->register_temp_buf(buf);

err:
  UNLOCK_MUTEX;
  if (error)
  {
    sql_print_error("\
Error in Log_event::read_log_event(): '%s', data_len: %d, event_type: %d",
		    error,data_len,head[EVENT_TYPE_OFFSET]);
    my_free(buf, MYF(MY_ALLOW_ZERO_PTR));
    /*
      The SQL slave thread will check if file->error<0 to know
      if there was an I/O error. Even if there is no "low-level" I/O errors
      with 'file', any of the high-level above errors is worrying
      enough to stop the SQL thread now ; as we are skipping the current event,
      going on with reading and successfully executing other events can
      only corrupt the slave's databases. So stop.
    */
    file->error= -1;
  }
  return res;
}


Log_event* Log_event::read_log_event(const char* buf, int event_len,
				     const char **error, bool old_format)
{
  if (event_len < EVENT_LEN_OFFSET ||
      (uint) event_len != uint4korr(buf+EVENT_LEN_OFFSET))
  {
    *error="Sanity check failed";		// Needed to free buffer
    return NULL; // general sanity check - will fail on a partial read
  }
  
  Log_event* ev = NULL;
  
  switch(buf[EVENT_TYPE_OFFSET]) {
  case QUERY_EVENT:
    ev  = new Query_log_event(buf, event_len, old_format);
    break;
  case LOAD_EVENT:
    ev = new Create_file_log_event(buf, event_len, old_format);
    break;
  case NEW_LOAD_EVENT:
    ev = new Load_log_event(buf, event_len, old_format);
    break;
  case ROTATE_EVENT:
    ev = new Rotate_log_event(buf, event_len, old_format);
    break;
  case SLAVE_EVENT:
    ev = new Slave_log_event(buf, event_len);
    break;
  case CREATE_FILE_EVENT:
    ev = new Create_file_log_event(buf, event_len, old_format);
    break;
  case APPEND_BLOCK_EVENT:
    ev = new Append_block_log_event(buf, event_len);
    break;
  case DELETE_FILE_EVENT:
    ev = new Delete_file_log_event(buf, event_len);
    break;
  case EXEC_LOAD_EVENT:
    ev = new Execute_load_log_event(buf, event_len);
    break;
  case START_EVENT:
    ev = new Start_log_event(buf, old_format);
    break;
  case STOP_EVENT:
    ev = new Stop_log_event(buf, old_format);
    break;
  case INTVAR_EVENT:
    ev = new Intvar_log_event(buf, old_format);
    break;
  case RAND_EVENT:
    ev = new Rand_log_event(buf, old_format);
    break;
  default:
    break;
  }
  if (!ev || !ev->is_valid())
  {
    delete ev;
#ifdef MYSQL_CLIENT
    if (!force_opt)
    {
      *error= "Found invalid event in binary log";
      return 0;
    }
    ev= new Unknown_log_event(buf, old_format);
#else
    *error= "Found invalid event in binary log";
    return 0;
#endif
  }
  ev->cached_event_len = event_len;
  return ev;  
}


#ifdef MYSQL_CLIENT
void Log_event::print_header(FILE* file)
{
  char llbuff[22];
  fputc('#', file);
  print_timestamp(file);
  fprintf(file, " server id %d  log_pos %s ", server_id,
	  llstr(log_pos,llbuff)); 
}

void Log_event::print_timestamp(FILE* file, time_t* ts)
{
  struct tm *res;
  if (!ts)
    ts = &when;
#ifdef MYSQL_SERVER				// This is always false
  struct tm tm_tmp;
  localtime_r(ts,(res= &tm_tmp));
#else
  res=localtime(ts);
#endif

  fprintf(file,"%02d%02d%02d %2d:%02d:%02d",
	  res->tm_year % 100,
	  res->tm_mon+1,
	  res->tm_mday,
	  res->tm_hour,
	  res->tm_min,
	  res->tm_sec);
}


void Start_log_event::print(FILE* file, bool short_form, char* last_db)
{
  if (short_form)
    return;

  print_header(file);
  fprintf(file, "\tStart: binlog v %d, server v %s created ", binlog_version,
	  server_version);
  print_timestamp(file);
  if (created)
    fprintf(file," at startup");
  fputc('\n', file);
  fflush(file);
}

void Stop_log_event::print(FILE* file, bool short_form, char* last_db)
{
  if (short_form)
    return;

  print_header(file);
  fprintf(file, "\tStop\n");
  fflush(file);
}

void Rotate_log_event::print(FILE* file, bool short_form, char* last_db)
{
  char buf[22];
  if (short_form)
    return;

  print_header(file);
  fprintf(file, "\tRotate to ");
  if (new_log_ident)
    my_fwrite(file, (byte*) new_log_ident, (uint)ident_len, 
	      MYF(MY_NABP | MY_WME));
  fprintf(file, "  pos: %s", llstr(pos, buf));
  if (flags & LOG_EVENT_FORCED_ROTATE_F)
    fprintf(file,"  forced by master");
  fputc('\n', file);
  fflush(file);
}

#endif /* #ifdef MYSQL_CLIENT */


Start_log_event::Start_log_event(const char* buf,
				 bool old_format)
  :Log_event(buf, old_format)
{
  buf += (old_format) ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;
  binlog_version = uint2korr(buf+ST_BINLOG_VER_OFFSET);
  memcpy(server_version, buf+ST_SERVER_VER_OFFSET,
	 ST_SERVER_VER_LEN);
  created = uint4korr(buf+ST_CREATED_OFFSET);
}

int Start_log_event::write_data(IO_CACHE* file)
{
  char buff[START_HEADER_LEN];
  int2store(buff + ST_BINLOG_VER_OFFSET,binlog_version);
  memcpy(buff + ST_SERVER_VER_OFFSET,server_version,ST_SERVER_VER_LEN);
  int4store(buff + ST_CREATED_OFFSET,created);
  return (my_b_safe_write(file, (byte*) buff, sizeof(buff)) ? -1 : 0);
}


Rotate_log_event::Rotate_log_event(const char* buf, int event_len,
				   bool old_format)
  :Log_event(buf, old_format),new_log_ident(NULL),alloced(0)
{
  // The caller will ensure that event_len is what we have at EVENT_LEN_OFFSET
  int header_size = (old_format) ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;
  uint ident_offset;
  if (event_len < header_size)
    return;
  buf += header_size;
  if (old_format)
  {
    ident_len = (uint)(event_len - OLD_HEADER_LEN);
    pos = 4;
    ident_offset = 0;
  }
  else
  {
    ident_len = (uint)(event_len - ROTATE_EVENT_OVERHEAD);
    pos = uint8korr(buf + R_POS_OFFSET);
    ident_offset = ROTATE_HEADER_LEN;
  }
  set_if_smaller(ident_len,FN_REFLEN-1);
  if (!(new_log_ident= my_strdup_with_length((byte*) buf +
					     ident_offset,
					     (uint) ident_len,
					     MYF(MY_WME))))
    return;
  alloced = 1;
}


int Rotate_log_event::write_data(IO_CACHE* file)
{
  char buf[ROTATE_HEADER_LEN];
  int8store(buf + R_POS_OFFSET, pos);
  return (my_b_safe_write(file, (byte*)buf, ROTATE_HEADER_LEN) ||
	  my_b_safe_write(file, (byte*)new_log_ident, (uint) ident_len));
}


#ifndef MYSQL_CLIENT
Query_log_event::Query_log_event(THD* thd_arg, const char* query_arg,
				 ulong query_length, bool using_trans)
  :Log_event(thd_arg, 0, using_trans), data_buf(0), query(query_arg),
   db(thd_arg->db), q_len((uint32) query_length),
  error_code(thd_arg->killed ? ER_SERVER_SHUTDOWN: thd_arg->net.last_errno),
  thread_id(thd_arg->thread_id)
{
  time_t end_time;
  time(&end_time);
  exec_time = (ulong) (end_time  - thd->start_time);
  db_len = (db) ? (uint32) strlen(db) : 0;
}
#endif

Query_log_event::Query_log_event(const char* buf, int event_len,
				 bool old_format)
  :Log_event(buf, old_format),data_buf(0), query(NULL), db(NULL)
{
  ulong data_len;
  if (old_format)
  {
    if ((uint)event_len < OLD_HEADER_LEN + QUERY_HEADER_LEN)
      return;				
    data_len = event_len - (QUERY_HEADER_LEN + OLD_HEADER_LEN);
    buf += OLD_HEADER_LEN;
  }
  else
  {
    if ((uint)event_len < QUERY_EVENT_OVERHEAD)
      return;				
    data_len = event_len - QUERY_EVENT_OVERHEAD;
    buf += LOG_EVENT_HEADER_LEN;
  }

  exec_time = uint4korr(buf + Q_EXEC_TIME_OFFSET);
  error_code = uint2korr(buf + Q_ERR_CODE_OFFSET);

  if (!(data_buf = (char*) my_malloc(data_len + 1, MYF(MY_WME))))
    return;

  memcpy(data_buf, buf + Q_DATA_OFFSET, data_len);
  thread_id = uint4korr(buf + Q_THREAD_ID_OFFSET);
  db = data_buf;
  db_len = (uint)buf[Q_DB_LEN_OFFSET];
  query=data_buf + db_len + 1;
  q_len = data_len - 1 - db_len;
  *((char*)query+q_len) = 0;
}


#ifdef MYSQL_CLIENT

void Query_log_event::print(FILE* file, bool short_form, char* last_db)
{
  char buff[40],*end;				// Enough for SET TIMESTAMP
  if (!short_form)
  {
    print_header(file);
    fprintf(file, "\tQuery\tthread_id=%lu\texec_time=%lu\terror_code=%d\n",
	    (ulong) thread_id, (ulong) exec_time, error_code);
  }

  bool same_db = 0;

  if (db && last_db)
  {
    if (!(same_db = !memcmp(last_db, db, db_len + 1)))
      memcpy(last_db, db, db_len + 1);
  }
  
  if (db && db[0] && !same_db)
    fprintf(file, "use %s;\n", db);
  end=int10_to_str((long) when, strmov(buff,"SET TIMESTAMP="),10);
  *end++=';';
  *end++='\n';
  my_fwrite(file, (byte*) buff, (uint) (end-buff),MYF(MY_NABP | MY_WME));
  my_fwrite(file, (byte*) query, q_len, MYF(MY_NABP | MY_WME));
  fprintf(file, ";\n");
}
#endif


int Query_log_event::write_data(IO_CACHE* file)
{
  if (!query)
    return -1;
  
  char buf[QUERY_HEADER_LEN]; 
  int4store(buf + Q_THREAD_ID_OFFSET, thread_id);
  int4store(buf + Q_EXEC_TIME_OFFSET, exec_time);
  buf[Q_DB_LEN_OFFSET] = (char) db_len;
  int2store(buf + Q_ERR_CODE_OFFSET, error_code);

  return (my_b_safe_write(file, (byte*) buf, QUERY_HEADER_LEN) ||
	  my_b_safe_write(file, (db) ? (byte*) db : (byte*)"", db_len + 1) ||
	  my_b_safe_write(file, (byte*) query, q_len)) ? -1 : 0;
}

Intvar_log_event::Intvar_log_event(const char* buf, bool old_format)
  :Log_event(buf, old_format)
{
  buf += (old_format) ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;
  type = buf[I_TYPE_OFFSET];
  val = uint8korr(buf+I_VAL_OFFSET);
}

const char* Intvar_log_event::get_var_type_name()
{
  switch(type) {
  case LAST_INSERT_ID_EVENT: return "LAST_INSERT_ID";
  case INSERT_ID_EVENT: return "INSERT_ID";
  default: /* impossible */ return "UNKNOWN";
  }
}

int Intvar_log_event::write_data(IO_CACHE* file)
{
  char buf[9];
  buf[I_TYPE_OFFSET] = type;
  int8store(buf + I_VAL_OFFSET, val);
  return my_b_safe_write(file, (byte*) buf, sizeof(buf));
}

#ifdef MYSQL_CLIENT
void Intvar_log_event::print(FILE* file, bool short_form, char* last_db)
{
  char llbuff[22];
  const char *msg;
  LINT_INIT(msg);

  if (!short_form)
  {
    print_header(file);
    fprintf(file, "\tIntvar\n");
  }

  fprintf(file, "SET ");
  switch (type) {
  case LAST_INSERT_ID_EVENT:
    msg="LAST_INSERT_ID";
    break;
  case INSERT_ID_EVENT:
    msg="INSERT_ID";
    break;
  }
  fprintf(file, "%s=%s;\n", msg, llstr(val,llbuff));
  fflush(file);
}
#endif

/*****************************************************************************
 *
 *  Rand log event
 *
 ****************************************************************************/
Rand_log_event::Rand_log_event(const char* buf, bool old_format)
  :Log_event(buf, old_format)
{
  buf += (old_format) ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;
  seed1 = uint8korr(buf+RAND_SEED1_OFFSET);
  seed2 = uint8korr(buf+RAND_SEED2_OFFSET);
}

int Rand_log_event::write_data(IO_CACHE* file)
{
  char buf[16];
  int8store(buf + RAND_SEED1_OFFSET, seed1);
  int8store(buf + RAND_SEED2_OFFSET, seed2);
  return my_b_safe_write(file, (byte*) buf, sizeof(buf));
}

#ifdef MYSQL_CLIENT
void Rand_log_event::print(FILE* file, bool short_form, char* last_db)
{
  char llbuff[22],llbuff2[22];
  if (!short_form)
  {
    print_header(file);
    fprintf(file, "\tRand\n");
  }
  fprintf(file, "SET @@RAND_SEED1=%s, @@RAND_SEED2=%s;\n",
	  llstr(seed1, llbuff),llstr(seed2, llbuff2));
  fflush(file);
}
#endif

int Load_log_event::write_data_header(IO_CACHE* file)
{
  char buf[LOAD_HEADER_LEN];
  int4store(buf + L_THREAD_ID_OFFSET, thread_id);
  int4store(buf + L_EXEC_TIME_OFFSET, exec_time);
  int4store(buf + L_SKIP_LINES_OFFSET, skip_lines);
  buf[L_TBL_LEN_OFFSET] = (char)table_name_len;
  buf[L_DB_LEN_OFFSET] = (char)db_len;
  int4store(buf + L_NUM_FIELDS_OFFSET, num_fields);
  return my_b_safe_write(file, (byte*)buf, LOAD_HEADER_LEN);
}

int Load_log_event::write_data_body(IO_CACHE* file)
{
  if (sql_ex.write_data(file))
    return 1;
  if (num_fields && fields && field_lens)
  {
    if (my_b_safe_write(file, (byte*)field_lens, num_fields) ||
	my_b_safe_write(file, (byte*)fields, field_block_len))
      return 1;
  }
  return (my_b_safe_write(file, (byte*)table_name, table_name_len + 1) ||
	  my_b_safe_write(file, (byte*)db, db_len + 1) ||
	  my_b_safe_write(file, (byte*)fname, fname_len));
}



static bool write_str(IO_CACHE *file, char *str, byte length)
{
  return (my_b_safe_write(file, &length, 1) ||
	  my_b_safe_write(file, (byte*) str, (int) length));
}


int sql_ex_info::write_data(IO_CACHE* file)
{
  if (new_format())
  {
    return (write_str(file, field_term, field_term_len) ||
	    write_str(file, enclosed,   enclosed_len) ||
	    write_str(file, line_term,  line_term_len) ||
	    write_str(file, line_start, line_start_len) ||
	    write_str(file, escaped,    escaped_len) ||
	    my_b_safe_write(file,(byte*) &opt_flags,1));
  }
  else
  {
    old_sql_ex old_ex;
    old_ex.field_term= *field_term;
    old_ex.enclosed=   *enclosed;
    old_ex.line_term=  *line_term;
    old_ex.line_start= *line_start;
    old_ex.escaped=    *escaped;
    old_ex.opt_flags=  opt_flags;
    old_ex.empty_flags=empty_flags;
    return my_b_safe_write(file, (byte*) &old_ex, sizeof(old_ex));
  }
}


static inline int read_str(char * &buf, char *buf_end, char * &str,
			   uint8 &len)
{
  if (buf + (uint) (uchar) *buf >= buf_end)
    return 1;
  len = (uint8) *buf;
  str= buf+1;
  buf+= (uint) len+1;
  return 0;
}


char* sql_ex_info::init(char* buf,char* buf_end,bool use_new_format)
{
  cached_new_format = use_new_format;
  if (use_new_format)
  {
    empty_flags=0;
    /*
      The code below assumes that buf will not disappear from
      under our feet during the lifetime of the event. This assumption
      holds true in the slave thread if the log is in new format, but is not
      the case when we have old format because we will be reusing net buffer
      to read the actual file before we write out the Create_file event.
    */
    if (read_str(buf, buf_end, field_term, field_term_len) ||
	read_str(buf, buf_end, enclosed,   enclosed_len) ||
	read_str(buf, buf_end, line_term,  line_term_len) ||
	read_str(buf, buf_end, line_start, line_start_len) ||
	read_str(buf, buf_end, escaped,	   escaped_len))
      return 0;
    opt_flags = *buf++;
  }
  else
  {
    field_term_len= enclosed_len= line_term_len= line_start_len= escaped_len=1;
    field_term = buf++;			// Use first byte in string
    enclosed=	 buf++;
    line_term=   buf++;
    line_start=  buf++;
    escaped=     buf++;
    opt_flags =  *buf++;
    empty_flags= *buf++;
    if (empty_flags & FIELD_TERM_EMPTY)
      field_term_len=0;
    if (empty_flags & ENCLOSED_EMPTY)
      enclosed_len=0;
    if (empty_flags & LINE_TERM_EMPTY)
      line_term_len=0;
    if (empty_flags & LINE_START_EMPTY)
      line_start_len=0;
    if (empty_flags & ESCAPED_EMPTY)
      escaped_len=0;
  }
  return buf;
}


#ifndef MYSQL_CLIENT
Load_log_event::Load_log_event(THD* thd_arg, sql_exchange* ex,
			       const char* db_arg, const char* table_name_arg,
			       List<Item>& fields_arg,
			       enum enum_duplicates handle_dup,
			       bool using_trans)
  :Log_event(thd_arg, 0, using_trans),thread_id(thd_arg->thread_id),
  num_fields(0),fields(0),
  field_lens(0),field_block_len(0),
  table_name(table_name_arg ? table_name_arg : ""),
  db(db_arg), fname(ex->file_name)
{
  time_t end_time;
  time(&end_time);
  exec_time = (ulong) (end_time  - thd_arg->start_time);
  /* db can never be a zero pointer in 4.0 */
  db_len = (uint32) strlen(db);
  table_name_len = (uint32) strlen(table_name);
  fname_len = (fname) ? (uint) strlen(fname) : 0;
  sql_ex.field_term = (char*) ex->field_term->ptr();
  sql_ex.field_term_len = (uint8) ex->field_term->length();
  sql_ex.enclosed = (char*) ex->enclosed->ptr();
  sql_ex.enclosed_len = (uint8) ex->enclosed->length();
  sql_ex.line_term = (char*) ex->line_term->ptr();
  sql_ex.line_term_len = (uint8) ex->line_term->length();
  sql_ex.line_start = (char*) ex->line_start->ptr();
  sql_ex.line_start_len = (uint8) ex->line_start->length();
  sql_ex.escaped = (char*) ex->escaped->ptr();
  sql_ex.escaped_len = (uint8) ex->escaped->length();
  sql_ex.opt_flags = 0;
  sql_ex.cached_new_format = -1;
    
  if (ex->dumpfile)
    sql_ex.opt_flags|= DUMPFILE_FLAG;
  if (ex->opt_enclosed)
    sql_ex.opt_flags|= OPT_ENCLOSED_FLAG;

  sql_ex.empty_flags = 0;

  switch (handle_dup) {
  case DUP_IGNORE: sql_ex.opt_flags|= IGNORE_FLAG; break;
  case DUP_REPLACE: sql_ex.opt_flags|= REPLACE_FLAG; break;
  case DUP_ERROR: break;	
  }

  if (!ex->field_term->length())
    sql_ex.empty_flags|= FIELD_TERM_EMPTY;
  if (!ex->enclosed->length())
    sql_ex.empty_flags|= ENCLOSED_EMPTY;
  if (!ex->line_term->length())
    sql_ex.empty_flags|= LINE_TERM_EMPTY;
  if (!ex->line_start->length())
    sql_ex.empty_flags|= LINE_START_EMPTY;
  if (!ex->escaped->length())
    sql_ex.empty_flags|= ESCAPED_EMPTY;
    
  skip_lines = ex->skip_lines;

  List_iterator<Item> li(fields_arg);
  field_lens_buf.length(0);
  fields_buf.length(0);
  Item* item;
  while ((item = li++))
  {
    num_fields++;
    uchar len = (uchar) strlen(item->name);
    field_block_len += len + 1;
    fields_buf.append(item->name, len + 1);
    field_lens_buf.append((char*)&len, 1);
  }

  field_lens = (const uchar*)field_lens_buf.ptr();
  fields = fields_buf.ptr();
}

#endif

/*
  The caller must do buf[event_len] = 0 before he starts using the
  constructed event.
*/

Load_log_event::Load_log_event(const char* buf, int event_len,
			       bool old_format)
  :Log_event(buf, old_format),num_fields(0),fields(0),
  field_lens(0),field_block_len(0),
  table_name(0),db(0),fname(0)
{
  if (!event_len) // derived class, will call copy_log_event() itself
    return;
  copy_log_event(buf, event_len, old_format);
}

int Load_log_event::copy_log_event(const char *buf, ulong event_len,
				   bool old_format)
{
  uint data_len;
  char* buf_end = (char*)buf + event_len;
  uint header_len= old_format ? OLD_HEADER_LEN : LOG_EVENT_HEADER_LEN;
  const char* data_head = buf + header_len;
  thread_id = uint4korr(data_head + L_THREAD_ID_OFFSET);
  exec_time = uint4korr(data_head + L_EXEC_TIME_OFFSET);
  skip_lines = uint4korr(data_head + L_SKIP_LINES_OFFSET);
  table_name_len = (uint)data_head[L_TBL_LEN_OFFSET];
  db_len = (uint)data_head[L_DB_LEN_OFFSET];
  num_fields = uint4korr(data_head + L_NUM_FIELDS_OFFSET);
	  
  int body_offset = ((buf[EVENT_TYPE_OFFSET] == LOAD_EVENT) ?
		     LOAD_HEADER_LEN + header_len :
		     get_data_body_offset());
  
  if ((int) event_len < body_offset)
    return 1;
  /*
    Sql_ex.init() on success returns the pointer to the first byte after
    the sql_ex structure, which is the start of field lengths array.
  */
  if (!(field_lens=(uchar*)sql_ex.init((char*)buf + body_offset,
		  buf_end,
		  buf[EVENT_TYPE_OFFSET] != LOAD_EVENT)))
    return 1;
  
  data_len = event_len - body_offset;
  if (num_fields > data_len) // simple sanity check against corruption
    return 1;
  for (uint i = 0; i < num_fields; i++)
    field_block_len += (uint)field_lens[i] + 1;

  fields = (char*)field_lens + num_fields;
  table_name  = fields + field_block_len;
  db = table_name + table_name_len + 1;
  fname = db + db_len + 1;
  fname_len = strlen(fname);
  // null termination is accomplished by the caller doing buf[event_len]=0
  return 0;
}

#ifdef MYSQL_CLIENT

void Load_log_event::print(FILE* file, bool short_form, char* last_db)
{
  print(file, short_form, last_db, 0);
}

void Load_log_event::print(FILE* file, bool short_form, char* last_db, bool commented)
{
  if (!short_form)
  {
    print_header(file);
    fprintf(file, "\tQuery\tthread_id=%ld\texec_time=%ld\n",
	    thread_id, exec_time);
  }

  bool same_db = 0;
  if (db && last_db)
  {
    if (!(same_db = !memcmp(last_db, db, db_len + 1)))
      memcpy(last_db, db, db_len + 1);
  }
  
  if (db && db[0] && !same_db)
    fprintf(file, "%suse %s;\n", 
            commented ? "# " : "",
            db);

  fprintf(file, "%sLOAD DATA ",
          commented ? "# " : "");
  if (check_fname_outside_temp_buf())
    fprintf(file, "LOCAL ");
  fprintf(file, "INFILE '%-*s' ", fname_len, fname);

  if (sql_ex.opt_flags & REPLACE_FLAG )
    fprintf(file," REPLACE ");
  else if (sql_ex.opt_flags & IGNORE_FLAG )
    fprintf(file," IGNORE ");
  
  fprintf(file, "INTO TABLE %s ", table_name);
  if (sql_ex.field_term)
  {
    fprintf(file, " FIELDS TERMINATED BY ");
    pretty_print_str(file, sql_ex.field_term, sql_ex.field_term_len);
  }

  if (sql_ex.enclosed)
  {
    if (sql_ex.opt_flags & OPT_ENCLOSED_FLAG )
      fprintf(file," OPTIONALLY ");
    fprintf(file, " ENCLOSED BY ");
    pretty_print_str(file, sql_ex.enclosed, sql_ex.enclosed_len);
  }
     
  if (sql_ex.escaped)
  {
    fprintf(file, " ESCAPED BY ");
    pretty_print_str(file, sql_ex.escaped, sql_ex.escaped_len);
  }
     
  bool line_lexem_added= false;
  if (sql_ex.line_term)
  {
    fprintf(file," LINES TERMINATED BY ");
    pretty_print_str(file, sql_ex.line_term, sql_ex.line_term_len);
    line_lexem_added= true;
  }

  if (sql_ex.line_start)
  {
    if (!line_lexem_added)
      fprintf(file," LINES");
    fprintf(file," STARTING BY ");
    pretty_print_str(file, sql_ex.line_start, sql_ex.line_start_len);
  }
     
  if ((long) skip_lines > 0)
    fprintf(file, " IGNORE %ld LINES", (long) skip_lines);

  if (num_fields)
  {
    uint i;
    const char* field = fields;
    fprintf(file, " (");
    for (i = 0; i < num_fields; i++)
    {
      if (i)
	fputc(',', file);
      fprintf(file, field);
	  
      field += field_lens[i]  + 1;
    }
    fputc(')', file);
  }

  fprintf(file, ";\n");
}

#endif /* #ifdef MYSQL_CLIENT */

#ifndef MYSQL_CLIENT

void Log_event::set_log_pos(MYSQL_LOG* log)
{
  if (!log_pos)
    log_pos = my_b_tell(&log->log_file);
}


void Load_log_event::set_fields(List<Item> &field_list)
{
  uint i;
  const char *field= fields;
  for (i= 0; i < num_fields; i++)
  {
    field_list.push_back(new Item_field(db, table_name, field));	  
    field+= field_lens[i]  + 1;
  }
}


Slave_log_event::Slave_log_event(THD* thd_arg,
				 struct st_relay_log_info* rli):
  Log_event(thd_arg,0,0),mem_pool(0),master_host(0)
{
  DBUG_ENTER("Slave_log_event");
  if (!rli->inited)				// QQ When can this happen ?
    DBUG_VOID_RETURN;
  
  MASTER_INFO* mi = rli->mi;
  // TODO: re-write this better without holding both locks at the same time
  pthread_mutex_lock(&mi->data_lock);
  pthread_mutex_lock(&rli->data_lock);
  master_host_len = strlen(mi->host);
  master_log_len = strlen(rli->master_log_name);
  // on OOM, just do not initialize the structure and print the error
  if ((mem_pool = (char*)my_malloc(get_data_size() + 1,
				   MYF(MY_WME))))
  {
    master_host = mem_pool + SL_MASTER_HOST_OFFSET ;
    memcpy(master_host, mi->host, master_host_len + 1);
    master_log = master_host + master_host_len + 1;
    memcpy(master_log, rli->master_log_name, master_log_len + 1);
    master_port = mi->port;
    master_pos = rli->master_log_pos;
    DBUG_PRINT("info", ("master_log: %s  pos: %d", master_log,
			(ulong) master_pos));
  }
  else
    sql_print_error("Out of memory while recording slave event");
  pthread_mutex_unlock(&rli->data_lock);
  pthread_mutex_unlock(&mi->data_lock);
  DBUG_VOID_RETURN;
}

#endif /* ! MYSQL_CLIENT */


Slave_log_event::~Slave_log_event()
{
  my_free(mem_pool, MYF(MY_ALLOW_ZERO_PTR));
}

#ifdef MYSQL_CLIENT

void Slave_log_event::print(FILE* file, bool short_form, char* last_db)
{
  char llbuff[22];
  if (short_form)
    return;
  print_header(file);
  fputc('\n', file);
  fprintf(file, "\
Slave: master_host: '%s'  master_port: %d  master_log: '%s'  master_pos: %s\n",
	  master_host, master_port, master_log, llstr(master_pos, llbuff));
}

#endif /* MYSQL_CLIENT */

int Slave_log_event::get_data_size()
{
  return master_host_len + master_log_len + 1 + SL_MASTER_HOST_OFFSET;
}

int Slave_log_event::write_data(IO_CACHE* file)
{
  int8store(mem_pool + SL_MASTER_POS_OFFSET, master_pos);
  int2store(mem_pool + SL_MASTER_PORT_OFFSET, master_port);
  // log and host are already there
  return my_b_safe_write(file, (byte*)mem_pool, get_data_size());
}


void Slave_log_event::init_from_mem_pool(int data_size)
{
  master_pos = uint8korr(mem_pool + SL_MASTER_POS_OFFSET);
  master_port = uint2korr(mem_pool + SL_MASTER_PORT_OFFSET);
  master_host = mem_pool + SL_MASTER_HOST_OFFSET;
  master_host_len = strlen(master_host);
  // safety
  master_log = master_host + master_host_len + 1;
  if (master_log > mem_pool + data_size)
  {
    master_host = 0;
    return;
  }
  master_log_len = strlen(master_log);
}

Slave_log_event::Slave_log_event(const char* buf, int event_len)
  :Log_event(buf,0),mem_pool(0),master_host(0)
{
  event_len -= LOG_EVENT_HEADER_LEN;
  if (event_len < 0)
    return;
  if (!(mem_pool = (char*) my_malloc(event_len + 1, MYF(MY_WME))))
    return;
  memcpy(mem_pool, buf + LOG_EVENT_HEADER_LEN, event_len);
  mem_pool[event_len] = 0;
  init_from_mem_pool(event_len);
}

#ifndef MYSQL_CLIENT
Create_file_log_event::
Create_file_log_event(THD* thd_arg, sql_exchange* ex,
		      const char* db_arg, const char* table_name_arg,
		      List<Item>& fields_arg, enum enum_duplicates handle_dup,
		      char* block_arg, uint block_len_arg, bool using_trans)
  :Load_log_event(thd_arg,ex,db_arg,table_name_arg,fields_arg,handle_dup,
		  using_trans),
   fake_base(0),block(block_arg),block_len(block_len_arg),
   file_id(thd_arg->file_id = mysql_bin_log.next_file_id())
{
  sql_ex.force_new_format();
}
#endif

int Create_file_log_event::write_data_body(IO_CACHE* file)
{
  int res;
  if ((res = Load_log_event::write_data_body(file)) || fake_base)
    return res;
  return (my_b_safe_write(file, (byte*) "", 1) ||
	  my_b_safe_write(file, (byte*) block, block_len));
}

int Create_file_log_event::write_data_header(IO_CACHE* file)
{
  int res;
  if ((res = Load_log_event::write_data_header(file)) || fake_base)
    return res;
  byte buf[CREATE_FILE_HEADER_LEN];
  int4store(buf + CF_FILE_ID_OFFSET, file_id);
  return my_b_safe_write(file, buf, CREATE_FILE_HEADER_LEN);
}

int Create_file_log_event::write_base(IO_CACHE* file)
{
  int res;
  fake_base = 1; // pretend we are Load event
  res = write(file);
  fake_base = 0;
  return res;
}

Create_file_log_event::Create_file_log_event(const char* buf, int len,
					     bool old_format)
  :Load_log_event(buf,0,old_format),fake_base(0),block(0),inited_from_old(0)
{
  int block_offset;
  if (copy_log_event(buf,len,old_format))
    return;
  if (!old_format)
  {
    file_id = uint4korr(buf + LOG_EVENT_HEADER_LEN +
			+ LOAD_HEADER_LEN + CF_FILE_ID_OFFSET);
    // + 1 for \0 terminating fname  
    block_offset = (LOG_EVENT_HEADER_LEN + Load_log_event::get_data_size() +
		    CREATE_FILE_HEADER_LEN + 1);
    if (len < block_offset)
      return;
    block = (char*)buf + block_offset;
    block_len = len - block_offset;
  }
  else
  {
    sql_ex.force_new_format();
    inited_from_old = 1;
  }
}


#ifdef MYSQL_CLIENT
void Create_file_log_event::print(FILE* file, bool short_form, 
				  char* last_db, bool enable_local)
{
  if (short_form)
  {
    if (enable_local && check_fname_outside_temp_buf())
      Load_log_event::print(file, 1, last_db);
    return;
  }

  if (enable_local)
  {
    Load_log_event::print(file, 1, last_db, !check_fname_outside_temp_buf());
    /* 
       That one is for "file_id: etc" below: in mysqlbinlog we want the #, in
       SHOW BINLOG EVENTS we don't.
    */
    fprintf(file, "#"); 
  }

  fprintf(file, " file_id: %d  block_len: %d\n", file_id, block_len);
}

void Create_file_log_event::print(FILE* file, bool short_form,
				  char* last_db)
{
  print(file,short_form,last_db,0);
}
#endif

#ifndef MYSQL_CLIENT
void Create_file_log_event::pack_info(String* packet)
{
  char buf1[256],buf[22], *end;
  String tmp(buf1, sizeof(buf1));
  tmp.length(0);
  tmp.append("db=");
  tmp.append(db, db_len);
  tmp.append(";table=");
  tmp.append(table_name, table_name_len);
  tmp.append(";file_id=");
  end= int10_to_str((long) file_id, buf, 10);
  tmp.append(buf, (uint32) (end-buf));
  tmp.append(";block_len=");
  end= int10_to_str((long) block_len, buf, 10);
  tmp.append(buf, (uint32) (end-buf));
  net_store_data(packet, (char*) tmp.ptr(), tmp.length());
}
#endif  

#ifndef MYSQL_CLIENT  
Append_block_log_event::Append_block_log_event(THD* thd_arg, const char* db_arg,
                                               char* block_arg,
					       uint block_len_arg,
					       bool using_trans)
  :Log_event(thd_arg,0, using_trans), block(block_arg),
   block_len(block_len_arg), file_id(thd_arg->file_id), db(db_arg)
{
}
#endif  
  
Append_block_log_event::Append_block_log_event(const char* buf, int len)
  :Log_event(buf, 0),block(0)
{
  if ((uint)len < APPEND_BLOCK_EVENT_OVERHEAD)
    return;
  file_id = uint4korr(buf + LOG_EVENT_HEADER_LEN + AB_FILE_ID_OFFSET);
  block = (char*)buf + APPEND_BLOCK_EVENT_OVERHEAD;
  block_len = len - APPEND_BLOCK_EVENT_OVERHEAD;
}

int Append_block_log_event::write_data(IO_CACHE* file)
{
  byte buf[APPEND_BLOCK_HEADER_LEN];
  int4store(buf + AB_FILE_ID_OFFSET, file_id);
  return (my_b_safe_write(file, buf, APPEND_BLOCK_HEADER_LEN) ||
	  my_b_safe_write(file, (byte*) block, block_len));
}

#ifdef MYSQL_CLIENT  
void Append_block_log_event::print(FILE* file, bool short_form,
				   char* last_db)
{
  if (short_form)
    return;
  print_header(file);
  fputc('\n', file);
  fprintf(file, "#Append_block: file_id: %d  block_len: %d\n",
	  file_id, block_len);
}
#endif  

#ifndef MYSQL_CLIENT
void Append_block_log_event::pack_info(String* packet)
{
  char buf1[256];
  sprintf(buf1, ";file_id=%u;block_len=%u", file_id, block_len);
  net_store_data(packet, buf1);
}

Delete_file_log_event::Delete_file_log_event(THD* thd_arg, const char* db_arg,
                                             bool using_trans) 
  :Log_event(thd_arg, 0, using_trans), file_id(thd_arg->file_id), db(db_arg)
{
}
#endif  


Delete_file_log_event::Delete_file_log_event(const char* buf, int len)
  :Log_event(buf, 0),file_id(0)
{
  if ((uint)len < DELETE_FILE_EVENT_OVERHEAD)
    return;
  file_id = uint4korr(buf + LOG_EVENT_HEADER_LEN + AB_FILE_ID_OFFSET);
}


int Delete_file_log_event::write_data(IO_CACHE* file)
{
 byte buf[DELETE_FILE_HEADER_LEN];
 int4store(buf + DF_FILE_ID_OFFSET, file_id);
 return my_b_safe_write(file, buf, DELETE_FILE_HEADER_LEN);
}

#ifdef MYSQL_CLIENT  
void Delete_file_log_event::print(FILE* file, bool short_form,
				  char* last_db)
{
  if (short_form)
    return;
  print_header(file);
  fputc('\n', file);
  fprintf(file, "#Delete_file: file_id=%u\n", file_id);
}
#endif  

#ifndef MYSQL_CLIENT
void Delete_file_log_event::pack_info(String* packet)
{
  char buf1[64];
  sprintf(buf1, ";file_id=%u", (uint) file_id);
  net_store_data(packet, buf1);
}
#endif  


#ifndef MYSQL_CLIENT  
Execute_load_log_event::Execute_load_log_event(THD* thd_arg, const char* db_arg,
                                               bool using_trans)
  :Log_event(thd_arg, 0, using_trans), file_id(thd_arg->file_id), db(db_arg)
{
}
#endif  
  
Execute_load_log_event::Execute_load_log_event(const char* buf, int len)
  :Log_event(buf, 0), file_id(0)
{
  if ((uint)len < EXEC_LOAD_EVENT_OVERHEAD)
    return;
  file_id = uint4korr(buf + LOG_EVENT_HEADER_LEN + EL_FILE_ID_OFFSET);
}

int Execute_load_log_event::write_data(IO_CACHE* file)
{
  byte buf[EXEC_LOAD_HEADER_LEN];
  int4store(buf + EL_FILE_ID_OFFSET, file_id);
  return my_b_safe_write(file, buf, EXEC_LOAD_HEADER_LEN);
}

#ifdef MYSQL_CLIENT  
void Execute_load_log_event::print(FILE* file, bool short_form,
				   char* last_db)
{
  if (short_form)
    return;
  print_header(file);
  fputc('\n', file);
  fprintf(file, "#Exec_load: file_id=%d\n",
	  file_id);
}
#endif  
#ifndef MYSQL_CLIENT
void Execute_load_log_event::pack_info(String* packet)
{
  char buf[64];
  sprintf(buf, ";file_id=%u", (uint) file_id);
  net_store_data(packet, buf);
}
#endif

#ifdef MYSQL_CLIENT
void Unknown_log_event::print(FILE* file, bool short_form, char* last_db)
{
  if (short_form)
    return;
  print_header(file);
  fputc('\n', file);
  fprintf(file, "# %s", "Unknown event\n");
}
#endif  

#ifndef MYSQL_CLIENT
int Query_log_event::exec_event(struct st_relay_log_info* rli)
{
  int expected_error, actual_error= 0;
  init_sql_alloc(&thd->mem_root, thd->variables.query_alloc_block_size,0);
  thd->db= (char*) rewrite_db(db);

  /*
    InnoDB internally stores the master log position it has processed so far;
    position to store is really pos + pending + event_len
    since we must store the pos of the END of the current log event
  */
  rli->event_len= get_event_len();

  if (db_ok(thd->db, replicate_do_db, replicate_ignore_db))
  {
    thd->set_time((time_t)when);
    thd->current_tablenr = 0;
    thd->query_length= q_len;
    VOID(pthread_mutex_lock(&LOCK_thread_count));
    thd->query = (char*)query;
    thd->query_id = query_id++;
    VOID(pthread_mutex_unlock(&LOCK_thread_count));
    thd->query_error = 0;			// clear error
    thd->net.last_errno = 0;
    thd->net.last_error[0] = 0;
    thd->slave_proxy_id = thread_id;		// for temp tables
	
    /*
      Sanity check to make sure the master did not get a really bad
      error on the query.
    */
    if (ignored_error_code((expected_error = error_code)) ||
	!check_expected_error(thd,rli,expected_error))
    {
      mysql_log.write(thd,COM_QUERY,"%s",thd->query);
      DBUG_PRINT("query",("%s",thd->query));
      mysql_parse(thd, thd->query, q_len);

      /*
	Set a flag if we are inside an transaction so that we can restart
	the transaction from the start if we are killed

	This will only be done if we are supporting transactional tables
	in the slave.
      */
      if (!strcmp(thd->query,"BEGIN"))
	rli->inside_transaction= opt_using_transactions;
      else if (!(strcmp(thd->query,"COMMIT") && strcmp(thd->query,"ROLLBACK")))
	rli->inside_transaction=0;

      /*
        If we expected a non-zero error code, and we don't get the same error
        code, and none of them should be ignored.
      */
      if ((expected_error != (actual_error = thd->net.last_errno)) &&
	  expected_error &&
	  !ignored_error_code(actual_error) &&
	  !ignored_error_code(expected_error))
      {
	slave_print_error(rli, 0,
                          "\
Query '%s' caused different errors on master and slave. \
Error on master: '%s' (%d), Error on slave: '%s' (%d). \
Default database: '%s'",
                          query,
                          ER_SAFE(expected_error),
                          expected_error,
                          actual_error ? thd->net.last_error: "no error",
                          actual_error,
                          print_slave_db_safe(db));
	thd->query_error= 1;
      }
      /*
        If we get the same error code as expected, or they should be ignored. 
      */
      else if (expected_error == actual_error ||
	       ignored_error_code(actual_error))
      {
	thd->query_error = 0;
	*rli->last_slave_error = 0;
	rli->last_slave_errno = 0;
      }
      /*
        Other cases: mostly we expected no error and get one.
      */
      else if (thd->query_error || thd->fatal_error)
      {
        slave_print_error(rli,actual_error,
			  "Error '%s' on query '%s'. Default database: '%s'",
                          (actual_error ? thd->net.last_error :
			   "unexpected success or fatal error"),
			  query,
                          print_slave_db_safe(db));
        thd->query_error= 1;
      }
    } 
    /* 
       End of sanity check. If the test was wrong, the query got a really bad
       error on the master, which could be inconsistent, abort and tell DBA to
       check/fix it. check_expected_error() already printed the message to
       stderr and rli, and set thd->query_error to 1.
    */
  } /* End of if (db_ok(... */

  VOID(pthread_mutex_lock(&LOCK_thread_count));
  thd->db= 0;	                        // prevent db from being freed
  thd->query= 0;			// just to be sure
  VOID(pthread_mutex_unlock(&LOCK_thread_count));
  // assume no convert for next query unless set explictly
  thd->variables.convert_set = 0;
  close_thread_tables(thd);      
  free_root(&thd->mem_root,0);
  return (thd->query_error ? thd->query_error : Log_event::exec_event(rli)); 
}

/*
  Does the data loading job when executing a LOAD DATA on the slave

  SYNOPSIS
    Load_log_event::exec_event
      net  
      rli                             
      use_rli_only_for_errors	  - if set to 1, rli is provided to 
                                  Load_log_event::exec_event only for this 
				  function to have RPL_LOG_NAME and 
				  rli->last_slave_error, both being used by 
				  error reports. rli's position advancing
				  is skipped (done by the caller which is
				  Execute_load_log_event::exec_event).
				  - if set to 0, rli is provided for full use,
				  i.e. for error reports and position
				  advancing.

  DESCRIPTION
    Does the data loading job when executing a LOAD DATA on the slave
 
  RETURN VALUE
    0           Success                                                 
    1    	Failure
*/

int Load_log_event::exec_event(NET* net, struct st_relay_log_info* rli, 
			       bool use_rli_only_for_errors)
{
  init_sql_alloc(&thd->mem_root, thd->variables.query_alloc_block_size, 0);
  thd->db= (char*) rewrite_db(db);
  DBUG_ASSERT(thd->query == 0);
  thd->query = 0;				// Should not be needed
  thd->query_error = 0;

  /*
    We test replicate_*_db rules. Note that we have already prepared the file to
    load, even if we are going to ignore and delete it now. So it is possible
    that we did a lot of disk writes for nothing. In other words, a big LOAD
    DATA INFILE on the master will still consume a lot of space on the slave
    (space in the relay log + space of temp files: twice the space of the file
    to load...) even if it will finally be ignored.
    TODO: fix this; this can be done by testing rules in
    Create_file_log_event::exec_event() and then discarding Append_block and
    al. Another way is do the filtering in the I/O thread (more efficient: no
    disk writes at all).
  */
  if (db_ok(thd->db, replicate_do_db, replicate_ignore_db))
  {
    thd->set_time((time_t)when);
    thd->current_tablenr = 0;
    VOID(pthread_mutex_lock(&LOCK_thread_count));
    thd->query_id = query_id++;
    VOID(pthread_mutex_unlock(&LOCK_thread_count));

    TABLE_LIST tables;
    bzero((char*) &tables,sizeof(tables));
    tables.db = thd->db;
    tables.alias = tables.real_name = (char*)table_name;
    tables.lock_type = TL_WRITE;
    tables.updating= 1;
    // the table will be opened in mysql_load    
    if (table_rules_on && !tables_ok(thd, &tables))
    {
      // TODO: this is a bug - this needs to be moved to the I/O thread
      if (net)
        skip_load_data_infile(net);
    }
    else
    {
      char llbuff[22];
      enum enum_duplicates handle_dup;
      if (sql_ex.opt_flags & REPLACE_FLAG)
	handle_dup= DUP_REPLACE;
      else if (sql_ex.opt_flags & IGNORE_FLAG)
        handle_dup= DUP_IGNORE;
      else
      {
        /*
          When replication is running fine, if it was DUP_ERROR on the 
          master then we could choose DUP_IGNORE here, because if DUP_ERROR
          suceeded on master, and data is identical on the master and slave,
          then there should be no uniqueness errors on slave, so DUP_IGNORE is
          the same as DUP_ERROR. But in the unlikely case of uniqueness errors
          (because the data on the master and slave happen to be different
	  (user error or bug), we want LOAD DATA to print an error message on
	  the slave to discover the problem.

          If reading from net (a 3.23 master), mysql_load() will change this
          to DUP_IGNORE.
        */
        handle_dup= DUP_ERROR;
      }

      sql_exchange ex((char*)fname, sql_ex.opt_flags & DUMPFILE_FLAG);
      String field_term(sql_ex.field_term,sql_ex.field_term_len);
      String enclosed(sql_ex.enclosed,sql_ex.enclosed_len);
      String line_term(sql_ex.line_term,sql_ex.line_term_len);
      String line_start(sql_ex.line_start,sql_ex.line_start_len);
      String escaped(sql_ex.escaped,sql_ex.escaped_len);
      ex.field_term= &field_term;
      ex.enclosed= &enclosed;
      ex.line_term= &line_term;
      ex.line_start= &line_start;
      ex.escaped= &escaped;
	    
      ex.opt_enclosed = (sql_ex.opt_flags & OPT_ENCLOSED_FLAG);
      if (sql_ex.empty_flags & FIELD_TERM_EMPTY)
	ex.field_term->length(0);

      ex.skip_lines = skip_lines;
      List<Item> field_list;
      set_fields(field_list);
      thd->slave_proxy_id = thread_id;
      if (net)
      {
	// mysql_load will use thd->net to read the file
	thd->net.vio = net->vio;
	/*
	  Make sure the client does not get confused about the packet sequence
	*/
	thd->net.pkt_nr = net->pkt_nr;
      }
      if (mysql_load(thd, &ex, &tables, field_list, handle_dup, net != 0,
		     TL_WRITE))
	thd->query_error = 1;
      /* log_pos is the position of the LOAD event in the master log */
      if (thd->cuted_fields)
	sql_print_error("\
Slave: load data infile on table '%s' at log position %s in log \
'%s' produced %ld warning(s). Default database: '%s'",
                        (char*) table_name,
                        llstr(log_pos,llbuff), RPL_LOG_NAME, 
			(ulong) thd->cuted_fields,
                        print_slave_db_safe(db));
      if (net)
        net->pkt_nr= thd->net.pkt_nr;
    }
  }
  else
  {
    /*
      We will just ask the master to send us /dev/null if we do not
      want to load the data.
      TODO: this a bug - needs to be done in I/O thread
    */
    if (net)
      skip_load_data_infile(net);
  }
	    
  thd->net.vio = 0; 
  thd->db= 0;					// prevent db from being freed
  close_thread_tables(thd);
  if (thd->query_error)
  {
    /* this err/sql_errno code is copy-paste from send_error() */
    const char *err;
    int sql_errno;
    if ((err=thd->net.last_error)[0])
      sql_errno=thd->net.last_errno;
    else
    {
      sql_errno=ER_UNKNOWN_ERROR;
      err=ER(sql_errno);       
    }
    slave_print_error(rli,sql_errno,"\
Error '%s' running LOAD DATA INFILE on table '%s'. Default database: '%s'",
		      err, (char*)table_name, print_slave_db_safe(db));
    free_root(&thd->mem_root,0);
    return 1;
  }
  free_root(&thd->mem_root,0);
	    
  if (thd->fatal_error)
  {
    slave_print_error(rli,ER_UNKNOWN_ERROR, "\
Fatal error running LOAD DATA INFILE on table '%s'. Default database: '%s'",
		      (char*)table_name, print_slave_db_safe(db));
    return 1;
  }

  return ( use_rli_only_for_errors ? 0 : Log_event::exec_event(rli) ); 
}


/*
  The master started

  IMPLEMENTATION
    - To handle the case where the master died without a stop event,
      we clean up all temporary tables that we got, if we are sure we
      can (see below).

  TODO
    - Remove all active user locks
*/

int Start_log_event::exec_event(struct st_relay_log_info* rli)
{

  switch (rli->mi->old_format) {
  case BINLOG_FORMAT_CURRENT : 
    /* 
       This is 4.x, so a Start_log_event is only at master startup,
       so we are sure the master has restarted and cleared his temp tables.
    */
    close_temporary_tables(thd);
    cleanup_load_tmpdir();
    /*
      As a transaction NEVER spans on 2 or more binlogs:
      if we have an active transaction at this point, the master died while
      writing the transaction to the binary log, i.e. while flushing the binlog
      cache to the binlog. As the write was started, the transaction had been
      committed on the master, so we lack of information to replay this
      transaction on the slave; all we can do is stop with error.
    */
    if (rli->inside_transaction)
    {
      slave_print_error(rli, 0,
                        "\
Rolling back unfinished transaction (no COMMIT or ROLLBACK) from relay log. \
Probably cause is that the master died while writing the transaction to it's \
binary log.");
      return(1);
    }
    break;
  /* 
     Now the older formats; in that case load_tmpdir is cleaned up by the I/O
     thread.
  */
  case BINLOG_FORMAT_323_LESS_57 :
    /*
      Cannot distinguish a Start_log_event generated at master startup and
      one generated by master FLUSH LOGS, so cannot be sure temp tables
      have to be dropped. So do nothing.
    */
    break;
 case BINLOG_FORMAT_323_GEQ_57 : 
    /*
      Can distinguish, based on the value of 'created',
      which was generated at master startup.
    */
    if (created)
      close_temporary_tables(thd);
    break;
  default :
    /* this case is impossible */
    return 1;
  }

  return Log_event::exec_event(rli);
}


/*
  The master stopped. Clean up all temporary tables + locks that the
  master may have set.

  TODO
    - Remove all active user locks
*/

int Stop_log_event::exec_event(struct st_relay_log_info* rli)
{
  /*
    do not clean up immediately after rotate event;
    QQ: this should be a useless test: the only case when it is false is when
    shutdown occured just after FLUSH LOGS. It has nothing to do with Rotate?
    By the way, immediately after a Rotate
    the I/O thread does not write the Stop to the relay log,
    so we won't come here in that case.
  */
  if (rli->master_log_pos > BIN_LOG_HEADER_SIZE) 
  {
    close_temporary_tables(thd);
    cleanup_load_tmpdir();
  }
  /*
    We do not want to update master_log pos because we get a rotate event
    before stop, so by now master_log_name is set to the next log.
    If we updated it, we will have incorrect master coordinates and this
    could give false triggers in MASTER_POS_WAIT() that we have reached
    the target position when in fact we have not.
  */
  rli->inc_pos(get_event_len(), 0);  
  flush_relay_log_info(rli);
  return 0;
}


/*
  Got a rotate log even from the master

  IMPLEMENTATION
    This is mainly used so that we can later figure out the logname and
    position for the master.

    We can't rotate the slave as this will cause infinitive rotations
    in a A -> B -> A setup.

  RETURN VALUES
    0	ok
*/

int Rotate_log_event::exec_event(struct st_relay_log_info* rli)
{
  DBUG_ENTER("Rotate_log_event::exec_event");

  pthread_mutex_lock(&rli->data_lock);
  /*
    If we are in a transaction: the only normal case is when the I/O thread was
    copying a big transaction, then it was stopped and restarted: we have this
    in the relay log:
    BEGIN
    ...
    ROTATE (a fake one)
    ...
    COMMIT or ROLLBACK
    In that case, we don't want to touch the coordinates which correspond to the
    beginning of the transaction.
  */
  if (!rli->inside_transaction)
  {
    memcpy(rli->master_log_name, new_log_ident, ident_len+1);
    rli->master_log_pos= pos;
    DBUG_PRINT("info", ("master_log_pos: %lu", (ulong) rli->master_log_pos));
  }
  rli->relay_log_pos += get_event_len();
  pthread_mutex_unlock(&rli->data_lock);
  pthread_cond_broadcast(&rli->data_cond);
  flush_relay_log_info(rli);
  DBUG_RETURN(0);
}


int Intvar_log_event::exec_event(struct st_relay_log_info* rli)
{
  switch (type) {
  case LAST_INSERT_ID_EVENT:
    thd->last_insert_id_used = 1;
    thd->last_insert_id = val;
    break;
  case INSERT_ID_EVENT:
    thd->next_insert_id = val;
    break;
  }
  rli->inc_pending(get_event_len());
  return 0;
}

int Rand_log_event::exec_event(struct st_relay_log_info* rli)
{
  thd->rand.seed1 = (ulong) seed1;
  thd->rand.seed2 = (ulong) seed2;
  rli->inc_pending(get_event_len());
  return 0;
}

int Slave_log_event::exec_event(struct st_relay_log_info* rli)
{
  if (mysql_bin_log.is_open())
    mysql_bin_log.write(this);
  return Log_event::exec_event(rli);
}

int Create_file_log_event::exec_event(struct st_relay_log_info* rli)
{
  char fname_buf[FN_REFLEN+10];
  char *p;
  int fd = -1;
  IO_CACHE file;
  int error = 1;

  bzero((char*)&file, sizeof(file));
  p = slave_load_file_stem(fname_buf, file_id, server_id);
  strmov(p, ".info");			// strmov takes less code than memcpy
  if ((fd = my_open(fname_buf, O_WRONLY|O_CREAT|O_BINARY|O_TRUNC,
		    MYF(MY_WME))) < 0 ||
      init_io_cache(&file, fd, IO_SIZE, WRITE_CACHE, (my_off_t)0, 0,
		    MYF(MY_WME|MY_NABP)))
  {
    slave_print_error(rli,my_errno, "Error in Create_file event: could not open file '%s'", fname_buf);
    goto err;
  }
  
  // a trick to avoid allocating another buffer
  strmov(p, ".data");
  fname = fname_buf;
  fname_len = (uint)(p-fname) + 5;
  if (write_base(&file))
  {
    strmov(p, ".info"); // to have it right in the error message
    slave_print_error(rli,my_errno, "Error in Create_file event: could not write to file '%s'", fname_buf);
    goto err;
  }
  end_io_cache(&file);
  my_close(fd, MYF(0));
  
  // fname_buf now already has .data, not .info, because we did our trick
  if ((fd = my_open(fname_buf, O_WRONLY|O_CREAT|O_BINARY|O_TRUNC,
		    MYF(MY_WME))) < 0)
  {
    slave_print_error(rli,my_errno, "Error in Create_file event: could not open file '%s'", fname_buf);
    goto err;
  }
  if (my_write(fd, (byte*) block, block_len, MYF(MY_WME+MY_NABP)))
  {
    slave_print_error(rli,my_errno, "Error in Create_file event: write to '%s' failed", fname_buf);
    goto err;
  }
  error=0;					// Everything is ok

err:
  if (error)
    end_io_cache(&file);
  if (fd >= 0)
    my_close(fd, MYF(0));
  return error ? 1 : Log_event::exec_event(rli);
}

int Delete_file_log_event::exec_event(struct st_relay_log_info* rli)
{
  char fname[FN_REFLEN+10];
  char *p= slave_load_file_stem(fname, file_id, server_id);
  memcpy(p, ".data", 6);
  (void) my_delete(fname, MYF(MY_WME));
  memcpy(p, ".info", 6);
  (void) my_delete(fname, MYF(MY_WME));
  return Log_event::exec_event(rli);
}

int Append_block_log_event::exec_event(struct st_relay_log_info* rli)
{
  char fname[FN_REFLEN+10];
  char *p= slave_load_file_stem(fname, file_id, server_id);
  int fd;
  int error = 1;

  memcpy(p, ".data", 6);
  if ((fd = my_open(fname, O_WRONLY|O_APPEND|O_BINARY, MYF(MY_WME))) < 0)
  {
    slave_print_error(rli,my_errno, "Error in Append_block event: could not open file '%s'", fname);
    goto err;
  }
  if (my_write(fd, (byte*) block, block_len, MYF(MY_WME+MY_NABP)))
  {
    slave_print_error(rli,my_errno, "Error in Append_block event: write to '%s' failed", fname);
    goto err;
  }
  error=0;

err:
  if (fd >= 0)
    my_close(fd, MYF(0));
  return error ? error : Log_event::exec_event(rli);
}

int Execute_load_log_event::exec_event(struct st_relay_log_info* rli)
{
  char fname[FN_REFLEN+10];
  char *p= slave_load_file_stem(fname, file_id, server_id);
  int fd;
  int error = 1;
  IO_CACHE file;
  Load_log_event* lev = 0;

  memcpy(p, ".info", 6);
  if ((fd = my_open(fname, O_RDONLY|O_BINARY, MYF(MY_WME))) < 0 ||
      init_io_cache(&file, fd, IO_SIZE, READ_CACHE, (my_off_t)0, 0,
		    MYF(MY_WME|MY_NABP)))
  {
    slave_print_error(rli,my_errno, "Error in Exec_load event: could not open file '%s'", fname);
    goto err;
  }
  if (!(lev = (Load_log_event*)Log_event::read_log_event(&file,
							 (pthread_mutex_t*)0,
							 (bool)0)) ||
      lev->get_type_code() != NEW_LOAD_EVENT)
  {
    slave_print_error(rli,0, "Error in Exec_load event: file '%s' appears corrupted", fname);
    goto err;
  }

  lev->thd = thd;
  /*
    lev->exec_event should use rli only for errors
    i.e. should not advance rli's position.
    lev->exec_event is the place where the table is loaded (it calls
    mysql_load()).
  */
  if (lev->exec_event(0,rli,1)) 
  {
    /*
      We want to indicate the name of the file that could not be loaded
      (SQL_LOADxxx).
      But as we are here we are sure the error is in rli->last_slave_error and
      rli->last_slave_errno (example of error: duplicate entry for key), so we
      don't want to overwrite it with the filename.
      What we want instead is add the filename to the current error message.
    */
    char *tmp= my_strdup(rli->last_slave_error,MYF(MY_WME));
    if (tmp)
    {
      slave_print_error(rli,
			rli->last_slave_errno, /* ok to re-use error code */
			"%s. Failed executing load from '%s'", 
			tmp, fname);
      my_free(tmp,MYF(0));
    }
    goto err;
  }
  /*
    We have an open file descriptor to the .info file; we need to close it
    or Windows will refuse to delete the file in my_delete().
  */
  if (fd >= 0)
  {
    my_close(fd, MYF(0));
    end_io_cache(&file);
    fd= -1;
  }
  (void) my_delete(fname, MYF(MY_WME));
  memcpy(p, ".data", 6);
  (void) my_delete(fname, MYF(MY_WME));
  error = 0;

err:
  delete lev;
  if (fd >= 0)
  {
    my_close(fd, MYF(0));
    end_io_cache(&file);
  }
  return error ? error : Log_event::exec_event(rli);
}

#endif /* !MYSQL_CLIENT */

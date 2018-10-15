/* Copyright (C) 2001 MySQL AB

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

#define MYSQL_CLIENT
#undef MYSQL_SERVER
#include "client_priv.h"
#include <time.h>
#include <assert.h>
#include "log_event.h"

#define BIN_LOG_HEADER_SIZE	4
#define PROBE_HEADER_LEN	(EVENT_LEN_OFFSET+4)


#define CLIENT_CAPABILITIES	(CLIENT_LONG_PASSWORD | CLIENT_LONG_FLAG | CLIENT_LOCAL_FILES)

char server_version[SERVER_VERSION_LENGTH];
ulong server_id = 0;

// needed by net_serv.c
ulong bytes_sent = 0L, bytes_received = 0L;
ulong mysqld_net_retry_count = 10L;
uint test_flags = 0; 

static FILE *result_file;

#ifndef DBUG_OFF
static const char* default_dbug_option = "d:t:o,/tmp/mysqlbinlog.trace";
#endif
static const char *load_default_groups[]= { "mysqlbinlog","client",0 };

void sql_print_error(const char *format, ...);

static bool one_database = 0;
static const char* database= 0;
static my_bool force_opt= 0, short_form= 0, remote_opt= 0;
static ulonglong offset = 0;
static const char* host = 0;
static int port = MYSQL_PORT;
static const char* sock= 0;
static const char* user = 0;
static const char* pass = "";
static ulonglong position = 0;
static short binlog_flags = 0; 
static MYSQL* mysql = NULL;

static const char* dirname_for_local_load= 0;

static void dump_local_log_entries(const char* logname);
static void dump_remote_log_entries(const char* logname);
static void dump_log_entries(const char* logname);
static void dump_remote_file(NET* net, const char* fname);
static void die(const char* fmt, ...);
static MYSQL* safe_connect();

class Load_log_processor
{
  char target_dir_name[MY_NFILE];
  int target_dir_name_len;
  DYNAMIC_ARRAY file_names;

  const char *create_file(Create_file_log_event *ce);
  void append_to_file(const char* fname, int flags, 
		      gptr data, uint size)
    {
      File file;
      if (((file= my_open(fname,flags,MYF(MY_WME))) < 0) ||
	  my_write(file,(byte*) data,size,MYF(MY_WME|MY_NABP)) ||
	  my_close(file,MYF(MY_WME)))
	exit(1);
    }

public:
  Load_log_processor()
    {
      init_dynamic_array(&file_names,sizeof(Create_file_log_event*),
			 100,100 CALLER_INFO);
    }

  ~Load_log_processor()
    {
      destroy();
      delete_dynamic(&file_names);
    }

  void init_by_dir_name(const char *dir)
    {
      target_dir_name_len= (convert_dirname(target_dir_name, dir, NullS) -
			    target_dir_name);
    }
  void init_by_cur_dir()
    {
      if (my_getwd(target_dir_name,sizeof(target_dir_name),MYF(MY_WME)))
	exit(1);
      target_dir_name_len= strlen(target_dir_name);
    }
  void destroy()
    {
      Create_file_log_event **ptr= (Create_file_log_event**)file_names.buffer;
      Create_file_log_event **end= ptr + file_names.elements;
      for (; ptr<end; ptr++)
      {
	if (*ptr)
	{
	  my_free((char*)(*ptr)->fname,MYF(MY_WME));
	  delete *ptr;
	  *ptr= 0;
	}
      }
    }
  Create_file_log_event *grab_event(uint file_id)
    {
      if (file_id >= file_names.elements)
        return 0;
      Create_file_log_event **ptr= 
	(Create_file_log_event**)file_names.buffer + file_id;
      Create_file_log_event *res= *ptr;
      *ptr= 0;
      return res;
    }
  void process(Create_file_log_event *ce)
    {
      const char *fname= create_file(ce);
      append_to_file(fname,O_CREAT|O_EXCL|O_BINARY|O_WRONLY,ce->block,
		     ce->block_len);
    }
  void process(Append_block_log_event *ae)
    {
      Create_file_log_event* ce= (ae->file_id < file_names.elements) ?
          *((Create_file_log_event**)file_names.buffer + ae->file_id) : 0;
        
      if (ce)
        append_to_file(ce->fname,O_APPEND|O_BINARY|O_WRONLY, ae->block,
		       ae->block_len);
      else
      {
        /*
          There is no Create_file event (a bad binlog or a big
          --position). Assuming it's a big --position, we just do nothing and
          print a warning.
        */
	fprintf(stderr,"Warning: ignoring Append_block as there is no \
Create_file event for file_id: %u\n",ae->file_id);
      }
    }
};


const char *Load_log_processor::create_file(Create_file_log_event *ce)
{
  const char *bname= ce->fname+dirname_length(ce->fname);
  uint blen= ce->fname_len - (bname-ce->fname);
  uint full_len= target_dir_name_len + blen + 9 + 9 + 1;
  uint version= 0;
  char *tmp, *ptr;

  if (!(tmp= my_malloc(full_len,MYF(MY_WME))) ||
      set_dynamic(&file_names,(gptr)&ce,ce->file_id))
  {
    die("Could not construct local filename %s%s",target_dir_name,bname);
    return 0;
  }

  memcpy(tmp, target_dir_name, target_dir_name_len);
  ptr= tmp+ target_dir_name_len;
  memcpy(ptr,bname,blen);
  ptr+= blen;
  ptr+= my_sprintf(ptr,(ptr,"-%x",ce->file_id));

  /*
    Note that this code has a possible race condition if there was was
    many simultaneous clients running which tried to create files at the same
    time. Fortunately this should never be the case.

    A better way to do this would be to use 'create_tmp_file() and avoid this
    race condition altogether on the expense of getting more cryptic file
    names.
  */
  for (;;)
  {
    sprintf(ptr,"-%x",version);
    if (access(tmp,F_OK))
      break;
    /* If we have to try more than 1000 times, something is seriously wrong */
    if (version++ > 1000)
    {
      die("Could not construct local filename %s%s",target_dir_name,bname);
      return 0;
    }
  }
  ce->set_fname_outside_temp_buf(tmp,strlen(tmp));
  return tmp;
}


Load_log_processor load_processor;

static struct my_option my_long_options[] =
{
#ifndef DBUG_OFF
  {"debug", '#', "Output debug log.", (gptr*) &default_dbug_option,
   (gptr*) &default_dbug_option, 0, GET_STR, OPT_ARG, 0, 0, 0, 0, 0, 0},
#endif
  {"database", 'd', "List entries for just this database (local log only).",
   (gptr*) &database, (gptr*) &database, 0, GET_STR_ALLOC, REQUIRED_ARG,
   0, 0, 0, 0, 0, 0},
  {"force-read", 'f', "Force reading unknown binlog events.",
   (gptr*) &force_opt, (gptr*) &force_opt, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0,
   0, 0},
  {"help", '?', "Display this help and exit.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"host", 'h', "Get the binlog from server.", (gptr*) &host, (gptr*) &host,
   0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"offset", 'o', "Skip the first N entries.", (gptr*) &offset, (gptr*) &offset,
   0, GET_ULL, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"password", 'p', "Password to connect to remote server.",
   0, 0, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"port", 'P', "Use port to connect to the remote server.",
   (gptr*) &port, (gptr*) &port, 0, GET_INT, REQUIRED_ARG, MYSQL_PORT, 0, 0,
   0, 0, 0},
  {"position", 'j', "Start reading the binlog at position N.",
   (gptr*) &position, (gptr*) &position, 0, GET_ULL, REQUIRED_ARG, 0, 0, 0, 0,
   0, 0},
  {"result-file", 'r', "Direct output to a given file.", 0, 0, 0, GET_STR,
   REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"read-from-remote-server", 'R', "Read binary logs from a MySQL server",
   (gptr*) &remote_opt, (gptr*) &remote_opt, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0,
   0, 0},
  {"short-form", 's', "Just show the queries, no extra info.",
   (gptr*) &short_form, (gptr*) &short_form, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0,
   0, 0},
  {"socket", 'S', "Socket file to use for connection.",
   (gptr*) &sock, (gptr*) &sock, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 
   0, 0},
  {"user", 'u', "Connect to the remote server as username.",
   (gptr*) &user, (gptr*) &user, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0,
   0, 0},
  {"local-load", 'l', "Prepare files for local load in directory.",
   (gptr*) &dirname_for_local_load, (gptr*) &dirname_for_local_load, 0,
   GET_STR_ALLOC, OPT_ARG, 0, 0, 0, 0, 0, 0},
  {"version", 'V', "Print version and exit.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0,
   0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};


void sql_print_error(const char *format,...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "ERROR: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

static void die(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "ERROR: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

static void print_version()
{
  printf("%s Ver 2.4 for %s at %s\n", my_progname, SYSTEM_TYPE, MACHINE_TYPE);
}


static void usage()
{
  print_version();
  puts("By Monty and Sasha, for your professional use\n\
This software comes with NO WARRANTY:  This is free software,\n\
and you are welcome to modify and redistribute it under the GPL license\n");

  printf("\
Dumps a MySQL binary log in a format usable for viewing or for piping to\n\
the mysql command line client\n\n");
  printf("Usage: %s [options] log-files\n", my_progname);
  my_print_help(my_long_options);
  my_print_variables(my_long_options);
}

static void dump_remote_file(NET* net, const char* fname)
{
  char buf[FN_REFLEN+1];
  uint len = (uint) strlen(fname);
  buf[0] = 0;
  memcpy(buf + 1, fname, len + 1);
  if(my_net_write(net, buf, len +2) || net_flush(net))
    die("Failed  requesting the remote dump of %s", fname);
  for(;;)
    {
      uint packet_len = my_net_read(net);
      if(packet_len == 0)
	{
	  if(my_net_write(net, "", 0) || net_flush(net))
	    die("Failed sending the ack packet");

	  // we just need to send something, as the server will read but
	  // not examine the packet - this is because mysql_load() sends an OK when it is done
	  break;
	}
      else if(packet_len == packet_error)
	die("Failed reading a packet during the dump of %s ", fname);

      if(!short_form)
	(void)my_fwrite(result_file, (byte*) net->read_pos, packet_len,MYF(0));
    }

  fflush(result_file);
}


extern "C" my_bool
get_one_option(int optid, const struct my_option *opt __attribute__((unused)),
	       char *argument)
{
  switch (optid) {
#ifndef DBUG_OFF
  case '#':
    DBUG_PUSH(argument ? argument : default_dbug_option);
    break;
#endif
  case 'd':
    one_database = 1;
    break;
  case 'p':
    pass = my_strdup(argument, MYF(0));
    break;
  case 'r':
    if (!(result_file = my_fopen(argument, O_WRONLY | O_BINARY, MYF(MY_WME))))
      exit(1);
    break;
  case 'R':
    remote_opt= 1;
    break;
  case 'V':
    print_version();
    exit(0);
  case '?':
    usage();
    exit(0);
  }
  return 0;
}


static int parse_args(int *argc, char*** argv)
{
  int ho_error;

  result_file = stdout;
  load_defaults("my",load_default_groups,argc,argv);
  if ((ho_error=handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);

  return 0;
}

static MYSQL* safe_connect()
{
  MYSQL *local_mysql = mysql_init(NULL);
  if(!local_mysql)
    die("Failed on mysql_init");

  if (!mysql_real_connect(local_mysql, host, user, pass, 0, port, sock, 0))
    die("failed on connect: %s", mysql_error(local_mysql));

  return local_mysql;
}

static void dump_log_entries(const char* logname)
{
  if (remote_opt)
    dump_remote_log_entries(logname);
  else
    dump_local_log_entries(logname);  
}

static int check_master_version(MYSQL* mysql)
{
  MYSQL_RES* res = 0;
  MYSQL_ROW row;
  const char* version;
  int old_format = 0;

  if (mysql_query(mysql, "SELECT VERSION()") ||
      !(res = mysql_store_result(mysql)))
  {
    mysql_close(mysql);
    die("Error checking master version: %s",
		    mysql_error(mysql));
  }
  if (!(row = mysql_fetch_row(res)))
  {
    mysql_free_result(res);
    mysql_close(mysql);
    die("Master returned no rows for SELECT VERSION()");
    return 1;
  }
  if (!(version = row[0]))
  {
    mysql_free_result(res);
    mysql_close(mysql);
    die("Master reported NULL for the version");
  }

  switch (*version) {
  case '3':
    old_format = 1;
    break;
  case '4':
  case '5':
    old_format = 0;
    break;
  default:
    sql_print_error("Master reported unrecognized MySQL version '%s'",
		    version);
    mysql_free_result(res);
    mysql_close(mysql);
    return 1;
  }
  mysql_free_result(res);
  return old_format;
}


static void dump_remote_log_entries(const char* logname)
{
  char buf[128];
  char last_db[FN_REFLEN+1] = "";
  uint len;
  NET* net = &mysql->net;
  int old_format;
  old_format = check_master_version(mysql);

  if (!position)
    position = BIN_LOG_HEADER_SIZE; // protect the innocent from spam
  if (position < BIN_LOG_HEADER_SIZE)
  {
    position = BIN_LOG_HEADER_SIZE;
    // warn the guity
    sql_print_error("Warning: The position in the binary log can't be less than %d.\nStarting from position %d\n", BIN_LOG_HEADER_SIZE, BIN_LOG_HEADER_SIZE);
  }
  int4store(buf, position);
  int2store(buf + BIN_LOG_HEADER_SIZE, binlog_flags);
  len = (uint) strlen(logname);
  int4store(buf + 6, 0);
  memcpy(buf + 10, logname,len);
  if (simple_command(mysql, COM_BINLOG_DUMP, buf, len + 10, 1))
    die("Error sending the log dump command");

  for (;;)
  {
    const char *error;
    len = net_safe_read(mysql);
    if (len == packet_error)
      die("Error reading packet from server: %s", mysql_error(mysql));
    if (len < 8 && net->read_pos[0] == 254)
      break; // end of data
    DBUG_PRINT("info",( "len= %u, net->read_pos[5] = %d\n",
			len, net->read_pos[5]));
    Log_event *ev = Log_event::read_log_event((const char*) net->read_pos + 1 ,
					      len - 1, &error, old_format);
    if (ev)
    {
      ev->print(result_file, short_form, last_db);
      if (ev->get_type_code() == LOAD_EVENT)
	dump_remote_file(net, ((Load_log_event*)ev)->fname);
      delete ev;
    }
    else
      die("Could not construct log event object");
  }
}


static int check_header(IO_CACHE* file)
{
  byte header[BIN_LOG_HEADER_SIZE];
  byte buf[PROBE_HEADER_LEN];
  int old_format=0;

  my_off_t pos = my_b_tell(file);
  my_b_seek(file, (my_off_t)0);
  if (my_b_read(file, header, sizeof(header)))
    die("Failed reading header;  Probably an empty file");
  if (memcmp(header, BINLOG_MAGIC, sizeof(header)))
    die("File is not a binary log file");
  if (!my_b_read(file, buf, sizeof(buf)))
  {
    if (buf[4] == START_EVENT)
    {
      uint event_len;
      event_len = uint4korr(buf + EVENT_LEN_OFFSET);
      old_format = (event_len < (LOG_EVENT_HEADER_LEN + START_HEADER_LEN));
    }
  }
  my_b_seek(file, pos);
  return old_format;
}


static void dump_local_log_entries(const char* logname)
{
  File fd = -1;
  IO_CACHE cache,*file= &cache;
  ulonglong rec_count = 0;
  char last_db[FN_REFLEN+1];
  byte tmp_buff[BIN_LOG_HEADER_SIZE];
  bool old_format = 0;

  last_db[0]=0;

  if (logname && logname[0] != '-')
  {
    if ((fd = my_open(logname, O_RDONLY | O_BINARY, MYF(MY_WME))) < 0)
      exit(1);
    if (init_io_cache(file, fd, 0, READ_CACHE, (my_off_t) position, 0,
		      MYF(MY_WME | MY_NABP)))
      exit(1);
    old_format = check_header(file);
  }
  else
  {
    if (init_io_cache(file, fileno(result_file), 0, READ_CACHE, (my_off_t) 0,
		      0, MYF(MY_WME | MY_NABP | MY_DONT_CHECK_FILESIZE)))
      exit(1);
    old_format = check_header(file);
    if (position)
    {
      /* skip 'position' characters from stdout */
      byte buff[IO_SIZE];
      my_off_t length,tmp;
      for (length= (my_off_t) position ; length > 0 ; length-=tmp)
      {
	tmp=min(length,sizeof(buff));
	if (my_b_read(file, buff, (uint) tmp))
	  exit(1);
      }
    }
    file->pos_in_file=position;
    file->seek_not_done=0;
  }

  if (!position)
    my_b_read(file, tmp_buff, BIN_LOG_HEADER_SIZE); // Skip header
  for (;;)
  {
    char llbuff[21];
    my_off_t old_off = my_b_tell(file);

    Log_event* ev = Log_event::read_log_event(file, old_format);
    if (!ev)
    {
      if (file->error)
	die("\
Could not read entry at offset %s : Error in log format or read error",
	    llstr(old_off,llbuff));
      // file->error == 0 means EOF, that's OK, we break in this case
      break;
    }
    if (rec_count >= offset)
    {
      if (!short_form)
        fprintf(result_file, "# at %s\n",llstr(old_off,llbuff));
      
      switch (ev->get_type_code()) {
      case QUERY_EVENT:
        if (one_database)
        {
          const char * log_dbname = ((Query_log_event*)ev)->db;
          if ((log_dbname != NULL) && (strcmp(log_dbname, database)))
          {
            rec_count++;
            delete ev;
            continue; // next
          }
        }
	ev->print(result_file, short_form, last_db);
        break;
      case CREATE_FILE_EVENT:
      {
	Create_file_log_event* ce= (Create_file_log_event*)ev;
        if (one_database)
        {
          /*
            We test if this event has to be ignored. If yes, we don't save this
            event; this will have the good side-effect of ignoring all related
            Append_block and Exec_load.
            Note that Load event from 3.23 is not tested.
          */
          const char * log_dbname = ce->db;            
          if ((log_dbname != NULL) && (strcmp(log_dbname, database)))
          {
            rec_count++;
            delete ev;
            continue; // next
          }
        }
        /*
          We print the event, but with a leading '#': this is just to inform
	  the user of the original command; the command we want to execute
	  will be a derivation of this original command (we will change the
	  filename and use LOCAL), prepared in the 'case EXEC_LOAD_EVENT'
	  below.
        */
	ce->print(result_file, short_form, last_db, true);
	load_processor.process(ce);
	ev= 0;
	break;
      }
      case APPEND_BLOCK_EVENT:
	ev->print(result_file, short_form, last_db);
	load_processor.process((Append_block_log_event*)ev);
	break;
      case EXEC_LOAD_EVENT:
      {
	ev->print(result_file, short_form, last_db);
	Execute_load_log_event *exv= (Execute_load_log_event*)ev;
	Create_file_log_event *ce= load_processor.grab_event(exv->file_id);
        /*
          if ce is 0, it probably means that we have not seen the Create_file
          event (a bad binlog, or most probably --position is after the
          Create_file event). Print a warning comment.
        */
        if (ce)
        {
          ce->print(result_file, short_form, last_db,true);
          my_free((char*)ce->fname,MYF(MY_WME));
          delete ce;
        }
        else
          fprintf(stderr,"Warning: ignoring Exec_load as there is no \
Create_file event for file_id: %u\n",exv->file_id);
	break;
      }
      default:
	ev->print(result_file, short_form, last_db);
      }
    }
    rec_count++;
    if (ev)
      delete ev;
  }
  if (fd >= 0)
    my_close(fd, MYF(MY_WME));
  end_io_cache(file);
}

#if MYSQL_VERSION_ID < 40101

typedef struct st_my_tmpdir
{
  char **list;
  uint cur, max;
} MY_TMPDIR;

#if defined( __WIN__) || defined(OS2)
#define DELIM ';'
#else
#define DELIM ':'
#endif

my_bool init_tmpdir(MY_TMPDIR *tmpdir, const char *pathlist)
{
  char *end, *copy;
  char buff[FN_REFLEN];
  DYNAMIC_ARRAY t_arr;
  if (my_init_dynamic_array(&t_arr, sizeof(char*), 1, 5))
    return TRUE;
  if (!pathlist || !pathlist[0])
  {
    /* Get default temporary directory */
    pathlist=getenv("TMPDIR");	/* Use this if possible */
#if defined( __WIN__) || defined(OS2)
    if (!pathlist)
      pathlist=getenv("TEMP");
    if (!pathlist)
      pathlist=getenv("TMP");
#endif
    if (!pathlist || !pathlist[0])
      pathlist=(char*) P_tmpdir;
  }
  do
  {
    end=strcend(pathlist, DELIM);
    convert_dirname(buff, pathlist, end);
    if (!(copy=my_strdup(buff, MYF(MY_WME))))
      return TRUE;
    if (insert_dynamic(&t_arr, (gptr)&copy))
      return TRUE;
    pathlist=end+1;
  }
  while (*end);
  freeze_size(&t_arr);
  tmpdir->list=(char **)t_arr.buffer;
  tmpdir->max=t_arr.elements-1;
  tmpdir->cur=0;
  return FALSE;
}

char *my_tmpdir(MY_TMPDIR *tmpdir)
{
  char *dir;
  dir=tmpdir->list[tmpdir->cur];
  tmpdir->cur= (tmpdir->cur == tmpdir->max) ? 0 : tmpdir->cur+1;
  return dir;
}

void free_tmpdir(MY_TMPDIR *tmpdir)
{
  uint i;
  for (i=0; i<=tmpdir->max; i++)
    my_free(tmpdir->list[i], MYF(0));
  my_free((gptr)tmpdir->list, MYF(0));
}

#endif

int main(int argc, char** argv)
{
  static char **defaults_argv;
  MY_INIT(argv[0]);

  parse_args(&argc, (char***)&argv);
  defaults_argv=argv;

  if (!argc)
  {
    usage();
    free_defaults(defaults_argv);
    return -1;
  }

  if (remote_opt)
    mysql = safe_connect();

  MY_TMPDIR tmpdir;
  tmpdir.list= 0;
  if (!dirname_for_local_load)
  {
    if (init_tmpdir(&tmpdir, 0))
      exit(1);
    dirname_for_local_load= my_tmpdir(&tmpdir);
  }

  if (dirname_for_local_load)
    load_processor.init_by_dir_name(dirname_for_local_load);
  else
    load_processor.init_by_cur_dir();

  while (--argc >= 0)
    dump_log_entries(*(argv++));

  if (tmpdir.list)
    free_tmpdir(&tmpdir);
  if (result_file != stdout)
    my_fclose(result_file, MYF(0));
  if (remote_opt)
    mysql_close(mysql);
  free_defaults(defaults_argv);
  my_end(0);
  return 0;
}

/*
  We must include this here as it's compiled with different options for
  the server
*/

#ifdef __WIN__
#include "log_event.cpp"
#else
#include "log_event.cc"
#endif

FIX_GCC_LINKING_PROBLEM

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

#include "embedded_priv.h"
#include <my_sys.h>
#include <mysys_err.h>
#include <m_string.h>
#include <m_ctype.h>
#include "errmsg.h"
#include <violite.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#ifdef	 HAVE_PWD_H
#include <pwd.h>
#endif
#if !defined(MSDOS) && !defined(__WIN__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_SELECT_H
#  include <select.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif
#ifdef HAVE_SYS_UN_H
#  include <sys/un.h>
#endif
#ifndef INADDR_NONE
#define INADDR_NONE	-1
#endif

static my_bool	mysql_client_init=0;
uint		mysql_port=0;
my_string	mysql_unix_port=0;

#define CLIENT_CAPABILITIES	(CLIENT_LONG_PASSWORD | CLIENT_LONG_FLAG | CLIENT_TRANSACTIONS)

#if defined(MSDOS) || defined(__WIN__)
#define ERRNO WSAGetLastError()
#define perror(A)
#else
#include <errno.h>
#define ERRNO errno
#define SOCKET_ERROR -1
#define closesocket(A) close(A)
#endif

static void mysqld_once_init(void);
static MYSQL_DATA *read_rows (MYSQL *mysql,MYSQL_FIELD *fields,
			      uint field_count);
static int read_one_row(MYSQL *mysql,uint fields,MYSQL_ROW row,
			ulong *lengths);
static void end_server(MYSQL *mysql);
static void read_user_name(char *name);
static void append_wild(char *to,char *end,const char *wild);
static int send_file_to_server(MYSQL *mysql,const char *filename);
static ulong mysql_sub_escape_string(CHARSET_INFO *charset_info, char *to,
				     const char *from, ulong length);

#define init_sigpipe_variables
#define set_sigpipe(mysql)
#define reset_sigpipe(mysql)

/*****************************************************************************
** read a packet from server. Give error message if socket was down
** or packet is an error message
*****************************************************************************/

ulong
net_safe_read(MYSQL *mysql)
{
  NET *net= &mysql->net;
  uint len=0;
  /* init_sigpipe_variables */
  /* Don't give sigpipe errors if the client doesn't want them */
  set_sigpipe(mysql);
  if (net->vio != 0)
    len=my_net_read(net);
  reset_sigpipe(mysql);
  if (len == packet_error || len == 0)
  {
    DBUG_PRINT("error",("Wrong connection or packet. fd: %s  len: %d",
			vio_description(net->vio),len));
    end_server(mysql);
    net->last_errno=(net->last_errno == ER_NET_PACKET_TOO_LARGE ? 
		     CR_NET_PACKET_TOO_LARGE:
		     CR_SERVER_LOST);
    strmov(net->last_error,ER(net->last_errno));
    return(packet_error);
  }
  if (net->read_pos[0] == 255)
  {

    if (len > 3)
    {
      char *pos=(char*) net->read_pos+1;
      if (mysql->protocol_version > 9)
      {						/* New client protocol */
	net->last_errno=uint2korr(pos);
	pos+=2;
	len-=2;
      }
      else
      {
	net->last_errno=CR_UNKNOWN_ERROR;
	len--;
      }
      (void) strmake(net->last_error,(char*) pos,
		     min(len,sizeof(net->last_error)-1));
    }
    else
    {
      net->last_errno=CR_UNKNOWN_ERROR;
      (void) strmov(net->last_error,ER(net->last_errno));
    }
    DBUG_PRINT("error",("Got error: %d (%s)", net->last_errno,
			net->last_error));
    return(packet_error);
  }
  return len;
}


/* Get the length of next field. Change parameter to point at fieldstart */
static ulong
net_field_length(uchar **packet)
{
  reg1 uchar *pos= *packet;
  if (*pos < 251)
  {
    (*packet)++;
    return (ulong) *pos;
  }
  if (*pos == 251)
  {
    (*packet)++;
    return NULL_LENGTH;
  }
  if (*pos == 252)
  {
    (*packet)+=3;
    return (ulong) uint2korr(pos+1);
  }
  if (*pos == 253)
  {
    (*packet)+=4;
    return (ulong) uint3korr(pos+1);
  }
  (*packet)+=9;					/* Must be 254 when here */
  return (ulong) uint4korr(pos+1);
}

/* Same as above, but returns ulonglong values */

static my_ulonglong
net_field_length_ll(uchar **packet)
{
  reg1 uchar *pos= *packet;
  if (*pos < 251)
  {
    (*packet)++;
    return (my_ulonglong) *pos;
  }
  if (*pos == 251)
  {
    (*packet)++;
    return (my_ulonglong) NULL_LENGTH;
  }
  if (*pos == 252)
  {
    (*packet)+=3;
    return (my_ulonglong) uint2korr(pos+1);
  }
  if (*pos == 253)
  {
    (*packet)+=4;
    return (my_ulonglong) uint3korr(pos+1);
  }
  (*packet)+=9;					/* Must be 254 when here */
#ifdef NO_CLIENT_LONGLONG
  return (my_ulonglong) uint4korr(pos+1);
#else
  return (my_ulonglong) uint8korr(pos+1);
#endif
}


static void free_rows(MYSQL_DATA *cur)
{
  if (cur)
  {
    free_root(&cur->alloc,MYF(0));
    my_free((gptr) cur,MYF(0));
  }
}


int
simple_command(MYSQL *mysql,enum enum_server_command command, const char *arg,
	       ulong length, my_bool skipp_check)
{
  NET *net= &mysql->net;
  int result= -1;

  /* Check that we are calling the client functions in right order */
  if (mysql->status != MYSQL_STATUS_READY)
  {
    strmov(net->last_error,ER(mysql->net.last_errno=CR_COMMANDS_OUT_OF_SYNC));
    goto end;
  }

  /* Clear result variables */
  mysql->net.last_error[0]=0;
  mysql->net.last_errno=0;
  mysql->info=0;
  mysql->affected_rows= ~(my_ulonglong) 0;

  /* Clear receive buffer and vio packet list */
  net_clear(net);
  vio_reset(net->vio);

  result = lib_dispatch_command(command, net, arg,length);
  if (!skipp_check)
    result= ((mysql->packet_length=net_safe_read(mysql)) == packet_error ?
	     -1 : 0);
 end:
  return result;
}


static void free_old_query(MYSQL *mysql)
{
  DBUG_ENTER("free_old_query");
  if (mysql->fields)
    free_root(&mysql->field_alloc,MYF(0));
  init_alloc_root(&mysql->field_alloc,8192,0);	/* Assume rowlength < 8192 */
  mysql->fields=0;
  mysql->field_count=0;				/* For API */
  DBUG_VOID_RETURN;
}

#ifdef HAVE_GETPWUID
struct passwd *getpwuid(uid_t);
char* getlogin(void);
#endif

#if !defined(MSDOS) && ! defined(VMS) && !defined(__WIN__)
static void read_user_name(char *name)
{
  DBUG_ENTER("read_user_name");
  if (geteuid() == 0)
    (void) strmov(name,"root");		/* allow use of surun */
  else
  {
#ifdef HAVE_GETPWUID
    struct passwd *skr;
    const char *str;
/*#ifdef __cplusplus
    extern "C" struct passwd *getpwuid(uid_t);
    extern "C" { char* getlogin(void); }
#else
    char * getlogin();
    struct passwd *getpwuid(uid_t);
#endif
*/
    if ((str=getlogin()) == NULL)
    {
      if ((skr=getpwuid(geteuid())) != NULL)
	str=skr->pw_name;
      else if (!(str=getenv("USER")) && !(str=getenv("LOGNAME")) &&
	       !(str=getenv("LOGIN")))
	str="UNKNOWN_USER";
    }
    (void) strmake(name,str,USERNAME_LENGTH);
#elif HAVE_CUSERID
    (void) cuserid(name);
#else
    strmov(name,"UNKNOWN_USER");
#endif
  }
  DBUG_VOID_RETURN;
}

#else /* If MSDOS || VMS */

static void read_user_name(char *name)
{
  char *str=getenv("USER");
  strmov(name,str ? str : "ODBC");	 /* ODBC will send user variable */
}

#endif

#ifdef __WIN__
static my_bool is_NT(void)
{
  char *os=getenv("OS");
  return (os && !strcmp(os, "Windows_NT")) ? 1 : 0;
}
#endif

/*
** Expand wildcard to a sql string
*/

static void
append_wild(char *to, char *end, const char *wild)
{
  end-=5;					/* Some extra */
  if (wild && wild[0])
  {
    to=strmov(to," like '");
    while (*wild && to < end)
    {
      if (*wild == '\\' || *wild == '\'')
	*to++='\\';
      *to++= *wild++;
    }
    if (*wild)					/* Too small buffer */
      *to++='%';				/* Nicer this way */
    to[0]='\'';
    to[1]=0;
  }
}



/**************************************************************************
** Init debugging if MYSQL_DEBUG environment variable is found
**************************************************************************/

void STDCALL
mysql_debug(const char *debug)
{
#ifndef DBUG_OFF
  char	*env;
  if (_db_on_)
    return;					/* Already using debugging */
  if (debug)
  {
    DEBUGGER_ON;
    DBUG_PUSH(debug);
  }
  else if ((env = getenv("MYSQL_DEBUG")))
  {
    DEBUGGER_ON;
    DBUG_PUSH(env);
#if !defined(_WINVER) && !defined(WINVER)
    puts("\n-------------------------------------------------------");
    puts("MYSQL_DEBUG found. libmysql started with the following:");
    puts(env);
    puts("-------------------------------------------------------\n");
#else
    {
      char buff[80];
      strmov(strmov(buff,"libmysql: "),env);
      MessageBox((HWND) 0,"Debugging variable MYSQL_DEBUG used",buff,MB_OK);
    }
#endif
  }
#endif
}

/**************************************************************************
** Shut down connection
**************************************************************************/

static void
end_server(MYSQL *mysql)
{
  DBUG_ENTER("end_server");
  if (mysql->net.vio != 0)
  {
    end_embedded_connection(&mysql->net);
    mysql->net.vio= 0;          /* Marker */
  }
  net_end(&mysql->net);
  free_old_query(mysql);
  DBUG_VOID_RETURN;
}


void STDCALL
mysql_free_result(MYSQL_RES *result)
{
  DBUG_ENTER("mysql_free_result");
  DBUG_PRINT("enter",("mysql_res: %lx",result));
  if (result)
  {
    if (result->handle && result->handle->status == MYSQL_STATUS_USE_RESULT)
    {
      DBUG_PRINT("warning",("Not all rows in set were read; Ignoring rows"));
      for (;;)
      {
	uint pkt_len;
	if ((pkt_len=(uint) net_safe_read(result->handle)) == packet_error)
	  break;
	if (pkt_len == 1 && result->handle->net.read_pos[0] == 254)
	  break;				/* End of data */
      }
      result->handle->status=MYSQL_STATUS_READY;
    }
    free_rows(result->data);
    if (result->fields)
      free_root(&result->field_alloc,MYF(0));
    if (result->row)
      my_free((gptr) result->row,MYF(0));
    my_free((gptr) result,MYF(0));
  }
  DBUG_VOID_RETURN;
}


/****************************************************************************
** Get options from my.cnf
****************************************************************************/

static const char *default_options[]=
{"port","socket","compress","password","pipe", "timeout", "user",
 "init-command", "host", "database", "debug", "return-found-rows",
 "ssl_key" ,"ssl_cert" ,"ssl_ca" ,"ssl_capath",
 "character-set-dir", "default-character-set",
 NullS
};

static TYPELIB option_types={array_elements(default_options)-1,
			     "options",default_options};

static void mysql_read_default_options(struct st_mysql_options *options,
				       const char *filename,const char *group)
{
  int argc;
  char *argv_buff[1],**argv;
  const char *groups[3];
  DBUG_ENTER("mysql_read_default_options");
  DBUG_PRINT("enter",("file: %s  group: %s",filename,group ? group :"NULL"));

  argc=1; argv=argv_buff; argv_buff[0]= (char*) "client";
  groups[0]= (char*) "client"; groups[1]= (char*) group; groups[2]=0;

  load_defaults(filename, groups, &argc, &argv);
  if (argc != 1)				/* If some default option */
  {
    char **option=argv;
    while (*++option)
    {
      /* DBUG_PRINT("info",("option: %s",option[0])); */
      if (option[0][0] == '-' && option[0][1] == '-')
      {
	char *end=strcend(*option,'=');
	char *opt_arg=0;
	if (*end)
	{
	  opt_arg=end+1;
	  *end=0;				/* Remove '=' */
	}
	switch (find_type(*option+2,&option_types,2)) {
	case 1:				/* port */
	  if (opt_arg)
	    options->port=atoi(opt_arg);
	  break;
	case 2:				/* socket */
	  if (opt_arg)
	  {
	    my_free(options->unix_socket,MYF(MY_ALLOW_ZERO_PTR));
	    options->unix_socket=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 3:				/* compress */
	  options->compress=1;
	  break;
	case 4:				/* password */
	  if (opt_arg)
	  {
	    my_free(options->password,MYF(MY_ALLOW_ZERO_PTR));
	    options->password=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 5:				/* pipe */
	  options->named_pipe=1;	/* Force named pipe */
	  break;
	case 6:				/* timeout */
	  if (opt_arg)
	    options->connect_timeout=atoi(opt_arg);
	  break;
	case 7:				/* user */
	  if (opt_arg)
	  {
	    my_free(options->user,MYF(MY_ALLOW_ZERO_PTR));
	    options->user=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 8:				/* init-command */
	  if (opt_arg)
	  {
	    my_free(options->init_command,MYF(MY_ALLOW_ZERO_PTR));
	    options->init_command=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 9:				/* host */
	  if (opt_arg)
	  {
	    my_free(options->host,MYF(MY_ALLOW_ZERO_PTR));
	    options->host=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 10:			/* database */
	  if (opt_arg)
	  {
	    my_free(options->db,MYF(MY_ALLOW_ZERO_PTR));
	    options->db=my_strdup(opt_arg,MYF(MY_WME));
	  }
	  break;
	case 11:			/* debug */
	  mysql_debug(opt_arg ? opt_arg : "d:t:o,/tmp/client.trace");
	  break;
	case 12:			/* return-found-rows */
	  options->client_flag|=CLIENT_FOUND_ROWS;
	  break;
	case 13:				/* Ignore SSL options */
	case 14:
	case 15:
	case 16:
	  break;
	case 17:			/* charset-lib */
	  my_free(options->charset_dir,MYF(MY_ALLOW_ZERO_PTR));
          options->charset_dir = my_strdup(opt_arg, MYF(MY_WME));
	  break;
	case 18:
	  my_free(options->charset_name,MYF(MY_ALLOW_ZERO_PTR));
          options->charset_name = my_strdup(opt_arg, MYF(MY_WME));
	  break;
	default:
	  DBUG_PRINT("warning",("unknown option: %s",option[0]));
	}
      }
    }
  }
  free_defaults(argv);
  DBUG_VOID_RETURN;
}


/***************************************************************************
** Change field rows to field structs
***************************************************************************/

static MYSQL_FIELD *
unpack_fields(MYSQL_DATA *data,MEM_ROOT *alloc,uint fields,
	      my_bool default_value, my_bool long_flag_protocol)
{
  MYSQL_ROWS	*row;
  MYSQL_FIELD	*field,*result;
  DBUG_ENTER("unpack_fields");

  field=result=(MYSQL_FIELD*) alloc_root(alloc,sizeof(MYSQL_FIELD)*fields);
  if (!result)
    DBUG_RETURN(0);

  for (row=data->data; row ; row = row->next,field++)
  {
    field->table=  strdup_root(alloc,(char*) row->data[0]);
    field->name=   strdup_root(alloc,(char*) row->data[1]);
    field->length= (uint) uint3korr(row->data[2]);
    field->type=   (enum enum_field_types) (uchar) row->data[3][0];
    if (long_flag_protocol)
    {
      field->flags=   uint2korr(row->data[4]);
      field->decimals=(uint) (uchar) row->data[4][2];
    }
    else
    {
      field->flags=   (uint) (uchar) row->data[4][0];
      field->decimals=(uint) (uchar) row->data[4][1];
    }
    if (INTERNAL_NUM_FIELD(field))
      field->flags|= NUM_FLAG;
    if (default_value && row->data[5])
      field->def=strdup_root(alloc,(char*) row->data[5]);
    else
      field->def=0;
    field->max_length= 0;
  }
  free_rows(data);				/* Free old data */
  DBUG_RETURN(result);
}


/* Read all rows (fields or data) from server */

static MYSQL_DATA *read_rows(MYSQL *mysql,MYSQL_FIELD *mysql_fields,
			     uint fields)
{
  uint	field,pkt_len;
  ulong len;
  uchar *cp;
  char	*to;
  MYSQL_DATA *result;
  MYSQL_ROWS **prev_ptr,*cur;
  NET *net = &mysql->net;
  DBUG_ENTER("read_rows");

  if ((pkt_len=(uint) net_safe_read(mysql)) == packet_error)
    DBUG_RETURN(0);
  if (!(result=(MYSQL_DATA*) my_malloc(sizeof(MYSQL_DATA),
				       MYF(MY_WME | MY_ZEROFILL))))
  {
    net->last_errno=CR_OUT_OF_MEMORY;
    strmov(net->last_error,ER(net->last_errno));
    DBUG_RETURN(0);
  }
  init_alloc_root(&result->alloc,8192,0);	/* Assume rowlength < 8192 */
  result->alloc.min_malloc=sizeof(MYSQL_ROWS);
  prev_ptr= &result->data;
  result->rows=0;
  result->fields=fields;

  while (*(cp=net->read_pos) != 254 || pkt_len != 1)
  {
    result->rows++;
    if (!(cur= (MYSQL_ROWS*) alloc_root(&result->alloc,
					    sizeof(MYSQL_ROWS))) ||
	!(cur->data= ((MYSQL_ROW)
		      alloc_root(&result->alloc,
				     (fields+1)*sizeof(char *)+pkt_len))))
    {
      free_rows(result);
      net->last_errno=CR_OUT_OF_MEMORY;
      strmov(net->last_error,ER(net->last_errno));
      DBUG_RETURN(0);
    }
    *prev_ptr=cur;
    prev_ptr= &cur->next;
    to= (char*) (cur->data+fields+1);
    for (field=0 ; field < fields ; field++)
    {
      if ((len=(ulong) net_field_length(&cp)) == NULL_LENGTH)
      {						/* null field */
	cur->data[field] = 0;
      }
      else
      {
	cur->data[field] = to;
	memcpy(to,(char*) cp,len); to[len]=0;
	to+=len+1;
	cp+=len;
	if (mysql_fields)
	{
	  if (mysql_fields[field].max_length < len)
	    mysql_fields[field].max_length=len;
	}
      }
    }
    cur->data[field]=to;			/* End of last field */
    if ((pkt_len=net_safe_read(mysql)) == packet_error)
    {
      free_rows(result);
      DBUG_RETURN(0);
    }
  }
  *prev_ptr=0;					/* last pointer is null */
  DBUG_PRINT("exit",("Got %d rows",result->rows));
  DBUG_RETURN(result);
}


/*
** Read one row. Uses packet buffer as storage for fields.
** When next packet is read, the previous field values are destroyed
*/


static int
read_one_row(MYSQL *mysql,uint fields,MYSQL_ROW row, ulong *lengths)
{
  uint field;
  ulong pkt_len,len;
  uchar *pos,*prev_pos;

  if ((pkt_len=net_safe_read(mysql)) == packet_error)
    return -1;
  if (pkt_len == 1 && mysql->net.read_pos[0] == 254)
    return 1;				/* End of data */
  prev_pos= 0;				/* allowed to write at packet[-1] */
  pos=mysql->net.read_pos;
  for (field=0 ; field < fields ; field++)
  {
    if ((len=(ulong) net_field_length(&pos)) == NULL_LENGTH)
    {						/* null field */
      row[field] = 0;
      *lengths++=0;
    }
    else
    {
      row[field] = (char*) pos;
      pos+=len;
      *lengths++=len;
    }
    if (prev_pos)
      *prev_pos=0;				/* Terminate prev field */
    prev_pos=pos;
  }
  row[field]=(char*) prev_pos+1;		/* End of last field */
  *prev_pos=0;					/* Terminate last field */
  return 0;
}

/****************************************************************************
** Init MySQL structure or allocate one
****************************************************************************/

MYSQL * STDCALL
mysql_init(MYSQL *mysql)
{
  mysqld_once_init();
  if (!mysql)
  {
    if (!(mysql=(MYSQL*) my_malloc(sizeof(*mysql),MYF(MY_WME | MY_ZEROFILL))))
      return 0;
    mysql->free_me=1;
    mysql->net.vio = 0;
  }
  else
    bzero((char*) (mysql),sizeof(*(mysql)));
  return mysql;
}


static void mysqld_once_init()
{
  if (!mysql_client_init)
  {
    mysql_client_init=1;
    my_init();					/* Will init threads */
    init_client_errs();
    mysql_port = MYSQL_PORT;
    mysql_debug(NullS);
  }
#ifdef THREAD
  else
    my_thread_init();         /* Init if new thread */
#endif
}

/**************************************************************************
** Connect to sql server
** If host == 0 then use localhost
**************************************************************************/

MYSQL * STDCALL
mysql_connect(MYSQL *mysql,const char *host,
	      const char *user, const char *passwd)
{
  MYSQL *res;
  mysql=mysql_init(mysql);			/* Make it thread safe */
  {
    DBUG_ENTER("mysql_connect");
    if (!(res=mysql_real_connect(mysql,host,user,passwd,NullS,0,NullS,0)))
    {
      if (mysql->free_me)
	my_free((gptr) mysql,MYF(0));
    }
    DBUG_RETURN(res);
  }
}


/*
** Note that the mysql argument must be initialized with mysql_init()
** before calling mysql_real_connect !
*/

MYSQL * STDCALL
mysql_real_connect(MYSQL *mysql,const char *host, const char *user,
		   const char *passwd, const char *db,
		   uint port, const char *unix_socket,uint client_flag)
{
  char		buff[100],charset_name_buff[16],*end,*host_info, *charset_name;
  uint		pkt_length;
  ulong		max_allowed_packet;
  NET		*net= &mysql->net;
  DBUG_ENTER("mysql_real_connect");
  DBUG_PRINT("enter",("host: %s  db: %s  user: %s",
		      host ? host : "(Null)",
		      db ? db : "(Null)",
		      user ? user : "(Null)"));

  /* 
     Check mysql_server_init was called.
     This code shouldn't be merged to 4.1
  */
  if (!server_inited)
  {
    net->last_errno=CR_UNKNOWN_ERROR;
    strmov(net->last_error,ER(net->last_errno));
    goto error;
  }

  net->vio = 0;				/* If something goes wrong */
  /* use default options */
  if (mysql->options.my_cnf_file || mysql->options.my_cnf_group)
  {
    mysql_read_default_options(&mysql->options,
			       (mysql->options.my_cnf_file ?
				mysql->options.my_cnf_file : "my"),
			       mysql->options.my_cnf_group);
    my_free(mysql->options.my_cnf_file,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.my_cnf_group,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.my_cnf_file=mysql->options.my_cnf_group=0;
  }

  /* Some empty-string-tests are done because of ODBC */
  if (!host || !host[0])
    host=mysql->options.host;
  if (!user || !user[0])
    user=mysql->options.user;
  if (!passwd)
  {
    passwd=mysql->options.password;
  }
  if (!db || !db[0])
    db=mysql->options.db;
  port=0;
  unix_socket=0;
  mysql->reconnect=1;			/* Reconnect as default */
  mysql->server_status=SERVER_STATUS_AUTOCOMMIT;
  host_info=(char*) ER(CR_EMBEDDED_CONNECTION);
  if (my_net_init(net, net->vio))
  {
    vio_delete(net->vio);
    net->last_errno=CR_OUT_OF_MEMORY;
    strmov(net->last_error,ER(net->last_errno));
    goto error;
  }

  /* Get version info */
  mysql->protocol_version= PROTOCOL_VERSION;	/* Assume this */
  start_embedded_connection(net);

  if ((pkt_length=net_safe_read(mysql)) == packet_error)
    goto error;

  /* Check if version of protocoll matches current one */

  mysql->protocol_version= net->read_pos[0];
  DBUG_DUMP("packet",(char*) net->read_pos,10);
  DBUG_PRINT("info",("mysql protocol version %d, server=%d",
		     PROTOCOL_VERSION, mysql->protocol_version));
  if (mysql->protocol_version != PROTOCOL_VERSION &&
      mysql->protocol_version != PROTOCOL_VERSION-1)
  {
    net->last_errno= CR_VERSION_ERROR;
    sprintf(net->last_error, ER(CR_VERSION_ERROR), mysql->protocol_version,
	    PROTOCOL_VERSION);
    goto error;
  }
  end=strend((char*) net->read_pos+1);
  mysql->thread_id=uint4korr(end+1);
  end+=5;
  strmake(mysql->scramble_buff,end,8);
  end+=9;
  if (pkt_length >= (uint) (end+1 - (char*) net->read_pos))
    mysql->server_capabilities=uint2korr(end);
  if (pkt_length >= (uint) (end+18 - (char*) net->read_pos))
  {
    /* New protocol with 16 bytes to describe server characteristics */
    mysql->server_language=end[2];
    mysql->server_status=uint2korr(end+3);
  }

  /* Set character set */
  if ((charset_name=mysql->options.charset_name))
  {
    const char *save=charsets_dir;
    if (mysql->options.charset_dir)
      charsets_dir=mysql->options.charset_dir;
    mysql->charset=get_charset_by_name(mysql->options.charset_name,
                                       MYF(MY_WME));
    charsets_dir=save;
  }
  else if (mysql->server_language)
  {
    charset_name=charset_name_buff;
    sprintf(charset_name,"%d",mysql->server_language);	/* In case of errors */
    mysql->charset=get_charset((uint8) mysql->server_language, MYF(MY_WME));
  }
  else
    mysql->charset=default_charset_info;

  if (!mysql->charset)
  {
    net->last_errno=CR_CANT_READ_CHARSET;
    if (mysql->options.charset_dir)
      sprintf(net->last_error,ER(net->last_errno),
              charset_name ? charset_name : "unknown",
              mysql->options.charset_dir);
    else
    {
      char cs_dir_name[FN_REFLEN];
      get_charsets_dir(cs_dir_name);
      sprintf(net->last_error,ER(net->last_errno),
              charset_name ? charset_name : "unknown",
              cs_dir_name);
    }
    goto error;
  }

  /* Save connection information */
  if (!user) user="";
  if (!passwd) passwd="";
  host=LOCAL_HOST;
  if (!my_multi_malloc(MYF(0),
		       &mysql->host_info, (uint) strlen(host_info)+1,
		       &mysql->host,      (uint) strlen(host)+1,
		       &mysql->unix_socket,unix_socket ?
		       (uint) strlen(unix_socket)+1 : (uint) 1,
		       &mysql->server_version,
		       (uint) (end - (char*) net->read_pos),
		       NullS) ||
      !(mysql->user=my_strdup(user,MYF(0))) ||
      !(mysql->passwd=my_strdup(passwd,MYF(0))))
  {
    strmov(net->last_error, ER(net->last_errno=CR_OUT_OF_MEMORY));
    goto error;
  }
  strmov(mysql->host_info,host_info);
  strmov(mysql->host,host);
  if (unix_socket)
    strmov(mysql->unix_socket,unix_socket);
  else
    mysql->unix_socket=0;
  strmov(mysql->server_version,(char*) net->read_pos+1);
  mysql->port=port;
  mysql->client_flag=client_flag | mysql->options.client_flag;
  DBUG_PRINT("info",("Server version = '%s'  capabilites: %ld  status: %d",
		     mysql->server_version,mysql->server_capabilities,
		     mysql->server_status));

  /* Send client information for access check */
  client_flag|=CLIENT_CAPABILITIES;
  client_flag&= ~CLIENT_COMPRESS;
  if (db)
    client_flag|=CLIENT_CONNECT_WITH_DB;
  int2store(buff,client_flag);
  mysql->client_flag=client_flag;

  max_allowed_packet=net->max_packet_size;
  int3store(buff+2,max_allowed_packet);
  if (user && user[0])
    strmake(buff+5,user,32);
  else
    read_user_name((char*) buff+5);
#ifdef _CUSTOMCONFIG_
#include "_cust_libmysql.h";
#endif
  DBUG_PRINT("info",("user: %s",buff+5));
  end=scramble(strend(buff+5)+1, mysql->scramble_buff, passwd,
	       (my_bool) (mysql->protocol_version == 9));

  if (db)
  {
    end=strmov(end+1,db);
    mysql->db=my_strdup(db,MYF(MY_WME));
    db=0;
  }
  if (my_net_write(net,buff,(ulong) (end-buff)) || net_flush(net))
    goto error;

  lib_connection_phase(net,2);

  if( net_safe_read(mysql) == packet_error)
    goto error;
  if (db && mysql_select_db(mysql,db))
    goto error;
  if (mysql->options.init_command)
  {
    my_bool reconnect=mysql->reconnect;
    mysql->reconnect=0;
    if (mysql_query(mysql,mysql->options.init_command))
      goto error;
    mysql_free_result(mysql_use_result(mysql));
    mysql->reconnect=reconnect;
  }

  DBUG_PRINT("exit",("Mysql handler: %lx",mysql));
  reset_sigpipe(mysql);
  DBUG_RETURN(mysql);

error:
  reset_sigpipe(mysql);
  DBUG_PRINT("error",("message: %u (%s)",net->last_errno,net->last_error));
  {
    /* Free alloced memory */
    my_bool free_me=mysql->free_me;
    end_server(mysql);
    mysql->free_me=0;
    mysql_close(mysql);
    mysql->free_me=free_me;
  }
  DBUG_RETURN(0);
}


/**************************************************************************
** Change user and database 
**************************************************************************/

my_bool	STDCALL mysql_change_user(MYSQL *mysql, const char *user, 
				  const char *passwd, const char *db)
{
  char buff[512],*pos=buff;
  DBUG_ENTER("mysql_change_user");

  if (!user)
    user="";
  if (!passwd)
    passwd="";

  pos=strmov(pos,user)+1;
  pos=scramble(pos, mysql->scramble_buff, passwd,
	       (my_bool) (mysql->protocol_version == 9));
  pos=strmov(pos+1,db ? db : "");
  if (simple_command(mysql,COM_CHANGE_USER, buff,(ulong) (pos-buff),0))
    DBUG_RETURN(1);

  my_free(mysql->user,MYF(MY_ALLOW_ZERO_PTR));
  my_free(mysql->passwd,MYF(MY_ALLOW_ZERO_PTR));
  my_free(mysql->db,MYF(MY_ALLOW_ZERO_PTR));

  mysql->user=  my_strdup(user,MYF(MY_WME));
  mysql->passwd=my_strdup(passwd,MYF(MY_WME));
  mysql->db=    db ? my_strdup(db,MYF(MY_WME)) : 0;
  DBUG_RETURN(0);
}


/**************************************************************************
** Set current database
**************************************************************************/

int STDCALL
mysql_select_db(MYSQL *mysql, const char *db)
{
  int error;
  DBUG_ENTER("mysql_select_db");
  DBUG_PRINT("enter",("db: '%s'",db));

  if ((error=simple_command(mysql,COM_INIT_DB,db,(ulong) strlen(db),0)))
    DBUG_RETURN(error);
  my_free(mysql->db,MYF(MY_ALLOW_ZERO_PTR));
  mysql->db=my_strdup(db,MYF(MY_WME));
  DBUG_RETURN(0);
}


/*************************************************************************
** Send a QUIT to the server and close the connection
** If handle is alloced by mysql connect free it.
*************************************************************************/

void STDCALL
mysql_close(MYSQL *mysql)
{
  DBUG_ENTER("mysql_close");
  if (mysql)					/* Some simple safety */
  {
    if (mysql->net.vio != 0)
    {
      free_old_query(mysql);
      mysql->status=MYSQL_STATUS_READY; /* Force command */
      simple_command(mysql,COM_QUIT,"",0,1);
      end_server(mysql);
    }
    my_free((gptr) mysql->host_info,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->user,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->passwd,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->db,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.init_command,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.user,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.host,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.password,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.unix_socket,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.db,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.my_cnf_file,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.my_cnf_group,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.charset_dir,MYF(MY_ALLOW_ZERO_PTR));
    my_free(mysql->options.charset_name,MYF(MY_ALLOW_ZERO_PTR));
    /* Clear pointers for better safety */
    mysql->host_info=mysql->user=mysql->passwd=mysql->db=0;
    bzero((char*) &mysql->options,sizeof(mysql->options));
    mysql->net.vio = 0;
#ifdef HAVE_OPENSSL
    ((VioConnectorFd*)(mysql->connector_fd))->delete();
    mysql->connector_fd = 0;
#endif /* HAVE_OPENSSL */
    if (mysql->free_me)
      my_free((gptr) mysql,MYF(0));
  }
  DBUG_VOID_RETURN;
}


/**************************************************************************
** Do a query. If query returned rows, free old rows.
** Read data by mysql_store_result or by repeat call of mysql_fetch_row
**************************************************************************/

int STDCALL
mysql_query(MYSQL *mysql, const char *query)
{
  return mysql_real_query(mysql,query, (ulong) strlen(query));
}

int STDCALL
mysql_send_query(MYSQL* mysql, const char* query, ulong length)
{
  return simple_command(mysql, COM_QUERY, query, length, 1);
}


int STDCALL
mysql_read_query_result(MYSQL *mysql)
{
  uchar *pos;
  ulong field_count;
  MYSQL_DATA *fields;
  uint length;
  DBUG_ENTER("mysql_read_query_result");

  if ((length=net_safe_read(mysql)) == packet_error)
    DBUG_RETURN(-1);
  free_old_query(mysql);			/* Free old result */
get_info:
  pos=(uchar*) mysql->net.read_pos;
  if ((field_count= net_field_length(&pos)) == 0)
  {
    mysql->affected_rows= net_field_length_ll(&pos);
    mysql->insert_id=	  net_field_length_ll(&pos);
    if (mysql->server_capabilities & CLIENT_TRANSACTIONS)
    {
      mysql->server_status=uint2korr(pos); pos+=2;
    }
    if (pos < mysql->net.read_pos+length && net_field_length(&pos))
      mysql->info=(char*) pos;
    DBUG_RETURN(0);
  }
  if (field_count == NULL_LENGTH)		/* LOAD DATA LOCAL INFILE */
  {
    int error=send_file_to_server(mysql,(char*) pos);
    if ((length=net_safe_read(mysql)) == packet_error || error)
      DBUG_RETURN(-1);
    goto get_info;				/* Get info packet */
  }
  if (!(mysql->server_status & SERVER_STATUS_AUTOCOMMIT))
    mysql->server_status|= SERVER_STATUS_IN_TRANS;

  mysql->extra_info= net_field_length_ll(&pos); /* Maybe number of rec */
  if (!(fields=read_rows(mysql,(MYSQL_FIELD*) 0,5)))
    DBUG_RETURN(-1);
  if (!(mysql->fields=unpack_fields(fields,&mysql->field_alloc,
				    (uint) field_count,0,
				    (my_bool) test(mysql->server_capabilities &
						   CLIENT_LONG_FLAG))))
    DBUG_RETURN(-1);
  mysql->status=MYSQL_STATUS_GET_RESULT;
  mysql->field_count=field_count;
  DBUG_RETURN(0);
}

/****************************************************************************
* A modified version of connect().  connect2() allows you to specify
* a timeout value, in seconds, that we should wait until we
* derermine we can't connect to a particular host.  If timeout is 0,
* my_connect() will behave exactly like connect().
*
* Base version coded by Steve Bernacki, Jr. <steve@navinet.net>
*****************************************************************************/

int my_connect(my_socket s, const struct sockaddr *name, uint namelen,
		    uint timeout)
{
#if defined(__WIN__) || defined(OS2)
  return connect(s, (struct sockaddr*) name, namelen);
#else
  int flags, res, s_err;
  SOCKOPT_OPTLEN_TYPE s_err_size = sizeof(uint);
  fd_set sfds;
  struct timeval tv;
  time_t start_time, now_time;

  /* If they passed us a timeout of zero, we should behave
   * exactly like the normal connect() call does.
   */

  if (timeout == 0)
    return connect(s, (struct sockaddr*) name, namelen);

  flags = fcntl(s, F_GETFL, 0);		  /* Set socket to not block */
#ifdef O_NONBLOCK
  fcntl(s, F_SETFL, flags | O_NONBLOCK);  /* and save the flags..  */
#endif

  res = connect(s, (struct sockaddr*) name, namelen);
  s_err = errno;			/* Save the error... */
  fcntl(s, F_SETFL, flags);
  if ((res != 0) && (s_err != EINPROGRESS))
  {
    errno = s_err;			/* Restore it */
    return(-1);
  }
  if (res == 0)				/* Connected quickly! */
    return(0);

  /* Otherwise, our connection is "in progress."  We can use
   * the select() call to wait up to a specified period of time
   * for the connection to suceed.  If select() returns 0
   * (after waiting howevermany seconds), our socket never became
   * writable (host is probably unreachable.)  Otherwise, if
   * select() returns 1, then one of two conditions exist:
   *
   * 1. An error occured.  We use getsockopt() to check for this.
   * 2. The connection was set up sucessfully: getsockopt() will
   * return 0 as an error.
   *
   * Thanks goes to Andrew Gierth <andrew@erlenstar.demon.co.uk>
   * who posted this method of timing out a connect() in
   * comp.unix.programmer on August 15th, 1997.
   */

  FD_ZERO(&sfds);
  FD_SET(s, &sfds);
  /*
   * select could be interrupted by a signal, and if it is, 
   * the timeout should be adjusted and the select restarted
   * to work around OSes that don't restart select and 
   * implementations of select that don't adjust tv upon
   * failure to reflect the time remaining
   */
  start_time = time(NULL);
  for (;;)
  {
    tv.tv_sec = (long) timeout;
    tv.tv_usec = 0;
    if ((res = select(s+1, NULL, &sfds, NULL, &tv)) >= 0)
      break;
    now_time=time(NULL);
    timeout-= (uint) (now_time - start_time);
    if (errno != EINTR || (int) timeout <= 0)
      return -1;
  }

  /* select() returned something more interesting than zero, let's
   * see if we have any errors.  If the next two statements pass,
   * we've got an open socket!
   */

  s_err=0;
  if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char*) &s_err, &s_err_size) != 0)
    return(-1);

  if (s_err)
  {						/* getsockopt could succeed */
    errno = s_err;
    return(-1);					/* but return an error... */
  }
  return(0);					/* It's all good! */
#endif
}


int STDCALL
mysql_real_query(MYSQL *mysql, const char *query, ulong length)
{
  DBUG_ENTER("mysql_real_query");
  DBUG_PRINT("enter",("handle: %lx",mysql));
  DBUG_PRINT("query",("Query = \"%s\"",query));
  if (mysql_send_query(mysql, query, length))
    DBUG_RETURN(-1);
  DBUG_RETURN(mysql_read_query_result(mysql));
}


static int
send_file_to_server(MYSQL *mysql, const char *filename)
{
  int fd, readcount;
  char buf[IO_SIZE*15],*tmp_name;
  DBUG_ENTER("send_file_to_server");

  fn_format(buf,filename,"","",4);		/* Convert to client format */
  if (!(tmp_name=my_strdup(buf,MYF(0))))
  {
    strmov(mysql->net.last_error, ER(mysql->net.last_errno=CR_OUT_OF_MEMORY));
    DBUG_RETURN(-1);
  }
  if ((fd = my_open(tmp_name,O_RDONLY, MYF(0))) < 0)
  {
    mysql->net.last_errno=EE_FILENOTFOUND;
    sprintf(buf,EE(mysql->net.last_errno),tmp_name,errno);
    strmake(mysql->net.last_error,buf,sizeof(mysql->net.last_error)-1);
    my_net_write(&mysql->net,"",0); net_flush(&mysql->net);
    my_free(tmp_name,MYF(0));
    DBUG_RETURN(-1);
  }

  while ((readcount = (int) my_read(fd,buf,sizeof(buf),MYF(0))) > 0)
  {
    if (my_net_write(&mysql->net,buf,readcount))
    {
      mysql->net.last_errno=CR_SERVER_LOST;
      strmov(mysql->net.last_error,ER(mysql->net.last_errno));
      DBUG_PRINT("error",("Lost connection to MySQL server during LOAD DATA of local file"));
      (void) my_close(fd,MYF(0));
      my_free(tmp_name,MYF(0));
      DBUG_RETURN(-1);
    }
  }
  (void) my_close(fd,MYF(0));
  /* Send empty packet to mark end of file */
  if (my_net_write(&mysql->net,"",0) || net_flush(&mysql->net))
  {
    mysql->net.last_errno=CR_SERVER_LOST;
    sprintf(mysql->net.last_error,ER(mysql->net.last_errno),errno);
    my_free(tmp_name,MYF(0));
    DBUG_RETURN(-1);
  }
  if (readcount < 0)
  {
    mysql->net.last_errno=EE_READ; /* the errmsg for not entire file read */
    sprintf(buf,EE(mysql->net.last_errno),tmp_name,errno);
    strmake(mysql->net.last_error,buf,sizeof(mysql->net.last_error)-1);
    my_free(tmp_name,MYF(0));
    DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}


/**************************************************************************
** Alloc result struct for buffered results. All rows are read to buffer.
** mysql_data_seek may be used.
**************************************************************************/

MYSQL_RES * STDCALL
mysql_store_result(MYSQL *mysql)
{
  MYSQL_RES *result;
  DBUG_ENTER("mysql_store_result");

  if (!mysql->fields)
    DBUG_RETURN(0);
  if (mysql->status != MYSQL_STATUS_GET_RESULT)
  {
    strmov(mysql->net.last_error,
	   ER(mysql->net.last_errno=CR_COMMANDS_OUT_OF_SYNC));
    DBUG_RETURN(0);
  }
  mysql->status=MYSQL_STATUS_READY;		/* server is ready */
  if (!(result=(MYSQL_RES*) my_malloc(sizeof(MYSQL_RES)+
				      sizeof(ulong)*mysql->field_count,
				      MYF(MY_WME | MY_ZEROFILL))))
  {
    mysql->net.last_errno=CR_OUT_OF_MEMORY;
    strmov(mysql->net.last_error, ER(mysql->net.last_errno));
    DBUG_RETURN(0);
  }
  result->eof=1;				/* Marker for buffered */
  result->lengths=(ulong*) (result+1);
  if (!(result->data=read_rows(mysql,mysql->fields,mysql->field_count)))
  {
    my_free((gptr) result,MYF(0));
    DBUG_RETURN(0);
  }
  mysql->affected_rows= result->row_count= result->data->rows;
  result->data_cursor=	result->data->data;
  result->fields=	mysql->fields;
  result->field_alloc=	mysql->field_alloc;
  result->field_count=	mysql->field_count;
  result->current_field=0;
  result->current_row=0;			/* Must do a fetch first */
  mysql->fields=0;				/* fields is now in result */
  DBUG_RETURN(result);				/* Data fetched */
}


/**************************************************************************
** Alloc struct for use with unbuffered reads. Data is fetched by domand
** when calling to mysql_fetch_row.
** mysql_data_seek is a noop.
**
** No other queries may be specified with the same MYSQL handle.
** There shouldn't be much processing per row because mysql server shouldn't
** have to wait for the client (and will not wait more than 30 sec/packet).
**************************************************************************/

MYSQL_RES * STDCALL
mysql_use_result(MYSQL *mysql)
{
  MYSQL_RES *result;
  DBUG_ENTER("mysql_use_result");

  if (!mysql->fields)
    DBUG_RETURN(0);
  if (mysql->status != MYSQL_STATUS_GET_RESULT)
  {
    strmov(mysql->net.last_error,
	   ER(mysql->net.last_errno=CR_COMMANDS_OUT_OF_SYNC));
    DBUG_RETURN(0);
  }
  if (!(result=(MYSQL_RES*) my_malloc(sizeof(*result)+
				      sizeof(ulong)*mysql->field_count,
				      MYF(MY_WME | MY_ZEROFILL))))
    DBUG_RETURN(0);
  result->lengths=(ulong*) (result+1);
  if (!(result->row=(MYSQL_ROW)
	my_malloc(sizeof(result->row[0])*(mysql->field_count+1), MYF(MY_WME))))
  {					/* Ptrs: to one row */
    my_free((gptr) result,MYF(0));
    DBUG_RETURN(0);
  }
  result->fields=	mysql->fields;
  result->field_alloc=	mysql->field_alloc;
  result->field_count=	mysql->field_count;
  result->current_field=0;
  result->handle=	mysql;
  result->current_row=	0;
  mysql->fields=0;			/* fields is now in result */
  mysql->status=MYSQL_STATUS_USE_RESULT;
  DBUG_RETURN(result);			/* Data is read to be fetched */
}



/**************************************************************************
** Return next field of the query results
**************************************************************************/

MYSQL_FIELD * STDCALL
mysql_fetch_field(MYSQL_RES *result)
{
  if (result->current_field >= result->field_count)
    return(NULL);
  return &result->fields[result->current_field++];
}


/**************************************************************************
**  Return next row of the query results
**************************************************************************/

MYSQL_ROW STDCALL
mysql_fetch_row(MYSQL_RES *res)
{
  DBUG_ENTER("mysql_fetch_row");
  if (!res->data)
  {						/* Unbufferred fetch */
    if (!res->eof)
    {
      if (!(read_one_row(res->handle,res->field_count,res->row, res->lengths)))
      {
	res->row_count++;
	DBUG_RETURN(res->current_row=res->row);
      }
      else
      {
	DBUG_PRINT("info",("end of data"));
	res->eof=1;
	res->handle->status=MYSQL_STATUS_READY;
      }
    }
    DBUG_RETURN((MYSQL_ROW) NULL);
  }
  {
    MYSQL_ROW tmp;
    if (!res->data_cursor)
    {
      DBUG_PRINT("info",("end of data"));
      DBUG_RETURN(res->current_row=(MYSQL_ROW) NULL);
    }
    tmp = res->data_cursor->data;
    res->data_cursor = res->data_cursor->next;
    DBUG_RETURN(res->current_row=tmp);
  }
}

/**************************************************************************
** Get column lengths of the current row
** If one uses mysql_use_result, res->lengths contains the length information,
** else the lengths are calculated from the offset between pointers.
**************************************************************************/

ulong * STDCALL
mysql_fetch_lengths(MYSQL_RES *res)
{
  ulong *lengths,*prev_length;
  byte *start;
  MYSQL_ROW column,end;

  if (!(column=res->current_row))
    return 0;					/* Something is wrong */
  if (res->data)
  {
    start=0;
    prev_length=0;				/* Keep gcc happy */
    lengths=res->lengths;
    for (end=column+res->field_count+1 ; column != end ; column++,lengths++)
    {
      if (!*column)
      {
	*lengths=0;				/* Null */
	continue;
      }
      if (start)				/* Found end of prev string */
	*prev_length= (uint) (*column-start-1);
      start= *column;
      prev_length=lengths;
    }
  }
  return res->lengths;
}

/**************************************************************************
** Move to a specific row and column
**************************************************************************/

void STDCALL
mysql_data_seek(MYSQL_RES *result, my_ulonglong row)
{
  MYSQL_ROWS	*tmp=0;
  DBUG_PRINT("info",("mysql_data_seek(%ld)",(long) row));
  if (result->data)
    for (tmp=result->data->data; row-- && tmp ; tmp = tmp->next) ;
  result->current_row=0;
  result->data_cursor = tmp;
}

/*************************************************************************
** put the row or field cursor one a position one got from mysql_row_tell()
** This doesn't restore any data. The next mysql_fetch_row or
** mysql_fetch_field will return the next row or field after the last used
*************************************************************************/

MYSQL_ROW_OFFSET STDCALL
mysql_row_seek(MYSQL_RES *result, MYSQL_ROW_OFFSET row)
{
  MYSQL_ROW_OFFSET return_value=result->data_cursor;
  result->current_row= 0;
  result->data_cursor= row;
  return return_value;
}


MYSQL_FIELD_OFFSET STDCALL
mysql_field_seek(MYSQL_RES *result, MYSQL_FIELD_OFFSET field_offset)
{
  MYSQL_FIELD_OFFSET return_value=result->current_field;
  result->current_field=field_offset;
  return return_value;
}

/*****************************************************************************
** List all databases
*****************************************************************************/

MYSQL_RES * STDCALL
mysql_list_dbs(MYSQL *mysql, const char *wild)
{
  char buff[255];
  DBUG_ENTER("mysql_list_dbs");

  append_wild(strmov(buff,"show databases"),buff+sizeof(buff),wild);
  if (mysql_query(mysql,buff))
    DBUG_RETURN(0);
  DBUG_RETURN (mysql_store_result(mysql));
}


/*****************************************************************************
** List all tables in a database
** If wild is given then only the tables matching wild is returned
*****************************************************************************/

MYSQL_RES * STDCALL
mysql_list_tables(MYSQL *mysql, const char *wild)
{
  char buff[255];
  DBUG_ENTER("mysql_list_tables");

  append_wild(strmov(buff,"show tables"),buff+sizeof(buff),wild);
  if (mysql_query(mysql,buff))
    DBUG_RETURN(0);
  DBUG_RETURN (mysql_store_result(mysql));
}


/**************************************************************************
** List all fields in a table
** If wild is given then only the fields matching wild is returned
** Instead of this use query:
** show fields in 'table' like "wild"
**************************************************************************/

MYSQL_RES * STDCALL
mysql_list_fields(MYSQL *mysql, const char *table, const char *wild)
{
  MYSQL_RES *result;
  MYSQL_DATA *query;
  char	     buff[257],*end;
  DBUG_ENTER("mysql_list_fields");
  DBUG_PRINT("enter",("table: '%s'  wild: '%s'",table,wild ? wild : ""));

  LINT_INIT(query);

  end=strmake(strmake(buff, table,128)+1,wild ? wild : "",128);
  if (simple_command(mysql,COM_FIELD_LIST,buff,(uint) (end-buff),1) ||
      !(query = read_rows(mysql,(MYSQL_FIELD*) 0,6)))
    DBUG_RETURN(NULL);

  free_old_query(mysql);
  if (!(result = (MYSQL_RES *) my_malloc(sizeof(MYSQL_RES),
					 MYF(MY_WME | MY_ZEROFILL))))
  {
    free_rows(query);
    DBUG_RETURN(NULL);
  }
  result->field_alloc=mysql->field_alloc;
  mysql->fields=0;
  result->field_count = (uint) query->rows;
  result->fields= unpack_fields(query,&result->field_alloc,
				result->field_count,1,
				(my_bool) test(mysql->server_capabilities &
					       CLIENT_LONG_FLAG));
  result->eof=1;
  DBUG_RETURN(result);
}

/* List all running processes (threads) in server */

MYSQL_RES * STDCALL
mysql_list_processes(MYSQL *mysql)
{
  MYSQL_DATA *fields;
  uint field_count;
  uchar *pos;
  DBUG_ENTER("mysql_list_processes");

  LINT_INIT(fields);
  if (simple_command(mysql,COM_PROCESS_INFO,"",0,0))
    DBUG_RETURN(0);
  free_old_query(mysql);
  pos=(uchar*) mysql->net.read_pos;
  field_count=(uint) net_field_length(&pos);
  if (!(fields = read_rows(mysql,(MYSQL_FIELD*) 0,5)))
    DBUG_RETURN(NULL);
  if (!(mysql->fields=unpack_fields(fields,&mysql->field_alloc,field_count,0,
				    (my_bool) test(mysql->server_capabilities &
						   CLIENT_LONG_FLAG))))
    DBUG_RETURN(0);
  mysql->status=MYSQL_STATUS_GET_RESULT;
  mysql->field_count=field_count;
  DBUG_RETURN(mysql_store_result(mysql));
}


int  STDCALL
mysql_create_db(MYSQL *mysql, const char *db)
{
  DBUG_ENTER("mysql_createdb");
  DBUG_PRINT("enter",("db: %s",db));
  DBUG_RETURN(simple_command(mysql,COM_CREATE_DB,db, (ulong) strlen(db),0));
}


int  STDCALL
mysql_drop_db(MYSQL *mysql, const char *db)
{
  DBUG_ENTER("mysql_drop_db");
  DBUG_PRINT("enter",("db: %s",db));
  DBUG_RETURN(simple_command(mysql,COM_DROP_DB,db,(ulong) strlen(db),0));
}


int STDCALL
mysql_shutdown(MYSQL *mysql)
{
  DBUG_ENTER("mysql_shutdown");
  DBUG_RETURN(simple_command(mysql,COM_SHUTDOWN,"",0,0));
}


int STDCALL
mysql_refresh(MYSQL *mysql,uint options)
{
  uchar bits[1];
  DBUG_ENTER("mysql_refresh");
  bits[0]= (uchar) options;
  DBUG_RETURN(simple_command(mysql,COM_REFRESH,(char*) bits,1,0));
}

int STDCALL
mysql_kill(MYSQL *mysql,ulong pid)
{
  char buff[12];
  DBUG_ENTER("mysql_kill");
  int4store(buff,pid);
  DBUG_RETURN(simple_command(mysql,COM_PROCESS_KILL,buff,4,0));
}


int STDCALL
mysql_dump_debug_info(MYSQL *mysql)
{
  DBUG_ENTER("mysql_dump_debug_info");
  DBUG_RETURN(simple_command(mysql,COM_DEBUG,"",0,0));
}

const char * STDCALL
mysql_stat(MYSQL *mysql)
{
  DBUG_ENTER("mysql_stat");
  if (simple_command(mysql,COM_STATISTICS,"",0,0))
    return mysql->net.last_error;
  mysql->net.read_pos[mysql->packet_length]=0;	/* End of stat string */
  if (!mysql->net.read_pos[0])
  {
    mysql->net.last_errno=CR_WRONG_HOST_INFO;
    strmov(mysql->net.last_error, ER(mysql->net.last_errno));
    return mysql->net.last_error;
  }
  DBUG_RETURN((char*) mysql->net.read_pos);
}


int STDCALL
mysql_ping(MYSQL *mysql)
{
  DBUG_ENTER("mysql_ping");
  DBUG_RETURN(simple_command(mysql,COM_PING,"",0,0));
}


const char * STDCALL
mysql_get_server_info(MYSQL *mysql)
{
  return((char*) mysql->server_version);
}


const char * STDCALL
mysql_get_host_info(MYSQL *mysql)
{
  return(mysql->host_info);
}


uint STDCALL
mysql_get_proto_info(MYSQL *mysql)
{
  return (mysql->protocol_version);
}

const char * STDCALL
mysql_get_client_info(void)
{
  return MYSQL_SERVER_VERSION;
}


int STDCALL
mysql_options(MYSQL *mysql,enum mysql_option option, const char *arg)
{
  DBUG_ENTER("mysql_option");
  DBUG_PRINT("enter",("option: %d",(int) option));
  switch (option) {
  case MYSQL_OPT_CONNECT_TIMEOUT:
    mysql->options.connect_timeout= *(uint*) arg;
    break;
  case MYSQL_OPT_COMPRESS:
    mysql->options.compress=1;			/* Remember for connect */
    break;
  case MYSQL_OPT_NAMED_PIPE:
    mysql->options.named_pipe=1;		/* Force named pipe */
    break;
  case MYSQL_INIT_COMMAND:
    my_free(mysql->options.init_command,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.init_command=my_strdup(arg,MYF(MY_WME));
    break;
  case MYSQL_READ_DEFAULT_FILE:
    my_free(mysql->options.my_cnf_file,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.my_cnf_file=my_strdup(arg,MYF(MY_WME));
    break;
  case MYSQL_READ_DEFAULT_GROUP:
    my_free(mysql->options.my_cnf_group,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.my_cnf_group=my_strdup(arg,MYF(MY_WME));
    break;
  case MYSQL_SET_CHARSET_DIR:
    my_free(mysql->options.charset_dir,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.charset_dir=my_strdup(arg,MYF(MY_WME));
    break;
  case MYSQL_SET_CHARSET_NAME:
    my_free(mysql->options.charset_name,MYF(MY_ALLOW_ZERO_PTR));
    mysql->options.charset_name=my_strdup(arg,MYF(MY_WME));
    break;
  default:
    DBUG_RETURN(-1);
  }
  DBUG_RETURN(0);
}

/****************************************************************************
** Functions to get information from the MySQL structure
** These are functions to make shared libraries more usable.
****************************************************************************/

/* MYSQL_RES */
my_ulonglong STDCALL mysql_num_rows(MYSQL_RES *res)
{
  return res->row_count;
}

unsigned int STDCALL mysql_num_fields(MYSQL_RES *res)
{
  return res->field_count;
}

my_bool STDCALL mysql_eof(MYSQL_RES *res)
{
  return res->eof;
}

MYSQL_FIELD * STDCALL mysql_fetch_field_direct(MYSQL_RES *res,uint fieldnr)
{
  return &(res)->fields[fieldnr];
}

MYSQL_FIELD * STDCALL mysql_fetch_fields(MYSQL_RES *res)
{
  return (res)->fields;
}

MYSQL_ROWS * STDCALL mysql_row_tell(MYSQL_RES *res)
{
  return res->data_cursor;
}

uint STDCALL mysql_field_tell(MYSQL_RES *res)
{
  return (res)->current_field;
}

/* MYSQL */

unsigned int STDCALL mysql_field_count(MYSQL *mysql)
{
  return mysql->field_count;
}

my_ulonglong STDCALL mysql_affected_rows(MYSQL *mysql)
{
  return (mysql)->affected_rows;
}

my_ulonglong STDCALL mysql_insert_id(MYSQL *mysql)
{
  return (mysql)->insert_id;
}

uint STDCALL mysql_errno(MYSQL *mysql)
{
  return (mysql)->net.last_errno;
}

const char * STDCALL mysql_error(MYSQL *mysql)
{
  return (mysql)->net.last_error;
}

const char *STDCALL mysql_info(MYSQL *mysql)
{
  return (mysql)->info;
}

ulong STDCALL mysql_thread_id(MYSQL *mysql)
{
  return (mysql)->thread_id;
}

const char * STDCALL mysql_character_set_name(MYSQL *mysql)
{
  return mysql->charset->name;
}


uint STDCALL mysql_thread_safe(void)
{
#ifdef THREAD
  return 1;
#else
  return 0;
#endif
}

/****************************************************************************
** Some support functions
****************************************************************************/

/*
** Add escape characters to a string (blob?) to make it suitable for a insert
** to should at least have place for length*2+1 chars
** Returns the length of the to string
*/

ulong STDCALL
mysql_escape_string(char *to,const char *from,ulong length)
{
  return mysql_sub_escape_string(default_charset_info,to,from,length);
}

ulong STDCALL
mysql_real_escape_string(MYSQL *mysql, char *to,const char *from,
			 ulong length)
{
  return mysql_sub_escape_string(mysql->charset,to,from,length);
}


static ulong
mysql_sub_escape_string(CHARSET_INFO *charset_info, char *to,
			const char *from, ulong length)
{
  const char *to_start=to;
  const char *end;
#ifdef USE_MB
  my_bool use_mb_flag=use_mb(charset_info);
#endif
  for (end=from+length; from != end ; from++)
  {
#ifdef USE_MB
    int l;
    if (use_mb_flag && (l = my_ismbchar(charset_info, from, end)))
    {
      while (l--)
	  *to++ = *from++;
      from--;
      continue;
    }
#endif
    switch (*from) {
    case 0:				/* Must be escaped for 'mysql' */
      *to++= '\\';
      *to++= '0';
      break;
    case '\n':				/* Must be escaped for logs */
      *to++= '\\';
      *to++= 'n';
      break;
    case '\r':
      *to++= '\\';
      *to++= 'r';
      break;
    case '\\':
      *to++= '\\';
      *to++= '\\';
      break;
    case '\'':
      *to++= '\\';
      *to++= '\'';
      break;
    case '"':				/* Better safe than sorry */
      *to++= '\\';
      *to++= '"';
      break;
    case '\032':			/* This gives problems on Win32 */
      *to++= '\\';
      *to++= 'Z';
      break;
    default:
      *to++= *from;
    }
  }
  *to=0;
  return (ulong) (to-to_start);
}


char * STDCALL
mysql_odbc_escape_string(MYSQL *mysql,
			 char *to, ulong to_length,
			 const char *from, ulong from_length,
			 void *param,
			 char * (*extend_buffer)
			 (void *, char *, ulong *))
{
  char *to_end=to+to_length-5;
  const char *end;
#ifdef USE_MB
  my_bool use_mb_flag=use_mb(mysql->charset);
#endif

  for (end=from+from_length; from != end ; from++)
  {
    if (to >= to_end)
    {
      to_length = (ulong) (end-from)+512;	/* We want this much more */
      if (!(to=(*extend_buffer)(param, to, &to_length)))
	return to;
      to_end=to+to_length-5;
    }
#ifdef USE_MB
    {
      int l;
      if (use_mb_flag && (l = my_ismbchar(mysql->charset, from, end)))
      {
	while (l--)
	  *to++ = *from++;
	from--;
	continue;
      }
    }
#endif
    switch (*from) {
    case 0:				/* Must be escaped for 'mysql' */
      *to++= '\\';
      *to++= '0';
      break;
    case '\n':				/* Must be escaped for logs */
      *to++= '\\';
      *to++= 'n';
      break;
    case '\r':
      *to++= '\\';
      *to++= 'r';
      break;
    case '\\':
      *to++= '\\';
      *to++= '\\';
      break;
    case '\'':
      *to++= '\\';
      *to++= '\'';
      break;
    case '"':				/* Better safe than sorry */
      *to++= '\\';
      *to++= '"';
      break;
    case '\032':			/* This gives problems on Win32 */
      *to++= '\\';
      *to++= 'Z';
      break;
    default:
      *to++= *from;
    }
  }
  return to;
}

void STDCALL
myodbc_remove_escape(MYSQL *mysql,char *name)
{
  char *to;
#ifdef USE_MB
  my_bool use_mb_flag=use_mb(mysql->charset);
  char *end;
  LINT_INIT(end);
  if (use_mb_flag)
    for (end=name; *end ; end++) ;
#endif

  for (to=name ; *name ; name++)
  {
#ifdef USE_MB
    int l;
    if (use_mb_flag && (l = my_ismbchar( mysql->charset, name , end ) ) )
    {
      while (l--)
	*to++ = *name++;
      name--;
      continue;
    }
#endif
    if (*name == '\\' && name[1])
      name++;
    *to++= *name;
  }
  *to=0;
}

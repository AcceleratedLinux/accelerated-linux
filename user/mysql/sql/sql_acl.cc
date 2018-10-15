/* Copyright (C) 2000-2003 MySQL AB

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
  The privileges are saved in the following tables:
  mysql/user	 ; super user who are allowed to do almoust anything
  mysql/host	 ; host priviliges. This is used if host is empty in mysql/db.
  mysql/db	 ; database privileges / user

  data in tables is sorted according to how many not-wild-cards there is
  in the relevant fields. Empty strings comes last.
*/

#include "mysql_priv.h"
#include "sql_acl.h"
#include "hash_filo.h"
#ifdef HAVE_REPLICATION
#include "sql_repl.h" //for tables_ok()
#endif
#include <m_ctype.h>
#include <assert.h>
#include <stdarg.h>

struct acl_host_and_ip
{
  char *hostname;
  long ip,ip_mask;			// Used with masked ip:s
};


class ACL_ACCESS {
public:
  ulong sort;
  ulong access;
};


/* ACL_HOST is used if no host is specified */

class ACL_HOST :public ACL_ACCESS
{
public:
  acl_host_and_ip host;
  char *db;
};


class ACL_USER :public ACL_ACCESS
{
public:
  acl_host_and_ip host;
  uint hostname_length;
  USER_RESOURCES user_resource;
  char *user,*password;
  ulong salt[2];
  enum SSL_type ssl_type;
  const char *ssl_cipher, *x509_issuer, *x509_subject;
};


class ACL_DB :public ACL_ACCESS
{
public:
  acl_host_and_ip host;
  char *user,*db;
};


class acl_entry :public hash_filo_element
{
public:
  ulong access;
  uint16 length;
  char key[1];					// Key will be stored here
};


static byte* acl_entry_get_key(acl_entry *entry,uint *length,
			       my_bool not_used __attribute__((unused)))
{
  *length=(uint) entry->length;
  return (byte*) entry->key;
}

#define ACL_KEY_LENGTH (sizeof(long)+NAME_LEN+17)

static DYNAMIC_ARRAY acl_hosts,acl_users,acl_dbs;
static MEM_ROOT mem, memex;
static bool initialized=0;
static bool allow_all_hosts=1;
static HASH acl_check_hosts, hash_tables;
static DYNAMIC_ARRAY acl_wild_hosts;
static hash_filo *acl_cache;
static uint grant_version=0;
static ulong get_access(TABLE *form,uint fieldnr);
static int acl_compare(ACL_ACCESS *a,ACL_ACCESS *b);
static ulong get_sort(uint count,...);
static void init_check_host(void);
static ACL_USER *find_acl_user(const char *host, const char *user);
static bool update_user_table(THD *thd, const char *host, const char *user,
			      const char *new_password);
static void update_hostname(acl_host_and_ip *host, const char *hostname);
static bool compare_hostname(const acl_host_and_ip *host,const char *hostname,
			     const char *ip);

/*
  Read grant privileges from the privilege tables in the 'mysql' database.

  SYNOPSIS
    acl_init()
    thd				Thread handler
    dont_read_acl_tables	Set to 1 if run with --skip-grant

  RETURN VALUES
    0	ok
    1	Could not initialize grant's
*/


my_bool acl_init(THD *org_thd, bool dont_read_acl_tables)
{
  THD  *thd;
  TABLE_LIST tables[3];
  TABLE *table;
  READ_RECORD read_record_info;
  MYSQL_LOCK *lock;
  my_bool return_val=1;
  DBUG_ENTER("acl_init");

  if (!acl_cache)
    acl_cache=new hash_filo(ACL_CACHE_SIZE,0,0,
			    (hash_get_key) acl_entry_get_key,
			    (hash_free_key) free);
  if (dont_read_acl_tables)
    DBUG_RETURN(0); /* purecov: tested */

  /*
    To be able to run this from boot, we allocate a temporary THD
  */
  if (!(thd=new THD))
    DBUG_RETURN(1); /* purecov: inspected */
  thd->store_globals();

  acl_cache->clear(1);				// Clear locked hostname cache
  thd->db= my_strdup("mysql",MYF(0));
  thd->db_length=5;				// Safety
  bzero((char*) &tables,sizeof(tables));
  tables[0].alias=tables[0].real_name=(char*) "host";
  tables[1].alias=tables[1].real_name=(char*) "user";
  tables[2].alias=tables[2].real_name=(char*) "db";
  tables[0].next=tables+1;
  tables[1].next=tables+2;
  tables[0].lock_type=tables[1].lock_type=tables[2].lock_type=TL_READ;
  tables[0].db=tables[1].db=tables[2].db=thd->db;

  if (open_tables(thd,tables))
  {
    sql_print_error("Fatal error: Can't open privilege tables: %s",
		    thd->net.last_error);
    goto end;
  }
  TABLE *ptr[3];				// Lock tables for quick update
  ptr[0]= tables[0].table;
  ptr[1]= tables[1].table;
  ptr[2]= tables[2].table;
  if (!(lock=mysql_lock_tables(thd,ptr,3)))
  {
    sql_print_error("Fatal error: Can't lock privilege tables: %s",
		    thd->net.last_error);
    goto end;
  }
  init_sql_alloc(&mem, ACL_ALLOC_BLOCK_SIZE, 0);
  init_read_record(&read_record_info,thd,table= tables[0].table,NULL,1,0);
  VOID(my_init_dynamic_array(&acl_hosts,sizeof(ACL_HOST),20,50));
  while (!(read_record_info.read_record(&read_record_info)))
  {
    ACL_HOST host;
    update_hostname(&host.host,get_field(&mem, table,0));
    host.db=	 get_field(&mem, table,1);
    host.access= get_access(table,2);
    host.access= fix_rights_for_db(host.access);
    host.sort=	 get_sort(2,host.host.hostname,host.db);
#ifndef TO_BE_REMOVED
    if (table->fields ==  8)
    {						// Without grant
      if (host.access & CREATE_ACL)
	host.access|=REFERENCES_ACL | INDEX_ACL | ALTER_ACL | CREATE_TMP_ACL;
    }
#endif
    VOID(push_dynamic(&acl_hosts,(gptr) &host));
  }
  qsort((gptr) dynamic_element(&acl_hosts,0,ACL_HOST*),acl_hosts.elements,
	sizeof(ACL_HOST),(qsort_cmp) acl_compare);
  end_read_record(&read_record_info);
  freeze_size(&acl_hosts);

  init_read_record(&read_record_info,thd,table=tables[1].table,NULL,1,0);
  VOID(my_init_dynamic_array(&acl_users,sizeof(ACL_USER),50,100));
  if (table->field[2]->field_length == 8 &&
      protocol_version == PROTOCOL_VERSION)
  {
    sql_print_error("Old 'user' table. (Check README or the Reference manual). Continuing --old-protocol"); /* purecov: tested */
    protocol_version=9; /* purecov: tested */
  }

  DBUG_PRINT("info",("user table fields: %d",table->fields));
  allow_all_hosts=0;
  while (!(read_record_info.read_record(&read_record_info)))
  {
    ACL_USER user;
    uint length=0;
    update_hostname(&user.host,get_field(&mem, table,0));
    user.user=get_field(&mem, table,1);
    user.password=get_field(&mem, table,2);
    if (user.password && (length=(uint) strlen(user.password)) == 8 &&
	protocol_version == PROTOCOL_VERSION)
    {
      sql_print_error(
		      "Found old style password for user '%s'. Ignoring user. (You may want to restart mysqld using --old-protocol)",
		      user.user ? user.user : ""); /* purecov: tested */
    }
    else if (length % 8 || length > 16)
    {
      sql_print_error(
		      "Found invalid password for user: '%s'@'%s'; Ignoring user",
		      user.user ? user.user : "",
		      user.host.hostname ? user.host.hostname : ""); /* purecov: tested */
      continue;					/* purecov: tested */
    }
    get_salt_from_password(user.salt,user.password);
    user.access=get_access(table,3) & GLOBAL_ACLS;
    user.sort=get_sort(2,user.host.hostname,user.user);
    user.hostname_length= (user.host.hostname ?
			   (uint) strlen(user.host.hostname) : 0);
    if (table->fields >= 31)	 /* Starting from 4.0.2 we have more fields */
    {
      char *ssl_type=get_field(&mem, table, 24);
      if (!ssl_type)
	user.ssl_type=SSL_TYPE_NONE;
      else if (!strcmp(ssl_type, "ANY"))
	user.ssl_type=SSL_TYPE_ANY;
      else if (!strcmp(ssl_type, "X509"))
	user.ssl_type=SSL_TYPE_X509;
      else  /* !strcmp(ssl_type, "SPECIFIED") */
	user.ssl_type=SSL_TYPE_SPECIFIED;

      user.ssl_cipher=	 get_field(&mem, table, 25);
      user.x509_issuer=  get_field(&mem, table, 26);
      user.x509_subject= get_field(&mem, table, 27);

      char *ptr = get_field(&mem, table, 28);
      user.user_resource.questions=atoi(ptr);
      ptr = get_field(&mem, table, 29);
      user.user_resource.updates=atoi(ptr);
      ptr = get_field(&mem, table, 30);
      user.user_resource.connections=atoi(ptr);
      if (user.user_resource.questions || user.user_resource.updates ||
	  user.user_resource.connections)
	mqh_used=1;
    }
    else
    {
      user.ssl_type=SSL_TYPE_NONE;
      bzero((char *)&(user.user_resource),sizeof(user.user_resource));
#ifndef TO_BE_REMOVED
      if (table->fields <= 13)
      {						// Without grant
	if (user.access & CREATE_ACL)
	  user.access|=REFERENCES_ACL | INDEX_ACL | ALTER_ACL;
      }
      /* Convert old privileges */
      user.access|= LOCK_TABLES_ACL | CREATE_TMP_ACL | SHOW_DB_ACL;
      if (user.access & FILE_ACL)
	user.access|= REPL_CLIENT_ACL | REPL_SLAVE_ACL;
      if (user.access & PROCESS_ACL)
	user.access|= SUPER_ACL | EXECUTE_ACL;
#endif
    }
    VOID(push_dynamic(&acl_users,(gptr) &user));
    if (!user.host.hostname || user.host.hostname[0] == wild_many &&
	!user.host.hostname[1])
      allow_all_hosts=1;			// Anyone can connect
  }
  qsort((gptr) dynamic_element(&acl_users,0,ACL_USER*),acl_users.elements,
	sizeof(ACL_USER),(qsort_cmp) acl_compare);
  end_read_record(&read_record_info);
  freeze_size(&acl_users);

  init_read_record(&read_record_info,thd,table=tables[2].table,NULL,1,0);
  VOID(my_init_dynamic_array(&acl_dbs,sizeof(ACL_DB),50,100));
  while (!(read_record_info.read_record(&read_record_info)))
  {
    ACL_DB db;
    update_hostname(&db.host,get_field(&mem, table,0));
    db.db=get_field(&mem, table,1);
    if (!db.db)
    {
      sql_print_error("Found an entry in the 'db' table with empty database name; Skipped");
      continue;
    }
    db.user=get_field(&mem, table,2);
    db.access=get_access(table,3);
    db.access=fix_rights_for_db(db.access);
    db.sort=get_sort(3,db.host.hostname,db.db,db.user);
#ifndef TO_BE_REMOVED
    if (table->fields <=  9)
    {						// Without grant
      if (db.access & CREATE_ACL)
	db.access|=REFERENCES_ACL | INDEX_ACL | ALTER_ACL;
    }
#endif
    VOID(push_dynamic(&acl_dbs,(gptr) &db));
  }
  qsort((gptr) dynamic_element(&acl_dbs,0,ACL_DB*),acl_dbs.elements,
	sizeof(ACL_DB),(qsort_cmp) acl_compare);
  end_read_record(&read_record_info);
  freeze_size(&acl_dbs);
  init_check_host();

  mysql_unlock_tables(thd, lock);
  initialized=1;
  thd->version--;				// Force close to free memory
  return_val=0;

end:
  close_thread_tables(thd);
  delete thd;
  if (org_thd)
    org_thd->store_globals();			/* purecov: inspected */
  else
  {
    /* Remember that we don't have a THD */
    my_pthread_setspecific_ptr(THR_THD,  0);
  }
  DBUG_RETURN(return_val);
}


void acl_free(bool end)
{
  free_root(&mem,MYF(0));
  delete_dynamic(&acl_hosts);
  delete_dynamic(&acl_users);
  delete_dynamic(&acl_dbs);
  delete_dynamic(&acl_wild_hosts);
  hash_free(&acl_check_hosts);
  if (!end)
    acl_cache->clear(1); /* purecov: inspected */
  else
  {
    delete acl_cache;
    acl_cache=0;
  }
}


/*
  Forget current privileges and read new privileges from the privilege tables

  SYNOPSIS
    acl_reload()
    thd			Thread handle
*/

void acl_reload(THD *thd)
{
  DYNAMIC_ARRAY old_acl_hosts,old_acl_users,old_acl_dbs;
  MEM_ROOT old_mem;
  bool old_initialized;
  DBUG_ENTER("acl_reload");

  if (thd && thd->locked_tables)
  {					// Can't have locked tables here
    thd->lock=thd->locked_tables;
    thd->locked_tables=0;
    close_thread_tables(thd);
  }
  if ((old_initialized=initialized))
    VOID(pthread_mutex_lock(&acl_cache->lock));

  old_acl_hosts=acl_hosts;
  old_acl_users=acl_users;
  old_acl_dbs=acl_dbs;
  old_mem=mem;
  delete_dynamic(&acl_wild_hosts);
  hash_free(&acl_check_hosts);

  if (acl_init(thd, 0))
  {					// Error. Revert to old list
    acl_free();				/* purecov: inspected */
    acl_hosts=old_acl_hosts;
    acl_users=old_acl_users;
    acl_dbs=old_acl_dbs;
    mem=old_mem;
    init_check_host();
  }
  else
  {
    free_root(&old_mem,MYF(0));
    delete_dynamic(&old_acl_hosts);
    delete_dynamic(&old_acl_users);
    delete_dynamic(&old_acl_dbs);
  }
  if (old_initialized)
    VOID(pthread_mutex_unlock(&acl_cache->lock));
  DBUG_VOID_RETURN;
}


/*
  Get all access bits from table after fieldnr
  We know that the access privileges ends when there is no more fields
  or the field is not an enum with two elements.
*/

static ulong get_access(TABLE *form, uint fieldnr)
{
  ulong access_bits=0,bit;
  char buff[2];
  String res(buff,sizeof(buff));
  Field **pos;

  for (pos=form->field+fieldnr, bit=1;
       *pos && (*pos)->real_type() == FIELD_TYPE_ENUM &&
	 ((Field_enum*) (*pos))->typelib->count == 2 ;
       pos++ , bit<<=1)
  {
    (*pos)->val_str(&res,&res);
    if (toupper(res[0]) == 'Y')
      access_bits|= bit;
  }
  return access_bits;
}


/*
  Return a number which, if sorted 'desc', puts strings in this order:
    no wildcards
    wildcards
    empty string
*/

static ulong get_sort(uint count,...)
{
  va_list args;
  va_start(args,count);
  ulong sort=0;

  while (count--)
  {
    char *str=va_arg(args,char*);
    uint chars=0,wild=0;

    if (str)
    {
      for (; *str ; str++)
      {
	if (*str == wild_many || *str == wild_one || *str == wild_prefix)
	  wild++;
	else
	  chars++;
      }
    }
    sort= (sort << 8) + (wild ? 1 : chars ? 2 : 0);
  }
  va_end(args);
  return sort;
}


static int acl_compare(ACL_ACCESS *a,ACL_ACCESS *b)
{
  if (a->sort > b->sort)
    return -1;
  if (a->sort < b->sort)
    return 1;
  return 0;
}


/*
  Get master privilges for user (priviliges for all tables).
  Required before connecting to MySQL
*/

ulong acl_getroot(THD *thd, const char *host, const char *ip, const char *user,
		  const char *password,const char *message,
		  char **priv_user, char *priv_host,
		  bool old_ver, USER_RESOURCES	*mqh)
{
  ulong user_access=NO_ACCESS;
  *priv_user=(char*) user;
  DBUG_ENTER("acl_getroot");

  bzero((char *)mqh,sizeof(USER_RESOURCES));
  if (!initialized)
  {
    // If no data allow anything
    DBUG_RETURN((ulong) ~NO_ACCESS);		/* purecov: tested */
  }
  VOID(pthread_mutex_lock(&acl_cache->lock));

  /*
    Get possible access from user_list. This is or'ed to others not
    fully specified
  */
  for (uint i=0 ; i < acl_users.elements ; i++)
  {
    ACL_USER *acl_user=dynamic_element(&acl_users,i,ACL_USER*);
    if (!acl_user->user || !strcmp(user,acl_user->user))
    {
      if (compare_hostname(&acl_user->host,host,ip))
      {
	if (!acl_user->password && !*password ||
	    (acl_user->password && *password &&
	     !check_scramble(password,message,acl_user->salt,
			     (my_bool) old_ver)))
	{
	  Vio *vio=thd->net.vio;
#ifdef HAVE_OPENSSL
	  SSL *ssl= (SSL*) vio->ssl_arg;
#endif
	  /*
	    In this point we know that user is allowed to connect
	    from given host by given username/password pair. Now
	    we check if SSL is required, if user is using SSL and
	    if X509 certificate attributes are OK
	  */
	  switch (acl_user->ssl_type) {
	  case SSL_TYPE_NOT_SPECIFIED:		// Impossible
	  case SSL_TYPE_NONE: /* SSL is not required to connect */
	    user_access=acl_user->access;
	    break;
#ifdef HAVE_OPENSSL
	  case SSL_TYPE_ANY: /* Any kind of SSL is good enough */
	    if (vio_type(vio) == VIO_TYPE_SSL)
	      user_access=acl_user->access;
	    break;
	  case SSL_TYPE_X509: /* Client should have any valid certificate. */
	    /*
	      We need to check for absence of SSL because without SSL
	      we should reject connection.
	    */
	    if (vio_type(vio) == VIO_TYPE_SSL && 
	        SSL_get_verify_result(ssl) == X509_V_OK &&
	        SSL_get_peer_certificate(ssl))
	      user_access=acl_user->access;
	    break;
	  case SSL_TYPE_SPECIFIED: /* Client should have specified attrib */
	    /*
	      We need to check for absence of SSL because without SSL
	      we should reject connection.
	    */
	    if (vio_type(vio) == VIO_TYPE_SSL && 
	        SSL_get_verify_result(ssl) == X509_V_OK)
	    {
	      if (acl_user->ssl_cipher)
	      {
		DBUG_PRINT("info",("comparing ciphers: '%s' and '%s'",
				   acl_user->ssl_cipher,
				   SSL_get_cipher(ssl)));
		if (!strcmp(acl_user->ssl_cipher,SSL_get_cipher(ssl)))
		  user_access=acl_user->access;
		else
		{
		  if (global_system_variables.log_warnings)
		    sql_print_error("X509 ciphers mismatch: should be '%s' but is '%s'",
				    acl_user->ssl_cipher,
				    SSL_get_cipher(ssl));
		  user_access=NO_ACCESS;
		  break;
		}
	      }
	      /* Prepare certificate (if exists) */
	      DBUG_PRINT("info",("checkpoint 1"));
	      X509* cert=SSL_get_peer_certificate(ssl);
	      DBUG_PRINT("info",("checkpoint 2"));
	      /* If X509 issuer is speified, we check it... */
	      if (acl_user->x509_issuer)
	      {
		DBUG_PRINT("info",("checkpoint 3"));
		char *ptr = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		DBUG_PRINT("info",("comparing issuers: '%s' and '%s'",
				   acl_user->x509_issuer, ptr));
		if (strcmp(acl_user->x509_issuer, ptr))
		{
		  if (global_system_variables.log_warnings)
		    sql_print_error("X509 issuer mismatch: should be '%s' but is '%s'",
				    acl_user->x509_issuer, ptr);
		  user_access=NO_ACCESS;
		  free(ptr);
		  break;
		}
		user_access=acl_user->access;
		free(ptr);
	      }
	      DBUG_PRINT("info",("checkpoint 4"));
	      /* X509 subject is specified, we check it .. */
	      if (acl_user->x509_subject)
	      {
		char *ptr= X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		DBUG_PRINT("info",("comparing subjects: '%s' and '%s'",
				   acl_user->x509_subject, ptr));
		if (strcmp(acl_user->x509_subject,ptr))
		{
		  if (global_system_variables.log_warnings)
		    sql_print_error("X509 subject mismatch: '%s' vs '%s'",
				    acl_user->x509_subject, ptr);
		  user_access=NO_ACCESS;
		}
		else
		  user_access=acl_user->access;
		free(ptr);
	      }
	      break;
	    }
#else  /* HAVE_OPENSSL */
          default:
            /*
                If we don't have SSL but SSL is required for this user the 
                authentication should fail.
            */
            break;
#endif /* HAVE_OPENSSL */
	  }
          
	  *mqh=acl_user->user_resource;
	  if (!acl_user->user)
	    *priv_user=(char*) "";	// Change to anonymous user /* purecov: inspected */
	  if (acl_user->host.hostname)
	    strmake(priv_host, acl_user->host.hostname, MAX_HOSTNAME);
	  else
	    *priv_host= 0;
	  break;
	}
#ifndef ALLOW_DOWNGRADE_OF_USERS
	break;				// Wrong password breaks loop /* purecov: inspected */
#endif
      }
    }
  }
  VOID(pthread_mutex_unlock(&acl_cache->lock));
  DBUG_RETURN(user_access);
}


/*
** Functions to add and change user and database privileges when one
** changes things with GRANT
*/

static byte* check_get_key(ACL_USER *buff,uint *length,
			   my_bool not_used __attribute__((unused)))
{
  *length=buff->hostname_length;
  return (byte*) buff->host.hostname;
}

static void acl_update_user(const char *user, const char *host,
			    const char *password,
			    enum SSL_type ssl_type,
			    const char *ssl_cipher,
			    const char *x509_issuer,
			    const char *x509_subject,
			    USER_RESOURCES  *mqh,
			    ulong privileges)
{
  for (uint i=0 ; i < acl_users.elements ; i++)
  {
    ACL_USER *acl_user=dynamic_element(&acl_users,i,ACL_USER*);
    if (!acl_user->user && !user[0] ||
	acl_user->user &&
	!strcmp(user,acl_user->user))
    {
      if (!acl_user->host.hostname && !host[0] ||
	  acl_user->host.hostname &&
	  !my_strcasecmp(host,acl_user->host.hostname))
      {
	acl_user->access=privileges;
	if (mqh->bits & 1)
	  acl_user->user_resource.questions=mqh->questions;
	if (mqh->bits & 2)
	  acl_user->user_resource.updates=mqh->updates;
	if (mqh->bits & 4)
	  acl_user->user_resource.connections=mqh->connections;
	if (ssl_type != SSL_TYPE_NOT_SPECIFIED)
	{
	  acl_user->ssl_type= ssl_type;
	  acl_user->ssl_cipher= (ssl_cipher ? strdup_root(&mem,ssl_cipher) :
				 0);
	  acl_user->x509_issuer= (x509_issuer ? strdup_root(&mem,x509_issuer) :
				  0);
	  acl_user->x509_subject= (x509_subject ?
				   strdup_root(&mem,x509_subject) : 0);
	}
	if (password)
	{
	  if (!password[0])
	    acl_user->password=0;
	  else
	  {
	    acl_user->password=(char*) "";	// Just point at something
	    get_salt_from_password(acl_user->salt,password);
	  }
	}
	break;
      }
    }
  }
}


static void acl_insert_user(const char *user, const char *host,
			    const char *password,
			    enum SSL_type ssl_type,
			    const char *ssl_cipher,
			    const char *x509_issuer,
			    const char *x509_subject,
			    USER_RESOURCES *mqh,
			    ulong privileges)
{
  ACL_USER acl_user;
  acl_user.user=*user ? strdup_root(&mem,user) : 0;
  update_hostname(&acl_user.host,strdup_root(&mem,host));
  acl_user.password=0;
  acl_user.access=privileges;
  acl_user.user_resource = *mqh;
  acl_user.sort=get_sort(2,acl_user.host.hostname,acl_user.user);
  acl_user.hostname_length=(uint) strlen(acl_user.host.hostname);
  acl_user.ssl_type= (ssl_type != SSL_TYPE_NOT_SPECIFIED ?
		      ssl_type : SSL_TYPE_NONE);
  acl_user.ssl_cipher=	ssl_cipher   ? strdup_root(&mem,ssl_cipher) : 0;
  acl_user.x509_issuer= x509_issuer  ? strdup_root(&mem,x509_issuer) : 0;
  acl_user.x509_subject=x509_subject ? strdup_root(&mem,x509_subject) : 0;
  if (password)
  {
    acl_user.password=(char*) "";		// Just point at something
    get_salt_from_password(acl_user.salt,password);
  }

  VOID(push_dynamic(&acl_users,(gptr) &acl_user));
  if (!acl_user.host.hostname || acl_user.host.hostname[0] == wild_many
      && !acl_user.host.hostname[1])
    allow_all_hosts=1;			// Anyone can connect /* purecov: tested */
  qsort((gptr) dynamic_element(&acl_users,0,ACL_USER*),acl_users.elements,
	sizeof(ACL_USER),(qsort_cmp) acl_compare);

  /* We must free acl_check_hosts as its memory is mapped to acl_user */
  delete_dynamic(&acl_wild_hosts);
  hash_free(&acl_check_hosts);
  init_check_host();
}


static void acl_update_db(const char *user, const char *host, const char *db,
			  ulong privileges)
{
  for (uint i=0 ; i < acl_dbs.elements ; i++)
  {
    ACL_DB *acl_db=dynamic_element(&acl_dbs,i,ACL_DB*);
    if (!acl_db->user && !user[0] ||
	acl_db->user &&
	!strcmp(user,acl_db->user))
    {
      if (!acl_db->host.hostname && !host[0] ||
	  acl_db->host.hostname && !my_strcasecmp(host,acl_db->host.hostname))
      {
	if (!acl_db->db && !db[0] ||
	    acl_db->db && !strcmp(db,acl_db->db))
	{
	  if (privileges)
	    acl_db->access=privileges;
	  else
	    delete_dynamic_element(&acl_dbs,i);
	}
      }
    }
  }
}


/*
  Insert a user/db/host combination into the global acl_cache

  SYNOPSIS
    acl_insert_db()
    user		User name
    host		Host name
    db			Database name
    privileges		Bitmap of privileges

  NOTES
    acl_cache->lock must be locked when calling this
*/

static void acl_insert_db(const char *user, const char *host, const char *db,
			  ulong privileges)
{
  ACL_DB acl_db;
  safe_mutex_assert_owner(&acl_cache->lock);
  acl_db.user=strdup_root(&mem,user);
  update_hostname(&acl_db.host,strdup_root(&mem,host));
  acl_db.db=strdup_root(&mem,db);
  acl_db.access=privileges;
  acl_db.sort=get_sort(3,acl_db.host.hostname,acl_db.db,acl_db.user);
  VOID(push_dynamic(&acl_dbs,(gptr) &acl_db));
  qsort((gptr) dynamic_element(&acl_dbs,0,ACL_DB*),acl_dbs.elements,
	sizeof(ACL_DB),(qsort_cmp) acl_compare);
}



/*
  Get privilege for a host, user and db combination
*/

ulong acl_get(const char *host, const char *ip, const char *bin_ip,
	     const char *user, const char *db)
{
  ulong host_access,db_access;
  uint i,key_length;
  db_access=0; host_access= ~0;
  char key[ACL_KEY_LENGTH],*tmp_db,*end;
  acl_entry *entry;

  VOID(pthread_mutex_lock(&acl_cache->lock));
  memcpy_fixed(&key,bin_ip,sizeof(struct in_addr));
  end=strmov((tmp_db=strmov(key+sizeof(struct in_addr),user)+1),db);
  if (lower_case_table_names)
  {
    casedn_str(tmp_db);
    db=tmp_db;
  }
  key_length=(uint) (end-key);
  if ((entry=(acl_entry*) acl_cache->search(key,key_length)))
  {
    db_access=entry->access;
    VOID(pthread_mutex_unlock(&acl_cache->lock));
    return db_access;
  }

  /*
    Check if there are some access rights for database and user
  */
  for (i=0 ; i < acl_dbs.elements ; i++)
  {
    ACL_DB *acl_db=dynamic_element(&acl_dbs,i,ACL_DB*);
    if (!acl_db->user || !strcmp(user,acl_db->user))
    {
      if (compare_hostname(&acl_db->host,host,ip))
      {
	if (!acl_db->db || !wild_compare(db,acl_db->db))
	{
	  db_access=acl_db->access;
	  if (acl_db->host.hostname)
	    goto exit;				// Fully specified. Take it
	  break; /* purecov: tested */
	}
      }
    }
  }
  if (!db_access)
    goto exit;					// Can't be better

  /*
    No host specified for user. Get hostdata from host table
  */
  host_access=0;				// Host must be found
  for (i=0 ; i < acl_hosts.elements ; i++)
  {
    ACL_HOST *acl_host=dynamic_element(&acl_hosts,i,ACL_HOST*);
    if (compare_hostname(&acl_host->host,host,ip))
    {
      if (!acl_host->db || !wild_compare(db,acl_host->db))
      {
	host_access=acl_host->access;		// Fully specified. Take it
	break;
      }
    }
  }
exit:
  /* Save entry in cache for quick retrieval */
  if ((entry= (acl_entry*) malloc(sizeof(acl_entry)+key_length)))
  {
    entry->access=(db_access & host_access);
    entry->length=key_length;
    memcpy((gptr) entry->key,key,key_length);
    acl_cache->add(entry);
  }
  VOID(pthread_mutex_unlock(&acl_cache->lock));
  return (db_access & host_access);
}


int wild_case_compare(const char *str,const char *wildstr)
{
  reg3 int flag;
  DBUG_ENTER("wild_case_compare");
  DBUG_PRINT("enter",("str: '%s'  wildstr: '%s'",str,wildstr));
  while (*wildstr)
  {
    while (*wildstr && *wildstr != wild_many && *wildstr != wild_one)
    {
      if (*wildstr == wild_prefix && wildstr[1])
	wildstr++;
      if (toupper(*wildstr++) != toupper(*str++)) DBUG_RETURN(1);
    }
    if (! *wildstr ) DBUG_RETURN (*str != 0);
    if (*wildstr++ == wild_one)
    {
      if (! *str++) DBUG_RETURN (1);	/* One char; skip */
    }
    else
    {						/* Found '*' */
      if (!*wildstr) DBUG_RETURN(0);		/* '*' as last char: OK */
      flag=(*wildstr != wild_many && *wildstr != wild_one);
      do
      {
	if (flag)
	{
	  char cmp;
	  if ((cmp= *wildstr) == wild_prefix && wildstr[1])
	    cmp=wildstr[1];
	  cmp=toupper(cmp);
	  while (*str && toupper(*str) != cmp)
	    str++;
	  if (!*str) DBUG_RETURN (1);
	}
	if (wild_case_compare(str,wildstr) == 0) DBUG_RETURN (0);
      } while (*str++);
      DBUG_RETURN(1);
    }
  }
  DBUG_RETURN (*str != '\0');
}


/*
  Check if there are any possible matching entries for this host

  NOTES
    All host names without wild cards are stored in a hash table,
    entries with wildcards are stored in a dynamic array
*/

static void init_check_host(void)
{
  DBUG_ENTER("init_check_host");
  VOID(my_init_dynamic_array(&acl_wild_hosts,sizeof(struct acl_host_and_ip),
			  acl_users.elements,1));
  VOID(hash_init(&acl_check_hosts,acl_users.elements,0,0,
		 (hash_get_key) check_get_key,0,HASH_CASE_INSENSITIVE));
  if (!allow_all_hosts)
  {
    for (uint i=0 ; i < acl_users.elements ; i++)
    {
      ACL_USER *acl_user=dynamic_element(&acl_users,i,ACL_USER*);
      if (strchr(acl_user->host.hostname,wild_many) ||
	  strchr(acl_user->host.hostname,wild_one) ||
	  acl_user->host.ip_mask)
      {						// Has wildcard
	uint j;
	for (j=0 ; j < acl_wild_hosts.elements ; j++)
	{					// Check if host already exists
	  acl_host_and_ip *acl=dynamic_element(&acl_wild_hosts,j,
					       acl_host_and_ip *);
	  if (!my_strcasecmp(acl_user->host.hostname,acl->hostname))
	    break;				// already stored
	}
	if (j == acl_wild_hosts.elements)	// If new
	  (void) push_dynamic(&acl_wild_hosts,(char*) &acl_user->host);
      }
      else if (!hash_search(&acl_check_hosts,(byte*) &acl_user->host,
			    (uint) strlen(acl_user->host.hostname)))
      {
	if (hash_insert(&acl_check_hosts,(byte*) acl_user))
	{					// End of memory
	  allow_all_hosts=1;			// Should never happen
	  DBUG_VOID_RETURN;
	}
      }
    }
  }
  freeze_size(&acl_wild_hosts);
  freeze_size(&acl_check_hosts.array);
  DBUG_VOID_RETURN;
}


/* Return true if there is no users that can match the given host */

bool acl_check_host(const char *host, const char *ip)
{
  if (allow_all_hosts)
    return 0;
  VOID(pthread_mutex_lock(&acl_cache->lock));

  if (host && hash_search(&acl_check_hosts,(byte*) host,(uint) strlen(host)) ||
      ip && hash_search(&acl_check_hosts,(byte*) ip,(uint) strlen(ip)))
  {
    VOID(pthread_mutex_unlock(&acl_cache->lock));
    return 0;					// Found host
  }
  for (uint i=0 ; i < acl_wild_hosts.elements ; i++)
  {
    acl_host_and_ip *acl=dynamic_element(&acl_wild_hosts,i,acl_host_and_ip*);
    if (compare_hostname(acl, host, ip))
    {
      VOID(pthread_mutex_unlock(&acl_cache->lock));
      return 0;					// Host ok
    }
  }
  VOID(pthread_mutex_unlock(&acl_cache->lock));
  return 1;					// Host is not allowed
}


/*
  Check if the user is allowed to change password

  SYNOPSIS:
    check_change_password()
    thd		THD
    host	hostname for the user
    user	user name

    RETURN VALUE
      0		OK
      1		ERROR  ; In this case the error is sent to the client.
*/

bool check_change_password(THD *thd, const char *host, const char *user)
{
  if (!initialized)
  {
    send_error(&thd->net, ER_PASSWORD_NOT_ALLOWED); /* purecov: inspected */
    return(1); /* purecov: inspected */
  }
  if (!thd->slave_thread &&
      (strcmp(thd->user,user) ||
       my_strcasecmp(host,thd->host_or_ip)))
  {
    if (check_access(thd, UPDATE_ACL, "mysql",0,1))
      return(1);
  }
  if (!thd->slave_thread && !thd->user[0])
  {
    send_error(&thd->net, ER_PASSWORD_ANONYMOUS_USER);
    return(1);
  }
  return(0);
}


/*
  Change a password for a user

  SYNOPSIS
    change_password()
    thd			Thread handle
    host		Hostname
    user		User name
    new_password	New password for host@user

  RETURN VALUES
    0	ok
    1	ERROR; In this case the error is sent to the client.
*/

bool change_password(THD *thd, const char *host, const char *user,
		     char *new_password)
{
  uint length=0;
  DBUG_ENTER("change_password");
  DBUG_PRINT("enter",("host: '%s'  user: '%s'  new_password: '%s'",
		      host,user,new_password));
  DBUG_ASSERT(host != 0);			// Ensured by parent

  length=(uint) strlen(new_password);
  new_password[length & 16]=0;

  VOID(pthread_mutex_lock(&acl_cache->lock));
  ACL_USER *acl_user;
  if (!(acl_user= find_acl_user(host,user)))
  {
    send_error(&thd->net, ER_PASSWORD_NO_MATCH);
    VOID(pthread_mutex_unlock(&acl_cache->lock));
    DBUG_RETURN(1);
  }
  if (update_user_table(thd,
			acl_user->host.hostname ? acl_user->host.hostname : "",
			acl_user->user ? acl_user->user : "",
			new_password))
  {
    VOID(pthread_mutex_unlock(&acl_cache->lock)); /* purecov: deadcode */
    send_error(&thd->net,0); /* purecov: deadcode */
    DBUG_RETURN(1); /* purecov: deadcode */
  }
  get_salt_from_password(acl_user->salt,new_password);
  if (!new_password[0])
    acl_user->password=0;
  else
    acl_user->password=(char*) "";		// Point at something
  acl_cache->clear(1);				// Clear locked hostname cache
  VOID(pthread_mutex_unlock(&acl_cache->lock));

  char buff[460];
  ulong query_length=
    my_sprintf(buff,
	       (buff,"SET PASSWORD FOR \"%-.120s\"@\"%-.120s\"=\"%-.120s\"",
		acl_user->user ? acl_user->user : "",
		acl_user->host.hostname ? acl_user->host.hostname : "",
		new_password));
  mysql_update_log.write(thd, buff, query_length);
  Query_log_event qinfo(thd, buff, query_length, 0);
  mysql_bin_log.write(&qinfo);
  DBUG_RETURN(0);
}


/*
  Find first entry that matches the current user
*/

static ACL_USER *
find_acl_user(const char *host, const char *user)
{
  DBUG_ENTER("find_acl_user");
  DBUG_PRINT("enter",("host: '%s'  user: '%s'",host,user));
  for (uint i=0 ; i < acl_users.elements ; i++)
  {
    ACL_USER *acl_user=dynamic_element(&acl_users,i,ACL_USER*);
    DBUG_PRINT("info",("strcmp('%s','%s'), compare_hostname('%s','%s'),",
		       user,
		       acl_user->user ? acl_user->user : "",
		       host,
		       acl_user->host.hostname ? acl_user->host.hostname :
		       ""));
    if (!acl_user->user && !user[0] ||
	acl_user->user && !strcmp(user,acl_user->user))
    {
      if (compare_hostname(&(acl_user->host),host,host))
      {
	DBUG_RETURN(acl_user);
      }
    }
  }
  DBUG_RETURN(0);
}


/*
  Comparing of hostnames

  NOTES
  A hostname may be of type:
  hostname   (May include wildcards);   monty.pp.sci.fi
  ip	   (May include wildcards);   192.168.0.0
  ip/netmask			      192.168.0.0/255.255.255.0

  A net mask of 0.0.0.0 is not allowed.
*/

static const char *calc_ip(const char *ip, long *val, char end)
{
  long ip_val,tmp;
  if (!(ip=str2int(ip,10,0,255,&ip_val)) || *ip != '.')
    return 0;
  ip_val<<=24;
  if (!(ip=str2int(ip+1,10,0,255,&tmp)) || *ip != '.')
    return 0;
  ip_val+=tmp<<16;
  if (!(ip=str2int(ip+1,10,0,255,&tmp)) || *ip != '.')
    return 0;
  ip_val+=tmp<<8;
  if (!(ip=str2int(ip+1,10,0,255,&tmp)) || *ip != end)
    return 0;
  *val=ip_val+tmp;
  return ip;
}


static void update_hostname(acl_host_and_ip *host, const char *hostname)
{
  host->hostname=(char*) hostname;		// This will not be modified!
  if (hostname &&
      (!(hostname=calc_ip(hostname,&host->ip,'/')) ||
       !(hostname=calc_ip(hostname+1,&host->ip_mask,'\0'))))
  {
    host->ip=host->ip_mask=0;			// Not a masked ip
  }
}


static bool compare_hostname(const acl_host_and_ip *host, const char *hostname,
			     const char *ip)
{
  long tmp;
  if (host->ip_mask && ip && calc_ip(ip,&tmp,'\0'))
  {
    return (tmp & host->ip_mask) == host->ip;
  }
  return (!host->hostname ||
	  (hostname && !wild_case_compare(hostname,host->hostname)) ||
	  (ip && !wild_compare(ip,host->hostname)));
}


/*
  Update grants in the user and database privilege tables
*/

static bool update_user_table(THD *thd, const char *host, const char *user,
			      const char *new_password)
{
  TABLE_LIST tables;
  TABLE *table;
  bool error=1;
  DBUG_ENTER("update_user_table");
  DBUG_PRINT("enter",("user: %s  host: %s",user,host));

  bzero((char*) &tables,sizeof(tables));
  tables.alias=tables.real_name=(char*) "user";
  tables.db=(char*) "mysql";
#ifdef HAVE_REPLICATION
  /*
    GRANT and REVOKE are applied the slave in/exclusion rules as they are
    some kind of updates to the mysql.% tables.
  */
  if (thd->slave_thread && table_rules_on)
  {
    /* 
       The tables must be marked "updating" so that tables_ok() takes them into
       account in tests.  It's ok to leave 'updating' set after tables_ok.
    */
    tables.updating= 1;
    /* Thanks to bzero, tables.next==0 */
    if (!tables_ok(0, &tables))
      DBUG_RETURN(0);
  }
#endif

  if (!(table=open_ltable(thd,&tables,TL_WRITE)))
    DBUG_RETURN(1); /* purecov: deadcode */
  table->field[0]->store(host,(uint) strlen(host));
  table->field[1]->store(user,(uint) strlen(user));

  if (table->file->index_read_idx(table->record[0],0,
				  (byte*) table->field[0]->ptr,0,
				  HA_READ_KEY_EXACT))
  {
    my_error(ER_PASSWORD_NO_MATCH,MYF(0));	/* purecov: deadcode */
    DBUG_RETURN(1);				/* purecov: deadcode */
  }
  store_record(table,1);
  table->field[2]->store(new_password,(uint) strlen(new_password));
  if ((error=table->file->update_row(table->record[1],table->record[0])))
  {
    table->file->print_error(error,MYF(0));	/* purecov: deadcode */
    goto end;					/* purecov: deadcode */
  }
  error=0;					// Record updated

end:
  close_thread_tables(thd);
  DBUG_RETURN(error);
}


/* Return 1 if we are allowed to create new users */

static bool test_if_create_new_users(THD *thd)
{
  bool create_new_users=1;    // Assume that we are allowed to create new users
  if (opt_safe_user_create && !(thd->master_access & INSERT_ACL))
  {
    TABLE_LIST tl;
    ulong db_access;
    bzero((char*) &tl,sizeof(tl));
    tl.db=	   (char*) "mysql";
    tl.real_name=  (char*) "user";
    db_access=acl_get(thd->host, thd->ip, (char*) &thd->remote.sin_addr,
		      thd->priv_user, tl.db);
    if (!(db_access & INSERT_ACL))
    {
      if (check_grant(thd,INSERT_ACL,&tl,0,1))
	create_new_users=0;
    }
  }
  return create_new_users;
}


/****************************************************************************
  Handle GRANT commands
****************************************************************************/

static int replace_user_table(THD *thd, TABLE *table, const LEX_USER &combo,
			      ulong rights, bool revoke_grant,
			      bool create_user)
{
  int error = -1;
  bool old_row_exists=0;
  char *password,empty_string[1];
  char what= (revoke_grant) ? 'N' : 'Y';
  DBUG_ENTER("replace_user_table");
  safe_mutex_assert_owner(&acl_cache->lock);

  password=empty_string;
  empty_string[0]=0;

  if (combo.password.str && combo.password.str[0])
  {
    if (combo.password.length != HASH_PASSWORD_LENGTH)
    {
      my_printf_error(ER_PASSWORD_NO_MATCH,
		      "Password hash should be a %d-digit hexadecimal number",
		      MYF(0),HASH_PASSWORD_LENGTH);
      DBUG_RETURN(-1);
    }
    password=combo.password.str;
  }

  table->field[0]->store(combo.host.str,combo.host.length);
  table->field[1]->store(combo.user.str,combo.user.length);
  table->file->index_init(0);
  if (table->file->index_read(table->record[0],
			      (byte*) table->field[0]->ptr,0,
			      HA_READ_KEY_EXACT))
  {
    if (!create_user)
    {
      if (what == 'N')
	my_printf_error(ER_NONEXISTING_GRANT,ER(ER_NONEXISTING_GRANT),
			MYF(0),combo.user.str,combo.host.str);
      else
	my_printf_error(ER_NO_PERMISSION_TO_CREATE_USER,
			ER(ER_NO_PERMISSION_TO_CREATE_USER),
			MYF(0),thd->user,
			thd->host_or_ip);
      error= -1;
      goto end;
    }
    old_row_exists = 0;
    restore_record(table,2);			// cp empty row from record[2]
    table->field[0]->store(combo.host.str,combo.host.length);
    table->field[1]->store(combo.user.str,combo.user.length);
    table->field[2]->store(password,(uint) strlen(password));
  }
  else
  {
    old_row_exists = 1;
    store_record(table,1);			// Save copy for update
    if (combo.password.str)			// If password given
      table->field[2]->store(password,(uint) strlen(password));
  }

  /* Update table columns with new privileges */

  Field **tmp_field;
  ulong priv;
  for (tmp_field= table->field+3, priv = SELECT_ACL;
       *tmp_field && (*tmp_field)->real_type() == FIELD_TYPE_ENUM &&
	 ((Field_enum*) (*tmp_field))->typelib->count == 2 ;
       tmp_field++, priv <<= 1)
  {
    if (priv & rights)				 // set requested privileges
      (*tmp_field)->store(&what,1);
  }
  rights=get_access(table,3);
  DBUG_PRINT("info",("table->fields: %d",table->fields));
  if (table->fields >= 31)		/* From 4.0.0 we have more fields */
  {
    /* We write down SSL related ACL stuff */
    switch (thd->lex.ssl_type) {
    case SSL_TYPE_ANY:
      table->field[24]->store("ANY",3);
      table->field[25]->store("",0);
      table->field[26]->store("",0);
      table->field[27]->store("",0);
      break;
    case SSL_TYPE_X509:
      table->field[24]->store("X509",4);
      table->field[25]->store("",0);
      table->field[26]->store("",0);
      table->field[27]->store("",0);
      break;
    case SSL_TYPE_SPECIFIED:
      table->field[24]->store("SPECIFIED",9);
      table->field[25]->store("",0);
      table->field[26]->store("",0);
      table->field[27]->store("",0);
      if (thd->lex.ssl_cipher)
	table->field[25]->store(thd->lex.ssl_cipher,
				strlen(thd->lex.ssl_cipher));
      if (thd->lex.x509_issuer)
	table->field[26]->store(thd->lex.x509_issuer,
				strlen(thd->lex.x509_issuer));
      if (thd->lex.x509_subject)
	table->field[27]->store(thd->lex.x509_subject,
				strlen(thd->lex.x509_subject));
      break;
    case SSL_TYPE_NOT_SPECIFIED:
      break;
    case SSL_TYPE_NONE:
      table->field[24]->store("",0);
      table->field[25]->store("",0);
      table->field[26]->store("",0);
      table->field[27]->store("",0);
      break;
    }

    USER_RESOURCES mqh = thd->lex.mqh;
    if (mqh.bits & 1)
      table->field[28]->store((longlong) mqh.questions);
    if (mqh.bits & 2)
      table->field[29]->store((longlong) mqh.updates);
    if (mqh.bits & 4)
      table->field[30]->store((longlong) mqh.connections);
    mqh_used = mqh_used || mqh.questions || mqh.updates || mqh.connections;
  }
  if (old_row_exists)
  {
    /*
      We should NEVER delete from the user table, as a uses can still
      use mysqld even if he doesn't have any privileges in the user table!
    */
    if (cmp_record(table,1) &&
	(error=table->file->update_row(table->record[1],table->record[0])))
    {						// This should never happen
      table->file->print_error(error,MYF(0));	/* purecov: deadcode */
      error= -1;				/* purecov: deadcode */
      goto end;					/* purecov: deadcode */
    }
  }
  else if ((error=table->file->write_row(table->record[0]))) // insert
  {						// This should never happen
    if (error && error != HA_ERR_FOUND_DUPP_KEY &&
	error != HA_ERR_FOUND_DUPP_UNIQUE)	/* purecov: inspected */
    {
      table->file->print_error(error,MYF(0));	/* purecov: deadcode */
      error= -1;				/* purecov: deadcode */
      goto end;					/* purecov: deadcode */
    }
  }
  error=0;					// Privileges granted / revoked

end:
  if (!error)
  {
    acl_cache->clear(1);			// Clear privilege cache
    if (!combo.password.str)
      password=0;				// No password given on command
    if (old_row_exists)
      acl_update_user(combo.user.str,combo.host.str,password,
		      thd->lex.ssl_type,
		      thd->lex.ssl_cipher,
		      thd->lex.x509_issuer,
		      thd->lex.x509_subject,
		      &thd->lex.mqh,
		      rights);
    else
      acl_insert_user(combo.user.str,combo.host.str,password,
		      thd->lex.ssl_type,
		      thd->lex.ssl_cipher,
		      thd->lex.x509_issuer,
		      thd->lex.x509_subject,
		      &thd->lex.mqh,
		      rights);
  }
  table->file->index_end();
  DBUG_RETURN(error);
}


/*
  change grants in the mysql.db table
*/

static int replace_db_table(TABLE *table, const char *db,
			    const LEX_USER &combo,
			    ulong rights, bool revoke_grant)
{
  uint i;
  ulong priv,store_rights;
  bool old_row_exists=0;
  int error;
  char what= (revoke_grant) ? 'N' : 'Y';
  DBUG_ENTER("replace_db_table");

  /* Check if there is such a user in user table in memory? */
  if (!initialized || !find_acl_user(combo.host.str,combo.user.str))
  {
    my_error(ER_PASSWORD_NO_MATCH,MYF(0));
    DBUG_RETURN(-1);
  }

  table->field[0]->store(combo.host.str,combo.host.length);
  table->field[1]->store(db,(uint) strlen(db));
  table->field[2]->store(combo.user.str,combo.user.length);
  table->file->index_init(0);
  if (table->file->index_read(table->record[0],(byte*) table->field[0]->ptr,0,
			      HA_READ_KEY_EXACT))
  {
    if (what == 'N')
    { // no row, no revoke
      my_printf_error(ER_NONEXISTING_GRANT,ER(ER_NONEXISTING_GRANT),MYF(0),
		      combo.user.str,combo.host.str);
      goto abort;
    }
    old_row_exists = 0;
    restore_record(table,2);			// cp empty row from record[2]
    table->field[0]->store(combo.host.str,combo.host.length);
    table->field[1]->store(db,(uint) strlen(db));
    table->field[2]->store(combo.user.str,combo.user.length);
  }
  else
  {
    old_row_exists = 1;
    store_record(table,1);
  }

  store_rights=get_rights_for_db(rights);
  for (i= 3, priv= 1; i < table->fields; i++, priv <<= 1)
  {
    if (priv & store_rights)			// do it if priv is chosen
      table->field [i]->store(&what,1);		// set requested privileges
  }
  rights=get_access(table,3);
  rights=fix_rights_for_db(rights);

  if (old_row_exists)
  {
    /* update old existing row */
    if (rights)
    {
      if ((error=table->file->update_row(table->record[1],table->record[0])))
	goto table_error;			/* purecov: deadcode */
    }
    else	/* must have been a revoke of all privileges */
    {
      if ((error = table->file->delete_row(table->record[1])))
	goto table_error;			/* purecov: deadcode */
    }
  }
  else if ((error=table->file->write_row(table->record[0])))
  {
    if (error && error != HA_ERR_FOUND_DUPP_KEY) /* purecov: inspected */
      goto table_error; /* purecov: deadcode */
  }

  acl_cache->clear(1);				// Clear privilege cache
  if (old_row_exists)
    acl_update_db(combo.user.str,combo.host.str,db,rights);
  else
    acl_insert_db(combo.user.str,combo.host.str,db,rights);
  table->file->index_end();
  DBUG_RETURN(0);

  /* This could only happen if the grant tables got corrupted */
table_error:
  table->file->print_error(error,MYF(0));	/* purecov: deadcode */
  table->file->index_end();

abort:
  DBUG_RETURN(-1);
}


class GRANT_COLUMN :public Sql_alloc
{
public:
  char *column;
  ulong rights;
  uint key_length;
  GRANT_COLUMN(String &c,  ulong y) :rights (y)
  {
    column= memdup_root(&memex,c.ptr(), key_length=c.length());
  }
};


static byte* get_key_column(GRANT_COLUMN *buff,uint *length,
			    my_bool not_used __attribute__((unused)))
{
  *length=buff->key_length;
  return (byte*) buff->column;
}


class GRANT_TABLE :public Sql_alloc
{
public:
  char *host,*db,*user,*tname, *hash_key;
  ulong privs, cols;
  ulong sort;
  uint key_length;
  HASH hash_columns;
  GRANT_TABLE (const char *h, const char *d,const char *u, const char *t,
	       ulong p, ulong c)
    : privs(p), cols(c)
  {
    host = strdup_root(&memex,h);
    db =   strdup_root(&memex,d);
    user = strdup_root(&memex,u);
    sort=  get_sort(3,host,db,user);
    tname= strdup_root(&memex,t);
    if (lower_case_table_names)
    {
      casedn_str(db);
      casedn_str(tname);
    }
    key_length =(uint) strlen(d)+(uint) strlen(u)+(uint) strlen(t)+3;
    hash_key = (char*) alloc_root(&memex,key_length);
    strmov(strmov(strmov(hash_key,user)+1,db)+1,tname);
    (void) hash_init(&hash_columns,0,0,0, (hash_get_key) get_key_column,0,
		     HASH_CASE_INSENSITIVE);
  }

  GRANT_TABLE (TABLE *form, TABLE *col_privs)
  {
    byte key[MAX_KEY_LENGTH];

    host =  get_field(&memex,form,0);
    db =    get_field(&memex,form,1);
    user =  get_field(&memex,form,2);
    if (!user)
      user=(char*) "";
    sort=   get_sort(3,host,db,user);
    tname = get_field(&memex,form,3);
    if (!host || !db || !tname)
    {
      /* Wrong table row; Ignore it */
      privs = cols = 0;				/* purecov: inspected */
      return;					/* purecov: inspected */
    }
    if (lower_case_table_names)
    {
      casedn_str(db);
      casedn_str(tname);
    }
    key_length = ((uint) strlen(db) + (uint) strlen(user) +
		  (uint) strlen(tname) + 3);
    hash_key = (char*) alloc_root(&memex,key_length);
    strmov(strmov(strmov(hash_key,user)+1,db)+1,tname);
    privs = (ulong) form->field[6]->val_int();
    cols  = (ulong) form->field[7]->val_int();
    privs = fix_rights_for_table(privs);
    cols =  fix_rights_for_column(cols);

    (void) hash_init(&hash_columns,0,0,0, (hash_get_key) get_key_column,0,
		     HASH_CASE_INSENSITIVE);
    if (cols)
    {
      int key_len;
      col_privs->field[0]->store(host,(uint) strlen(host));
      col_privs->field[1]->store(db,(uint) strlen(db));
      col_privs->field[2]->store(user,(uint) strlen(user));
      col_privs->field[3]->store(tname,(uint) strlen(tname));
      key_len=(col_privs->field[0]->pack_length()+
	       col_privs->field[1]->pack_length()+
	       col_privs->field[2]->pack_length()+
	       col_privs->field[3]->pack_length());
      key_copy(key,col_privs,0,key_len);
      col_privs->field[4]->store("",0);
      col_privs->file->index_init(0);
      if (col_privs->file->index_read(col_privs->record[0],
				      (byte*) col_privs->field[0]->ptr,
				      key_len, HA_READ_KEY_EXACT))
      {
	cols = 0; /* purecov: deadcode */
	return;
      }
      do
      {
	String *res,column_name;
	GRANT_COLUMN *mem_check;
	/* As column name is a string, we don't have to supply a buffer */
	res=col_privs->field[4]->val_str(&column_name,&column_name);
	ulong priv= (ulong) col_privs->field[6]->val_int();
	if (!(mem_check = new GRANT_COLUMN(*res,
					   fix_rights_for_column(priv))))
	{
	  /* Don't use this entry */
	  privs = cols = 0;			/* purecov: deadcode */
	  return;				/* purecov: deadcode */
	}
	hash_insert(&hash_columns, (byte *) mem_check);
      } while (!col_privs->file->index_next(col_privs->record[0]) &&
	       !key_cmp(col_privs,key,0,key_len));
    }
  }
  bool ok() { return privs != 0 || cols != 0; }
};


static byte* get_grant_table(GRANT_TABLE *buff,uint *length,
			     my_bool not_used __attribute__((unused)))
{
  *length=buff->key_length;
  return (byte*) buff->hash_key;
}


void free_grant_table(GRANT_TABLE *grant_table)
{
  hash_free(&grant_table->hash_columns);
}


/* Search after a matching grant. Prefer exact grants before not exact ones */

static GRANT_TABLE *table_hash_search(const char *host,const char* ip,
				      const char *db,
				      const char *user, const char *tname,
				      bool exact)
{
  char helping [NAME_LEN*2+USERNAME_LENGTH+3];
  uint len;
  GRANT_TABLE *grant_table,*found=0;
  safe_mutex_assert_owner(&LOCK_grant);

  len  = (uint) (strmov(strmov(strmov(helping,user)+1,db)+1,tname)-helping)+ 1;
  for (grant_table=(GRANT_TABLE*) hash_search(&hash_tables,(byte*) helping,
					      len) ;
       grant_table ;
       grant_table= (GRANT_TABLE*) hash_next(&hash_tables,(byte*) helping,len))
  {
    if (exact)
    {
      if ((host && !my_strcasecmp(host,grant_table->host)) ||
	  (ip && !strcmp(ip,grant_table->host)))
	return grant_table;
    }
    else
    {
      if (((host && !wild_case_compare(host,grant_table->host)) ||
	  (ip && !wild_case_compare(ip,grant_table->host))) &&
          (!found || found->sort < grant_table->sort))
	found=grant_table;
    }
  }
  return found;
}



inline GRANT_COLUMN *
column_hash_search(GRANT_TABLE *t, const char *cname, uint length)
{
  return (GRANT_COLUMN*) hash_search(&t->hash_columns, (byte*) cname,length);
}


static int replace_column_table(GRANT_TABLE *g_t,
				TABLE *table, const LEX_USER &combo,
				List <LEX_COLUMN> &columns,
				const char *db, const char *table_name,
				ulong rights, bool revoke_grant)
{
  int error=0,result=0;
  uint key_length;
  byte key[MAX_KEY_LENGTH];
  DBUG_ENTER("replace_column_table");

  table->field[0]->store(combo.host.str,combo.host.length);
  table->field[1]->store(db,(uint) strlen(db));
  table->field[2]->store(combo.user.str,combo.user.length);
  table->field[3]->store(table_name,(uint) strlen(table_name));
  key_length=(table->field[0]->pack_length()+ table->field[1]->pack_length()+
	      table->field[2]->pack_length()+ table->field[3]->pack_length());
  key_copy(key,table,0,key_length);

  rights &= COL_ACLS;				// Only ACL for columns

  /* first fix privileges for all columns in column list */

  List_iterator <LEX_COLUMN> iter(columns);
  class LEX_COLUMN *xx;
  table->file->index_init(0);
  while ((xx=iter++))
  {
    ulong privileges = xx->rights;
    bool old_row_exists=0;
    key_restore(table,key,0,key_length);
    table->field[4]->store(xx->column.ptr(),xx->column.length());

    if (table->file->index_read(table->record[0],(byte*) table->field[0]->ptr,
				0, HA_READ_KEY_EXACT))
    {
      if (revoke_grant)
      {
	my_printf_error(ER_NONEXISTING_TABLE_GRANT,
			ER(ER_NONEXISTING_TABLE_GRANT),MYF(0),
			combo.user.str, combo.host.str,table_name); /* purecov: inspected */
	result= -1; /* purecov: inspected */
	continue; /* purecov: inspected */
      }
      old_row_exists = 0;
      restore_record(table,2);			// Get empty record
      key_restore(table,key,0,key_length);
      table->field[4]->store(xx->column.ptr(),xx->column.length());
    }
    else
    {
      ulong tmp= (ulong) table->field[6]->val_int();
      tmp=fix_rights_for_column(tmp);

      if (revoke_grant)
	privileges = tmp & ~(privileges | rights);
      else
	privileges |= tmp;
      old_row_exists = 1;
      store_record(table,1);			// copy original row
    }

    table->field[6]->store((longlong) get_rights_for_column(privileges));

    if (old_row_exists)
    {
      if (privileges)
	error=table->file->update_row(table->record[1],table->record[0]);
      else
	error=table->file->delete_row(table->record[1]);
      if (error)
      {
	table->file->print_error(error,MYF(0)); /* purecov: inspected */
	result= -1;				/* purecov: inspected */
	goto end;				/* purecov: inspected */
      }
      GRANT_COLUMN *grant_column = column_hash_search(g_t,
						      xx->column.ptr(),
						      xx->column.length());
      if (grant_column)				// Should always be true
	grant_column->rights = privileges;	// Update hash
    }
    else					// new grant
    {
      if ((error=table->file->write_row(table->record[0])))
      {
	table->file->print_error(error,MYF(0)); /* purecov: inspected */
	result= -1;				/* purecov: inspected */
	goto end;				/* purecov: inspected */
      }
      GRANT_COLUMN *grant_column = new GRANT_COLUMN(xx->column,privileges);
      hash_insert(&g_t->hash_columns,(byte*) grant_column);
    }
  }
  table->file->index_end();

  /*
    If revoke of privileges on the table level, remove all such privileges
    for all columns
  */

  if (revoke_grant)
  {
    table->file->index_init(0);
    if (table->file->index_read(table->record[0], (byte*) table->field[0]->ptr,
				key_length, HA_READ_KEY_EXACT))
      goto end;

    /* Scan through all rows with the same host,db,user and table */
    do
    {
      ulong privileges = (ulong) table->field[6]->val_int();
      privileges=fix_rights_for_column(privileges);
      store_record(table,1);

      if (privileges & rights)	// is in this record the priv to be revoked ??
      {
	GRANT_COLUMN *grant_column = NULL;
	char  colum_name_buf[HOSTNAME_LENGTH+1];
	String column_name(colum_name_buf,sizeof(colum_name_buf));

	privileges&= ~rights;
	table->field[6]->store((longlong)
			       get_rights_for_column(privileges));
	table->field[4]->val_str(&column_name,&column_name);
	grant_column = column_hash_search(g_t,
					  column_name.ptr(),
					  column_name.length());
	if (privileges)
	{
	  int tmp_error;
	  if ((tmp_error=table->file->update_row(table->record[1],
						 table->record[0])))
	  {					/* purecov: deadcode */
	    table->file->print_error(tmp_error,MYF(0)); /* purecov: deadcode */
	    result= -1;				/* purecov: deadcode */
	    goto end;				/* purecov: deadcode */
	  }
	  if (grant_column)
	    grant_column->rights  = privileges; // Update hash
	}
	else
	{
	  int tmp_error;
	  if ((tmp_error = table->file->delete_row(table->record[1])))
	  {					/* purecov: deadcode */
	    table->file->print_error(tmp_error,MYF(0)); /* purecov: deadcode */
	    result= -1;				/* purecov: deadcode */
	    goto end;				/* purecov: deadcode */
	  }
	  if (grant_column)
	    hash_delete(&g_t->hash_columns,(byte*) grant_column);
	}
      }
    } while (!table->file->index_next(table->record[0]) &&
	     !key_cmp(table,key,0,key_length));
  }

end:
  table->file->index_end();
  DBUG_RETURN(result);
}


static int replace_table_table(THD *thd, GRANT_TABLE *grant_table,
			       TABLE *table, const LEX_USER &combo,
			       const char *db, const char *table_name,
			       ulong rights, ulong col_rights,
			       bool revoke_grant)
{
  char grantor[HOSTNAME_LENGTH+USERNAME_LENGTH+2];
  int old_row_exists = 1;
  int error=0;
  ulong store_table_rights, store_col_rights;
  DBUG_ENTER("replace_table_table");
  safe_mutex_assert_owner(&LOCK_grant);

  strxmov(grantor, thd->user, "@", thd->host_or_ip, NullS);

  /*
    The following should always succeed as new users are created before
    this function is called!
  */
  if (!find_acl_user(combo.host.str,combo.user.str))
  {
    my_error(ER_PASSWORD_NO_MATCH,MYF(0));	/* purecov: deadcode */
    DBUG_RETURN(-1);				/* purecov: deadcode */
  }

  restore_record(table,2);			// Get empty record
  table->field[0]->store(combo.host.str,combo.host.length);
  table->field[1]->store(db,(uint) strlen(db));
  table->field[2]->store(combo.user.str,combo.user.length);
  table->field[3]->store(table_name,(uint) strlen(table_name));
  store_record(table,1);			// store at pos 1

  if (table->file->index_read_idx(table->record[0],0,
				  (byte*) table->field[0]->ptr,0,
				  HA_READ_KEY_EXACT))
  {
    /*
      The following should never happen as we first check the in memory
      grant tables for the user.  There is however always a small change that
      the user has modified the grant tables directly.
    */
    if (revoke_grant)
    { // no row, no revoke
      my_printf_error(ER_NONEXISTING_TABLE_GRANT,
		      ER(ER_NONEXISTING_TABLE_GRANT),MYF(0),
		      combo.user.str,combo.host.str,
		      table_name);		/* purecov: deadcode */
      DBUG_RETURN(-1);				/* purecov: deadcode */
    }
    old_row_exists = 0;
    restore_record(table,1);			// Get saved record
  }

  store_table_rights= get_rights_for_table(rights);
  store_col_rights=   get_rights_for_column(col_rights);
  if (old_row_exists)
  {
    ulong j,k;
    store_record(table,1);
    j = (ulong) table->field[6]->val_int();
    k = (ulong) table->field[7]->val_int();

    if (revoke_grant)
    {
      /* column rights are already fixed in mysql_table_grant */
      store_table_rights=j & ~store_table_rights;
    }
    else
    {
      store_table_rights|= j;
      store_col_rights|=   k;
    }
  }

  table->field[4]->store(grantor,(uint) strlen(grantor));
  table->field[6]->store((longlong) store_table_rights);
  table->field[7]->store((longlong) store_col_rights);
  rights=fix_rights_for_table(store_table_rights);
  col_rights=fix_rights_for_column(store_col_rights);

  if (old_row_exists)
  {
    if (store_table_rights || store_col_rights)
    {
      if ((error=table->file->update_row(table->record[1],table->record[0])))
	goto table_error;			/* purecov: deadcode */
    }
    else if ((error = table->file->delete_row(table->record[1])))
      goto table_error;				/* purecov: deadcode */
  }
  else
  {
    error=table->file->write_row(table->record[0]);
    if (error && error != HA_ERR_FOUND_DUPP_KEY)
      goto table_error;				/* purecov: deadcode */
  }

  if (rights | col_rights)
  {
    grant_table->privs= rights;
    grant_table->cols=	col_rights;
  }
  else
  {
    hash_delete(&hash_tables,(byte*) grant_table);
  }
  DBUG_RETURN(0);

  /* This should never happen */
table_error:
  table->file->print_error(error,MYF(0)); /* purecov: deadcode */
  DBUG_RETURN(-1); /* purecov: deadcode */
}


/*
  Store table level and column level grants in the privilege tables

  SYNOPSIS
    mysql_table_grant()
    thd			Thread handle
    table_list		List of tables to give grant
    user_list		List of users to give grant
    columns		List of columns to give grant
    rights		Table level grant
    revoke_grant	Set to 1 if this is a REVOKE command

  RETURN
    0	ok
    1	error
*/

int mysql_table_grant(THD *thd, TABLE_LIST *table_list,
		      List <LEX_USER> &user_list,
		      List <LEX_COLUMN> &columns, ulong rights,
		      bool revoke_grant)
{
  ulong column_priv= 0;
  List_iterator <LEX_USER> str_list (user_list);
  LEX_USER *Str;
  TABLE_LIST tables[3];
  bool create_new_users=0;
  DBUG_ENTER("mysql_table_grant");

  if (!initialized)
  {
    send_error(&(thd->net), ER_UNKNOWN_COM_ERROR); /* purecov: inspected */
    return 1;					/* purecov: inspected */
  }
  if (rights & ~TABLE_ACLS)
  {
    my_error(ER_ILLEGAL_GRANT_FOR_TABLE,MYF(0));
    DBUG_RETURN(-1);
  }

  if (columns.elements && !revoke_grant)
  {
    TABLE *table;
    class LEX_COLUMN *column;
    List_iterator <LEX_COLUMN> column_iter(columns);

    if (!(table=open_ltable(thd,table_list,TL_READ)))
      DBUG_RETURN(-1);
    while ((column = column_iter++))
    {
      if (!find_field_in_table(thd,table,column->column.ptr(),
			       column->column.length(),0,0))
      {
	my_printf_error(ER_BAD_FIELD_ERROR,ER(ER_BAD_FIELD_ERROR),MYF(0),
			column->column.c_ptr(), table_list->alias);
	DBUG_RETURN(-1);
      }
      column_priv|= column->rights;
    }
    close_thread_tables(thd);
  }
  else if (!(rights & CREATE_ACL) && !revoke_grant)
  {
    char buf[FN_REFLEN];
    sprintf(buf,"%s/%s/%s.frm",mysql_data_home, table_list->db,
	    table_list->real_name);
    fn_format(buf,buf,"","",4+16+32);
    if (access(buf,F_OK))
    {
      my_error(ER_NO_SUCH_TABLE,MYF(0),table_list->db, table_list->alias);
      DBUG_RETURN(-1);
    }
  }

  /* open the mysql.tables_priv and mysql.columns_priv tables */

  bzero((char*) &tables,sizeof(tables));
  tables[0].alias=tables[0].real_name= (char*) "user";
  tables[1].alias=tables[1].real_name= (char*) "tables_priv";
  tables[2].alias=tables[2].real_name= (char*) "columns_priv";
  tables[0].next=tables+1;
  /* Don't open column table if we don't need it ! */
  tables[1].next=((column_priv ||
		   (revoke_grant && ((rights & COL_ACLS) || columns.elements)))
		  ? tables+2 : 0);
  tables[0].lock_type=tables[1].lock_type=tables[2].lock_type=TL_WRITE;
  tables[0].db=tables[1].db=tables[2].db=(char*) "mysql";

#ifdef HAVE_REPLICATION
  /*
    GRANT and REVOKE are applied the slave in/exclusion rules as they are
    some kind of updates to the mysql.% tables.
  */
  if (thd->slave_thread && table_rules_on)
  {
    /* 
       The tables must be marked "updating" so that tables_ok() takes them into
       account in tests.
    */
    tables[0].updating= tables[1].updating= tables[2].updating= 1;
    if (!tables_ok(0, tables))
      DBUG_RETURN(0);
  }
#endif

  if (open_and_lock_tables(thd,tables))
  {						// Should never happen
    close_thread_tables(thd);			/* purecov: deadcode */
    DBUG_RETURN(-1);				/* purecov: deadcode */
  }

  if (!revoke_grant)
    create_new_users= test_if_create_new_users(thd);
  int result=0;
  pthread_mutex_lock(&LOCK_grant);
  MEM_ROOT *old_root=my_pthread_getspecific_ptr(MEM_ROOT*,THR_MALLOC);
  my_pthread_setspecific_ptr(THR_MALLOC,&memex);

  while ((Str = str_list++))
  {
    int error;
    GRANT_TABLE *grant_table;
    if (!Str->host.str)
    {
      Str->host.str=(char*) "%";
      Str->host.length=1;
    }
    if (Str->host.length > HOSTNAME_LENGTH ||
	Str->user.length > USERNAME_LENGTH)
    {
      my_error(ER_GRANT_WRONG_HOST_OR_USER,MYF(0));
      result= -1;
      continue;
    }
    /* Create user if needed */
    pthread_mutex_lock(&acl_cache->lock);
    error=replace_user_table(thd, tables[0].table, *Str,
			     0, revoke_grant, create_new_users);
    pthread_mutex_unlock(&acl_cache->lock);
    if (error)
    {
      result= -1;				// Remember error
      continue;					// Add next user
    }

    /* Find/create cached table grant */
    grant_table= table_hash_search(Str->host.str,NullS,table_list->db,
				   Str->user.str,
				   table_list->real_name,1);
    if (!grant_table)
    {
      if (revoke_grant)
      {
	my_printf_error(ER_NONEXISTING_TABLE_GRANT,
			ER(ER_NONEXISTING_TABLE_GRANT),MYF(0),
			Str->user.str, Str->host.str, table_list->real_name);
	result= -1;
	continue;
      }
      grant_table = new GRANT_TABLE (Str->host.str,table_list->db,
				     Str->user.str,
				     table_list->real_name,
				     rights,
				     column_priv);
      if (!grant_table)				// end of memory
      {
	result= -1;				/* purecov: deadcode */
	continue;				/* purecov: deadcode */
      }
      hash_insert(&hash_tables,(byte*) grant_table);
    }

    /* If revoke_grant, calculate the new column privilege for tables_priv */
    if (revoke_grant)
    {
      class LEX_COLUMN *column;
      List_iterator <LEX_COLUMN> column_iter(columns);
      GRANT_COLUMN *grant_column;

      /* Fix old grants */
      while ((column = column_iter++))
      {
	grant_column = column_hash_search(grant_table,
					  column->column.ptr(),
					  column->column.length());
	if (grant_column)
	  grant_column->rights&= ~(column->rights | rights);
      }
      /* scan trough all columns to get new column grant */
      column_priv= 0;
      for (uint idx=0 ; idx < grant_table->hash_columns.records ; idx++)
      {
	grant_column= (GRANT_COLUMN*) hash_element(&grant_table->hash_columns,
						   idx);
	grant_column->rights&= ~rights;		// Fix other columns
	column_priv|= grant_column->rights;
      }
    }
    else
    {
      column_priv|= grant_table->cols;
    }


    /* update table and columns */

    if (replace_table_table(thd,grant_table,tables[1].table,*Str,
			    table_list->db,
			    table_list->real_name,
			    rights, column_priv, revoke_grant))
    {						// Crashend table ??
      result= -1;			       /* purecov: deadcode */
    }
    else if (tables[2].table)
    {
      if ((replace_column_table(grant_table,tables[2].table, *Str,
				columns,
				table_list->db,
				table_list->real_name,
				rights, revoke_grant)))
      {
	result= -1;
      }
    }
  }
  grant_option=TRUE;
  my_pthread_setspecific_ptr(THR_MALLOC,old_root);
  pthread_mutex_unlock(&LOCK_grant);
  if (!result)
    send_ok(&thd->net);
  /* Tables are automatically closed */
  DBUG_RETURN(result);
}


int mysql_grant (THD *thd, const char *db, List <LEX_USER> &list,
		 ulong rights, bool revoke_grant)
{
  List_iterator <LEX_USER> str_list (list);
  LEX_USER *Str;
  char tmp_db[NAME_LEN+1];
  bool create_new_users=0;
  TABLE_LIST tables[2];
  DBUG_ENTER("mysql_grant");

  if (!initialized)
  {
    send_error(&(thd->net), ER_UNKNOWN_COM_ERROR); /* purecov: tested */
    DBUG_RETURN(1); /* purecov: tested */
  }

  if (lower_case_table_names && db)
  {
    strmov(tmp_db,db);
    casedn_str(tmp_db);
    db=tmp_db;
  }

  /* open the mysql.user and mysql.db tables */

  tables[0].alias=tables[0].real_name=(char*) "user";
  tables[1].alias=tables[1].real_name=(char*) "db";
  tables[0].next=tables+1;
  tables[1].next=0;
  tables[0].lock_type=tables[1].lock_type=TL_WRITE;
  tables[0].db=tables[1].db=(char*) "mysql";
  tables[0].table=tables[1].table=0;

#ifdef HAVE_REPLICATION
  /*
    GRANT and REVOKE are applied the slave in/exclusion rules as they are
    some kind of updates to the mysql.% tables.
  */
  if (thd->slave_thread && table_rules_on)
  {
    /* 
       The tables must be marked "updating" so that tables_ok() takes them into
       account in tests.
    */
    tables[0].updating= tables[1].updating= 1;
    if (!tables_ok(0, tables))
      DBUG_RETURN(0);
  }
#endif

  if (open_and_lock_tables(thd,tables))
  {						// This should never happen
    close_thread_tables(thd);			/* purecov: deadcode */
    DBUG_RETURN(-1);				/* purecov: deadcode */
  }

  if (!revoke_grant)
    create_new_users= test_if_create_new_users(thd);

  /* go through users in user_list */
  pthread_mutex_lock(&LOCK_grant);
  VOID(pthread_mutex_lock(&acl_cache->lock));
  grant_version++;

  int result=0;
  while ((Str = str_list++))
  {
    if (!Str->host.str)
    {
      Str->host.str=(char*) "%";
      Str->host.length=1;
    }
    if (Str->host.length > HOSTNAME_LENGTH ||
	Str->user.length > USERNAME_LENGTH)
    {
      my_error(ER_GRANT_WRONG_HOST_OR_USER,MYF(0));
      result= -1;
      continue;
    }
    if ((replace_user_table(thd,
			    tables[0].table,
			    *Str,
			    (!db ? rights : 0), revoke_grant,
			    create_new_users)))
      result= -1;
    else if (db)
    {
      ulong db_rights= rights & DB_ACLS;
      if (db_rights  == rights)
      {
	if (replace_db_table(tables[1].table, db, *Str, db_rights,
			     revoke_grant))
	  result= -1;
      }
      else
      {
	net_printf(&thd->net,ER_WRONG_USAGE,"DB GRANT","GLOBAL PRIVILEGES");
	result= 1;
      }
    }
  }
  VOID(pthread_mutex_unlock(&acl_cache->lock));
  pthread_mutex_unlock(&LOCK_grant);
  close_thread_tables(thd);

  if (!result)
    send_ok(&thd->net);
  DBUG_RETURN(result);
}


/* Free grant array if possible */

void  grant_free(void)
{
  DBUG_ENTER("grant_free");
  grant_option = FALSE;
  hash_free(&hash_tables);
  free_root(&memex,MYF(0));
  DBUG_VOID_RETURN;
}


/* Init grant array if possible */

my_bool grant_init(THD *org_thd)
{
  THD  *thd;
  TABLE_LIST tables[2];
  MYSQL_LOCK *lock;
  my_bool return_val= 1;
  TABLE *t_table, *c_table;
  DBUG_ENTER("grant_init");

  grant_option = FALSE;
  (void) hash_init(&hash_tables,0,0,0, (hash_get_key) get_grant_table,
		   (hash_free_key) free_grant_table,0);
  init_sql_alloc(&memex, ACL_ALLOC_BLOCK_SIZE, 0);

  /* Don't do anything if running with --skip-grant */
  if (!initialized)
    DBUG_RETURN(0);				/* purecov: tested */

  if (!(thd=new THD))
    DBUG_RETURN(1);				/* purecov: deadcode */
  thd->store_globals();
  thd->db= my_strdup("mysql",MYF(0));
  thd->db_length=5;				// Safety
  bzero((char*) &tables,sizeof(tables));
  tables[0].alias=tables[0].real_name= (char*) "tables_priv";
  tables[1].alias=tables[1].real_name= (char*) "columns_priv";
  tables[0].next=tables+1;
  tables[0].lock_type=tables[1].lock_type=TL_READ;
  tables[0].db=tables[1].db=thd->db;

  if (open_tables(thd,tables))
    goto end;

  TABLE *ptr[2];				// Lock tables for quick update
  ptr[0]= tables[0].table;
  ptr[1]= tables[1].table;
  if (!(lock=mysql_lock_tables(thd,ptr,2)))
    goto end;

  t_table = tables[0].table; c_table = tables[1].table;
  t_table->file->index_init(0);
  if (t_table->file->index_first(t_table->record[0]))
  {
    t_table->file->index_end();
    return_val= 0;
    goto end_unlock;
  }
  grant_option= TRUE;
  t_table->file->index_end();

  /* Will be restored by org_thd->store_globals() */
  my_pthread_setspecific_ptr(THR_MALLOC,&memex);
  do
  {
    GRANT_TABLE *mem_check;
    if (!(mem_check=new GRANT_TABLE(t_table,c_table)) ||
	mem_check->ok() && hash_insert(&hash_tables,(byte*) mem_check))
    {
      /* This could only happen if we are out memory */
      grant_option= FALSE;			/* purecov: deadcode */
      goto end_unlock;
    }
  }
  while (!t_table->file->index_next(t_table->record[0]));

  return_val=0;					// Return ok

end_unlock:
  mysql_unlock_tables(thd, lock);
  thd->version--;				// Force close to free memory

end:
  close_thread_tables(thd);
  delete thd;
  if (org_thd)
    org_thd->store_globals();
  else
  {
    /* Remember that we don't have a THD */
    my_pthread_setspecific_ptr(THR_THD,  0);
  }
  DBUG_RETURN(return_val);
}


/*
  Reload grant array if possible

  SYNOPSIS
    grant_reload()
    thd			Thread handler

  NOTES
    Locked tables are checked by acl_init and doesn't have to be checked here
*/

void grant_reload(THD *thd)
{
  HASH old_hash_tables;
  bool old_grant_option;
  MEM_ROOT old_mem;
  DBUG_ENTER("grant_reload");

  pthread_mutex_lock(&LOCK_grant);
  grant_version++;
  old_hash_tables=hash_tables;
  old_grant_option= grant_option;
  old_mem = memex;

  if (grant_init(thd))
  {						// Error. Revert to old hash
    grant_free();				/* purecov: deadcode */
    hash_tables=old_hash_tables;		/* purecov: deadcode */
    grant_option= old_grant_option;		/* purecov: deadcode */
    memex = old_mem;				/* purecov: deadcode */
  }
  else
  {
    hash_free(&old_hash_tables);
    free_root(&old_mem,MYF(0));
  }
  pthread_mutex_unlock(&LOCK_grant);
  DBUG_VOID_RETURN;
}


/****************************************************************************
  Check grants
  All errors are written directly to the client if command name is given !
****************************************************************************/

bool check_grant(THD *thd, ulong want_access, TABLE_LIST *tables,
		 uint show_table, bool no_errors)
{
  TABLE_LIST *table;
  char *user = thd->priv_user;

  want_access &= ~thd->master_access;
  if (!want_access)
    return 0;					// ok

  pthread_mutex_lock(&LOCK_grant);
  for (table=tables; table ;table=table->next)
  {
    if (!(~table->grant.privilege & want_access))
    {
      table->grant.want_privilege=0;
      continue;					// Already checked
    }
    GRANT_TABLE *grant_table = table_hash_search(thd->host,thd->ip,
						 table->db,user,
						 table->real_name,0);
    if (!grant_table)
    {
      want_access &= ~table->grant.privilege;
      goto err;					// No grants
    }
    if (show_table)
      continue;					// We have some priv on this

    table->grant.grant_table=grant_table;	// Remember for column test
    table->grant.version=grant_version;
    table->grant.privilege|= grant_table->privs;
    table->grant.want_privilege= ((want_access & COL_ACLS)
				  & ~table->grant.privilege);

    if (!(~table->grant.privilege & want_access))
      continue;

    if (want_access & ~(grant_table->cols | table->grant.privilege))
    {
      want_access &= ~(grant_table->cols | table->grant.privilege);
      goto err;					// impossible
    }
  }
  pthread_mutex_unlock(&LOCK_grant);
  return 0;

err:
  pthread_mutex_unlock(&LOCK_grant);
  if (!no_errors)				// Not a silent skip of table
  {
    const char *command="";
    if (want_access & SELECT_ACL)
      command ="select";
    else if (want_access & INSERT_ACL)
      command = "insert";
    else if (want_access & UPDATE_ACL)
      command = "update";
    else if (want_access & DELETE_ACL)
      command = "delete";
    else if (want_access & DROP_ACL)
      command = "drop";
    else if (want_access & CREATE_ACL)
      command = "create";
    else if (want_access & ALTER_ACL)
      command = "alter";
    else if (want_access & INDEX_ACL)
      command = "index";
    else if (want_access & GRANT_ACL)
      command = "grant";
    net_printf(&thd->net,ER_TABLEACCESS_DENIED_ERROR,
	       command,
	       thd->priv_user,
	       thd->host_or_ip,
	       table ? table->real_name : "unknown");
  }
  return 1;
}


bool check_grant_column(THD *thd,TABLE *table, const char *name,
			uint length, uint show_tables)
{
  GRANT_TABLE *grant_table;
  GRANT_COLUMN *grant_column;

  ulong want_access=table->grant.want_privilege;
  if (!want_access)
    return 0;					// Already checked
  if (!grant_option)
    goto err2;

  pthread_mutex_lock(&LOCK_grant);

  /* reload table if someone has modified any grants */

  if (table->grant.version != grant_version)
  {
    table->grant.grant_table=
      table_hash_search(thd->host,thd->ip,thd->db,
			thd->priv_user,
			table->real_name,0);	/* purecov: inspected */
    table->grant.version=grant_version;		/* purecov: inspected */
  }
  if (!(grant_table=table->grant.grant_table))
    goto err;					/* purecov: deadcode */

  grant_column=column_hash_search(grant_table, name, length);
  if (grant_column && !(~grant_column->rights & want_access))
  {
    pthread_mutex_unlock(&LOCK_grant);
    return 0;
  }
#ifdef NOT_USED
  if (show_tables && (grant_column || table->grant.privilege & COL_ACLS))
  {
    pthread_mutex_unlock(&LOCK_grant);		/* purecov: deadcode */
    return 0;					/* purecov: deadcode */
  }
#endif

  /* We must use my_printf_error() here! */
err:
  pthread_mutex_unlock(&LOCK_grant);
err2:
  if (!show_tables)
  {
    char command[128];
    get_privilege_desc(command, sizeof(command), want_access);
    my_printf_error(ER_COLUMNACCESS_DENIED_ERROR,
		    ER(ER_COLUMNACCESS_DENIED_ERROR),
		    MYF(0),
		    command,
		    thd->priv_user,
		    thd->host_or_ip,
		    name,
		    table ? table->real_name : "unknown");
  }
  return 1;
}


bool check_grant_all_columns(THD *thd, ulong want_access, TABLE *table)
{
  GRANT_TABLE *grant_table;
  GRANT_COLUMN *grant_column;
  Field *field=0,**ptr;

  want_access &= ~table->grant.privilege;
  if (!want_access)
    return 0;					// Already checked

  pthread_mutex_lock(&LOCK_grant);

  /* reload table if someone has modified any grants */

  if (table->grant.version != grant_version)
  {
    table->grant.grant_table=
      table_hash_search(thd->host,thd->ip,thd->db,
			thd->priv_user,
			table->real_name,0);	/* purecov: inspected */
    table->grant.version=grant_version;		/* purecov: inspected */
  }
  /* The following should always be true */
  if (!(grant_table=table->grant.grant_table))
    goto err;					/* purecov: inspected */

  for (ptr=table->field; (field= *ptr) ; ptr++)
  {
    grant_column=column_hash_search(grant_table, field->field_name,
				    (uint) strlen(field->field_name));
    if (!grant_column || (~grant_column->rights & want_access))
      goto err;
  }
  pthread_mutex_unlock(&LOCK_grant);
  return 0;

  /* We must use my_printf_error() here! */
err:
  pthread_mutex_unlock(&LOCK_grant);

  const char *command="";
  if (want_access & SELECT_ACL)
    command ="select";
  else if (want_access & INSERT_ACL)
    command = "insert";
  my_printf_error(ER_COLUMNACCESS_DENIED_ERROR,
		  ER(ER_COLUMNACCESS_DENIED_ERROR),
		  MYF(0),
		  command,
		  thd->priv_user,
		  thd->host_or_ip,
		  field ? field->field_name : "unknown",
		  table->real_name);
  return 1;
}


/*
  Check if a user has the right to access a database
  Access is accepted if he has a grant for any table in the database
  Return 1 if access is denied
*/

bool check_grant_db(THD *thd,const char *db)
{
  char helping [NAME_LEN+USERNAME_LENGTH+2];
  uint len;
  bool error=1;

  len  = (uint) (strmov(strmov(helping,thd->priv_user)+1,db)-helping)+ 1;
  pthread_mutex_lock(&LOCK_grant);

  for (uint idx=0 ; idx < hash_tables.records ; idx++)
  {
    GRANT_TABLE *grant_table = (GRANT_TABLE*) hash_element(&hash_tables,idx);
    if (len < grant_table->key_length &&
	!memcmp(grant_table->hash_key,helping,len) &&
	(thd->host && !wild_case_compare(thd->host,grant_table->host) ||
	 (thd->ip && !wild_case_compare(thd->ip,grant_table->host))))
    {
      error=0;					// Found match
      break;
    }
  }
  pthread_mutex_unlock(&LOCK_grant);
  return error;
}

/*****************************************************************************
  Functions to retrieve the grant for a table/column  (for SHOW functions)
*****************************************************************************/

ulong get_table_grant(THD *thd, TABLE_LIST *table)
{
  uint privilege;
  char *user = thd->priv_user;
  const char *db = table->db ? table->db : thd->db;
  GRANT_TABLE *grant_table;

  pthread_mutex_lock(&LOCK_grant);
  grant_table = table_hash_search(thd->host,thd->ip,db,user,
				  table->real_name, 0);
  table->grant.grant_table=grant_table; // Remember for column test
  table->grant.version=grant_version;
  if (grant_table)
    table->grant.privilege|= grant_table->privs;
  privilege= table->grant.privilege;
  pthread_mutex_unlock(&LOCK_grant);
  return privilege;
}


ulong get_column_grant(THD *thd, TABLE_LIST *table, Field *field)
{
  GRANT_TABLE *grant_table;
  GRANT_COLUMN *grant_column;
  ulong priv;

  pthread_mutex_lock(&LOCK_grant);
  /* reload table if someone has modified any grants */
  if (table->grant.version != grant_version)
  {
    table->grant.grant_table=
      table_hash_search(thd->host,thd->ip,thd->db,
			thd->priv_user,
			table->real_name,0);	/* purecov: inspected */
    table->grant.version=grant_version;		/* purecov: inspected */
  }

  if (!(grant_table=table->grant.grant_table))
    priv=table->grant.privilege;
  else
  {
    grant_column=column_hash_search(grant_table, field->field_name,
				    (uint) strlen(field->field_name));
    if (!grant_column)
      priv=table->grant.privilege;
    else
      priv=table->grant.privilege | grant_column->rights;
  }
  pthread_mutex_unlock(&LOCK_grant);
  return priv;
}

/* Help function for mysql_show_grants */

static void add_user_option(String *grant, ulong value, const char *name)
{
  if (value)
  {
    char buff[22], *p; // just as in int2str
    grant->append(' ');
    grant->append(name, strlen(name));
    grant->append(' ');
    p=int10_to_str(value, buff, 10);
    grant->append(buff,p-buff);
  }
}

static const char *command_array[]=
{
  "SELECT", "INSERT","UPDATE","DELETE","CREATE", "DROP", "RELOAD","SHUTDOWN",
  "PROCESS","FILE","GRANT","REFERENCES","INDEX", "ALTER", "SHOW DATABASES",
  "SUPER", "CREATE TEMPORARY TABLES", "LOCK TABLES", "EXECUTE",
  "REPLICATION SLAVE", "REPLICATION CLIENT",
};

static uint command_lengths[]=
{
  6,6,6,6,6,4,6,8,7,4,5,10,5,5,14,5,23,11,7,17,18
};


/*
  SHOW GRANTS;  Send grants for a user to the client

  IMPLEMENTATION
   Send to client grant-like strings depicting user@host privileges
*/

int mysql_show_grants(THD *thd,LEX_USER *lex_user)
{
  ulong want_access;
  uint counter,index;
  int  error = 0;
  ACL_USER *acl_user; ACL_DB *acl_db;
  char buff[1024];
  DBUG_ENTER("mysql_show_grants");

  LINT_INIT(acl_user);
  if (!initialized)
  {
    send_error(&(thd->net), ER_UNKNOWN_COM_ERROR);
    DBUG_RETURN(-1);
  }
  if (!lex_user->host.str)
  {
    lex_user->host.str=(char*) "%";
    lex_user->host.length=1;
  }
  if (lex_user->host.length > HOSTNAME_LENGTH ||
      lex_user->user.length > USERNAME_LENGTH)
  {
    my_error(ER_GRANT_WRONG_HOST_OR_USER,MYF(0));
    DBUG_RETURN(-1);
  }

  for (counter=0 ; counter < acl_users.elements ; counter++)
  {
    const char *user,*host;
    acl_user=dynamic_element(&acl_users,counter,ACL_USER*);
    if (!(user=acl_user->user))
      user="";
    if (!(host=acl_user->host.hostname))
      host="%";
    if (!strcmp(lex_user->user.str,user) &&
	!my_strcasecmp(lex_user->host.str,host))
      break;
  }
  if (counter == acl_users.elements)
  {
    my_printf_error(ER_NONEXISTING_GRANT,ER(ER_NONEXISTING_GRANT),
		    MYF(0),lex_user->user.str,lex_user->host.str);
    DBUG_RETURN(-1);
  }

  Item_string *field=new Item_string("",0);
  List<Item> field_list;
  field->name=buff;
  field->max_length=1024;
  strxmov(buff,"Grants for ",lex_user->user.str,"@",
	  lex_user->host.str,NullS);
  field_list.push_back(field);
  if (send_fields(thd,field_list,1))
    DBUG_RETURN(-1);

  pthread_mutex_lock(&LOCK_grant);
  VOID(pthread_mutex_lock(&acl_cache->lock));

  /* Add first global access grants */
  {
    want_access=acl_user->access;
    String global(buff,sizeof(buff));
    global.length(0);
    global.append("GRANT ",6);

    if (test_all_bits(want_access, (GLOBAL_ACLS & ~ GRANT_ACL)))
      global.append("ALL PRIVILEGES",14);
    else if (!(want_access & ~GRANT_ACL))
      global.append("USAGE",5);
    else
    {
      bool found=0;
      ulong j,test_access= want_access & ~GRANT_ACL;
      for (counter=0, j = SELECT_ACL;j <= GLOBAL_ACLS;counter++,j <<= 1)
      {
	if (test_access & j)
	{
	  if (found)
	    global.append(", ",2);
	  found=1;
	  global.append(command_array[counter],command_lengths[counter]);
	}
      }
    }
    global.append (" ON *.* TO '",12);
    global.append(lex_user->user.str,lex_user->user.length);
    global.append ("'@'",3);
    global.append(lex_user->host.str,lex_user->host.length);
    global.append ('\'');
    if (acl_user->password)
    {
      char passd_buff[HASH_PASSWORD_LENGTH+1];
      make_password_from_salt(passd_buff,acl_user->salt);
      global.append(" IDENTIFIED BY PASSWORD '",25);
      global.append(passd_buff);
      global.append('\'');
    }
    /* "show grants" SSL related stuff */
    if (acl_user->ssl_type == SSL_TYPE_ANY)
      global.append(" REQUIRE SSL",12);
    else if (acl_user->ssl_type == SSL_TYPE_X509)
      global.append(" REQUIRE X509",13);
    else if (acl_user->ssl_type == SSL_TYPE_SPECIFIED)
    {
      int ssl_options = 0;
      global.append(" REQUIRE ",9);
      if (acl_user->x509_issuer)
      {
	ssl_options++;
	global.append("ISSUER \'",8);
	global.append(acl_user->x509_issuer,strlen(acl_user->x509_issuer));
	global.append('\'');
      }
      if (acl_user->x509_subject)
      {
	if (ssl_options++)
	  global.append(' ');
	global.append("SUBJECT \'",9);
	global.append(acl_user->x509_subject,strlen(acl_user->x509_subject));
	global.append('\'');
      }
      if (acl_user->ssl_cipher)
      {
	if (ssl_options++)
	  global.append(' ');
	global.append("CIPHER '",8);
	global.append(acl_user->ssl_cipher,strlen(acl_user->ssl_cipher));
	global.append('\'');
      }
    }
    if ((want_access & GRANT_ACL) ||
	(acl_user->user_resource.questions | acl_user->user_resource.updates |
	 acl_user->user_resource.connections))
    {
      global.append(" WITH",5);
      if (want_access & GRANT_ACL)
	global.append(" GRANT OPTION",13);
      add_user_option(&global, acl_user->user_resource.questions,
		      "MAX_QUERIES_PER_HOUR");
      add_user_option(&global, acl_user->user_resource.updates,
		      "MAX_UPDATES_PER_HOUR");
      add_user_option(&global, acl_user->user_resource.connections,
		      "MAX_CONNECTIONS_PER_HOUR");
    }
    thd->packet.length(0);
    net_store_data(&thd->packet,global.ptr(),global.length());
    if (my_net_write(&thd->net,(char*) thd->packet.ptr(),
		     thd->packet.length()))
    {
      error=-1; goto end;
    }
  }

  /* Add database access */
  for (counter=0 ; counter < acl_dbs.elements ; counter++)
  {
    const char *user,*host;

    acl_db=dynamic_element(&acl_dbs,counter,ACL_DB*);
    if (!(user=acl_db->user))
      user="";
    if (!(host=acl_db->host.hostname))
      host="";

    if (!strcmp(lex_user->user.str,user) &&
	!my_strcasecmp(lex_user->host.str,host))
    {
      want_access=acl_db->access;
      if (want_access)
      {
	String db(buff,sizeof(buff));
	db.length(0);
	db.append("GRANT ",6);

	if (test_all_bits(want_access,(DB_ACLS & ~GRANT_ACL)))
	  db.append("ALL PRIVILEGES",14);
	else if (!(want_access & ~GRANT_ACL))
	  db.append("USAGE",5);
	else
	{
	  int found=0, cnt;
	  ulong j,test_access= want_access & ~GRANT_ACL;
	  for (cnt=0, j = SELECT_ACL; j <= DB_ACLS; cnt++,j <<= 1)
	  {
	    if (test_access & j)
	    {
	      if (found)
		db.append(", ",2);
	      found = 1;
	      db.append(command_array[cnt],command_lengths[cnt]);
	    }
	  }
	}
	db.append(" ON `",5);
	db.append(acl_db->db);
	db.append("`.* TO '",8);
	db.append(lex_user->user.str,lex_user->user.length);
	db.append("'@'",3);
	db.append(lex_user->host.str, lex_user->host.length);
	db.append('\'');
	if (want_access & GRANT_ACL)
	  db.append(" WITH GRANT OPTION",18);
	thd->packet.length(0);
	net_store_data(&thd->packet,db.ptr(),db.length());
	if (my_net_write(&thd->net,(char*) thd->packet.ptr(),
			 thd->packet.length()))
	{
	  error=-1;
	  goto end;
	}
      }
    }
  }

  /* Add column access */
  for (index=0 ; index < hash_tables.records ; index++)
  {
    const char *user,*host;
    GRANT_TABLE *grant_table= (GRANT_TABLE*) hash_element(&hash_tables,index);

    if (!(user=grant_table->user))
      user="";
    if (!(host=grant_table->host))
      host="";

    if (!strcmp(lex_user->user.str,user) &&
	!my_strcasecmp(lex_user->host.str,host))
    {
      ulong table_access= grant_table->privs;
      if ((table_access | grant_table->cols) != 0)
      {
	String global(buff,sizeof(buff));
	ulong test_access= (table_access | grant_table->cols) & ~GRANT_ACL;

	global.length(0);
	global.append("GRANT ",6);

	if (test_all_bits(table_access, (TABLE_ACLS & ~GRANT_ACL)))
	  global.append("ALL PRIVILEGES",14);
 	else if (!test_access)
 	  global.append("USAGE",5);
	else
	{
	  int found= 0;
	  ulong j;

	  for (counter= 0, j= SELECT_ACL; j <= TABLE_ACLS; counter++, j<<= 1)
	  {
	    if (test_access & j)
	    {
	      if (found)
		global.append(", ",2);
	      found= 1;
	      global.append(command_array[counter],command_lengths[counter]);

	      if (grant_table->cols)
	      {
		uint found_col= 0;
		for (uint col_index=0 ;
		     col_index < grant_table->hash_columns.records ;
		     col_index++)
		{
		  GRANT_COLUMN *grant_column = (GRANT_COLUMN*)
		    hash_element(&grant_table->hash_columns,col_index);
		  if (grant_column->rights & j)
		  {
		    if (!found_col)
		    {
		      found_col= 1;
		      /*
			If we have a duplicated table level privilege, we
			must write the access privilege name again.
		      */
		      if (table_access & j)
		      {
			global.append(", ", 2);
			global.append(command_array[counter],
				      command_lengths[counter]);
		      }
		      global.append(" (",2);
		    }
		    else
		      global.append(", ",2);
		    global.append(grant_column->column,
				  grant_column->key_length);
		  }
		}
		if (found_col)
		  global.append(')');
	      }
	    }
	  }
	}
	global.append(" ON `",5);
	global.append(grant_table->db);
	global.append("`.`",3);
	global.append(grant_table->tname);
	global.append("` TO '",6);
	global.append(lex_user->user.str,lex_user->user.length);
	global.append("'@'",3);
	global.append(lex_user->host.str,lex_user->host.length);
	global.append('\'');
	if (table_access & GRANT_ACL)
	  global.append(" WITH GRANT OPTION",18);
	thd->packet.length(0);
	net_store_data(&thd->packet,global.ptr(),global.length());
	if (my_net_write(&thd->net,(char*) thd->packet.ptr(),
			 thd->packet.length()))
	{
	  error=-1;
	  break;
	}
      }
    }
  }

end:
  VOID(pthread_mutex_unlock(&acl_cache->lock));
  pthread_mutex_unlock(&LOCK_grant);

  send_eof(&thd->net);
  DBUG_RETURN(error);
}


/*
  Make a clear-text version of the requested privilege.
*/

void get_privilege_desc(char *to, uint max_length, ulong access)
{
  uint pos;
  char *start=to;
  DBUG_ASSERT(max_length >= 30);		// For end ',' removal

  if (access)
  {
    max_length--;				// Reserve place for end-zero
    for (pos=0 ; access ; pos++, access>>=1)
    {
      if ((access & 1) &&
	  command_lengths[pos] + (uint) (to-start) < max_length)
      {
	to= strmov(to, command_array[pos]);
	*to++=',';
      }
    }
    to--;					// Remove end ','
  }
  *to=0;
}


void get_mqh(const char *user, const char *host, USER_CONN *uc)
{
  ACL_USER *acl_user;
  if (initialized && (acl_user= find_acl_user(host,user)))
    uc->user_resources= acl_user->user_resource;
  else
    bzero((char*) &uc->user_resources, sizeof(uc->user_resources));
}



/*****************************************************************************
  Instantiate used templates
*****************************************************************************/

#ifdef __GNUC__
template class List_iterator<LEX_COLUMN>;
template class List_iterator<LEX_USER>;
template class List<LEX_COLUMN>;
template class List<LEX_USER>;
#endif

/*
 * ProFTPD: mod_sql_postgres -- Support for connecting to Postgres databases.
 * Time-stamp: <1999-10-04 03:21:21 root>
 * Copyright (c) 2001 Andrew Houghton
 * Copyright (c) 2004 TJ Saunders
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Andrew Houghton and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 *
 * $Id: mod_sql_postgres.c,v 1.26 2004/12/16 18:17:42 castaglia Exp $
 */

/*
 * $Libraries: -lm -lpq $
 */

/* 
 * Internal define used for debug and logging.  All backends are encouraged
 * to use the same format.
 */
#define MOD_SQL_POSTGRES_VERSION	"mod_sql_postgres/4.02"

#define _POSTGRES_PORT "5432"

#include <libpq-fe.h>
#include "conf.h"
#include "../contrib/mod_sql.h"

/* 
 * timer-handling code adds the need for a couple of forward declarations
 */
MODRET cmd_close( cmd_rec *cmd );
module sql_postgres_module;

/* 
 * db_conn_struct: an internal struct to hold connection information. This 
 *  connection information is backend-specific; the members here reflect 
 *  the information Postgres needs for connections.  
 */
struct db_conn_struct {

  /* Postgres-specific members */

  char *host;
  char *user;
  char *pass;
  char *db;
  char *port;

  char *connectstring;

  PGconn *postgres;
  PGresult *result;
};

typedef struct db_conn_struct db_conn_t;

/*
 * This struct is a wrapper for whatever backend data is needed to access 
 * the database, and supports named connections, connection counting, and 
 * timer handling.  
 */

struct conn_entry_struct {
  char *name;
  void *data;

  /* timer handling */

  int timer;
  int ttl;

  /* connection handling */

  unsigned int connections;
};

typedef struct conn_entry_struct conn_entry_t;

#define DEF_CONN_POOL_SIZE 10

static pool *conn_pool = NULL;
static array_header *conn_cache = NULL;

/*
 *  _sql_get_connection: walks the connection cache looking for the named
 *   connection.  Returns NULL if unsuccessful, a pointer to the conn_entry_t
 *   if successful.
 */
static conn_entry_t *_sql_get_connection(char *name)
{
  conn_entry_t *entry = NULL;
  int cnt;

  if (name == NULL) return NULL;

  /* walk the array looking for our entry */
  for (cnt=0; cnt < conn_cache->nelts; cnt++) {
    entry = ((conn_entry_t **) conn_cache->elts)[cnt];
    if (!strcmp(name, entry->name)) {
      return entry;
    }
  }

  return NULL;
}

/* 
 * _sql_add_connection: internal helper function to maintain a cache of 
 *  connections.  Since we expect the number of named connections to
 *  be small, simply use an array header to hold them.  We don't allow 
 *  duplicate connection names.
 *
 * Returns: NULL if the insertion was unsuccessful, a pointer to the 
 *  conn_entry_t that was created if successful.
 */
static void *_sql_add_connection(pool *p, char *name, db_conn_t *conn)
{
  conn_entry_t *entry = NULL;

  if ((!name) || (!conn) || (!p)) return NULL;
  
  if (_sql_get_connection(name)) {
    /* duplicated name */
    return NULL;
  }

  entry = (conn_entry_t *) pcalloc( p, sizeof( conn_entry_t ));
  entry->name = name;
  entry->data = conn;

  *((conn_entry_t **) push_array(conn_cache)) = entry;

  return entry;
}

/* _sql_check_cmd: tests to make sure the cmd_rec is valid and is 
 *  properly filled in.  If not, it's grounds for the daemon to
 *  shutdown.
 */
static void _sql_check_cmd(cmd_rec *cmd, char *msg) {
  if ((!cmd) || (!cmd->tmp_pool)) {
    pr_log_pri(PR_LOG_ERR, MOD_SQL_POSTGRES_VERSION
      ": '%s' was passed an invalid cmd_rec. Shutting down.", msg);
    sql_log(DEBUG_WARN, "'%s' was passed an invalid cmd_rec. Shutting down.",
      msg);
    end_login(1);
  }    

  return;
}

/*
 * _sql_timer_callback: when a timer goes off, this is the function
 *  that gets called.  This function makes assumptions about the 
 *  db_conn_t members.
 */
static int _sql_timer_callback(CALLBACK_FRAME) {
  conn_entry_t *entry = NULL;
  int cnt = 0;
  cmd_rec *cmd = NULL;
 
  for (cnt=0; cnt < conn_cache->nelts; cnt++) {
    entry = ((conn_entry_t **) conn_cache->elts)[cnt];

    if (entry->timer == p2) {
      sql_log(DEBUG_INFO, "timer expired for connection '%s'", entry->name);
      cmd = _sql_make_cmd( conn_pool, 2, entry->name, "1" );
      cmd_close( cmd );
      SQL_FREE_CMD(cmd);
      entry->timer = 0;
    }
  }

  return 0;
}

/* 
 * _build_error: constructs a modret_t filled with error information;
 *  mod_sql_postgres calls this function and returns the resulting mod_ret_t
 *  whenever a call to the database results in an error.
 */
static modret_t *_build_error(cmd_rec *cmd, db_conn_t *conn) {
  if (!conn)
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");

  return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION,
    PQerrorMessage(conn->postgres));
}

/*
 * _build_data: both cmd_select and cmd_procedure potentially
 *  return data to mod_sql; this function builds a modret to return
 *  that data.  This is Postgres specific; other backends may choose 
 *  to do things differently.
 */
static modret_t *_build_data(cmd_rec *cmd, db_conn_t *conn) {
  PGresult *result = NULL;
  sql_data_t *sd = NULL;
  char **data = NULL;
  int index = 0;
  int field = 0;
  int row =0;

  if (!conn) 
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");

  result = conn->result;

  sd = (sql_data_t *) pcalloc(cmd->tmp_pool, sizeof(sql_data_t));
  sd->rnum = (unsigned long) PQntuples(result);
  sd->fnum = (unsigned long) PQnfields(result);

  data = (char **) pcalloc( cmd->tmp_pool, sizeof(char *) * 
			    ((sd->rnum * sd->fnum) + 1) );
  
  for (row = 0; row < sd->rnum; row++) {
    for (field = 0; field < sd->fnum; field++) {
      data[index++] = pstrdup(cmd->tmp_pool, PQgetvalue(result, row, field));
    }
  }
  data[index] = NULL;

  sd->data = data;

  return mod_create_data( cmd, (void *) sd );
}

/*
 * cmd_open: attempts to open a named connection to the database.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *
 * Returns:
 *  either a properly filled error modret_t if a connection could not be
 *  opened, or a simple non-error modret_t.
 *
 * Notes:
 *  mod_sql depends on these semantics -- a backend should not open
 *  a connection unless mod_sql requests it, nor close one unless 
 *  mod_sql requests it.  Connection counting is *REQUIRED* for complete
 *  compatibility; a connection should not be closed unless the count
 *  reaches 0, and ideally will not need to be re-opened for counts > 1.
 */
MODRET cmd_open(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_open");

  _sql_check_cmd(cmd, "cmd_open" );

  if (cmd->argc < 1) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_open");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }    

  /* get the named connection */

  if (!(entry = _sql_get_connection(cmd->argv[0]))) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_open");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  } 

  conn = (db_conn_t *) entry->data;

  /* if we're already open (connections > 0) increment connections 
   * reset our timer if we have one, and return HANDLED 
   */
  if ((entry->connections > 0) && 
      (PQstatus(conn->postgres) == CONNECTION_OK)) {
    entry->connections++;
    if (entry->timer) 
      pr_timer_reset(entry->timer, &sql_postgres_module);

    sql_log(DEBUG_INFO, "connection '%s' count is now %d", entry->name,
      entry->connections);
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_open");
    return HANDLED(cmd);
  }

  /* make sure we have a new conn struct */
  conn->postgres = PQconnectdb(conn->connectstring);
  
  if (PQstatus(conn->postgres) == CONNECTION_BAD) {
    /* if it didn't work, return an error */
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_open");
    return _build_error( cmd, conn );
  }

  /* bump connections */
  entry->connections++;

  /* set up our timer if necessary */
  if (entry->ttl > 0) {
    entry->timer = pr_timer_add(entry->ttl, -1, &sql_postgres_module,
      _sql_timer_callback);
    sql_log(DEBUG_INFO, "connection '%s' - %d second timer started",
      entry->name, entry->ttl);

    /* timed connections get re-bumped so they don't go away when cmd_close
     * is called.
     */
    entry->connections++;
  }

  /* return HANDLED */
  sql_log(DEBUG_INFO, "connection '%s' opened", entry->name);

  sql_log(DEBUG_INFO, "connection '%s' count is now %d", entry->name,
    entry->connections);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_open");
  return HANDLED(cmd);
}

/*
 * cmd_close: attempts to close the named connection.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 * Optional:
 *  cmd->argv[1]: close immediately
 *
 * Returns:
 *  either a properly filled error modret_t if a connection could not be
 *  closed, or a simple non-error modret_t.  For the case of mod_sql_postgres,
 *  there are no error codes returned by the close call; other backends
 *  may be able to return a useful error message.
 *
 * Notes:
 *  mod_sql depends on these semantics -- a backend should not open
 *  a connection unless mod_sql requests it, nor close one unless 
 *  mod_sql requests it.  Connection counting is *REQUIRED* for complete
 *  compatibility; a connection should not be closed unless the count
 *  reaches 0, and should not need to be re-opened for counts > 1.
 * 
 *  If argv[1] exists and is not NULL, the connection should be immediately
 *  closed and the connection count should be reset to 0.
 */
MODRET cmd_close(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_close");

  _sql_check_cmd(cmd, "cmd_close");

  if ((cmd->argc < 1) || (cmd->argc > 2)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_close");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  if (!(entry = _sql_get_connection(cmd->argv[0]))) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_close");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  /* if we're closed already (connections == 0) return HANDLED */
  if (entry->connections == 0) {
    sql_log(DEBUG_INFO, "connection '%s' count is now %d", entry->name,
      entry->connections);

    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_close");
    return HANDLED(cmd);
  }

  /* decrement connections. If our count is 0 or we received a second arg
   * close the connection, explicitly set the counter to 0, and remove any
   * timers.
   */
  if (((--entry->connections) == 0 ) || ((cmd->argc == 2) && (cmd->argv[1]))) {
    PQfinish(conn->postgres);
    conn->postgres = NULL;
    entry->connections = 0;

    if (entry->timer) {
      pr_timer_remove(entry->timer, &sql_postgres_module);
      entry->timer = 0;
      sql_log(DEBUG_INFO, "connection '%s' - timer stopped", entry->name);
    }

    sql_log(DEBUG_INFO, "connection '%s' closed", entry->name);
  }

  sql_log(DEBUG_INFO, "connection '%s' count is now %d", entry->name,
    entry->connections);
  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_close");
  
  return HANDLED(cmd);
}

/*
 * cmd_defineconnection: takes all information about a database
 *  connection and stores it for later use.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: username portion of the SQLConnectInfo directive
 *  cmd->argv[2]: password portion of the SQLConnectInfo directive
 *  cmd->argv[3]: info portion of the SQLConnectInfo directive
 * Optional:
 *  cmd->argv[4]: time-to-live in seconds
 *
 * Returns:
 *  either a properly filled error modret_t if the connection could not
 *  defined, or a simple non-error modret_t.
 *
 * Notes:
 *  time-to-live is the length of time to allow a connection to remain unused;
 *  once that amount of time has passed, a connection should be closed and 
 *  it's connection count should be reduced to 0.  If ttl is 0, or ttl is not 
 *  a number or ttl is negative, the connection will be assumed to have no
 *  associated timer.
 */
MODRET cmd_defineconnection(cmd_rec *cmd) {
  char *info = NULL;
  char *name = NULL;

  char *db = NULL;
  char *host = NULL;
  char *port = NULL;

  char *havehost = NULL;
  char *haveport = NULL;
  
  char *connectstring = NULL;
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL; 

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_defineconnection");

  _sql_check_cmd(cmd, "cmd_defineconnection");

  if ((cmd->argc < 4) || (cmd->argc > 5) || (!cmd->argv[0])) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_defineconnection");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  conn = (db_conn_t *) palloc(conn_pool, sizeof(db_conn_t));

  name = pstrdup(conn_pool, cmd->argv[0]);
  conn->user = pstrdup(conn_pool, cmd->argv[1]);
  conn->pass = pstrdup(conn_pool, cmd->argv[2]);

  info = cmd->argv[3];

  db = pstrdup(cmd->tmp_pool, info);

  havehost = strchr(db, '@');
  haveport = strchr(db, ':');

  /*
   * if haveport, parse it, otherwise default it. 
   * if haveport, set it to '\0'
   *
   * if havehost, parse it, otherwise default it.
   * if havehost, set it to '\0'
   */

  if (haveport) {
    port = haveport + 1;
    *haveport = '\0';
  } else {
    port = _POSTGRES_PORT;
  }

  if (havehost) {
    host = havehost + 1;
    *havehost = '\0';
  } else {
    host = "localhost";
  }

  conn->host = pstrdup(conn_pool, host);
  conn->db   = pstrdup(conn_pool, db);
  conn->port = pstrdup(conn_pool, port);

  /* setup the connect string the way postgres likes it */
  connectstring = pstrcat(cmd->tmp_pool, "host='", conn->host, "' port='",
			  conn->port,"' dbname='", conn->db, "' user='",
			  conn->user,"' password='", conn->pass, "'", NULL);
  conn->connectstring = pstrdup(conn_pool, connectstring);


  /* insert the new conn_info into the connection hash */
  if (!(entry = _sql_add_connection(conn_pool, name, (void *) conn))) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_defineconnection");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION,
      "named connection already exists");
  }

  entry->ttl = (cmd->argc == 5) ? 
    (int) strtol(cmd->argv[4], (char **)NULL, 10) : 0;
  if (entry->ttl < 0) 
    entry->ttl = 0;

  entry->timer = 0;
  entry->connections = 0;

  sql_log(DEBUG_INFO, " name: '%s'", entry->name);
  sql_log(DEBUG_INFO, " user: '%s'", conn->user);
  sql_log(DEBUG_INFO, " host: '%s'", conn->host);
  sql_log(DEBUG_INFO, "   db: '%s'", conn->db);
  sql_log(DEBUG_INFO, " port: '%s'", conn->port);
  sql_log(DEBUG_INFO, "  ttl: '%d'", entry->ttl);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_defineconnection");
  return HANDLED(cmd);
}

/*
 * cmd_exit: closes all open connections.
 *
 * Inputs:
 *  None
 *
 * Returns:
 *  A simple non-error modret_t.
 */
static modret_t *cmd_exit(cmd_rec *cmd) {
  register unsigned int i = 0;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_exit");

  for (i = 0; i < conn_cache->nelts; i++) {
    conn_entry_t *entry = ((conn_entry_t **) conn_cache->elts)[i];

    if (entry->connections > 0) {
      cmd_rec *close_cmd = _sql_make_cmd(conn_pool, 2, entry->name, "1");
      cmd_close(close_cmd);
      destroy_pool(close_cmd->pool);
    }
  }

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_exit");

  return HANDLED(cmd);
}

/*
 * cmd_select: executes a SELECT query. properly constructing the query
 *  based on the inputs.  See mod_sql.h for the definition of the _sql_data
 *  structure which is used to return the result data.
 *
 * cmd_select takes either exactly two inputs, or more than two.  If only
 *  two inputs are given, the second is a monolithic query string.  See 
 *  the examples below.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: table 
 *  cmd->argv[2]: select string
 * Optional:
 *  cmd->argv[3]: where clause 
 *  cmd->argv[4]: requested number of return rows (LIMIT)
 *  
 *  etc.        : other options, such as "GROUP BY", "ORDER BY",
 *                and "DISTINCT" will start at cmd->arg[5].  All 
 *                backends MUST support 'DISTINCT', the other
 *                arguments are optional (but encouraged).         
 *
 * Returns:
 *  either a properly filled error modret_t if the select failed, or a 
 *  modret_t with the result data filled in.
 *
 * Example:
 *  These are example queries that would be executed for Postgres; other
 *  backends will have different SQL syntax.
 *  
 *  argv[] = "default","user","userid, count", "userid='aah'","2"
 *  query  = "SELECT userid, count FROM user WHERE userid='aah' LIMIT 2"
 *
 *  argv[] = "default","usr1, usr2","usr1.foo, usr2.bar"
 *  query  = "SELECT usr1.foo, usr2.bar FROM usr1, usr2"
 *
 *  argv[] = "default","usr1","foo",,,"DISTINCT"
 *  query  = "SELECT DISTINCT foo FROM usr1"
 *
 *  argv[] = "default","bar FROM usr1 WHERE tmp=1 ORDER BY bar"
 *  query  = "SELECT bar FROM usr1 WHERE tmp=1 ORDER BY bar"
 *
 * Notes:
 *  certain selects could return huge amounts of data.  do whatever is
 *  possible to minimize the amount of data copying here.
 */
MODRET cmd_select(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;
  modret_t *cmr = NULL;
  modret_t *dmr = NULL;
  char *query = NULL;
  int cnt = 0;
  cmd_rec *close_cmd;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_select");

  _sql_check_cmd(cmd, "cmd_select");

  if (cmd->argc < 2) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }
  
  conn = (db_conn_t *) entry->data;

  cmr = cmd_open(cmd);
  if (MODRET_ERROR(cmr)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
    return cmr;
  }

  /* construct the query string */
  if (cmd->argc == 2) {
    query = pstrcat(cmd->tmp_pool, "SELECT ", cmd->argv[1], NULL);
  } else {
    query = pstrcat( cmd->tmp_pool, cmd->argv[2], " FROM ", 
		     cmd->argv[1], NULL );
    if ((cmd->argc > 3) && (cmd->argv[3]))
      query = pstrcat( cmd->tmp_pool, query, " WHERE ", cmd->argv[3], NULL );
    if ((cmd->argc > 4) && (cmd->argv[4]))
      query = pstrcat( cmd->tmp_pool, query, " LIMIT ", cmd->argv[4], NULL );
    if (cmd->argc > 5) {

      /* handle the optional arguments -- they're rare, so in this case
       * we'll play with the already constructed query string, but in 
       * general we should probably take optional arguments into account 
       * and put the query string together later once we know what they are.
       */
    
      for (cnt=5; cnt < cmd->argc; cnt++) {
	if ((cmd->argv[cnt]) && !strcasecmp("DISTINCT",cmd->argv[cnt])) {
	  query = pstrcat( cmd->tmp_pool, "DISTINCT ", query, NULL);
	}
      }
    }

    query = pstrcat( cmd->tmp_pool, "SELECT ", query, NULL);    
  }

  /* log the query string */
  sql_log(DEBUG_INFO, "query \"%s\"", query);

  /* perform the query.  if it doesn't work, log the error, close the
   * connection then return the error from the query processing.
   */
  if (!(conn->result = PQexec(conn->postgres, query)) ||
      (PQresultStatus(conn->result) != PGRES_TUPLES_OK)) {
    dmr = _build_error( cmd, conn );

    if (conn->result) PQclear(conn->result);

    close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
    cmd_close(close_cmd);
    SQL_FREE_CMD(close_cmd);

    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
    return dmr;
  }

  /* get the data. if it doesn't work, log the error, close the
   * connection then return the error from the data processing.
   */
  dmr = _build_data( cmd, conn );

  PQclear(conn->result);

  if (MODRET_ERROR(dmr)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");

    close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
    cmd_close(close_cmd);
    SQL_FREE_CMD(close_cmd);

    return dmr;
  }    

  /* close the connection, return the data. */
  close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
  cmd_close(close_cmd);
  SQL_FREE_CMD(close_cmd);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
  return dmr;
}

/*
 * cmd_insert: executes an INSERT query, properly constructing the query
 *  based on the inputs.
 *
 * cmd_insert takes either exactly two inputs, or exactly four.  If only
 *  two inputs are given, the second is a monolithic query string.  See 
 *  the examples below.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: table
 *  cmd->argv[2]: field string
 *  cmd->argv[3]: value string
 *
 * Returns:
 *  either a properly filled error modret_t if the insert failed, or a 
 *  simple non-error modret_t.
 *
 * Example:
 *  These are example queries that would be executed for Postgres; other
 *  backends will have different SQL syntax.
 *  
 *  argv[] = "default","log","userid, date, count", "'aah', now(), 2"
 *  query  = "INSERT INTO log (userid, date, count) VALUES ('aah', now(), 2)"
 *
 *  argv[] = "default"," INTO foo VALUES ('do','re','mi','fa')"
 *  query  = "INSERT INTO foo VALUES ('do','re','mi','fa')"
 *
 * Notes:
 *  none
 */
MODRET cmd_insert(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;
  modret_t *cmr = NULL;
  modret_t *dmr = NULL;
  char *query = NULL;
  cmd_rec *close_cmd;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_insert");

  _sql_check_cmd(cmd, "cmd_insert");

  if ((cmd->argc != 2) && (cmd->argc != 4)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_insert");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_insert");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  cmr = cmd_open(cmd);
  if (MODRET_ERROR(cmr)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_insert");
    return cmr;
  }

  /* construct the query string */
  if (cmd->argc == 2) {
    query = pstrcat(cmd->tmp_pool, "INSERT ", cmd->argv[1], NULL);
  } else {
    query = pstrcat( cmd->tmp_pool, "INSERT INTO ", cmd->argv[1], " (",
		     cmd->argv[2], ") VALUES (", cmd->argv[3], ")",
		     NULL );
  }

  /* log the query string */
  sql_log(DEBUG_INFO, "query \"%s\"", query);

  /* perform the query.  if it doesn't work, log the error, close the
   * connection then return the error from the query processing.
   */
  if (!(conn->result = PQexec(conn->postgres, query)) ||
      (PQresultStatus(conn->result) != PGRES_COMMAND_OK)) {
    dmr = _build_error( cmd, conn );

    if (conn->result) PQclear(conn->result);

    close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
    cmd_close(close_cmd);
    SQL_FREE_CMD(close_cmd);

    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_insert");
    return dmr;
  }

  PQclear(conn->result);

  /* close the connection and return HANDLED. */
  close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
  cmd_close(close_cmd);
  SQL_FREE_CMD(close_cmd);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_insert");
  return HANDLED(cmd);
}

/*
 * cmd_update: executes an UPDATE query, properly constructing the query
 *  based on the inputs.
 *
 * cmd_update takes either exactly two, three, or four inputs.  If only
 *  two inputs are given, the second is a monolithic query string.  See 
 *  the examples below.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: table
 *  cmd->argv[2]: update string
 * Optional:
 *  cmd->argv[3]: where string
 *
 * Returns:
 *  either a properly filled error modret_t if the update failed, or a 
 *  simple non-error modret_t. *  
 *
 * Example:
 *  These are example queries that would be executed for Postgres; other
 *  backends will have different SQL syntax.
 *  
 *  argv[] = "default","user","count=count+1", "userid='joesmith'"
 *  query  = "UPDATE user SET count=count+1 WHERE userid='joesmith'"
 *
 * Notes:
 *  argv[3] is optional -- it may be NULL, or it may not exist at all.  
 *  make sure this is handled correctly. 
 */
MODRET cmd_update(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;
  modret_t *cmr = NULL;
  modret_t *dmr = NULL;
  char *query = NULL;
  cmd_rec *close_cmd;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_update");

  _sql_check_cmd(cmd, "cmd_update");

  if ((cmd->argc < 2) || (cmd->argc > 4)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_update");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_update");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  cmr = cmd_open(cmd);
  if (MODRET_ERROR(cmr)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_update");
    return cmr;
  }

  if (cmd->argc == 2) {
    query = pstrcat(cmd->tmp_pool, "UPDATE ", cmd->argv[1], NULL);
  } else {
    /* construct the query string */
    query = pstrcat( cmd->tmp_pool, "UPDATE ", cmd->argv[1], " SET ",
		     cmd->argv[2], NULL );
    if ((cmd->argc > 3) && (cmd->argv[3]))
      query = pstrcat( cmd->tmp_pool, query, " WHERE ", cmd->argv[3], NULL );
  }

  /* log the query string */
  sql_log(DEBUG_INFO, "query \"%s\"", query);

  /* perform the query.  if it doesn't work, log the error, close the
   * connection then return the error from the query processing.
   */
  if (!(conn->result = PQexec(conn->postgres, query)) ||
      (PQresultStatus(conn->result) != PGRES_COMMAND_OK)) {
    dmr = _build_error( cmd, conn );

    if (conn->result) PQclear(conn->result);

    close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
    cmd_close(close_cmd);
    SQL_FREE_CMD(close_cmd);

    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_update");
    return dmr;
  }

  PQclear(conn->result);

  /* close the connection, return HANDLED.  */
  close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
  cmd_close(close_cmd);
  SQL_FREE_CMD(close_cmd);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_update");
  return HANDLED(cmd);
}

/*
 * cmd_procedure: executes a stored procedure.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: procedure name
 *  cmd->argv[2]: procedure string
 *
 * Returns:
 *  either a properly filled error modret_t if the procedure failed in
 *  some way, or a modret_t with the result data.  If a procedure
 *  returns data, it should be returned in the same way as cmd_select.
 *
 * Notes:
 *  not every backend will support stored procedures.  Backends which do
 *  not support stored procedures should return an error with a descriptive
 *  error message (something like 'backend does not support procedures').
 */
MODRET cmd_procedure(cmd_rec *cmd) {
  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_procedure");

  _sql_check_cmd(cmd, "cmd_procedure");

  if (cmd->argc != 3) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_procedure");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* PostgreSQL supports procedures, but the backend doesn't. */

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_procedure");

  return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION,
    "backend does not support procedures");
}

/*
 * cmd_query: executes a freeform query string, with no syntax checking.
 *
 * cmd_query takes exactly two inputs, the connection and the query string.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: query string
 *
 * Returns:
 *  depending on the query type, returns a modret_t with data, a non-error
 *  modret_t, or a properly filled error modret_t if the query failed.
 *
 * Example:
 *  None.  The query should be passed directly to the backend database.
 *  
 * Notes:
 *  None.
 */
MODRET cmd_query(cmd_rec *cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;
  modret_t *cmr = NULL;
  modret_t *dmr = NULL;
  char *query = NULL;
  cmd_rec *close_cmd;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_query");

  _sql_check_cmd(cmd, "cmd_query");

  if (cmd->argc != 2) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_query");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_query");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  cmr = cmd_open(cmd);
  if (MODRET_ERROR(cmr)) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_query");
    return cmr;
  }

  query = pstrcat(cmd->tmp_pool, cmd->argv[1], NULL);

  /* log the query string */
  sql_log( DEBUG_INFO, "query \"%s\"", query); 

  /* perform the query.  if it doesn't work, log the error, close the
   * connection then return the error from the query processing.
   */
  if (!(conn->result = PQexec(conn->postgres, query)) ||
      ((PQresultStatus(conn->result) != PGRES_TUPLES_OK) &&
       (PQresultStatus(conn->result) != PGRES_COMMAND_OK))) {
    dmr = _build_error( cmd, conn );

    if (conn->result) PQclear(conn->result);

    close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
    cmd_close(close_cmd);
    SQL_FREE_CMD(close_cmd);

    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_select");
    return dmr;
  }

  /* get data if necessary. if it doesn't work, log the error, close the
   * connection then return the error from the data processing.
   */

  if ( PQresultStatus( conn->result ) == PGRES_TUPLES_OK ) {
    dmr = _build_data( cmd, conn );

    PQclear(conn->result);

    if (MODRET_ERROR(dmr)) {
      sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_query");
    }
  } else {
    dmr = HANDLED(cmd);
  }

  /* close the connection, return the data. */
  close_cmd = _sql_make_cmd( cmd->tmp_pool, 1, entry->name );
  cmd_close(close_cmd);
  SQL_FREE_CMD(close_cmd);

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_query");
  return dmr;
}

/*
 * cmd_escapestring: certain strings sent to a database should be properly
 *  escaped -- for instance, quotes need to be escaped to insure that 
 *  a query string is properly formatted.  cmd_escapestring does whatever
 *  is necessary to escape the special characters in a string. 
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: string to escape
 *
 * Returns:
 *  this command CANNOT fail.  The return string is null-terminated and 
 *  stored in the data field of the modret_t structure.
 *
 * Notes:
 *  Different languages may escape different characters in different ways.
 *  A backend should handle this correctly, where possible.  If there is
 *  no client library function to do the string conversion, it is strongly
 *  recommended that the backend module writer do whatever is necessry (read
 *  the database documentation and figure it out) to do the conversion
 *  themselves in this function.
 *
 *  A backend MUST supply a working escapestring implementation.  Simply
 *  copying the data from argv[0] into the data field of the modret allows
 *  for possible SQL injection attacks when this backend is used.
 */
MODRET cmd_escapestring(cmd_rec * cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;
  char *unescaped = NULL;
  char *escaped = NULL;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_escapestring");

  _sql_check_cmd(cmd, "cmd_escapestring");

  if (cmd->argc != 2) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_escapestring");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_escapestring");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  /* Note: the PQescapeString() function appeared in the C API as of
   * Postgres-7.2.
   */
  unescaped = cmd->argv[1];
  escaped = (char *) pcalloc(cmd->tmp_pool, sizeof(char) *
    (strlen(unescaped) * 2) + 1);

  PQescapeString(escaped, unescaped, strlen(unescaped));

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_escapestring");
  return mod_create_data(cmd, (void *) escaped);
}

/*
 * cmd_checkauth: some backend databases may provide backend-specific
 *  methods to check passwords.  This function takes a cleartext password
 *  and a hashed password and checks to see if they are the same.
 *
 * Inputs:
 *  cmd->argv[0]: connection name
 *  cmd->argv[1]: cleartext string
 *  cmd->argv[2]: hashed string
 *
 * Returns:
 *  HANDLED(cmd)                        -- passwords match
 *  ERROR_INT(cmd, PR_AUTH_NOPWD)       -- missing password
 *  ERROR_INT(cmd, PR_AUTH_BADPWD)      -- passwords don't match
 *  ERROR_INT(cmd, PR_AUTH_DISABLEPWD)  -- password is disabled
 *  ERROR_INT(cmd, PR_AUTH_AGEPWD)      -- password is aged
 *  ERROR(cmd)                          -- unknown error
 *
 * Notes:
 *  If this backend does not provide this functionality, this cmd *must*
 *  return ERROR.
 */
MODRET cmd_checkauth(cmd_rec * cmd) {
  conn_entry_t *entry = NULL;
  db_conn_t *conn = NULL;

  sql_log(DEBUG_FUNC, "%s", "entering \tpostgres cmd_checkauth");

  _sql_check_cmd(cmd, "cmd_checkauth");

  if (cmd->argc != 3) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_checkauth");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "badly formed request");
  }

  /* get the named connection -- not used in this case, but for consistency */
  entry = _sql_get_connection(cmd->argv[0]);
  if (!entry) {
    sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_checkauth");
    return ERROR_MSG(cmd, MOD_SQL_POSTGRES_VERSION, "unknown named connection");
  }

  conn = (db_conn_t *) entry->data;

  sql_log(DEBUG_FUNC, "%s", "exiting \tpostgres cmd_checkauth");

  /* PostgreSQL doesn't provide this functionality */

  return ERROR(cmd);
}

/*
 * cmd_identify: returns API information and an identification string for 
 *  the backend handler.  mod_sql will call this at initialization and 
 *  display the identification string.  The API version information is 
 *  used by mod_sql to identify available command handlers.
 *
 * Inputs:
 *  None.  The cmd->tmp_pool can be used to construct the return data, but
 *  do not depend on any other portion of the cmd_rec to be useful in any way.
 *
 * Returns:
 *  A sql_data_t of *exactly* this form:
 *   sql_data_t->rnum    = 1;
 *   sql_data_t->fnum    = 2;
 *   sql_data_t->data[0] = "identification string"
 *   sql_data_t->data[0] = "API version"
 *
 * Notes:
 *  See mod_sql.h for currently accepted APIs.
 */
MODRET cmd_identify(cmd_rec * cmd) {
  sql_data_t *sd = NULL;

  _sql_check_cmd(cmd, "cmd_identify");

  sd = (sql_data_t *) pcalloc( cmd->tmp_pool, sizeof(sql_data_t));
  sd->data = (char **) pcalloc( cmd->tmp_pool, sizeof(char *) * 2);

  sd->rnum = 1;
  sd->fnum = 2;

  sd->data[0] = MOD_SQL_POSTGRES_VERSION;
  sd->data[1] = MOD_SQL_API_V1;

  return mod_create_data(cmd, (void *) sd);
}  

/* SQL cmdtable: mod_sql requires each backend module to define a cmdtable
 *  with this exact name. ALL these functions must be defined; mod_sql checks
 *  that they all exist on startup and ProFTPD will refuse to start if they
 *  aren't defined.
 */
static cmdtable sql_postgres_cmdtable[] = {
  { CMD, "sql_open",             G_NONE, cmd_open,             FALSE, FALSE },
  { CMD, "sql_close",            G_NONE, cmd_close,            FALSE, FALSE },
  { CMD, "sql_defineconnection", G_NONE, cmd_defineconnection, FALSE, FALSE },
  { CMD, "sql_exit",             G_NONE, cmd_exit,             FALSE, FALSE },
  { CMD, "sql_select",           G_NONE, cmd_select,           FALSE, FALSE },
  { CMD, "sql_insert",           G_NONE, cmd_insert,           FALSE, FALSE },
  { CMD, "sql_update",           G_NONE, cmd_update,           FALSE, FALSE },
  { CMD, "sql_procedure",        G_NONE, cmd_procedure,        FALSE, FALSE },
  { CMD, "sql_query",            G_NONE, cmd_query,            FALSE, FALSE },
  { CMD, "sql_escapestring",     G_NONE, cmd_escapestring,     FALSE, FALSE },
  { CMD, "sql_checkauth",        G_NONE, cmd_checkauth,        FALSE, FALSE },
  { CMD, "sql_identify",         G_NONE, cmd_identify,         FALSE, FALSE },

  { 0, NULL }
};

/* Event handlers
 *

static void sql_postgres_mod_load_ev(const void *event_data,
    void *user_data) {

  if (strcmp("mod_sql_postgres.c", (const char *) event_data) == 0) {
    /* Register ourselves with mod_sql. */
    if (sql_register_backend("postgres", sql_postgres_cmdtable) < 0) {
      pr_log_pri(PR_LOG_NOTICE, MOD_SQL_POSTGRES_VERSION
        ": notice: error registering backend: %s", strerror(errno));
      end_login(1);
    }
  }
}

static void sql_postgres_mod_unload_ev(const void *event_data,
    void *user_data) {

  if (strcmp("mod_sql_postgres.c", (const char *) event_data) == 0) {
    /* Unegister ourselves with mod_sql. */
    if (sql_unregister_backend("postgres") < 0) {
      pr_log_pri(PR_LOG_NOTICE, MOD_SQL_POSTGRES_VERSION
        ": notice: error unregistering backend: %s", strerror(errno));
      end_login(1);
    }

    /* Unregister ourselves from all events. */
    pr_event_unregister(&sql_postgres_module, NULL, NULL);
  }
}

/* Initialization routines
 */

static int sql_postgres_init(void) {

  /* Register listeners for the load and unload events. */
  pr_event_register(&sql_postgres_module, "core.module-load",
    sql_postgres_mod_load_ev, NULL);
  pr_event_register(&sql_postgres_module, "core.module-unload",
    sql_postgres_mod_unload_ev, NULL);

  return 0;
}

static int sql_postgres_sess_init(void) {
  conn_pool = make_sub_pool(session.pool);
  conn_cache = make_array(make_sub_pool(session.pool), DEF_CONN_POOL_SIZE,
    sizeof(conn_entry_t));

  return 0;
}

/*
 * sql_postgres_module: The standard module struct for all ProFTPD modules.
 *  We use the pre-fork handler to initialize the conn_cache array header.
 *  Other backend modules may not need any init functions, or may need
 *  to extend the init functions to initialize other internal variables.
 */
module sql_postgres_module = {
  /* Always NULL */
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "sql_postgres",

  /* Module configuration directive handlers */
  NULL,

  /* Module command handlers */
  NULL,

  /* Module authentication handlers */
  NULL,

  /* Module initialization */
  sql_postgres_init,

  /* Session initialization */
  sql_postgres_sess_init,

  /* Module version */
  MOD_SQL_POSTGRES_VERSION
};

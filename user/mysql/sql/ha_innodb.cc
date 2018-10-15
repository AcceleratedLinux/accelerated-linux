/* Copyright (C) 2000 MySQL AB & Innobase Oy

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

/* This file defines the InnoDB handler: the interface between MySQL and
InnoDB */

/* TODO list for the InnoDB handler:
  - Ask Monty if strings of different languages can exist in the same
    database. Answer: in 4.1 yes.
*/

#ifdef __GNUC__
#pragma implementation				// gcc: Class implementation
#endif

#include "mysql_priv.h"
#include "slave.h"

#ifdef HAVE_INNOBASE_DB
#include <m_ctype.h>
#include <assert.h>
#include <hash.h>
#include <myisampack.h>

#define MAX_ULONG_BIT ((ulong) 1 << (sizeof(ulong)*8-1))

#include "ha_innodb.h"

pthread_mutex_t innobase_mutex;

/* Store MySQL definition of 'byte': in Linux it is char while InnoDB
uses unsigned char; the header univ.i which we include next defines
'byte' as a macro which expands to 'unsigned char' */

typedef byte	mysql_byte;

#define INSIDE_HA_INNOBASE_CC

/* Include necessary InnoDB headers */
extern "C" {
#include "../innobase/include/univ.i"
#include "../innobase/include/os0file.h"
#include "../innobase/include/os0thread.h"
#include "../innobase/include/srv0start.h"
#include "../innobase/include/srv0srv.h"
#include "../innobase/include/trx0roll.h"
#include "../innobase/include/trx0trx.h"
#include "../innobase/include/trx0sys.h"
#include "../innobase/include/row0ins.h"
#include "../innobase/include/row0mysql.h"
#include "../innobase/include/row0sel.h"
#include "../innobase/include/row0upd.h"
#include "../innobase/include/log0log.h"
#include "../innobase/include/lock0lock.h"
#include "../innobase/include/dict0crea.h"
#include "../innobase/include/btr0cur.h"
#include "../innobase/include/btr0btr.h"
#include "../innobase/include/fsp0fsp.h"
}

#define HA_INNOBASE_ROWS_IN_TABLE 10000 /* to get optimization right */
#define HA_INNOBASE_RANGE_COUNT	  100

bool 	innodb_skip 		= 0;
uint 	innobase_init_flags 	= 0;
ulong 	innobase_cache_size 	= 0;

/* The default values for the following, type long, start-up parameters
are declared in mysqld.cc: */

long innobase_mirrored_log_groups, innobase_log_files_in_group,
     innobase_log_file_size, innobase_log_buffer_size,
     innobase_buffer_pool_size, innobase_additional_mem_pool_size,
     innobase_file_io_threads, innobase_lock_wait_timeout,
     innobase_thread_concurrency, innobase_force_recovery;

/* The default values for the following char* start-up parameters
are determined in innobase_init below: */
  
char*	innobase_data_home_dir			= NULL;
char*	innobase_data_file_path 		= NULL;
char*	innobase_log_group_home_dir		= NULL;
char*	innobase_log_arch_dir			= NULL;
/* The following has a misleading name: starting from 4.0.5, this also
affects Windows: */
char*	innobase_unix_file_flush_method		= NULL;

/* Below we have boolean-valued start-up parameters, and their default
values */

uint	innobase_flush_log_at_trx_commit	= 1;
my_bool innobase_log_archive			= FALSE;
my_bool	innobase_use_native_aio			= FALSE;
my_bool	innobase_fast_shutdown			= TRUE;

static char *internal_innobase_data_file_path	= NULL;

/* The following counter is used to convey information to InnoDB
about server activity: in selects it is not sensible to call
srv_active_wake_master_thread after each fetch or search, we only do
it every INNOBASE_WAKE_INTERVAL'th step. */

#define INNOBASE_WAKE_INTERVAL	32
ulong	innobase_active_counter	= 0;

char*	innobase_home 	= NULL;

char    innodb_dummy_stmt_trx_handle = 'D';

static HASH 	innobase_open_tables;

static mysql_byte* innobase_get_key(INNOBASE_SHARE *share,uint *length,
			      my_bool not_used __attribute__((unused)));
static INNOBASE_SHARE *get_share(const char *table_name);
static void free_share(INNOBASE_SHARE *share);
static void innobase_print_error(const char* db_errpfx, char* buffer);

/* General functions */

/**********************************************************************
Save some CPU by testing the value of srv_thread_concurrency in inline
functions. */
inline
void
innodb_srv_conc_enter_innodb(
/*=========================*/
	trx_t*	trx)	/* in: transaction handle */
{
	if (srv_thread_concurrency >= 500) {

		return;
	}

	srv_conc_enter_innodb(trx);
}

/**********************************************************************
Save some CPU by testing the value of srv_thread_concurrency in inline
functions. */
inline
void
innodb_srv_conc_exit_innodb(
/*========================*/
	trx_t*	trx)	/* in: transaction handle */
{
	if (srv_thread_concurrency >= 500) {

		return;
	}

	srv_conc_exit_innodb(trx);
}

/**********************************************************************
Releases possible search latch and InnoDB thread FIFO ticket. These should
be released at each SQL statement end, and also when mysqld passes the
control to the client. It does no harm to release these also in the middle
of an SQL statement. */
inline
void
innobase_release_stat_resources(
/*============================*/
	trx_t*	trx)	/* in: transaction object */
{
	if (trx->has_search_latch) {
		trx_search_latch_release_if_reserved(trx);
	}

	if (trx->declared_to_be_inside_innodb) {
		/* Release our possible ticket in the FIFO */

		srv_conc_force_exit_innodb(trx);
	}
}

/************************************************************************
Call this function when mysqld passes control to the client. That is to
avoid deadlocks on the adaptive hash S-latch possibly held by thd. For more
documentation, see handler.cc. */

void
innobase_release_temporary_latches(
/*===============================*/
        void*   innobase_tid)
{
        innobase_release_stat_resources((trx_t*)innobase_tid);
}

/************************************************************************
Increments innobase_active_counter and every INNOBASE_WAKE_INTERVALth
time calls srv_active_wake_master_thread. This function should be used
when a single database operation may introduce a small need for
server utility activity, like checkpointing. */
inline
void
innobase_active_small(void)
/*=======================*/
{
	innobase_active_counter++;

	if ((innobase_active_counter % INNOBASE_WAKE_INTERVAL) == 0) {
		srv_active_wake_master_thread();
	}
}

/************************************************************************
Converts an InnoDB error code to a MySQL error code and also tells to MySQL
about a possible transaction rollback inside InnoDB caused by a lock wait
timeout or a deadlock. */
static
int
convert_error_code_to_mysql(
/*========================*/
			/* out: MySQL error code */
	int	error,	/* in: InnoDB error code */
	THD*	thd)	/* in: user thread handle or NULL */
{
	if (error == DB_SUCCESS) {

		return(0);

  	} else if (error == (int) DB_DUPLICATE_KEY) {

    		return(HA_ERR_FOUND_DUPP_KEY);

 	} else if (error == (int) DB_RECORD_NOT_FOUND) {

    		return(HA_ERR_NO_ACTIVE_RECORD);

 	} else if (error == (int) DB_ERROR) {

    		return(-1); /* unspecified error */

 	} else if (error == (int) DB_DEADLOCK) {
 		/* Since we rolled back the whole transaction, we must
 		tell it also to MySQL so that MySQL knows to empty the
 		cached binlog for this transaction */

 		if (thd) {
 			ha_rollback(thd);
 		}

    		return(HA_ERR_LOCK_DEADLOCK);

 	} else if (error == (int) DB_LOCK_WAIT_TIMEOUT) {

 		/* Since we rolled back the whole transaction, we must
 		tell it also to MySQL so that MySQL knows to empty the
 		cached binlog for this transaction */

 		if (thd) {
 			ha_rollback(thd);
 		}

    		return(HA_ERR_LOCK_WAIT_TIMEOUT);

 	} else if (error == (int) DB_NO_REFERENCED_ROW) {

    		return(HA_ERR_NO_REFERENCED_ROW);

 	} else if (error == (int) DB_ROW_IS_REFERENCED) {

    		return(HA_ERR_ROW_IS_REFERENCED);

        } else if (error == (int) DB_CANNOT_ADD_CONSTRAINT) {

    		return(HA_ERR_CANNOT_ADD_FOREIGN);

        } else if (error == (int) DB_CANNOT_DROP_CONSTRAINT) {

		return(HA_WRONG_CREATE_OPTION);

        } else if (error == (int) DB_COL_APPEARS_TWICE_IN_INDEX) {

    		return(HA_ERR_CRASHED);

 	} else if (error == (int) DB_OUT_OF_FILE_SPACE) {

    		return(HA_ERR_RECORD_FILE_FULL);

 	} else if (error == (int) DB_TABLE_IS_BEING_USED) {

    		return(HA_ERR_WRONG_COMMAND);

 	} else if (error == (int) DB_TABLE_NOT_FOUND) {

    		return(HA_ERR_KEY_NOT_FOUND);

  	} else if (error == (int) DB_TOO_BIG_RECORD) {

    		return(HA_ERR_TO_BIG_ROW);

  	} else if (error == (int) DB_CORRUPTION) {

    		return(HA_ERR_CRASHED);
  	} else if (error == (int) DB_NO_SAVEPOINT) {

    		return(HA_ERR_NO_SAVEPOINT);
    	} else {
    		return(-1);			// Unknown error
    	}
}

extern "C" {
/*****************************************************************
Prints info of a THD object (== user session thread) to the
standard output. NOTE that /mysql/innobase/trx/trx0trx.c must contain
the prototype for this function! */

void
innobase_mysql_print_thd(
/*=====================*/
	char*   buf,	/* in/out: buffer where to print, must be at least
			400 bytes */
        void*   input_thd)/* in: pointer to a MySQL THD object */
{
  	THD*    thd;
	char*   old_buf = buf;

        thd = (THD*) input_thd;

	/*  We cannot use the return value of normal sprintf() as this is
	not portable to some old non-Posix Unixes, e.g., some old SCO
	Unixes */

  	buf += my_sprintf(buf,
			 (buf, "MySQL thread id %lu, query id %lu",
			  thd->thread_id, thd->query_id));
    	if (thd->host) {
	        *buf = ' ';
		buf++;
	        buf = strnmov(buf, thd->host, 30);
  	}

  	if (thd->ip) {
	        *buf = ' ';
		buf++;
	        buf=strnmov(buf, thd->ip, 20);
  	}

  	if (thd->user) {
	        *buf = ' ';
		buf++;
	        buf=strnmov(buf, thd->user, 20);
  	}

  	if (thd->proc_info) {
	        *buf = ' ';
		buf++;
	        buf=strnmov(buf, thd->proc_info, 50);
  	}

  	if (thd->query) {
	        *buf = '\n';
		buf++;
	        buf=strnmov(buf, thd->query, 150);
  	}  

	buf[0] = '\n';
	buf[1] = '\0'; /* Note that we must put a null character here to end
		       the printed string */

	/* We test the printed length did not overrun the buffer length of
	400 bytes */

 	ut_a(strlen(old_buf) < 400);
}
}

/*************************************************************************
Gets the InnoDB transaction handle for a MySQL handler object, creates
an InnoDB transaction struct if the corresponding MySQL thread struct still
lacks one. */
static
trx_t*
check_trx_exists(
/*=============*/
			/* out: InnoDB transaction handle */
	THD*	thd)	/* in: user thread handle */
{
	trx_t*	trx;

	ut_a(thd == current_thd);

	trx = (trx_t*) thd->transaction.all.innobase_tid;

	if (trx == NULL) {
	        DBUG_ASSERT(thd != NULL);
		trx = trx_allocate_for_mysql();

		trx->mysql_thd = thd;
		trx->mysql_query_str = &((*thd).query);
		
		thd->transaction.all.innobase_tid = trx;

		/* The execution of a single SQL statement is denoted by
		a 'transaction' handle which is a dummy pointer: InnoDB
		remembers internally where the latest SQL statement
		started, and if error handling requires rolling back the
		latest statement, InnoDB does a rollback to a savepoint. */

		thd->transaction.stmt.innobase_tid =
		                  (void*)&innodb_dummy_stmt_trx_handle;
	} else {
		if (trx->magic_n != TRX_MAGIC_N) {
			mem_analyze_corruption((byte*)trx);

			ut_a(0);
		}
	}

	if (thd->options & OPTION_NO_FOREIGN_KEY_CHECKS) {
		trx->check_foreigns = FALSE;
	} else {
		trx->check_foreigns = TRUE;
	}

	if (thd->options & OPTION_RELAXED_UNIQUE_CHECKS) {
		trx->check_unique_secondary = FALSE;
	} else {
		trx->check_unique_secondary = TRUE;
	}

	return(trx);
}

/*************************************************************************
Updates the user_thd field in a handle and also allocates a new InnoDB
transaction handle if needed, and updates the transaction fields in the
prebuilt struct. */
inline
int
ha_innobase::update_thd(
/*====================*/
			/* out: 0 or error code */
	THD*	thd)	/* in: thd to use the handle */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	trx_t*		trx;
	
	trx = check_trx_exists(thd);

	if (prebuilt->trx != trx) {

		row_update_prebuilt_trx(prebuilt, trx);
	}

	user_thd = thd;

	return(0);
}


/*   BACKGROUND INFO: HOW THE MYSQL QUERY CACHE WORKS WITH INNODB
     ------------------------------------------------------------

1) The use of the query cache for TBL is disabled when there is an
uncommitted change to TBL.

2) When a change to TBL commits, InnoDB stores the current value of
its global trx id counter, let us denote it by INV_TRX_ID, to the table object
in the InnoDB data dictionary, and does only allow such transactions whose
id >= INV_TRX_ID to use the query cache.

3) When InnoDB does an INSERT/DELETE/UPDATE to a table TBL, or an implicit
modification because an ON DELETE CASCADE, we invalidate the MySQL query cache
of TBL immediately.

How this is implemented inside InnoDB:

1) Since every modification always sets an IX type table lock on the InnoDB
table, it is easy to check if there can be uncommitted modifications for a
table: just check if there are locks in the lock list of the table.

2) When a transaction inside InnoDB commits, it reads the global trx id
counter and stores the value INV_TRX_ID to the tables on which it had a lock.

3) If there is an implicit table change from ON DELETE CASCADE or SET NULL,
InnoDB calls an invalidate method for the MySQL query cache for that table.

How this is implemented inside sql_cache.cc:

1) The query cache for an InnoDB table TBL is invalidated immediately at an
INSERT/UPDATE/DELETE, just like in the case of MyISAM. No need to delay
invalidation to the transaction commit.

2) To store or retrieve a value from the query cache of an InnoDB table TBL,
any query must first ask InnoDB's permission. We must pass the thd as a
parameter because InnoDB will look at the trx id, if any, associated with
that thd.

3) Use of the query cache for InnoDB tables is now allowed also when
AUTOCOMMIT==0 or we are inside BEGIN ... COMMIT. Thus transactions no longer
put restrictions on the use of the query cache.
*/

/**********************************************************************
The MySQL query cache uses this to check from InnoDB if the query cache at
the moment is allowed to operate on an InnoDB table. The SQL query must
be a non-locking SELECT.

The query cache is allowed to operate on certain query only if this function
returns TRUE for all tables in the query.

If thd is not in the autocommit state, this function also starts a new
transaction for thd if there is no active trx yet, and assigns a consistent
read view to it if there is no read view yet. */

my_bool
innobase_query_caching_of_table_permitted(
/*======================================*/
				/* out: TRUE if permitted, FALSE if not;
				note that the value FALSE does not mean
				we should invalidate the query cache:
				invalidation is called explicitly */
	THD*	thd,		/* in: thd of the user who is trying to
				store a result to the query cache or
				retrieve it */
	char*	full_name,	/* in: concatenation of database name,
				the null character '\0', and the table
				name */
	uint	full_name_len)	/* in: length of the full name, i.e.
				len(dbname) + len(tablename) + 1 */
{
	ibool	is_autocommit;
	trx_t*	trx;
	char*	ptr;
	char	norm_name[1000];

	ut_a(full_name_len < 999);

	if (thd->variables.tx_isolation == ISO_SERIALIZABLE) {
		/* In the SERIALIZABLE mode we add LOCK IN SHARE MODE to every
		plain SELECT */
	
		return((my_bool)FALSE);
	}

	trx = (trx_t*) thd->transaction.all.innobase_tid;

	if (trx == NULL) {
		trx = check_trx_exists(thd);
	}

	innobase_release_stat_resources(trx);

	if (!(thd->options & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN))) {

		is_autocommit = TRUE;
	} else {
		is_autocommit = FALSE;

	}

	if (is_autocommit && trx->conc_state == TRX_NOT_STARTED) {
		/* We are going to retrieve the query result from the
		query cache. This cannot be a store operation because then
		we would have started the trx already.

		We can imagine we instantaneously serialize
		this consistent read trx to the current trx id counter.
		If trx2 would have changed the tables of a query
		result stored in the cache, and trx2 would have already
		committed, making the result obsolete, then trx2 would have
		already invalidated the cache. Thus we can trust the result
		in the cache is ok for this query. */

		return((my_bool)TRUE);
	}
	
	/* Normalize the table name to InnoDB format */

	memcpy(norm_name, full_name, full_name_len);

	norm_name[strlen(norm_name)] = '/'; /* InnoDB uses '/' as the
					    separator between db and table */
	norm_name[full_name_len] = '\0';
#ifdef __WIN__
	/* Put to lower case */

	ptr = norm_name;

	while (*ptr != '\0') {
	        *ptr = tolower(*ptr);
	        ptr++;
	}
#endif
	if (row_search_check_if_query_cache_permitted(trx, norm_name)) {

		printf("Query cache for %s permitted\n", norm_name);

		return((my_bool)TRUE);
	}

	printf("Query cache for %s NOT permitted\n", norm_name);

	return((my_bool)FALSE);
}

extern "C" {
/*********************************************************************
Invalidates the MySQL query cache for the table.
NOTE that the exact prototype of this function has to be in
/innobase/row/row0ins.c! */

void
innobase_invalidate_query_cache(
/*============================*/
	trx_t*	trx,		/* in: transaction which modifies the table */
	char*	full_name,	/* in: concatenation of database name, null
				char '\0', table name, null char'\0';
				NOTE that in Windows this is always
				in LOWER CASE! */
	ulint	full_name_len)	/* in: full name length where also the null
				chars count */
{
	/* Argument TRUE below means we are using transactions */
#ifdef HAVE_QUERY_CACHE
	query_cache.invalidate((THD*)(trx->mysql_thd),
					(const char*)full_name,
					(uint32)full_name_len,
					TRUE);
#endif
}
}

/*********************************************************************
Call this when you have opened a new table handle in HANDLER, before you
call index_read_idx() etc. Actually, we can let the cursor stay open even
over a transaction commit! Then you should call this before every operation,
fetch next etc. This function inits the necessary things even after a
transaction commit. */

void
ha_innobase::init_table_handle_for_HANDLER(void)
/*============================================*/
{
        row_prebuilt_t* prebuilt;

        /* If current thd does not yet have a trx struct, create one.
        If the current handle does not yet have a prebuilt struct, create
        one. Update the trx pointers in the prebuilt struct. Normally
        this operation is done in external_lock. */

        update_thd(current_thd);

        /* Initialize the prebuilt struct much like it would be inited in
        external_lock */

        prebuilt = (row_prebuilt_t*)innobase_prebuilt;

	innobase_release_stat_resources(prebuilt->trx);

        /* If the transaction is not started yet, start it */

        trx_start_if_not_started_noninline(prebuilt->trx);

        /* Assign a read view if the transaction does not have it yet */

        trx_assign_read_view(prebuilt->trx);

        /* We did the necessary inits in this function, no need to repeat them
        in row_search_for_mysql */

        prebuilt->sql_stat_start = FALSE;

        /* We let HANDLER always to do the reads as consistent reads, even
        if the trx isolation level would have been specified as SERIALIZABLE */

        prebuilt->select_lock_type = LOCK_NONE;

        /* Always fetch all columns in the index record */

        prebuilt->hint_no_need_to_fetch_extra_cols = FALSE;

        /* We want always to fetch all columns in the whole row? Or do
	we???? */

        prebuilt->read_just_key = FALSE;

	prebuilt->used_in_HANDLER = TRUE;
}

/*************************************************************************
Opens an InnoDB database. */

bool
innobase_init(void)
/*===============*/
			/* out: TRUE if error */
{
	static char current_dir[3];		// Set if using current lib
	int		err;
	bool		ret;
	char 	        *default_path;

  	DBUG_ENTER("innobase_init");

  	os_innodb_umask = (ulint)my_umask;

	/* First calculate the default path for innodb_data_home_dir etc.,
	in case the user has not given any value.

	Note that when using the embedded server, the datadirectory is not
	necessarily the current directory of this program. */

	if (mysql_embedded) {
		default_path = mysql_real_data_home;
	} else {
	  	/* It's better to use current lib, to keep paths short */
	  	current_dir[0] = FN_CURLIB;
	  	current_dir[1] = FN_LIBCHAR;
	  	current_dir[2] = 0;
	  	default_path = current_dir;
	}

	ut_a(default_path);

	if (specialflag & SPECIAL_NO_PRIOR) {
	        srv_set_thread_priorities = FALSE;
	} else {
	        srv_set_thread_priorities = TRUE;
	        srv_query_thread_priority = QUERY_PRIOR;
	}
	
	/* Set InnoDB initialization parameters according to the values
	read from MySQL .cnf file */

	/*--------------- Data files -------------------------*/

	/* The default dir for data files is the datadir of MySQL */

	srv_data_home = (innobase_data_home_dir ? innobase_data_home_dir :
			 default_path);

	/* Set default InnoDB data file size to 10 MB and let it be
  	auto-extending. Thus users can use InnoDB in >= 4.0 without having
	to specify any startup options. */

	if (!innobase_data_file_path) {
  		innobase_data_file_path = (char*) "ibdata1:10M:autoextend";
	}

	/* Since InnoDB edits the argument in the next call, we make another
	copy of it: */

	internal_innobase_data_file_path = my_strdup(innobase_data_file_path,
						   MYF(MY_WME));

	ret = (bool) srv_parse_data_file_paths_and_sizes(
				internal_innobase_data_file_path,
				&srv_data_file_names,
				&srv_data_file_sizes,
				&srv_data_file_is_raw_partition,
				&srv_n_data_files,
				&srv_auto_extend_last_data_file,
				&srv_last_file_size_max);
	if (ret == FALSE) {
	  	sql_print_error(
			"InnoDB: syntax error in innodb_data_file_path");
	  	DBUG_RETURN(TRUE);
	}

	/* -------------- Log files ---------------------------*/

	/* The default dir for log files is the datadir of MySQL */
	
	if (!innobase_log_group_home_dir) {
	  	innobase_log_group_home_dir = default_path;
	}
	  	
	/* Since innodb_log_arch_dir has no relevance under MySQL,
	starting from 4.0.6 we always set it the same as
	innodb_log_group_home_dir: */

	innobase_log_arch_dir = innobase_log_group_home_dir;

	srv_arch_dir = innobase_log_arch_dir;

	ret = (bool)
		srv_parse_log_group_home_dirs(innobase_log_group_home_dir,
						&srv_log_group_home_dirs);

	if (ret == FALSE || innobase_mirrored_log_groups != 1) {
		fprintf(stderr,
		"InnoDB: syntax error in innodb_log_group_home_dir\n"
		"InnoDB: or a wrong number of mirrored log groups\n");

		DBUG_RETURN(TRUE);
	}

	/* --------------------------------------------------*/

	srv_file_flush_method_str = innobase_unix_file_flush_method;

	srv_n_log_groups = (ulint) innobase_mirrored_log_groups;
	srv_n_log_files = (ulint) innobase_log_files_in_group;
	srv_log_file_size = (ulint) innobase_log_file_size;

	srv_log_archive_on = (ulint) innobase_log_archive;
	srv_log_buffer_size = (ulint) innobase_log_buffer_size;
	srv_flush_log_at_trx_commit = (ulint) innobase_flush_log_at_trx_commit;

	srv_pool_size = (ulint) innobase_buffer_pool_size;

	srv_mem_pool_size = (ulint) innobase_additional_mem_pool_size;

	srv_n_file_io_threads = (ulint) innobase_file_io_threads;

	srv_lock_wait_timeout = (ulint) innobase_lock_wait_timeout;
	srv_thread_concurrency = (ulint) innobase_thread_concurrency;
	srv_force_recovery = (ulint) innobase_force_recovery;

	srv_fast_shutdown = (ibool) innobase_fast_shutdown;

	srv_print_verbose_log = mysql_embedded ? 0 : 1;

	if (strcmp(default_charset_info->name, "latin1") == 0) {

		/* Store the character ordering table to InnoDB.
		For non-latin1 charsets we use the MySQL comparison
		functions, and consequently we do not need to know
		the ordering internally in InnoDB. */

		memcpy(srv_latin1_ordering,
				default_charset_info->sort_order, 256);
	}

	/* Since we in this module access directly the fields of a trx
        struct, and due to different headers and flags it might happen that
	mutex_t has a different size in this module and in InnoDB
	modules, we check at run time that the size is the same in
	these compilation modules. */

	srv_sizeof_trx_t_in_ha_innodb_cc = sizeof(trx_t);

	err = innobase_start_or_create_for_mysql();

	if (err != DB_SUCCESS) {

		DBUG_RETURN(1);
	}

	(void) hash_init(&innobase_open_tables,32,0,0,
			 		(hash_get_key) innobase_get_key,0,0);
	pthread_mutex_init(&innobase_mutex,MY_MUTEX_INIT_FAST);

	/* If this is a replication slave and we needed to do a crash recovery,
	set the master binlog position to what InnoDB internally knew about
	how far we got transactions durable inside InnoDB. There is a
	problem here: if the user used also MyISAM tables, InnoDB might not
	know the right position for them.

	THIS DOES NOT WORK CURRENTLY because replication seems to initialize
	glob_mi also after innobase_init. */
	
/*	if (trx_sys_mysql_master_log_pos != -1) {
		ut_memcpy(glob_mi.log_file_name, trx_sys_mysql_master_log_name,
				1 + ut_strlen(trx_sys_mysql_master_log_name));
		glob_mi.pos = trx_sys_mysql_master_log_pos;
	}
*/
  	DBUG_RETURN(0);
}

/***********************************************************************
Closes an InnoDB database. */

bool
innobase_end(void)
/*==============*/
				/* out: TRUE if error */
{
	int	err;

	DBUG_ENTER("innobase_end");

	err = innobase_shutdown_for_mysql();
	hash_free(&innobase_open_tables);
	my_free(internal_innobase_data_file_path,MYF(MY_ALLOW_ZERO_PTR));
	pthread_mutex_destroy(&innobase_mutex);

	if (err != DB_SUCCESS) {

	  DBUG_RETURN(1);
	}

  	DBUG_RETURN(0);
}

/********************************************************************
Flushes InnoDB logs to disk and makes a checkpoint. Really, a commit
flushes logs, and the name of this function should be innobase_checkpoint. */

bool
innobase_flush_logs(void)
/*=====================*/
				/* out: TRUE if error */
{
  	bool 	result = 0;

  	DBUG_ENTER("innobase_flush_logs");

	log_buffer_flush_to_disk();

  	DBUG_RETURN(result);
}

/*************************************************************************
Gets the free space in an InnoDB database: returned in units of kB. */

uint
innobase_get_free_space(void)
/*=========================*/
			/* out: free space in kB */
{
	return((uint) fsp_get_available_space_in_free_extents(0));
}

/*********************************************************************
Commits a transaction in an InnoDB database. */

void
innobase_commit_low(
/*================*/
	trx_t*	trx)	/* in: transaction handle */
{
	if (trx->conc_state == TRX_NOT_STARTED) {

	        return;
	}

        /* TODO: Guilhem should check if master_log_name, pending
        etc. are right if the master log gets rotated! Possible bug here.
	Comment by Heikki March 4, 2003. */

        if (current_thd->slave_thread) {
                /* Update the replication position info inside InnoDB */

                trx->mysql_master_log_file_name
                                        = active_mi->rli.master_log_name;
                trx->mysql_master_log_pos = ((ib_longlong)
					 (active_mi->rli.master_log_pos +
					  active_mi->rli.event_len +
					  active_mi->rli.pending));
        }

	trx_commit_for_mysql(trx);
}

/*********************************************************************
Commits a transaction in an InnoDB database or marks an SQL statement
ended. */

int
innobase_commit(
/*============*/
			/* out: 0 */
	THD*	thd,	/* in: MySQL thread handle of the user for whom
			the transaction should be committed */
	void*	trx_handle)/* in: InnoDB trx handle or
			&innodb_dummy_stmt_trx_handle: the latter means
			that the current SQL statement ended, and we should
			mark the start of a new statement with a savepoint */
{
	trx_t*	trx;

  	DBUG_ENTER("innobase_commit");
  	DBUG_PRINT("trans", ("ending transaction"));

	trx = check_trx_exists(thd);

	/* Release a possible FIFO ticket and search latch. Since we will
	reserve the kernel mutex, we have to release the search system latch
	first to obey the latching order. */

	innobase_release_stat_resources(trx);

	/* The flag thd->transaction.all.innodb_active_trans is set to 1 in
	::external_lock, ::start_stmt, and innobase_savepoint, and it is only
	set to 0 in a commit or a rollback. If it is 0 we know there cannot be
	resources to be freed and we could return immediately. For the time
	being we play safe and do the cleanup though there should be nothing
	to clean up. */

	if (thd->transaction.all.innodb_active_trans == 0
	    && trx->conc_state != TRX_NOT_STARTED) {
	    
	        fprintf(stderr,
"InnoDB: Error: thd->transaction.all.innodb_active_trans == 0\n"
"InnoDB: but trx->conc_state != TRX_NOT_STARTED\n");
	}

	if (trx_handle != (void*)&innodb_dummy_stmt_trx_handle
	    || (!(thd->options & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN)))) {
	        
		innobase_commit_low(trx);

		thd->transaction.all.innodb_active_trans = 0;
	} else {
		if (trx->auto_inc_lock) {
			/* If we had reserved the auto-inc lock for some
			table in this SQL statement we release it now */
		  	
			row_unlock_table_autoinc_for_mysql(trx);
		}
		/* Store the current undo_no of the transaction so that we
		know where to roll back if we have to roll back the next
		SQL statement */

		trx_mark_sql_stat_end(trx);
	}

	/* Tell the InnoDB server that there might be work for utility
	threads: */

	srv_active_wake_master_thread();

	DBUG_RETURN(0);
}

/*********************************************************************
This is called when MySQL writes the binlog entry for the current
transaction. Writes to the InnoDB tablespace info which tells where the
MySQL binlog entry for the current transaction ended. Also commits the
transaction inside InnoDB but does NOT flush InnoDB log files to disk.
To flush you have to call innobase_flush_log_to_disk. We have separated
flushing to eliminate the bottleneck of LOCK_log in log.cc which disabled
InnoDB's group commit capability. */

int
innobase_report_binlog_offset_and_commit(
/*=====================================*/
                                /* out: 0 */
        THD*    thd,            /* in: user thread */
        void*   trx_handle,     /* in: InnoDB trx handle */
        char*   log_file_name,  /* in: latest binlog file name */
        my_off_t end_offset)    /* in: the offset in the binlog file
                                   up to which we wrote */
{
	trx_t*	trx;

	trx = (trx_t*)trx_handle;

	ut_a(trx != NULL);

	trx->mysql_log_file_name = log_file_name;  	
	trx->mysql_log_offset = (ib_longlong)end_offset;
	
	trx->flush_log_later = TRUE;

  	innobase_commit(thd, trx_handle);

	trx->flush_log_later = FALSE;

	return(0);
}

/*********************************************************************
This is called after MySQL has written the binlog entry for the current
transaction. Flushes the InnoDB log files to disk if required. */

int
innobase_commit_complete(
/*=====================*/
                                /* out: 0 */
        void*   trx_handle)     /* in: InnoDB trx handle */
{
	trx_t*	trx;

	if (srv_flush_log_at_trx_commit == 0) {

	        return(0);
	}

	trx = (trx_t*)trx_handle;

	ut_a(trx != NULL);

  	trx_commit_complete_for_mysql(trx);

	return(0);
}

/*********************************************************************
Rolls back a transaction or the latest SQL statement. */

int
innobase_rollback(
/*==============*/
			/* out: 0 or error number */
	THD*	thd,	/* in: handle to the MySQL thread of the user
			whose transaction should be rolled back */
	void*	trx_handle)/* in: InnoDB trx handle or a dummy stmt handle;
			the latter means we roll back the latest SQL
			statement */
{
	int	error = 0;
	trx_t*	trx;

	DBUG_ENTER("innobase_rollback");
	DBUG_PRINT("trans", ("aborting transaction"));

	trx = check_trx_exists(thd);

	/* Release a possible FIFO ticket and search latch. Since we will
	reserve the kernel mutex, we have to release the search system latch
	first to obey the latching order. */

	innobase_release_stat_resources(trx);

        if (trx->auto_inc_lock) {
		/* If we had reserved the auto-inc lock for some table (if
		we come here to roll back the latest SQL statement) we
		release it now before a possibly lengthy rollback */
		
		row_unlock_table_autoinc_for_mysql(trx);
	}

	if (trx_handle != (void*)&innodb_dummy_stmt_trx_handle
	    || (!(thd->options & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN)))) {

		error = trx_rollback_for_mysql(trx);
		thd->transaction.all.innodb_active_trans = 0;
	} else {
		error = trx_rollback_last_sql_stat_for_mysql(trx);
	}

	DBUG_RETURN(convert_error_code_to_mysql(error, NULL));
}

/*********************************************************************
Rolls back a transaction to a savepoint. */

int
innobase_rollback_to_savepoint(
/*===========================*/
				/* out: 0 if success, HA_ERR_NO_SAVEPOINT if
				no savepoint with the given name */
	THD*	thd,		/* in: handle to the MySQL thread of the user
				whose transaction should be rolled back */
	char*	savepoint_name,	/* in: savepoint name */
	my_off_t* binlog_cache_pos)/* out: position which corresponds to the
				savepoint in the binlog cache of this
				transaction, not defined if error */
{
	ib_longlong mysql_binlog_cache_pos;
	int	    error = 0;
	trx_t*	    trx;

	DBUG_ENTER("innobase_rollback_to_savepoint");

	trx = check_trx_exists(thd);

	/* Release a possible FIFO ticket and search latch. Since we will
	reserve the kernel mutex, we have to release the search system latch
	first to obey the latching order. */

	innobase_release_stat_resources(trx);

	error = trx_rollback_to_savepoint_for_mysql(trx, savepoint_name,
						&mysql_binlog_cache_pos);

	*binlog_cache_pos = (my_off_t)mysql_binlog_cache_pos;

	DBUG_RETURN(convert_error_code_to_mysql(error, NULL));
}

/*********************************************************************
Sets a transaction savepoint. */

int
innobase_savepoint(
/*===============*/
				/* out: always 0, that is, always succeeds */
	THD*	thd,		/* in: handle to the MySQL thread */
	char*	savepoint_name,	/* in: savepoint name */
	my_off_t binlog_cache_pos)/* in: offset up to which the current
				transaction has cached log entries to its
				binlog cache, not defined if no transaction
				active, or we are in the autocommit state, or
				binlogging is not switched on */
{
	int	error = 0;
	trx_t*	trx;

	DBUG_ENTER("innobase_savepoint");

	if (!(thd->options & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN))) {
		/* In the autocommit state there is no sense to set a
		savepoint: we return immediate success */
	        DBUG_RETURN(0);
	}

	trx = check_trx_exists(thd);

	/* Release a possible FIFO ticket and search latch. Since we will
	reserve the kernel mutex, we have to release the search system latch
	first to obey the latching order. */

	innobase_release_stat_resources(trx);

	/* Setting a savepoint starts a transaction inside InnoDB since
	it allocates resources for it (memory to store the savepoint name,
	for example) */

	thd->transaction.all.innodb_active_trans = 1;

	error = trx_savepoint_for_mysql(trx, savepoint_name,
					     (ib_longlong)binlog_cache_pos);

	DBUG_RETURN(convert_error_code_to_mysql(error, NULL));
}

/*********************************************************************
Frees a possible InnoDB trx object associated with the current THD. */

int
innobase_close_connection(
/*======================*/
                        /* out: 0 or error number */
        THD*    thd)    /* in: handle to the MySQL thread of the user
                        whose transaction should be rolled back */
{
        trx_t*  trx;

        trx = (trx_t*)thd->transaction.all.innobase_tid;

        if (NULL != trx) {
                innobase_rollback(thd, (void*)trx);

                trx_free_for_mysql(trx);

                thd->transaction.all.innobase_tid = NULL;
        }

        return(0);
}

/**********************************************************************
Prints an error message. */
static
void
innobase_print_error(
/*=================*/
	const char*	db_errpfx,	/* in: error prefix text */
	char*		buffer)		/* in: error text */
{
  	sql_print_error("%s:  %s", db_errpfx, buffer);
}


/*****************************************************************************
** InnoDB database tables
*****************************************************************************/

/********************************************************************
This function is not relevant since we store the tables and indexes
into our own tablespace, not as files, whose extension this function would
give. */

const char**
ha_innobase::bas_ext() const
/*========================*/
				/* out: file extension strings, currently not
				used */
{
	static const char* ext[] = {".InnoDB", NullS};

	return(ext);
}

/*********************************************************************
Normalizes a table name string. A normalized name consists of the
database name catenated to '/' and table name. An example:
test/mytable. On Windows normalization puts both the database name and the
table name always to lower case. */
static
void
normalize_table_name(
/*=================*/
	char*		norm_name,	/* out: normalized name as a
					null-terminated string */
	const char*	name)		/* in: table name string */
{
	char*	name_ptr;
	char*	db_ptr;
	char*	ptr;

	/* Scan name from the end */

	ptr = strend(name)-1;

	while (ptr >= name && *ptr != '\\' && *ptr != '/') {
		ptr--;
	}

	name_ptr = ptr + 1;

	DBUG_ASSERT(ptr > name);

	ptr--;

	while (ptr >= name && *ptr != '\\' && *ptr != '/') {
		ptr--;
	}

	db_ptr = ptr + 1;

	memcpy(norm_name, db_ptr, strlen(name) + 1 - (db_ptr - name));

	norm_name[name_ptr - db_ptr - 1] = '/';

#ifdef __WIN__
	/* Put to lower case */

	ptr = norm_name;

	while (*ptr != '\0') {
	        *ptr = tolower(*ptr);
	        ptr++;
	}
#endif
}

/*********************************************************************
Creates and opens a handle to a table which already exists in an InnoDB
database. */

int
ha_innobase::open(
/*==============*/
					/* out: 1 if error, 0 if success */
	const char*	name,		/* in: table name */
	int 		mode,		/* in: not used */
	uint 		test_if_locked)	/* in: not used */
{
	dict_table_t*	ib_table;
  	int 		error	= 0;
  	char		norm_name[1000];

	DBUG_ENTER("ha_innobase::open");

	UT_NOT_USED(mode);
	UT_NOT_USED(test_if_locked);

	normalize_table_name(norm_name, name);

	user_thd = NULL;

	last_query_id = (ulong)-1;

	active_index = 0;
	active_index_before_scan = (uint)-1; /* undefined value */

	if (!(share=get_share(name)))
	  DBUG_RETURN(1);

	/* Create buffers for packing the fields of a record. Why
	table->reclength did not work here? Obviously, because char
	fields when packed actually became 1 byte longer, when we also
	stored the string length as the first byte. */

	upd_and_key_val_buff_len = table->reclength + table->max_key_length
							+ MAX_REF_PARTS * 3;
	if (!(mysql_byte*) my_multi_malloc(MYF(MY_WME),
				     &upd_buff, upd_and_key_val_buff_len,
				     &key_val_buff, upd_and_key_val_buff_len,
				     NullS)) {
	  	free_share(share);
	  	DBUG_RETURN(1);
  	}

	/* Get pointer to a table object in InnoDB dictionary cache */

	ib_table = dict_table_get_and_increment_handle_count(
				      		     norm_name, NULL);
 	if (NULL == ib_table) {
	        ut_print_timestamp(stderr);
	        fprintf(stderr, "  InnoDB error:\n"
"Cannot find table %s from the internal data dictionary\n"
"of InnoDB though the .frm file for the table exists. Maybe you\n"
"have deleted and recreated InnoDB data files but have forgotten\n"
"to delete the corresponding .frm files of InnoDB tables, or you\n"
"have moved .frm files to another database?\n"
"Look from section 15.1 of http://www.innodb.com/ibman.html\n"
"how you can resolve the problem.\n",
			  norm_name);

	        free_share(share);
    		my_free((char*) upd_buff, MYF(0));
    		my_errno = ENOENT;
    		DBUG_RETURN(1);
  	}

	innobase_prebuilt = row_create_prebuilt(ib_table);

	((row_prebuilt_t*)innobase_prebuilt)->mysql_row_len = table->reclength;

	/* Looks like MySQL-3.23 sometimes has primary key number != 0 */

 	primary_key = table->primary_key;
	key_used_on_scan = primary_key;

	/* Allocate a buffer for a 'row reference'. A row reference is
	a string of bytes of length ref_length which uniquely specifies
        a row in our table. Note that MySQL may also compare two row
        references for equality by doing a simple memcmp on the strings
        of length ref_length! */

  	if (!row_table_got_default_clust_index(ib_table)) {
	        if (primary_key >= MAX_KEY) {
	                fprintf(stderr,
		    "InnoDB: Error: table %s has a primary key in InnoDB\n"
		    "InnoDB: data dictionary, but not in MySQL!\n", name);
		}

		((row_prebuilt_t*)innobase_prebuilt)
				->clust_index_was_generated = FALSE;
 		/*
		  MySQL allocates the buffer for ref. key_info->key_length
		  includes space for all key columns + one byte for each column
		  that may be NULL. ref_length must be as exact as possible to
		  save space, because all row reference buffers are allocated
		  based on ref_length.
		*/
 
  		ref_length = table->key_info[primary_key].key_length;
	} else {
	        if (primary_key != MAX_KEY) {
	                fprintf(stderr,
		    "InnoDB: Error: table %s has no primary key in InnoDB\n"
		    "InnoDB: data dictionary, but has one in MySQL!\n"
		    "InnoDB: If you created the table with a MySQL\n"
                    "InnoDB: version < 3.23.54 and did not define a primary\n"
                    "InnoDB: key, but defined a unique key with all non-NULL\n"
                    "InnoDB: columns, then MySQL internally treats that key\n"
                    "InnoDB: as the primary key. You can fix this error by\n"
		    "InnoDB: dump + DROP + CREATE + reimport of the table.\n",
				name);
		}

		((row_prebuilt_t*)innobase_prebuilt)
				->clust_index_was_generated = TRUE;

  		ref_length = DATA_ROW_ID_LEN;

		/*
		  If we automatically created the clustered index, then
		  MySQL does not know about it, and MySQL must NOT be aware
		  of the index used on scan, to make it avoid checking if we
		  update the column of the index. That is why we assert below
		  that key_used_on_scan is the undefined value MAX_KEY.
		  The column is the row id in the automatical generation case,
		  and it will never be updated anyway.
		*/
	       
		if (key_used_on_scan != MAX_KEY) {
	                fprintf(stderr,
"InnoDB: Warning: table %s key_used_on_scan is %lu even though there is no\n"
"InnoDB: primary key inside InnoDB.\n",
				name, (ulint)key_used_on_scan);
		}
	}

	auto_inc_counter_for_this_stat = 0;

	block_size = 16 * 1024;	/* Index block size in InnoDB: used by MySQL
				in query optimization */

	/* Init table lock structure */
	thr_lock_data_init(&share->lock,&lock,(void*) 0);

  	info(HA_STATUS_NO_LOCK | HA_STATUS_VARIABLE | HA_STATUS_CONST);

  	DBUG_RETURN(0);
}

/*********************************************************************
Does nothing. */

void
ha_innobase::initialize(void)
/*=========================*/
{
}

/**********************************************************************
Closes a handle to an InnoDB table. */

int
ha_innobase::close(void)
/*====================*/
				/* out: error number */
{
  	DBUG_ENTER("ha_innobase::close");

	row_prebuilt_free((row_prebuilt_t*) innobase_prebuilt);

    	my_free((char*) upd_buff, MYF(0));
        free_share(share);

	/* Tell InnoDB server that there might be work for
	utility threads: */

	srv_active_wake_master_thread();

  	DBUG_RETURN(0);
}

/* The following accessor functions should really be inside MySQL code! */

/******************************************************************
Gets field offset for a field in a table. */
inline
uint
get_field_offset(
/*=============*/
			/* out: offset */
	TABLE*	table,	/* in: MySQL table object */
	Field*	field)	/* in: MySQL field object */
{
	return((uint) (field->ptr - (char*) table->record[0]));
}

/******************************************************************
Checks if a field in a record is SQL NULL. Uses the record format
information in table to track the null bit in record. */
inline
uint
field_in_record_is_null(
/*====================*/
			/* out: 1 if NULL, 0 otherwise */
	TABLE*	table,	/* in: MySQL table object */
	Field*	field,	/* in: MySQL field object */
	char*	record)	/* in: a row in MySQL format */
{
	int	null_offset;

	if (!field->null_ptr) {

		return(0);
	}

	null_offset = (uint) ((char*) field->null_ptr
					- (char*) table->record[0]);

	if (record[null_offset] & field->null_bit) {

		return(1);
	}

	return(0);
}

/******************************************************************
Sets a field in a record to SQL NULL. Uses the record format
information in table to track the null bit in record. */
inline
void
set_field_in_record_to_null(
/*========================*/
	TABLE*	table,	/* in: MySQL table object */
	Field*	field,	/* in: MySQL field object */
	char*	record)	/* in: a row in MySQL format */
{
	int	null_offset;

	null_offset = (uint) ((char*) field->null_ptr
					- (char*) table->record[0]);

	record[null_offset] = record[null_offset] | field->null_bit;
}

/******************************************************************
Resets SQL NULL bits in a record to zero. */
inline
void
reset_null_bits(
/*============*/
	TABLE*	table,	/* in: MySQL table object */
	char*	record)	/* in: a row in MySQL format */
{
	bzero(record, table->null_bytes);
}

extern "C" {
/*****************************************************************
InnoDB uses this function is to compare two data fields for which the
data type is such that we must use MySQL code to compare them. NOTE that the
prototype of this function is in rem0cmp.c in InnoDB source code!
If you change this function, remember to update the prototype there! */

int
innobase_mysql_cmp(
/*===============*/
					/* out: 1, 0, -1, if a is greater,
					equal, less than b, respectively */
	int		mysql_type,	/* in: MySQL type */
	unsigned char*	a,		/* in: data field */
	unsigned int	a_length,	/* in: data field length,
					not UNIV_SQL_NULL */
	unsigned char*	b,		/* in: data field */
	unsigned int	b_length)	/* in: data field length,
					not UNIV_SQL_NULL */
{
	enum_field_types	mysql_tp;
	int                     ret;

	DBUG_ASSERT(a_length != UNIV_SQL_NULL);
	DBUG_ASSERT(b_length != UNIV_SQL_NULL);

	mysql_tp = (enum_field_types) mysql_type;

	switch (mysql_tp) {

	case FIELD_TYPE_STRING:
	case FIELD_TYPE_VAR_STRING:
	case FIELD_TYPE_TINY_BLOB:
	case FIELD_TYPE_MEDIUM_BLOB:
	case FIELD_TYPE_BLOB:
	case FIELD_TYPE_LONG_BLOB:
  		ret = my_sortncmp((const char*) a, a_length,
				  (const char*) b, b_length);
		if (ret < 0) {
		        return(-1);
		} else if (ret > 0) {
		        return(1);
		} else {
		        return(0);
	        }
	default:
		assert(0);
	}

	return(0);
}
}

/******************************************************************
Converts a MySQL type to an InnoDB type. */
inline
ulint
get_innobase_type_from_mysql_type(
/*==============================*/
			/* out: DATA_BINARY, DATA_VARCHAR, ... */
	Field*	field)	/* in: MySQL field */
{
	/* The following asserts check that the MySQL type code fits in
	8 bits: this is used in ibuf and also when DATA_NOT_NULL is
	ORed to the type */

	DBUG_ASSERT((ulint)FIELD_TYPE_STRING < 256);
	DBUG_ASSERT((ulint)FIELD_TYPE_VAR_STRING < 256);
	DBUG_ASSERT((ulint)FIELD_TYPE_DOUBLE < 256);
	DBUG_ASSERT((ulint)FIELD_TYPE_FLOAT < 256);
	DBUG_ASSERT((ulint)FIELD_TYPE_DECIMAL < 256);

	switch (field->type()) {
	        /* NOTE that we only allow string types in DATA_MYSQL
		and DATA_VARMYSQL */
		case FIELD_TYPE_VAR_STRING: if (field->flags & BINARY_FLAG) {

						return(DATA_BINARY);
					} else if (strcmp(
						   default_charset_info->name,
							"latin1") == 0) {
						return(DATA_VARCHAR);
					} else {
						return(DATA_VARMYSQL);
					}
		case FIELD_TYPE_STRING: if (field->flags & BINARY_FLAG) {

						return(DATA_FIXBINARY);
					} else if (strcmp(
						   default_charset_info->name,
							"latin1") == 0) {
						return(DATA_CHAR);
					} else {
						return(DATA_MYSQL);
					}
		case FIELD_TYPE_LONG:
		case FIELD_TYPE_LONGLONG:
		case FIELD_TYPE_TINY:
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_INT24:
		case FIELD_TYPE_DATE:
		case FIELD_TYPE_DATETIME:
		case FIELD_TYPE_YEAR:
		case FIELD_TYPE_NEWDATE:
		case FIELD_TYPE_ENUM:
		case FIELD_TYPE_SET:
		case FIELD_TYPE_TIME:
		case FIELD_TYPE_TIMESTAMP:
					return(DATA_INT);
		case FIELD_TYPE_FLOAT:
					return(DATA_FLOAT);
		case FIELD_TYPE_DOUBLE:
					return(DATA_DOUBLE);
		case FIELD_TYPE_DECIMAL:
					return(DATA_DECIMAL);
		case FIELD_TYPE_TINY_BLOB:
		case FIELD_TYPE_MEDIUM_BLOB:
		case FIELD_TYPE_BLOB:
		case FIELD_TYPE_LONG_BLOB:
					return(DATA_BLOB);
		default:
					assert(0);
	}

	return(0);
}

/***********************************************************************
Stores a key value for a row to a buffer. */

uint
ha_innobase::store_key_val_for_row(
/*===============================*/
				/* out: key value length as stored in buff */
	uint 		keynr,	/* in: key number */
	char*		buff,	/* in/out: buffer for the key value (in MySQL
				format) */
	uint		buff_len,/* in: buffer length */
	const mysql_byte* record)/* in: row in MySQL format */
{
	KEY*		key_info 	= table->key_info + keynr;
  	KEY_PART_INFO*	key_part	= key_info->key_part;
  	KEY_PART_INFO*	end		= key_part + key_info->key_parts;
	char*		buff_start	= buff;
	enum_field_types mysql_type;
	Field*		field;
	ulint		blob_len;
	byte*		blob_data;
	ibool		is_null;

  	DBUG_ENTER("store_key_val_for_row");

	/* The format for storing a key field in MySQL is the following:

	1. If the column can be NULL, then in the first byte we put 1 if the
	field value is NULL, 0 otherwise.

	2. If the column is of a BLOB type (it must be a column prefix field
	in this case), then we put the length of the data in the field to the
	next 2 bytes, in the little-endian format. If the field is SQL NULL,
	then these 2 bytes are set to 0. Note that the length of data in the
	field is <= column prefix length.

	3. In a column prefix field, prefix_len next bytes are reserved for
	data. In a normal field the max field length next bytes are reserved
	for data. For a VARCHAR(n) the max field length is n. If the stored
	value is the SQL NULL then these data bytes are set to 0. */	

	/* We have to zero-fill the buffer so that MySQL is able to use a
	simple memcmp to compare two key values to determine if they are
	equal. MySQL does this to compare contents of two 'ref' values. */

	bzero(buff, buff_len);

  	for (; key_part != end; key_part++) {
	        is_null = FALSE;

    		if (key_part->null_bit) {
      			if (record[key_part->null_offset]
						& key_part->null_bit) {
				*buff = 1;
				is_null = TRUE;
      			} else {
				*buff = 0;
			}
			buff++;
    		}

		field = key_part->field;
		mysql_type = field->type();

		if (mysql_type == FIELD_TYPE_TINY_BLOB
		    || mysql_type == FIELD_TYPE_MEDIUM_BLOB
		    || mysql_type == FIELD_TYPE_BLOB
		    || mysql_type == FIELD_TYPE_LONG_BLOB) {

			ut_a(key_part->key_part_flag & HA_PART_KEY);

		        if (is_null) {
				 buff += key_part->length + 2;
				 
				 continue;
			}
		    
		        blob_data = row_mysql_read_blob_ref(&blob_len,
				(byte*) (record
				+ (ulint)get_field_offset(table, field)),
					(ulint) field->pack_length());

			ut_a(get_field_offset(table, field)
						     == key_part->offset);
			if (blob_len > key_part->length) {
			        blob_len = key_part->length;
			}

			/* MySQL reserves 2 bytes for the length and the
			storage of the number is little-endian */

			ut_a(blob_len < 256);
			*((byte*)buff) = (byte)blob_len;
			buff += 2;

			memcpy(buff, blob_data, blob_len);

			buff += key_part->length;
		} else {
		        if (is_null) {
				 buff += key_part->length;
				 
				 continue;
			}
			memcpy(buff, record + key_part->offset,
							key_part->length);
			buff += key_part->length;
		}
  	}

	ut_a(buff <= buff_start + buff_len);

	DBUG_RETURN((uint)(buff - buff_start));
}

/******************************************************************
Builds a template to the prebuilt struct. */
static
void
build_template(
/*===========*/
	row_prebuilt_t*	prebuilt,	/* in: prebuilt struct */
	THD*		thd,		/* in: current user thread, used
					only if templ_type is
					ROW_MYSQL_REC_FIELDS */
	TABLE*		table,		/* in: MySQL table */
	ulint		templ_type)	/* in: ROW_MYSQL_WHOLE_ROW or
					ROW_MYSQL_REC_FIELDS */
{
	dict_index_t*	index;
	dict_index_t*	clust_index;
	mysql_row_templ_t* templ;
	Field*		field;
	ulint		n_fields;
	ulint		n_requested_fields	= 0;
	ibool		fetch_all_in_key	= FALSE;
	ulint		i;

	clust_index = dict_table_get_first_index_noninline(prebuilt->table);

	if (!prebuilt->hint_no_need_to_fetch_extra_cols) {
		/* We have a hint that we should at least fetch all
		columns in the key, or all columns in the table */

		if (prebuilt->read_just_key) {
			/* MySQL has instructed us that it is enough to
			fetch the columns in the key; looks like MySQL
			can set this flag also when there is only a
			prefix of the column in the key: in that case we
			retrieve the whole column from the clustered
			index */

			fetch_all_in_key = TRUE;
		} else {
			/* We are building a temporary table: fetch all
 			columns; the reason is that MySQL may use the
			clustered index key to store rows, but the mechanism
			we use below to detect required columns does not
			reveal that. Actually, it might be enough to
			fetch only all in the key also in this case! */

			templ_type = ROW_MYSQL_WHOLE_ROW;
		}
	}

	if (prebuilt->select_lock_type == LOCK_X) {
		/* We always retrieve the whole clustered index record if we
		use exclusive row level locks, for example, if the read is
		done in an UPDATE statement. */

	        templ_type = ROW_MYSQL_WHOLE_ROW;
	}

	if (templ_type == ROW_MYSQL_REC_FIELDS) {
		/* In versions < 3.23.50 we always retrieved the clustered
		index record if prebuilt->select_lock_type == LOCK_S,
		but there is really not need for that, and in some cases
		performance could be seriously degraded because the MySQL
		optimizer did not know about our convention! */

		index = prebuilt->index;
	} else {
		index = clust_index;
	}

	if (index == clust_index) {
		prebuilt->need_to_access_clustered = TRUE;
	} else {
		prebuilt->need_to_access_clustered = FALSE;
		/* Below we check column by column if we need to access
		the clustered index */
	}

	n_fields = (ulint)table->fields;

	if (!prebuilt->mysql_template) {
		prebuilt->mysql_template = (mysql_row_templ_t*)
						mem_alloc_noninline(
					n_fields * sizeof(mysql_row_templ_t));
	}

	prebuilt->template_type = templ_type;
	prebuilt->null_bitmap_len = table->null_bytes;

	prebuilt->templ_contains_blob = FALSE;

	for (i = 0; i < n_fields; i++) {
		templ = prebuilt->mysql_template + n_requested_fields;
		field = table->field[i];

		if (templ_type == ROW_MYSQL_REC_FIELDS
			&& !(fetch_all_in_key
			     && dict_index_contains_col_or_prefix(index, i))
			&& thd->query_id != field->query_id
			&& thd->query_id != (field->query_id ^ MAX_ULONG_BIT)
			&& thd->query_id !=
				(field->query_id ^ (MAX_ULONG_BIT >> 1))) {

			/* This field is not needed in the query, skip it */

			goto skip_field;
		}

		n_requested_fields++;

		templ->col_no = i;

		if (index == clust_index) {
			templ->rec_field_no = (index->table->cols + i)
								->clust_pos;
		} else {
			templ->rec_field_no = dict_index_get_nth_col_pos(
								index, i);
		}

		if (templ->rec_field_no == ULINT_UNDEFINED) {
			prebuilt->need_to_access_clustered = TRUE;
		}

		if (field->null_ptr) {
			templ->mysql_null_byte_offset =
				(ulint) ((char*) field->null_ptr
					- (char*) table->record[0]);

			templ->mysql_null_bit_mask = (ulint) field->null_bit;
		} else {
			templ->mysql_null_bit_mask = 0;
		}

		templ->mysql_col_offset = (ulint)
					get_field_offset(table, field);

		templ->mysql_col_len = (ulint) field->pack_length();
		templ->type = get_innobase_type_from_mysql_type(field);
		templ->is_unsigned = (ulint) (field->flags & UNSIGNED_FLAG);

		if (templ->type == DATA_BLOB) {
			prebuilt->templ_contains_blob = TRUE;
		}
skip_field:
		;
	}

	prebuilt->n_template = n_requested_fields;

	if (prebuilt->need_to_access_clustered) {
		/* Change rec_field_no's to correspond to the clustered index
		record */
		for (i = 0; i < n_requested_fields; i++) {
			templ = prebuilt->mysql_template + i;

			templ->rec_field_no =
			    (index->table->cols + templ->col_no)->clust_pos;
		}
	}
}

/************************************************************************
Stores a row in an InnoDB database, to the table specified in this
handle. */

int
ha_innobase::write_row(
/*===================*/
				/* out: error code */
	mysql_byte* 	record)	/* in: a row in MySQL format */
{
	row_prebuilt_t* prebuilt = (row_prebuilt_t*)innobase_prebuilt;
  	int 		error;
	longlong	auto_inc;
	longlong	dummy;
	ibool           incremented_auto_inc_for_stat = FALSE;
	ibool           incremented_auto_inc_counter = FALSE;
	ibool           skip_auto_inc_decr;

  	DBUG_ENTER("ha_innobase::write_row");

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

  	statistic_increment(ha_write_count, &LOCK_status);

  	if (table->time_stamp) {
    		update_timestamp(record + table->time_stamp - 1);
    	}

	if (last_query_id != user_thd->query_id) {
	        prebuilt->sql_stat_start = TRUE;
                last_query_id = user_thd->query_id;

		innobase_release_stat_resources(prebuilt->trx);
	}

  	if (table->next_number_field && record == table->record[0]) {
		/* This is the case where the table has an
		auto-increment column */

		/* Initialize the auto-inc counter if it has not been
		initialized yet */

		if (0 == dict_table_autoinc_peek(prebuilt->table)) {

			/* This call initializes the counter */
		        error = innobase_read_and_init_auto_inc(&dummy);

			if (error) {
				/* Deadlock or lock wait timeout */

				goto func_exit;
			}

			/* We have to set sql_stat_start to TRUE because
			the above call probably has called a select, and
			has reset that flag; row_insert_for_mysql has to
			know to set the IX intention lock on the table,
			something it only does at the start of each
			statement */

			prebuilt->sql_stat_start = TRUE;
		}

	        /* Fetch the value the user possibly has set in the
	        autoincrement field */

	        auto_inc = table->next_number_field->val_int();

		/* In replication and also otherwise the auto-inc column 
		can be set with SET INSERT_ID. Then we must look at
		user_thd->next_insert_id. If it is nonzero and the user
		has not supplied a value, we must use it, and use values
		incremented by 1 in all subsequent inserts within the
		same SQL statement! */

		if (auto_inc == 0 && user_thd->next_insert_id != 0) {
		        auto_inc = user_thd->next_insert_id;
		        auto_inc_counter_for_this_stat = auto_inc;
		}

		if (auto_inc == 0 && auto_inc_counter_for_this_stat) {
			/* The user set the auto-inc counter for
			this SQL statement with SET INSERT_ID. We must
			assign sequential values from the counter. */

			auto_inc_counter_for_this_stat++;
			incremented_auto_inc_for_stat = TRUE;

			auto_inc = auto_inc_counter_for_this_stat;

			/* We give MySQL a new value to place in the
			auto-inc column */
			user_thd->next_insert_id = auto_inc;
		}

		if (auto_inc != 0) {
			/* This call will calculate the max of the current
			value and the value supplied by the user and
			update the counter accordingly */

			/* We have to use the transactional lock mechanism
			on the auto-inc counter of the table to ensure
			that replication and roll-forward of the binlog
			exactly imitates also the given auto-inc values.
			The lock is released at each SQL statement's
			end. */

			innodb_srv_conc_enter_innodb(prebuilt->trx);
			error = row_lock_table_autoinc_for_mysql(prebuilt);
			innodb_srv_conc_exit_innodb(prebuilt->trx);

			if (error != DB_SUCCESS) {

				error = convert_error_code_to_mysql(error,
								    user_thd);
				goto func_exit;
			}	

			dict_table_autoinc_update(prebuilt->table, auto_inc);
		} else {
			innodb_srv_conc_enter_innodb(prebuilt->trx);

			if (!prebuilt->trx->auto_inc_lock) {

				error = row_lock_table_autoinc_for_mysql(
								prebuilt);
				if (error != DB_SUCCESS) {
 					innodb_srv_conc_exit_innodb(
							prebuilt->trx);

					error = convert_error_code_to_mysql(
							error, user_thd);
					goto func_exit;
				}
			}	

			/* The following call gets the value of the auto-inc
			counter of the table and increments it by 1 */

			auto_inc = dict_table_autoinc_get(prebuilt->table);
			incremented_auto_inc_counter = TRUE;

			innodb_srv_conc_exit_innodb(prebuilt->trx);

			/* We can give the new value for MySQL to place in
			the field */

			user_thd->next_insert_id = auto_inc;
		}

		/* This call of a handler.cc function places
		user_thd->next_insert_id to the column value, if the column
		value was not set by the user */

    		update_auto_increment();
	}

	if (prebuilt->mysql_template == NULL
			|| prebuilt->template_type != ROW_MYSQL_WHOLE_ROW) {
		/* Build the template used in converting quickly between
		the two database formats */

		build_template(prebuilt, NULL, table, ROW_MYSQL_WHOLE_ROW);
	}

	innodb_srv_conc_enter_innodb(prebuilt->trx);

	error = row_insert_for_mysql((byte*) record, prebuilt);

	innodb_srv_conc_exit_innodb(prebuilt->trx);

	if (error != DB_SUCCESS) {
	        /* If the insert did not succeed we restore the value of
		the auto-inc counter we used; note that this behavior was
		introduced only in version 4.0.4.
		NOTE that a REPLACE command handles a duplicate key error
		itself, and we must not decrement the autoinc counter
		if we are performing a REPLACE statement.
		NOTE 2: if there was an error, for example a deadlock,
		which caused InnoDB to roll back the whole transaction
		already in the call of row_insert_for_mysql(), we may no
		longer have the AUTO-INC lock, and cannot decrement
		the counter here. */

	        skip_auto_inc_decr = FALSE;

	        if (error == DB_DUPLICATE_KEY
		    && (user_thd->lex.sql_command == SQLCOM_REPLACE
			|| user_thd->lex.sql_command
			                 == SQLCOM_REPLACE_SELECT)) {

		        skip_auto_inc_decr= TRUE;
		}

	        if (!skip_auto_inc_decr && incremented_auto_inc_counter
		    && prebuilt->trx->auto_inc_lock) {
	                dict_table_autoinc_decrement(prebuilt->table);
	        }

		if (!skip_auto_inc_decr && incremented_auto_inc_for_stat
		    && prebuilt->trx->auto_inc_lock) {
		        auto_inc_counter_for_this_stat--;
		}
	}

	error = convert_error_code_to_mysql(error, user_thd);

	/* Tell InnoDB server that there might be work for
	utility threads: */
func_exit:
	innobase_active_small();

  	DBUG_RETURN(error);
}

/******************************************************************
Converts field data for storage in an InnoDB update vector. */
inline
mysql_byte*
innobase_convert_and_store_changed_col(
/*===================================*/
				/* out: pointer to the end of the converted
				data in the buffer */
	upd_field_t*	ufield,	/* in/out: field in the update vector */
	mysql_byte*	buf,	/* in: buffer we can use in conversion */
	mysql_byte*	data,	/* in: column data to store */
	ulint		len,	/* in: data len */
	ulint		col_type,/* in: data type in InnoDB type numbers */
	ulint		is_unsigned)/* in: != 0 if an unsigned integer type */
{
	uint	i;

	if (len == UNIV_SQL_NULL) {
		data = NULL;
	} else if (col_type == DATA_VARCHAR || col_type == DATA_BINARY
		   || col_type == DATA_VARMYSQL) {
	        /* Remove trailing spaces */
        	while (len > 0 && data[len - 1] == ' ') {
	                len--;
	        }
	} else if (col_type == DATA_INT) {
		/* Store integer data in InnoDB in a big-endian
		format, sign bit negated, if signed */

		for (i = 0; i < len; i++) {
			buf[len - 1 - i] = data[i];
		}

		if (!is_unsigned) {
			buf[0] = buf[0] ^ 128;
		}

		data = buf;

		buf += len;
	}

	ufield->new_val.data = data;
	ufield->new_val.len = len;

	return(buf);
}

/**************************************************************************
Checks which fields have changed in a row and stores information
of them to an update vector. */
static
int
calc_row_difference(
/*================*/
					/* out: error number or 0 */
	upd_t*		uvect,		/* in/out: update vector */
	mysql_byte* 	old_row,	/* in: old row in MySQL format */
	mysql_byte* 	new_row,	/* in: new row in MySQL format */
	struct st_table* table,		/* in: table in MySQL data
					dictionary */
	mysql_byte*	upd_buff,	/* in: buffer to use */
	ulint		buff_len,	/* in: buffer length */
	row_prebuilt_t*	prebuilt,	/* in: InnoDB prebuilt struct */
	THD*		thd)		/* in: user thread */
{
	mysql_byte*	original_upd_buff = upd_buff;
	Field*		field;
	uint		n_fields;
	ulint		o_len;
	ulint		n_len;
	byte*	        o_ptr;
        byte*	        n_ptr;
        byte*	        buf;
	upd_field_t*	ufield;
	ulint		col_type;
	ulint		is_unsigned;
	ulint		n_changed = 0;
	uint		i;

	n_fields = table->fields;

	/* We use upd_buff to convert changed fields */
	buf = (byte*) upd_buff;

	for (i = 0; i < n_fields; i++) {
		field = table->field[i];

		/* if (thd->query_id != field->query_id) { */
			/* TODO: check that these fields cannot have
			changed! */

		/*	goto skip_field;
		}*/

		o_ptr = (byte*) old_row + get_field_offset(table, field);
		n_ptr = (byte*) new_row + get_field_offset(table, field);
		o_len = field->pack_length();
		n_len = field->pack_length();

		col_type = get_innobase_type_from_mysql_type(field);
		is_unsigned = (ulint) (field->flags & UNSIGNED_FLAG);

		switch (col_type) {

		case DATA_BLOB:
			o_ptr = row_mysql_read_blob_ref(&o_len, o_ptr, o_len);
			n_ptr = row_mysql_read_blob_ref(&n_len, n_ptr, n_len);
			break;
		case DATA_VARCHAR:
		case DATA_BINARY:
		case DATA_VARMYSQL:
			o_ptr = row_mysql_read_var_ref_noninline(&o_len,
								o_ptr);
			n_ptr = row_mysql_read_var_ref_noninline(&n_len,
								n_ptr);
		default:
			;
		}

		if (field->null_ptr) {
			if (field_in_record_is_null(table, field,
							(char*) old_row)) {
				o_len = UNIV_SQL_NULL;
			}

			if (field_in_record_is_null(table, field,
							(char*) new_row)) {
				n_len = UNIV_SQL_NULL;
			}
		}

		if (o_len != n_len || (o_len != UNIV_SQL_NULL &&
					0 != memcmp(o_ptr, n_ptr, o_len))) {
			/* The field has changed */

			ufield = uvect->fields + n_changed;

			buf = (byte*)
                          innobase_convert_and_store_changed_col(ufield,
					  (mysql_byte*)buf,
					  (mysql_byte*)n_ptr, n_len, col_type,
						is_unsigned);
			ufield->exp = NULL;
			ufield->field_no =
					(prebuilt->table->cols + i)->clust_pos;
			n_changed++;
		}
	}

	uvect->n_fields = n_changed;
	uvect->info_bits = 0;

	ut_a(buf <= (byte*)original_upd_buff + buff_len);

	return(0);
}

/**************************************************************************
Updates a row given as a parameter to a new value. Note that we are given
whole rows, not just the fields which are updated: this incurs some
overhead for CPU when we check which fields are actually updated.
TODO: currently InnoDB does not prevent the 'Halloween problem':
in a searched update a single row can get updated several times
if its index columns are updated! */

int
ha_innobase::update_row(
/*====================*/
					/* out: error number or 0 */
	const mysql_byte* 	old_row,/* in: old row in MySQL format */
	mysql_byte* 		new_row)/* in: new row in MySQL format */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	upd_t*		uvect;
	int		error = 0;

	DBUG_ENTER("ha_innobase::update_row");

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

        if (table->time_stamp) {
                update_timestamp(new_row + table->time_stamp - 1);
	}

	if (last_query_id != user_thd->query_id) {
	        prebuilt->sql_stat_start = TRUE;
                last_query_id = user_thd->query_id;

		innobase_release_stat_resources(prebuilt->trx);
	}

	if (prebuilt->upd_node) {
		uvect = prebuilt->upd_node->update;
	} else {
		uvect = row_get_prebuilt_update_vector(prebuilt);
	}

	/* Build an update vector from the modified fields in the rows
	(uses upd_buff of the handle) */

	calc_row_difference(uvect, (mysql_byte*) old_row, new_row, table,
			upd_buff, (ulint)upd_and_key_val_buff_len,
			prebuilt, user_thd);

	/* This is not a delete */
	prebuilt->upd_node->is_delete = FALSE;

	assert(prebuilt->template_type == ROW_MYSQL_WHOLE_ROW);

	innodb_srv_conc_enter_innodb(prebuilt->trx);

	error = row_update_for_mysql((byte*) old_row, prebuilt);

	innodb_srv_conc_exit_innodb(prebuilt->trx);

	error = convert_error_code_to_mysql(error, user_thd);

	/* Tell InnoDB server that there might be work for
	utility threads: */

	innobase_active_small();

	DBUG_RETURN(error);
}

/**************************************************************************
Deletes a row given as the parameter. */

int
ha_innobase::delete_row(
/*====================*/
					/* out: error number or 0 */
	const mysql_byte* record)	/* in: a row in MySQL format */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	int		error = 0;

	DBUG_ENTER("ha_innobase::delete_row");

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

	if (last_query_id != user_thd->query_id) {
	        prebuilt->sql_stat_start = TRUE;
                last_query_id = user_thd->query_id;

		innobase_release_stat_resources(prebuilt->trx);
	}

	if (!prebuilt->upd_node) {
		row_get_prebuilt_update_vector(prebuilt);
	}

	/* This is a delete */

	prebuilt->upd_node->is_delete = TRUE;

	innodb_srv_conc_enter_innodb(prebuilt->trx);

	error = row_update_for_mysql((byte*) record, prebuilt);

	innodb_srv_conc_exit_innodb(prebuilt->trx);

	error = convert_error_code_to_mysql(error, user_thd);

	/* Tell the InnoDB server that there might be work for
	utility threads: */

	innobase_active_small();

	DBUG_RETURN(error);
}

/**********************************************************************
Initializes a handle to use an index. */

int
ha_innobase::index_init(
/*====================*/
			/* out: 0 or error number */
	uint 	keynr)	/* in: key (index) number */
{
	int 	error	= 0;
  	DBUG_ENTER("index_init");

	error = change_active_index(keynr);

  	DBUG_RETURN(error);
}

/**********************************************************************
Currently does nothing. */

int
ha_innobase::index_end(void)
/*========================*/
{
	int 	error	= 0;
  	DBUG_ENTER("index_end");

  	DBUG_RETURN(error);
}

/*************************************************************************
Converts a search mode flag understood by MySQL to a flag understood
by InnoDB. */
inline
ulint
convert_search_mode_to_innobase(
/*============================*/
	enum ha_rkey_function	find_flag)
{
	switch (find_flag) {
  		case HA_READ_KEY_EXACT:		return(PAGE_CUR_GE);
  			/* the above does not require the index to be UNIQUE */
  		case HA_READ_KEY_OR_NEXT:	return(PAGE_CUR_GE);
		case HA_READ_KEY_OR_PREV:	return(PAGE_CUR_LE);
		case HA_READ_AFTER_KEY:		return(PAGE_CUR_G);
		case HA_READ_BEFORE_KEY:	return(PAGE_CUR_L);
		case HA_READ_PREFIX:		return(PAGE_CUR_GE);
	        case HA_READ_PREFIX_LAST:       return(PAGE_CUR_LE);
		  /* In MySQL-4.0 HA_READ_PREFIX and HA_READ_PREFIX_LAST always
		  pass a complete-field prefix of a key value as the search
		  tuple. I.e., it is not allowed that the last field would
		  just contain n first bytes of the full field value.
		  MySQL uses a 'padding' trick to convert LIKE 'abc%'
		  type queries so that it can use as a search tuple
		  a complete-field-prefix of a key value. Thus, the InnoDB
		  search mode PAGE_CUR_LE_OR_EXTENDS is never used.
		  TODO: when/if MySQL starts to use also partial-field
		  prefixes, we have to deal with stripping of spaces
		  and comparison of non-latin1 char type fields in
		  innobase_mysql_cmp() to get PAGE_CUR_LE_OR_EXTENDS to
		  work correctly. */

		default:			assert(0);
	}

	return(0);
}

/*
   BACKGROUND INFO: HOW A SELECT SQL QUERY IS EXECUTED
   ---------------------------------------------------
The following does not cover all the details, but explains how we determine
the start of a new SQL statement, and what is associated with it.

For each table in the database the MySQL interpreter may have several
table handle instances in use, also in a single SQL query. For each table
handle instance there is an InnoDB  'prebuilt' struct which contains most
of the InnoDB data associated with this table handle instance.

  A) if the user has not explicitly set any MySQL table level locks:

  1) MySQL calls ::external_lock to set an 'intention' table level lock on
the table of the handle instance. There we set
prebuilt->sql_stat_start = TRUE. The flag sql_stat_start should be set
true if we are taking this table handle instance to use in a new SQL
statement issued by the user. We also increment trx->n_mysql_tables_in_use.

  2) If prebuilt->sql_stat_start == TRUE we 'pre-compile' the MySQL search
instructions to prebuilt->template of the table handle instance in
::index_read. The template is used to save CPU time in large joins.

  3) In row_search_for_mysql, if prebuilt->sql_stat_start is true, we
allocate a new consistent read view for the trx if it does not yet have one,
or in the case of a locking read, set an InnoDB 'intention' table level
lock on the table.

  4) We do the SELECT. MySQL may repeatedly call ::index_read for the
same table handle instance, if it is a join.

  5) When the SELECT ends, MySQL removes its intention table level locks
in ::external_lock. When trx->n_mysql_tables_in_use drops to zero,
 (a) we execute a COMMIT there if the autocommit is on,
 (b) we also release possible 'SQL statement level resources' InnoDB may
have for this SQL statement. The MySQL interpreter does NOT execute
autocommit for pure read transactions, though it should. That is why the
table handler in that case has to execute the COMMIT in ::external_lock.

  B) If the user has explicitly set MySQL table level locks, then MySQL
does NOT call ::external_lock at the start of the statement. To determine
when we are at the start of a new SQL statement we at the start of
::index_read also compare the query id to the latest query id where the
table handle instance was used. If it has changed, we know we are at the
start of a new SQL statement. Since the query id can theoretically
overwrap, we use this test only as a secondary way of determining the
start of a new SQL statement. */


/**************************************************************************
Positions an index cursor to the index specified in the handle. Fetches the
row if any. */

int
ha_innobase::index_read(
/*====================*/
					/* out: 0, HA_ERR_KEY_NOT_FOUND,
					or error number */
	mysql_byte*		buf,	/* in/out: buffer for the returned
					row */
	const mysql_byte* 	key_ptr,/* in: key value; if this is NULL
					we position the cursor at the
					start or end of index; this can
					also contain an InnoDB row id, in
					which case key_len is the InnoDB
					row id length; the key value can
					also be a prefix of a full key value,
					and the last column can be a prefix
					of a full column */
	uint			key_len,/* in: key value length */
	enum ha_rkey_function find_flag)/* in: search flags from my_base.h */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	ulint		mode;
	dict_index_t*	index;
	ulint		match_mode 	= 0;
	int 		error;
	ulint		ret;

  	DBUG_ENTER("index_read");

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

  	statistic_increment(ha_read_key_count, &LOCK_status);

	if (last_query_id != user_thd->query_id) {
	        prebuilt->sql_stat_start = TRUE;
                last_query_id = user_thd->query_id;

		innobase_release_stat_resources(prebuilt->trx);
	}

	index = prebuilt->index;

	/* Note that if the index for which the search template is built is not
        necessarily prebuilt->index, but can also be the clustered index */

	if (prebuilt->sql_stat_start) {
		build_template(prebuilt, user_thd, table,
							ROW_MYSQL_REC_FIELDS);
	}

	if (key_ptr) {
	        /* Convert the search key value to InnoDB format into
		prebuilt->search_tuple */

		row_sel_convert_mysql_key_to_innobase(prebuilt->search_tuple,
					(byte*) key_val_buff,
					(ulint)upd_and_key_val_buff_len,
					index,
					(byte*) key_ptr,
					(ulint) key_len);
	} else {
		/* We position the cursor to the last or the first entry
		in the index */

 		dtuple_set_n_fields(prebuilt->search_tuple, 0);
	}

	mode = convert_search_mode_to_innobase(find_flag);

	match_mode = 0;

	if (find_flag == HA_READ_KEY_EXACT) {
		match_mode = ROW_SEL_EXACT;

	} else if (find_flag == HA_READ_PREFIX
				|| find_flag == HA_READ_PREFIX_LAST) {
		match_mode = ROW_SEL_EXACT_PREFIX;
	}

	last_match_mode = match_mode;

	innodb_srv_conc_enter_innodb(prebuilt->trx);

	ret = row_search_for_mysql((byte*) buf, mode, prebuilt, match_mode, 0);

	innodb_srv_conc_exit_innodb(prebuilt->trx);

	if (ret == DB_SUCCESS) {
		error = 0;
		table->status = 0;

	} else if (ret == DB_RECORD_NOT_FOUND) {
		error = HA_ERR_KEY_NOT_FOUND;
		table->status = STATUS_NOT_FOUND;

	} else if (ret == DB_END_OF_INDEX) {
		error = HA_ERR_KEY_NOT_FOUND;
		table->status = STATUS_NOT_FOUND;
	} else {
		error = convert_error_code_to_mysql(ret, user_thd);
		table->status = STATUS_NOT_FOUND;
	}

	DBUG_RETURN(error);
}

/***********************************************************************
The following functions works like index_read, but it find the last
row with the current key value or prefix. */

int
ha_innobase::index_read_last(
/*=========================*/
			           /* out: 0, HA_ERR_KEY_NOT_FOUND, or an
				   error code */
        mysql_byte*       buf,     /* out: fetched row */
        const mysql_byte* key_ptr, /* in: key value, or a prefix of a full
				   key value */
	uint              key_len) /* in: length of the key val or prefix
				   in bytes */
{
        return(index_read(buf, key_ptr, key_len, HA_READ_PREFIX_LAST));
}

/************************************************************************
Changes the active index of a handle. */

int
ha_innobase::change_active_index(
/*=============================*/
			/* out: 0 or error code */
	uint 	keynr)	/* in: use this index; MAX_KEY means always clustered
			index, even if it was internally generated by
			InnoDB */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	KEY*		key=0;
	statistic_increment(ha_read_key_count, &LOCK_status);
	DBUG_ENTER("change_active_index");

	ut_a(prebuilt->trx ==
	     (trx_t*) current_thd->transaction.all.innobase_tid);

	active_index = keynr;

	if (keynr != MAX_KEY && table->keys > 0) {
		key = table->key_info + active_index;

		prebuilt->index = dict_table_get_index_noninline(
						     prebuilt->table,
						     key->name);
        } else {
		prebuilt->index = dict_table_get_first_index_noninline(
							   prebuilt->table);
	}

	if (!prebuilt->index) {
	       sql_print_error(
"Innodb could not find key n:o %u with name %s from dict cache for table %s",
	      keynr, key ? key->name : "NULL", prebuilt->table->name);
	      DBUG_RETURN(1);
	}

	assert(prebuilt->search_tuple != 0);

	dtuple_set_n_fields(prebuilt->search_tuple, prebuilt->index->n_fields);

	dict_index_copy_types(prebuilt->search_tuple, prebuilt->index,
			prebuilt->index->n_fields);

	/* Maybe MySQL changes the active index for a handle also
	during some queries, we do not know: then it is safest to build
	the template such that all columns will be fetched. */

	build_template(prebuilt, NULL, table, ROW_MYSQL_WHOLE_ROW);

	DBUG_RETURN(0);
}

/**************************************************************************
Positions an index cursor to the index specified in keynr. Fetches the
row if any. */
/* ??? This is only used to read whole keys ??? */

int
ha_innobase::index_read_idx(
/*========================*/
					/* out: error number or 0 */
	mysql_byte*	buf,		/* in/out: buffer for the returned
					row */
	uint 		keynr,		/* in: use this index */
	const mysql_byte* key,		/* in: key value; if this is NULL
					we position the cursor at the
					start or end of index */
	uint		key_len,	/* in: key value length */
	enum ha_rkey_function find_flag)/* in: search flags from my_base.h */
{
	if (change_active_index(keynr)) {

		return(1);
	}

	return(index_read(buf, key, key_len, find_flag));
}

/***************************************************************************
Reads the next or previous row from a cursor, which must have previously been
positioned using index_read. */

int
ha_innobase::general_fetch(
/*=======================*/
				/* out: 0, HA_ERR_END_OF_FILE, or error
				number */
	mysql_byte* 	buf,	/* in/out: buffer for next row in MySQL
				format */
	uint 	direction,	/* in: ROW_SEL_NEXT or ROW_SEL_PREV */
	uint	match_mode)	/* in: 0, ROW_SEL_EXACT, or
				ROW_SEL_EXACT_PREFIX */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	ulint		ret;
	int		error	= 0;

	DBUG_ENTER("general_fetch");

	ut_a(prebuilt->trx ==
	     (trx_t*) current_thd->transaction.all.innobase_tid);

	innodb_srv_conc_enter_innodb(prebuilt->trx);

	ret = row_search_for_mysql((byte*)buf, 0, prebuilt, match_mode,
								direction);
	innodb_srv_conc_exit_innodb(prebuilt->trx);

	if (ret == DB_SUCCESS) {
		error = 0;
		table->status = 0;

	} else if (ret == DB_RECORD_NOT_FOUND) {
		error = HA_ERR_END_OF_FILE;
		table->status = STATUS_NOT_FOUND;

	} else if (ret == DB_END_OF_INDEX) {
		error = HA_ERR_END_OF_FILE;
		table->status = STATUS_NOT_FOUND;
	} else {
		error = convert_error_code_to_mysql(ret, user_thd);
		table->status = STATUS_NOT_FOUND;
	}

	DBUG_RETURN(error);
}

/***************************************************************************
Reads the next row from a cursor, which must have previously been
positioned using index_read. */

int
ha_innobase::index_next(
/*====================*/
				/* out: 0, HA_ERR_END_OF_FILE, or error
				number */
	mysql_byte* 	buf)	/* in/out: buffer for next row in MySQL
				format */
{
  	statistic_increment(ha_read_next_count, &LOCK_status);

	return(general_fetch(buf, ROW_SEL_NEXT, 0));
}

/***********************************************************************
Reads the next row matching to the key value given as the parameter. */

int
ha_innobase::index_next_same(
/*=========================*/
				/* out: 0, HA_ERR_END_OF_FILE, or error
				number */
	mysql_byte* 	buf,	/* in/out: buffer for the row */
	const mysql_byte* key,	/* in: key value */
	uint 		keylen)	/* in: key value length */
{
  	statistic_increment(ha_read_next_count, &LOCK_status);

	return(general_fetch(buf, ROW_SEL_NEXT, last_match_mode));
}

/***************************************************************************
Reads the previous row from a cursor, which must have previously been
positioned using index_read. */

int
ha_innobase::index_prev(
/*====================*/
				/* out: 0, HA_ERR_END_OF_FILE, or error
				number */
	mysql_byte* 	buf)	/* in/out: buffer for previous row in MySQL
				format */
{
	return(general_fetch(buf, ROW_SEL_PREV, 0));
}

/************************************************************************
Positions a cursor on the first record in an index and reads the
corresponding row to buf. */

int
ha_innobase::index_first(
/*=====================*/
				/* out: 0, HA_ERR_END_OF_FILE,
				or error code */
	mysql_byte*	buf)	/* in/out: buffer for the row */
{
	int	error;

  	DBUG_ENTER("index_first");
  	statistic_increment(ha_read_first_count, &LOCK_status);

  	error = index_read(buf, NULL, 0, HA_READ_AFTER_KEY);

        /* MySQL does not seem to allow this to return HA_ERR_KEY_NOT_FOUND */

  	if (error == HA_ERR_KEY_NOT_FOUND) {
  		error = HA_ERR_END_OF_FILE;
  	}

  	DBUG_RETURN(error);
}

/************************************************************************
Positions a cursor on the last record in an index and reads the
corresponding row to buf. */

int
ha_innobase::index_last(
/*====================*/
				/* out: 0, HA_ERR_END_OF_FILE, or error code */
	mysql_byte*	buf)	/* in/out: buffer for the row */
{
	int	error;

  	DBUG_ENTER("index_first");
  	statistic_increment(ha_read_last_count, &LOCK_status);

  	error = index_read(buf, NULL, 0, HA_READ_BEFORE_KEY);

        /* MySQL does not seem to allow this to return HA_ERR_KEY_NOT_FOUND */

  	if (error == HA_ERR_KEY_NOT_FOUND) {
  		error = HA_ERR_END_OF_FILE;
  	}

  	DBUG_RETURN(error);
}

/********************************************************************
Initialize a table scan. */

int
ha_innobase::rnd_init(
/*==================*/
			/* out: 0 or error number */
	bool	scan)	/* in: ???????? */
{
	int	err;

	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;

	/* Store the active index value so that we can restore the original
	value after a scan */

	active_index_before_scan = active_index;

	if (prebuilt->clust_index_was_generated) {
		err = change_active_index(MAX_KEY);
	} else {
		err = change_active_index(primary_key);
	}

  	start_of_scan = 1;

 	return(err);
}

/*********************************************************************
Ends a table scan. */

int
ha_innobase::rnd_end(void)
/*======================*/
				/* out: 0 or error number */
{
	/* Restore the old active_index back; MySQL may assume that a table
	scan does not change active_index. We only restore the value if
	MySQL has called rnd_init before: sometimes MySQL seems to call
	rnd_end WITHOUT calling rnd_init. */

	if (active_index_before_scan != (uint)-1) {

		change_active_index(active_index_before_scan);

		active_index_before_scan = (uint)-1;
	}

  	return(index_end());
}

/*********************************************************************
Reads the next row in a table scan (also used to read the FIRST row
in a table scan). */

int
ha_innobase::rnd_next(
/*==================*/
			/* out: 0, HA_ERR_END_OF_FILE, or error number */
	mysql_byte* buf)/* in/out: returns the row in this buffer,
			in MySQL format */
{
	int	error;

  	DBUG_ENTER("rnd_next");
  	statistic_increment(ha_read_rnd_next_count, &LOCK_status);

  	if (start_of_scan) {
		error = index_first(buf);
		if (error == HA_ERR_KEY_NOT_FOUND) {
			error = HA_ERR_END_OF_FILE;
		}
		start_of_scan = 0;
	} else {
		error = general_fetch(buf, ROW_SEL_NEXT, 0);
	}

  	DBUG_RETURN(error);
}

/**************************************************************************
Fetches a row from the table based on a row reference. */

int
ha_innobase::rnd_pos(
/*=================*/
				/* out: 0, HA_ERR_KEY_NOT_FOUND,
				or error code */
	mysql_byte* 	buf,	/* in/out: buffer for the row */
	mysql_byte*	pos)	/* in: primary key value of the row in the
				MySQL format, or the row id if the clustered
				index was internally generated by InnoDB;
				the length of data in pos has to be
				ref_length */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	int		error;
	uint		keynr	= active_index;
	DBUG_ENTER("rnd_pos");
	DBUG_DUMP("key", (char*) pos, ref_length);

	statistic_increment(ha_read_rnd_count, &LOCK_status);

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

	if (prebuilt->clust_index_was_generated) {
		/* No primary key was defined for the table and we
		generated the clustered index from the row id: the
		row reference is the row id, not any key value
		that MySQL knows of */

		error = change_active_index(MAX_KEY);
	} else {
		error = change_active_index(primary_key);
	}

	if (error) {
	        DBUG_PRINT("error",("Got error: %ld",error));
		DBUG_RETURN(error);
	}

	/* Note that we assume the length of the row reference is fixed
        for the table, and it is == ref_length */

	error = index_read(buf, pos, ref_length, HA_READ_KEY_EXACT);
	if (error)
	{
	  DBUG_PRINT("error",("Got error: %ld",error));
	}
	change_active_index(keynr);

  	DBUG_RETURN(error);
}

/*************************************************************************
Stores a reference to the current row to 'ref' field of the handle. Note
that in the case where we have generated the clustered index for the
table, the function parameter is illogical: we MUST ASSUME that 'record'
is the current 'position' of the handle, because if row ref is actually
the row id internally generated in InnoDB, then 'record' does not contain
it. We just guess that the row id must be for the record where the handle
was positioned the last time. */

void
ha_innobase::position(
/*==================*/
	const mysql_byte*	record)	/* in: row in MySQL format */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	uint		len;

	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

	if (prebuilt->clust_index_was_generated) {
		/* No primary key was defined for the table and we
		generated the clustered index from row id: the
		row reference will be the row id, not any key value
		that MySQL knows of */

		len = DATA_ROW_ID_LEN;

		memcpy(ref, prebuilt->row_id, len);
	} else {
		len = store_key_val_for_row(primary_key, (char*)ref,
							 ref_length, record);
	}

	/* Since we do not store len to the buffer 'ref', we must assume
	that len is always fixed for this table. The following assertion
	checks this. */
  
	if (len != ref_length) {
	        fprintf(stderr,
	 "InnoDB: Error: stored ref len is %lu, but table ref len is %lu\n",
		  (ulint)len, (ulint)ref_length);
	}
}

/*********************************************************************
Creates a table definition to an InnoDB database. */
static
int
create_table_def(
/*=============*/
	trx_t*		trx,		/* in: InnoDB transaction handle */
	TABLE*		form,		/* in: information on table
					columns and indexes */
	const char*	table_name)	/* in: table name */
{
	Field*		field;
	dict_table_t*	table;
	ulint		n_cols;
  	int 		error;
  	ulint		col_type;
  	ulint		nulls_allowed;
	ulint		unsigned_type;
	ulint		binary_type;
	ulint		nonlatin1_type;
  	ulint		i;

  	DBUG_ENTER("create_table_def");
  	DBUG_PRINT("enter", ("table_name: %s", table_name));

	n_cols = form->fields;

	/* The '0' below specifies that everything is currently
	created in tablespace 0 */

	table = dict_mem_table_create((char*) table_name, 0, n_cols);

	for (i = 0; i < n_cols; i++) {
		field = form->field[i];

		col_type = get_innobase_type_from_mysql_type(field);
		if (field->null_ptr) {
			nulls_allowed = 0;
		} else {
			nulls_allowed = DATA_NOT_NULL;
		}

		if (field->flags & UNSIGNED_FLAG) {
			unsigned_type = DATA_UNSIGNED;
		} else {
			unsigned_type = 0;
		}

		if (col_type == DATA_BLOB
		    && strcmp(default_charset_info->name, "latin1") != 0) {
			nonlatin1_type = DATA_NONLATIN1;
		} else {
		        nonlatin1_type = 0;
		}

		if (field->flags & BINARY_FLAG) {
			binary_type = DATA_BINARY_TYPE;
		        nonlatin1_type = 0;
		} else {
			binary_type = 0;
		}

		dict_mem_table_add_col(table, (char*) field->field_name,
					col_type, (ulint)field->type()
					| nulls_allowed | unsigned_type
					| nonlatin1_type | binary_type,
					field->pack_length(), 0);
	}

	error = row_create_table_for_mysql(table, trx);

	error = convert_error_code_to_mysql(error, NULL);

	DBUG_RETURN(error);
}

/*********************************************************************
Creates an index in an InnoDB database. */
static
int
create_index(
/*=========*/
	trx_t*		trx,		/* in: InnoDB transaction handle */
	TABLE*		form,		/* in: information on table
					columns and indexes */
	const char*	table_name,	/* in: table name */
	uint		key_num)	/* in: index number */
{
	Field*		field;
	dict_index_t*	index;
  	int 		error;
	ulint		n_fields;
	KEY*		key;
	KEY_PART_INFO*	key_part;
	ulint		ind_type;
	ulint		col_type;
	ulint		prefix_len;
  	ulint		i;
  	ulint		j;

  	DBUG_ENTER("create_index");

	key = form->key_info + key_num;

    	n_fields = key->key_parts;

    	ind_type = 0;

    	if (key_num == form->primary_key)
	{
		ind_type = ind_type | DICT_CLUSTERED;
	}

	if (key->flags & HA_NOSAME ) {
		ind_type = ind_type | DICT_UNIQUE;
	}

	/* The '0' below specifies that everything in InnoDB is currently
	created in tablespace 0 */

	index = dict_mem_index_create((char*) table_name, key->name, 0,
						ind_type, n_fields);
	for (i = 0; i < n_fields; i++) {
		key_part = key->key_part + i;

		/* (The flag HA_PART_KEY denotes in MySQL a column prefix
		field in an index: we only store a specified number of first
		bytes of the column to the index field.) The flag does not
		seem to be properly set by MySQL. Let us fall back on testing
		the length of the key part versus the column. */
		
		field = NULL;
		for (j = 0; j < form->fields; j++) {

			field = form->field[j];

			if (strlen(field->field_name)
			    == strlen(key_part->field->field_name)
			    && 0 == ut_cmp_in_lower_case(
					(char*)field->field_name,
					(char*)key_part->field->field_name,
					strlen(field->field_name))) {
				/* Found the corresponding column */

				break;
			}
		}

		ut_a(j < form->fields);

		col_type = get_innobase_type_from_mysql_type(key_part->field);

		if (DATA_BLOB == col_type
		    || key_part->length < field->pack_length()) {

		        prefix_len = key_part->length;

			if (col_type == DATA_INT
			    || col_type == DATA_FLOAT
			    || col_type == DATA_DOUBLE
			    || col_type == DATA_DECIMAL) {
			        fprintf(stderr,
"InnoDB: error: MySQL is trying to create a column prefix index field\n"
"InnoDB: on an inappropriate data type. Table name %s, column name %s.\n",
				  table_name, key_part->field->field_name);
			        
			        prefix_len = 0;
			}
		} else {
		        prefix_len = 0;
		}

		if (prefix_len >= DICT_MAX_COL_PREFIX_LEN) {
			DBUG_RETURN(-1);
		}

		/* We assume all fields should be sorted in ascending
		order, hence the '0': */

		dict_mem_index_add_field(index,
				(char*) key_part->field->field_name,
				0, prefix_len);
	}

	error = row_create_index_for_mysql(index, trx);

	error = convert_error_code_to_mysql(error, NULL);

	DBUG_RETURN(error);
}

/*********************************************************************
Creates an index to an InnoDB table when the user has defined no
primary index. */
static
int
create_clustered_index_when_no_primary(
/*===================================*/
	trx_t*		trx,		/* in: InnoDB transaction handle */
	const char*	table_name)	/* in: table name */
{
	dict_index_t*	index;
  	int 		error;

	/* The first '0' below specifies that everything in InnoDB is
	currently created in file space 0 */

	index = dict_mem_index_create((char*) table_name,
				      (char*) "GEN_CLUST_INDEX",
				      0, DICT_CLUSTERED, 0);
	error = row_create_index_for_mysql(index, trx);

	error = convert_error_code_to_mysql(error, NULL);

	return(error);
}

/*********************************************************************
Creates a new table to an InnoDB database. */

int
ha_innobase::create(
/*================*/
					/* out: error number */
	const char*	name,		/* in: table name */
	TABLE*		form,		/* in: information on table
					columns and indexes */
	HA_CREATE_INFO*	create_info)	/* in: more information of the
					created table, contains also the
					create statement string */
{
	int		error;
	dict_table_t*	innobase_table;
	trx_t*		parent_trx;
	trx_t*		trx;
	int		primary_key_no;
	uint		i;
	char		name2[FN_REFLEN];
	char		norm_name[FN_REFLEN];
	THD		*thd= current_thd;

  	DBUG_ENTER("ha_innobase::create");

	DBUG_ASSERT(thd != NULL);

	if (form->fields > 1000) {
		/* The limit probably should be REC_MAX_N_FIELDS - 3 = 1020,
		but we play safe here */

	        return(HA_ERR_TO_BIG_ROW);
	} 

	/* Get the transaction associated with the current thd, or create one
	if not yet created */
	
	parent_trx = check_trx_exists(current_thd);

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(parent_trx);	
	
	trx = trx_allocate_for_mysql();
		
	trx->mysql_thd = thd;
	trx->mysql_query_str = &((*thd).query);

	if (thd->options & OPTION_NO_FOREIGN_KEY_CHECKS) {
		trx->check_foreigns = FALSE;
	}

	if (thd->options & OPTION_RELAXED_UNIQUE_CHECKS) {
		trx->check_unique_secondary = FALSE;
	}

	if (lower_case_table_names) {
		srv_lower_case_table_names = TRUE;
	} else {
		srv_lower_case_table_names = FALSE;
	}

	fn_format(name2, name, "", "",2);	// Remove the .frm extension

	normalize_table_name(norm_name, name2);

	/* Latch the InnoDB data dictionary exclusively so that no deadlocks
	or lock waits can happen in it during a table create operation.
	Drop table etc. do this latching in row0mysql.c. */

	row_mysql_lock_data_dictionary(trx);

	/* Create the table definition in InnoDB */

  	error = create_table_def(trx, form, norm_name);
  	
  	if (error) {
		innobase_commit_low(trx);

		row_mysql_unlock_data_dictionary(trx);

  		trx_free_for_mysql(trx);

 		DBUG_RETURN(error);
 	}

	/* Look for a primary key */

	primary_key_no= (table->primary_key != MAX_KEY ?
			 (int) table->primary_key : 
			 -1);

	/* Our function row_get_mysql_key_number_for_index assumes
	the primary key is always number 0, if it exists */

	DBUG_ASSERT(primary_key_no == -1 || primary_key_no == 0);

	/* Create the keys */

	if (form->keys == 0 || primary_key_no == -1) {
		/* Create an index which is used as the clustered index;
		order the rows by their row id which is internally generated
		by InnoDB */

		error = create_clustered_index_when_no_primary(trx,
							norm_name);
  		if (error) {
			innobase_commit_low(trx);

			row_mysql_unlock_data_dictionary(trx);

			trx_free_for_mysql(trx);

			DBUG_RETURN(error);
      		}
	}

	if (primary_key_no != -1) {
		/* In InnoDB the clustered index must always be created
		first */
	    	if ((error = create_index(trx, form, norm_name,
					  (uint) primary_key_no))) {
			innobase_commit_low(trx);

			row_mysql_unlock_data_dictionary(trx);

  			trx_free_for_mysql(trx);

			DBUG_RETURN(error);
      		}
      	}

	for (i = 0; i < form->keys; i++) {

		if (i != (uint) primary_key_no) {

    			if ((error = create_index(trx, form, norm_name, i))) {

			  	innobase_commit_low(trx);

				row_mysql_unlock_data_dictionary(trx);

  				trx_free_for_mysql(trx);

				DBUG_RETURN(error);
      			}
      		}
  	}

	if (current_thd->query != NULL) {
  	
		error = row_table_add_foreign_constraints(trx,
					current_thd->query, norm_name);

		error = convert_error_code_to_mysql(error, NULL);

		if (error) {
			innobase_commit_low(trx);

			row_mysql_unlock_data_dictionary(trx);

  			trx_free_for_mysql(trx);

			DBUG_RETURN(error);
		}
	}

  	innobase_commit_low(trx);

	row_mysql_unlock_data_dictionary(trx);

	/* Flush the log to reduce probability that the .frm files and
	the InnoDB data dictionary get out-of-sync if the user runs
	with innodb_flush_log_at_trx_commit = 0 */

	log_buffer_flush_to_disk();

	innobase_table = dict_table_get(norm_name, NULL);

	DBUG_ASSERT(innobase_table != 0);

	/* Tell the InnoDB server that there might be work for
	utility threads: */

	srv_active_wake_master_thread();

  	trx_free_for_mysql(trx);

	DBUG_RETURN(0);
}

/*********************************************************************
Drops a table from an InnoDB database. Before calling this function,
MySQL calls innobase_commit to commit the transaction of the current user.
Then the current user cannot have locks set on the table. Drop table
operation inside InnoDB will remove all locks any user has on the table
inside InnoDB. */

int
ha_innobase::delete_table(
/*======================*/
				/* out: error number */
	const char*	name)	/* in: table name */
{
	ulint	name_len;
	int	error;
	trx_t*	parent_trx;
	trx_t*	trx;
	char	norm_name[1000];

  	DBUG_ENTER("ha_innobase::delete_table");

	/* Get the transaction associated with the current thd, or create one
	if not yet created */
	
	parent_trx = check_trx_exists(current_thd);

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(parent_trx);	

	if (lower_case_table_names) {
		srv_lower_case_table_names = TRUE;
	} else {
		srv_lower_case_table_names = FALSE;
	}

	trx = trx_allocate_for_mysql();

	trx->mysql_thd = current_thd;
	trx->mysql_query_str = &((*current_thd).query);

	name_len = strlen(name);

	assert(name_len < 1000);

	/* Strangely, MySQL passes the table name without the '.frm'
	extension, in contrast to ::create */

	normalize_table_name(norm_name, name);

  	/* Drop the table in InnoDB */

  	error = row_drop_table_for_mysql(norm_name, trx);

	/* Flush the log to reduce probability that the .frm files and
	the InnoDB data dictionary get out-of-sync if the user runs
	with innodb_flush_log_at_trx_commit = 0 */

	log_buffer_flush_to_disk();

	/* Tell the InnoDB server that there might be work for
	utility threads: */

	srv_active_wake_master_thread();

  	innobase_commit_low(trx);

  	trx_free_for_mysql(trx);

	error = convert_error_code_to_mysql(error, NULL);

	DBUG_RETURN(error);
}

/*********************************************************************
Removes all tables in the named database inside InnoDB. */

int
innobase_drop_database(
/*===================*/
			/* out: error number */
	char*	path)	/* in: database path; inside InnoDB the name
			of the last directory in the path is used as
			the database name: for example, in 'mysql/data/test'
			the database name is 'test' */
{
	ulint	len		= 0;
	trx_t*	parent_trx;
	trx_t*	trx;
	char*	ptr;
	int	error;
	char	namebuf[10000];

	/* Get the transaction associated with the current thd, or create one
	if not yet created */
	
	parent_trx = check_trx_exists(current_thd);

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(parent_trx);	

	ptr = strend(path) - 2;

	while (ptr >= path && *ptr != '\\' && *ptr != '/') {
		ptr--;
		len++;
	}

	ptr++;

	memcpy(namebuf, ptr, len);
	namebuf[len] = '/';
	namebuf[len + 1] = '\0';
#ifdef __WIN__
	casedn_str(namebuf);
#endif
	trx = trx_allocate_for_mysql();
	trx->mysql_thd = current_thd;
	trx->mysql_query_str = &((*current_thd).query);

  	error = row_drop_database_for_mysql(namebuf, trx);

	/* Flush the log to reduce probability that the .frm files and
	the InnoDB data dictionary get out-of-sync if the user runs
	with innodb_flush_log_at_trx_commit = 0 */

	log_buffer_flush_to_disk();

	/* Tell the InnoDB server that there might be work for
	utility threads: */

	srv_active_wake_master_thread();

  	innobase_commit_low(trx);
  	trx_free_for_mysql(trx);

	error = convert_error_code_to_mysql(error, NULL);

	return(error);
}

/*************************************************************************
Renames an InnoDB table. */

int
ha_innobase::rename_table(
/*======================*/
				/* out: 0 or error code */
	const char*	from,	/* in: old name of the table */
	const char*	to)	/* in: new name of the table */
{
	ulint	name_len1;
	ulint	name_len2;
	int	error;
	trx_t*	parent_trx;
	trx_t*	trx;
	char	norm_from[1000];
	char	norm_to[1000];

  	DBUG_ENTER("ha_innobase::rename_table");

	/* Get the transaction associated with the current thd, or create one
	if not yet created */
	
	parent_trx = check_trx_exists(current_thd);

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(parent_trx);	

	if (lower_case_table_names) {
		srv_lower_case_table_names = TRUE;
	} else {
		srv_lower_case_table_names = FALSE;
	}

	trx = trx_allocate_for_mysql();
	trx->mysql_thd = current_thd;
	trx->mysql_query_str = &((*current_thd).query);

	name_len1 = strlen(from);
	name_len2 = strlen(to);

	assert(name_len1 < 1000);
	assert(name_len2 < 1000);

	normalize_table_name(norm_from, from);
	normalize_table_name(norm_to, to);

  	/* Rename the table in InnoDB */

  	error = row_rename_table_for_mysql(norm_from, norm_to, trx);

	/* Flush the log to reduce probability that the .frm files and
	the InnoDB data dictionary get out-of-sync if the user runs
	with innodb_flush_log_at_trx_commit = 0 */

	log_buffer_flush_to_disk();

	/* Tell the InnoDB server that there might be work for
	utility threads: */

	srv_active_wake_master_thread();

  	innobase_commit_low(trx);
  	trx_free_for_mysql(trx);

	error = convert_error_code_to_mysql(error, NULL);

	DBUG_RETURN(error);
}

/*************************************************************************
Estimates the number of index records in a range. */

ha_rows
ha_innobase::records_in_range(
/*==========================*/
						/* out: estimated number of
						rows */
	int 			keynr,		/* in: index number */
	const mysql_byte*	start_key,	/* in: start key value of the
						range, may also be empty */
	uint 			start_key_len,	/* in: start key val len, may
						also be 0 */
	enum ha_rkey_function 	start_search_flag,/* in: start search condition
						e.g., 'greater than' */
	const mysql_byte*	end_key,	/* in: range end key val, may
						also be empty */
	uint 			end_key_len,	/* in: range end key val len,
						may also be 0 */
	enum ha_rkey_function 	end_search_flag)/* in: range end search cond */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	KEY*		key;
	dict_index_t*	index;
	mysql_byte*	key_val_buff2 	= (mysql_byte*) my_malloc(
						  table->reclength
      						+ table->max_key_length + 100,
								MYF(MY_WME));
	ulint		buff2_len = table->reclength
      						+ table->max_key_length + 100;
	dtuple_t*	range_start;
	dtuple_t*	range_end;
	ib_longlong	n_rows;
	ulint		mode1;
	ulint		mode2;
	void*           heap1;
	void*           heap2;

   	DBUG_ENTER("records_in_range");

	/* We do not know if MySQL can call this function before calling
	external_lock(). To be safe, update the thd of the current table
	handle. */

	update_thd(current_thd);

	prebuilt->trx->op_info = (char*)"estimating records in index range";

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(prebuilt->trx);

	active_index = keynr;

	key = table->key_info + active_index;

	index = dict_table_get_index_noninline(prebuilt->table, key->name);

	range_start = dtuple_create_for_mysql(&heap1, key->key_parts);
 	dict_index_copy_types(range_start, index, key->key_parts);

	range_end = dtuple_create_for_mysql(&heap2, key->key_parts);
 	dict_index_copy_types(range_end, index, key->key_parts);

	row_sel_convert_mysql_key_to_innobase(
				range_start, (byte*) key_val_buff,
				(ulint)upd_and_key_val_buff_len,
				index,
				(byte*) start_key,
				(ulint) start_key_len);

	row_sel_convert_mysql_key_to_innobase(
				range_end, (byte*) key_val_buff2,
				buff2_len, index,
				(byte*) end_key,
				(ulint) end_key_len);

	mode1 = convert_search_mode_to_innobase(start_search_flag);
	mode2 = convert_search_mode_to_innobase(end_search_flag);

	n_rows = btr_estimate_n_rows_in_range(index, range_start,
						mode1, range_end, mode2);
	dtuple_free_for_mysql(heap1);
	dtuple_free_for_mysql(heap2);

    	my_free((char*) key_val_buff2, MYF(0));

	prebuilt->trx->op_info = (char*)"";

	/* The MySQL optimizer seems to believe an estimate of 0 rows is
	always accurate and may return the result 'Empty set' based on that.
	The accuracy is not guaranteed, and even if it were, for a locking
	read we should anyway perform the search to set the next-key lock.
	Add 1 to the value to make sure MySQL does not make the assumption! */

	if (n_rows == 0) {
	        n_rows = 1;
	}

	DBUG_RETURN((ha_rows) n_rows);
}

/*************************************************************************
Gives an UPPER BOUND to the number of rows in a table. This is used in
filesort.cc. */

ha_rows
ha_innobase::estimate_number_of_rows(void)
/*======================================*/
			/* out: upper bound of rows */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	dict_index_t*	index;
	ulonglong	estimate;
	ulonglong	local_data_file_length;

 	DBUG_ENTER("estimate_number_of_rows");

	/* We do not know if MySQL can call this function before calling
	external_lock(). To be safe, update the thd of the current table
	handle. */

	update_thd(current_thd);

	prebuilt->trx->op_info = (char*)
	                         "calculating upper bound for table rows";

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(prebuilt->trx);

	index = dict_table_get_first_index_noninline(prebuilt->table);

	local_data_file_length = ((ulonglong) index->stat_n_leaf_pages)
    							* UNIV_PAGE_SIZE;

	/* Calculate a minimum length for a clustered index record and from
	that an upper bound for the number of rows. Since we only calculate
	new statistics in row0mysql.c when a table has grown by a threshold
	factor, we must add a safety factor 2 in front of the formula below. */

	estimate = 2 * local_data_file_length /
					 dict_index_calc_min_rec_len(index);

	prebuilt->trx->op_info = (char*)"";

	DBUG_RETURN((ha_rows) estimate);
}

/*************************************************************************
How many seeks it will take to read through the table. This is to be
comparable to the number returned by records_in_range so that we can
decide if we should scan the table or use keys. */

double
ha_innobase::scan_time()
/*====================*/
			/* out: estimated time measured in disk seeks */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;

	/* Since MySQL seems to favor table scans too much over index
	searches, we pretend that a sequential read takes the same time
	as a random disk read, that is, we do not divide the following
	by 10, which would be physically realistic. */
	
	return((double) (prebuilt->table->stat_clustered_index_size));
}

/**********************************************************************
Calculate the time it takes to read a set of ranges through an index
This enables us to optimise reads for clustered indexes. */

double
ha_innobase::read_time(
/*===================*/
			/* out: estimated time measured in disk seeks */
	uint    index,	/* in: key number */
	uint	ranges,	/* in: how many ranges */
	ha_rows rows)	/* in: estimated number of rows in the ranges */
{
	ha_rows total_rows;
	double  time_for_scan;
  
	if (index != table->primary_key)
	  return handler::read_time(index, ranges, rows); // Not clustered

	if (rows <= 2)
	  return (double) rows;

	/* Assume that the read time is proportional to the scan time for all
	rows + at most one seek per range. */

	time_for_scan= scan_time();

	if ((total_rows= estimate_number_of_rows()) < rows)
	  return time_for_scan;

	return (ranges + (double) rows / (double) total_rows * time_for_scan);
}

/*************************************************************************
Returns statistics information of the table to the MySQL interpreter,
in various fields of the handle object. */

void
ha_innobase::info(
/*==============*/
	uint flag)	/* in: what information MySQL requests */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	dict_table_t*	ib_table;
	dict_index_t*	index;
	ha_rows		rec_per_key;
	ulong		j;
	ulong		i;

 	DBUG_ENTER("info");

        /* If we are forcing recovery at a high level, we will suppress
	statistics calculation on tables, because that may crash the
	server if an index is badly corrupted. */

        if (srv_force_recovery >= SRV_FORCE_NO_IBUF_MERGE) {

                return;
        }

	/* We do not know if MySQL can call this function before calling
	external_lock(). To be safe, update the thd of the current table
	handle. */

	update_thd(current_thd);

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	prebuilt->trx->op_info = (char*)"returning various info to MySQL";

	trx_search_latch_release_if_reserved(prebuilt->trx);

 	ib_table = prebuilt->table;

 	if (flag & HA_STATUS_TIME) {
 		/* In sql_show we call with this flag: update then statistics
 		so that they are up-to-date */

	        prebuilt->trx->op_info = (char*)"updating table statistics";

 		dict_update_statistics(ib_table);

		prebuilt->trx->op_info = (char*)
		                          "returning various info to MySQL";
 	}

	if (flag & HA_STATUS_VARIABLE) {
    		records = (ha_rows)ib_table->stat_n_rows;
    		deleted = 0;
    		data_file_length = ((ulonglong)
				ib_table->stat_clustered_index_size)
    					* UNIV_PAGE_SIZE;
    		index_file_length = ((ulonglong)
				ib_table->stat_sum_of_other_index_sizes)
    					* UNIV_PAGE_SIZE;
    		delete_length = 0;
    		check_time = 0;

    		if (records == 0) {
    			mean_rec_length = 0;
    		} else {
    			mean_rec_length = (ulong) (data_file_length / records);
    		}
    	}

	if (flag & HA_STATUS_CONST) {
		index = dict_table_get_first_index_noninline(ib_table);

		if (prebuilt->clust_index_was_generated) {
			index = dict_table_get_next_index_noninline(index);
		}

		for (i = 0; i < table->keys; i++) {
			if (index == NULL) {
				ut_print_timestamp(stderr);
			        fprintf(stderr,
"  InnoDB: Error: table %s contains less indexes inside InnoDB\n"
"InnoDB: than are defined in the MySQL .frm file. Have you mixed up\n"
"InnoDB: .frm files from different installations? See section\n"
"InnoDB: 15.1 at http://www.innodb.com/ibman.html\n",
				   ib_table->name);
				break;
			}

			for (j = 0; j < table->key_info[i].key_parts; j++) {

				if (j + 1 > index->n_uniq) {
				        ut_print_timestamp(stderr);
			                fprintf(stderr,
"  InnoDB: Error: index %s of %s has %lu columns unique inside InnoDB\n"
"InnoDB: but MySQL is asking statistics for %lu columns. Have you mixed up\n"
"InnoDB: .frm files from different installations? See section\n"
"InnoDB: 15.1 at http://www.innodb.com/ibman.html\n",
						index->name,
						ib_table->name, index->n_uniq,
						j + 1);
				        break;
				}

				if (index->stat_n_diff_key_vals[j + 1] == 0) {

					rec_per_key = records;
				} else {
					rec_per_key = (ha_rows)(records /
   				         index->stat_n_diff_key_vals[j + 1]);
				}

				/* Since MySQL seems to favor table scans
				too much over index searches, we pretend
				index selectivity is 2 times better than
				our estimate: */

				rec_per_key = rec_per_key / 2;

				if (rec_per_key == 0) {
					rec_per_key = 1;
				}

 				table->key_info[i].rec_per_key[j]=
				  rec_per_key >= ~(ulong) 0 ? ~(ulong) 0 :
				  rec_per_key;
			}

			index = dict_table_get_next_index_noninline(index);
		}
	}

  	if (flag & HA_STATUS_ERRKEY) {
		ut_a(prebuilt->trx && prebuilt->trx->magic_n == TRX_MAGIC_N);

		errkey = (unsigned int) row_get_mysql_key_number_for_index(
				       (dict_index_t*)
				       trx_get_error_info(prebuilt->trx));
  	}

	prebuilt->trx->op_info = (char*)"";

  	DBUG_VOID_RETURN;
}

/**************************************************************************
Updates index cardinalities of the table, based on 10 random dives into
each index tree. This does NOT calculate exact statistics of the table. */

int
ha_innobase::analyze(
/*=================*/			 
					/* out: returns always 0 (success) */
	THD*		thd,		/* in: connection thread handle */
	HA_CHECK_OPT*	check_opt)	/* in: currently ignored */
{
	/* Simply call ::info() with all the flags */
	info(HA_STATUS_TIME | HA_STATUS_CONST | HA_STATUS_VARIABLE);

	return(0);
}


int ha_innobase::optimize(THD* thd, HA_CHECK_OPT* check_opt)
{
  return ha_innobase::analyze(thd,check_opt);
}

/***********************************************************************
Tries to check that an InnoDB table is not corrupted. If corruption is
noticed, prints to stderr information about it. In case of corruption
may also assert a failure and crash the server. */

int
ha_innobase::check(
/*===============*/
					/* out: HA_ADMIN_CORRUPT or
					HA_ADMIN_OK */
	THD* 		thd,		/* in: user thread handle */
	HA_CHECK_OPT* 	check_opt)	/* in: check options, currently
					ignored */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
	ulint		ret;

	ut_a(prebuilt->trx && prebuilt->trx->magic_n == TRX_MAGIC_N);
	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);

	if (prebuilt->mysql_template == NULL) {
		/* Build the template; we will use a dummy template
		in index scans done in checking */

		build_template(prebuilt, NULL, table, ROW_MYSQL_WHOLE_ROW);
	}

	ret = row_check_table_for_mysql(prebuilt);

	if (ret == DB_SUCCESS) {
		return(HA_ADMIN_OK);
	}

  	return(HA_ADMIN_CORRUPT); 
}

/*****************************************************************
Adds information about free space in the InnoDB tablespace to a table comment
which is printed out when a user calls SHOW TABLE STATUS. Adds also info on
foreign keys. */

char*
ha_innobase::update_table_comment(
/*==============================*/
				/* out: table comment + InnoDB free space +
				info on foreign keys */
        const char*	comment)/* in: table comment defined by user */
{
	row_prebuilt_t* prebuilt = (row_prebuilt_t*)innobase_prebuilt;
  	uint 		length 	= strlen(comment);
  	char*		str 	= my_malloc(length + 16500, MYF(0));
  	char*		pos;

	/* We do not know if MySQL can call this function before calling
	external_lock(). To be safe, update the thd of the current table
	handle. */

	update_thd(current_thd);

	prebuilt->trx->op_info = (char*)"returning table comment";

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(prebuilt->trx);
   	
	if (!str) {
	        prebuilt->trx->op_info = (char*)"";

    		return((char*)comment);
	}

	pos = str;
  	if (length) {
    		pos=strmov(str, comment);
    		*pos++=';';
    		*pos++=' ';
  	}

  	pos += my_sprintf(pos,
			  (pos,"InnoDB free: %lu kB",
			   (ulong) innobase_get_free_space()));

	/* We assume 16000 - length bytes of space to print info; the limit
        16000 bytes is arbitrary, and MySQL could handle at least 64000
	bytes */
  
	if (length < 16000) {
  		dict_print_info_on_foreign_keys(FALSE, pos, 16000 - length,
							prebuilt->table);
	}

        prebuilt->trx->op_info = (char*)"";

  	return(str);
}

/***********************************************************************
Gets the foreign key create info for a table stored in InnoDB. */

char*
ha_innobase::get_foreign_key_create_info(void)
/*==========================================*/
			/* out, own: character string in the form which
			can be inserted to the CREATE TABLE statement,
			MUST be freed with ::free_foreign_key_create_info */
{
	row_prebuilt_t* prebuilt = (row_prebuilt_t*)innobase_prebuilt;
	char*	str;

	ut_a(prebuilt != NULL);

	/* We do not know if MySQL can call this function before calling
	external_lock(). To be safe, update the thd of the current table
	handle. */

	update_thd(current_thd);

        prebuilt->trx->op_info = (char*)"getting info on foreign keys";

	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(prebuilt->trx);
	
	str = (char*)ut_malloc(10000);

	str[0] = '\0';
	
  	dict_print_info_on_foreign_keys(TRUE, str, 9000, prebuilt->table);

        prebuilt->trx->op_info = (char*)"";

  	return(str);
}			

/***********************************************************************
Frees the foreign key create info for a table stored in InnoDB, if it is
non-NULL. */

void
ha_innobase::free_foreign_key_create_info(
/*======================================*/
	char*	str)	/* in, own: create info string to free  */
{
	if (str) {
		ut_free(str);
	}
}

/***********************************************************************
Tells something additional to the handler about how to do things. */

int
ha_innobase::extra(
/*===============*/
			   /* out: 0 or error number */
	enum ha_extra_function operation)
                           /* in: HA_EXTRA_DONT_USE_CURSOR_TO_UPDATE */
{
	row_prebuilt_t*	prebuilt = (row_prebuilt_t*) innobase_prebuilt;

	/* Warning: since it is not sure that MySQL calls external_lock
	before calling this function, the trx field in prebuilt can be
	obsolete! */

	switch (operation) {
 		case HA_EXTRA_RESET:
  		case HA_EXTRA_RESET_STATE:
	        	prebuilt->read_just_key = 0;
    	        	break;
		case HA_EXTRA_NO_KEYREAD:
    			prebuilt->read_just_key = 0;
    			break;
	        case HA_EXTRA_DONT_USE_CURSOR_TO_UPDATE:
			prebuilt->hint_no_need_to_fetch_extra_cols = FALSE;
			break;
	        case HA_EXTRA_KEYREAD:
	        	prebuilt->read_just_key = 1;
	        	break;
		default:/* Do nothing */
			;
	}

	return(0);
}

/**********************************************************************
????????????? */

int
ha_innobase::reset(void)
/*====================*/
{
  	return(0);
}

/**********************************************************************
MySQL calls this function at the start of each SQL statement inside LOCK
TABLES. Inside LOCK TABLES the ::external_lock method does not work to
mark SQL statement borders. Note also a special case: if a temporary table
is created inside LOCK TABLES, MySQL has not called external_lock() at all
on that table. */

int
ha_innobase::start_stmt(
/*====================*/
	              /* out: 0 or error code */
	THD*    thd)  /* in: handle to the user thread */
{
	row_prebuilt_t* prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	trx_t*		trx;

	update_thd(thd);

	trx = prebuilt->trx;

	/* Here we release the search latch and the InnoDB thread FIFO ticket
	if they were reserved. They should have been released already at the
	end of the previous statement, but because inside LOCK TABLES the
	lock count method does not work to mark the end of a SELECT statement,
	that may not be the case. We MUST release the search latch before an
	INSERT, for example. */

	innobase_release_stat_resources(trx);

	if (trx->isolation_level <= TRX_ISO_READ_COMMITTED
	    						&& trx->read_view) {
	    	/* At low transaction isolation levels we let
		each consistent read set its own snapshot */

	    	read_view_close_for_mysql(trx);
	}

	auto_inc_counter_for_this_stat = 0;
	prebuilt->sql_stat_start = TRUE;
	prebuilt->hint_no_need_to_fetch_extra_cols = TRUE;
	prebuilt->read_just_key = 0;

	if (!prebuilt->mysql_has_locked) {
	        /* This handle is for a temporary table created inside
	        this same LOCK TABLES; since MySQL does NOT call external_lock
	        in this case, we must use x-row locks inside InnoDB to be
	        prepared for an update of a row */
	  
	        prebuilt->select_lock_type = LOCK_X;
	}
	
	/* Set the MySQL flag to mark that there is an active transaction */
	thd->transaction.all.innodb_active_trans = 1;

	return(0);
}

/**********************************************************************
Maps a MySQL trx isolation level code to the InnoDB isolation level code */
inline
ulint
innobase_map_isolation_level(
/*=========================*/
					/* out: InnoDB isolation level */
	enum_tx_isolation	iso)	/* in: MySQL isolation level code */
{
	switch(iso) {
		case ISO_REPEATABLE_READ: return(TRX_ISO_REPEATABLE_READ);
		case ISO_READ_COMMITTED: return(TRX_ISO_READ_COMMITTED);
		case ISO_SERIALIZABLE: return(TRX_ISO_SERIALIZABLE);
		case ISO_READ_UNCOMMITTED: return(TRX_ISO_READ_UNCOMMITTED);
		default: ut_a(0); return(0);
	}	
}
	
/**********************************************************************
As MySQL will execute an external lock for every new table it uses when it
starts to process an SQL statement (an exception is when MySQL calls
start_stmt for the handle) we can use this function to store the pointer to
the THD in the handle. We will also use this function to communicate
to InnoDB that a new SQL statement has started and that we must store a
savepoint to our transaction handle, so that we are able to roll back
the SQL statement in case of an error. */

int
ha_innobase::external_lock(
/*=======================*/
			        /* out: 0 or error code */
	THD*	thd,		/* in: handle to the user thread */
	int 	lock_type)	/* in: lock type */
{
	row_prebuilt_t* prebuilt = (row_prebuilt_t*) innobase_prebuilt;
	int 		error = 0;
	trx_t*		trx;

  	DBUG_ENTER("ha_innobase::external_lock");
	DBUG_PRINT("enter",("lock_type: %d", lock_type));

	update_thd(thd);

	trx = prebuilt->trx;

	prebuilt->sql_stat_start = TRUE;
	prebuilt->hint_no_need_to_fetch_extra_cols = TRUE;

	prebuilt->read_just_key = 0;

	if (lock_type == F_WRLCK) {

		/* If this is a SELECT, then it is in UPDATE TABLE ...
		or SELECT ... FOR UPDATE */
		prebuilt->select_lock_type = LOCK_X;
	}

	if (lock_type != F_UNLCK) {
		/* MySQL is setting a new table lock */

		/* Set the MySQL flag to mark that there is an active
		transaction */
		thd->transaction.all.innodb_active_trans = 1;

		trx->n_mysql_tables_in_use++;
		prebuilt->mysql_has_locked = TRUE;

		if (trx->n_mysql_tables_in_use == 1) {
		        trx->isolation_level = innobase_map_isolation_level(
						(enum_tx_isolation)
						thd->variables.tx_isolation);
		}

		if (trx->isolation_level == TRX_ISO_SERIALIZABLE
		    && prebuilt->select_lock_type == LOCK_NONE) {

		    	/* To get serializable execution we let InnoDB
		    	conceptually add 'LOCK IN SHARE MODE' to all SELECTs
			which otherwise would have been consistent reads */

			prebuilt->select_lock_type = LOCK_S;
		}

		if (prebuilt->select_lock_type != LOCK_NONE) {

		  	trx->mysql_n_tables_locked++;
		}

		DBUG_RETURN(error);
	}

	/* MySQL is releasing a table lock */

	trx->n_mysql_tables_in_use--;
	prebuilt->mysql_has_locked = FALSE;
	auto_inc_counter_for_this_stat = 0;

	/* If the MySQL lock count drops to zero we know that the current SQL
	statement has ended */

	if (trx->n_mysql_tables_in_use == 0) {

	        trx->mysql_n_tables_locked = 0;
		prebuilt->used_in_HANDLER = FALSE;
			
		/* Release a possible FIFO ticket and search latch. Since we
		may reserve the kernel mutex, we have to release the search
		system latch first to obey the latching order. */

	        innobase_release_stat_resources(trx);

		if (!(thd->options
				 & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN))) {
			if (thd->transaction.all.innodb_active_trans != 0) {
		    	        innobase_commit(thd, trx);
			}
		} else {
			if (trx->isolation_level <= TRX_ISO_READ_COMMITTED
	    						&& trx->read_view) {

				/* At low transaction isolation levels we let
				each consistent read set its own snapshot */

				read_view_close_for_mysql(trx);
			}
		}
	}

	DBUG_RETURN(error);
}

/****************************************************************************
Implements the SHOW INNODB STATUS command. Send the output of the InnoDB
Monitor to the client. */

int
innodb_show_status(
/*===============*/
	THD*	thd)	/* in: the MySQL query thread of the caller */
{
	String* 	packet 	= &thd->packet;
	char*		buf;
	trx_t*		trx;

  	DBUG_ENTER("innodb_show_status");
	
	if (innodb_skip) {
	        my_message(ER_NOT_SUPPORTED_YET,
	  "Cannot call SHOW INNODB STATUS because skip-innodb is defined",
			   MYF(0));
                DBUG_RETURN(-1);
        }

	trx = check_trx_exists(thd);

	innobase_release_stat_resources(trx);

	/* We let the InnoDB Monitor to output at most 60 kB of text, add
	a safety margin of 100 kB for buffer overruns */

	buf = (char*)ut_malloc(160 * 1024);
	
	srv_sprintf_innodb_monitor(buf, 60 * 1024);
	
	List<Item> field_list;

	field_list.push_back(new Item_empty_string("Status", strlen(buf)));

	if (send_fields(thd, field_list, 1)) {

		ut_free(buf);

	  	DBUG_RETURN(-1);
	}

  	packet->length(0);
  
  	net_store_data(packet, buf);
  
  	if (my_net_write(&thd->net, (char*)thd->packet.ptr(),
						packet->length())) {
		ut_free(buf);
	
    		DBUG_RETURN(-1);
    	}

	ut_free(buf);

  	send_eof(&thd->net);

  	DBUG_RETURN(0);
}

/****************************************************************************
 Handling the shared INNOBASE_SHARE structure that is needed to provide table
 locking.
****************************************************************************/

static mysql_byte* innobase_get_key(INNOBASE_SHARE *share,uint *length,
			      my_bool not_used __attribute__((unused)))
{
  *length=share->table_name_length;
  return (mysql_byte*) share->table_name;
}

static INNOBASE_SHARE *get_share(const char *table_name)
{
  INNOBASE_SHARE *share;
  pthread_mutex_lock(&innobase_mutex);
  uint length=(uint) strlen(table_name);
  if (!(share=(INNOBASE_SHARE*) hash_search(&innobase_open_tables,
					(mysql_byte*) table_name,
					    length)))
  {
    if ((share=(INNOBASE_SHARE *) my_malloc(sizeof(*share)+length+1,
				       MYF(MY_WME | MY_ZEROFILL))))
    {
      share->table_name_length=length;
      share->table_name=(char*) (share+1);
      strmov(share->table_name,table_name);
      if (hash_insert(&innobase_open_tables, (mysql_byte*) share))
      {
	pthread_mutex_unlock(&innobase_mutex);
	my_free((gptr) share,0);
	return 0;
      }
      thr_lock_init(&share->lock);
      pthread_mutex_init(&share->mutex,MY_MUTEX_INIT_FAST);
    }
  }
  share->use_count++;
  pthread_mutex_unlock(&innobase_mutex);
  return share;
}

static void free_share(INNOBASE_SHARE *share)
{
  pthread_mutex_lock(&innobase_mutex);
  if (!--share->use_count)
  {
    hash_delete(&innobase_open_tables, (mysql_byte*) share);
    thr_lock_delete(&share->lock);
    pthread_mutex_destroy(&share->mutex);
    my_free((gptr) share, MYF(0));
  }
  pthread_mutex_unlock(&innobase_mutex);
}

/*********************************************************************
Converts a MySQL table lock stored in the 'lock' field of the handle to
a proper type before storing pointer to the lock into an array of pointers.
MySQL also calls this if it wants to reset some table locks to a not-locked
state during the processing of an SQL query. An example is that during a
SELECT the read lock is released early on the 'const' tables where we only
fetch one row. MySQL does not call this when it releases all locks at the
end of an SQL statement. */

THR_LOCK_DATA**
ha_innobase::store_lock(
/*====================*/
						/* out: pointer to the next
						element in the 'to' array */
	THD*			thd,		/* in: user thread handle */
	THR_LOCK_DATA**		to,		/* in: pointer to an array
						of pointers to lock structs;
						pointer to the 'lock' field
						of current handle is stored
						next to this array */
	enum thr_lock_type 	lock_type)	/* in: lock type to store in
						'lock' */
{
	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;

	if (lock_type == TL_READ_WITH_SHARED_LOCKS ||
	    lock_type == TL_READ_NO_INSERT) {
		/* This is a SELECT ... IN SHARE MODE, or
		we are doing a complex SQL statement like
		INSERT INTO ... SELECT ... and the logical logging (MySQL
		binlog) requires the use of a locking read */

		prebuilt->select_lock_type = LOCK_S;
	} else if (lock_type != TL_IGNORE) {

	        /* In ha_berkeley.cc there is a comment that MySQL
	        may in exceptional cases call this with TL_IGNORE also
	        when it is NOT going to release the lock. */

	        /* We set possible LOCK_X value in external_lock, not yet
		here even if this would be SELECT ... FOR UPDATE */

		prebuilt->select_lock_type = LOCK_NONE;
	}

	if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK) {

    		/* If we are not doing a LOCK TABLE, then allow multiple
		writers */

    		if ((lock_type >= TL_WRITE_CONCURRENT_INSERT &&
	 	    lock_type <= TL_WRITE) && !thd->in_lock_tables) {

      			lock_type = TL_WRITE_ALLOW_WRITE;
      		}

		/* In queries of type INSERT INTO t1 SELECT ... FROM t2 ...
		MySQL would use the lock TL_READ_NO_INSERT on t2, and that
		would conflict with TL_WRITE_ALLOW_WRITE, blocking all inserts
		to t2. Convert the lock to a normal read lock to allow
		concurrent inserts to t2. */
      		
		if (lock_type == TL_READ_NO_INSERT && !thd->in_lock_tables) {
			lock_type = TL_READ;
		}
		
 		lock.type=lock_type;
  	}

  	*to++= &lock;

	return(to);
}

/***********************************************************************
This function initializes the auto-inc counter if it has not been
initialized yet. This function does not change the value of the auto-inc
counter if it already has been initialized. In parameter ret returns
the value of the auto-inc counter. */

int
ha_innobase::innobase_read_and_init_auto_inc(
/*=========================================*/
				/* out: 0 or error code: deadlock or
				lock wait timeout */
	longlong*	ret)	/* out: auto-inc value */
{
  	row_prebuilt_t* prebuilt	= (row_prebuilt_t*) innobase_prebuilt;
    	longlong        auto_inc;
  	int     	error;

  	ut_a(prebuilt);
	ut_a(prebuilt->trx ==
		(trx_t*) current_thd->transaction.all.innobase_tid);
	ut_a(prebuilt->table);
	
	/* In case MySQL calls this in the middle of a SELECT query, release
	possible adaptive hash latch to avoid deadlocks of threads */

	trx_search_latch_release_if_reserved(prebuilt->trx);

	auto_inc = dict_table_autoinc_read(prebuilt->table);

	if (auto_inc != 0) {
		/* Already initialized */
		*ret = auto_inc;
	
		return(0);
	}

	error = row_lock_table_autoinc_for_mysql(prebuilt);

	if (error != DB_SUCCESS) {
		error = convert_error_code_to_mysql(error, user_thd);

		goto func_exit;
	}	

	/* Check again if someone has initialized the counter meanwhile */
	auto_inc = dict_table_autoinc_read(prebuilt->table);

	if (auto_inc != 0) {
		*ret = auto_inc;
	
		return(0);
	}

  	(void) extra(HA_EXTRA_KEYREAD);
  	index_init(table->next_number_index);

	/* We use an exclusive lock when we read the max key value from the
  	auto-increment column index. This is because then build_template will
  	advise InnoDB to fetch all columns. In SHOW TABLE STATUS the query
  	id of the auto-increment column is not changed, and previously InnoDB
  	did not fetch it, causing SHOW TABLE STATUS to show wrong values
  	for the autoinc column. */

  	prebuilt->select_lock_type = LOCK_X;

  	/* Play safe and also give in another way the hint to fetch
  	all columns in the key: */
  	
	prebuilt->hint_no_need_to_fetch_extra_cols = FALSE;

	prebuilt->trx->mysql_n_tables_locked += 1;
  
	error = index_last(table->record[1]);

  	if (error) {
		if (error == HA_ERR_END_OF_FILE) {
			/* The table was empty, initialize to 1 */
			auto_inc = 1;

			error = 0;
		} else {
			/* Deadlock or a lock wait timeout */
  			auto_inc = -1;

  			goto func_exit;
  		}
  	} else {
		/* Initialize to max(col) + 1 */
    		auto_inc = (longlong) table->next_number_field->
                        	val_int_offset(table->rec_buff_length) + 1;
  	}

	dict_table_autoinc_initialize(prebuilt->table, auto_inc);

func_exit:
  	(void) extra(HA_EXTRA_NO_KEYREAD);

	index_end();

	*ret = auto_inc;

  	return(error);
}

/***********************************************************************
This function initializes the auto-inc counter if it has not been
initialized yet. This function does not change the value of the auto-inc
counter if it already has been initialized. Returns the value of the
auto-inc counter. */

longlong
ha_innobase::get_auto_increment()
/*=============================*/
                         /* out: auto-increment column value, -1 if error
                         (deadlock or lock wait timeout) */
{
  	longlong        nr;
  	int     	error;
	
	error = innobase_read_and_init_auto_inc(&nr);

	if (error) {

		return(-1);
	}

	return(nr);
}

#endif /* HAVE_INNOBASE_DB */

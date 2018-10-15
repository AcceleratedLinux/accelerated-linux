/******************************************************
The transaction

(c) 1996 Innobase Oy

Created 3/26/1996 Heikki Tuuri
*******************************************************/

#ifndef trx0trx_h
#define trx0trx_h

#include "univ.i"
#include "trx0types.h"
#include "lock0types.h"
#include "usr0types.h"
#include "que0types.h"
#include "mem0mem.h"
#include "read0types.h"

extern ulint	trx_n_mysql_transactions;

/************************************************************************
Releases the search latch if trx has reserved it. */

void
trx_search_latch_release_if_reserved(
/*=================================*/
        trx_t*     trx); /* in: transaction */
/********************************************************************
Retrieves the error_info field from a trx. */

void*
trx_get_error_info(
/*===============*/
		     /* out: the error info */
	trx_t*  trx); /* in: trx object */
/********************************************************************
Creates and initializes a transaction object. */

trx_t*
trx_create(
/*=======*/
			/* out, own: the transaction */
	sess_t*	sess);	/* in: session or NULL */
/************************************************************************
Creates a transaction object for MySQL. */

trx_t*
trx_allocate_for_mysql(void);
/*========================*/
				/* out, own: transaction object */
/************************************************************************
Creates a transaction object for background operations by the master thread. */

trx_t*
trx_allocate_for_background(void);
/*=============================*/
				/* out, own: transaction object */
/************************************************************************
Frees a transaction object. */

void
trx_free(
/*=====*/
	trx_t*	trx);	/* in, own: trx object */
/************************************************************************
Frees a transaction object for MySQL. */

void
trx_free_for_mysql(
/*===============*/
	trx_t*	trx);	/* in, own: trx object */
/************************************************************************
Frees a transaction object of a background operation of the master thread. */

void
trx_free_for_background(
/*====================*/
	trx_t*	trx);	/* in, own: trx object */
/********************************************************************
Creates trx objects for transactions and initializes the trx list of
trx_sys at database start. Rollback segment and undo log lists must
already exist when this function is called, because the lists of
transactions to be rolled back or cleaned up are built based on the
undo log lists. */

void
trx_lists_init_at_db_start(void);
/*============================*/
/********************************************************************
Starts a new transaction. */

ibool
trx_start(
/*======*/
			/* out: TRUE if success, FALSE if the rollback
			segment could not support this many transactions */
	trx_t* 	trx,	/* in: transaction */
	ulint	rseg_id);/* in: rollback segment id; if ULINT_UNDEFINED
			is passed, the system chooses the rollback segment
			automatically in a round-robin fashion */
/********************************************************************
Starts a new transaction. */

ibool
trx_start_low(
/*==========*/
			/* out: TRUE */
	trx_t* 	trx,	/* in: transaction */
	ulint	rseg_id);/* in: rollback segment id; if ULINT_UNDEFINED
			is passed, the system chooses the rollback segment
			automatically in a round-robin fashion */
/*****************************************************************
Starts the transaction if it is not yet started. */
UNIV_INLINE
void
trx_start_if_not_started(
/*=====================*/
	trx_t*	trx);	/* in: transaction */
/*****************************************************************
Starts the transaction if it is not yet started. Assumes we have reserved
the kernel mutex! */
UNIV_INLINE
void
trx_start_if_not_started_low(
/*=========================*/
	trx_t*	trx);	/* in: transaction */
/*****************************************************************
Starts the transaction if it is not yet started. */

void
trx_start_if_not_started_noninline(
/*===============================*/
	trx_t*	trx);	/* in: transaction */
/********************************************************************
Commits a transaction. */

void
trx_commit_off_kernel(
/*==================*/
	trx_t*	trx);	/* in: transaction */
/********************************************************************
Cleans up a transaction at database startup. The cleanup is needed if
the transaction already got to the middle of a commit when the database
crashed, andf we cannot roll it back. */

void
trx_cleanup_at_db_startup(
/*======================*/
	trx_t*	trx);	/* in: transaction */
/**************************************************************************
Does the transaction commit for MySQL. */

ulint
trx_commit_for_mysql(
/*=================*/
			/* out: 0 or error number */
	trx_t*	trx);	/* in: trx handle */
/**************************************************************************
If required, flushes the log to disk if we called trx_commit_for_mysql()
with trx->flush_log_later == TRUE. */

ulint
trx_commit_complete_for_mysql(
/*==========================*/
			/* out: 0 or error number */
	trx_t*	trx);	/* in: trx handle */
/**************************************************************************
Marks the latest SQL statement ended. */

void
trx_mark_sql_stat_end(
/*==================*/
	trx_t*	trx);	/* in: trx handle */
/************************************************************************
Assigns a read view for a consistent read query. All the consistent reads
within the same transaction will get the same read view, which is created
when this function is first called for a new started transaction. */

read_view_t*
trx_assign_read_view(
/*=================*/
			/* out: consistent read view */
	trx_t*	trx);	/* in: active transaction */
/***************************************************************
The transaction must be in the TRX_QUE_LOCK_WAIT state. Puts it to
the TRX_QUE_RUNNING state and releases query threads which were
waiting for a lock in the wait_thrs list. */

void
trx_end_lock_wait(
/*==============*/
	trx_t*	trx);	/* in: transaction */
/********************************************************************
Sends a signal to a trx object. */

ibool
trx_sig_send(
/*=========*/
					/* out: TRUE if the signal was
					successfully delivered */
	trx_t*		trx,		/* in: trx handle */
	ulint		type,		/* in: signal type */
	ulint		sender,		/* in: TRX_SIG_SELF or
					TRX_SIG_OTHER_SESS */
	ibool		reply,		/* in: TRUE if the sender of the signal
					wants reply after the operation induced
					by the signal is completed; if type
					is TRX_SIG_END_WAIT, this must be
					FALSE */
	que_thr_t*	receiver_thr,	/* in: query thread which wants the
					reply, or NULL */
	trx_savept_t* 	savept,		/* in: possible rollback savepoint, or
					NULL */
	que_thr_t**	next_thr);	/* in/out: next query thread to run;
					if the value which is passed in is
					a pointer to a NULL pointer, then the
					calling function can start running
					a new query thread; if the parameter
					is NULL, it is ignored */
/********************************************************************
Send the reply message when a signal in the queue of the trx has
been handled. */

void
trx_sig_reply(
/*==========*/
	trx_t*		trx,		/* in: trx handle */
	trx_sig_t*	sig,		/* in: signal */
	que_thr_t**	next_thr);	/* in/out: next query thread to run;
					if the value which is passed in is
					a pointer to a NULL pointer, then the
					calling function can start running
					a new query thread */
/********************************************************************
Removes the signal object from a trx signal queue. */

void
trx_sig_remove(
/*===========*/
	trx_t*		trx,	/* in: trx handle */
	trx_sig_t*	sig);	/* in, own: signal */
/********************************************************************
Starts handling of a trx signal. */

void
trx_sig_start_handle(
/*=================*/
	trx_t*		trx,		/* in: trx handle */
	que_thr_t**	next_thr);	/* in/out: next query thread to run;
					if the value which is passed in is
					a pointer to a NULL pointer, then the
					calling function can start running
					a new query thread */
/********************************************************************
Ends signal handling. If the session is in the error state, and
trx->graph_before_signal_handling != NULL, returns control to the error
handling routine of the graph (currently only returns the control to the
graph root which then sends an error message to the client). */

void
trx_end_signal_handling(
/*====================*/
	trx_t*	trx);	/* in: trx */
/*************************************************************************
Creates a commit command node struct. */

commit_node_t*
commit_node_create(
/*===============*/
				/* out, own: commit node struct */
	mem_heap_t*	heap);	/* in: mem heap where created */
/***************************************************************
Performs an execution step for a commit type node in a query graph. */

que_thr_t*
trx_commit_step(
/*============*/
				/* out: query thread to run next, or NULL */
	que_thr_t*	thr);	/* in: query thread */
/**************************************************************************
Prints info about a transaction to the standard output. The caller must
own the kernel mutex. */

void
trx_print(
/*======*/
	char*	buf,	/* in/out: buffer where to print, must be at least
			800 bytes */
	trx_t* trx); 	/* in: transaction */


/* Signal to a transaction */
struct trx_sig_struct{
	ulint		type;		/* signal type */
	ulint		state;		/* TRX_SIG_WAITING or
					TRX_SIG_BEING_HANDLED */
	ulint		sender;		/* TRX_SIG_SELF or
					TRX_SIG_OTHER_SESS */
	ibool		reply;		/* TRUE if the sender of the signal
					wants reply after the operation induced
					by the signal is completed; if this
					field is TRUE and the receiver field
					below is NULL, then a SUCCESS message
					is sent to the client of the session
					to which this trx belongs */
	que_thr_t*	receiver;	/* query thread which wants the reply,
					or NULL */
	trx_savept_t	savept;		/* possible rollback savepoint */
	UT_LIST_NODE_T(trx_sig_t)
			signals;	/* queue of pending signals to the
					transaction */
	UT_LIST_NODE_T(trx_sig_t)
			reply_signals;	/* list of signals for which the sender
					transaction is waiting a reply */
};

#define TRX_MAGIC_N	91118598

/* The transaction handle; every session has a trx object which is freed only
when the session is freed; in addition there may be session-less transactions
rolling back after a database recovery */

struct trx_struct{
	ulint		magic_n;
	/* All the next fields are protected by the kernel mutex, except the
	undo logs which are protected by undo_mutex */
	char*		op_info;	/* English text describing the
					current operation, or an empty
					string */
	ulint		type;		/* TRX_USER, TRX_PURGE */
	ulint		conc_state;	/* state of the trx from the point
					of view of concurrency control:
					TRX_ACTIVE, TRX_COMMITTED_IN_MEMORY,
					... */
        time_t          start_time;     /* time the trx object was created
                                        or the state last time became
                                        TRX_ACTIVE */
	ulint		isolation_level;/* TRX_ISO_REPEATABLE_READ, ... */
	ibool		check_foreigns;	/* normally TRUE, but if the user
					wants to suppress foreign key checks,
					(in table imports, for example) we
					set this FALSE */
	ibool		check_unique_secondary;
					/* normally TRUE, but if the user
					wants to speed up inserts by
					suppressing unique key checks
					for secondary indexes when we decide
					if we can use the insert buffer for
					them, we set this FALSE */
	dulint		id;		/* transaction id */
	dulint		no;		/* transaction serialization number ==
					max trx id when the transaction is 
					moved to COMMITTED_IN_MEMORY state */
	ibool		flush_log_later;/* when we commit the transaction
					in MySQL's binlog write, we will
					flush the log to disk later in
					a separate call */
	dulint		commit_lsn;	/* lsn at the time of the commit */
	ibool		dict_operation;	/* TRUE if the trx is used to create
					a table, create an index, or drop a
					table */
	dulint		table_id;	/* table id if the preceding field is
					TRUE */
	/*------------------------------*/
        void*           mysql_thd;      /* MySQL thread handle corresponding
                                        to this trx, or NULL */
	char**		mysql_query_str;/* pointer to the field in mysqld_thd
					which contains the pointer to the
					current SQL query string */
	char*		mysql_log_file_name;
					/* if MySQL binlog is used, this field
					contains a pointer to the latest file
					name; this is NULL if binlog is not
					used */
	ib_longlong	mysql_log_offset;/* if MySQL binlog is used, this field
					contains the end offset of the binlog
					entry */
	char*		mysql_master_log_file_name;
					/* if the database server is a MySQL
					replication slave, we have here the
					master binlog name up to which
					replication has processed; otherwise
					this is a pointer to a null
					character */
	ib_longlong	mysql_master_log_pos;
					/* if the database server is a MySQL
					replication slave, this is the
					position in the log file up to which
					replication has processed */
	os_thread_id_t	mysql_thread_id;/* id of the MySQL thread associated
					with this transaction object */
	ulint		mysql_process_no;/* since in Linux, 'top' reports
					process id's and not thread id's, we
					store the process number too */
	/*------------------------------*/
	ulint		n_mysql_tables_in_use; /* number of Innobase tables
					used in the processing of the current
					SQL statement in MySQL */
        ulint           mysql_n_tables_locked;
                                        /* how many tables the current SQL
					statement uses, except those
					in consistent read */
	ibool		dict_operation_lock_mode;
					/* 0, RW_S_LATCH, or RW_X_LATCH:
					the latch mode trx currently holds
					on dict_operation_lock */
        ibool           has_search_latch;
			                /* TRUE if this trx has latched the
			                search system latch in S-mode */
	ulint		search_latch_timeout;
					/* If we notice that someone is
					waiting for our S-lock on the search
					latch to be released, we wait in
					row0sel.c for BTR_SEA_TIMEOUT new
					searches until we try to keep
					the search latch again over
					calls from MySQL; this is intended
					to reduce contention on the search
					latch */
	/*------------------------------*/
	ibool		declared_to_be_inside_innodb;
					/* this is TRUE if we have declared
					this transaction in
					srv_conc_enter_innodb to be inside the
					InnoDB engine */
	ulint		n_tickets_to_enter_innodb;
					/* this can be > 0 only when
					declared_to_... is TRUE; when we come
					to srv_conc_innodb_enter, if the value
					here is > 0, we decrement this by 1 */ 
	/*------------------------------*/
	lock_t*		auto_inc_lock;	/* possible auto-inc lock reserved by
					the transaction; note that it is also
					in the lock list trx_locks */
	UT_LIST_NODE_T(trx_t)
			trx_list;	/* list of transactions */
	UT_LIST_NODE_T(trx_t)
			mysql_trx_list;	/* list of transactions created for
					MySQL */
	/*------------------------------*/
	ulint		error_state;	/* 0 if no error, otherwise error
					number; NOTE That ONLY the thread
					doing the transaction is allowed to
					set this field: this is NOT protected
					by the kernel mutex */
	void*		error_info;	/* if the error number indicates a
					duplicate key error, a pointer to
					the problematic index is stored here */
	sess_t*		sess;		/* session of the trx, NULL if none */
 	ulint		que_state;	/* TRX_QUE_RUNNING, TRX_QUE_LOCK_WAIT,
					... */
	que_t*		graph;		/* query currently run in the session,
					or NULL if none; NOTE that the query
					belongs to the session, and it can
					survive over a transaction commit, if
					it is a stored procedure with a COMMIT
					WORK statement, for instance */
	ulint		n_active_thrs;	/* number of active query threads */
	ibool		handling_signals;/* this is TRUE as long as the trx
					is handling signals */
	que_t*		graph_before_signal_handling;
					/* value of graph when signal handling
					for this trx started: this is used to
					return control to the original query
					graph for error processing */
	trx_sig_t	sig;		/* one signal object can be allocated
					in this space, avoiding mem_alloc */
	UT_LIST_BASE_NODE_T(trx_sig_t)
			signals;	/* queue of processed or pending
					signals to the trx */
	UT_LIST_BASE_NODE_T(trx_sig_t)
			reply_signals;	/* list of signals sent by the query
					threads of this trx for which a thread
					is waiting for a reply; if this trx is
					killed, the reply requests in the list
					must be canceled */
	/*------------------------------*/
	lock_t*		wait_lock;	/* if trx execution state is
					TRX_QUE_LOCK_WAIT, this points to
					the lock request, otherwise this is
					NULL */
	ibool		was_chosen_as_deadlock_victim;
					/* when the transaction decides to wait
					for a lock, this it sets this to FALSE;
					if another transaction chooses this
					transaction as a victim in deadlock
					resolution, it sets this to TRUE */
	time_t          wait_started;   /* lock wait started at this time */
	UT_LIST_BASE_NODE_T(que_thr_t)
			wait_thrs;	/* query threads belonging to this
					trx that are in the QUE_THR_LOCK_WAIT
					state */
	ulint		deadlock_mark;	/* a mark field used in deadlock
					checking algorithm */
	/*------------------------------*/
	mem_heap_t*	lock_heap;	/* memory heap for the locks of the
					transaction */
	UT_LIST_BASE_NODE_T(lock_t) 
			trx_locks;	/* locks reserved by the transaction */
	/*------------------------------*/
	mem_heap_t*	read_view_heap;	/* memory heap for the read view */
	read_view_t*	read_view;	/* consistent read view or NULL */
	/*------------------------------*/
	UT_LIST_BASE_NODE_T(trx_named_savept_t) 
			trx_savepoints;	/* savepoints set with SAVEPOINT ...,
					oldest first */
	/*------------------------------*/
	mutex_t		undo_mutex;	/* mutex protecting the fields in this
					section (down to undo_no_arr), EXCEPT
					last_sql_stat_start, which can be
					accessed only when we know that there
					cannot be any activity in the undo
					logs! */
	dulint		undo_no;	/* next undo log record number to
					assign */
	trx_savept_t	last_sql_stat_start;
					/* undo_no when the last sql statement
					was started: in case of an error, trx
					is rolled back down to this undo
					number; see note at undo_mutex! */
	trx_rseg_t*	rseg;		/* rollback segment assigned to the
					transaction, or NULL if not assigned
					yet */
	trx_undo_t*	insert_undo;	/* pointer to the insert undo log, or 
					NULL if no inserts performed yet */
	trx_undo_t* 	update_undo;	/* pointer to the update undo log, or
					NULL if no update performed yet */
	dulint		roll_limit;	/* least undo number to undo during
					a rollback */
	ulint		pages_undone;	/* number of undo log pages undone
					since the last undo log truncation */
	trx_undo_arr_t*	undo_no_arr;	/* array of undo numbers of undo log
					records which are currently processed
					by a rollback operation */
};

#define TRX_MAX_N_THREADS	32	/* maximum number of concurrent
					threads running a single operation of
					a transaction, e.g., a parallel query */
/* Transaction types */
#define	TRX_USER		1	/* normal user transaction */
#define	TRX_PURGE		2	/* purge transaction: this is not
					inserted to the trx list of trx_sys
					and no rollback segment is assigned to
					this */
/* Transaction concurrency states */
#define	TRX_NOT_STARTED		1
#define	TRX_ACTIVE		2
#define	TRX_COMMITTED_IN_MEMORY	3

/* Transaction execution states when trx state is TRX_ACTIVE */
#define TRX_QUE_RUNNING		1	/* transaction is running */
#define TRX_QUE_LOCK_WAIT	2	/* transaction is waiting for a lock */
#define TRX_QUE_ROLLING_BACK	3	/* transaction is rolling back */
#define TRX_QUE_COMMITTING	4	/* transaction is committing */

/* Transaction isolation levels */
#define TRX_ISO_READ_UNCOMMITTED	1	/* dirty read: non-locking
						SELECTs are performed so that
						we do not look at a possible
						earlier version of a record;
						thus they are not 'consistent'
						reads under this isolation
						level; otherwise like level
						2 */

#define TRX_ISO_READ_COMMITTED		2	/* somewhat Oracle-like
						isolation, except that in
						range UPDATE and DELETE we
						must block phantom rows
						with next-key locks;
						SELECT ... FOR UPDATE and ...
						LOCK IN SHARE MODE only lock
						the index records, NOT the
						gaps before them, and thus
						allow free inserting;
						each consistent read reads its
						own snapshot */

#define TRX_ISO_REPEATABLE_READ		3	/* this is the default;
						all consistent reads in the
						same trx read the same
						snapshot;
						full next-key locking used
						in locking reads to block
						insertions into gaps */

#define TRX_ISO_SERIALIZABLE		4	/* all plain SELECTs are
						converted to LOCK IN SHARE
						MODE reads */

/* Types of a trx signal */
#define TRX_SIG_NO_SIGNAL		100
#define TRX_SIG_TOTAL_ROLLBACK		1
#define TRX_SIG_ROLLBACK_TO_SAVEPT	2
#define TRX_SIG_COMMIT			3
#define	TRX_SIG_ERROR_OCCURRED		4
#define TRX_SIG_BREAK_EXECUTION		5

/* Sender types of a signal */
#define TRX_SIG_SELF		1	/* sent by the session itself, or
					by an error occurring within this
					session */
#define TRX_SIG_OTHER_SESS	2	/* sent by another session (which
					must hold rights to this) */
/* Signal states */
#define	TRX_SIG_WAITING		1
#define TRX_SIG_BEING_HANDLED	2
					
/* Commit command node in a query graph */
struct commit_node_struct{
	que_common_t	common;	/* node type: QUE_NODE_COMMIT */
	ulint		state;	/* node execution state */
};

/* Commit node states */
#define COMMIT_NODE_SEND	1
#define COMMIT_NODE_WAIT	2


#ifndef UNIV_NONINL
#include "trx0trx.ic"
#endif

#endif 

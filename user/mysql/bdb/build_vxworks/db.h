/* DO NOT EDIT: automatically built by dist/s_vxworks. */
/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 *	$Id: db.src,v 11.121 2001/01/10 15:43:08 sue Exp $
 */

#ifndef _DB_H_
#define	_DB_H_

#ifndef __NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <stdio.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * XXX
 * Handle function prototypes and the keyword "const".  This steps on name
 * space that DB doesn't control, but all of the other solutions are worse.
 *
 * XXX
 * While Microsoft's compiler is ANSI C compliant, it doesn't have _STDC_
 * defined by default, you specify a command line flag or #pragma to turn
 * it on.  Don't do that, however, because some of Microsoft's own header
 * files won't compile.
 */
#undef	__P
#if defined(__STDC__) || defined(__cplusplus) || defined(_MSC_VER)
#define	__P(protos)	protos		/* ANSI C prototypes */
#else
#define	const
#define	__P(protos)	()		/* K&R C preprocessor */
#endif

/*
 * !!!
 * DB needs basic information about specifically sized types.  If they're
 * not provided by the system, typedef them here.
 *
 * We protect them against multiple inclusion using __BIT_TYPES_DEFINED__,
 * as does BIND and Kerberos, since we don't know for sure what #include
 * files the user is using.
 *
 * !!!
 * We also provide the standard u_int, u_long etc., if they're not provided
 * by the system.
 */
#ifndef	__BIT_TYPES_DEFINED__
#define	__BIT_TYPES_DEFINED__
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
#endif


#define	DB_VERSION_MAJOR	3
#define	DB_VERSION_MINOR	2
#define	DB_VERSION_PATCH	9
#define	DB_VERSION_STRING	"Sleepycat Software: Berkeley DB 3.2.9a: (October 17, 2003)"

typedef	u_int32_t	db_pgno_t;	/* Page number type. */
typedef	u_int16_t	db_indx_t;	/* Page offset type. */
#define	DB_MAX_PAGES	0xffffffff	/* >= # of pages in a file */

typedef	u_int32_t	db_recno_t;	/* Record number type. */
#define	DB_MAX_RECORDS	0xffffffff	/* >= # of records in a tree */

/* Forward structure declarations, so applications get type checking. */
struct __db;		typedef struct __db DB;
#ifdef DB_DBM_HSEARCH
			typedef struct __db DBM;
#endif
struct __db_bt_stat;	typedef struct __db_bt_stat DB_BTREE_STAT;
struct __db_dbt;	typedef struct __db_dbt DBT;
struct __db_env;	typedef struct __db_env DB_ENV;
struct __db_h_stat;	typedef struct __db_h_stat DB_HASH_STAT;
struct __db_ilock;	typedef struct __db_ilock DB_LOCK_ILOCK;
struct __db_lock_stat;	typedef struct __db_lock_stat DB_LOCK_STAT;
struct __db_lock_u;	typedef struct __db_lock_u DB_LOCK;
struct __db_lockreq;	typedef struct __db_lockreq DB_LOCKREQ;
struct __db_log_stat;	typedef struct __db_log_stat DB_LOG_STAT;
struct __db_lsn;	typedef struct __db_lsn DB_LSN;
struct __db_mpool_finfo;typedef struct __db_mpool_finfo DB_MPOOL_FINFO;
struct __db_mpool_fstat;typedef struct __db_mpool_fstat DB_MPOOL_FSTAT;
struct __db_mpool_stat;	typedef struct __db_mpool_stat DB_MPOOL_STAT;
struct __db_mpoolfile;	typedef struct __db_mpoolfile DB_MPOOLFILE;
struct __db_qam_stat;	typedef struct __db_qam_stat DB_QUEUE_STAT;
struct __db_txn;	typedef struct __db_txn DB_TXN;
struct __db_txn_active;	typedef struct __db_txn_active DB_TXN_ACTIVE;
struct __db_txn_stat;	typedef struct __db_txn_stat DB_TXN_STAT;
struct __dbc;		typedef struct __dbc DBC;
struct __dbc_internal;	typedef struct __dbc_internal DBC_INTERNAL;
struct __fh_t;		typedef struct __fh_t DB_FH;
struct __key_range;	typedef struct __key_range DB_KEY_RANGE;

/* Key/data structure -- a Data-Base Thang. */
struct __db_dbt {
	/*
	 * data/size must be fields 1 and 2 for DB 1.85 compatibility.
	 */
	void	 *data;			/* Key/data */
	u_int32_t size;			/* key/data length */

	u_int32_t ulen;			/* RO: length of user buffer. */
	u_int32_t dlen;			/* RO: get/put record length. */
	u_int32_t doff;			/* RO: get/put record offset. */

	void	*app_private;		/* Application-private handle. */
#define	DB_DBT_ISSET	0x001		/* Lower level calls set value. */
#define	DB_DBT_MALLOC	0x002		/* Return in malloc'd memory. */
#define	DB_DBT_PARTIAL	0x004		/* Partial put/get. */
#define	DB_DBT_REALLOC	0x008		/* Return in realloc'd memory. */
#define	DB_DBT_USERMEM	0x010		/* Return in user's memory. */
#define	DB_DBT_DUPOK	0x020		/* Insert if duplicate. */
	u_int32_t flags;
};

/*
 * Common flags --
 *	Interfaces which use any of these common flags should never have
 *	interface specific flags in this range.
 */
#define	DB_CREATE	      0x000001	/* Create file as necessary. */
#define	DB_CXX_NO_EXCEPTIONS  0x000002	/* C++: return error values. */
#define	DB_FORCE	      0x000004	/* Force (anything). */
#define	DB_NOMMAP	      0x000008	/* Don't mmap underlying file. */
#define	DB_RDONLY	      0x000010	/* Read-only (O_RDONLY). */
#define	DB_RECOVER	      0x000020	/* Run normal recovery. */
#define	DB_THREAD	      0x000040	/* Applications are threaded. */
#define	DB_TXN_NOSYNC	      0x000080	/* Do not sync log on commit. */
#define	DB_USE_ENVIRON	      0x000100	/* Use the environment. */
#define	DB_USE_ENVIRON_ROOT   0x000200	/* Use the environment if root. */

/*
 * Flags private to db_env_create.
 */
#define	DB_CLIENT	      0x000400	/* Open for a client environment. */

/*
 * Flags private to db_create.
 */
#define	DB_XA_CREATE	      0x000400	/* Open in an XA environment. */

/*
 * Flags private to DBENV->open.
 */
#define	DB_INIT_CDB	      0x000400	/* Concurrent Access Methods. */
#define	DB_INIT_LOCK	      0x000800	/* Initialize locking. */
#define	DB_INIT_LOG	      0x001000	/* Initialize logging. */
#define	DB_INIT_MPOOL	      0x002000	/* Initialize mpool. */
#define	DB_INIT_TXN	      0x004000	/* Initialize transactions. */
#define	DB_JOINENV	      0x008000  /* Initialize all subsystems present. */
#define	DB_LOCKDOWN	      0x010000	/* Lock memory into physical core. */
#define	DB_PRIVATE	      0x020000	/* DB_ENV is process local. */
#define	DB_RECOVER_FATAL      0x040000	/* Run catastrophic recovery. */
#define	DB_SYSTEM_MEM	      0x080000	/* Use system-backed memory. */

/*
 * Flags private to DB->open.
 */
#define	DB_EXCL		      0x000400	/* Exclusive open (O_EXCL). */
#define	DB_FCNTL_LOCKING      0x000800	/* UNDOC: fcntl(2) locking. */
#define	DB_ODDFILESIZE	      0x001000  /* UNDOC: truncate to N * pgsize. */
#define	DB_RDWRMASTER	      0x002000  /* UNDOC: allow subdb master open R/W */
#define	DB_TRUNCATE	      0x004000	/* Discard existing DB (O_TRUNC). */
#define	DB_EXTENT	      0x008000  /* UNDOC: dealing with an extent. */

/*
 * Flags private to DBENV->txn_begin.
 */
#define	DB_TXN_NOWAIT	      0x000400	/* Do not wait for locks in this TXN. */
#define	DB_TXN_SYNC	      0x000800	/* Always sync log on commit. */

/*
 * Flags private to DBENV->set_flags.
 */
#define	DB_CDB_ALLDB	      0x000400	/* In CDB, lock across environment. */

/*
 * Flags private to DB->set_feedback's callback.
 */
#define	DB_UPGRADE	      0x000400	/* Upgrading. */
#define	DB_VERIFY	      0x000800  /* Verifying. */

/*
 * Flags private to DB->set_flags.
 *
 * DB->set_flags does not share common flags and so values start at 0x01.
 */
#define	DB_DUP			0x0001	/* Btree, Hash: duplicate keys. */
#define	DB_DUPSORT		0x0002	/* Btree, Hash: duplicate keys. */
#define	DB_RECNUM		0x0004	/* Btree: record numbers. */
#define	DB_RENUMBER		0x0008	/* Recno: renumber on insert/delete. */
#define	DB_REVSPLITOFF		0x0010	/* Btree: turn off reverse splits. */
#define	DB_SNAPSHOT		0x0020	/* Recno: snapshot the input. */

/*
 * Flags private to DB->join.
 *
 * DB->join does not share common flags and so values start at 0x01.
 */
#define	DB_JOIN_NOSORT		0x0001  /* Don't try to optimize join. */

/*
 * Flags private to DB->verify.
 *
 * DB->verify does not share common flags and so values start at 0x01.
 */
#define	DB_AGGRESSIVE	      0x0001  /* Salvage anything which might be data.*/
#define	DB_NOORDERCHK	      0x0002  /* Skip order check; subdb w/ user func */
#define	DB_ORDERCHKONLY	      0x0004  /* Only perform an order check on subdb */
#define	DB_PR_PAGE	      0x0008  /* Show page contents (-da). */
#define	DB_PR_HEADERS	      0x0010  /* Show only page headers (-dh). */
#define	DB_PR_RECOVERYTEST    0x0020  /* Recovery test (-dr). */
#define	DB_SALVAGE	      0x0040  /* Salvage what looks like data. */
/*
 * !!!
 * These must not go over 0x8000, or they will collide with the flags
 * used by __bam_vrfy_subtree.
 */
#define	DB_VRFY_FLAGMASK      0xffff  /* For masking above flags. */

/*
 * Deadlock detector modes; used in the DBENV structure to configure the
 * locking subsystem.
 */
#define	DB_LOCK_NORUN		0
#define	DB_LOCK_DEFAULT		1	/* Default policy. */
#define	DB_LOCK_OLDEST		2	/* Abort oldest transaction. */
#define	DB_LOCK_RANDOM		3	/* Abort random transaction. */
#define	DB_LOCK_YOUNGEST	4	/* Abort youngest transaction. */

/*******************************************************
 * Environment.
 *******************************************************/
#define	DB_REGION_MAGIC	0x120897	/* Environment magic number. */

typedef enum {
	DB_NOTICE_LOGFILE_CHANGED
	/* DB_NOTICE_DISK_LOW */
} db_notices;

typedef enum {
	DB_TXN_ABORT,
	DB_TXN_BACKWARD_ROLL,
	DB_TXN_FORWARD_ROLL,
	DB_TXN_OPENFILES
} db_recops;

#define	DB_UNDO(op)	((op) == DB_TXN_ABORT || (op) == DB_TXN_BACKWARD_ROLL)
#define	DB_REDO(op)	((op) == DB_TXN_FORWARD_ROLL)

struct __db_env {
	/*******************************************************
	 * Public: owned by the application.
	 *******************************************************/
	FILE		*db_errfile;	/* Error message file stream. */
	const char	*db_errpfx;	/* Error message prefix. */
					/* Callbacks. */
	void (*db_errcall) __P((const char *, char *));
	void (*db_feedback) __P((DB_ENV *, int, int));
	void (*db_noticecall) __P((DB_ENV *, db_notices));
	void (*db_paniccall) __P((DB_ENV *, int));
	int  (*db_recovery_init) __P((DB_ENV *));

	/*
	 * Currently, the verbose list is a bit field with room for 32
	 * entries.  There's no reason that it needs to be limited, if
	 * there are ever more than 32 entries, convert to a bit array.
	 */
#define	DB_VERB_CHKPOINT	0x0001	/* List checkpoints. */
#define	DB_VERB_DEADLOCK	0x0002	/* Deadlock detection information. */
#define	DB_VERB_RECOVERY	0x0004	/* Recovery information. */
#define	DB_VERB_WAITSFOR	0x0008	/* Dump waits-for table. */
	u_int32_t	 verbose;	/* Verbose output. */

	void		*app_private;	/* Application-private handle. */

	/* Locking. */
	u_int8_t	*lk_conflicts;	/* Two dimensional conflict matrix. */
	u_int32_t	 lk_modes;	/* Number of lock modes in table. */
	u_int32_t	 lk_max;	/* Maximum number of locks. */
	u_int32_t	 lk_max_lockers;/* Maximum number of lockers. */
	u_int32_t	 lk_max_objects;/* Maximum number of locked objects. */
	u_int32_t	 lk_detect;	/* Deadlock detect on all conflicts. */

	/* Logging. */
	u_int32_t	 lg_bsize;	/* Buffer size. */
	u_int32_t	 lg_max;	/* Maximum file size. */

	/* Memory pool. */
	u_int32_t	 mp_gbytes;	/* Cachesize: GB. */
	u_int32_t	 mp_bytes;	/* Cachesize: Bytes. */
	size_t		 mp_size;	/* DEPRECATED: Cachesize: bytes. */
	int		 mp_ncache;	/* Number of cache regions. */
	size_t		 mp_mmapsize;	/* Maximum file size for mmap. */

	/* Transactions. */
	u_int32_t	 tx_max;	/* Maximum number of transactions. */
	time_t		 tx_timestamp;	/* Recover to specific timestamp. */
	int (*tx_recover)		/* Dispatch function for recovery. */
	    __P((DB_ENV *, DBT *, DB_LSN *, db_recops));

	/*******************************************************
	 * Private: owned by DB.
	 *******************************************************/
	int		 db_panic;	/* Panic causing errno. */

	/* User files, paths. */
	char		*db_home;	/* Database home. */
	char		*db_log_dir;	/* Database log file directory. */
	char		*db_tmp_dir;	/* Database tmp file directory. */

	char	       **db_data_dir;	/* Database data file directories. */
	int		 data_cnt;	/* Database data file slots. */
	int		 data_next;	/* Next Database data file slot. */

	int		 db_mode;	/* Default open permissions. */

	void		*reginfo;	/* REGINFO structure reference. */
	DB_FH		*lockfhp;	/* fcntl(2) locking file handle. */
	long		 shm_key;	/* shmget(2) key. */

	void		*lg_handle;	/* Log handle. */

	void		*lk_handle;	/* Lock handle. */

	void		*mp_handle;	/* Mpool handle. */

	void		*tx_handle;	/* Txn handle. */

	int	      (**dtab)		/* Dispatch table */
			    __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
	size_t		 dtab_size;	/* Slots in the dispatch table. */

	void		*cl_handle;	/* RPC: remote client handle. */
	long		 cl_id;		/* RPC: Remote client env id. */

	int		 dblocal_ref;	/* DB_ENV_DBLOCAL: reference count. */
	u_int32_t	 db_mutexlocks; /* db_set_mutexlocks */

	/*
	 * List of open DB handles for this DB_ENV, used for cursor
	 * adjustment.  Must be protected for multi-threaded support.
	 *
	 * !!!
	 * As this structure is allocated in per-process memory, the
	 * mutex may need to be stored elsewhere on architectures unable
	 * to support mutexes in heap memory, e.g. HP/UX 9.
	 */
	void		*dblist_mutexp;	/* Mutex. */
	/*
	 * !!!
	 * Explicit representation of structure in queue.h.
	 * LIST_HEAD(dblist, __db);
	 */
	struct {
		struct __db *lh_first;
	} dblist;

	/*
	 * XA support.
	 *
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__db_env);
	 */
	struct {
		struct __db_env *tqe_next;
		struct __db_env **tqe_prev;
	} links;
	int		 xa_rmid;	/* XA Resource Manager ID. */
	DB_TXN		*xa_txn;	/* XA Current transaction. */

	void	*cj_internal;		/* C++/Java private. */

					/* Methods. */
	int  (*close) __P((DB_ENV *, u_int32_t));
	void (*err) __P((const DB_ENV *, int, const char *, ...));
	void (*errx) __P((const DB_ENV *, const char *, ...));
	int  (*open) __P((DB_ENV *, const char *, u_int32_t, int));
	int  (*remove) __P((DB_ENV *, const char *, u_int32_t));
	int  (*set_data_dir) __P((DB_ENV *, const char *));
	void (*set_errcall) __P((DB_ENV *, void (*)(const char *, char *)));
	void (*set_errfile) __P((DB_ENV *, FILE *));
	void (*set_errpfx) __P((DB_ENV *, const char *));
	int  (*set_feedback) __P((DB_ENV *, void (*)(DB_ENV *, int, int)));
	int  (*set_flags) __P((DB_ENV *, u_int32_t, int));
	int  (*set_mutexlocks) __P((DB_ENV *, int));
	void (*set_noticecall) __P((DB_ENV *, void (*)(DB_ENV *, db_notices)));
	int  (*set_paniccall) __P((DB_ENV *, void (*)(DB_ENV *, int)));
	int  (*set_recovery_init) __P((DB_ENV *, int (*)(DB_ENV *)));
	int  (*set_server) __P((DB_ENV *, char *, long, long, u_int32_t));
	int  (*set_shm_key) __P((DB_ENV *, long));
	int  (*set_tmp_dir) __P((DB_ENV *, const char *));
	int  (*set_verbose) __P((DB_ENV *, u_int32_t, int));

	int  (*set_lg_bsize) __P((DB_ENV *, u_int32_t));
	int  (*set_lg_dir) __P((DB_ENV *, const char *));
	int  (*set_lg_max) __P((DB_ENV *, u_int32_t));

	int  (*set_lk_conflicts) __P((DB_ENV *, u_int8_t *, int));
	int  (*set_lk_detect) __P((DB_ENV *, u_int32_t));
	int  (*set_lk_max) __P((DB_ENV *, u_int32_t));
	int  (*set_lk_max_locks) __P((DB_ENV *, u_int32_t));
	int  (*set_lk_max_lockers) __P((DB_ENV *, u_int32_t));
	int  (*set_lk_max_objects) __P((DB_ENV *, u_int32_t));

	int  (*set_mp_mmapsize) __P((DB_ENV *, size_t));
	int  (*set_cachesize) __P((DB_ENV *, u_int32_t, u_int32_t, int));

	int  (*set_tx_max) __P((DB_ENV *, u_int32_t));
	int  (*set_tx_recover) __P((DB_ENV *,
		int (*)(DB_ENV *, DBT *, DB_LSN *, db_recops)));
	int  (*set_tx_timestamp) __P((DB_ENV *, time_t *));

#ifdef CONFIG_TEST
#define	DB_TEST_PREOPEN		 1	/* before __os_open */
#define	DB_TEST_POSTOPEN	 2	/* after __os_open */
#define	DB_TEST_POSTLOGMETA	 3	/* after logging meta in btree */
#define	DB_TEST_POSTLOG		 4	/* after logging all pages */
#define	DB_TEST_POSTSYNC	 5	/* after syncing the log */
#define	DB_TEST_PRERENAME	 6	/* before __os_rename */
#define	DB_TEST_POSTRENAME	 7	/* after __os_rename */
	int		 test_abort;	/* Abort value for testing. */
	int		 test_copy;	/* Copy value for testing. */
#endif

#define	DB_ENV_CDB		0x00001	/* DB_INIT_CDB. */
#define	DB_ENV_CDB_ALLDB	0x00002	/* CDB environment wide locking. */
#define	DB_ENV_CREATE		0x00004	/* DB_CREATE set. */
#define	DB_ENV_DBLOCAL		0x00008	/* DB_ENV allocated for private DB. */
#define	DB_ENV_LOCKDOWN		0x00010	/* DB_LOCKDOWN set. */
#define	DB_ENV_NOMMAP		0x00020	/* DB_NOMMAP set. */
#define	DB_ENV_OPEN_CALLED	0x00040	/* DBENV->open called (paths valid). */
#define	DB_ENV_PRIVATE		0x00080	/* DB_PRIVATE set. */
#define	DB_ENV_RPCCLIENT	0x00100	/* DB_CLIENT set. */
#define	DB_ENV_STANDALONE	0x00200	/* Test: freestanding environment. */
#define	DB_ENV_SYSTEM_MEM	0x00400	/* DB_SYSTEM_MEM set. */
#define	DB_ENV_THREAD		0x00800	/* DB_THREAD set. */
#define	DB_ENV_TXN_NOSYNC	0x01000	/* DB_TXN_NOSYNC set. */
#define	DB_ENV_USER_ALLOC	0x02000	/* User allocated the structure. */
	u_int32_t	 flags;		/* Flags. */
};

/*******************************************************
 * Access methods.
 *******************************************************/
/*
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_BTREE=1,
	DB_HASH,
	DB_RECNO,
	DB_QUEUE,
	DB_UNKNOWN			/* Figure it out on open. */
} DBTYPE;

#define	DB_BTREEVERSION	8		/* Current btree version. */
#define	DB_BTREEOLDVER	6		/* Oldest btree version supported. */
#define	DB_BTREEMAGIC	0x053162

#define	DB_HASHVERSION	7		/* Current hash version. */
#define	DB_HASHOLDVER	4		/* Oldest hash version supported. */
#define	DB_HASHMAGIC	0x061561

#define	DB_QAMVERSION	3		/* Current queue version. */
#define	DB_QAMOLDVER	1		/* Oldest queue version supported. */
#define	DB_QAMMAGIC	0x042253

#define	DB_LOGVERSION	3		/* Current log version. */
#define	DB_LOGOLDVER	3		/* Oldest log version supported. */
#define	DB_LOGMAGIC	0x040988

/*
 * DB access method and cursor operation values.  Each value is an operation
 * code to which additional bit flags are added.
 */
#define	DB_AFTER	 1		/* c_put() */
#define	DB_APPEND	 2		/* put() */
#define	DB_BEFORE	 3		/* c_put() */
#define	DB_CACHED_COUNTS 4		/* stat() */
#define	DB_CHECKPOINT	 5		/* log_put(), log_get() */
#define	DB_CONSUME	 6		/* get() */
#define	DB_CONSUME_WAIT  7		/* get() */
#define	DB_CURLSN	 8		/* log_put() */
#define	DB_CURRENT	 9		/* c_get(), c_put(), log_get() */
#define	DB_FIRST	10		/* c_get(), log_get() */
#define	DB_FLUSH	11		/* log_put() */
#define	DB_GET_BOTH	12		/* get(), c_get() */
#define	DB_GET_BOTHC	13		/* c_get() (internal) */
#define	DB_GET_RECNO	14		/* c_get() */
#define	DB_JOIN_ITEM	15		/* c_get(); do not do primary lookup */
#define	DB_KEYFIRST	16		/* c_put() */
#define	DB_KEYLAST	17		/* c_put() */
#define	DB_LAST		18		/* c_get(), log_get() */
#define	DB_NEXT		19		/* c_get(), log_get() */
#define	DB_NEXT_DUP	20		/* c_get() */
#define	DB_NEXT_NODUP	21		/* c_get() */
#define	DB_NODUPDATA	22		/* put(), c_put() */
#define	DB_NOOVERWRITE	23		/* put() */
#define	DB_NOSYNC	24		/* close() */
#define	DB_POSITION	25		/* c_dup() */
#define	DB_POSITIONI	26		/* c_dup() (internal) */
#define	DB_PREV		27		/* c_get(), log_get() */
#define	DB_PREV_NODUP	28		/* c_get(), log_get() */
#define	DB_RECORDCOUNT	29		/* stat() */
#define	DB_SET		30		/* c_get(), log_get() */
#define	DB_SET_RANGE	31		/* c_get() */
#define	DB_SET_RECNO	32		/* get(), c_get() */
#define	DB_WRITECURSOR	33		/* cursor() */
#define	DB_WRITELOCK	34		/* cursor() (internal) */

/* This has to change when the max opcode hits 255. */
#define	DB_OPFLAGS_MASK	0x000000ff	/* Mask for operations flags. */
#define	DB_RMW		0x80000000	/* Acquire write flag immediately. */

/*
 * DB (user visible) error return codes.
 *
 * !!!
 * Changes to any of the user visible error return codes must be reflected
 * in java/src/com/sleepycat/db/Db.java.
 *
 * !!!
 * For source compatibility with DB 2.X deadlock return (EAGAIN), use the
 * following:
 *	#include <errno.h>
 *	#define DB_LOCK_DEADLOCK EAGAIN
 *
 * !!!
 * We don't want our error returns to conflict with other packages where
 * possible, so pick a base error value that's hopefully not common.  We
 * document that we own the error name space from -30,800 to -30,999.
 */
/* Public error return codes. */
#define	DB_INCOMPLETE		(-30999)/* Sync didn't finish. */
#define	DB_KEYEMPTY		(-30998)/* Key/data deleted or never created. */
#define	DB_KEYEXIST		(-30997)/* The key/data pair already exists. */
#define	DB_LOCK_DEADLOCK	(-30996)/* Deadlock. */
#define	DB_LOCK_NOTGRANTED	(-30995)/* Lock unavailable. */
#define	DB_NOSERVER		(-30994)/* Server panic return. */
#define	DB_NOSERVER_HOME	(-30993)/* Bad home sent to server. */
#define	DB_NOSERVER_ID		(-30992)/* Bad ID sent to server. */
#define	DB_NOTFOUND		(-30991)/* Key/data pair not found (EOF). */
#define	DB_OLD_VERSION		(-30990)/* Out-of-date version. */
#define	DB_RUNRECOVERY		(-30989)/* Panic return. */
#define	DB_VERIFY_BAD		(-30988)/* Verify failed; bad format. */

/* DB (private) error return codes. */
#define	DB_ALREADY_ABORTED	(-30899)
#define	DB_DELETED		(-30898)/* Recovery file marked deleted. */
#define	DB_JAVA_CALLBACK	(-30897)/* Exception during a java callback. */
#define	DB_NEEDSPLIT		(-30896)/* Page needs to be split. */
#define	DB_SWAPBYTES		(-30895)/* Database needs byte swapping. */
#define	DB_TXN_CKP		(-30894)/* Encountered ckp record in log. */
#define	DB_VERIFY_FATAL		(-30893)/* Fatal: DB->verify cannot proceed. */

#define	DB_FILE_ID_LEN		20	/* DB file ID length. */

/* DB access method description structure. */
struct __db {
	/*******************************************************
	 * Public: owned by the application.
	 *******************************************************/
	u_int32_t pgsize;		/* Database logical page size. */

					/* Callbacks. */
	int (*db_append_recno) __P((DB *, DBT *, db_recno_t));
	void (*db_feedback) __P((DB *, int, int));
	void *(*db_malloc) __P((size_t));
	void *(*db_realloc) __P((void *, size_t));
	int (*dup_compare) __P((DB *, const DBT *, const DBT *));

	void	*app_private;		/* Application-private handle. */

	/*******************************************************
	 * Private: owned by DB.
	 *******************************************************/
	DB_ENV  *dbenv;			/* Backing environment. */

	DBTYPE	 type;			/* DB access method type. */

	DB_MPOOLFILE *mpf;		/* Backing buffer pool. */

	void	*mutexp;		/* Synchronization for free threading */

	u_int8_t fileid[DB_FILE_ID_LEN];/* File's unique ID for locking. */

	u_int32_t adj_fileid;		/* File's unique ID for curs. adj. */

#define	DB_LOGFILEID_INVALID	-1
	int32_t	 log_fileid;		/* File's unique ID for logging. */
	db_pgno_t meta_pgno;		/* Meta page number */
	DB_TXN	*open_txn;		/* Transaction to protect creates. */

	long	 cl_id;			/* RPC: remote client id. */

	/*
	 * !!!
	 * Some applications use DB but implement their own locking outside of
	 * DB.  If they're using fcntl(2) locking on the underlying database
	 * file, and we open and close a file descriptor for that file, we will
	 * discard their locks.  The DB_FCNTL_LOCKING flag to DB->open is an
	 * undocumented interface to support this usage which leaves any file
	 * descriptors we open until DB->close.  This will only work with the
	 * DB->open interface and simple caches, e.g., creating a transaction
	 * thread may open/close file descriptors this flag doesn't protect.
	 * Locking with fcntl(2) on a file that you don't own is a very, very
	 * unsafe thing to do.  'Nuff said.
	 */
	DB_FH	*saved_open_fhp;	/* Saved file handle. */

	/*
	 * Linked list of DBP's, used in the log's dbentry table
	 * to keep track of all open db handles for a given log id.
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__db) links;
	 */
	struct {
		struct __db *tqe_next;
		struct __db **tqe_prev;
	} links;

	/*
	 * Linked list of DBP's, linked from the DB_ENV, used to
	 * keep track of all open db handles for cursor adjustment.
	 *
	 * XXX
	 * Eventually, this should be merged with "links" above.
	 *
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * LIST_ENTRY(__db) dblistlinks;
	 */
	struct {
		struct __db *le_next;
		struct __db **le_prev;
	} dblistlinks;

	/*
	 * Cursor queues.
	 *
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_HEAD(free_queue, __dbc);
	 * TAILQ_HEAD(active_queue, __dbc);
	 * TAILQ_HEAD(join_queue, __dbc);
	 */
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} free_queue;
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} active_queue;
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} join_queue;

	void	*bt_internal;		/* Btree/Recno access method private. */
	void	*cj_internal;		/* C++/Java private. */
	void	*h_internal;		/* Hash access method private. */
	void	*q_internal;		/* Queue access method private. */
	void	*xa_internal;		/* XA private. */

					/* Methods. */
	int  (*close) __P((DB *, u_int32_t));
	int  (*cursor) __P((DB *, DB_TXN *, DBC **, u_int32_t));
	int  (*del) __P((DB *, DB_TXN *, DBT *, u_int32_t));
	void (*err) __P((DB *, int, const char *, ...));
	void (*errx) __P((DB *, const char *, ...));
	int  (*fd) __P((DB *, int *));
	int  (*get) __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
	int  (*get_byteswapped) __P((DB *));
	DBTYPE
	     (*get_type) __P((DB *));
	int  (*join) __P((DB *, DBC **, DBC **, u_int32_t));
	int  (*key_range) __P((DB *,
		DB_TXN *, DBT *, DB_KEY_RANGE *, u_int32_t));
	int  (*open) __P((DB *,
		const char *, const char *, DBTYPE, u_int32_t, int));
	int  (*put) __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
	int  (*remove) __P((DB *, const char *, const char *, u_int32_t));
	int  (*rename) __P((DB *,
	    const char *, const char *, const char *, u_int32_t));
	int  (*set_append_recno) __P((DB *, int (*)(DB *, DBT *, db_recno_t)));
	int  (*set_cachesize) __P((DB *, u_int32_t, u_int32_t, int));
	int  (*set_dup_compare) __P((DB *,
	    int (*)(DB *, const DBT *, const DBT *)));
	void (*set_errcall) __P((DB *, void (*)(const char *, char *)));
	void (*set_errfile) __P((DB *, FILE *));
	void (*set_errpfx) __P((DB *, const char *));
	int  (*set_feedback) __P((DB *, void (*)(DB *, int, int)));
	int  (*set_flags) __P((DB *, u_int32_t));
	int  (*set_lorder) __P((DB *, int));
	int  (*set_malloc) __P((DB *, void *(*)(size_t)));
	int  (*set_pagesize) __P((DB *, u_int32_t));
	int  (*set_paniccall) __P((DB *, void (*)(DB_ENV *, int)));
	int  (*set_realloc) __P((DB *, void *(*)(void *, size_t)));
	int  (*stat) __P((DB *, void *, void *(*)(size_t), u_int32_t));
	int  (*sync) __P((DB *, u_int32_t));
	int  (*upgrade) __P((DB *, const char *, u_int32_t));
	int  (*verify) __P((DB *,
	    const char *, const char *, FILE *, u_int32_t));

	int  (*set_bt_compare) __P((DB *,
	    int (*)(DB *, const DBT *, const DBT *)));
	int  (*set_bt_maxkey) __P((DB *, u_int32_t));
	int  (*set_bt_minkey) __P((DB *, u_int32_t));
	int  (*set_bt_prefix) __P((DB *,
	    size_t (*)(DB *, const DBT *, const DBT *)));

	int  (*set_h_ffactor) __P((DB *, u_int32_t));
	int  (*set_h_hash) __P((DB *,
	    u_int32_t (*)(DB *, const void *, u_int32_t)));
	int  (*set_h_nelem) __P((DB *, u_int32_t));

	int  (*set_re_delim) __P((DB *, int));
	int  (*set_re_len) __P((DB *, u_int32_t));
	int  (*set_re_pad) __P((DB *, int));
	int  (*set_re_source) __P((DB *, const char *));
	int  (*set_q_extentsize) __P((DB *, u_int32_t));

	int  (*db_am_remove) __P((DB *, const char *,
	    const char *, DB_LSN *, int (**)(DB *, void*), void **));
	int  (*db_am_rename) __P((DB *,
	    const char *, const char *, const char *));

#define	DB_OK_BTREE	0x01
#define	DB_OK_HASH	0x02
#define	DB_OK_QUEUE	0x04
#define	DB_OK_RECNO	0x08
	u_int32_t	am_ok;		/* Legal AM choices. */

#define	DB_AM_DISCARD	0x00001		/* Discard any cached pages. */
#define	DB_AM_DUP	0x00002		/* DB_DUP. */
#define	DB_AM_DUPSORT	0x00004		/* DB_DUPSORT. */
#define	DB_AM_INMEM	0x00008		/* In-memory; no sync on close. */
#define	DB_AM_PGDEF	0x00010		/* Page size was defaulted. */
#define	DB_AM_RDONLY	0x00020		/* Database is readonly. */
#define	DB_AM_RECOVER	0x00040		/* DBP opened by recovery routine. */
#define	DB_AM_SUBDB	0x00080		/* Subdatabases supported. */
#define	DB_AM_SWAP	0x00100		/* Pages need to be byte-swapped. */
#define	DB_AM_TXN	0x00200		/* DBP was in a transaction. */
#define	DB_AM_VERIFYING	0x00400		/* DB handle is in the verifier. */
#define	DB_BT_RECNUM	0x00800		/* DB_RECNUM. */
#define	DB_BT_REVSPLIT	0x01000		/* DB_REVSPLITOFF. */
#define	DB_DBM_ERROR	0x02000		/* Error in DBM/NDBM database. */
#define	DB_OPEN_CALLED	0x04000		/* DB->open called. */
#define	DB_RE_DELIMITER	0x08000		/* Variablen length delimiter set. */
#define	DB_RE_FIXEDLEN	0x10000		/* Fixed-length records. */
#define	DB_RE_PAD	0x20000		/* Fixed-length record pad. */
#define	DB_RE_RENUMBER	0x40000		/* DB_RENUMBER. */
#define	DB_RE_SNAPSHOT	0x80000		/* DB_SNAPSHOT. */
	u_int32_t flags;
};

/*
 * DB_LOCK_ILOCK --
 *	Internal DB access method lock.
 */
struct __db_ilock {
	db_pgno_t pgno;			/* Page being locked. */
	u_int8_t fileid[DB_FILE_ID_LEN];/* File id. */
#define	DB_RECORD_LOCK	1
#define	DB_PAGE_LOCK	2
	u_int8_t type;			/* Record or Page lock */
};

/*
 * DB_LOCK --
 *	The structure is allocated by the caller and filled in during a
 *	lock_get request (or a lock_vec/DB_LOCK_GET).
 */
struct __db_lock_u {
	size_t		off;		/* Offset of the lock in the region */
	u_int32_t	ndx;		/* Index of the object referenced by
					 * this lock; used for locking. */
	u_int32_t	gen;		/* Generation number of this lock. */
};

/* Cursor description structure. */
struct __dbc {
	DB *dbp;			/* Related DB access method. */
	DB_TXN	 *txn;			/* Associated transaction. */

	/*
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__dbc) links;	Active/free cursor queues.
	 */
	struct {
		DBC *tqe_next;
		DBC **tqe_prev;
	} links;

	DBT	  rkey;			/* Returned key. */
	DBT	  rdata;		/* Returned data. */

	u_int32_t lid;			/* Default process' locker id. */
	u_int32_t locker;		/* Locker for this operation. */
	DBT	  lock_dbt;		/* DBT referencing lock. */
	DB_LOCK_ILOCK lock;		/* Object to be locked. */
	DB_LOCK	  mylock;		/* Lock held on this cursor. */

	long	  cl_id;		/* Remote client id. */

	DBTYPE	  dbtype;		/* Cursor type. */

	DBC_INTERNAL *internal;		/* Access method private. */

	int (*c_close) __P((DBC *));	/* Methods: public. */
	int (*c_count) __P((DBC *, db_recno_t *, u_int32_t));
	int (*c_del) __P((DBC *, u_int32_t));
	int (*c_dup) __P((DBC *, DBC **, u_int32_t));
	int (*c_get) __P((DBC *, DBT *, DBT *, u_int32_t));
	int (*c_put) __P((DBC *, DBT *, DBT *, u_int32_t));

					/* Methods: private. */
	int (*c_am_close) __P((DBC *, db_pgno_t, int *));
	int (*c_am_del) __P((DBC *));
	int (*c_am_destroy) __P((DBC *));
	int (*c_am_get) __P((DBC *, DBT *, DBT *, u_int32_t, db_pgno_t *));
	int (*c_am_put) __P((DBC *, DBT *, DBT *, u_int32_t, db_pgno_t *));
	int (*c_am_writelock) __P((DBC *));

#define	DBC_ACTIVE	0x001		/* Cursor is being used. */
#define	DBC_OPD		0x002		/* Cursor references off-page dups. */
#define	DBC_RECOVER	0x004		/* Cursor created by  recovery routine
					 * (do not log or lock).
					 */
#define	DBC_RMW		0x008		/* Acquire write flag in read op. */
#define	DBC_WRITECURSOR	0x010		/* Cursor may be used to write (CDB). */
#define	DBC_WRITEDUP	0x020		/* idup'ed DBC_WRITECURSOR (CDB). */
#define	DBC_WRITER	0x040		/* Cursor immediately writing (CDB). */
#define	DBC_TRANSIENT	0x080		/* Cursor is transient. */
#define	DBC_COMPENSATE	0x100		/* Cursor is doing compensation
					 * do not lock.
					 */
	u_int32_t flags;
};

/* Key range statistics structure */
struct __key_range {
	double less;
	double equal;
	double greater;
};

/* Btree/Recno statistics structure. */
struct __db_bt_stat {
	u_int32_t bt_magic;		/* Magic number. */
	u_int32_t bt_version;		/* Version number. */
	u_int32_t bt_metaflags;		/* Metadata flags. */
	u_int32_t bt_nkeys;		/* Number of unique keys. */
	u_int32_t bt_ndata;		/* Number of data items. */
	u_int32_t bt_pagesize;		/* Page size. */
	u_int32_t bt_maxkey;		/* Maxkey value. */
	u_int32_t bt_minkey;		/* Minkey value. */
	u_int32_t bt_re_len;		/* Fixed-length record length. */
	u_int32_t bt_re_pad;		/* Fixed-length record pad. */
	u_int32_t bt_levels;		/* Tree levels. */
	u_int32_t bt_int_pg;		/* Internal pages. */
	u_int32_t bt_leaf_pg;		/* Leaf pages. */
	u_int32_t bt_dup_pg;		/* Duplicate pages. */
	u_int32_t bt_over_pg;		/* Overflow pages. */
	u_int32_t bt_free;		/* Pages on the free list. */
	u_int32_t bt_int_pgfree;	/* Bytes free in internal pages. */
	u_int32_t bt_leaf_pgfree;	/* Bytes free in leaf pages. */
	u_int32_t bt_dup_pgfree;	/* Bytes free in duplicate pages. */
	u_int32_t bt_over_pgfree;	/* Bytes free in overflow pages. */
};

/* Queue statistics structure. */
struct __db_qam_stat {
	u_int32_t qs_magic;		/* Magic number. */
	u_int32_t qs_version;		/* Version number. */
	u_int32_t qs_metaflags;		/* Metadata flags. */
	u_int32_t qs_nkeys;		/* Number of unique keys. */
	u_int32_t qs_ndata;		/* Number of data items. */
	u_int32_t qs_pagesize;		/* Page size. */
	u_int32_t qs_pages;		/* Data pages. */
	u_int32_t qs_re_len;		/* Fixed-length record length. */
	u_int32_t qs_re_pad;		/* Fixed-length record pad. */
	u_int32_t qs_pgfree;		/* Bytes free in data pages. */
	u_int32_t qs_first_recno;	/* First not deleted record. */
	u_int32_t qs_cur_recno;		/* Last allocated record number. */
};

/* Hash statistics structure. */
struct __db_h_stat {
	u_int32_t hash_magic;		/* Magic number. */
	u_int32_t hash_version;		/* Version number. */
	u_int32_t hash_metaflags;	/* Metadata flags. */
	u_int32_t hash_nkeys;		/* Number of unique keys. */
	u_int32_t hash_ndata;		/* Number of data items. */
	u_int32_t hash_pagesize;	/* Page size. */
	u_int32_t hash_nelem;		/* Original nelem specified. */
	u_int32_t hash_ffactor;		/* Fill factor specified at create. */
	u_int32_t hash_buckets;		/* Number of hash buckets. */
	u_int32_t hash_free;		/* Pages on the free list. */
	u_int32_t hash_bfree;		/* Bytes free on bucket pages. */
	u_int32_t hash_bigpages;	/* Number of big key/data pages. */
	u_int32_t hash_big_bfree;	/* Bytes free on big item pages. */
	u_int32_t hash_overflows;	/* Number of overflow pages. */
	u_int32_t hash_ovfl_free;	/* Bytes free on ovfl pages. */
	u_int32_t hash_dup;		/* Number of dup pages. */
	u_int32_t hash_dup_free;	/* Bytes free on duplicate pages. */
};

int   db_create __P((DB **, DB_ENV *, u_int32_t));
int   db_env_create __P((DB_ENV **, u_int32_t));
int   db_env_set_func_close __P((int (*)(int)));
int   db_env_set_func_dirfree __P((void (*)(char **, int)));
int   db_env_set_func_dirlist __P((int (*)(const char *, char ***, int *)));
int   db_env_set_func_exists __P((int (*)(const char *, int *)));
int   db_env_set_func_free __P((void (*)(void *)));
int   db_env_set_func_fsync __P((int (*)(int)));
int   db_env_set_func_ioinfo __P((int (*)(const char *,
	  int, u_int32_t *, u_int32_t *, u_int32_t *)));
int   db_env_set_func_malloc __P((void *(*)(size_t)));
int   db_env_set_func_map __P((int (*)(char *, size_t, int, int, void **)));
int   db_env_set_func_open __P((int (*)(const char *, int, ...)));
int   db_env_set_func_read __P((ssize_t (*)(int, void *, size_t)));
int   db_env_set_func_realloc __P((void *(*)(void *, size_t)));
int   db_env_set_func_rename __P((int (*)(const char *, const char *)));
int   db_env_set_func_seek
	  __P((int (*)(int, size_t, db_pgno_t, u_int32_t, int, int)));
int   db_env_set_func_sleep __P((int (*)(u_long, u_long)));
int   db_env_set_func_unlink __P((int (*)(const char *)));
int   db_env_set_func_unmap __P((int (*)(void *, size_t)));
int   db_env_set_func_write __P((ssize_t (*)(int, const void *, size_t)));
int   db_env_set_func_yield __P((int (*)(void)));
int   db_env_set_pageyield __P((int));
int   db_env_set_panicstate __P((int));
int   db_env_set_region_init __P((int));
int   db_env_set_tas_spins __P((u_int32_t));
char *db_strerror __P((int));
char *db_version __P((int *, int *, int *));

/*******************************************************
 * Locking
 *******************************************************/
#define	DB_LOCKVERSION	1

/* Flag values for lock_vec(), lock_get(). */
#define	DB_LOCK_NOWAIT		0x01	/* Don't wait on unavailable lock. */
#define	DB_LOCK_RECORD		0x02	/* Internal: record lock. */
#define	DB_LOCK_UPGRADE		0x04	/* Internal: upgrade existing lock. */
#define	DB_LOCK_SWITCH		0x08	/* Internal: switch existing lock. */

/* Flag values for lock_detect(). */
#define	DB_LOCK_CONFLICT	0x01	/* Run on any conflict. */

/*
 * Request types.
 *
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_LOCK_DUMP=0,			/* Display held locks. */
	DB_LOCK_GET,			/* Get the lock. */
	DB_LOCK_INHERIT,		/* Pass locks to parent. */
	DB_LOCK_PUT,			/* Release the lock. */
	DB_LOCK_PUT_ALL,		/* Release locker's locks. */
	DB_LOCK_PUT_OBJ			/* Release locker's locks on obj. */
} db_lockop_t;

/*
 * Simple R/W lock modes and for multi-granularity intention locking.
 *
 * !!!
 * These values are NOT random, as they are used as an index into the lock
 * conflicts arrays, i.e., DB_LOCK_IWRITE must be == 3, and DB_LOCK_IREAD
 * must be == 4.
 *
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_LOCK_NG=0,			/* Not granted. */
	DB_LOCK_READ,			/* Shared/read. */
	DB_LOCK_WRITE,			/* Exclusive/write. */
	DB_LOCK_WAIT,			/* Wait for event */
	DB_LOCK_IWRITE,			/* Intent exclusive/write. */
	DB_LOCK_IREAD,			/* Intent to share/read. */
	DB_LOCK_IWR			/* Intent to read and write. */
} db_lockmode_t;

/*
 * Status of a lock.
 */
typedef enum {
	DB_LSTAT_ABORTED,		/* Lock belongs to an aborted txn. */
	DB_LSTAT_ERR,			/* Lock is bad. */
	DB_LSTAT_FREE,			/* Lock is unallocated. */
	DB_LSTAT_HELD,			/* Lock is currently held. */
	DB_LSTAT_NOGRANT,		/* Lock was not granted. */
	DB_LSTAT_PENDING,		/* Lock was waiting and has been
					 * promoted; waiting for the owner
					 * to run and upgrade it to held. */
	DB_LSTAT_WAITING		/* Lock is on the wait queue. */
} db_status_t;

/* Lock request structure. */
struct __db_lockreq {
	db_lockop_t	 op;		/* Operation. */
	db_lockmode_t	 mode;		/* Requested mode. */
	u_int32_t	 locker;	/* Locker identity. */
	DBT		*obj;		/* Object being locked. */
	DB_LOCK		 lock;		/* Lock returned. */
};

/*
 * Commonly used conflict matrices.
 *
 */

/* Multi-granularity locking. */
#define	DB_LOCK_RIW_N	7
extern const u_int8_t db_riw_conflicts[];

struct __db_lock_stat {
	u_int32_t st_lastid;		/* Last allocated locker ID. */
	u_int32_t st_maxlocks;		/* Maximum number of locks in table. */
	u_int32_t st_maxlockers;	/* Maximum number of lockers in table. */
	u_int32_t st_maxobjects;	/* Maximum number of objects in table. */
	u_int32_t st_nmodes;		/* Number of lock modes. */
	u_int32_t st_nlocks;		/* Current number of locks. */
	u_int32_t st_maxnlocks;		/* Maximum number of locks so far. */
	u_int32_t st_nlockers;		/* Current number of lockers. */
	u_int32_t st_maxnlockers;	/* Maximum number of lockers so far. */
	u_int32_t st_nobjects;		/* Current number of objects. */
	u_int32_t st_maxnobjects;	/* Maximum number of objects so far. */
	u_int32_t st_nconflicts;	/* Number of lock conflicts. */
	u_int32_t st_nrequests;		/* Number of lock gets. */
	u_int32_t st_nreleases;		/* Number of lock puts. */
	u_int32_t st_nnowaits;		/* Number of requests that would have
					   waited, but NOWAIT was set. */
	u_int32_t st_ndeadlocks;	/* Number of lock deadlocks. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_regsize;		/* Region size. */
};

int	  lock_detect __P((DB_ENV *, u_int32_t, u_int32_t, int *));
int	  lock_get __P((DB_ENV *,
	    u_int32_t, u_int32_t, const DBT *, db_lockmode_t, DB_LOCK *));
int	  lock_id __P((DB_ENV *, u_int32_t *));
int	  lock_put __P((DB_ENV *, DB_LOCK *));
int	  lock_stat __P((DB_ENV *, DB_LOCK_STAT **, void *(*)(size_t)));
int	  lock_vec __P((DB_ENV *,
	    u_int32_t, u_int32_t, DB_LOCKREQ *, int, DB_LOCKREQ **));

/*******************************************************
 * Logging.
 *******************************************************/
/* Flag values for log_archive(). */
#define	DB_ARCH_ABS		0x001	/* Absolute pathnames. */
#define	DB_ARCH_DATA		0x002	/* Data files. */
#define	DB_ARCH_LOG		0x004	/* Log files. */

/*
 * A DB_LSN has two parts, a fileid which identifies a specific file, and an
 * offset within that file.  The fileid is an unsigned 4-byte quantity that
 * uniquely identifies a file within the log directory -- currently a simple
 * counter inside the log.  The offset is also an unsigned 4-byte value.  The
 * log manager guarantees the offset is never more than 4 bytes by switching
 * to a new log file before the maximum length imposed by an unsigned 4-byte
 * offset is reached.
 */
struct __db_lsn {
	u_int32_t	file;		/* File ID. */
	u_int32_t	offset;		/* File offset. */
};

/* Log statistics structure. */
struct __db_log_stat {
	u_int32_t st_magic;		/* Log file magic number. */
	u_int32_t st_version;		/* Log file version number. */
	int st_mode;			/* Log file mode. */
	u_int32_t st_lg_bsize;		/* Log buffer size. */
	u_int32_t st_lg_max;		/* Maximum log file size. */
	u_int32_t st_w_bytes;		/* Bytes to log. */
	u_int32_t st_w_mbytes;		/* Megabytes to log. */
	u_int32_t st_wc_bytes;		/* Bytes to log since checkpoint. */
	u_int32_t st_wc_mbytes;		/* Megabytes to log since checkpoint. */
	u_int32_t st_wcount;		/* Total writes to the log. */
	u_int32_t st_wcount_fill;	/* Overflow writes to the log. */
	u_int32_t st_scount;		/* Total syncs to the log. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_cur_file;		/* Current log file number. */
	u_int32_t st_cur_offset;	/* Current log file offset. */
	u_int32_t st_regsize;		/* Region size. */
};

int	 log_archive __P((DB_ENV *, char **[], u_int32_t, void *(*)(size_t)));
int	 log_compare __P((const DB_LSN *, const DB_LSN *));
int	 log_file __P((DB_ENV *, const DB_LSN *, char *, size_t));
int	 log_flush __P((DB_ENV *, const DB_LSN *));
int	 log_get __P((DB_ENV *, DB_LSN *, DBT *, u_int32_t));
int	 log_put __P((DB_ENV *, DB_LSN *, const DBT *, u_int32_t));
int	 log_register __P((DB_ENV *, DB *, const char *));
int	 log_stat __P((DB_ENV *, DB_LOG_STAT **, void *(*)(size_t)));
int	 log_unregister __P((DB_ENV *, DB *));

/*******************************************************
 * Mpool
 *******************************************************/
/* Flag values for memp_fget(). */
#define	DB_MPOOL_CREATE		0x001	/* Create a page. */
#define	DB_MPOOL_LAST		0x002	/* Return the last page. */
#define	DB_MPOOL_NEW		0x004	/* Create a new page. */
#define	DB_MPOOL_NEW_GROUP	0x008	/* Create a group of pages. */
#define	DB_MPOOL_EXTENT		0x010	/* Get for an extent. */

/* Flag values for memp_fput(), memp_fset(). */
#define	DB_MPOOL_CLEAN		0x001	/* Page is not modified. */
#define	DB_MPOOL_DIRTY		0x002	/* Page is modified. */
#define	DB_MPOOL_DISCARD	0x004	/* Don't cache the page. */

/* Mpool statistics structure. */
struct __db_mpool_stat {
	u_int32_t st_cache_hit;		/* Pages found in the cache. */
	u_int32_t st_cache_miss;	/* Pages not found in the cache. */
	u_int32_t st_map;		/* Pages from mapped files. */
	u_int32_t st_page_create;	/* Pages created in the cache. */
	u_int32_t st_page_in;		/* Pages read in. */
	u_int32_t st_page_out;		/* Pages written out. */
	u_int32_t st_ro_evict;		/* Clean pages forced from the cache. */
	u_int32_t st_rw_evict;		/* Dirty pages forced from the cache. */
	u_int32_t st_hash_buckets;	/* Number of hash buckets. */
	u_int32_t st_hash_searches;	/* Total hash chain searches. */
	u_int32_t st_hash_longest;	/* Longest hash chain searched. */
	u_int32_t st_hash_examined;	/* Total hash entries searched. */
	u_int32_t st_page_clean;	/* Clean pages. */
	u_int32_t st_page_dirty;	/* Dirty pages. */
	u_int32_t st_page_trickle;	/* Pages written by memp_trickle. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_gbytes;		/* Total cache size: GB. */
	u_int32_t st_bytes;		/* Total cache size: B. */
	u_int32_t st_ncache;		/* Number of caches. */
	u_int32_t st_regsize;		/* Cache size. */
};

/* Mpool file open information structure. */
struct __db_mpool_finfo {
	int	   ftype;		/* File type. */
	DBT	  *pgcookie;		/* Byte-string passed to pgin/pgout. */
	u_int8_t  *fileid;		/* Unique file ID. */
	int32_t	   lsn_offset;		/* LSN offset in page. */
	u_int32_t  clear_len;		/* Cleared length on created pages. */
};

/* Mpool file statistics structure. */
struct __db_mpool_fstat {
	char *file_name;		/* File name. */
	size_t st_pagesize;		/* Page size. */
	u_int32_t st_cache_hit;		/* Pages found in the cache. */
	u_int32_t st_cache_miss;	/* Pages not found in the cache. */
	u_int32_t st_map;		/* Pages from mapped files. */
	u_int32_t st_page_create;	/* Pages created in the cache. */
	u_int32_t st_page_in;		/* Pages read in. */
	u_int32_t st_page_out;		/* Pages written out. */
};

int	memp_fclose __P((DB_MPOOLFILE *));
int	memp_fget __P((DB_MPOOLFILE *, db_pgno_t *, u_int32_t, void *));
int	memp_fopen __P((DB_ENV *, const char *,
	    u_int32_t, int, size_t, DB_MPOOL_FINFO *, DB_MPOOLFILE **));
int	memp_fput __P((DB_MPOOLFILE *, void *, u_int32_t));
int	memp_fset __P((DB_MPOOLFILE *, void *, u_int32_t));
int	memp_fsync __P((DB_MPOOLFILE *));
int	memp_register __P((DB_ENV *, int,
	    int (*)(DB_ENV *, db_pgno_t, void *, DBT *),
	    int (*)(DB_ENV *, db_pgno_t, void *, DBT *)));
int	memp_stat __P((DB_ENV *,
	    DB_MPOOL_STAT **, DB_MPOOL_FSTAT ***, void *(*)(size_t)));
int	memp_sync __P((DB_ENV *, DB_LSN *));
int	memp_trickle __P((DB_ENV *, int, int *));

/*******************************************************
 * Transactions.
 *******************************************************/
#define	DB_TXNVERSION	1

/* Operations values to the tx_recover() function. */
#define	DB_TXN_BACKWARD_ROLL	1	/* Read the log backwards. */
#define	DB_TXN_FORWARD_ROLL	2	/* Read the log forwards. */
#define	DB_TXN_OPENFILES	3	/* Read for open files. */
#define	DB_TXN_REDO		4	/* Redo the operation. */
#define	DB_TXN_UNDO		5	/* Undo the operation. */

/* Internal transaction status values. */

/* Transaction statistics structure. */
struct __db_txn_active {
	u_int32_t	txnid;		/* Transaction ID */
	u_int32_t	parentid;	/* Transaction ID of parent */
	DB_LSN		lsn;		/* Lsn of the begin record */
};

struct __db_txn_stat {
	DB_LSN	  st_last_ckp;		/* lsn of the last checkpoint */
	DB_LSN	  st_pending_ckp;	/* last checkpoint did not finish */
	time_t	  st_time_ckp;		/* time of last checkpoint */
	u_int32_t st_last_txnid;	/* last transaction id given out */
	u_int32_t st_maxtxns;		/* maximum txns possible */
	u_int32_t st_naborts;		/* number of aborted transactions */
	u_int32_t st_nbegins;		/* number of begun transactions */
	u_int32_t st_ncommits;		/* number of committed transactions */
	u_int32_t st_nactive;		/* number of active transactions */
	u_int32_t st_maxnactive;	/* maximum active transactions */
	DB_TXN_ACTIVE
		 *st_txnarray;		/* array of active transactions */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_regsize;		/* Region size. */
};

int	  txn_abort __P((DB_TXN *));
int	  txn_begin __P((DB_ENV *, DB_TXN *, DB_TXN **, u_int32_t));
int	  txn_checkpoint __P((DB_ENV *, u_int32_t, u_int32_t, u_int32_t));
int	  txn_commit __P((DB_TXN *, u_int32_t));
u_int32_t txn_id __P((DB_TXN *));
int	  txn_prepare __P((DB_TXN *));
int	  txn_stat __P((DB_ENV *, DB_TXN_STAT **, void *(*)(size_t)));

#ifndef DB_DBM_HSEARCH
#define	DB_DBM_HSEARCH	0		/* No historic interfaces by default. */
#endif
#if DB_DBM_HSEARCH != 0
/*******************************************************
 * Dbm/Ndbm historic interfaces.
 *******************************************************/
#define	DBM_INSERT	0		/* Flags to dbm_store(). */
#define	DBM_REPLACE	1

/*
 * The DB support for ndbm(3) always appends this suffix to the
 * file name to avoid overwriting the user's original database.
 */
#define	DBM_SUFFIX	".db"

#if defined(_XPG4_2)
typedef struct {
	char *dptr;
	size_t dsize;
} datum;
#else
typedef struct {
	char *dptr;
	int dsize;
} datum;
#endif

/*
 * Translate DBM calls into DB calls so that DB doesn't step on the
 * application's name space.
 *
 * The global variables dbrdonly, dirf and pagf were not retained when 4BSD
 * replaced the dbm interface with ndbm, and are not supported here.
 */
#define	dbminit(a)	__db_dbm_init(a)
#define	dbmclose	__db_dbm_close
#if !defined(__cplusplus)
#define	delete(a)	__db_dbm_delete(a)
#endif
#define	fetch(a)	__db_dbm_fetch(a)
#define	firstkey	__db_dbm_firstkey
#define	nextkey(a)	__db_dbm_nextkey(a)
#define	store(a, b)	__db_dbm_store(a, b)

/* Prototype the DB calls. */
int	 __db_dbm_close __P((void));
int	 __db_dbm_dbrdonly __P((void));
int	 __db_dbm_delete __P((datum));
int	 __db_dbm_dirf __P((void));
datum	 __db_dbm_fetch __P((datum));
datum	 __db_dbm_firstkey __P((void));
int	 __db_dbm_init __P((char *));
datum	 __db_dbm_nextkey __P((datum));
int	 __db_dbm_pagf __P((void));
int	 __db_dbm_store __P((datum, datum));

/*
 * Translate NDBM calls into DB calls so that DB doesn't step on the
 * application's name space.
 */
#define	dbm_clearerr(a)		__db_ndbm_clearerr(a)
#define	dbm_close(a)		__db_ndbm_close(a)
#define	dbm_delete(a, b)	__db_ndbm_delete(a, b)
#define	dbm_dirfno(a)		__db_ndbm_dirfno(a)
#define	dbm_error(a)		__db_ndbm_error(a)
#define	dbm_fetch(a, b)		__db_ndbm_fetch(a, b)
#define	dbm_firstkey(a)		__db_ndbm_firstkey(a)
#define	dbm_nextkey(a)		__db_ndbm_nextkey(a)
#define	dbm_open(a, b, c)	__db_ndbm_open(a, b, c)
#define	dbm_pagfno(a)		__db_ndbm_pagfno(a)
#define	dbm_rdonly(a)		__db_ndbm_rdonly(a)
#define	dbm_store(a, b, c, d)	__db_ndbm_store(a, b, c, d)

/* Prototype the DB calls. */
int	 __db_ndbm_clearerr __P((DBM *));
void	 __db_ndbm_close __P((DBM *));
int	 __db_ndbm_delete __P((DBM *, datum));
int	 __db_ndbm_dirfno __P((DBM *));
int	 __db_ndbm_error __P((DBM *));
datum	 __db_ndbm_fetch __P((DBM *, datum));
datum	 __db_ndbm_firstkey __P((DBM *));
datum	 __db_ndbm_nextkey __P((DBM *));
DBM	*__db_ndbm_open __P((const char *, int, int));
int	 __db_ndbm_pagfno __P((DBM *));
int	 __db_ndbm_rdonly __P((DBM *));
int	 __db_ndbm_store __P((DBM *, datum, datum, int));

/*******************************************************
 * Hsearch historic interface.
 *******************************************************/
typedef enum {
	FIND, ENTER
} ACTION;

typedef struct entry {
	char *key;
	char *data;
} ENTRY;

/*
 * Translate HSEARCH calls into DB calls so that DB doesn't step on the
 * application's name space.
 */
#define	hcreate(a)	__db_hcreate(a)
#define	hdestroy	__db_hdestroy
#define	hsearch(a, b)	__db_hsearch(a, b)

/* Prototype the DB calls. */
int	 __db_hcreate __P((size_t));
void	 __db_hdestroy __P((void));
ENTRY	*__db_hsearch __P((ENTRY, ACTION));
#endif /* DB_DBM_HSEARCH */

/*
 * XXX
 * MacOS: Reset Metrowerks C enum sizes.
 */
#ifdef __MWERKS__
#pragma enumsalwaysint reset
#endif

#if defined(__cplusplus)
}
#endif

#endif /* !_DB_H_ */

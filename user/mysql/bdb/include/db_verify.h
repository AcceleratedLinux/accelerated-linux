/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 * $Id: db_verify.h,v 1.18 2000/12/31 17:51:52 bostic Exp $
 */

#ifndef _DB_VERIFY_H_
#define	_DB_VERIFY_H_

/*
 * Structures and macros for the storage and retrieval of all information
 * needed for inter-page verification of a database.
 */

/*
 * EPRINT is the macro for error printing.  Takes as an arg the arg set
 * for DB->err.
 */
#define	EPRINT(x)							\
	do {								\
		if (!LF_ISSET(DB_SALVAGE))				\
			__db_err x;					\
	} while (0)

/* For fatal type errors--i.e., verifier bugs. */
#define	TYPE_ERR_PRINT(dbenv, func, pgno, ptype)			\
    EPRINT(((dbenv), "%s called on nonsensical page %lu of type %lu",	\
	(func), (u_long)(pgno), (u_long)(ptype)));

/* Is x a power of two?  (Tests true for zero, which doesn't matter here.) */
#define	POWER_OF_TWO(x)	(((x) & ((x) - 1)) == 0)

#define	IS_VALID_PAGESIZE(x)						\
	(POWER_OF_TWO(x) && (x) >= DB_MIN_PGSIZE && ((x) <= DB_MAX_PGSIZE))

/*
 * Note that 0 is, in general, a valid pgno, despite equalling PGNO_INVALID;
 * we have to test it separately where it's not appropriate.
 */
#define	IS_VALID_PGNO(x)	((x) <= vdp->last_pgno)

/*
 * Flags understood by the btree structure checks (esp. __bam_vrfy_subtree).
 * These share the same space as the global flags to __db_verify, and must not
 * dip below 0x00010000.
 */
#define	ST_DUPOK	0x00010000	/* Duplicates are acceptable. */
#define	ST_DUPSET	0x00020000	/* Subtree is in a duplicate tree. */
#define	ST_DUPSORT	0x00040000	/* Duplicates are sorted. */
#define	ST_IS_RECNO	0x00080000	/* Subtree is a recno. */
#define	ST_OVFL_LEAF	0x00100000	/* Overflow reffed from leaf page. */
#define	ST_RECNUM	0x00200000	/* Subtree has record numbering on. */
#define	ST_RELEN	0x00400000	/* Subtree has fixed-length records. */
#define	ST_TOPLEVEL	0x00800000	/* Subtree == entire tree */

/*
 * Flags understood by __bam_salvage and __db_salvage.  These need not share
 * the same space with the __bam_vrfy_subtree flags, but must share with
 * __db_verify.
 */
#define	SA_SKIPFIRSTKEY	0x00080000

/*
 * VRFY_DBINFO is the fundamental structure;  it either represents the database
 * of subdatabases, or the sole database if there are no subdatabases.
 */
struct __vrfy_dbinfo {
	/* Info about this database in particular. */
	DBTYPE		type;

	/* List of subdatabase meta pages, if any. */
	LIST_HEAD(__subdbs, __vrfy_childinfo) subdbs;

	/* File-global info--stores VRFY_PAGEINFOs for each page. */
	DB *pgdbp;

	/* Child database--stores VRFY_CHILDINFOs of each page. */
	DB *cdbp;

	/* Page info structures currently in use. */
	LIST_HEAD(__activepips, __vrfy_pageinfo) activepips;

	/*
	 * DB we use to keep track of which pages are linked somehow
	 * during verification.  0 is the default, "unseen";  1 is seen.
	 */
	DB *pgset;

	/*
	 * This is a database we use during salvaging to keep track of which
	 * overflow and dup pages we need to come back to at the end and print
	 * with key "UNKNOWN".  Pages which print with a good key get set
	 * to SALVAGE_IGNORE;  others get set, as appropriate, to SALVAGE_LDUP,
	 * SALVAGE_LRECNODUP, SALVAGE_OVERFLOW for normal db overflow pages,
	 * and SALVAGE_BTREE, SALVAGE_LRECNO, and SALVAGE_HASH for subdb
	 * pages.
	 */
#define	SALVAGE_INVALID		0
#define	SALVAGE_IGNORE		1
#define	SALVAGE_LDUP		2
#define	SALVAGE_LRECNODUP	3
#define	SALVAGE_OVERFLOW	4
#define	SALVAGE_LBTREE		5
#define	SALVAGE_HASH		6
#define	SALVAGE_LRECNO		7
	DB *salvage_pages;

	db_pgno_t	last_pgno;
	db_pgno_t	pgs_remaining;	/* For dbp->db_feedback(). */

	/* Queue needs these to verify data pages in the first pass. */
	u_int32_t	re_len;
	u_int32_t	rec_page;

#define	SALVAGE_PRINTHEADER	0x01
#define	SALVAGE_PRINTFOOTER	0x02
	u_int32_t	flags;
}; /* VRFY_DBINFO */

/*
 * The amount of state information we need per-page is small enough that
 * it's not worth the trouble to define separate structures for each
 * possible type of page, and since we're doing verification with these we
 * have to be open to the possibility that page N will be of a completely
 * unexpected type anyway.  So we define one structure here with all the
 * info we need for inter-page verification.
 */
struct __vrfy_pageinfo {
	u_int8_t	type;
	u_int8_t	bt_level;
	u_int8_t	unused1;
	u_int8_t	unused2;
	db_pgno_t	pgno;
	db_pgno_t	prev_pgno;
	db_pgno_t	next_pgno;

	/* meta pages */
	db_pgno_t	root;
	db_pgno_t	free;		/* Free list head. */

	db_indx_t	entries;	/* Actual number of entries. */
	u_int16_t	unused;
	db_recno_t	rec_cnt;	/* Record count. */
	u_int32_t	re_len;		/* Record length. */
	u_int32_t	bt_minkey;
	u_int32_t	bt_maxkey;
	u_int32_t	h_ffactor;
	u_int32_t	h_nelem;

	/* overflow pages */
	/*
	 * Note that refcount is the refcount for an overflow page; pi_refcount
	 * is this structure's own refcount!
	 */
	u_int32_t	refcount;
	u_int32_t	olen;

#define	VRFY_DUPS_UNSORTED	0x0001	/* Have to flag the negative! */
#define	VRFY_HAS_DUPS		0x0002
#define	VRFY_HAS_DUPSORT	0x0004	/* Has the flag set. */
#define	VRFY_HAS_SUBDBS		0x0008
#define	VRFY_HAS_RECNUMS	0x0010
#define	VRFY_INCOMPLETE		0x0020	/* Meta or item order checks incomp. */
#define	VRFY_IS_ALLZEROES	0x0040	/* Hash page we haven't touched? */
#define	VRFY_IS_FIXEDLEN	0x0080
#define	VRFY_IS_RECNO		0x0100
#define	VRFY_IS_RRECNO		0x0200
#define	VRFY_OVFL_LEAFSEEN	0x0400
	u_int32_t	flags;

	LIST_ENTRY(__vrfy_pageinfo) links;
	u_int32_t	pi_refcount;
}; /* VRFY_PAGEINFO */

struct __vrfy_childinfo {
	db_pgno_t	pgno;

#define	V_DUPLICATE	1		/* off-page dup metadata */
#define	V_OVERFLOW	2		/* overflow page */
#define	V_RECNO		3		/* btree internal or leaf page */
	u_int32_t	type;
	db_recno_t	nrecs;		/* record count on a btree subtree */
	u_int32_t	tlen;		/* ovfl. item total size */

	LIST_ENTRY(__vrfy_childinfo) links;
}; /* VRFY_CHILDINFO */

#endif /* _DB_VERIFY_H_ */

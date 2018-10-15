/******************************************************
Data dictionary system

(c) 1996 Innobase Oy

Created 1/8/1996 Heikki Tuuri
*******************************************************/

#ifndef dict0dict_h
#define dict0dict_h

#include "univ.i"
#include "dict0types.h"
#include "dict0mem.h"
#include "data0type.h"
#include "data0data.h"
#include "sync0sync.h"
#include "sync0rw.h"
#include "mem0mem.h"
#include "rem0types.h"
#include "btr0types.h"
#include "ut0mem.h"
#include "ut0lst.h"
#include "hash0hash.h"
#include "ut0rnd.h"
#include "ut0byte.h"
#include "trx0types.h"

/*************************************************************************
Accepts a specified string. Comparisons are case-insensitive. */

char*
dict_accept(
/*========*/
			/* out: if string was accepted, the pointer
			is moved after that, else ptr is returned */
	char*	ptr,	/* in: scan from this */
	const char* string,/* in: accept only this string as the next
			non-whitespace string */
	ibool*	success);/* out: TRUE if accepted */
/************************************************************************
Decrements the count of open MySQL handles to a table. */

void
dict_table_decrement_handle_count(
/*==============================*/
	dict_table_t*	table);	/* in: table */
/**************************************************************************
Inits the data dictionary module. */

void
dict_init(void);
/*===========*/
/**************************************************************************
Returns a stored procedure object and memoryfixes it. */
UNIV_INLINE
dict_proc_t*
dict_procedure_get(
/*===============*/
				/* out: procedure, NULL if does not exist */
	char*	proc_name,	/* in: table name */
	trx_t*	trx);		/* in: transaction handle or NULL */
/**************************************************************************
Adds a stored procedure object to the dictionary cache. */

void
dict_procedure_add_to_cache(
/*========================*/
	dict_proc_t*	proc);	/* in: procedure */
/**************************************************************************
Reserves a parsed copy of a stored procedure to execute. If there are no
free parsed copies left at the moment, parses a new copy. Takes the copy off
the list of copies: the copy must be returned there with
dict_procedure_release_parsed_copy. */

que_t*
dict_procedure_reserve_parsed_copy(
/*===============================*/
				/* out: the query graph */
	dict_proc_t*	proc);	/* in: dictionary procedure node */
/**************************************************************************
Releases a parsed copy of an executed stored procedure. Puts the copy to the
list of copies. */

void
dict_procedure_release_parsed_copy(
/*===============================*/
	que_t*	graph);	/* in: query graph of a stored procedure */
/*************************************************************************
Gets the column data type. */
UNIV_INLINE
dtype_t*
dict_col_get_type(
/*==============*/
	dict_col_t*	col);
/*************************************************************************
Gets the column number. */
UNIV_INLINE
ulint
dict_col_get_no(
/*============*/
	dict_col_t*	col);
/*************************************************************************
Gets the column position in the clustered index. */
UNIV_INLINE
ulint
dict_col_get_clust_pos(
/*===================*/
	dict_col_t*	col);
/************************************************************************
Initializes the autoinc counter. It is not an error to initialize an already
initialized counter. */

void
dict_table_autoinc_initialize(
/*==========================*/
	dict_table_t*	table,	/* in: table */
	ib_longlong	value);	/* in: next value to assign to a row */
/************************************************************************
Gets the next autoinc value (== autoinc counter value), 0 if not yet
initialized. If initialized, increments the counter by 1. */

ib_longlong
dict_table_autoinc_get(
/*===================*/
				/* out: value for a new row, or 0 */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Decrements the autoinc counter value by 1. */

void
dict_table_autoinc_decrement(
/*=========================*/
	dict_table_t*	table);	/* in: table */
/************************************************************************
Reads the next autoinc value (== autoinc counter value), 0 if not yet
initialized. */

ib_longlong
dict_table_autoinc_read(
/*====================*/
				/* out: value for a new row, or 0 */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Peeks the autoinc counter value, 0 if not yet initialized. Does not
increment the counter. The read not protected by any mutex! */

ib_longlong
dict_table_autoinc_peek(
/*====================*/
				/* out: value of the counter */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Updates the autoinc counter if the value supplied is equal or bigger than the
current value. If not inited, does nothing. */

void
dict_table_autoinc_update(
/*======================*/

	dict_table_t*	table,	/* in: table */
	ib_longlong	value);	/* in: value which was assigned to a row */
/**************************************************************************
Adds a table object to the dictionary cache. */

void
dict_table_add_to_cache(
/*====================*/
	dict_table_t*	table);	/* in: table */
/**************************************************************************
Removes a table object from the dictionary cache. */

void
dict_table_remove_from_cache(
/*=========================*/
	dict_table_t*	table);	/* in, own: table */
/**************************************************************************
Renames a table object. */

ibool
dict_table_rename_in_cache(
/*=======================*/
					/* out: TRUE if success */
	dict_table_t*	table,		/* in: table */
	char*		new_name,	/* in: new name */
	ibool		rename_also_foreigns);/* in: in ALTER TABLE we want
					to preserve the original table name
					in constraints which reference it */
/**************************************************************************
Adds a foreign key constraint object to the dictionary cache. May free
the object if there already is an object with the same identifier in.
At least one of foreign table or referenced table must already be in
the dictionary cache! */

ulint
dict_foreign_add_to_cache(
/*======================*/
					/* out: DB_SUCCESS or error code */
	dict_foreign_t*	foreign);	/* in, own: foreign key constraint */
/*************************************************************************
Scans a table create SQL string and adds to the data dictionary
the foreign key constraints declared in the string. This function
should be called after the indexes for a table have been created.
Each foreign key constraint must be accompanied with indexes in
bot participating tables. The indexes are allowed to contain more
fields than mentioned in the constraint. */

ulint
dict_create_foreign_constraints(
/*============================*/
				/* out: error code or DB_SUCCESS */
	trx_t*	trx,		/* in: transaction */
	char*	sql_string,	/* in: table create statement where
				foreign keys are declared like:
				FOREIGN KEY (a, b) REFERENCES table2(c, d),
				table2 can be written also with the database
				name before it: test.table2; the default
				database id the database of parameter name */
	char*	name);		/* in: table full name in the normalized form
				database_name/table_name */
/**************************************************************************
Parses the CONSTRAINT id's to be dropped in an ALTER TABLE statement. */

ulint
dict_foreign_parse_drop_constraints(
/*================================*/
						/* out: DB_SUCCESS or
						DB_CANNOT_DROP_CONSTRAINT if
						syntax error or the constraint
						id does not match */
	mem_heap_t*	heap,			/* in: heap from which we can
						allocate memory */
	trx_t*		trx,			/* in: transaction */
	dict_table_t*	table,			/* in: table */
	ulint*		n,			/* out: number of constraints
						to drop */
	char***		constraints_to_drop);	/* out: id's of the
						constraints to drop */
/**************************************************************************
Returns a table object and memoryfixes it. NOTE! This is a high-level
function to be used mainly from outside the 'dict' directory. Inside this
directory dict_table_get_low is usually the appropriate function. */

dict_table_t*
dict_table_get(
/*===========*/
				/* out: table, NULL if does not exist */
	char*	table_name,	/* in: table name */
	trx_t*	trx);		/* in: transaction handle */
/**************************************************************************
Returns a table object and increments MySQL open handle count on the table.
*/

dict_table_t*
dict_table_get_and_increment_handle_count(
/*======================================*/
				/* out: table, NULL if does not exist */
	char*	table_name,	/* in: table name */
	trx_t*	trx);		/* in: transaction handle or NULL */
/**************************************************************************
Returns a table object, based on table id, and memoryfixes it. */

dict_table_t*
dict_table_get_on_id(
/*=================*/
				/* out: table, NULL if does not exist */
	dulint	table_id,	/* in: table id */
	trx_t*	trx);		/* in: transaction handle */
/**************************************************************************
Returns a table object, based on table id, and memoryfixes it. */
UNIV_INLINE
dict_table_t*
dict_table_get_on_id_low(
/*=====================*/
				/* out: table, NULL if does not exist */
	dulint	table_id,	/* in: table id */
	trx_t*	trx);		/* in: transaction handle */
/**************************************************************************
Releases a table from being memoryfixed. Currently this has no relevance. */
UNIV_INLINE
void
dict_table_release(
/*===============*/
	dict_table_t*	table);	/* in: table to be released */
/**************************************************************************
Checks if a table is in the dictionary cache. */
UNIV_INLINE
dict_table_t*
dict_table_check_if_in_cache_low(
/*==============================*/
				/* out: table, NULL if not found */
	char*	table_name);	/* in: table name */
/**************************************************************************
Gets a table; loads it to the dictionary cache if necessary. A low-level
function. */
UNIV_INLINE
dict_table_t*
dict_table_get_low(
/*===============*/
				/* out: table, NULL if not found */
	char*	table_name);	/* in: table name */
/**************************************************************************
Returns an index object. */
UNIV_INLINE
dict_index_t*
dict_table_get_index(
/*=================*/
				/* out: index, NULL if does not exist */
	dict_table_t*	table,	/* in: table */
	char*		name);	/* in: index name */
/**************************************************************************
Returns an index object. */

dict_index_t*
dict_table_get_index_noninline(
/*===========================*/
				/* out: index, NULL if does not exist */
	dict_table_t*	table,	/* in: table */
	char*		name);	/* in: index name */
/**************************************************************************
Prints a table definition. */

void
dict_table_print(
/*=============*/
	dict_table_t*	table);	/* in: table */
/**************************************************************************
Prints a table data. */

void
dict_table_print_low(
/*=================*/
	dict_table_t*	table);	/* in: table */
/**************************************************************************
Prints a table data when we know the table name. */

void
dict_table_print_by_name(
/*=====================*/
	char*	name);
/**************************************************************************
Sprintfs to a string info on foreign keys of a table. */

void
dict_print_info_on_foreign_keys(
/*============================*/
	ibool		create_table_format, /* in: if TRUE then print in
				a format suitable to be inserted into
				a CREATE TABLE, otherwise in the format
				of SHOW TABLE STATUS */
	char*		str,	/* in/out: pointer to a string */
	ulint		len,	/* in: space in str available for info */
	dict_table_t*	table);	/* in: table */
/**************************************************************************
Sprintfs to a string info on a foreign key of a table in a format suitable
for CREATE TABLE. */

char*
dict_print_info_on_foreign_key_in_create_format(
/*============================================*/
                                /* out: how far in buf we printed */
	dict_foreign_t*	foreign,/* in: foreign key constraint */
	char*		buf);	/* in: buffer of at least 5000 bytes */
/************************************************************************
Gets the first index on the table (the clustered index). */
UNIV_INLINE
dict_index_t*
dict_table_get_first_index(
/*=======================*/
				/* out: index, NULL if none exists */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Gets the first index on the table (the clustered index). */

dict_index_t*
dict_table_get_first_index_noninline(
/*=================================*/
				/* out: index, NULL if none exists */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Gets the next index on the table. */
UNIV_INLINE
dict_index_t*
dict_table_get_next_index(
/*======================*/
				/* out: index, NULL if none left */
	dict_index_t*	index);	/* in: index */
/************************************************************************
Gets the next index on the table. */

dict_index_t*
dict_table_get_next_index_noninline(
/*================================*/
				/* out: index, NULL if none left */
	dict_index_t*	index);	/* in: index */
/************************************************************************
Gets the number of user-defined columns in a table in the dictionary
cache. */
UNIV_INLINE
ulint
dict_table_get_n_user_cols(
/*=======================*/
				/* out: number of user-defined (e.g., not
				ROW_ID) columns of a table */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Gets the number of system columns in a table in the dictionary cache. */
UNIV_INLINE
ulint
dict_table_get_n_sys_cols(
/*======================*/
				/* out: number of system (e.g.,
				ROW_ID) columns of a table */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Gets the number of all columns (also system) in a table in the dictionary
cache. */
UNIV_INLINE
ulint
dict_table_get_n_cols(
/*==================*/
				/* out: number of columns of a table */
	dict_table_t*	table);	/* in: table */
/************************************************************************
Gets the nth column of a table. */
UNIV_INLINE
dict_col_t*
dict_table_get_nth_col(
/*===================*/
				/* out: pointer to column object */
	dict_table_t*	table,	/* in: table */
	ulint		pos);	/* in: position of column */
/************************************************************************
Gets the nth column of a table. */

dict_col_t*
dict_table_get_nth_col_noninline(
/*=============================*/
				/* out: pointer to column object */
	dict_table_t*	table,	/* in: table */
	ulint		pos);	/* in: position of column */
/************************************************************************
Gets the given system column of a table. */
UNIV_INLINE
dict_col_t*
dict_table_get_sys_col(
/*===================*/
				/* out: pointer to column object */
	dict_table_t*	table,	/* in: table */
	ulint		sys);	/* in: DATA_ROW_ID, ... */
/************************************************************************
Gets the given system column number of a table. */
UNIV_INLINE
ulint
dict_table_get_sys_col_no(
/*======================*/
				/* out: column number */
	dict_table_t*	table,	/* in: table */
	ulint		sys);	/* in: DATA_ROW_ID, ... */
/***********************************************************************
Copies types of columns contained in table to tuple. */

void
dict_table_copy_types(
/*==================*/
	dtuple_t*	tuple,	/* in: data tuple */
	dict_table_t*	table);	/* in: index */
/**************************************************************************
Looks for an index with the given id. NOTE that we do not reserve
the dictionary mutex: this function is for emergency purposes like
printing info of a corrupt database page! */

dict_index_t*
dict_index_find_on_id_low(
/*======================*/
			/* out: index or NULL if not found from cache */
	dulint	id);	/* in: index id */
/**************************************************************************
Adds an index to dictionary cache. */

ibool
dict_index_add_to_cache(
/*====================*/
				/* out: TRUE if success */
	dict_table_t*	table,	/* in: table on which the index is */
	dict_index_t*	index);	/* in, own: index; NOTE! The index memory
				object is freed in this function! */
/************************************************************************
Gets the number of fields in the internal representation of an index,
including fields added by the dictionary system. */
UNIV_INLINE
ulint
dict_index_get_n_fields(
/*====================*/
				/* out: number of fields */
	dict_index_t*	index);	/* in: an internal representation of index
				(in the dictionary cache) */
/************************************************************************
Gets the number of fields in the internal representation of an index
that uniquely determine the position of an index entry in the index, if
we do not take multiversioning into account: in the B-tree use the value
returned by dict_index_get_n_unique_in_tree. */
UNIV_INLINE
ulint
dict_index_get_n_unique(
/*====================*/
				/* out: number of fields */
	dict_index_t*	index);	/* in: an internal representation of index
				(in the dictionary cache) */
/************************************************************************
Gets the number of fields in the internal representation of an index
which uniquely determine the position of an index entry in the index, if
we also take multiversioning into account. */
UNIV_INLINE
ulint
dict_index_get_n_unique_in_tree(
/*============================*/
				/* out: number of fields */
	dict_index_t*	index);	/* in: an internal representation of index
				(in the dictionary cache) */
/************************************************************************
Gets the number of user-defined ordering fields in the index. In the internal
representation we add the row id to the ordering fields to make all indexes
unique, but this function returns the number of fields the user defined
in the index as ordering fields. */
UNIV_INLINE
ulint
dict_index_get_n_ordering_defined_by_user(
/*======================================*/
				/* out: number of fields */
	dict_index_t*	index);	/* in: an internal representation of index
				(in the dictionary cache) */
/************************************************************************
Gets the nth field of an index. */
UNIV_INLINE
dict_field_t*
dict_index_get_nth_field(
/*=====================*/
				/* out: pointer to field object */
	dict_index_t*	index,	/* in: index */
	ulint		pos);	/* in: position of field */
/************************************************************************
Gets pointer to the nth field data type in an index. */
UNIV_INLINE
dtype_t*
dict_index_get_nth_type(
/*====================*/
				/* out: data type */
	dict_index_t*	index,	/* in: index */
	ulint		pos);	/* in: position of the field */
/************************************************************************
Gets the column number of the nth field in an index. */
UNIV_INLINE
ulint
dict_index_get_nth_col_no(
/*======================*/
				/* out: column number */
	dict_index_t*	index,	/* in: index */
	ulint		pos);	/* in: position of the field */
/************************************************************************
Looks for column n in an index. */

ulint
dict_index_get_nth_col_pos(
/*=======================*/
				/* out: position in internal representation
				of the index; if not contained, returns
				ULINT_UNDEFINED */
	dict_index_t*	index,	/* in: index */
	ulint		n);	/* in: column number */
/************************************************************************
Returns TRUE if the index contains a column or a prefix of that column. */

ibool
dict_index_contains_col_or_prefix(
/*==============================*/
				/* out: TRUE if contains the column or its
				prefix */
	dict_index_t*	index,	/* in: index */
	ulint		n);	/* in: column number */
/************************************************************************
Looks for a matching field in an index. The column and the prefix len has
to be the same. */

ulint
dict_index_get_nth_field_pos(
/*=========================*/
				/* out: position in internal representation
				of the index; if not contained, returns
				ULINT_UNDEFINED */
	dict_index_t*	index,	/* in: index from which to search */
	dict_index_t*	index2,	/* in: index */
	ulint		n);	/* in: field number in index2 */
/************************************************************************
Looks for column n position in the clustered index. */

ulint
dict_table_get_nth_col_pos(
/*=======================*/
				/* out: position in internal representation
				of the clustered index */
	dict_table_t*	table,	/* in: table */
	ulint		n);	/* in: column number */
/************************************************************************
Returns the position of a system column in an index. */
UNIV_INLINE
ulint
dict_index_get_sys_col_pos(
/*=======================*/
				/* out: position, ULINT_UNDEFINED if not
				contained */
	dict_index_t*	index,	/* in: index */
	ulint		type);	/* in: DATA_ROW_ID, ... */
/***********************************************************************
Copies types of fields contained in index to tuple. */

void
dict_index_copy_types(
/*==================*/
	dtuple_t*	tuple,		/* in: data tuple */
	dict_index_t*	index,		/* in: index */
	ulint		n_fields);	/* in: number of field types to copy */
/************************************************************************
Gets the value of a system column in a clustered index record. The clustered
index must contain the system column: if the index is unique, row id is
not contained there! */
UNIV_INLINE
dulint
dict_index_rec_get_sys_col(
/*=======================*/
				/* out: system column value */
	dict_index_t*	index,	/* in: clustered index describing the record */
	ulint		type,	/* in: column type: DATA_ROLL_PTR, ... */
	rec_t*		rec);	/* in: record */
/*************************************************************************
Gets the index tree where the index is stored. */
UNIV_INLINE
dict_tree_t*
dict_index_get_tree(
/*================*/
				/* out: index tree */
	dict_index_t*	index);	/* in: index */
/*************************************************************************
Gets the column data type. */
UNIV_INLINE
dtype_t*
dict_col_get_type(
/*==============*/
	dict_col_t*	col);
/*************************************************************************
Gets the field order criterion. */
UNIV_INLINE
ulint
dict_field_get_order(
/*=================*/
	dict_field_t*	field);
/*************************************************************************
Gets the field column. */
UNIV_INLINE
dict_col_t*
dict_field_get_col(
/*===============*/
	dict_field_t*	field);
/**************************************************************************
Creates an index tree struct. */

dict_tree_t*
dict_tree_create(
/*=============*/
				/* out, own: created tree */
	dict_index_t*	index);	/* in: the index for which to create: in the
				case of a mixed tree, this should be the
				index of the cluster object */
/**************************************************************************
Frees an index tree struct. */

void
dict_tree_free(
/*===========*/
	dict_tree_t*	tree);	/* in, own: index tree */
/**************************************************************************
In an index tree, finds the index corresponding to a record in the tree. */

dict_index_t*
dict_tree_find_index(
/*=================*/
				/* out: index */
	dict_tree_t*	tree,	/* in: index tree */
	rec_t*		rec);	/* in: record for which to find correct index */
/**************************************************************************
In an index tree, finds the index corresponding to a dtuple which is used
in a search to a tree. */

dict_index_t*
dict_tree_find_index_for_tuple(
/*===========================*/
				/* out: index; NULL if the tuple does not
				contain the mix id field in a mixed tree */
	dict_tree_t*	tree,	/* in: index tree */
	dtuple_t*	tuple);	/* in: tuple for which to find index */
/***********************************************************************
Checks if a table which is a mixed cluster member owns a record. */
UNIV_INLINE
ibool
dict_is_mixed_table_rec(
/*====================*/
				/* out: TRUE if the record belongs to this
				table */
	dict_table_t*	table,	/* in: table in a mixed cluster */
	rec_t*		rec);	/* in: user record in the clustered index */
/**************************************************************************
Returns an index object if it is found in the dictionary cache. */

dict_index_t*
dict_index_get_if_in_cache(
/*=======================*/
				/* out: index, NULL if not found */
	dulint	index_id);	/* in: index id */
/**************************************************************************
Checks that a tuple has n_fields_cmp value in a sensible range, so that
no comparison can occur with the page number field in a node pointer. */

ibool
dict_tree_check_search_tuple(
/*=========================*/
				/* out: TRUE if ok */
	dict_tree_t*	tree,	/* in: index tree */
	dtuple_t*	tuple);	/* in: tuple used in a search */
/**************************************************************************
Builds a node pointer out of a physical record and a page number. */

dtuple_t*
dict_tree_build_node_ptr(
/*=====================*/
				/* out, own: node pointer */
	dict_tree_t*	tree,	/* in: index tree */
	rec_t*		rec,	/* in: record for which to build node pointer */
	ulint		page_no,/* in: page number to put in node pointer */
	mem_heap_t*	heap,	/* in: memory heap where pointer created */
	ulint           level);  /* in: level of rec in tree: 0 means leaf
				level */
/**************************************************************************
Copies an initial segment of a physical record, long enough to specify an
index entry uniquely. */

rec_t*
dict_tree_copy_rec_order_prefix(
/*============================*/
				/* out: pointer to the prefix record */
	dict_tree_t*	tree,	/* in: index tree */
	rec_t*		rec,	/* in: record for which to copy prefix */
	byte**		buf,	/* in/out: memory buffer for the copied prefix,
				or NULL */
	ulint*		buf_size);/* in/out: buffer size */
/**************************************************************************
Builds a typed data tuple out of a physical record. */

dtuple_t*
dict_tree_build_data_tuple(
/*=======================*/
				/* out, own: data tuple */
	dict_tree_t*	tree,	/* in: index tree */
	rec_t*		rec,	/* in: record for which to build data tuple */
	mem_heap_t*	heap);	/* in: memory heap where tuple created */
/*************************************************************************
Gets the space id of the root of the index tree. */
UNIV_INLINE
ulint
dict_tree_get_space(
/*================*/
				/* out: space id */
	dict_tree_t*	tree);	/* in: tree */
/*************************************************************************
Sets the space id of the root of the index tree. */
UNIV_INLINE
void
dict_tree_set_space(
/*================*/
	dict_tree_t*	tree,	/* in: tree */
	ulint		space);	/* in: space id */
/*************************************************************************
Gets the page number of the root of the index tree. */
UNIV_INLINE
ulint
dict_tree_get_page(
/*===============*/
				/* out: page number */
	dict_tree_t*	tree);	/* in: tree */
/*************************************************************************
Sets the page number of the root of index tree. */
UNIV_INLINE
void
dict_tree_set_page(
/*===============*/
	dict_tree_t*	tree,	/* in: tree */
	ulint		page);	/* in: page number */
/*************************************************************************
Gets the type of the index tree. */
UNIV_INLINE
ulint
dict_tree_get_type(
/*===============*/
				/* out: type */
	dict_tree_t*	tree);	/* in: tree */
/*************************************************************************
Gets the read-write lock of the index tree. */
UNIV_INLINE
rw_lock_t*
dict_tree_get_lock(
/*===============*/
				/* out: read-write lock */
	dict_tree_t*	tree);	/* in: tree */
/************************************************************************
Returns free space reserved for future updates of records. This is
relevant only in the case of many consecutive inserts, as updates
which make the records bigger might fragment the index. */
UNIV_INLINE
ulint
dict_tree_get_space_reserve(
/*========================*/
				/* out: number of free bytes on page,
				reserved for updates */
	dict_tree_t*	tree);	/* in: a tree */
/*************************************************************************
Calculates the minimum record length in an index. */

ulint
dict_index_calc_min_rec_len(
/*========================*/
	dict_index_t*	index);	/* in: index */
/*************************************************************************
Calculates new estimates for table and index statistics. The statistics
are used in query optimization. */

void
dict_update_statistics_low(
/*=======================*/
	dict_table_t*	table,		/* in: table */
	ibool		has_dict_mutex);/* in: TRUE if the caller has the
					dictionary mutex */	
/*************************************************************************
Calculates new estimates for table and index statistics. The statistics
are used in query optimization. */

void
dict_update_statistics(
/*===================*/
	dict_table_t*	table);	/* in: table */
/************************************************************************
Reserves the dictionary system mutex for MySQL. */

void
dict_mutex_enter_for_mysql(void);
/*============================*/
/************************************************************************
Releases the dictionary system mutex for MySQL. */

void
dict_mutex_exit_for_mysql(void);
/*===========================*/

/* The following len must be at least 10000 bytes! */
#define DICT_FOREIGN_ERR_BUF_LEN	10000

/* Buffers for storing detailed information about the latest foreign key
and unique key errors */
extern char*	dict_foreign_err_buf;
extern char*	dict_unique_err_buf;
extern mutex_t	dict_foreign_err_mutex; /* mutex protecting the buffers */

extern dict_sys_t*	dict_sys;	/* the dictionary system */
extern rw_lock_t	dict_operation_lock;

/* Dictionary system struct */
struct dict_sys_struct{
	mutex_t		mutex;		/* mutex protecting the data
					dictionary; protects also the
					disk-based dictionary system tables;
					this mutex serializes CREATE TABLE
					and DROP TABLE, as well as reading
					the dictionary data for a table from
					system tables */
	dulint		row_id;		/* the next row id to assign;
					NOTE that at a checkpoint this
					must be written to the dict system
					header and flushed to a file; in
					recovery this must be derived from
					the log records */
	hash_table_t* 	table_hash;	/* hash table of the tables, based
					on name */
	hash_table_t* 	table_id_hash;	/* hash table of the tables, based
					on id */
	hash_table_t* 	col_hash;	/* hash table of the columns */
	hash_table_t*	procedure_hash;	/* hash table of the stored
					procedures */
	UT_LIST_BASE_NODE_T(dict_table_t)
			table_LRU; 	/* LRU list of tables */
	ulint		size;		/* varying space in bytes occupied
					by the data dictionary table and
					index objects */
	dict_table_t*	sys_tables;	/* SYS_TABLES table */
	dict_table_t*	sys_columns;	/* SYS_COLUMNS table */
	dict_table_t*	sys_indexes;	/* SYS_INDEXES table */
	dict_table_t*	sys_fields;	/* SYS_FIELDS table */
};					

#ifndef UNIV_NONINL
#include "dict0dict.ic"
#endif

#endif

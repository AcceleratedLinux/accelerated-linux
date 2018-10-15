/******************************************************
Interface between Innobase row operations and MySQL.
Contains also create table and other data dictionary operations.

(c) 2000 Innobase Oy

Created 9/17/2000 Heikki Tuuri
*******************************************************/

#include "row0mysql.h"

#ifdef UNIV_NONINL
#include "row0mysql.ic"
#endif

#include "row0ins.h"
#include "row0sel.h"
#include "row0upd.h"
#include "row0row.h"
#include "que0que.h"
#include "pars0pars.h"
#include "dict0dict.h"
#include "dict0crea.h"
#include "dict0load.h"
#include "trx0roll.h"
#include "trx0purge.h"
#include "lock0lock.h"
#include "rem0cmp.h"
#include "log0log.h"
#include "btr0sea.h"

/* A dummy variable used to fool the compiler */
ibool	row_mysql_identically_false	= FALSE;

/* List of tables we should drop in background. ALTER TABLE in MySQL requires
that the table handler can drop the table in background when there are no
queries to it any more. Protected by the kernel mutex. */
typedef struct row_mysql_drop_struct	row_mysql_drop_t;
struct row_mysql_drop_struct{
	char*				table_name;
	UT_LIST_NODE_T(row_mysql_drop_t) row_mysql_drop_list;
};

UT_LIST_BASE_NODE_T(row_mysql_drop_t)	row_mysql_drop_list;
ibool	row_mysql_drop_list_inited 	= FALSE;

/***********************************************************************
Reads a MySQL format variable-length field (like VARCHAR) length and
returns pointer to the field data. */

byte*
row_mysql_read_var_ref_noninline(
/*=============================*/
			/* out: field + 2 */
	ulint*	len,	/* out: variable-length field length */
	byte*	field)	/* in: field */
{
	return(row_mysql_read_var_ref(len, field));
}

/***********************************************************************
Stores a reference to a BLOB in the MySQL format. */

void
row_mysql_store_blob_ref(
/*=====================*/
	byte*	dest,		/* in: where to store */
	ulint	col_len,	/* in: dest buffer size: determines into
				how many bytes the BLOB length is stored,
				the space for the length may vary from 1
				to 4 bytes */
	byte*	data,		/* in: BLOB data; if the value to store
				is SQL NULL this should be NULL pointer */
	ulint	len)		/* in: BLOB length; if the value to store
				is SQL NULL this should be 0; remember
				also to set the NULL bit in the MySQL record
				header! */
{
	/* MySQL might assume the field is set to zero except the length and
	the pointer fields */

	memset(dest, '\0', col_len);

	/* In dest there are 1 - 4 bytes reserved for the BLOB length,
	and after that 8 bytes reserved for the pointer to the data.
	In 32-bit architectures we only use the first 4 bytes of the pointer
	slot. */

	ut_a(col_len - 8 > 1 || len < 256);
	ut_a(col_len - 8 > 2 || len < 256 * 256);
	ut_a(col_len - 8 > 3 || len < 256 * 256 * 256);

	mach_write_to_n_little_endian(dest, col_len - 8, len);

	ut_memcpy(dest + col_len - 8, (byte*)&data, sizeof(byte*));	
}

/***********************************************************************
Reads a reference to a BLOB in the MySQL format. */

byte*
row_mysql_read_blob_ref(
/*====================*/
				/* out: pointer to BLOB data */
	ulint*	len,		/* out: BLOB length */
	byte*	ref,		/* in: BLOB reference in the MySQL format */
	ulint	col_len)	/* in: BLOB reference length (not BLOB
				length) */
{
	byte*	data;

	*len = mach_read_from_n_little_endian(ref, col_len - 8);

	ut_memcpy((byte*)&data, ref + col_len - 8, sizeof(byte*));

	return(data);
}

/******************************************************************
Convert a row in the MySQL format to a row in the Innobase format. */
static
void
row_mysql_convert_row_to_innobase(
/*==============================*/
	dtuple_t*	row,		/* in/out: Innobase row where the
					field type information is already
					copied there, or will be copied
					later */
	row_prebuilt_t*	prebuilt,	/* in: prebuilt struct where template
					must be of type ROW_MYSQL_WHOLE_ROW */
	byte*		mysql_rec)	/* in: row in the MySQL format;
					NOTE: do not discard as long as
					row is used, as row may contain
					pointers to this record! */
{
	mysql_row_templ_t*	templ;	
	dfield_t*		dfield;
	ulint			i;
	
	ut_ad(prebuilt->template_type == ROW_MYSQL_WHOLE_ROW);
	ut_ad(prebuilt->mysql_template);

	for (i = 0; i < prebuilt->n_template; i++) {

		templ = prebuilt->mysql_template + i;
		dfield = dtuple_get_nth_field(row, i);

		if (templ->mysql_null_bit_mask != 0) {
			/* Column may be SQL NULL */

			if (mysql_rec[templ->mysql_null_byte_offset] &
 					(byte) (templ->mysql_null_bit_mask)) {

				/* It is SQL NULL */

				dfield_set_data(dfield, NULL, UNIV_SQL_NULL);

				goto next_column;
			}
		}			
		
		row_mysql_store_col_in_innobase_format(dfield,
					prebuilt->ins_upd_rec_buff
						+ templ->mysql_col_offset,
					mysql_rec + templ->mysql_col_offset,
					templ->mysql_col_len,
					templ->type, templ->is_unsigned);
next_column:
		;
	} 
}

/********************************************************************
Handles user errors and lock waits detected by the database engine. */

ibool
row_mysql_handle_errors(
/*====================*/
				/* out: TRUE if it was a lock wait and
				we should continue running the query thread */
	ulint*		new_err,/* out: possible new error encountered in
				lock wait, or if no new error, the value
				of trx->error_state at the entry of this
				function */
	trx_t*		trx,	/* in: transaction */
	que_thr_t*	thr,	/* in: query thread */
	trx_savept_t*	savept)	/* in: savepoint or NULL */
{
	ulint	err;

handle_new_error:
	err = trx->error_state;
	
	ut_a(err != DB_SUCCESS);
	
	trx->error_state = DB_SUCCESS;

	if (err == DB_DUPLICATE_KEY) {
           	if (savept) {
			/* Roll back the latest, possibly incomplete
			insertion or update */

			trx_general_rollback_for_mysql(trx, TRUE, savept);
		}
	} else if (err == DB_TOO_BIG_RECORD) {
           	if (savept) {
			/* Roll back the latest, possibly incomplete
			insertion or update */

			trx_general_rollback_for_mysql(trx, TRUE, savept);
		}
		/* MySQL will roll back the latest SQL statement */
	} else if (err == DB_ROW_IS_REFERENCED
		   || err == DB_NO_REFERENCED_ROW
		   || err == DB_CANNOT_ADD_CONSTRAINT) {
           	if (savept) {
			/* Roll back the latest, possibly incomplete
			insertion or update */

			trx_general_rollback_for_mysql(trx, TRUE, savept);
		}
		/* MySQL will roll back the latest SQL statement */
	} else if (err == DB_LOCK_WAIT) {

		srv_suspend_mysql_thread(thr);

		if (trx->error_state != DB_SUCCESS) {
			que_thr_stop_for_mysql(thr);

			goto handle_new_error;
		}

		*new_err = err;

		return(TRUE);

	} else if (err == DB_DEADLOCK || err == DB_LOCK_WAIT_TIMEOUT) {
		/* Roll back the whole transaction; this resolution was added
		to version 3.23.43 */

		trx_general_rollback_for_mysql(trx, FALSE, NULL);
				
	} else if (err == DB_OUT_OF_FILE_SPACE) {
           	if (savept) {
			/* Roll back the latest, possibly incomplete
			insertion or update */

			trx_general_rollback_for_mysql(trx, TRUE, savept);
		}
		/* MySQL will roll back the latest SQL statement */

	} else if (err == DB_MUST_GET_MORE_FILE_SPACE) {

		fprintf(stderr,
		"InnoDB: The database cannot continue operation because of\n"
		"InnoDB: lack of space. You must add a new data file to\n"
		"InnoDB: my.cnf and restart the database.\n");
		
		exit(1);
	} else if (err == DB_CORRUPTION) {

	       fprintf(stderr,
	    "InnoDB: We detected index corruption in an InnoDB type table.\n"
	    "InnoDB: You have to dump + drop + reimport the table or, in\n"
	    "InnoDB: a case of widespread corruption, dump all InnoDB\n"
	    "InnoDB: tables and recreate the whole InnoDB tablespace.\n"
	    "InnoDB: If the mysqld server crashes after the startup or when\n"
	    "InnoDB: you dump the tables, look at section 6.1 of\n"
	    "InnoDB: http://www.innodb.com/ibman.html for help.\n");

	} else {
		fprintf(stderr, "InnoDB: unknown error code %lu\n", err);
		ut_a(0);
	}		

	if (trx->error_state != DB_SUCCESS) {
		*new_err = trx->error_state;
	} else {
		*new_err = err;
	}
	
	trx->error_state = DB_SUCCESS;

	return(FALSE);
}

/************************************************************************
Create a prebuilt struct for a MySQL table handle. */

row_prebuilt_t*
row_create_prebuilt(
/*================*/
				/* out, own: a prebuilt struct */
	dict_table_t*	table)	/* in: Innobase table handle */
{
	row_prebuilt_t*	prebuilt;
	mem_heap_t*	heap;
	dict_index_t*	clust_index;
	dtuple_t*	ref;
	ulint		ref_len;
	ulint		i;
	
	heap = mem_heap_create(128);

	prebuilt = mem_heap_alloc(heap, sizeof(row_prebuilt_t));

	prebuilt->magic_n = ROW_PREBUILT_ALLOCATED;
	prebuilt->magic_n2 = ROW_PREBUILT_ALLOCATED;

	prebuilt->table = table;

	prebuilt->trx = NULL;

	prebuilt->sql_stat_start = TRUE;

	prebuilt->mysql_has_locked = FALSE;

	prebuilt->index = NULL;

	prebuilt->used_in_HANDLER = FALSE;

	prebuilt->n_template = 0;
	prebuilt->mysql_template = NULL;

	prebuilt->heap = heap;
	prebuilt->ins_node = NULL;

	prebuilt->ins_upd_rec_buff = NULL;
	
	prebuilt->upd_node = NULL;
	prebuilt->ins_graph = NULL;
	prebuilt->upd_graph = NULL;

  	prebuilt->pcur = btr_pcur_create_for_mysql();
  	prebuilt->clust_pcur = btr_pcur_create_for_mysql();

	prebuilt->select_lock_type = LOCK_NONE;

	prebuilt->sel_graph = NULL;

	prebuilt->search_tuple = dtuple_create(heap,
						dict_table_get_n_cols(table));
	
	clust_index = dict_table_get_first_index(table);

	ref_len = dict_index_get_n_unique(clust_index);

	ref = dtuple_create(heap, ref_len);

	dict_index_copy_types(ref, clust_index, ref_len);

	prebuilt->clust_ref = ref;

	for (i = 0; i < MYSQL_FETCH_CACHE_SIZE; i++) {
		prebuilt->fetch_cache[i] = NULL;
	}

	prebuilt->n_fetch_cached = 0;

	prebuilt->blob_heap = NULL;

	prebuilt->old_vers_heap = NULL;

	return(prebuilt);
}

/************************************************************************
Free a prebuilt struct for a MySQL table handle. */

void
row_prebuilt_free(
/*==============*/
	row_prebuilt_t*	prebuilt)	/* in, own: prebuilt struct */
{
	ulint	i;

	if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED
	    || prebuilt->magic_n2 != ROW_PREBUILT_ALLOCATED) {
		fprintf(stderr,
"InnoDB: Error: trying to free a corrupt\n"
"InnoDB: table handle. Magic n %lu, magic n2 %lu, table name %s\n",
		prebuilt->magic_n, prebuilt->magic_n2, prebuilt->table->name);

		mem_analyze_corruption((byte*)prebuilt);

		ut_a(0);
	}

	prebuilt->magic_n = ROW_PREBUILT_FREED;
	prebuilt->magic_n2 = ROW_PREBUILT_FREED;

	btr_pcur_free_for_mysql(prebuilt->pcur);
	btr_pcur_free_for_mysql(prebuilt->clust_pcur);

	if (prebuilt->mysql_template) {
		mem_free(prebuilt->mysql_template);
	}

	if (prebuilt->ins_graph) {
		que_graph_free_recursive(prebuilt->ins_graph);
	}

	if (prebuilt->sel_graph) {
		que_graph_free_recursive(prebuilt->sel_graph);
	}
	
	if (prebuilt->upd_graph) {
		que_graph_free_recursive(prebuilt->upd_graph);
	}
	
	if (prebuilt->blob_heap) {
		mem_heap_free(prebuilt->blob_heap);
	}

	if (prebuilt->old_vers_heap) {
		mem_heap_free(prebuilt->old_vers_heap);
	}
	
	for (i = 0; i < MYSQL_FETCH_CACHE_SIZE; i++) {
		if (prebuilt->fetch_cache[i] != NULL) {

			if ((ROW_PREBUILT_FETCH_MAGIC_N !=
			    mach_read_from_4((prebuilt->fetch_cache[i]) - 4))
			    || (ROW_PREBUILT_FETCH_MAGIC_N !=
			    mach_read_from_4((prebuilt->fetch_cache[i])
			    			+ prebuilt->mysql_row_len))) {
				fprintf(stderr,
			"InnoDB: Error: trying to free a corrupt\n"
			"InnoDB: fetch buffer.\n");

				mem_analyze_corruption(
						prebuilt->fetch_cache[i]);

				ut_a(0);
			}

			mem_free((prebuilt->fetch_cache[i]) - 4);
		}
	}

	dict_table_decrement_handle_count(prebuilt->table);

	mem_heap_free(prebuilt->heap);
}

/*************************************************************************
Updates the transaction pointers in query graphs stored in the prebuilt
struct. */

void
row_update_prebuilt_trx(
/*====================*/
					/* out: prebuilt dtuple */
	row_prebuilt_t*	prebuilt,	/* in: prebuilt struct in MySQL
					handle */
	trx_t*		trx)		/* in: transaction handle */
{	
	if (trx->magic_n != TRX_MAGIC_N) {
		fprintf(stderr,
		"InnoDB: Error: trying to use a corrupt\n"
		"InnoDB: trx handle. Magic n %lu\n",
		trx->magic_n);

		mem_analyze_corruption((byte*)trx);

		ut_a(0);
	}

	if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED) {
		fprintf(stderr,
		"InnoDB: Error: trying to use a corrupt\n"
		"InnoDB: table handle. Magic n %lu, table name %s\n",
		prebuilt->magic_n, prebuilt->table->name);

		mem_analyze_corruption((byte*)prebuilt);

		ut_a(0);
	}

	prebuilt->trx = trx;

	if (prebuilt->ins_graph) {
		prebuilt->ins_graph->trx = trx;
	}

	if (prebuilt->upd_graph) {
		prebuilt->upd_graph->trx = trx;
	}

	if (prebuilt->sel_graph) {
		prebuilt->sel_graph->trx = trx;
	}	
}

/*************************************************************************
Gets pointer to a prebuilt dtuple used in insertions. If the insert graph
has not yet been built in the prebuilt struct, then this function first
builds it. */
static
dtuple_t*
row_get_prebuilt_insert_row(
/*========================*/
					/* out: prebuilt dtuple */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	ins_node_t*	node;
	dtuple_t*	row;
	dict_table_t*	table	= prebuilt->table;
	ulint		i;

	ut_ad(prebuilt && table && prebuilt->trx);
	
	if (prebuilt->ins_node == NULL) {

		/* Not called before for this handle: create an insert node
		and query graph to the prebuilt struct */

		node = ins_node_create(INS_DIRECT, table, prebuilt->heap);
		
		prebuilt->ins_node = node;

		if (prebuilt->ins_upd_rec_buff == NULL) {
			prebuilt->ins_upd_rec_buff = mem_heap_alloc(
						prebuilt->heap,
						prebuilt->mysql_row_len);
		}
		
		row = dtuple_create(prebuilt->heap,
					dict_table_get_n_cols(table));

		dict_table_copy_types(row, table);

		/* We init the value of every field to the SQL NULL to avoid
		a debug assertion from failing */

		for (i = 0; i < dtuple_get_n_fields(row); i++) {
		    
		        dtuple_get_nth_field(row, i)->len = UNIV_SQL_NULL;
		}

		ins_node_set_new_row(node, row);

		prebuilt->ins_graph =
			que_node_get_parent(
				pars_complete_graph_for_exec(node,
							prebuilt->trx,
							prebuilt->heap));
		prebuilt->ins_graph->state = QUE_FORK_ACTIVE;
	}

	return(prebuilt->ins_node->row);	
}

/*************************************************************************
Updates the table modification counter and calculates new estimates
for table and index statistics if necessary. */
UNIV_INLINE
void
row_update_statistics_if_needed(
/*============================*/
	dict_table_t*	table)	/* in: table */
{
	ulint	counter;
	
	counter = table->stat_modified_counter;

	table->stat_modified_counter = counter + 1;

	/* Calculate new statistics if 1 / 16 of table has been modified
	since the last time a statistics batch was run, or if
	stat_modified_counter > 2 000 000 000 (to avoid wrap-around).
	We calculate statistics at most every 16th round, since we may have
	a counter table which is very small and updated very often. */

	if (counter > 2000000000
	    || ((ib_longlong)counter > 16 + table->stat_n_rows / 16)) {

		dict_update_statistics(table);
	}	
}
		  	
/*************************************************************************
Unlocks an AUTO_INC type lock possibly reserved by trx. */

void		  	
row_unlock_table_autoinc_for_mysql(
/*===============================*/
	trx_t*	trx)	/* in: transaction */
{
	if (!trx->auto_inc_lock) {

		return;
	}

	lock_table_unlock_auto_inc(trx);
}

/*************************************************************************
Sets an AUTO_INC type lock on the table mentioned in prebuilt. The
AUTO_INC lock gives exclusive access to the auto-inc counter of the
table. The lock is reserved only for the duration of an SQL statement.
It is not compatible with another AUTO_INC or exclusive lock on the
table. */

int
row_lock_table_autoinc_for_mysql(
/*=============================*/
					/* out: error code or DB_SUCCESS */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in the MySQL
					table handle */
{
	trx_t*		trx 		= prebuilt->trx;
	ins_node_t*	node		= prebuilt->ins_node;
	que_thr_t*	thr;
	ulint		err;
	ibool		was_lock_wait;
	
	ut_ad(trx);
	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	
	if (trx->auto_inc_lock) {

		return(DB_SUCCESS);
	}

	trx->op_info = (char *) "setting auto-inc lock";

	if (node == NULL) {
		row_get_prebuilt_insert_row(prebuilt);
		node = prebuilt->ins_node;
	}

	/* We use the insert query graph as the dummy graph needed
	in the lock module call */

	thr = que_fork_get_first_thr(prebuilt->ins_graph);

	que_thr_move_to_run_state_for_mysql(thr, trx);

run_again:
	thr->run_node = node;
	thr->prev_node = node;

	/* It may be that the current session has not yet started
	its transaction, or it has been committed: */
		
	trx_start_if_not_started(trx);

	err = lock_table(0, prebuilt->table, LOCK_AUTO_INC, thr);

	trx->error_state = err;

	if (err != DB_SUCCESS) {
		que_thr_stop_for_mysql(thr);

		was_lock_wait = row_mysql_handle_errors(&err, trx, thr, NULL);

		if (was_lock_wait) {
			goto run_again;
		}

		trx->op_info = (char *) "";

		return(err);
	}

	que_thr_stop_for_mysql_no_error(thr, trx);
		
	trx->op_info = (char *) "";

	return((int) err);	
}
					
/*************************************************************************
Does an insert for MySQL. */

int
row_insert_for_mysql(
/*=================*/
					/* out: error code or DB_SUCCESS */
	byte*		mysql_rec,	/* in: row in the MySQL format */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	trx_savept_t	savept;
	que_thr_t*	thr;
	ulint		err;
	ibool		was_lock_wait;
	trx_t*		trx 		= prebuilt->trx;
	ins_node_t*	node		= prebuilt->ins_node;
	
	ut_ad(trx);
	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	
	if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED) {
		fprintf(stderr,
		"InnoDB: Error: trying to free a corrupt\n"
		"InnoDB: table handle. Magic n %lu, table name %s\n",
		prebuilt->magic_n, prebuilt->table->name);

		mem_analyze_corruption((byte*)prebuilt);

		ut_a(0);
	}

	if (srv_created_new_raw || srv_force_recovery) {
		fprintf(stderr,
		"InnoDB: A new raw disk partition was initialized or\n"
		"InnoDB: innodb_force_recovery is on: we do not allow\n"
		"InnoDB: database modifications by the user. Shut down\n"
		"InnoDB: mysqld and edit my.cnf so that newraw is replaced\n"
		"InnoDB: with raw, and innodb_force_... is removed.\n");

		return(DB_ERROR);
	}

	trx->op_info = (char *) "inserting";

	trx_start_if_not_started(trx);

	if (node == NULL) {
		row_get_prebuilt_insert_row(prebuilt);
		node = prebuilt->ins_node;
	}

	row_mysql_convert_row_to_innobase(node->row, prebuilt, mysql_rec);
	
	savept = trx_savept_take(trx);
	
	thr = que_fork_get_first_thr(prebuilt->ins_graph);

	if (prebuilt->sql_stat_start) {
		node->state = INS_NODE_SET_IX_LOCK;
		prebuilt->sql_stat_start = FALSE;
	} else {
		node->state = INS_NODE_ALLOC_ROW_ID;
	}
	
	que_thr_move_to_run_state_for_mysql(thr, trx);

run_again:
	thr->run_node = node;
	thr->prev_node = node;

	row_ins_step(thr);
	
	err = trx->error_state;

	if (err != DB_SUCCESS) {
		que_thr_stop_for_mysql(thr);

		was_lock_wait = row_mysql_handle_errors(&err, trx, thr,
								&savept);
		if (was_lock_wait) {
			goto run_again;
		}

		trx->op_info = (char *) "";

		return(err);
	}

	que_thr_stop_for_mysql_no_error(thr, trx);
	
	prebuilt->table->stat_n_rows++;

	srv_n_rows_inserted++;
	
	if (prebuilt->table->stat_n_rows == 0) {
		/* Avoid wrap-over */
		prebuilt->table->stat_n_rows--;
	}	

	row_update_statistics_if_needed(prebuilt->table);
	trx->op_info = (char *) "";

	return((int) err);
}

/*************************************************************************
Builds a dummy query graph used in selects. */

void
row_prebuild_sel_graph(
/*===================*/
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	sel_node_t*	node;

	ut_ad(prebuilt && prebuilt->trx);
	
	if (prebuilt->sel_graph == NULL) {

		node = sel_node_create(prebuilt->heap);
				
		prebuilt->sel_graph =
			que_node_get_parent(
				pars_complete_graph_for_exec(node,
							prebuilt->trx,
							prebuilt->heap));

		prebuilt->sel_graph->state = QUE_FORK_ACTIVE;
	}
}

/*************************************************************************
Creates an query graph node of 'update' type to be used in the MySQL
interface. */

upd_node_t*
row_create_update_node_for_mysql(
/*=============================*/
				/* out, own: update node */
	dict_table_t*	table,	/* in: table to update */
	mem_heap_t*	heap)	/* in: mem heap from which allocated */
{
	upd_node_t*	node;

	node = upd_node_create(heap);
		
	node->in_mysql_interface = TRUE;
	node->is_delete = FALSE;
	node->searched_update = FALSE;
	node->select_will_do_update = FALSE;
	node->select = NULL;
	node->pcur = btr_pcur_create_for_mysql();
	node->table = table;

	node->update = upd_create(dict_table_get_n_cols(table), heap);

	node->update_n_fields = dict_table_get_n_cols(table);
	
	UT_LIST_INIT(node->columns);
	node->has_clust_rec_x_lock = TRUE;
	node->cmpl_info = 0;

	node->table_sym = NULL;
	node->col_assign_list = NULL;

	return(node);
}

/*************************************************************************
Gets pointer to a prebuilt update vector used in updates. If the update
graph has not yet been built in the prebuilt struct, then this function
first builds it. */

upd_t*
row_get_prebuilt_update_vector(
/*===========================*/
					/* out: prebuilt update vector */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	dict_table_t*	table	= prebuilt->table;
	upd_node_t*	node;

	ut_ad(prebuilt && table && prebuilt->trx);
	
	if (prebuilt->upd_node == NULL) {

		/* Not called before for this handle: create an update node
		and query graph to the prebuilt struct */

		node = row_create_update_node_for_mysql(table, prebuilt->heap);

		prebuilt->upd_node = node;
		
		prebuilt->upd_graph =
			que_node_get_parent(
				pars_complete_graph_for_exec(node,
							prebuilt->trx,
							prebuilt->heap));
		prebuilt->upd_graph->state = QUE_FORK_ACTIVE;
	}

	return(prebuilt->upd_node->update);
}

/*************************************************************************
Does an update or delete of a row for MySQL. */

int
row_update_for_mysql(
/*=================*/
					/* out: error code or DB_SUCCESS */
	byte*		mysql_rec,	/* in: the row to be updated, in
					the MySQL format */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	trx_savept_t	savept;
	ulint		err;
	que_thr_t*	thr;
	ibool		was_lock_wait;
	dict_index_t*	clust_index; 
/*	ulint		ref_len; */
	upd_node_t*	node;
	dict_table_t*	table		= prebuilt->table;
	trx_t*		trx		= prebuilt->trx;
/*	mem_heap_t*	heap;
	dtuple_t*	search_tuple;
	dtuple_t*	row_tuple;
	mtr_t		mtr; */
	
	ut_ad(prebuilt && trx);
	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	UT_NOT_USED(mysql_rec);
	
	if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED) {
		fprintf(stderr,
		"InnoDB: Error: trying to free a corrupt\n"
		"InnoDB: table handle. Magic n %lu, table name %s\n",
		prebuilt->magic_n, prebuilt->table->name);

		mem_analyze_corruption((byte*)prebuilt);

		ut_a(0);
	}

	if (srv_created_new_raw || srv_force_recovery) {
		fprintf(stderr,
		"InnoDB: A new raw disk partition was initialized or\n"
		"InnoDB: innodb_force_recovery is on: we do not allow\n"
		"InnoDB: database modifications by the user. Shut down\n"
		"InnoDB: mysqld and edit my.cnf so that newraw is replaced\n"
		"InnoDB: with raw, and innodb_force_... is removed.\n");

		return(DB_ERROR);
	}

	trx->op_info = (char *) "updating or deleting";

	trx_start_if_not_started(trx);

	node = prebuilt->upd_node;

	clust_index = dict_table_get_first_index(table);

	if (prebuilt->pcur->btr_cur.index == clust_index) {
		btr_pcur_copy_stored_position(node->pcur, prebuilt->pcur);
	} else {
		btr_pcur_copy_stored_position(node->pcur,
							prebuilt->clust_pcur);
	}
		
	ut_a(node->pcur->rel_pos == BTR_PCUR_ON);
	 	
	/* MySQL seems to call rnd_pos before updating each row it
	has cached: we can get the correct cursor position from
	prebuilt->pcur; NOTE that we cannot build the row reference
	from mysql_rec if the clustered index was automatically
	generated for the table: MySQL does not know anything about
	the row id used as the clustered index key */

	savept = trx_savept_take(trx);
	
	thr = que_fork_get_first_thr(prebuilt->upd_graph);

	node->state = UPD_NODE_UPDATE_CLUSTERED;

	ut_ad(!prebuilt->sql_stat_start);

	que_thr_move_to_run_state_for_mysql(thr, trx);

run_again:
	thr->run_node = node;
	thr->prev_node = node;

	row_upd_step(thr);

	err = trx->error_state;

	if (err != DB_SUCCESS) {
		que_thr_stop_for_mysql(thr);
		
		if (err == DB_RECORD_NOT_FOUND) {
			trx->error_state = DB_SUCCESS;
			trx->op_info = (char *) "";

			return((int) err);
		}
	
		was_lock_wait = row_mysql_handle_errors(&err, trx, thr,
								&savept);
		if (was_lock_wait) {
			goto run_again;
		}

		trx->op_info = (char *) "";

		return(err);
	}

	que_thr_stop_for_mysql_no_error(thr, trx);

	if (node->is_delete) {
		if (prebuilt->table->stat_n_rows > 0) {
			prebuilt->table->stat_n_rows--;
		}

		srv_n_rows_deleted++;
	} else {
		srv_n_rows_updated++;
	}

	row_update_statistics_if_needed(prebuilt->table);

	trx->op_info = (char *) "";

	return((int) err);
}

/**************************************************************************
Does a cascaded delete or set null in a foreign key operation. */

ulint
row_update_cascade_for_mysql(
/*=========================*/
				/* out: error code or DB_SUCCESS */
	que_thr_t*	thr,	/* in: query thread */
	upd_node_t*	node,	/* in: update node used in the cascade
				or set null operation */
	dict_table_t*	table)	/* in: table where we do the operation */
{
	ulint	err;
	trx_t*	trx;

	trx = thr_get_trx(thr);
run_again:
	thr->run_node = node;
	thr->prev_node = node;

	row_upd_step(thr);

	err = trx->error_state;

	/* Note that the cascade node is a subnode of another InnoDB
	query graph node. We do a normal lock wait in this node, but
	all errors are handled by the parent node. */

	if (err == DB_LOCK_WAIT) {
		/* Handle lock wait here */
	
		que_thr_stop_for_mysql(thr);

		srv_suspend_mysql_thread(thr);

		/* Note that a lock wait may also end in a lock wait timeout,
		or this transaction is picked as a victim in selective
		deadlock resolution */

		if (trx->error_state != DB_SUCCESS) {

			return(trx->error_state);
		}

		/* Retry operation after a normal lock wait */
		
		goto run_again;
	}

	if (err != DB_SUCCESS) {

		return(err);
	}

	if (node->is_delete) {
		if (table->stat_n_rows > 0) {
			table->stat_n_rows--;
		}

		srv_n_rows_deleted++;
	} else {
		srv_n_rows_updated++;
	}

	row_update_statistics_if_needed(table);

	return(err);
}

/*************************************************************************
Checks if a table is such that we automatically created a clustered
index on it (on row id). */

ibool
row_table_got_default_clust_index(
/*==============================*/
	dict_table_t*	table)
{
	dict_index_t*	clust_index;

	clust_index = dict_table_get_first_index(table);

	if (dtype_get_mtype(dict_index_get_nth_type(clust_index, 0))
	 							== DATA_SYS) {
	 	return(TRUE);
	}

	return(FALSE);
}

/*************************************************************************
Calculates the key number used inside MySQL for an Innobase index. We have
to take into account if we generated a default clustered index for the table */

ulint
row_get_mysql_key_number_for_index(
/*===============================*/
	dict_index_t*	index)
{
	dict_index_t*	ind;
	ulint		i;

	ut_a(index);

	i = 0;
	ind = dict_table_get_first_index(index->table);

	while (index != ind) {
		ind = dict_table_get_next_index(ind);
		i++;
	}

	if (row_table_got_default_clust_index(index->table)) {
		ut_a(i > 0);
		i--;
	}

	return(i);
}

/*************************************************************************
Recovers an orphaned tmp table inside InnoDB by renaming it. In the table
name #sql becomes rsql, and "_recover_innodb_tmp_table" is catenated to
the end of name. table->name should be of the form
"dbname/rsql..._recover_innodb_tmp_table". This renames a table whose
name is "#sql..." */
static
int
row_mysql_recover_tmp_table(
/*========================*/
				/* out: error code or DB_SUCCESS */
	dict_table_t*	table,	/* in: table definition */
	trx_t*		trx)	/* in: transaction handle */
{
	char*	ptr;
	char	old_name[1000];

	ut_memcpy(old_name, table->name, ut_strlen(table->name) + 1);

	ptr = old_name;

	for (;;) {
		if (ptr >= old_name + ut_strlen(table->name) - 6) {
			trx_commit_for_mysql(trx);

			return(DB_ERROR);
		}

		if (0 == ut_memcmp(ptr, (char*)"/rsql", 5)) {
			ptr++;
			*ptr = '#';

			break;
		}

		ptr++;
	}

	old_name[ut_strlen(table->name)
			- ut_strlen("_recover_innodb_tmp_table")] = '\0';

	return(row_rename_table_for_mysql(old_name, table->name, trx));
}

/*************************************************************************
Locks the data dictionary in shared mode from modifications, for performing
foreign key check, rollback, or other operation invisible to MySQL. */

void
row_mysql_freeze_data_dictionary(
/*=============================*/
	trx_t*	trx)	/* in: transaction */
{
	ut_a(trx->dict_operation_lock_mode == 0);
	
	rw_lock_s_lock(&dict_operation_lock);

	trx->dict_operation_lock_mode = RW_S_LATCH;
}

/*************************************************************************
Unlocks the data dictionary shared lock. */

void
row_mysql_unfreeze_data_dictionary(
/*===============================*/
	trx_t*	trx)	/* in: transaction */
{
	ut_a(trx->dict_operation_lock_mode == RW_S_LATCH);

	rw_lock_s_unlock(&dict_operation_lock);

	trx->dict_operation_lock_mode = 0;
}

/*************************************************************************
Locks the data dictionary exclusively for performing a table create or other
data dictionary modification operation. */

void
row_mysql_lock_data_dictionary(
/*===========================*/
	trx_t*	trx)	/* in: transaction */
{
	ut_a(trx->dict_operation_lock_mode == 0);
	
	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks or lock waits can occur then in these operations */

	rw_lock_x_lock(&dict_operation_lock);
	trx->dict_operation_lock_mode = RW_X_LATCH;

	mutex_enter(&(dict_sys->mutex));
}

/*************************************************************************
Unlocks the data dictionary exclusive lock. */

void
row_mysql_unlock_data_dictionary(
/*=============================*/
	trx_t*	trx)	/* in: transaction */
{
	ut_a(trx->dict_operation_lock_mode == RW_X_LATCH);

	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks can occur then in these operations */

	mutex_exit(&(dict_sys->mutex));
	rw_lock_x_unlock(&dict_operation_lock);

	trx->dict_operation_lock_mode = 0;
}

/*************************************************************************
Does a table creation operation for MySQL. If the name of the created
table ends to characters INNODB_MONITOR, then this also starts
printing of monitor output by the master thread. */

int
row_create_table_for_mysql(
/*=======================*/
				/* out: error code or DB_SUCCESS */
	dict_table_t*	table,	/* in: table definition */
	trx_t*		trx)	/* in: transaction handle */
{
	tab_node_t*	node;
	mem_heap_t*	heap;
	que_thr_t*	thr;
	ulint		namelen;
	ulint		keywordlen;
	ulint		err;

	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
	ut_ad(trx->dict_operation_lock_mode == RW_X_LATCH);
	ut_ad(mutex_own(&(dict_sys->mutex)));
	
	if (srv_created_new_raw) {
		fprintf(stderr,
		"InnoDB: A new raw disk partition was initialized or\n"
		"InnoDB: innodb_force_recovery is on: we do not allow\n"
		"InnoDB: database modifications by the user. Shut down\n"
		"InnoDB: mysqld and edit my.cnf so that newraw is replaced\n"
		"InnoDB: with raw, and innodb_force_... is removed.\n");

		trx_commit_for_mysql(trx);

		return(DB_ERROR);
	}

	trx->op_info = (char *) "creating table";
	
	if (0 == ut_strcmp(table->name, (char*)"mysql/host")
	    || 0 == ut_strcmp(table->name, (char*)"mysql/user")
	    || 0 == ut_strcmp(table->name, (char*)"mysql/db")) {
	    	
		fprintf(stderr,
    "InnoDB: Error: trying to create a MySQL system table %s of type InnoDB.\n"
    "InnoDB: MySQL system tables must be of the MyISAM type!\n",
		table->name);

		trx_commit_for_mysql(trx);

		return(DB_ERROR);
	}

	trx_start_if_not_started(trx);

	namelen = ut_strlen(table->name);

	keywordlen = ut_strlen("_recover_innodb_tmp_table");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
 		     (char*)"_recover_innodb_tmp_table", keywordlen)) {

		/* MySQL prevents accessing of tables whose name begins
		with #sql, that is temporary tables. If mysqld crashes in
		the middle of an ALTER TABLE, we may get an orphaned
		#sql-table in the tablespace. We have here a special
		mechanism to recover such tables by renaming them to
		rsql... */
 				
		return(row_mysql_recover_tmp_table(table, trx));
	}

	namelen = ut_strlen(table->name);

	keywordlen = ut_strlen((char *) "innodb_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
 				(char *) "innodb_monitor", keywordlen)) {

		/* Table name ends to characters innodb_monitor:
		start monitor prints */
 				
		srv_print_innodb_monitor = TRUE;

		/* The lock timeout monitor thread also takes care
		of InnoDB monitor prints */

		os_event_set(srv_lock_timeout_thread_event);
	}

	keywordlen = ut_strlen((char *) "innodb_lock_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
 				(char *) "innodb_lock_monitor", keywordlen)) {

		srv_print_innodb_monitor = TRUE;
		srv_print_innodb_lock_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	}

	keywordlen = ut_strlen((char *) "innodb_tablespace_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
				      (char *) "innodb_tablespace_monitor", 
				      keywordlen)) {

		srv_print_innodb_tablespace_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	}

	keywordlen = ut_strlen((char *) "innodb_table_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
				      (char *) "innodb_table_monitor",
				      keywordlen)) {

		srv_print_innodb_table_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	}

	keywordlen = ut_strlen("innodb_mem_validate");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(table->name + namelen - keywordlen,
 				(char*)"innodb_mem_validate", keywordlen)) {

	        /* We define here a debugging feature intended for
		developers */

	        printf("Validating InnoDB memory:\n"
		 "to use this feature you must compile InnoDB with\n"
		 "UNIV_MEM_DEBUG defined in univ.i and the server must be\n"
		 "quiet because allocation from a mem heap is not protected\n"
		 "by any semaphore.\n");

		ut_a(mem_validate());
		      
		printf("Memory validated\n");
	}

	heap = mem_heap_create(512);

	trx->dict_operation = TRUE;
	
	node = tab_create_graph_create(table, heap);

	thr = pars_complete_graph_for_exec(node, trx, heap);

	ut_a(thr == que_fork_start_command(que_node_get_parent(thr),
						SESS_COMM_EXECUTE, 0));
	que_run_threads(thr);

	err = trx->error_state;

	if (err != DB_SUCCESS) {
		/* We have special error handling here */
		
		trx->error_state = DB_SUCCESS;
		
		trx_general_rollback_for_mysql(trx, FALSE, NULL);

		if (err == DB_OUT_OF_FILE_SPACE) {
			fprintf(stderr, 
     "InnoDB: Warning: cannot create table %s because tablespace full\n",
				 table->name);
		     	row_drop_table_for_mysql(table->name, trx);
		} else {
		       	ut_a(err == DB_DUPLICATE_KEY);

	    		ut_print_timestamp(stderr);

			fprintf(stderr, 
     "  InnoDB: Error: table %s already exists in InnoDB internal\n"
     "InnoDB: data dictionary. Have you deleted the .frm file\n"
     "InnoDB: and not used DROP TABLE? Have you used DROP DATABASE\n"
     "InnoDB: for InnoDB tables in MySQL version <= 3.23.43?\n"
     "InnoDB: See the Restrictions section of the InnoDB manual.\n",
				 table->name);
			fprintf(stderr,
     "InnoDB: You can drop the orphaned table inside InnoDB by\n"
     "InnoDB: creating an InnoDB table with the same name in another\n"
     "InnoDB: database and moving the .frm file to the current database.\n"
     "InnoDB: Then MySQL thinks the table exists, and DROP TABLE will\n"
     "InnoDB: succeed.\n"
     "InnoDB: You can look further help from section 15.1 of\n"
     "InnoDB: http://www.innodb.com/ibman.html\n");
		}

		trx->error_state = DB_SUCCESS;
	}

	que_graph_free((que_t*) que_node_get_parent(thr));

	trx->op_info = (char *) "";

	return((int) err);
}

/*************************************************************************
Does an index creation operation for MySQL. TODO: currently failure
to create an index results in dropping the whole table! This is no problem
currently as all indexes must be created at the same time as the table. */

int
row_create_index_for_mysql(
/*=======================*/
					/* out: error number or DB_SUCCESS */
	dict_index_t*	index,		/* in: index definition */
	trx_t*		trx)		/* in: transaction handle */
{
	ind_node_t*	node;
	mem_heap_t*	heap;
	que_thr_t*	thr;
	ulint		namelen;
	ulint		keywordlen;
	ulint		err;
	ulint		i, j;
	
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	
	trx->op_info = (char *) "creating index";

	/* Check that the same column does not appear twice in the index.
	Starting from 4.0.14 InnoDB should be able to cope with that, but
	safer not to allow them. */

	for (i = 0; i < dict_index_get_n_fields(index); i++) {
		for (j = 0; j < i; j++) {
			if (0 == ut_strcmp(
			      dict_index_get_nth_field(index, j)->name,
			      dict_index_get_nth_field(index, i)->name)) {

				ut_print_timestamp(stderr);

				fprintf(stderr,
"  InnoDB: Error: column %s appears twice in index %s of table %s\n"
"InnoDB: This is not allowed in InnoDB.\n",
				dict_index_get_nth_field(index, i)->name,
				index->name, index->table_name);

				err = DB_COL_APPEARS_TWICE_IN_INDEX;

				goto error_handling;
			}
		}
	}

	trx_start_if_not_started(trx);

	namelen = ut_strlen(index->table_name);

	keywordlen = ut_strlen("_recover_innodb_tmp_table");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(
				index->table_name + namelen - keywordlen,
 			(char*)"_recover_innodb_tmp_table", keywordlen)) {

		return(DB_SUCCESS);
	}

	heap = mem_heap_create(512);

	trx->dict_operation = TRUE;

	node = ind_create_graph_create(index, heap);

	thr = pars_complete_graph_for_exec(node, trx, heap);

	ut_a(thr == que_fork_start_command(que_node_get_parent(thr),
						SESS_COMM_EXECUTE, 0));
	que_run_threads(thr);

 	err = trx->error_state;

	que_graph_free((que_t*) que_node_get_parent(thr));

error_handling:

	if (err != DB_SUCCESS) {
		/* We have special error handling here */
		
		trx->error_state = DB_SUCCESS;

		trx_general_rollback_for_mysql(trx, FALSE, NULL);

		row_drop_table_for_mysql(index->table_name, trx);

		trx->error_state = DB_SUCCESS;
	}
	
	trx->op_info = (char *) "";

	return((int) err);
}

/*************************************************************************
Scans a table create SQL string and adds to the data dictionary
the foreign key constraints declared in the string. This function
should be called after the indexes for a table have been created.
Each foreign key constraint must be accompanied with indexes in
bot participating tables. The indexes are allowed to contain more
fields than mentioned in the constraint. Check also that foreign key
constraints which reference this table are ok. */

int
row_table_add_foreign_constraints(
/*==============================*/
				/* out: error code or DB_SUCCESS */
	trx_t*	trx,		/* in: transaction */
	char*	sql_string,	/* in: table create statement where
				foreign keys are declared like:
				FOREIGN KEY (a, b) REFERENCES table2(c, d),
				table2 can be written also with the database
				name before it: test.table2 */
	char*	name)		/* in: table full name in the normalized form
				database_name/table_name */
{
	ulint	namelen;
	ulint	keywordlen;
	ulint	err;

	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
	ut_a(sql_string);
	
	trx->op_info = (char *) "adding foreign keys";

	trx_start_if_not_started(trx);

	namelen = ut_strlen(name);

	keywordlen = ut_strlen("_recover_innodb_tmp_table");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(
				name + namelen - keywordlen,
 			(char*)"_recover_innodb_tmp_table", keywordlen)) {

		return(DB_SUCCESS);
	}

	trx->dict_operation = TRUE;

	err = dict_create_foreign_constraints(trx, sql_string, name);

	if (err == DB_SUCCESS) {
		/* Check that also referencing constraints are ok */
		err = dict_load_foreigns(name);
	}

	if (err != DB_SUCCESS) {
		/* We have special error handling here */
		
		trx->error_state = DB_SUCCESS;

		trx_general_rollback_for_mysql(trx, FALSE, NULL);

		row_drop_table_for_mysql(name, trx);

		trx->error_state = DB_SUCCESS;
	}

	return((int) err);
}

/*************************************************************************
Drops a table for MySQL as a background operation. MySQL relies on Unix
in ALTER TABLE to the fact that the table handler does not remove the
table before all handles to it has been removed. Furhermore, the MySQL's
call to drop table must be non-blocking. Therefore we do the drop table
as a background operation, which is taken care of by the master thread
in srv0srv.c. */
static
int
row_drop_table_for_mysql_in_background(
/*===================================*/
			/* out: error code or DB_SUCCESS */
	char*	name)	/* in: table name */
{
	ulint	error;
	trx_t*	trx;

	trx = trx_allocate_for_background();

/*	fprintf(stderr, "InnoDB: Dropping table %s in background drop list\n",
							name); */
  	/* Drop the table in InnoDB */

  	error = row_drop_table_for_mysql(name, trx);

	if (error != DB_SUCCESS) {
		fprintf(stderr,
	"InnoDB: Error: Dropping table %s in background drop list failed\n",
								name);
	}
  	
	/* Flush the log to reduce probability that the .frm files and
	the InnoDB data dictionary get out-of-sync if the user runs
	with innodb_flush_log_at_trx_commit = 0 */
	
	log_buffer_flush_to_disk();

  	trx_commit_for_mysql(trx);

  	trx_free_for_background(trx);

	return(DB_SUCCESS);
}

/*************************************************************************
The master thread in srv0srv.c calls this regularly to drop tables which
we must drop in background after queries to them have ended. Such lazy
dropping of tables is needed in ALTER TABLE on Unix. */

ulint
row_drop_tables_for_mysql_in_background(void)
/*=========================================*/
					/* out: how many tables dropped
					+ remaining tables in list */
{
	row_mysql_drop_t*	drop;
	dict_table_t*		table;
	ulint			n_tables;
	ulint			n_tables_dropped = 0;
loop:	
	mutex_enter(&kernel_mutex);

	if (!row_mysql_drop_list_inited) {

		UT_LIST_INIT(row_mysql_drop_list);
		row_mysql_drop_list_inited = TRUE;
	}

	drop = UT_LIST_GET_FIRST(row_mysql_drop_list);
	
	n_tables = UT_LIST_GET_LEN(row_mysql_drop_list);

	mutex_exit(&kernel_mutex);

	if (drop == NULL) {

		return(n_tables + n_tables_dropped);
	}

	mutex_enter(&(dict_sys->mutex));
	table = dict_table_get_low(drop->table_name);
	mutex_exit(&(dict_sys->mutex));

	if (table == NULL) {
	        /* If for some reason the table has already been dropped
		through some other mechanism, do not try to drop it */

	        goto already_dropped;
	}

	if (table->n_mysql_handles_opened > 0
				|| table->n_foreign_key_checks_running > 0) {

		return(n_tables + n_tables_dropped);
	}

	n_tables_dropped++;
							
	row_drop_table_for_mysql_in_background(drop->table_name);

already_dropped:
	mutex_enter(&kernel_mutex);

	UT_LIST_REMOVE(row_mysql_drop_list, row_mysql_drop_list, drop);

        ut_print_timestamp(stderr);
        fprintf(stderr,
		"  InnoDB: Dropped table %s in background drop queue.\n",
		drop->table_name);

	mem_free(drop->table_name);

	mem_free(drop);

	mutex_exit(&kernel_mutex);

	goto loop;
}

/*************************************************************************
Get the background drop list length. NOTE: the caller must own the kernel
mutex! */

ulint
row_get_background_drop_list_len_low(void)
/*======================================*/
					/* out: how many tables in list */
{
	ut_ad(mutex_own(&kernel_mutex));

	if (!row_mysql_drop_list_inited) {

		UT_LIST_INIT(row_mysql_drop_list);
		row_mysql_drop_list_inited = TRUE;
	}
	
	return(UT_LIST_GET_LEN(row_mysql_drop_list));
}

/*************************************************************************
Adds a table to the list of tables which the master thread drops in
background. We need this on Unix because in ALTER TABLE MySQL may call
drop table even if the table has running queries on it. */
static
void
row_add_table_to_background_drop_list(
/*==================================*/
	dict_table_t*	table)	/* in: table */
{
	row_mysql_drop_t*	drop;
	
	drop = mem_alloc(sizeof(row_mysql_drop_t));

	drop->table_name = mem_alloc(1 + ut_strlen(table->name));

	ut_memcpy(drop->table_name, table->name, 1 + ut_strlen(table->name));

	mutex_enter(&kernel_mutex);

	if (!row_mysql_drop_list_inited) {

		UT_LIST_INIT(row_mysql_drop_list);
		row_mysql_drop_list_inited = TRUE;
	}

	UT_LIST_ADD_LAST(row_mysql_drop_list, row_mysql_drop_list, drop);
	
/*	fprintf(stderr, "InnoDB: Adding table %s to background drop list\n",
							drop->table_name); */
	mutex_exit(&kernel_mutex);
}

/*************************************************************************
Drops a table for MySQL. If the name of the dropped table ends to
characters INNODB_MONITOR, then this also stops printing of monitor
output by the master thread. */

int
row_drop_table_for_mysql(
/*=====================*/
				/* out: error code or DB_SUCCESS */
	char*	name,		/* in: table name */
	trx_t*	trx)		/* in: transaction handle */
{
	dict_table_t*	table;
	que_thr_t*	thr;
	que_t*		graph;
	ulint		err;
	char*		str1;
	char*		str2;
	ulint		len;
	ulint		namelen;
	ulint		keywordlen;
	ibool		locked_dictionary	= FALSE;
	char		buf[10000];

	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	ut_a(name != NULL);

	if (srv_created_new_raw) {
		fprintf(stderr,
		"InnoDB: A new raw disk partition was initialized or\n"
		"InnoDB: innodb_force_recovery is on: we do not allow\n"
		"InnoDB: database modifications by the user. Shut down\n"
		"InnoDB: mysqld and edit my.cnf so that newraw is replaced\n"
		"InnoDB: with raw, and innodb_force_... is removed.\n");

		return(DB_ERROR);
	}

	trx->op_info = (char *) "dropping table";

	trx_start_if_not_started(trx);

	namelen = ut_strlen(name);
	keywordlen = ut_strlen((char *) "innodb_monitor");

	if (namelen >= keywordlen
	    && 0 == ut_memcmp(name + namelen - keywordlen,
			      (char *) "innodb_monitor", keywordlen)) {

		/* Table name ends to characters innodb_monitor:
		stop monitor prints */
 				
		srv_print_innodb_monitor = FALSE;
		srv_print_innodb_lock_monitor = FALSE;
	}

	keywordlen = ut_strlen((char *) "innodb_lock_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(name + namelen - keywordlen,
				      (char *) "innodb_lock_monitor",
				      keywordlen)) {

		srv_print_innodb_monitor = FALSE;
		srv_print_innodb_lock_monitor = FALSE;
	}

	keywordlen = ut_strlen((char *) "innodb_tablespace_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(name + namelen - keywordlen,
				      (char *) "innodb_tablespace_monitor",
				      keywordlen)) {

		srv_print_innodb_tablespace_monitor = FALSE;
	}

	keywordlen = ut_strlen((char *) "innodb_table_monitor");

	if (namelen >= keywordlen
		    && 0 == ut_memcmp(name + namelen - keywordlen,
				      (char *) "innodb_table_monitor",
				      keywordlen)) {

		srv_print_innodb_table_monitor = FALSE;
	}

	/* We use the private SQL parser of Innobase to generate the
	query graphs needed in deleting the dictionary data from system
	tables in Innobase. Deleting a row from SYS_INDEXES table also
	frees the file segments of the B-tree associated with the index. */

	str1 = (char *) 
	"PROCEDURE DROP_TABLE_PROC () IS\n"
	"table_name CHAR;\n"
	"sys_foreign_id CHAR;\n"
	"table_id CHAR;\n"
	"index_id CHAR;\n"
	"foreign_id CHAR;\n"
	"found INT;\n"
	"BEGIN\n"
	"table_name := '";
	
	str2 = (char *) 
	"';\n"
	"SELECT ID INTO table_id\n"
	"FROM SYS_TABLES\n"
	"WHERE NAME = table_name;\n"
	"IF (SQL % NOTFOUND) THEN\n"
	"	COMMIT WORK;\n"
	"	RETURN;\n"
	"END IF;\n"
	"found := 1;\n"
	"SELECT ID INTO sys_foreign_id\n"
	"FROM SYS_TABLES\n"
	"WHERE NAME = 'SYS_FOREIGN';\n"
	"IF (SQL % NOTFOUND) THEN\n"
	"	found := 0;\n"
	"END IF;\n"
	"IF (table_name = 'SYS_FOREIGN') THEN\n"
	"	found := 0;\n"
	"END IF;\n"
	"IF (table_name = 'SYS_FOREIGN_COLS') THEN\n"
	"	found := 0;\n"
	"END IF;\n"
	"WHILE found = 1 LOOP\n"
	"	SELECT ID INTO foreign_id\n"
	"	FROM SYS_FOREIGN\n"
	"	WHERE FOR_NAME = table_name;\n"	
	"	IF (SQL % NOTFOUND) THEN\n"
	"		found := 0;\n"
	"	ELSE"
	"		DELETE FROM SYS_FOREIGN_COLS WHERE ID = foreign_id;\n"
	"		DELETE FROM SYS_FOREIGN WHERE ID = foreign_id;\n"
	"	END IF;\n"
	"END LOOP;\n"
	"found := 1;\n"
	"WHILE found = 1 LOOP\n"
	"	SELECT ID INTO index_id\n"
	"	FROM SYS_INDEXES\n"
	"	WHERE TABLE_ID = table_id;\n"	
	"	IF (SQL % NOTFOUND) THEN\n"
	"		found := 0;\n"
	"	ELSE"
	"		DELETE FROM SYS_FIELDS WHERE INDEX_ID = index_id;\n"
	"		DELETE FROM SYS_INDEXES WHERE ID = index_id\n"
	"					 AND TABLE_ID = table_id;\n"
	"	END IF;\n"
	"END LOOP;\n"
	"DELETE FROM SYS_COLUMNS WHERE TABLE_ID = table_id;\n"
	"DELETE FROM SYS_TABLES WHERE ID = table_id;\n"
	"COMMIT WORK;\n"
	"END;\n";

	len = ut_strlen(str1);

	ut_memcpy(buf, str1, len);
	ut_memcpy(buf + len, name, ut_strlen(name));

	len += ut_strlen(name);

	ut_memcpy(buf + len, str2, ut_strlen(str2) + 1);

	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks can occur then in these operations */

	if (trx->dict_operation_lock_mode != RW_X_LATCH) {
		/* Prevent foreign key checks etc. while we are dropping the
		table */

		row_mysql_lock_data_dictionary(trx);

		locked_dictionary = TRUE;
	}

	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
	
	graph = pars_sql(buf);

	ut_a(graph);

	graph->trx = trx;
	trx->graph = NULL;

	graph->fork_type = QUE_FORK_MYSQL_INTERFACE;

	table = dict_table_get_low(name);

	if (!table) {
		err = DB_TABLE_NOT_FOUND;
	    	ut_print_timestamp(stderr);

		fprintf(stderr, 
     	"  InnoDB: Error: table %s does not exist in the InnoDB internal\n"
     	"InnoDB: data dictionary though MySQL is trying to drop it.\n"
     	"InnoDB: Have you copied the .frm file of the table to the\n"
	"InnoDB: MySQL database directory from another database?\n"
	"InnoDB: You can look further help from section 15.1 of\n"
        "InnoDB: http://www.innodb.com/ibman.html\n",
				 name);
		goto funct_exit;
	}

	if (table->n_mysql_handles_opened > 0) {
		
	        ut_print_timestamp(stderr);
	        fprintf(stderr,
		  "  InnoDB: Warning: MySQL is trying to drop table %s\n"
		  "InnoDB: though there are still open handles to it.\n"
		  "InnoDB: Adding the table to the background drop queue.\n",
		  table->name);

		row_add_table_to_background_drop_list(table);

		err = DB_SUCCESS;

		goto funct_exit;
	}

	if (table->n_foreign_key_checks_running > 0) {
		
	        ut_print_timestamp(stderr);
	        fprintf(stderr,
		  "  InnoDB: You are trying to drop table %s\n"
		  "InnoDB: though there are foreign key check running on it.\n"
		  "InnoDB: Adding the table to the background drop queue.\n",
		  table->name);

		row_add_table_to_background_drop_list(table);

		err = DB_SUCCESS;

		goto funct_exit;
	}
	
	/* Remove any locks there are on the table or its records */
	
	lock_reset_all_on_table(table);

	trx->dict_operation = TRUE;
	trx->table_id = table->id;

	ut_a(thr = que_fork_start_command(graph, SESS_COMM_EXECUTE, 0));

	que_run_threads(thr);

	err = trx->error_state;

	if (err != DB_SUCCESS) {
		ut_a(err == DB_OUT_OF_FILE_SPACE);

		err = DB_MUST_GET_MORE_FILE_SPACE;
		
		row_mysql_handle_errors(&err, trx, thr, NULL);

		ut_a(0);
	} else {
		dict_table_remove_from_cache(table);

		if (dict_load_table(name) != NULL) {
			ut_print_timestamp(stderr);
			fprintf(stderr,
"  InnoDB: Error: dropping of table %s failed!\n", name);

		}
	}
funct_exit:

	if (locked_dictionary) {
		row_mysql_unlock_data_dictionary(trx);	
	}

	que_graph_free(graph);
	
  	trx_commit_for_mysql(trx);

	trx->op_info = (char *) "";

	srv_wake_master_thread();

	return((int) err);
}

/*************************************************************************
Drops a database for MySQL. */

int
row_drop_database_for_mysql(
/*========================*/
			/* out: error code or DB_SUCCESS */
	char*	name,	/* in: database name which ends to '/' */
	trx_t*	trx)	/* in: transaction handle */
{
        dict_table_t* table;
	char*	table_name;
	int	err	= DB_SUCCESS;
	
	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	ut_a(name != NULL);
	ut_a(name[strlen(name) - 1] == '/');
	
	trx->op_info = (char *) "dropping database";
	
	trx_start_if_not_started(trx);
loop:
	row_mysql_lock_data_dictionary(trx);

	while ((table_name = dict_get_first_table_name_in_db(name))) {
		ut_a(memcmp(table_name, name, strlen(name)) == 0);

		table = dict_table_get_low(table_name);

		ut_a(table);

		/* Wait until MySQL does not have any queries running on
		the table */

		if (table->n_mysql_handles_opened > 0) {
			row_mysql_unlock_data_dictionary(trx);

			ut_print_timestamp(stderr);
			fprintf(stderr,
		"  InnoDB: Warning: MySQL is trying to drop database %s\n"
	    	"InnoDB: though there are still open handles to table %s.\n",
				name, table_name);

		        os_thread_sleep(1000000);

		        mem_free(table_name);

		        goto loop;
		}

		err = row_drop_table_for_mysql(table_name, trx);

		mem_free(table_name);

		if (err != DB_SUCCESS) {
			fprintf(stderr,
	"InnoDB: DROP DATABASE %s failed with error %lu for table %s\n",
				name, (ulint)err, table_name);
			break;
		}
	}

	row_mysql_unlock_data_dictionary(trx);
	
	trx_commit_for_mysql(trx);

	trx->op_info = (char *) "";

	return(err);
}

/*************************************************************************
Checks if a table name contains the string "/#sql" which denotes temporary
tables in MySQL. */
static
ibool
row_is_mysql_tmp_table_name(
/*========================*/
			/* out: TRUE if temporary table */
	char*	name)	/* in: table name in the form 'database/tablename' */
{
	ulint	i;

	for (i = 0; i <= ut_strlen(name) - 5; i++) {
		if (ut_memcmp(name + i, (char*)"/#sql", 5) == 0) {

			return(TRUE);
		}
	}

	return(FALSE);
}

/*************************************************************************
Renames a table for MySQL. */

int
row_rename_table_for_mysql(
/*=======================*/
				/* out: error code or DB_SUCCESS */
	char*	old_name,	/* in: old table name */
	char*	new_name,	/* in: new table name */
	trx_t*	trx)		/* in: transaction handle */
{
	dict_table_t*	table;
	que_thr_t*	thr;
	que_t*		graph			= NULL;
	ulint		err;
	char*		str1;
	char*		str2;
	char*		str3;
	mem_heap_t*	heap			= NULL;
	char**		constraints_to_drop	= NULL;
	ulint		n_constraints_to_drop	= 0;
        ibool           recovering_temp_table   = FALSE;
        ulint           namelen;
        ulint           keywordlen;
	ulint		len;
	ulint		i;
	char		buf[10000];

	ut_ad(trx->mysql_thread_id == os_thread_get_curr_id());
	ut_a(old_name != NULL);
	ut_a(new_name != NULL);

	if (srv_created_new_raw || srv_force_recovery) {
		fprintf(stderr,
		"InnoDB: A new raw disk partition was initialized or\n"
		"InnoDB: innodb_force_recovery is on: we do not allow\n"
		"InnoDB: database modifications by the user. Shut down\n"
		"InnoDB: mysqld and edit my.cnf so that newraw is replaced\n"
		"InnoDB: with raw, and innodb_force_... is removed.\n");

  		trx_commit_for_mysql(trx);
		return(DB_ERROR);
	}
	
	if (0 == ut_strcmp(new_name, (char*)"mysql/host")
	    || 0 == ut_strcmp(new_name, (char*)"mysql/user")
	    || 0 == ut_strcmp(new_name, (char*)"mysql/db")) {
	    	
		fprintf(stderr,
    "InnoDB: Error: trying to create a MySQL system table %s of type InnoDB.\n"
    "InnoDB: MySQL system tables must be of the MyISAM type!\n",
		new_name);

  		trx_commit_for_mysql(trx);
		return(DB_ERROR);
	}

	trx->op_info = (char *) "renaming table";
	trx_start_if_not_started(trx);

        namelen = ut_strlen(new_name);

        keywordlen = ut_strlen("_recover_innodb_tmp_table");

        if (namelen >= keywordlen
                    && 0 == ut_memcmp(new_name + namelen - keywordlen,
                     (char*)"_recover_innodb_tmp_table", keywordlen)) {

                recovering_temp_table = TRUE;
        }

	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks can occur then in these operations */

	if (!recovering_temp_table) {		
		row_mysql_lock_data_dictionary(trx);
	}

	table = dict_table_get_low(old_name);

	if (!table) {
		err = DB_TABLE_NOT_FOUND;

		goto funct_exit;
	}

	str1 = (char *) 
	"PROCEDURE RENAME_TABLE_PROC () IS\n"
	"new_table_name CHAR;\n"
	"old_table_name CHAR;\n"
	"BEGIN\n"
	"new_table_name :='";

	str2 = (char *) 
	"';\nold_table_name := '";

	if (row_is_mysql_tmp_table_name(new_name)) {

		/* MySQL is doing an ALTER TABLE command and it renames the
		original table to a temporary table name. We want to preserve
		the original foreign key constraint definitions despite the
		name change. An exception is those constraints for which
		the ALTER TABLE contained DROP FOREIGN KEY <foreign key id>.*/

		heap = mem_heap_create(100);
		
		err = dict_foreign_parse_drop_constraints(heap, trx,
					table,
					&n_constraints_to_drop,
					&constraints_to_drop);
		if (err != DB_SUCCESS) {

			goto funct_exit;
		}
		
		str3 = mem_heap_alloc(heap,
					1000 + 500 * n_constraints_to_drop);
		*str3 = '\0';
		sprintf(str3,
			"';\n"
			"UPDATE SYS_TABLES SET NAME = new_table_name\n"
			"WHERE NAME = old_table_name;\n");

		for (i = 0; i < n_constraints_to_drop; i++) {
			sprintf(str3 + strlen(str3),
			"DELETE FROM SYS_FOREIGN_COLS WHERE ID = '%s';\n"
			"DELETE FROM SYS_FOREIGN WHERE ID = '%s';\n",
				constraints_to_drop[i],
				constraints_to_drop[i]);
		}

		sprintf(str3 + strlen(str3),
			"END;\n");

		ut_a(strlen(str3) < 1000 + 500 * n_constraints_to_drop);
	} else {
		str3 = (char*)
		"';\n"
		"UPDATE SYS_TABLES SET NAME = new_table_name\n"
		"WHERE NAME = old_table_name;\n"
		"UPDATE SYS_FOREIGN SET FOR_NAME = new_table_name\n"
		"WHERE FOR_NAME = old_table_name;\n"
		"UPDATE SYS_FOREIGN SET REF_NAME = new_table_name\n"
		"WHERE REF_NAME = old_table_name;\n"
		"END;\n";
	}

	len = ut_strlen(str1);

	ut_memcpy(buf, str1, len);

	ut_memcpy(buf + len, new_name, ut_strlen(new_name));

	len += ut_strlen(new_name);

	ut_memcpy(buf + len, str2, ut_strlen(str2));

	len += ut_strlen(str2);

	ut_memcpy(buf + len, old_name, ut_strlen(old_name));

	len += ut_strlen(old_name);

	ut_memcpy(buf + len, str3, ut_strlen(str3) + 1);
	
	graph = pars_sql(buf);

	ut_a(graph);

	graph->trx = trx;
	trx->graph = NULL;

	graph->fork_type = QUE_FORK_MYSQL_INTERFACE;

	ut_a(thr = que_fork_start_command(graph, SESS_COMM_EXECUTE, 0));

	que_run_threads(thr);

	err = trx->error_state;

	if (err != DB_SUCCESS) {
		if (err == DB_DUPLICATE_KEY) {
	    		ut_print_timestamp(stderr);

			fprintf(stderr,
     "  InnoDB: Error: table %s exists in the InnoDB internal data\n"
     "InnoDB: dictionary though MySQL is trying rename table %s to it.\n"
     "InnoDB: Have you deleted the .frm file and not used DROP TABLE?\n"
     "InnoDB: You can look further help from section 15.1 of\n"
     "InnoDB: http://www.innodb.com/ibman.html\n",
			new_name, old_name);

			fprintf(stderr,
     "InnoDB: If table %s is a temporary table #sql..., then it can be that\n"
     "InnoDB: there are still queries running on the table, and it will be\n"
     "InnoDB: dropped automatically when the queries end.\n", new_name);
			
			fprintf(stderr,
     "InnoDB: You can drop the orphaned table inside InnoDB by\n"
     "InnoDB: creating an InnoDB table with the same name in another\n"
     "InnoDB: database and moving the .frm file to the current database.\n"
     "InnoDB: Then MySQL thinks the table exists, and DROP TABLE will\n"
     "InnoDB: succeed.\n");
		}

		trx->error_state = DB_SUCCESS;
		trx_general_rollback_for_mysql(trx, FALSE, NULL);
		trx->error_state = DB_SUCCESS;
	} else {
		ut_a(dict_table_rename_in_cache(table, new_name,
				!row_is_mysql_tmp_table_name(new_name)));

		if (row_is_mysql_tmp_table_name(old_name)) {

			/* MySQL is doing an ALTER TABLE command and it
			renames the created temporary table to the name
			of the original table. In the ALTER TABLE we maybe
			created some FOREIGN KEY constraints for the temporary
			table. But we want to load also the foreign key
			constraint definitions for the original table name. */

			err = dict_load_foreigns(new_name);

			if (err != DB_SUCCESS) {

	    			ut_print_timestamp(stderr);

				fprintf(stderr,
     "  InnoDB: Error: in ALTER TABLE table %s\n"
     "InnoDB: has or is referenced in foreign key constraints\n"
     "InnoDB: which are not compatible with the new table definition.\n",
     new_name);
     
				ut_a(dict_table_rename_in_cache(table,
							old_name, FALSE));
						
				trx->error_state = DB_SUCCESS;
				trx_general_rollback_for_mysql(trx, FALSE,
									NULL);
				trx->error_state = DB_SUCCESS;
			}
		}
	}
funct_exit:
	if (!recovering_temp_table) {		
		row_mysql_unlock_data_dictionary(trx);
	}

	if (graph) {
		que_graph_free(graph);
	}

	if (heap) {
		mem_heap_free(heap);
	}
	
  	trx_commit_for_mysql(trx);

	trx->op_info = (char *) "";

	return((int) err);
}

/*************************************************************************
Checks that the index contains entries in an ascending order, unique
constraint is not broken, and calculates the number of index entries
in the read view of the current transaction. */
static
ibool
row_scan_and_check_index(
/*=====================*/
					/* out: TRUE if ok */
	row_prebuilt_t*	prebuilt,	/* in: prebuilt struct in MySQL */
	dict_index_t*	index,		/* in: index */
	ulint*		n_rows)		/* out: number of entries seen in the
					current consistent read */
{
	mem_heap_t*	heap;
	dtuple_t*	prev_entry = NULL;
	ulint		matched_fields;
	ulint		matched_bytes;
	byte*		buf;
	ulint		ret;
	rec_t*		rec;
	ibool		is_ok	= TRUE;
	int		cmp;
	ibool		contains_null;
	ulint		i;
	char           	err_buf[1000];
	
	*n_rows = 0;
	
	buf = mem_alloc(UNIV_PAGE_SIZE);
	heap = mem_heap_create(100);
	
	/* Make a dummy template in prebuilt, which we will use
	in scanning the index entries */

	prebuilt->index = index;
	prebuilt->sql_stat_start = TRUE;
	prebuilt->template_type = ROW_MYSQL_DUMMY_TEMPLATE;
	prebuilt->n_template = 0;
	prebuilt->need_to_access_clustered = FALSE;

 	dtuple_set_n_fields(prebuilt->search_tuple, 0);

	prebuilt->select_lock_type = LOCK_NONE;

	ret = row_search_for_mysql(buf, PAGE_CUR_G, prebuilt, 0, 0);
loop:
	if (ret != DB_SUCCESS) {

		mem_free(buf);
		mem_heap_free(heap);

		return(is_ok);
	}

	*n_rows = *n_rows + 1;
	
	/* row_search... returns the index record in buf, record origin offset
	within buf stored in the first 4 bytes, because we have built a dummy
	template */
	
	rec = buf + mach_read_from_4(buf);
	
	if (prev_entry != NULL) {
		matched_fields = 0;
		matched_bytes = 0;
	
		cmp = cmp_dtuple_rec_with_match(prev_entry, rec,
						&matched_fields,
						&matched_bytes);
		contains_null = FALSE;

		/* In a unique secondary index we allow equal key values if
		they contain SQL NULLs */

	        for (i = 0;
                     i < dict_index_get_n_ordering_defined_by_user(index);
		     i++) {
	                if (UNIV_SQL_NULL == dfield_get_len(
                                      dtuple_get_nth_field(prev_entry, i))) {

                        	contains_null = TRUE;
	                }
	        }

		if (cmp > 0) {
			fprintf(stderr,
			"Error: index records in a wrong order in index %s\n",
								index->name);

	  		dtuple_sprintf(err_buf, 900, prev_entry);
	  		fprintf(stderr, "InnoDB: prev record %s\n", err_buf);

	  		rec_sprintf(err_buf, 900, rec);
	  		fprintf(stderr, "InnoDB: record %s\n", err_buf);

			is_ok = FALSE;
		} else if ((index->type & DICT_UNIQUE)
			   && !contains_null
			   && matched_fields >=
			   dict_index_get_n_ordering_defined_by_user(index)) {

			fprintf(stderr, "Error: duplicate key in index %s\n",
								index->name);

	  		dtuple_sprintf(err_buf, 900, prev_entry);
	  		fprintf(stderr, "InnoDB: prev record %s\n", err_buf);

	  		rec_sprintf(err_buf, 900, rec);
	  		fprintf(stderr, "InnoDB: record %s\n", err_buf);

			is_ok = FALSE;			   	
		}
	}

	mem_heap_empty(heap);
	
	prev_entry = row_rec_to_index_entry(ROW_COPY_DATA, index, rec, heap);

	ret = row_search_for_mysql(buf, PAGE_CUR_G, prebuilt, 0, ROW_SEL_NEXT);

	goto loop;	
}

/*************************************************************************
Checks a table for corruption. */

ulint
row_check_table_for_mysql(
/*======================*/
					/* out: DB_ERROR or DB_SUCCESS */
	row_prebuilt_t*	prebuilt)	/* in: prebuilt struct in MySQL
					handle */
{
	dict_table_t*	table		= prebuilt->table;
	dict_index_t*	index;
	ulint		n_rows;
	ulint		n_rows_in_table	= ULINT_UNDEFINED;
	ulint		ret 		= DB_SUCCESS;
	ulint		old_isolation_level;
	
	prebuilt->trx->op_info = (char *) "checking table";

	old_isolation_level = prebuilt->trx->isolation_level;

	/* We must run the index record counts at an isolation level
	>= READ COMMITTED, because a dirty read can see a wrong number
	of records in some index; to play safe, we use always
	REPEATABLE READ here */

	prebuilt->trx->isolation_level = TRX_ISO_REPEATABLE_READ;
	
	index = dict_table_get_first_index(table);

	while (index != NULL) {
      		/* fprintf(stderr, "Validating index %s\n", index->name); */
	
		if (!btr_validate_tree(index->tree)) {
			ret = DB_ERROR;
		} else {
			if (!row_scan_and_check_index(prebuilt,
							index, &n_rows)) {
				ret = DB_ERROR;
			}

			/* fprintf(stderr, "%lu entries in index %s\n", n_rows,
			  index->name); */

			if (index == dict_table_get_first_index(table)) {
				n_rows_in_table = n_rows;
			} else if (n_rows != n_rows_in_table) {

				ret = DB_ERROR;
 
				fprintf(stderr,
		"Error: index %s contains %lu entries, should be %lu\n",
					index->name, n_rows, n_rows_in_table);
			}
		}

		index = dict_table_get_next_index(index);
	}

	/* Restore the original isolation level */
	prebuilt->trx->isolation_level = old_isolation_level;
	
	/* We validate also the whole adaptive hash index for all tables
	at every CHECK TABLE */

	if (!btr_search_validate()) {

		ret = DB_ERROR;
	}

	prebuilt->trx->op_info = (char *) "";

	return(ret);
}

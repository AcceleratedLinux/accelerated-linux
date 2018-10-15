/******************************************************
The database buffer replacement algorithm

(c) 1995 Innobase Oy

Created 11/5/1995 Heikki Tuuri
*******************************************************/

#include "buf0lru.h"

#ifdef UNIV_NONINL
#include "buf0lru.ic"
#include "srv0srv.h"	/* Needed to getsrv_print_innodb_monitor */
#endif

#include "ut0byte.h"
#include "ut0lst.h"
#include "ut0rnd.h"
#include "sync0sync.h"
#include "sync0rw.h"
#include "hash0hash.h"
#include "os0sync.h"
#include "fil0fil.h"
#include "btr0btr.h"
#include "buf0buf.h"
#include "buf0flu.h"
#include "buf0rea.h"
#include "btr0sea.h"
#include "os0file.h"
#include "log0recv.h"

/* The number of blocks from the LRU_old pointer onward, including the block
pointed to, must be 3/8 of the whole LRU list length, except that the
tolerance defined below is allowed. Note that the tolerance must be small
enough such that for even the BUF_LRU_OLD_MIN_LEN long LRU list, the
LRU_old pointer is not allowed to point to either end of the LRU list. */

#define BUF_LRU_OLD_TOLERANCE	20

/* The whole LRU list length is divided by this number to determine an
initial segment in buf_LRU_get_recent_limit */

#define BUF_LRU_INITIAL_RATIO	8

/**********************************************************************
Takes a block out of the LRU list and page hash table and sets the block
state to BUF_BLOCK_REMOVE_HASH. */
static
void
buf_LRU_block_remove_hashed_page(
/*=============================*/
	buf_block_t*	block);	/* in: block, must contain a file page and
				be in a state where it can be freed; there
				may or may not be a hash index to the page */
/**********************************************************************
Puts a file page whose has no hash index to the free list. */
static
void
buf_LRU_block_free_hashed_page(
/*===========================*/
	buf_block_t*	block);	/* in: block, must contain a file page and
				be in a state where it can be freed */

/**********************************************************************
Gets the minimum LRU_position field for the blocks in an initial segment
(determined by BUF_LRU_INITIAL_RATIO) of the LRU list. The limit is not
guaranteed to be precise, because the ulint_clock may wrap around. */

ulint
buf_LRU_get_recent_limit(void)
/*==========================*/
			/* out: the limit; zero if could not determine it */
{
	buf_block_t*	block;
	ulint		len;
	ulint		limit;

	mutex_enter(&(buf_pool->mutex));

	len = UT_LIST_GET_LEN(buf_pool->LRU);

	if (len < BUF_LRU_OLD_MIN_LEN) {
		/* The LRU list is too short to do read-ahead */

		mutex_exit(&(buf_pool->mutex));

		return(0);
	}

	block = UT_LIST_GET_FIRST(buf_pool->LRU);

	limit = block->LRU_position - len / BUF_LRU_INITIAL_RATIO;

	mutex_exit(&(buf_pool->mutex));

	return(limit);
}

/**********************************************************************
Look for a replaceable block from the end of the LRU list and put it to
the free list if found. */

ibool
buf_LRU_search_and_free_block(
/*==========================*/
				/* out: TRUE if freed */
	ulint	n_iterations)   /* in: how many times this has been called
				repeatedly without result: a high value means
				that we should search farther; if value is
				k < 10, then we only search k/10 * [number
				of pages in the buffer pool] from the end
				of the LRU list */
{
	buf_block_t*	block;
	ulint		distance = 0;
	ibool		freed;

	mutex_enter(&(buf_pool->mutex));
	
	freed = FALSE;
	
	block = UT_LIST_GET_LAST(buf_pool->LRU);

	while (block != NULL) {

		if (buf_flush_ready_for_replace(block)) {

			if (buf_debug_prints) {
				printf(
				"Putting space %lu page %lu to free list\n",
					block->space, block->offset);
			}
			
			buf_LRU_block_remove_hashed_page(block);

			mutex_exit(&(buf_pool->mutex));

			btr_search_drop_page_hash_index(block->frame);

			mutex_enter(&(buf_pool->mutex));

			ut_a(block->buf_fix_count == 0);

			buf_LRU_block_free_hashed_page(block);

			freed = TRUE;

			break;
		}

		block = UT_LIST_GET_PREV(LRU, block);
		distance++;

		if (!freed && n_iterations <= 10
		    && distance > 100 + (n_iterations * buf_pool->curr_size)
					/ 10) {

			buf_pool->LRU_flush_ended = 0;

			mutex_exit(&(buf_pool->mutex));
			
			return(FALSE);
		}
	}

	if (buf_pool->LRU_flush_ended > 0) {
		buf_pool->LRU_flush_ended--;
	}
 
	if (!freed) {
		buf_pool->LRU_flush_ended = 0;
	}

	mutex_exit(&(buf_pool->mutex));
	
	return(freed);
}
	
/**********************************************************************
Tries to remove LRU flushed blocks from the end of the LRU list and put them
to the free list. This is beneficial for the efficiency of the insert buffer
operation, as flushed pages from non-unique non-clustered indexes are here
taken out of the buffer pool, and their inserts redirected to the insert
buffer. Otherwise, the flushed blocks could get modified again before read
operations need new buffer blocks, and the i/o work done in flushing would be
wasted. */

void
buf_LRU_try_free_flushed_blocks(void)
/*=================================*/
{
	mutex_enter(&(buf_pool->mutex));

	while (buf_pool->LRU_flush_ended > 0) {

		mutex_exit(&(buf_pool->mutex));

		buf_LRU_search_and_free_block(1);
		
		mutex_enter(&(buf_pool->mutex));
	}

	mutex_exit(&(buf_pool->mutex));
}	

/**********************************************************************
Returns a free block from buf_pool. The block is taken off the free list.
If it is empty, blocks are moved from the end of the LRU list to the free
list. */

buf_block_t*
buf_LRU_get_free_block(void)
/*========================*/
				/* out: the free control block */
{
	buf_block_t*	block		= NULL;
	ibool		freed;
	ulint		n_iterations	= 1;
	ibool		mon_value_was   = 0; /* remove bug */
	ibool		started_monitor	= FALSE;
loop:
	mutex_enter(&(buf_pool->mutex));

	if (!recv_recovery_on && UT_LIST_GET_LEN(buf_pool->free)
	   + UT_LIST_GET_LEN(buf_pool->LRU) < buf_pool->max_size / 10) {
	   	ut_print_timestamp(stderr);

	   	fprintf(stderr,
"  InnoDB: ERROR: over 9 / 10 of the buffer pool is occupied by\n"
"InnoDB: lock heaps or the adaptive hash index!\n"
"InnoDB: We intentionally generate a seg fault to print a stack trace\n"
"InnoDB: on Linux!\n");

		ut_a(0);
	   
	} else if (!recv_recovery_on && UT_LIST_GET_LEN(buf_pool->free)
	   + UT_LIST_GET_LEN(buf_pool->LRU) < buf_pool->max_size / 5) {

	   	/* Over 80 % of the buffer pool is occupied by lock heaps
	   	or the adaptive hash index. This may be a memory leak! */

	   	ut_print_timestamp(stderr);
	   	fprintf(stderr,
"  InnoDB: WARNING: over 4 / 5 of the buffer pool is occupied by\n"
"InnoDB: lock heaps or the adaptive hash index! Check that your\n"
"InnoDB: transactions do not set too many row locks.\n"
"InnoDB: Your buffer pool size is %lu MB. Maybe you should make\n"
"InnoDB: the buffer pool bigger?\n"
"InnoDB: Starting the InnoDB Monitor to print diagnostics, including\n"
"InnoDB: lock heap and hash index sizes.\n",
		buf_pool->curr_size / (1024 * 1024 / UNIV_PAGE_SIZE));

		srv_print_innodb_monitor = TRUE;

	} else if (!recv_recovery_on && UT_LIST_GET_LEN(buf_pool->free)
	   + UT_LIST_GET_LEN(buf_pool->LRU) < buf_pool->max_size / 4) {

		/* Switch off the InnoDB Monitor; this is a simple way
		to stop the monitor if the situation becomes less urgent,
		but may also surprise users! */

		srv_print_innodb_monitor = FALSE;
	}
	
	/* If there is a block in the free list, take it */
	if (UT_LIST_GET_LEN(buf_pool->free) > 0) {
		
		block = UT_LIST_GET_FIRST(buf_pool->free);
		UT_LIST_REMOVE(free, buf_pool->free, block);
		block->state = BUF_BLOCK_READY_FOR_USE;

		mutex_exit(&(buf_pool->mutex));

		if (started_monitor) {
			srv_print_innodb_monitor = mon_value_was;
		}	

		return(block);
	}
	
	/* If no block was in the free list, search from the end of the LRU
	list and try to free a block there */

	mutex_exit(&(buf_pool->mutex));

	freed = buf_LRU_search_and_free_block(n_iterations);

	if (freed > 0) {
		goto loop;
	}

	if (n_iterations > 30) {
		ut_print_timestamp(stderr);
		fprintf(stderr,
		"InnoDB: Warning: difficult to find free blocks from\n"
		"InnoDB: the buffer pool (%lu search iterations)! Consider\n"
		"InnoDB: increasing the buffer pool size.\n",
						n_iterations);
		fprintf(stderr,
		"InnoDB: It is also possible that in your Unix version\n"
		"InnoDB: fsync is very slow, or completely frozen inside\n"
		"InnoDB: the OS kernel. Then upgrading to a newer version\n"
		"InnoDB: of your operating system may help. Look at the\n"
		"InnoDB: number of fsyncs in diagnostic info below.\n");

		fprintf(stderr,
		"InnoDB: Pending flushes (fsync) log: %lu; buffer pool: %lu\n",
	       			fil_n_pending_log_flushes,
				fil_n_pending_tablespace_flushes);
		fprintf(stderr,
	"InnoDB: %lu OS file reads, %lu OS file writes, %lu OS fsyncs\n",
			os_n_file_reads, os_n_file_writes, os_n_fsyncs);

		fprintf(stderr,
		"InnoDB: Starting InnoDB Monitor to print further\n"
		"InnoDB: diagnostics to the standard output.\n");

		mon_value_was = srv_print_innodb_monitor;
		started_monitor = TRUE;
		srv_print_innodb_monitor = TRUE;
	}

	/* No free block was found: try to flush the LRU list */

	buf_flush_free_margin();

	os_aio_simulated_wake_handler_threads();

	mutex_enter(&(buf_pool->mutex));

	if (buf_pool->LRU_flush_ended > 0) {
		/* We have written pages in an LRU flush. To make the insert
		buffer more efficient, we try to move these pages to the free
		list. */

		mutex_exit(&(buf_pool->mutex));

		buf_LRU_try_free_flushed_blocks();
	} else {
		mutex_exit(&(buf_pool->mutex));
	}

	if (n_iterations > 10) {

		os_thread_sleep(500000);
	}

	n_iterations++;

	goto loop;	
}	

/***********************************************************************
Moves the LRU_old pointer so that the length of the old blocks list
is inside the allowed limits. */
UNIV_INLINE
void
buf_LRU_old_adjust_len(void)
/*========================*/
{
	ulint	old_len;
	ulint	new_len;

	ut_ad(buf_pool->LRU_old);
	ut_ad(mutex_own(&(buf_pool->mutex)));
	ut_ad(3 * (BUF_LRU_OLD_MIN_LEN / 8) > BUF_LRU_OLD_TOLERANCE + 5);

	for (;;) {
		old_len = buf_pool->LRU_old_len;
		new_len = 3 * (UT_LIST_GET_LEN(buf_pool->LRU) / 8);

		/* Update the LRU_old pointer if necessary */
	
		if (old_len < new_len - BUF_LRU_OLD_TOLERANCE) {
		
			buf_pool->LRU_old = UT_LIST_GET_PREV(LRU,
							buf_pool->LRU_old);
			(buf_pool->LRU_old)->old = TRUE;
			buf_pool->LRU_old_len++;

		} else if (old_len > new_len + BUF_LRU_OLD_TOLERANCE) {

			(buf_pool->LRU_old)->old = FALSE;
			buf_pool->LRU_old = UT_LIST_GET_NEXT(LRU,
							buf_pool->LRU_old);
			buf_pool->LRU_old_len--;
		} else {
			ut_ad(buf_pool->LRU_old); /* Check that we did not
						fall out of the LRU list */
			return;
		}
	}
}

/***********************************************************************
Initializes the old blocks pointer in the LRU list.
This function should be called when the LRU list grows to
BUF_LRU_OLD_MIN_LEN length. */
static
void
buf_LRU_old_init(void)
/*==================*/
{
	buf_block_t*	block;

	ut_ad(UT_LIST_GET_LEN(buf_pool->LRU) == BUF_LRU_OLD_MIN_LEN);

	/* We first initialize all blocks in the LRU list as old and then use
	the adjust function to move the LRU_old pointer to the right
	position */

	block = UT_LIST_GET_FIRST(buf_pool->LRU);

	while (block != NULL) {
		block->old = TRUE;
		block = UT_LIST_GET_NEXT(LRU, block);
	}

	buf_pool->LRU_old = UT_LIST_GET_FIRST(buf_pool->LRU);
	buf_pool->LRU_old_len = UT_LIST_GET_LEN(buf_pool->LRU);
	
	buf_LRU_old_adjust_len();
}	    	

/**********************************************************************
Removes a block from the LRU list. */
UNIV_INLINE
void
buf_LRU_remove_block(
/*=================*/
	buf_block_t*	block)	/* in: control block */
{
	ut_ad(buf_pool);
	ut_ad(block);
	ut_ad(mutex_own(&(buf_pool->mutex)));
		
	/* If the LRU_old pointer is defined and points to just this block,
	move it backward one step */

	if (block == buf_pool->LRU_old) {

		/* Below: the previous block is guaranteed to exist, because
		the LRU_old pointer is only allowed to differ by the
		tolerance value from strict 3/8 of the LRU list length. */

		buf_pool->LRU_old = UT_LIST_GET_PREV(LRU, block);
		(buf_pool->LRU_old)->old = TRUE;

		buf_pool->LRU_old_len++;
		ut_ad(buf_pool->LRU_old);
	}

	/* Remove the block from the LRU list */
	UT_LIST_REMOVE(LRU, buf_pool->LRU, block);

	/* If the LRU list is so short that LRU_old not defined, return */
	if (UT_LIST_GET_LEN(buf_pool->LRU) < BUF_LRU_OLD_MIN_LEN) {

		buf_pool->LRU_old = NULL;

		return;
	}

	ut_ad(buf_pool->LRU_old);	

	/* Update the LRU_old_len field if necessary */
	if (block->old) {

		buf_pool->LRU_old_len--;
	}

	/* Adjust the length of the old block list if necessary */
	buf_LRU_old_adjust_len();
}	    	

/**********************************************************************
Adds a block to the LRU list end. */
UNIV_INLINE
void
buf_LRU_add_block_to_end_low(
/*=========================*/
	buf_block_t*	block)	/* in: control block */
{
	buf_block_t*	last_block;
	
	ut_ad(buf_pool);
	ut_ad(block);
	ut_ad(mutex_own(&(buf_pool->mutex)));

	block->old = TRUE;

	last_block = UT_LIST_GET_LAST(buf_pool->LRU);

	if (last_block) {
		block->LRU_position = last_block->LRU_position;
	} else {
		block->LRU_position = buf_pool_clock_tic();
	}			

	UT_LIST_ADD_LAST(LRU, buf_pool->LRU, block);

	if (UT_LIST_GET_LEN(buf_pool->LRU) >= BUF_LRU_OLD_MIN_LEN) {

		buf_pool->LRU_old_len++;
	}

	if (UT_LIST_GET_LEN(buf_pool->LRU) > BUF_LRU_OLD_MIN_LEN) {

		ut_ad(buf_pool->LRU_old);

		/* Adjust the length of the old block list if necessary */

		buf_LRU_old_adjust_len();

	} else if (UT_LIST_GET_LEN(buf_pool->LRU) == BUF_LRU_OLD_MIN_LEN) {

		/* The LRU list is now long enough for LRU_old to become
		defined: init it */

		buf_LRU_old_init();
	}
}	    	

/**********************************************************************
Adds a block to the LRU list. */
UNIV_INLINE
void
buf_LRU_add_block_low(
/*==================*/
	buf_block_t*	block,	/* in: control block */
	ibool		old)	/* in: TRUE if should be put to the old blocks
				in the LRU list, else put to the start; if the
				LRU list is very short, the block is added to
				the start, regardless of this parameter */
{
	ulint	cl;
	
	ut_ad(buf_pool);
	ut_ad(block);
	ut_ad(mutex_own(&(buf_pool->mutex)));

	block->old = old;
	cl = buf_pool_clock_tic();

	if (!old || (UT_LIST_GET_LEN(buf_pool->LRU) < BUF_LRU_OLD_MIN_LEN)) {

		UT_LIST_ADD_FIRST(LRU, buf_pool->LRU, block);

		block->LRU_position = cl;		
		block->freed_page_clock = buf_pool->freed_page_clock;
	} else {
		UT_LIST_INSERT_AFTER(LRU, buf_pool->LRU, buf_pool->LRU_old,
								block);
		buf_pool->LRU_old_len++;

		/* We copy the LRU position field of the previous block
		to the new block */

		block->LRU_position = (buf_pool->LRU_old)->LRU_position;
	}

	if (UT_LIST_GET_LEN(buf_pool->LRU) > BUF_LRU_OLD_MIN_LEN) {

		ut_ad(buf_pool->LRU_old);

		/* Adjust the length of the old block list if necessary */

		buf_LRU_old_adjust_len();

	} else if (UT_LIST_GET_LEN(buf_pool->LRU) == BUF_LRU_OLD_MIN_LEN) {

		/* The LRU list is now long enough for LRU_old to become
		defined: init it */

		buf_LRU_old_init();
	}	
}	    	

/**********************************************************************
Adds a block to the LRU list. */

void
buf_LRU_add_block(
/*==============*/
	buf_block_t*	block,	/* in: control block */
	ibool		old)	/* in: TRUE if should be put to the old
				blocks in the LRU list, else put to the start;
				if the LRU list is very short, the block is
				added to the start, regardless of this
				parameter */
{
	buf_LRU_add_block_low(block, old);
}

/**********************************************************************
Moves a block to the start of the LRU list. */

void
buf_LRU_make_block_young(
/*=====================*/
	buf_block_t*	block)	/* in: control block */
{
	buf_LRU_remove_block(block);
	buf_LRU_add_block_low(block, FALSE);
}

/**********************************************************************
Moves a block to the end of the LRU list. */

void
buf_LRU_make_block_old(
/*===================*/
	buf_block_t*	block)	/* in: control block */
{
	buf_LRU_remove_block(block);
	buf_LRU_add_block_to_end_low(block);
}

/**********************************************************************
Puts a block back to the free list. */

void
buf_LRU_block_free_non_file_page(
/*=============================*/
	buf_block_t*	block)	/* in: block, must not contain a file page */
{
	ut_ad(mutex_own(&(buf_pool->mutex)));
	ut_ad(block);
	
	ut_ad((block->state == BUF_BLOCK_MEMORY)
	      || (block->state == BUF_BLOCK_READY_FOR_USE));

	block->state = BUF_BLOCK_NOT_USED;

#ifdef UNIV_DEBUG	
	/* Wipe contents of page to reveal possible stale pointers to it */
	memset(block->frame, '\0', UNIV_PAGE_SIZE);
#endif	
	UT_LIST_ADD_FIRST(free, buf_pool->free, block);
}

/**********************************************************************
Takes a block out of the LRU list and page hash table and sets the block
state to BUF_BLOCK_REMOVE_HASH. */
static
void
buf_LRU_block_remove_hashed_page(
/*=============================*/
	buf_block_t*	block)	/* in: block, must contain a file page and
				be in a state where it can be freed; there
				may or may not be a hash index to the page */
{
	ut_ad(mutex_own(&(buf_pool->mutex)));
	ut_ad(block);
	
	ut_ad(block->state == BUF_BLOCK_FILE_PAGE);

	ut_a(block->io_fix == 0);
	ut_a(block->buf_fix_count == 0);
	ut_a(ut_dulint_cmp(block->oldest_modification, ut_dulint_zero) == 0);

	buf_LRU_remove_block(block);

	buf_pool->freed_page_clock += 1;

 	buf_frame_modify_clock_inc(block->frame);
		
	HASH_DELETE(buf_block_t, hash, buf_pool->page_hash,
			buf_page_address_fold(block->space, block->offset),
			block);

	block->state = BUF_BLOCK_REMOVE_HASH;
}

/**********************************************************************
Puts a file page whose has no hash index to the free list. */
static
void
buf_LRU_block_free_hashed_page(
/*===========================*/
	buf_block_t*	block)	/* in: block, must contain a file page and
				be in a state where it can be freed */
{
	ut_ad(mutex_own(&(buf_pool->mutex)));
	ut_ad(block->state == BUF_BLOCK_REMOVE_HASH);

	block->state = BUF_BLOCK_MEMORY;

	buf_LRU_block_free_non_file_page(block);
}
				
/**************************************************************************
Validates the LRU list. */

ibool
buf_LRU_validate(void)
/*==================*/
{
	buf_block_t*	block;
	ulint		old_len;
	ulint		new_len;
	ulint		LRU_pos;
	
	ut_ad(buf_pool);
	mutex_enter(&(buf_pool->mutex));

	if (UT_LIST_GET_LEN(buf_pool->LRU) >= BUF_LRU_OLD_MIN_LEN) {

		ut_a(buf_pool->LRU_old);
		old_len = buf_pool->LRU_old_len;
		new_len = 3 * (UT_LIST_GET_LEN(buf_pool->LRU) / 8);
		ut_a(old_len >= new_len - BUF_LRU_OLD_TOLERANCE);
		ut_a(old_len <= new_len + BUF_LRU_OLD_TOLERANCE);
	}
		
	UT_LIST_VALIDATE(LRU, buf_block_t, buf_pool->LRU);

	block = UT_LIST_GET_FIRST(buf_pool->LRU);

	old_len = 0;

	while (block != NULL) {

		ut_a(block->state == BUF_BLOCK_FILE_PAGE);

		if (block->old) {
			old_len++;
		}

		if (buf_pool->LRU_old && (old_len == 1)) {
			ut_a(buf_pool->LRU_old == block);
		}

		LRU_pos	= block->LRU_position;

		block = UT_LIST_GET_NEXT(LRU, block);

		if (block) {
			/* If the following assert fails, it may
			not be an error: just the buf_pool clock
			has wrapped around */
			ut_a(LRU_pos >= block->LRU_position);
		}
	}

	if (buf_pool->LRU_old) {
		ut_a(buf_pool->LRU_old_len == old_len);
	} 

	UT_LIST_VALIDATE(free, buf_block_t, buf_pool->free);

	block = UT_LIST_GET_FIRST(buf_pool->free);

	while (block != NULL) {
		ut_a(block->state == BUF_BLOCK_NOT_USED);

		block = UT_LIST_GET_NEXT(free, block);
	}

	mutex_exit(&(buf_pool->mutex));
	return(TRUE);
}

/**************************************************************************
Prints the LRU list. */

void
buf_LRU_print(void)
/*===============*/
{
	buf_block_t*	block;
	buf_frame_t*	frame;
	ulint		len;
	
	ut_ad(buf_pool);
	mutex_enter(&(buf_pool->mutex));

	printf("Pool ulint clock %lu\n", buf_pool->ulint_clock);

	block = UT_LIST_GET_FIRST(buf_pool->LRU);

	len = 0;

	while (block != NULL) {

		printf("BLOCK %lu ", block->offset);

		if (block->old) {
			printf("old ");
		}

		if (block->buf_fix_count) {
			printf("buffix count %lu ", block->buf_fix_count);
		}

		if (block->io_fix) {
			printf("io_fix %lu ", block->io_fix);
		}

		if (ut_dulint_cmp(block->oldest_modification,
				ut_dulint_zero) > 0) {
			printf("modif. ");
		}

		printf("LRU pos %lu ", block->LRU_position);
		
		frame = buf_block_get_frame(block);

		printf("type %lu ", fil_page_get_type(frame));
		printf("index id %lu ", ut_dulint_get_low(
					btr_page_get_index_id(frame)));

		block = UT_LIST_GET_NEXT(LRU, block);
		len++;
		if (len % 10 == 0) {
			printf("\n");
		}
	}

	mutex_exit(&(buf_pool->mutex));
}

/******************************************************
The index tree persistent cursor

(c) 1996 Innobase Oy

Created 2/23/1996 Heikki Tuuri
*******************************************************/

#include "btr0pcur.h"

#ifdef UNIV_NONINL
#include "btr0pcur.ic"
#endif

#include "ut0byte.h"
#include "rem0cmp.h"

/******************************************************************
Allocates memory for a persistent cursor object and initializes the cursor. */

btr_pcur_t*
btr_pcur_create_for_mysql(void)
/*============================*/
				/* out, own: persistent cursor */
{
	btr_pcur_t*	pcur;

	pcur = mem_alloc(sizeof(btr_pcur_t));

	pcur->btr_cur.index = NULL;
	btr_pcur_init(pcur);
	
	return(pcur);
}

/******************************************************************
Frees the memory for a persistent cursor object. */

void
btr_pcur_free_for_mysql(
/*====================*/
	btr_pcur_t*	cursor)	/* in, own: persistent cursor */
{
	if (cursor->old_rec_buf != NULL) {

		mem_free(cursor->old_rec_buf);

		cursor->old_rec = NULL;
		cursor->old_rec_buf = NULL;
	}

	cursor->btr_cur.page_cur.rec = NULL;
	cursor->old_rec = NULL;
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	
	cursor->latch_mode = BTR_NO_LATCHES;
	cursor->pos_state = BTR_PCUR_NOT_POSITIONED;

	mem_free(cursor);
}

/******************************************************************
The position of the cursor is stored by taking an initial segment of the
record the cursor is positioned on, before, or after, and copying it to the
cursor data structure, or just setting a flag if the cursor id before the
first in an EMPTY tree, or after the last in an EMPTY tree. NOTE that the
page where the cursor is positioned must not be empty if the index tree is
not totally empty! */

void
btr_pcur_store_position(
/*====================*/
	btr_pcur_t*	cursor, /* in: persistent cursor */
	mtr_t*		mtr)	/* in: mtr */
{
	page_cur_t*	page_cursor;
	rec_t*		rec;
	dict_tree_t*	tree;
	page_t*		page;
	
	ut_a(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);

	tree = btr_cur_get_tree(btr_pcur_get_btr_cur(cursor));

	page_cursor = btr_pcur_get_page_cur(cursor);

	rec = page_cur_get_rec(page_cursor);
	page = buf_frame_align(rec);

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_S_FIX)
	      || mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	ut_a(cursor->latch_mode != BTR_NO_LATCHES);

	if (page_get_n_recs(page) == 0) {
		/* It must be an empty index tree */

		ut_a(btr_page_get_next(page, mtr) == FIL_NULL
		     && btr_page_get_prev(page, mtr) == FIL_NULL);

		if (rec == page_get_supremum_rec(page)) {

			cursor->rel_pos = BTR_PCUR_AFTER_LAST_IN_TREE;
			cursor->old_stored = BTR_PCUR_OLD_STORED;

			return;
		}

		cursor->rel_pos = BTR_PCUR_BEFORE_FIRST_IN_TREE;
		cursor->old_stored = BTR_PCUR_OLD_STORED;

		return;
	} 

	if (rec == page_get_supremum_rec(page)) {

		rec = page_rec_get_prev(rec);

		cursor->rel_pos = BTR_PCUR_AFTER;

	} else if (rec == page_get_infimum_rec(page)) {

		rec = page_rec_get_next(rec);

		cursor->rel_pos = BTR_PCUR_BEFORE;
	} else {
		cursor->rel_pos = BTR_PCUR_ON;
	}
	
	cursor->old_stored = BTR_PCUR_OLD_STORED;
	cursor->old_rec = dict_tree_copy_rec_order_prefix(tree, rec,
						&(cursor->old_rec_buf),
						&(cursor->buf_size));
									
	cursor->modify_clock = buf_frame_get_modify_clock(page);
}

/******************************************************************
Copies the stored position of a pcur to another pcur. */

void
btr_pcur_copy_stored_position(
/*==========================*/
	btr_pcur_t*	pcur_receive,	/* in: pcur which will receive the
					position info */
	btr_pcur_t*	pcur_donate)	/* in: pcur from which the info is
					copied */
{
	if (pcur_receive->old_rec_buf) {
		mem_free(pcur_receive->old_rec_buf);
	}

	ut_memcpy((byte*)pcur_receive, (byte*)pcur_donate, sizeof(btr_pcur_t));

	if (pcur_donate->old_rec_buf) {

		pcur_receive->old_rec_buf = mem_alloc(pcur_donate->buf_size);
	
		ut_memcpy(pcur_receive->old_rec_buf, pcur_donate->old_rec_buf,
						pcur_donate->buf_size);
		pcur_receive->old_rec = pcur_receive->old_rec_buf
			+ (pcur_donate->old_rec - pcur_donate->old_rec_buf);
	}	
}

/******************************************************************
Restores the stored position of a persistent cursor bufferfixing the page and
obtaining the specified latches. If the cursor position was saved when the
(1) cursor was positioned on a user record: this function restores the position
to the last record LESS OR EQUAL to the stored record;
(2) cursor was positioned on a page infimum record: restores the position to
the last record LESS than the user record which was the successor of the page
infimum;
(3) cursor was positioned on the page supremum: restores to the first record
GREATER than the user record which was the predecessor of the supremum.
(4) cursor was positioned before the first or after the last in an empty tree:
restores to before first or after the last in the tree. */

ibool
btr_pcur_restore_position(
/*======================*/
					/* out: TRUE if the cursor position
					was stored when it was on a user record
					and it can be restored on a user record
					whose ordering fields are identical to
					the ones of the original user record */
	ulint		latch_mode,	/* in: BTR_SEARCH_LEAF, ... */
	btr_pcur_t*	cursor, 	/* in: detached persistent cursor */
	mtr_t*		mtr)		/* in: mtr */
{
	dict_tree_t*	tree;
	page_t*		page;
	dtuple_t*	tuple;
	ulint		mode;
	ulint		old_mode;
	ibool		from_left;
	mem_heap_t*	heap;

	ut_a(cursor->pos_state == BTR_PCUR_WAS_POSITIONED
			|| cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_a(cursor->old_stored == BTR_PCUR_OLD_STORED);

	if (cursor->rel_pos == BTR_PCUR_AFTER_LAST_IN_TREE
	    || cursor->rel_pos == BTR_PCUR_BEFORE_FIRST_IN_TREE) {

	    	if (cursor->rel_pos == BTR_PCUR_BEFORE_FIRST_IN_TREE) {
	    		from_left = TRUE;
	    	} else {
	    		from_left = FALSE;
	    	}

		btr_cur_open_at_index_side(from_left,
			btr_pcur_get_btr_cur(cursor)->index, latch_mode,
					btr_pcur_get_btr_cur(cursor), mtr);
		return(FALSE);
	}
	
	ut_a(cursor->old_rec);

	page = btr_cur_get_page(btr_pcur_get_btr_cur(cursor));

	if (latch_mode == BTR_SEARCH_LEAF || latch_mode == BTR_MODIFY_LEAF) {
		/* Try optimistic restoration */
	    
		if (buf_page_optimistic_get(latch_mode, page,
						cursor->modify_clock, mtr)) {
			cursor->pos_state = BTR_PCUR_IS_POSITIONED;

			buf_page_dbg_add_level(page, SYNC_TREE_NODE);
			
			if (cursor->rel_pos == BTR_PCUR_ON) {

				cursor->latch_mode = latch_mode;

				ut_ad(cmp_rec_rec(cursor->old_rec,
					btr_pcur_get_rec(cursor),
					dict_tree_find_index(
					    btr_cur_get_tree(
						btr_pcur_get_btr_cur(cursor)),
					    btr_pcur_get_rec(cursor)))
					== 0); 

				return(TRUE);
			}

			return(FALSE);
		}
	}

	/* If optimistic restoration did not succeed, open the cursor anew */

	heap = mem_heap_create(256);
	
	tree = btr_cur_get_tree(btr_pcur_get_btr_cur(cursor));
	tuple = dict_tree_build_data_tuple(tree, cursor->old_rec, heap);

	/* Save the old search mode of the cursor */
	old_mode = cursor->search_mode;
	
	if (cursor->rel_pos == BTR_PCUR_ON) {
		mode = PAGE_CUR_LE;
	} else if (cursor->rel_pos == BTR_PCUR_AFTER) {
		mode = PAGE_CUR_G;
	} else {
		ut_ad(cursor->rel_pos == BTR_PCUR_BEFORE);
		mode = PAGE_CUR_L;
	}

	btr_pcur_open_with_no_init(btr_pcur_get_btr_cur(cursor)->index, tuple,
					mode, latch_mode, cursor, 0, mtr);

	cursor->old_stored = BTR_PCUR_OLD_STORED;
	
	/* Restore the old search mode */
	cursor->search_mode = old_mode;

	if (cursor->rel_pos == BTR_PCUR_ON
	    && btr_pcur_is_on_user_rec(cursor, mtr)
	    && 0 == cmp_dtuple_rec(tuple, btr_pcur_get_rec(cursor))) {

	        /* We have to store the NEW value for the modify clock, since
	        the cursor can now be on a different page! */

	        cursor->modify_clock = buf_frame_get_modify_clock(
				    buf_frame_align(btr_pcur_get_rec(cursor)));
		mem_heap_free(heap);

		return(TRUE);
	}

	mem_heap_free(heap);

	/* We have to store position information, modify clock value, etc.
        because the cursor may now be on a different page */

	btr_pcur_store_position(cursor, mtr);

	return(FALSE);
}

/******************************************************************
If the latch mode of the cursor is BTR_LEAF_SEARCH or BTR_LEAF_MODIFY,
releases the page latch and bufferfix reserved by the cursor.
NOTE! In the case of BTR_LEAF_MODIFY, there should not exist changes
made by the current mini-transaction to the data protected by the
cursor latch, as then the latch must not be released until mtr_commit. */

void
btr_pcur_release_leaf(
/*==================*/
	btr_pcur_t*	cursor, /* in: persistent cursor */
	mtr_t*		mtr)	/* in: mtr */
{
	page_t*	page;

	ut_a(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	
	page = btr_cur_get_page(btr_pcur_get_btr_cur(cursor));
	
	btr_leaf_page_release(page, cursor->latch_mode, mtr);
	
	cursor->latch_mode = BTR_NO_LATCHES;	

	cursor->pos_state = BTR_PCUR_WAS_POSITIONED;
}

/*************************************************************
Moves the persistent cursor to the first record on the next page. Releases the
latch on the current page, and bufferunfixes it. Note that there must not be
modifications on the current page, as then the x-latch can be released only in
mtr_commit. */

void
btr_pcur_move_to_next_page(
/*=======================*/
	btr_pcur_t*	cursor,	/* in: persistent cursor; must be on the
				last record of the current page */
	mtr_t*		mtr)	/* in: mtr */
{
	ulint	next_page_no;
	ulint	space;
	page_t*	page;
	page_t*	next_page;

	ut_a(cursor->pos_state == BTR_PCUR_IS_POSITIONED);	
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	ut_ad(btr_pcur_is_after_last_on_page(cursor, mtr));	

	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	
	page = btr_pcur_get_page(cursor);

	next_page_no = btr_page_get_next(page, mtr);
	space = buf_frame_get_space_id(page);

	ut_ad(next_page_no != FIL_NULL);	

	next_page = btr_page_get(space, next_page_no, cursor->latch_mode, mtr);
	buf_block_align(next_page)->check_index_page_at_flush = TRUE;

	btr_leaf_page_release(page, cursor->latch_mode, mtr);
	
	page_cur_set_before_first(next_page, btr_pcur_get_page_cur(cursor));

	page_check_dir(next_page);
}

/*************************************************************
Moves the persistent cursor backward if it is on the first record of the page.
Commits mtr. Note that to prevent a possible deadlock, the operation
first stores the position of the cursor, commits mtr, acquires the necessary
latches and restores the cursor position again before returning. The
alphabetical position of the cursor is guaranteed to be sensible on
return, but it may happen that the cursor is not positioned on the last
record of any page, because the structure of the tree may have changed
during the time when the cursor had no latches. */

void
btr_pcur_move_backward_from_page(
/*=============================*/
	btr_pcur_t*	cursor,	/* in: persistent cursor, must be on the first
				record of the current page */
	mtr_t*		mtr)	/* in: mtr */
{
	ulint	prev_page_no;
	ulint	space;
	page_t*	page;
	page_t*	prev_page;
	ulint	latch_mode;
	ulint	latch_mode2;

	ut_a(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	ut_ad(btr_pcur_is_before_first_on_page(cursor, mtr));	
	ut_ad(!btr_pcur_is_before_first_in_tree(cursor, mtr));	
	
	latch_mode = cursor->latch_mode;
	
	if (latch_mode == BTR_SEARCH_LEAF) {

		latch_mode2 = BTR_SEARCH_PREV;

	} else if (latch_mode == BTR_MODIFY_LEAF) {

		latch_mode2 = BTR_MODIFY_PREV;
	} else {
		latch_mode2 = 0; /* To eliminate compiler warning */
		ut_error;
	}

	btr_pcur_store_position(cursor, mtr);

	mtr_commit(mtr);

	mtr_start(mtr);

	btr_pcur_restore_position(latch_mode2, cursor, mtr);	

	page = btr_pcur_get_page(cursor);

	prev_page_no = btr_page_get_prev(page, mtr);
	space = buf_frame_get_space_id(page);

	if (btr_pcur_is_before_first_on_page(cursor, mtr)
					&& (prev_page_no != FIL_NULL)) {	

		prev_page = btr_pcur_get_btr_cur(cursor)->left_page;

		btr_leaf_page_release(page, latch_mode, mtr);

		page_cur_set_after_last(prev_page,
						btr_pcur_get_page_cur(cursor));
	} else if (prev_page_no != FIL_NULL) {
		
		/* The repositioned cursor did not end on an infimum record on
		a page. Cursor repositioning acquired a latch also on the
		previous page, but we do not need the latch: release it. */
	
		prev_page = btr_pcur_get_btr_cur(cursor)->left_page;

		btr_leaf_page_release(prev_page, latch_mode, mtr);
	}

	cursor->latch_mode = latch_mode;

	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
}

/*************************************************************
Moves the persistent cursor to the previous record in the tree. If no records
are left, the cursor stays 'before first in tree'. */

ibool
btr_pcur_move_to_prev(
/*==================*/
				/* out: TRUE if the cursor was not before first
				in tree */
	btr_pcur_t*	cursor,	/* in: persistent cursor; NOTE that the
				function may release the page latch */
	mtr_t*		mtr)	/* in: mtr */
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;

	if (btr_pcur_is_before_first_on_page(cursor, mtr)) {

		if (btr_pcur_is_before_first_in_tree(cursor, mtr)) {

			return(FALSE);
		}

		btr_pcur_move_backward_from_page(cursor, mtr);

		return(TRUE);
	}

	btr_pcur_move_to_prev_on_page(cursor, mtr);

	return(TRUE);
}

/******************************************************************
If mode is PAGE_CUR_G or PAGE_CUR_GE, opens a persistent cursor on the first
user record satisfying the search condition, in the case PAGE_CUR_L or
PAGE_CUR_LE, on the last user record. If no such user record exists, then
in the first case sets the cursor after last in tree, and in the latter case
before first in tree. The latching mode must be BTR_SEARCH_LEAF or
BTR_MODIFY_LEAF. */

void
btr_pcur_open_on_user_rec(
/*======================*/
	dict_index_t*	index,		/* in: index */
	dtuple_t*	tuple,		/* in: tuple on which search done */
	ulint		mode,		/* in: PAGE_CUR_L, ... */
	ulint		latch_mode,	/* in: BTR_SEARCH_LEAF or
					BTR_MODIFY_LEAF */
	btr_pcur_t*	cursor, 	/* in: memory buffer for persistent
					cursor */
	mtr_t*		mtr)		/* in: mtr */
{
	btr_pcur_open(index, tuple, mode, latch_mode, cursor, mtr);

	if ((mode == PAGE_CUR_GE) || (mode == PAGE_CUR_G)) {
	
		if (btr_pcur_is_after_last_on_page(cursor, mtr)) {

			btr_pcur_move_to_next_user_rec(cursor, mtr);
		}
	} else {
		ut_ad((mode == PAGE_CUR_LE) || (mode == PAGE_CUR_L));

		/* Not implemented yet */

		ut_error;
	}
}

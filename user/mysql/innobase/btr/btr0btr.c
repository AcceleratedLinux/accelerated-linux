/******************************************************
The B-tree

(c) 1994-1996 Innobase Oy

Created 6/2/1994 Heikki Tuuri
*******************************************************/

#include "btr0btr.h"

#ifdef UNIV_NONINL
#include "btr0btr.ic"
#endif

#include "fsp0fsp.h"
#include "page0page.h"
#include "btr0cur.h"
#include "btr0sea.h"
#include "btr0pcur.h"
#include "rem0cmp.h"
#include "lock0lock.h"
#include "ibuf0ibuf.h"

/*
Latching strategy of the InnoDB B-tree
--------------------------------------
A tree latch protects all non-leaf nodes of the tree. Each node of a tree
also has a latch of its own.

A B-tree operation normally first acquires an S-latch on the tree. It
searches down the tree and releases the tree latch when it has the
leaf node latch. To save CPU time we do not acquire any latch on
non-leaf nodes of the tree during a search, those pages are only bufferfixed.

If an operation needs to restructure the tree, it acquires an X-latch on
the tree before searching to a leaf node. If it needs, for example, to
split a leaf,
(1) InnoDB decides the split point in the leaf,
(2) allocates a new page,
(3) inserts the appropriate node pointer to the first non-leaf level,
(4) releases the tree X-latch,
(5) and then moves records from the leaf to the new allocated page.

Node pointers
-------------
Leaf pages of a B-tree contain the index records stored in the
tree. On levels n > 0 we store 'node pointers' to pages on level
n - 1. For each page there is exactly one node pointer stored:
thus the our tree is an ordinary B-tree, not a B-link tree.

A node pointer contains a prefix P of an index record. The prefix
is long enough so that it determines an index record uniquely.
The file page number of the child page is added as the last
field. To the child page we can store node pointers or index records
which are >= P in the alphabetical order, but < P1 if there is
a next node pointer on the level, and P1 is its prefix.

If a node pointer with a prefix P points to a non-leaf child, 
then the leftmost record in the child must have the same
prefix P. If it points to a leaf node, the child is not required
to contain any record with a prefix equal to P. The leaf case
is decided this way to allow arbitrary deletions in a leaf node
without touching upper levels of the tree.

We have predefined a special minimum record which we
define as the smallest record in any alphabetical order.
A minimum record is denoted by setting a bit in the record
header. A minimum record acts as the prefix of a node pointer
which points to a leftmost node on any level of the tree.

File page allocation
--------------------
In the root node of a B-tree there are two file segment headers.
The leaf pages of a tree are allocated from one file segment, to
make them consecutive on disk if possible. From the other file segment
we allocate pages for the non-leaf levels of the tree.
*/

/* If this many inserts occur sequentially, it affects page split */
#define BTR_PAGE_SEQ_INSERT_LIMIT	5

/******************************************************************
Creates a new index page to the tree (not the root, and also not
used in page reorganization). */
static
void
btr_page_create(
/*============*/
	page_t*		page,	/* in: page to be created */
	dict_tree_t*	tree,	/* in: index tree */
	mtr_t*		mtr);	/* in: mtr */
/******************************************************************
Sets the child node file address in a node pointer. */
UNIV_INLINE
void
btr_node_ptr_set_child_page_no(
/*===========================*/
	rec_t*	rec,		/* in: node pointer record */
	ulint	page_no,	/* in: child node address */
	mtr_t*	mtr);		/* in: mtr */
/****************************************************************
Returns the upper level node pointer to a page. It is assumed that
mtr holds an x-latch on the tree. */
static
rec_t*
btr_page_get_father_node_ptr(
/*=========================*/
				/* out: pointer to node pointer record */
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page: must contain at least one
				user record */
	mtr_t*		mtr);	/* in: mtr */
/*****************************************************************
Empties an index page. */
static
void
btr_page_empty(
/*===========*/
	page_t*	page,	/* in: page to be emptied */
	mtr_t*	mtr);	/* in: mtr */
/*****************************************************************
Returns TRUE if the insert fits on the appropriate half-page
with the chosen split_rec. */
static
ibool
btr_page_insert_fits(
/*=================*/
					/* out: TRUE if fits */
	btr_cur_t*	cursor,		/* in: cursor at which insert
					should be made */
	rec_t*		split_rec,	/* in: suggestion for first record
					on upper half-page, or NULL if
					tuple should be first */
	dtuple_t*	tuple);		/* in: tuple to insert */	

/******************************************************************
Gets the root node of a tree and x-latches it. */

page_t*
btr_root_get(
/*=========*/
				/* out: root page, x-latched */
	dict_tree_t*	tree,	/* in: index tree */
	mtr_t*		mtr)	/* in: mtr */
{
	ulint	space;
	ulint	root_page_no;
	page_t*	root;
	
	space = dict_tree_get_space(tree);
	root_page_no = dict_tree_get_page(tree);

	root = btr_page_get(space, root_page_no, RW_X_LATCH, mtr);
	
	return(root);
}

/*****************************************************************
Gets pointer to the previous user record in the tree. It is assumed that
the caller has appropriate latches on the page and its neighbor. */

rec_t*
btr_get_prev_user_rec(
/*==================*/
			/* out: previous user record, NULL if there is none */
	rec_t*	rec,	/* in: record on leaf level */
	mtr_t*	mtr)	/* in: mtr holding a latch on the page, and if
			needed, also to the previous page */
{
	page_t*	page;
	page_t*	prev_page;
	ulint	prev_page_no;
	rec_t*	prev_rec;
	ulint	space;

	page = buf_frame_align(rec);
	
	if (page_get_infimum_rec(page) != rec) {

		prev_rec = page_rec_get_prev(rec);

		if (page_get_infimum_rec(page) != prev_rec) {

			return(prev_rec);
		}
	}
	
	prev_page_no = btr_page_get_prev(page, mtr);
	space = buf_frame_get_space_id(page);
	
	if (prev_page_no != FIL_NULL) {

		prev_page = buf_page_get_with_no_latch(space, prev_page_no,
									mtr);
		/* The caller must already have a latch to the brother */
		ut_ad((mtr_memo_contains(mtr, buf_block_align(prev_page),
		      				MTR_MEMO_PAGE_S_FIX))
		      || (mtr_memo_contains(mtr, buf_block_align(prev_page),
		      				MTR_MEMO_PAGE_X_FIX)));

		prev_rec = page_rec_get_prev(page_get_supremum_rec(prev_page));

		return(prev_rec);
	}

	return(NULL);
}

/*****************************************************************
Gets pointer to the next user record in the tree. It is assumed that the
caller has appropriate latches on the page and its neighbor. */

rec_t*
btr_get_next_user_rec(
/*==================*/
			/* out: next user record, NULL if there is none */
	rec_t*	rec,	/* in: record on leaf level */
	mtr_t*	mtr)	/* in: mtr holding a latch on the page, and if
			needed, also to the next page */
{
	page_t*	page;
	page_t*	next_page;
	ulint	next_page_no;
	rec_t*	next_rec;
	ulint	space;

	page = buf_frame_align(rec);
	
	if (page_get_supremum_rec(page) != rec) {

		next_rec = page_rec_get_next(rec);

		if (page_get_supremum_rec(page) != next_rec) {

			return(next_rec);
		}
	}
	
	next_page_no = btr_page_get_next(page, mtr);
	space = buf_frame_get_space_id(page);
	
	if (next_page_no != FIL_NULL) {

		next_page = buf_page_get_with_no_latch(space, next_page_no,
									mtr);
		/* The caller must already have a latch to the brother */
		ut_ad((mtr_memo_contains(mtr, buf_block_align(next_page),
		      				MTR_MEMO_PAGE_S_FIX))
		      || (mtr_memo_contains(mtr, buf_block_align(next_page),
		      				MTR_MEMO_PAGE_X_FIX)));

		next_rec = page_rec_get_next(page_get_infimum_rec(next_page));

		return(next_rec);
	}

	return(NULL);
}

/******************************************************************
Creates a new index page to the tree (not the root, and also not used in
page reorganization). */
static
void
btr_page_create(
/*============*/
	page_t*		page,	/* in: page to be created */
	dict_tree_t*	tree,	/* in: index tree */
	mtr_t*		mtr)	/* in: mtr */
{
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	page_create(page, mtr);
	buf_block_align(page)->check_index_page_at_flush = TRUE;
	
	btr_page_set_index_id(page, tree->id, mtr);
}

/******************************************************************
Allocates a new file page to be used in an ibuf tree. Takes the page from
the free list of the tree, which must contain pages! */
static
page_t*
btr_page_alloc_for_ibuf(
/*====================*/
				/* out: new allocated page, x-latched */
	dict_tree_t*	tree,	/* in: index tree */
	mtr_t*		mtr)	/* in: mtr */
{
	fil_addr_t	node_addr;
	page_t*		root;
	page_t*		new_page;

	root = btr_root_get(tree, mtr);
	
	node_addr = flst_get_first(root + PAGE_HEADER
					+ PAGE_BTR_IBUF_FREE_LIST, mtr);
	ut_a(node_addr.page != FIL_NULL);

	new_page = buf_page_get(dict_tree_get_space(tree), node_addr.page,
							RW_X_LATCH, mtr);
	buf_page_dbg_add_level(new_page, SYNC_TREE_NODE_NEW);

	flst_remove(root + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST,
		    new_page + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST_NODE,
									mtr);
	ut_ad(flst_validate(root + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST, mtr));

	return(new_page);
}

/******************************************************************
Allocates a new file page to be used in an index tree. NOTE: we assume
that the caller has made the reservation for free extents! */

page_t*
btr_page_alloc(
/*===========*/
					/* out: new allocated page, x-latched;
					NULL if out of space */
	dict_tree_t*	tree,		/* in: index tree */
	ulint		hint_page_no,	/* in: hint of a good page */
	byte		file_direction,	/* in: direction where a possible
					page split is made */
	ulint		level,		/* in: level where the page is placed
					in the tree */
	mtr_t*		mtr)		/* in: mtr */
{
	fseg_header_t*	seg_header;
	page_t*		root;
	page_t*		new_page;
	ulint		new_page_no;

	if (tree->type & DICT_IBUF) {

		return(btr_page_alloc_for_ibuf(tree, mtr));
	}

	root = btr_root_get(tree, mtr);
		
	if (level == 0) {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_LEAF;
	} else {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_TOP;
	}

	/* Parameter TRUE below states that the caller has made the
	reservation for free extents, and thus we know that a page can
	be allocated: */
	
	new_page_no = fseg_alloc_free_page_general(seg_header, hint_page_no,
						file_direction, TRUE, mtr);
	if (new_page_no == FIL_NULL) {

		return(NULL);
	}

	new_page = buf_page_get(dict_tree_get_space(tree), new_page_no,
							RW_X_LATCH, mtr);
	buf_page_dbg_add_level(new_page, SYNC_TREE_NODE_NEW);
							
	return(new_page);
}	

/******************************************************************
Gets the number of pages in a B-tree. */

ulint
btr_get_size(
/*=========*/
				/* out: number of pages */
	dict_index_t*	index,	/* in: index */
	ulint		flag)	/* in: BTR_N_LEAF_PAGES or BTR_TOTAL_SIZE */
{
	fseg_header_t*	seg_header;
	page_t*		root;
	ulint		n;
	ulint		dummy;
	mtr_t		mtr;

	mtr_start(&mtr);

	mtr_s_lock(dict_tree_get_lock(index->tree), &mtr);

	root = btr_root_get(index->tree, &mtr);
		
	if (flag == BTR_N_LEAF_PAGES) {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_LEAF;
		
		fseg_n_reserved_pages(seg_header, &n, &mtr);
		
	} else if (flag == BTR_TOTAL_SIZE) {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_TOP;

		n = fseg_n_reserved_pages(seg_header, &dummy, &mtr);
		
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_LEAF;
		
		n += fseg_n_reserved_pages(seg_header, &dummy, &mtr);		
	} else {
		ut_a(0);
	}

	mtr_commit(&mtr);

	return(n);
}	

/******************************************************************
Frees a page used in an ibuf tree. Puts the page to the free list of the
ibuf tree. */
static
void
btr_page_free_for_ibuf(
/*===================*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page to be freed, x-latched */	
	mtr_t*		mtr)	/* in: mtr */
{
	page_t*		root;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	root = btr_root_get(tree, mtr);
	
	flst_add_first(root + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST,
		       page + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST_NODE, mtr);

	ut_ad(flst_validate(root + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST, mtr));
}

/******************************************************************
Frees a file page used in an index tree. Can be used also to (BLOB)
external storage pages, because the page level 0 can be given as an
argument. */

void
btr_page_free_low(
/*==============*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page to be freed, x-latched */	
	ulint		level,	/* in: page level */
	mtr_t*		mtr)	/* in: mtr */
{
	fseg_header_t*	seg_header;
	page_t*		root;
	ulint		space;
	ulint		page_no;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	/* The page gets invalid for optimistic searches: increment the frame
	modify clock */

	buf_frame_modify_clock_inc(page);
	
	if (tree->type & DICT_IBUF) {

		btr_page_free_for_ibuf(tree, page, mtr);

		return;
	}

	root = btr_root_get(tree, mtr);
	
	if (level == 0) {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_LEAF;
	} else {
		seg_header = root + PAGE_HEADER + PAGE_BTR_SEG_TOP;
	}

	space = buf_frame_get_space_id(page);
	page_no = buf_frame_get_page_no(page);
	
	fseg_free_page(seg_header, space, page_no, mtr);
}	

/******************************************************************
Frees a file page used in an index tree. NOTE: cannot free field external
storage pages because the page must contain info on its level. */

void
btr_page_free(
/*==========*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page to be freed, x-latched */	
	mtr_t*		mtr)	/* in: mtr */
{
	ulint		level;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	level = btr_page_get_level(page, mtr);
	
	btr_page_free_low(tree, page, level, mtr);
}	

/******************************************************************
Sets the child node file address in a node pointer. */
UNIV_INLINE
void
btr_node_ptr_set_child_page_no(
/*===========================*/
	rec_t*	rec,		/* in: node pointer record */
	ulint	page_no,	/* in: child node address */
	mtr_t*	mtr)		/* in: mtr */
{
	ulint	n_fields;
	byte*	field;
	ulint	len;

	ut_ad(0 < btr_page_get_level(buf_frame_align(rec), mtr));
	
	n_fields = rec_get_n_fields(rec);

	/* The child address is in the last field */	
	field = rec_get_nth_field(rec, n_fields - 1, &len);

	ut_ad(len == 4);
	
	mlog_write_ulint(field, page_no, MLOG_4BYTES, mtr);
}

/****************************************************************
Returns the child page of a node pointer and x-latches it. */
static
page_t*
btr_node_ptr_get_child(
/*===================*/
	 			/* out: child page, x-latched */
	rec_t*	node_ptr,	/* in: node pointer */
	mtr_t*	mtr)		/* in: mtr */
{
	ulint	page_no;
	ulint	space;
	page_t*	page;
	
	space = buf_frame_get_space_id(node_ptr);
	page_no = btr_node_ptr_get_child_page_no(node_ptr);

	page = btr_page_get(space, page_no, RW_X_LATCH, mtr);
	
	return(page);
}

/****************************************************************
Returns the upper level node pointer to a page. It is assumed that mtr holds
an x-latch on the tree. */
static
rec_t*
btr_page_get_father_for_rec(
/*========================*/
				/* out: pointer to node pointer record,
				its page x-latched */
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page: must contain at least one
				user record */
	rec_t*		user_rec,/* in: user_record on page */
	mtr_t*		mtr)	/* in: mtr */
{
	mem_heap_t*	heap;
	dtuple_t*	tuple;
	btr_cur_t	cursor;
	rec_t*		node_ptr;

	ut_ad(mtr_memo_contains(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK));
	ut_a(user_rec != page_get_supremum_rec(page));
	ut_a(user_rec != page_get_infimum_rec(page));
	
	ut_ad(dict_tree_get_page(tree) != buf_frame_get_page_no(page));

	heap = mem_heap_create(100);

	tuple = dict_tree_build_node_ptr(tree, user_rec, 0, heap,
					 btr_page_get_level(page, mtr));

	/* In the following, we choose just any index from the tree as the
	first parameter for btr_cur_search_to_nth_level. */
	
	btr_cur_search_to_nth_level(UT_LIST_GET_FIRST(tree->tree_indexes),
				btr_page_get_level(page, mtr) + 1,
				tuple, PAGE_CUR_LE,
				BTR_CONT_MODIFY_TREE, &cursor, 0, mtr);

	node_ptr = btr_cur_get_rec(&cursor);

	if (btr_node_ptr_get_child_page_no(node_ptr) !=
                                                buf_frame_get_page_no(page)) {
		fprintf(stderr,
"InnoDB: Dump of the child page:\n");
		buf_page_print(buf_frame_align(page));
		fprintf(stderr,
"InnoDB: Dump of the parent page:\n");
		buf_page_print(buf_frame_align(node_ptr));

      		fprintf(stderr,
"InnoDB: Corruption of an index tree: table %s, index %s,\n"
"InnoDB: father ptr page no %lu, child page no %lu\n",
                    (UT_LIST_GET_FIRST(tree->tree_indexes))->table_name,
                    (UT_LIST_GET_FIRST(tree->tree_indexes))->name,
                    btr_node_ptr_get_child_page_no(node_ptr),
                    buf_frame_get_page_no(page));
     		page_rec_print(page_rec_get_next(page_get_infimum_rec(page)));
     		page_rec_print(node_ptr);

      		fprintf(stderr,
"InnoDB: You should dump + drop + reimport the table to fix the\n"
"InnoDB: corruption. If the crash happens at the database startup, see\n"
"InnoDB: section 6.1 of http://www.innodb.com/ibman.html about forcing\n"
"InnoDB: recovery. Then dump + drop + reimport.\n");
	}

	ut_a(btr_node_ptr_get_child_page_no(node_ptr) ==
						buf_frame_get_page_no(page));
	mem_heap_free(heap);

	return(node_ptr);
}

/****************************************************************
Returns the upper level node pointer to a page. It is assumed that
mtr holds an x-latch on the tree. */
static
rec_t*
btr_page_get_father_node_ptr(
/*=========================*/
				/* out: pointer to node pointer record */
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page: must contain at least one
				user record */
	mtr_t*		mtr)	/* in: mtr */
{
	return(btr_page_get_father_for_rec(tree, page,
		page_rec_get_next(page_get_infimum_rec(page)), mtr));
}

/****************************************************************
Creates the root node for a new index tree. */

ulint
btr_create(
/*=======*/
			/* out: page number of the created root, FIL_NULL if
			did not succeed */
	ulint	type,	/* in: type of the index */
	ulint	space,	/* in: space where created */
	dulint	index_id,/* in: index id */
	mtr_t*	mtr)	/* in: mini-transaction handle */
{
	ulint		page_no;
	buf_frame_t*	ibuf_hdr_frame;
	buf_frame_t*	frame;
	page_t*		page;

	/* Create the two new segments (one, in the case of an ibuf tree) for
	the index tree; the segment headers are put on the allocated root page
	(for an ibuf tree, not in the root, but on a separate ibuf header
	page) */

	if (type & DICT_IBUF) {
		/* Allocate first the ibuf header page */
		ibuf_hdr_frame = fseg_create(space, 0,
				IBUF_HEADER + IBUF_TREE_SEG_HEADER, mtr);

		buf_page_dbg_add_level(ibuf_hdr_frame, SYNC_TREE_NODE_NEW);

		ut_ad(buf_frame_get_page_no(ibuf_hdr_frame)
 						== IBUF_HEADER_PAGE_NO);
		/* Allocate then the next page to the segment: it will be the
 		tree root page */

 		page_no = fseg_alloc_free_page(
				ibuf_hdr_frame + IBUF_HEADER
 				+ IBUF_TREE_SEG_HEADER, IBUF_TREE_ROOT_PAGE_NO,
				FSP_UP, mtr);
		ut_ad(page_no == IBUF_TREE_ROOT_PAGE_NO);

		frame = buf_page_get(space, page_no, RW_X_LATCH, mtr);
	} else {
		frame = fseg_create(space, 0, PAGE_HEADER + PAGE_BTR_SEG_TOP,
									mtr);
	}
	
	if (frame == NULL) {

		return(FIL_NULL);
	}

	page_no = buf_frame_get_page_no(frame);
	
	buf_page_dbg_add_level(frame, SYNC_TREE_NODE_NEW);

	if (type & DICT_IBUF) {
		/* It is an insert buffer tree: initialize the free list */

		ut_ad(page_no == IBUF_TREE_ROOT_PAGE_NO);
		
		flst_init(frame + PAGE_HEADER + PAGE_BTR_IBUF_FREE_LIST, mtr);
	} else {	
		/* It is a non-ibuf tree: create a file segment for leaf
		pages */
		fseg_create(space, page_no, PAGE_HEADER + PAGE_BTR_SEG_LEAF,
									mtr);
		/* The fseg create acquires a second latch on the page,
		therefore we must declare it: */
		buf_page_dbg_add_level(frame, SYNC_TREE_NODE_NEW);
	}
	
	/* Create a new index page on the the allocated segment page */
	page = page_create(frame, mtr);
	buf_block_align(page)->check_index_page_at_flush = TRUE;

	/* Set the index id of the page */
	btr_page_set_index_id(page, index_id, mtr);

	/* Set the level of the new index page */
	btr_page_set_level(page, 0, mtr);
	
	/* Set the next node and previous node fields */
	btr_page_set_next(page, FIL_NULL, mtr);
	btr_page_set_prev(page, FIL_NULL, mtr);

	/* We reset the free bits for the page to allow creation of several
	trees in the same mtr, otherwise the latch on a bitmap page would
	prevent it because of the latching order */
	
	ibuf_reset_free_bits_with_type(type, page);

	/* In the following assertion we test that two records of maximum
	allowed size fit on the root page: this fact is needed to ensure
	correctness of split algorithms */

	ut_ad(page_get_max_insert_size(page, 2) > 2 * BTR_PAGE_MAX_REC_SIZE);

	return(page_no);
}

/****************************************************************
Frees a B-tree except the root page, which MUST be freed after this
by calling btr_free_root. */

void
btr_free_but_not_root(
/*==================*/
	ulint	space,		/* in: space where created */
	ulint	root_page_no)	/* in: root page number */
{
	ibool	finished;
	page_t*	root;
	mtr_t	mtr;

leaf_loop:	
	mtr_start(&mtr);
	
	root = btr_page_get(space, root_page_no, RW_X_LATCH, &mtr);	

	/* NOTE: page hash indexes are dropped when a page is freed inside
	fsp0fsp. */

	finished = fseg_free_step(
				root + PAGE_HEADER + PAGE_BTR_SEG_LEAF, &mtr);
	mtr_commit(&mtr);

	if (!finished) {

		goto leaf_loop;
	}
top_loop:
	mtr_start(&mtr);
	
	root = btr_page_get(space, root_page_no, RW_X_LATCH, &mtr);	

	finished = fseg_free_step_not_header(
				root + PAGE_HEADER + PAGE_BTR_SEG_TOP, &mtr);
	mtr_commit(&mtr);

	if (!finished) {

		goto top_loop;
	}	
}

/****************************************************************
Frees the B-tree root page. Other tree MUST already have been freed. */

void
btr_free_root(
/*==========*/
	ulint	space,		/* in: space where created */
	ulint	root_page_no,	/* in: root page number */
	mtr_t*	mtr)		/* in: a mini-transaction which has already
				been started */
{
	ibool	finished;
	page_t*	root;

	root = btr_page_get(space, root_page_no, RW_X_LATCH, mtr);

	btr_search_drop_page_hash_index(root);	
top_loop:	
	finished = fseg_free_step(
				root + PAGE_HEADER + PAGE_BTR_SEG_TOP, mtr);
	if (!finished) {

		goto top_loop;
	}	
}

/*****************************************************************
Reorganizes an index page. */
static
void
btr_page_reorganize_low(
/*====================*/
	ibool	recovery,/* in: TRUE if called in recovery: locks should not
			be updated, i.e., there cannot exist locks on the
			page, and a hash index should not be dropped: it
			cannot exist */
	page_t*	page,	/* in: page to be reorganized */
	mtr_t*	mtr)	/* in: mtr */
{
	page_t*	new_page;
	ulint	log_mode;
	ulint	data_size1;
	ulint	data_size2;
	ulint	max_ins_size1;
	ulint	max_ins_size2;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	data_size1 = page_get_data_size(page);
	max_ins_size1 = page_get_max_insert_size_after_reorganize(page, 1);

	/* Write the log record */
	mlog_write_initial_log_record(page, MLOG_PAGE_REORGANIZE, mtr);

	/* Turn logging off */
	log_mode = mtr_set_log_mode(mtr, MTR_LOG_NONE);

	new_page = buf_frame_alloc();

	/* Copy the old page to temporary space */
	buf_frame_copy(new_page, page);

	if (!recovery) {
		btr_search_drop_page_hash_index(page);
	}

	/* Recreate the page: note that global data on page (possible
	segment headers, next page-field, etc.) is preserved intact */

	page_create(page, mtr);
	buf_block_align(page)->check_index_page_at_flush = TRUE;
	
	/* Copy the records from the temporary space to the recreated page;
	do not copy the lock bits yet */

	page_copy_rec_list_end_no_locks(page, new_page,
					page_get_infimum_rec(new_page), mtr);
	/* Copy max trx id to recreated page */
	page_set_max_trx_id(page, page_get_max_trx_id(new_page));
	
	if (!recovery) {
		/* Update the record lock bitmaps */
		lock_move_reorganize_page(page, new_page);
	}

	data_size2 = page_get_data_size(page);
	max_ins_size2 = page_get_max_insert_size_after_reorganize(page, 1);

	if (data_size1 != data_size2 || max_ins_size1 != max_ins_size2) {
		buf_page_print(page);
		buf_page_print(new_page);
	        fprintf(stderr,
"InnoDB: Error: page old data size %lu new data size %lu\n"
"InnoDB: Error: page old max ins size %lu new max ins size %lu\n"
"InnoDB: Make a detailed bug report and send it to mysql@lists.mysql.com\n",
			data_size1, data_size2, max_ins_size1, max_ins_size2);
	}

	buf_frame_free(new_page);

	/* Restore logging mode */
	mtr_set_log_mode(mtr, log_mode);
}

/*****************************************************************
Reorganizes an index page. */

void
btr_page_reorganize(
/*================*/
	page_t*	page,	/* in: page to be reorganized */
	mtr_t*	mtr)	/* in: mtr */
{
	btr_page_reorganize_low(FALSE, page, mtr);
}

/***************************************************************
Parses a redo log record of reorganizing a page. */

byte*
btr_parse_page_reorganize(
/*======================*/
			/* out: end of log record or NULL */
	byte*	ptr,	/* in: buffer */
	byte*	end_ptr __attribute__((unused)), /* in: buffer end */
	page_t*	page,	/* in: page or NULL */
	mtr_t*	mtr)	/* in: mtr or NULL */
{
	ut_ad(ptr && end_ptr);

	/* The record is empty, except for the record initial part */

	if (page) {
		btr_page_reorganize_low(TRUE, page, mtr);
	}

	return(ptr);
}

/*****************************************************************
Empties an index page. */
static
void
btr_page_empty(
/*===========*/
	page_t*	page,	/* in: page to be emptied */
	mtr_t*	mtr)	/* in: mtr */
{
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	btr_search_drop_page_hash_index(page);

	/* Recreate the page: note that global data on page (possible
	segment headers, next page-field, etc.) is preserved intact */

	page_create(page, mtr);
	buf_block_align(page)->check_index_page_at_flush = TRUE;
}

/*****************************************************************
Makes tree one level higher by splitting the root, and inserts
the tuple. It is assumed that mtr contains an x-latch on the tree.
NOTE that the operation of this function must always succeed,
we cannot reverse it: therefore enough free disk space must be
guaranteed to be available before this function is called. */

rec_t*
btr_root_raise_and_insert(
/*======================*/
				/* out: inserted record */
	btr_cur_t*	cursor,	/* in: cursor at which to insert: must be
				on the root page; when the function returns,
				the cursor is positioned on the predecessor
				of the inserted record */
	dtuple_t*	tuple,	/* in: tuple to insert */
	mtr_t*		mtr)	/* in: mtr */
{
	dict_tree_t*	tree;
	page_t*		root;
	page_t*		new_page;
	ulint		new_page_no;
	rec_t*		rec;
	mem_heap_t*	heap;
	dtuple_t*	node_ptr;
	ulint		level;	
	rec_t*		node_ptr_rec;
	page_cur_t*	page_cursor;
	
	root = btr_cur_get_page(cursor);
	tree = btr_cur_get_tree(cursor);

	ut_ad(dict_tree_get_page(tree) == buf_frame_get_page_no(root));
	ut_ad(mtr_memo_contains(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, buf_block_align(root),
			      				MTR_MEMO_PAGE_X_FIX));
	btr_search_drop_page_hash_index(root);

	/* Allocate a new page to the tree. Root splitting is done by first
	moving the root records to the new page, emptying the root, putting
	a node pointer to the new page, and then splitting the new page. */
	
	new_page = btr_page_alloc(tree, 0, FSP_NO_DIR,
				  btr_page_get_level(root, mtr), mtr);

	btr_page_create(new_page, tree, mtr);

	level = btr_page_get_level(root, mtr);
	
	/* Set the levels of the new index page and root page */
	btr_page_set_level(new_page, level, mtr);
	btr_page_set_level(root, level + 1, mtr);
	
	/* Set the next node and previous node fields of new page */
	btr_page_set_next(new_page, FIL_NULL, mtr);
	btr_page_set_prev(new_page, FIL_NULL, mtr);

	/* Move the records from root to the new page */

	page_move_rec_list_end(new_page, root, page_get_infimum_rec(root),
									mtr);
	/* If this is a pessimistic insert which is actually done to
	perform a pessimistic update then we have stored the lock
	information of the record to be inserted on the infimum of the
	root page: we cannot discard the lock structs on the root page */
	
	lock_update_root_raise(new_page, root);

	/* Create a memory heap where the node pointer is stored */
	heap = mem_heap_create(100);

	rec = page_rec_get_next(page_get_infimum_rec(new_page));
	new_page_no = buf_frame_get_page_no(new_page);
	
	/* Build the node pointer (= node key and page address) for the
	child */

	node_ptr = dict_tree_build_node_ptr(tree, rec, new_page_no, heap,
					                          level);
	/* Reorganize the root to get free space */
	btr_page_reorganize(root, mtr);	

	page_cursor = btr_cur_get_page_cur(cursor);
	
	/* Insert node pointer to the root */

	page_cur_set_before_first(root, page_cursor);

	node_ptr_rec = page_cur_tuple_insert(page_cursor, node_ptr, mtr);

	ut_ad(node_ptr_rec);

	/* The node pointer must be marked as the predefined minimum record,
	as there is no lower alphabetical limit to records in the leftmost
	node of a level: */

	btr_set_min_rec_mark(node_ptr_rec, mtr);
		
	/* Free the memory heap */
	mem_heap_free(heap);

	/* We play safe and reset the free bits for the new page */

/*	printf("Root raise new page no %lu\n",
					buf_frame_get_page_no(new_page)); */

	ibuf_reset_free_bits(UT_LIST_GET_FIRST(tree->tree_indexes),
								new_page);
	/* Reposition the cursor to the child node */
	page_cur_search(new_page, tuple, PAGE_CUR_LE, page_cursor);
	
	/* Split the child and insert tuple */
	return(btr_page_split_and_insert(cursor, tuple, mtr));
}	

/*****************************************************************
Decides if the page should be split at the convergence point of inserts
converging to the left. */

ibool
btr_page_get_split_rec_to_left(
/*===========================*/
				/* out: TRUE if split recommended */
	btr_cur_t*	cursor,	/* in: cursor at which to insert */
	rec_t**		split_rec) /* out: if split recommended,
				the first record on upper half page,
				or NULL if tuple to be inserted should
				be first */
{
	page_t*	page;
	rec_t*	insert_point;
	rec_t*	infimum;

	page = btr_cur_get_page(cursor);
	insert_point = btr_cur_get_rec(cursor);

	if ((page_header_get_ptr(page, PAGE_LAST_INSERT)
	    == page_rec_get_next(insert_point))
	    && (page_header_get_field(page, PAGE_DIRECTION) == PAGE_LEFT)
	    && ((page_header_get_field(page, PAGE_N_DIRECTION)
	     	 			>= BTR_PAGE_SEQ_INSERT_LIMIT)
	     	|| (page_header_get_field(page, PAGE_N_DIRECTION) + 1
	     	 			>= page_get_n_recs(page)))) {

	     	infimum = page_get_infimum_rec(page);
	     	 			
		if ((infimum != insert_point)
		    && (page_rec_get_next(infimum) != insert_point)) {

			*split_rec = insert_point;
		} else {
	     		*split_rec = page_rec_get_next(insert_point);
	     	}

		return(TRUE);
	}

	return(FALSE);
}

/*****************************************************************
Decides if the page should be split at the convergence point of inserts
converging to the right. */

ibool
btr_page_get_split_rec_to_right(
/*============================*/
				/* out: TRUE if split recommended */
	btr_cur_t*	cursor,	/* in: cursor at which to insert */
	rec_t**		split_rec) /* out: if split recommended,
				the first record on upper half page,
				or NULL if tuple to be inserted should
				be first */
{
	page_t*	page;
	rec_t*	insert_point;
	rec_t*	supremum;

	page = btr_cur_get_page(cursor);
	insert_point = btr_cur_get_rec(cursor);

	if ((page_header_get_ptr(page, PAGE_LAST_INSERT) == insert_point)
	    && (page_header_get_field(page, PAGE_DIRECTION) == PAGE_RIGHT)
	    && ((page_header_get_field(page, PAGE_N_DIRECTION)
	     	 			>= BTR_PAGE_SEQ_INSERT_LIMIT)
	     	|| (page_header_get_field(page, PAGE_N_DIRECTION) + 1
	     	 			>= page_get_n_recs(page)))) {

	     	supremum = page_get_supremum_rec(page);
	     	 			
		if ((page_rec_get_next(insert_point) != supremum)
		    && (page_rec_get_next(page_rec_get_next(insert_point))
			!= supremum)
		    && (page_rec_get_next(page_rec_get_next(
					page_rec_get_next(insert_point)))
			!= supremum)) {

			/* If there are >= 3 user records up from the insert
			point, split all but 2 off */

			*split_rec = page_rec_get_next(page_rec_get_next(
					page_rec_get_next(insert_point)));
		} else {
			/* Else split at inserted record */
	     		*split_rec = NULL;
	     	}

		return(TRUE);
	}

	return(FALSE);
}

/*****************************************************************
Calculates a split record such that the tuple will certainly fit on
its half-page when the split is performed. We assume in this function
only that the cursor page has at least one user record. */
static
rec_t*
btr_page_get_sure_split_rec(
/*========================*/
					/* out: split record, or NULL if
					tuple will be the first record on
					upper half-page */
	btr_cur_t*	cursor,		/* in: cursor at which insert
					should be made */
	dtuple_t*	tuple)		/* in: tuple to insert */	
{
	page_t*	page;
	ulint	insert_size;
	ulint	free_space;
	ulint	total_data;
	ulint	total_n_recs;
	ulint	total_space;
	ulint	incl_data;
	rec_t*	ins_rec;
	rec_t*	rec;
	rec_t*	next_rec;
	ulint	n;
	
	page = btr_cur_get_page(cursor);
	
	insert_size = rec_get_converted_size(tuple);
	free_space  = page_get_free_space_of_empty();

	/* free_space is now the free space of a created new page */

	total_data   = page_get_data_size(page) + insert_size;
	total_n_recs = page_get_n_recs(page) + 1;
	ut_ad(total_n_recs >= 2);
	total_space  = total_data + page_dir_calc_reserved_space(total_n_recs);

	n = 0;
	incl_data = 0;
	ins_rec = btr_cur_get_rec(cursor);
	rec = page_get_infimum_rec(page);

	/* We start to include records to the left half, and when the
	space reserved by them exceeds half of total_space, then if
	the included records fit on the left page, they will be put there
	if something was left over also for the right page,
	otherwise the last included record will be the first on the right
	half page */

	for (;;) {
		/* Decide the next record to include */
		if (rec == ins_rec) {
			rec = NULL;	/* NULL denotes that tuple is
					now included */
		} else if (rec == NULL) {
			rec = page_rec_get_next(ins_rec);
		} else {
			rec = page_rec_get_next(rec);
		}

		if (rec == NULL) {
			/* Include tuple */
			incl_data += insert_size;
		} else {
			incl_data += rec_get_size(rec);
		}

		n++;
		
		if (incl_data + page_dir_calc_reserved_space(n)
                    >= total_space / 2) {

                    	if (incl_data + page_dir_calc_reserved_space(n)
                    	    <= free_space) {
                    	    	/* The next record will be the first on
                    	    	the right half page if it is not the
                    	    	supremum record of page */

				if (rec == ins_rec) {
					next_rec = NULL;
				} else if (rec == NULL) {
					next_rec = page_rec_get_next(ins_rec);
				} else {
					next_rec = page_rec_get_next(rec);
				}
				if (next_rec != page_get_supremum_rec(page)) {

					return(next_rec);
				}
                    	}

			return(rec);
		}
	}
}		

/*****************************************************************
Returns TRUE if the insert fits on the appropriate half-page with the
chosen split_rec. */
static
ibool
btr_page_insert_fits(
/*=================*/
					/* out: TRUE if fits */
	btr_cur_t*	cursor,		/* in: cursor at which insert
					should be made */
	rec_t*		split_rec,	/* in: suggestion for first record
					on upper half-page, or NULL if
					tuple to be inserted should be first */
	dtuple_t*	tuple)		/* in: tuple to insert */	
{
	page_t*	page;
	ulint	insert_size;
	ulint	free_space;
	ulint	total_data;
	ulint	total_n_recs;
	rec_t*	rec;
	rec_t*	end_rec;
	
	page = btr_cur_get_page(cursor);
	
	insert_size = rec_get_converted_size(tuple);
	free_space  = page_get_free_space_of_empty();

	/* free_space is now the free space of a created new page */

	total_data   = page_get_data_size(page) + insert_size;
	total_n_recs = page_get_n_recs(page) + 1;
	
	/* We determine which records (from rec to end_rec, not including
	end_rec) will end up on the other half page from tuple when it is
	inserted. */
	
	if (split_rec == NULL) {
		rec = page_rec_get_next(page_get_infimum_rec(page));
		end_rec = page_rec_get_next(btr_cur_get_rec(cursor));

	} else if (cmp_dtuple_rec(tuple, split_rec) >= 0) {

		rec = page_rec_get_next(page_get_infimum_rec(page));
 		end_rec = split_rec;
	} else {
		rec = split_rec;
		end_rec = page_get_supremum_rec(page);
	}

	if (total_data + page_dir_calc_reserved_space(total_n_recs)
	    <= free_space) {

		/* Ok, there will be enough available space on the
		half page where the tuple is inserted */

		return(TRUE);
	}

	while (rec != end_rec) {
		/* In this loop we calculate the amount of reserved
		space after rec is removed from page. */

		total_data -= rec_get_size(rec);
		total_n_recs--;

		if (total_data + page_dir_calc_reserved_space(total_n_recs)
                    <= free_space) {

			/* Ok, there will be enough available space on the
			half page where the tuple is inserted */

			return(TRUE);
		}

		rec = page_rec_get_next(rec);
	}

	return(FALSE);
}		

/***********************************************************
Inserts a data tuple to a tree on a non-leaf level. It is assumed
that mtr holds an x-latch on the tree. */

void
btr_insert_on_non_leaf_level(
/*=========================*/
	dict_tree_t*	tree,	/* in: tree */
	ulint		level,	/* in: level, must be > 0 */
	dtuple_t*	tuple,	/* in: the record to be inserted */
	mtr_t*		mtr)	/* in: mtr */
{
	big_rec_t*	dummy_big_rec;
	btr_cur_t	cursor;		
	ulint		err;
	rec_t*		rec;

	ut_ad(level > 0);
	
	/* In the following, choose just any index from the tree as the
	first parameter for btr_cur_search_to_nth_level. */

	btr_cur_search_to_nth_level(UT_LIST_GET_FIRST(tree->tree_indexes),
				    level, tuple, PAGE_CUR_LE,
				    BTR_CONT_MODIFY_TREE,
				    &cursor, 0, mtr);

	err = btr_cur_pessimistic_insert(BTR_NO_LOCKING_FLAG
					| BTR_KEEP_SYS_FLAG
					| BTR_NO_UNDO_LOG_FLAG,
					&cursor, tuple,
					&rec, &dummy_big_rec, NULL, mtr);
	ut_a(err == DB_SUCCESS);
}

/******************************************************************
Attaches the halves of an index page on the appropriate level in an
index tree. */
static
void
btr_attach_half_pages(
/*==================*/
	dict_tree_t*	tree,		/* in: the index tree */
	page_t*		page,		/* in: page to be split */
	rec_t*		split_rec,	/* in: first record on upper
					half page */
	page_t*		new_page,	/* in: the new half page */
	ulint		direction,	/* in: FSP_UP or FSP_DOWN */
	mtr_t*		mtr)		/* in: mtr */
{
	ulint		space;
	rec_t*		node_ptr;
	page_t*		prev_page;
	page_t*		next_page;
	ulint		prev_page_no;
	ulint		next_page_no;
	ulint		level;
	page_t*		lower_page;
	page_t*		upper_page;
	ulint		lower_page_no;
	ulint		upper_page_no;
	dtuple_t*	node_ptr_upper;
	mem_heap_t* 	heap;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains(mtr, buf_block_align(new_page),
			      				MTR_MEMO_PAGE_X_FIX));

	/* Based on split direction, decide upper and lower pages */
	if (direction == FSP_DOWN) {

		lower_page_no = buf_frame_get_page_no(new_page);
		upper_page_no = buf_frame_get_page_no(page);
		lower_page = new_page;
		upper_page = page;

		/* Look from the tree for the node pointer to page */
		node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);

		/* Replace the address of the old child node (= page) with the 
		address of the new lower half */

		btr_node_ptr_set_child_page_no(node_ptr, lower_page_no, mtr);
	} else {
		lower_page_no = buf_frame_get_page_no(page);
		upper_page_no = buf_frame_get_page_no(new_page);
		lower_page = page;
		upper_page = new_page;
	}
				   
	/* Create a memory heap where the data tuple is stored */
	heap = mem_heap_create(100);

	/* Get the level of the split pages */
	level = btr_page_get_level(page, mtr);

	/* Build the node pointer (= node key and page address) for the upper
	half */

	node_ptr_upper = dict_tree_build_node_ptr(tree, split_rec,
					     upper_page_no, heap, level);

	/* Insert it next to the pointer to the lower half. Note that this
	may generate recursion leading to a split on the higher level. */

	btr_insert_on_non_leaf_level(tree, level + 1, node_ptr_upper, mtr);
		
	/* Free the memory heap */
	mem_heap_free(heap);

	/* Get the previous and next pages of page */

	prev_page_no = btr_page_get_prev(page, mtr);
	next_page_no = btr_page_get_next(page, mtr);
	space = buf_frame_get_space_id(page);
	
	/* Update page links of the level */
	
	if (prev_page_no != FIL_NULL) {

		prev_page = btr_page_get(space, prev_page_no, RW_X_LATCH, mtr);

		btr_page_set_next(prev_page, lower_page_no, mtr);
	}

	if (next_page_no != FIL_NULL) {

		next_page = btr_page_get(space, next_page_no, RW_X_LATCH, mtr);

		btr_page_set_prev(next_page, upper_page_no, mtr);
	}
	
	btr_page_set_prev(lower_page, prev_page_no, mtr);
	btr_page_set_next(lower_page, upper_page_no, mtr);
	btr_page_set_level(lower_page, level, mtr);

	btr_page_set_prev(upper_page, lower_page_no, mtr);
	btr_page_set_next(upper_page, next_page_no, mtr);
	btr_page_set_level(upper_page, level, mtr);
}

/*****************************************************************
Splits an index page to halves and inserts the tuple. It is assumed
that mtr holds an x-latch to the index tree. NOTE: the tree x-latch
is released within this function! NOTE that the operation of this
function must always succeed, we cannot reverse it: therefore
enough free disk space must be guaranteed to be available before
this function is called. */

rec_t*
btr_page_split_and_insert(
/*======================*/
				/* out: inserted record; NOTE: the tree
				x-latch is released! NOTE: 2 free disk
				pages must be available! */
	btr_cur_t*	cursor,	/* in: cursor at which to insert; when the
				function returns, the cursor is positioned
				on the predecessor of the inserted record */
	dtuple_t*	tuple,	/* in: tuple to insert */
	mtr_t*		mtr)	/* in: mtr */
{
	dict_tree_t*	tree;
	page_t*		page;
	ulint		page_no;
	byte		direction;
	ulint		hint_page_no;
	page_t*		new_page;
	rec_t*		split_rec;
	page_t*		left_page;
	page_t*		right_page;
	page_t*		insert_page;
	page_cur_t*	page_cursor;
	rec_t*		first_rec;
	byte*		buf = 0; /* remove warning */
	rec_t*		move_limit;
	ibool		insert_will_fit;
	ulint		n_iterations = 0;
	rec_t*		rec;
func_start:
	tree = btr_cur_get_tree(cursor);
	
	ut_ad(mtr_memo_contains(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK));
	ut_ad(rw_lock_own(dict_tree_get_lock(tree), RW_LOCK_EX));

	page = btr_cur_get_page(cursor);

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	ut_ad(page_get_n_recs(page) >= 2);
	
	page_no = buf_frame_get_page_no(page);

	/* 1. Decide the split record; split_rec == NULL means that the
	tuple to be inserted should be the first record on the upper
	half-page */

	if (n_iterations > 0) {
		direction = FSP_UP;
		hint_page_no = page_no + 1;
		split_rec = btr_page_get_sure_split_rec(cursor, tuple);
		
	} else if (btr_page_get_split_rec_to_right(cursor, &split_rec)) {
		direction = FSP_UP;
		hint_page_no = page_no + 1;

	} else if (btr_page_get_split_rec_to_left(cursor, &split_rec)) {
		direction = FSP_DOWN;
		hint_page_no = page_no - 1;
	} else {
		direction = FSP_UP;
		hint_page_no = page_no + 1;
		split_rec = page_get_middle_rec(page);
	}

	/* 2. Allocate a new page to the tree */
	new_page = btr_page_alloc(tree, hint_page_no, direction,
					btr_page_get_level(page, mtr), mtr);
	btr_page_create(new_page, tree, mtr);
	
	/* 3. Calculate the first record on the upper half-page, and the
	first record (move_limit) on original page which ends up on the
	upper half */

	if (split_rec != NULL) {
		first_rec = split_rec;
		move_limit = split_rec;
	} else {
		buf = mem_alloc(rec_get_converted_size(tuple));

		first_rec = rec_convert_dtuple_to_rec(buf, tuple);
		move_limit = page_rec_get_next(btr_cur_get_rec(cursor));
	}
	
	/* 4. Do first the modifications in the tree structure */

	btr_attach_half_pages(tree, page, first_rec, new_page, direction, mtr);

	if (split_rec == NULL) {
		mem_free(buf);
	}

	/* If the split is made on the leaf level and the insert will fit
	on the appropriate half-page, we may release the tree x-latch.
	We can then move the records after releasing the tree latch,
	thus reducing the tree latch contention. */

	insert_will_fit = btr_page_insert_fits(cursor, split_rec, tuple);
	
	if (insert_will_fit && (btr_page_get_level(page, mtr) == 0)) {

		mtr_memo_release(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK);
	}

	/* 5. Move then the records to the new page */
	if (direction == FSP_DOWN) {
/*		printf("Split left\n"); */

		page_move_rec_list_start(new_page, page, move_limit, mtr);
		left_page = new_page;
		right_page = page;

		lock_update_split_left(right_page, left_page);
	} else {
/*		printf("Split right\n"); */

		page_move_rec_list_end(new_page, page, move_limit, mtr);
		left_page = page;
		right_page = new_page;

		lock_update_split_right(right_page, left_page);
	}

	/* 6. The split and the tree modification is now completed. Decide the
	page where the tuple should be inserted */

	if (split_rec == NULL) {
		insert_page = right_page;

	} else if (cmp_dtuple_rec(tuple, first_rec) >= 0) {

		insert_page = right_page;
	} else {
		insert_page = left_page;
	}

	/* 7. Reposition the cursor for insert and try insertion */
	page_cursor = btr_cur_get_page_cur(cursor);

	page_cur_search(insert_page, tuple, PAGE_CUR_LE, page_cursor);

	rec = page_cur_tuple_insert(page_cursor, tuple, mtr);

	if (rec != NULL) {
		/* Insert fit on the page: update the free bits for the
		left and right pages in the same mtr */

		ibuf_update_free_bits_for_two_pages_low(cursor->index,
							left_page,
							right_page, mtr);
		/* printf("Split and insert done %lu %lu\n",
				buf_frame_get_page_no(left_page),
				buf_frame_get_page_no(right_page)); */
		return(rec);
	}
	
	/* 8. If insert did not fit, try page reorganization */

	btr_page_reorganize(insert_page, mtr);

	page_cur_search(insert_page, tuple, PAGE_CUR_LE, page_cursor);
	rec = page_cur_tuple_insert(page_cursor, tuple, mtr);

	if (rec == NULL) {
		/* The insert did not fit on the page: loop back to the
		start of the function for a new split */
		
		/* We play safe and reset the free bits for new_page */
		ibuf_reset_free_bits(cursor->index, new_page);

		/* printf("Split second round %lu\n",
					buf_frame_get_page_no(page)); */
		n_iterations++;
		ut_ad(n_iterations < 2);
		ut_ad(!insert_will_fit);

		goto func_start;
	}

	/* Insert fit on the page: update the free bits for the
	left and right pages in the same mtr */

	ibuf_update_free_bits_for_two_pages_low(cursor->index, left_page,
							right_page, mtr);
	/* printf("Split and insert done %lu %lu\n",
				buf_frame_get_page_no(left_page),
				buf_frame_get_page_no(right_page)); */

	ut_ad(page_validate(left_page, UT_LIST_GET_FIRST(tree->tree_indexes)));
	ut_ad(page_validate(right_page, UT_LIST_GET_FIRST(tree->tree_indexes)));

	return(rec);
}

/*****************************************************************
Removes a page from the level list of pages. */
static
void
btr_level_list_remove(
/*==================*/
	dict_tree_t*	tree __attribute__((unused)), /* in: index tree */
	page_t*		page,	/* in: page to remove */
	mtr_t*		mtr)	/* in: mtr */
{	
	ulint	space;
	ulint	prev_page_no;
	page_t*	prev_page;
	ulint	next_page_no;
	page_t*	next_page;
	
	ut_ad(tree && page && mtr);
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	/* Get the previous and next page numbers of page */

	prev_page_no = btr_page_get_prev(page, mtr);
	next_page_no = btr_page_get_next(page, mtr);
	space = buf_frame_get_space_id(page);
	
	/* Update page links of the level */
	
	if (prev_page_no != FIL_NULL) {

		prev_page = btr_page_get(space, prev_page_no, RW_X_LATCH, mtr);

		btr_page_set_next(prev_page, next_page_no, mtr);
	}

	if (next_page_no != FIL_NULL) {

		next_page = btr_page_get(space, next_page_no, RW_X_LATCH, mtr);

		btr_page_set_prev(next_page, prev_page_no, mtr);
	}
}
	
/********************************************************************
Writes the redo log record for setting an index record as the predefined
minimum record. */
UNIV_INLINE
void
btr_set_min_rec_mark_log(
/*=====================*/
	rec_t*	rec,	/* in: record */
	mtr_t*	mtr)	/* in: mtr */
{
	mlog_write_initial_log_record(rec, MLOG_REC_MIN_MARK, mtr);

	/* Write rec offset as a 2-byte ulint */
	mlog_catenate_ulint(mtr, rec - buf_frame_align(rec), MLOG_2BYTES);
}

/********************************************************************
Parses the redo log record for setting an index record as the predefined
minimum record. */

byte*
btr_parse_set_min_rec_mark(
/*=======================*/
			/* out: end of log record or NULL */
	byte*	ptr,	/* in: buffer */
	byte*	end_ptr,/* in: buffer end */
	page_t*	page,	/* in: page or NULL */
	mtr_t*	mtr)	/* in: mtr or NULL */
{
	rec_t*	rec;

	if (end_ptr < ptr + 2) {

		return(NULL);
	}

	if (page) {
		rec = page + mach_read_from_2(ptr);

		btr_set_min_rec_mark(rec, mtr);
	}

	return(ptr + 2);
}

/********************************************************************
Sets a record as the predefined minimum record. */

void
btr_set_min_rec_mark(
/*=================*/
	rec_t*	rec,	/* in: record */
	mtr_t*	mtr)	/* in: mtr */
{
	ulint	info_bits;

	info_bits = rec_get_info_bits(rec);

	rec_set_info_bits(rec, info_bits | REC_INFO_MIN_REC_FLAG);

	btr_set_min_rec_mark_log(rec, mtr);
}

/*****************************************************************
Deletes on the upper level the node pointer to a page. */

void
btr_node_ptr_delete(
/*================*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page whose node pointer is deleted */
	mtr_t*		mtr)	/* in: mtr */
{
	rec_t*		node_ptr;
	btr_cur_t	cursor;
	ibool		compressed;
	ulint		err;
	
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	/* Delete node pointer on father page */

	node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);

	btr_cur_position(UT_LIST_GET_FIRST(tree->tree_indexes), node_ptr,
								&cursor);
	compressed = btr_cur_pessimistic_delete(&err, TRUE, &cursor, FALSE,
									mtr);
	ut_a(err == DB_SUCCESS);

	if (!compressed) {
		btr_cur_compress_if_useful(&cursor, mtr);
	}
}

/*****************************************************************
If page is the only on its level, this function moves its records to the
father page, thus reducing the tree height. */
static
void
btr_lift_page_up(
/*=============*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page which is the only on its level;
				must not be empty: use
				btr_discard_only_page_on_level if the last
				record from the page should be removed */
	mtr_t*		mtr)	/* in: mtr */
{
	rec_t*	node_ptr;
	page_t*	father_page;
	ulint	page_level;
	
	ut_ad(btr_page_get_prev(page, mtr) == FIL_NULL);
	ut_ad(btr_page_get_next(page, mtr) == FIL_NULL);
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
			      				MTR_MEMO_PAGE_X_FIX));
	node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);
	father_page = buf_frame_align(node_ptr);
	
	page_level = btr_page_get_level(page, mtr);

	btr_search_drop_page_hash_index(page);
	
	/* Make the father empty */
	btr_page_empty(father_page, mtr);

	/* Move records to the father */
 	page_copy_rec_list_end(father_page, page, page_get_infimum_rec(page),
									mtr);
	lock_update_copy_and_discard(father_page, page);

	btr_page_set_level(father_page, page_level, mtr);

	/* Free the file page */
	btr_page_free(tree, page, mtr);		

	/* We play safe and reset the free bits for the father */
	ibuf_reset_free_bits(UT_LIST_GET_FIRST(tree->tree_indexes),
								father_page);
	ut_ad(page_validate(father_page,
				UT_LIST_GET_FIRST(tree->tree_indexes)));
	ut_ad(btr_check_node_ptr(tree, father_page, mtr));
}	

/*****************************************************************
Tries to merge the page first to the left immediate brother if such a
brother exists, and the node pointers to the current page and to the brother
reside on the same page. If the left brother does not satisfy these
conditions, looks at the right brother. If the page is the only one on that
level lifts the records of the page to the father page, thus reducing the
tree height. It is assumed that mtr holds an x-latch on the tree and on the
page. If cursor is on the leaf level, mtr must also hold x-latches to the
brothers, if they exist. NOTE: it is assumed that the caller has reserved
enough free extents so that the compression will always succeed if done! */

void
btr_compress(
/*=========*/
	btr_cur_t*	cursor,	/* in: cursor on the page to merge or lift;
				the page must not be empty: in record delete
				use btr_discard_page if the page would become
				empty */
	mtr_t*		mtr)	/* in: mtr */
{
	dict_tree_t*	tree;
	ulint		space;
	ulint		left_page_no;
	ulint		right_page_no;
	page_t*		merge_page;
	page_t*		father_page;
	ibool		is_left;
	page_t*		page;
	rec_t*		orig_pred;
	rec_t*		orig_succ;
	rec_t*		node_ptr;
	ulint		data_size;
	ulint		n_recs;
	ulint		max_ins_size;
	ulint		max_ins_size_reorg;
	ulint		level;
	
	page = btr_cur_get_page(cursor);
	tree = btr_cur_get_tree(cursor);

	ut_ad(mtr_memo_contains(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	level = btr_page_get_level(page, mtr);
	space = dict_tree_get_space(tree);

	left_page_no = btr_page_get_prev(page, mtr);
	right_page_no = btr_page_get_next(page, mtr);

/*	printf("Merge left page %lu right %lu \n", left_page_no,
							right_page_no); */

	node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);
	father_page = buf_frame_align(node_ptr);

	/* Decide the page to which we try to merge and which will inherit
	the locks */

	if (left_page_no != FIL_NULL) {

		is_left = TRUE;
		merge_page = btr_page_get(space, left_page_no, RW_X_LATCH,
									mtr);
	} else if (right_page_no != FIL_NULL) {

		is_left = FALSE;
		merge_page = btr_page_get(space, right_page_no, RW_X_LATCH,
									mtr);
	} else {
		/* The page is the only one on the level, lift the records
		to the father */
		btr_lift_page_up(tree, page, mtr);

		return;
	}
	
	n_recs = page_get_n_recs(page);
	data_size = page_get_data_size(page);

	max_ins_size_reorg = page_get_max_insert_size_after_reorganize(
							merge_page, n_recs);
	if (data_size > max_ins_size_reorg) {

		/* No space for merge */

		return;
	}

	ut_ad(page_validate(merge_page, cursor->index));

	max_ins_size = page_get_max_insert_size(merge_page, n_recs);
	
	if (data_size > max_ins_size) {

		/* We have to reorganize merge_page */

		btr_page_reorganize(merge_page, mtr);

		max_ins_size = page_get_max_insert_size(merge_page, n_recs);

		ut_ad(page_validate(merge_page, cursor->index));
		ut_ad(page_get_max_insert_size(merge_page, n_recs)
							== max_ins_size_reorg);
	}

	if (data_size > max_ins_size) {

		/* Add fault tolerance, though this should never happen */

		return;
	}

	btr_search_drop_page_hash_index(page);

	/* Remove the page from the level list */
	btr_level_list_remove(tree, page, mtr);

	if (is_left) {
		btr_node_ptr_delete(tree, page, mtr);
	} else {
		/* Replace the address of the old child node (= page) with the 
		address of the merge page to the right */

		btr_node_ptr_set_child_page_no(node_ptr, right_page_no, mtr);

		btr_node_ptr_delete(tree, merge_page, mtr);
	}
	
	/* Move records to the merge page */
	if (is_left) {
		orig_pred = page_rec_get_prev(
					page_get_supremum_rec(merge_page));
		page_copy_rec_list_start(merge_page, page,
					page_get_supremum_rec(page), mtr);

		lock_update_merge_left(merge_page, orig_pred, page);
	} else {
		orig_succ = page_rec_get_next(
					page_get_infimum_rec(merge_page));
		page_copy_rec_list_end(merge_page, page,
					page_get_infimum_rec(page), mtr);

		lock_update_merge_right(orig_succ, page);
	}

	/* We have added new records to merge_page: update its free bits */
	ibuf_update_free_bits_if_full(cursor->index, merge_page,
					UNIV_PAGE_SIZE, ULINT_UNDEFINED);
					
	ut_ad(page_validate(merge_page, cursor->index));

	/* Free the file page */
	btr_page_free(tree, page, mtr);		

	ut_ad(btr_check_node_ptr(tree, merge_page, mtr));
}	

/*****************************************************************
Discards a page that is the only page on its level. */
static
void
btr_discard_only_page_on_level(
/*===========================*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: page which is the only on its level */
	mtr_t*		mtr)	/* in: mtr */
{
	rec_t*	node_ptr;
	page_t*	father_page;
	ulint	page_level;
	
	ut_ad(btr_page_get_prev(page, mtr) == FIL_NULL);
	ut_ad(btr_page_get_next(page, mtr) == FIL_NULL);
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	btr_search_drop_page_hash_index(page);

	node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);
	father_page = buf_frame_align(node_ptr);

	page_level = btr_page_get_level(page, mtr);

	lock_update_discard(page_get_supremum_rec(father_page), page);

	btr_page_set_level(father_page, page_level, mtr);

	/* Free the file page */
	btr_page_free(tree, page, mtr);		

	if (buf_frame_get_page_no(father_page) == dict_tree_get_page(tree)) {
		/* The father is the root page */

		btr_page_empty(father_page, mtr);

		/* We play safe and reset the free bits for the father */
		ibuf_reset_free_bits(UT_LIST_GET_FIRST(tree->tree_indexes),
								father_page);
	} else {
		ut_ad(page_get_n_recs(father_page) == 1);

		btr_discard_only_page_on_level(tree, father_page, mtr);
	}
}	

/*****************************************************************
Discards a page from a B-tree. This is used to remove the last record from
a B-tree page: the whole page must be removed at the same time. This cannot
be used for the root page, which is allowed to be empty. */

void
btr_discard_page(
/*=============*/
	btr_cur_t*	cursor,	/* in: cursor on the page to discard: not on
				the root page */
	mtr_t*		mtr)	/* in: mtr */
{
	dict_tree_t*	tree;
	ulint		space;
	ulint		left_page_no;
	ulint		right_page_no;
	page_t*		merge_page;
	ibool		is_left;
	page_t*		page;
	rec_t*		node_ptr;
	
	page = btr_cur_get_page(cursor);
	tree = btr_cur_get_tree(cursor);

	ut_ad(dict_tree_get_page(tree) != buf_frame_get_page_no(page));
	ut_ad(mtr_memo_contains(mtr, dict_tree_get_lock(tree),
							MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	space = dict_tree_get_space(tree);
	
	/* Decide the page which will inherit the locks */

	left_page_no = btr_page_get_prev(page, mtr);
	right_page_no = btr_page_get_next(page, mtr);

	if (left_page_no != FIL_NULL) {
		is_left = TRUE;
		merge_page = btr_page_get(space, left_page_no, RW_X_LATCH,
									mtr);
	} else if (right_page_no != FIL_NULL) {
		is_left = FALSE;
		merge_page = btr_page_get(space, right_page_no, RW_X_LATCH,
									mtr);
	} else {
		btr_discard_only_page_on_level(tree, page, mtr);

		return;
	}

	btr_search_drop_page_hash_index(page);
	
	if (left_page_no == FIL_NULL && btr_page_get_level(page, mtr) > 0) {

		/* We have to mark the leftmost node pointer on the right
		side page as the predefined minimum record */

		node_ptr = page_rec_get_next(page_get_infimum_rec(merge_page));

		ut_ad(node_ptr != page_get_supremum_rec(merge_page));

		btr_set_min_rec_mark(node_ptr, mtr);
	}	
	
	btr_node_ptr_delete(tree, page, mtr);

	/* Remove the page from the level list */
	btr_level_list_remove(tree, page, mtr);

	if (is_left) {
		lock_update_discard(page_get_supremum_rec(merge_page), page);
	} else {
		lock_update_discard(page_rec_get_next(
				    page_get_infimum_rec(merge_page)), page);
	}

	/* Free the file page */
	btr_page_free(tree, page, mtr);		

	ut_ad(btr_check_node_ptr(tree, merge_page, mtr));
}	

/*****************************************************************
Prints size info of a B-tree. */

void
btr_print_size(
/*===========*/
	dict_tree_t*	tree)	/* in: index tree */
{
	page_t*		root;
	fseg_header_t*	seg;
	mtr_t		mtr;

	if (tree->type & DICT_IBUF) {
		printf(
	"Sorry, cannot print info of an ibuf tree: use ibuf functions\n");

		return;
	}

	mtr_start(&mtr);
	
	root = btr_root_get(tree, &mtr);

	seg = root + PAGE_HEADER + PAGE_BTR_SEG_TOP;

	printf("INFO OF THE NON-LEAF PAGE SEGMENT\n");
	fseg_print(seg, &mtr);

	if (!(tree->type & DICT_UNIVERSAL)) {

		seg = root + PAGE_HEADER + PAGE_BTR_SEG_LEAF;

		printf("INFO OF THE LEAF PAGE SEGMENT\n");
		fseg_print(seg, &mtr);
	}

	mtr_commit(&mtr); 	
}

/****************************************************************
Prints recursively index tree pages. */
static
void
btr_print_recursive(
/*================*/
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: index page */
	ulint		width,	/* in: print this many entries from start
				and end */
	mtr_t*		mtr)	/* in: mtr */
{
	page_cur_t	cursor;
	ulint		n_recs;
	ulint		i	= 0;
	mtr_t		mtr2;
	rec_t*		node_ptr;
	page_t*		child;
	
	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	printf("NODE ON LEVEL %lu page number %lu\n",
		btr_page_get_level(page, mtr), buf_frame_get_page_no(page));
	
	page_print(page, width, width);
	
	n_recs = page_get_n_recs(page);
	
	page_cur_set_before_first(page, &cursor);
	page_cur_move_to_next(&cursor);

	while (!page_cur_is_after_last(&cursor)) {

		if (0 == btr_page_get_level(page, mtr)) {

			/* If this is the leaf level, do nothing */

		} else if ((i <= width) || (i >= n_recs - width)) {

			mtr_start(&mtr2);

			node_ptr = page_cur_get_rec(&cursor);

			child = btr_node_ptr_get_child(node_ptr, &mtr2);

			btr_print_recursive(tree, child, width, &mtr2);
			mtr_commit(&mtr2);
		}

		page_cur_move_to_next(&cursor);
		i++;
	}
}

/******************************************************************
Prints directories and other info of all nodes in the tree. */

void
btr_print_tree(
/*===========*/
	dict_tree_t*	tree,	/* in: tree */
	ulint		width)	/* in: print this many entries from start
				and end */
{
	mtr_t	mtr;
	page_t*	root;

	printf("--------------------------\n");
	printf("INDEX TREE PRINT\n");

	mtr_start(&mtr);

	root = btr_root_get(tree, &mtr);

	btr_print_recursive(tree, root, width, &mtr);

	mtr_commit(&mtr);

	btr_validate_tree(tree);
}

/****************************************************************
Checks that the node pointer to a page is appropriate. */

ibool
btr_check_node_ptr(
/*===============*/
				/* out: TRUE */
	dict_tree_t*	tree,	/* in: index tree */
	page_t*		page,	/* in: index page */
	mtr_t*		mtr)	/* in: mtr */
{
	mem_heap_t*	heap;
	rec_t*		node_ptr;
	dtuple_t*	node_ptr_tuple;

	ut_ad(mtr_memo_contains(mtr, buf_block_align(page),
							MTR_MEMO_PAGE_X_FIX));
	if (dict_tree_get_page(tree) == buf_frame_get_page_no(page)) {

		return(TRUE);
	}

	node_ptr = btr_page_get_father_node_ptr(tree, page, mtr);
 
	if (btr_page_get_level(page, mtr) == 0) {

		return(TRUE);
	}
	
	heap = mem_heap_create(256);
		
	node_ptr_tuple = dict_tree_build_node_ptr(
				tree,
				page_rec_get_next(page_get_infimum_rec(page)),
				0, heap, btr_page_get_level(page, mtr));
				
	ut_a(cmp_dtuple_rec(node_ptr_tuple, node_ptr) == 0);

	mem_heap_free(heap);

	return(TRUE);
}

/****************************************************************
Checks the size and number of fields in a record based on the definition of
the index. */

ibool
btr_index_rec_validate(
/*====================*/
					/* out: TRUE if ok */
	rec_t*		rec,		/* in: index record */
	dict_index_t*	index,		/* in: index */
	ibool		dump_on_error)	/* in: TRUE if the function
					should print hex dump of record
					and page on error */
{
	dtype_t* type;
	byte*	data;
	ulint	len;
	ulint	n;
	ulint	i;
	page_t*	page;
	char	err_buf[1000];

	page = buf_frame_align(rec);
	
	if (index->type & DICT_UNIVERSAL) {
	        /* The insert buffer index tree can contain records from any
	        other index: we cannot check the number of fields or
	        their length */

	        return(TRUE);
	}

	n = dict_index_get_n_fields(index);

	if (rec_get_n_fields(rec) != n) {
		fprintf(stderr,
"InnoDB: Record in index %s in table %s, page %lu, at offset %lu\n"
"InnoDB: has %lu fields, should have %lu\n",
			index->name, index->table_name,
			buf_frame_get_page_no(page), (ulint)(rec - page),
			rec_get_n_fields(rec), n);

		if (!dump_on_error) {

			return(FALSE);
		}

		buf_page_print(page);

		rec_sprintf(err_buf, 900, rec);
	  	fprintf(stderr, "InnoDB: corrupt record %s\n", err_buf);

		return(FALSE);
	}

	for (i = 0; i < n; i++) {
		data = rec_get_nth_field(rec, i, &len);

		type = dict_index_get_nth_type(index, i);
		
		if ((dict_index_get_nth_field(index, i)->prefix_len == 0
		    && len != UNIV_SQL_NULL && dtype_is_fixed_size(type)
		    && len != dtype_get_fixed_size(type))
		   ||
		   (dict_index_get_nth_field(index, i)->prefix_len > 0
		    && len != UNIV_SQL_NULL && dtype_is_fixed_size(type)
		    && len !=
			   dict_index_get_nth_field(index, i)->prefix_len)) {

			fprintf(stderr,
"InnoDB: Record in index %s in table %s, page %lu, at offset %lu\n"
"InnoDB: field %lu len is %lu, should be %lu\n",
				index->name, index->table_name,
				buf_frame_get_page_no(page),
				(ulint)(rec - page),
				i, len, dtype_get_fixed_size(type));

			if (!dump_on_error) {
	
				return(FALSE);
			}

			buf_page_print(page);

			rec_sprintf(err_buf, 900, rec);
	  		fprintf(stderr,
                             "InnoDB: corrupt record %s\n", err_buf);

			return(FALSE);
		}
	}

	return(TRUE);			
}

/****************************************************************
Checks the size and number of fields in records based on the definition of
the index. */
static
ibool
btr_index_page_validate(
/*====================*/
				/* out: TRUE if ok */
	page_t*		page,	/* in: index page */
	dict_index_t*	index)	/* in: index */
{
	rec_t*		rec;
	page_cur_t 	cur;
	ibool		ret	= TRUE;
	
	page_cur_set_before_first(page, &cur);
	page_cur_move_to_next(&cur);

	for (;;) {
		rec = (&cur)->rec;

		if (page_cur_is_after_last(&cur)) {

			break;
		}

		if (!btr_index_rec_validate(rec, index, TRUE)) {

			return(FALSE);
		}

		page_cur_move_to_next(&cur);
	}

	return(ret);	
}

/****************************************************************
Validates index tree level. */
static
ibool
btr_validate_level(
/*===============*/
				/* out: TRUE if ok */
	dict_tree_t*	tree,	/* in: index tree */
	ulint		level)	/* in: level number */
{
	ulint		space;
	page_t*		page;
	page_t*		right_page = 0; /* remove warning */
	page_t*		father_page;
	page_t*		right_father_page;
	rec_t*		node_ptr;
	rec_t*		right_node_ptr;
	ulint		right_page_no;
	ulint		left_page_no;
	page_cur_t	cursor;
	mem_heap_t*	heap;
	dtuple_t*	node_ptr_tuple;
	ibool		ret	= TRUE;
	dict_index_t*	index;
	mtr_t		mtr;
	char		err_buf[1000];
	
	mtr_start(&mtr);

	mtr_x_lock(dict_tree_get_lock(tree), &mtr);
	
	page = btr_root_get(tree, &mtr);

	space = buf_frame_get_space_id(page);

	while (level != btr_page_get_level(page, &mtr)) {

		ut_a(btr_page_get_level(page, &mtr) > 0);

		page_cur_set_before_first(page, &cursor);
		page_cur_move_to_next(&cursor);

		node_ptr = page_cur_get_rec(&cursor);
		page = btr_node_ptr_get_child(node_ptr, &mtr);
	}

	index = UT_LIST_GET_FIRST(tree->tree_indexes);
	
	/* Now we are on the desired level. Loop through the pages on that
	level. */
loop:
	mtr_x_lock(dict_tree_get_lock(tree), &mtr);

	/* Check ordering etc. of records */

	if (!page_validate(page, index)) {
		fprintf(stderr,
"InnoDB: Error in page %lu in index %s table %s, index tree level %lu\n",
			buf_frame_get_page_no(page), index->name,
			index->table_name, level);

		ret = FALSE;
	} else if (level == 0) {
		/* We are on level 0. Check that the records have the right
		number of fields, and field lengths are right. */

		if (!btr_index_page_validate(page, index)) {

			ret = FALSE;
		}
	}
	
	ut_a(btr_page_get_level(page, &mtr) == level);

	right_page_no = btr_page_get_next(page, &mtr);
	left_page_no = btr_page_get_prev(page, &mtr);

	ut_a((page_get_n_recs(page) > 0)
	     || ((level == 0) &&
		  (buf_frame_get_page_no(page) == dict_tree_get_page(tree))));

	if (right_page_no != FIL_NULL) {

		right_page = btr_page_get(space, right_page_no, RW_X_LATCH,
									&mtr);
		if (cmp_rec_rec(page_rec_get_prev(page_get_supremum_rec(page)),
			page_rec_get_next(page_get_infimum_rec(right_page)),
			UT_LIST_GET_FIRST(tree->tree_indexes)) >= 0) {

 			fprintf(stderr,
		"InnoDB: Error on pages %lu and %lu in index %s table %s\n",
				buf_frame_get_page_no(page),
				right_page_no,
				index->name, index->table_name);

			fprintf(stderr,
			"InnoDB: records in wrong order on adjacent pages\n");

			buf_page_print(page);
			buf_page_print(right_page);

			rec_sprintf(err_buf, 900,
				page_rec_get_prev(page_get_supremum_rec(page)));
	  		fprintf(stderr, "InnoDB: record %s\n", err_buf);

			rec_sprintf(err_buf, 900,
			page_rec_get_next(page_get_infimum_rec(right_page)));
	  		fprintf(stderr, "InnoDB: record %s\n", err_buf);

	  		ret = FALSE;
	  	}
	}
	
	if (level > 0 && left_page_no == FIL_NULL) {
		ut_a(REC_INFO_MIN_REC_FLAG & rec_get_info_bits(
			page_rec_get_next(page_get_infimum_rec(page))));
	}

	if (buf_frame_get_page_no(page) != dict_tree_get_page(tree)) {

		/* Check father node pointers */
	
		node_ptr = btr_page_get_father_node_ptr(tree, page, &mtr);
		father_page = buf_frame_align(node_ptr);

		if (btr_node_ptr_get_child_page_no(node_ptr) !=
						buf_frame_get_page_no(page)
		   || node_ptr != btr_page_get_father_for_rec(tree, page,
		   	page_rec_get_prev(page_get_supremum_rec(page)),
								&mtr)) {
 			fprintf(stderr,
			"InnoDB: Error on page %lu in index %s table %s\n",
				buf_frame_get_page_no(page),
				index->name, index->table_name);

			fprintf(stderr,
			"InnoDB: node pointer to the page is wrong\n");

			buf_page_print(father_page);
			buf_page_print(page);

			rec_sprintf(err_buf, 900, node_ptr);
				
	  		fprintf(stderr, "InnoDB: node ptr %s\n", err_buf);

			fprintf(stderr,
				"InnoDB: node ptr child page n:o %lu\n",
				btr_node_ptr_get_child_page_no(node_ptr));

			rec_sprintf(err_buf, 900,
			 	btr_page_get_father_for_rec(tree, page,
		   	 	page_rec_get_prev(page_get_supremum_rec(page)),
					&mtr));

	  		fprintf(stderr, "InnoDB: record on page %s\n",
								err_buf);
		   	ret = FALSE;

		   	goto node_ptr_fails;
		}

		if (btr_page_get_level(page, &mtr) > 0) {
			heap = mem_heap_create(256);
		
			node_ptr_tuple = dict_tree_build_node_ptr(
					tree,
					page_rec_get_next(
						page_get_infimum_rec(page)),
						0, heap,
       					btr_page_get_level(page, &mtr));

			if (cmp_dtuple_rec(node_ptr_tuple, node_ptr) != 0) {

	 			fprintf(stderr,
			"InnoDB: Error on page %lu in index %s table %s\n",
					buf_frame_get_page_no(page),
					index->name, index->table_name);

				buf_page_print(father_page);
				buf_page_print(page);

	  			fprintf(stderr,
                	"InnoDB: Error: node ptrs differ on levels > 0\n");
							
				rec_sprintf(err_buf, 900, node_ptr);
				
	  			fprintf(stderr, "InnoDB: node ptr %s\n",
								err_buf);
				rec_sprintf(err_buf, 900,
				  page_rec_get_next(
					page_get_infimum_rec(page)));
				
	  			fprintf(stderr, "InnoDB: first rec %s\n",
								err_buf);
		   		ret = FALSE;
				mem_heap_free(heap);

		   		goto node_ptr_fails;
			}

			mem_heap_free(heap);
		}

		if (left_page_no == FIL_NULL) {
			ut_a(node_ptr == page_rec_get_next(
					page_get_infimum_rec(father_page)));
			ut_a(btr_page_get_prev(father_page, &mtr) == FIL_NULL);
		}

		if (right_page_no == FIL_NULL) {
			ut_a(node_ptr == page_rec_get_prev(
					page_get_supremum_rec(father_page)));
			ut_a(btr_page_get_next(father_page, &mtr) == FIL_NULL);
		}

		if (right_page_no != FIL_NULL) {

			right_node_ptr = btr_page_get_father_node_ptr(tree,
							right_page, &mtr);
			if (page_rec_get_next(node_ptr) !=
					page_get_supremum_rec(father_page)) {

				if (right_node_ptr !=
						page_rec_get_next(node_ptr)) {
					ret = FALSE;
					fprintf(stderr,
			"InnoDB: node pointer to the right page is wrong\n");

	 				fprintf(stderr,
			"InnoDB: Error on page %lu in index %s table %s\n",
					buf_frame_get_page_no(page),
					index->name, index->table_name);

					buf_page_print(father_page);
					buf_page_print(page);
					buf_page_print(right_page);
				}
			} else {
				right_father_page = buf_frame_align(
							right_node_ptr);
							
				if (right_node_ptr != page_rec_get_next(
					   		page_get_infimum_rec(
							right_father_page))) {
					ret = FALSE;
					fprintf(stderr,
			"InnoDB: node pointer 2 to the right page is wrong\n");

	 				fprintf(stderr,
			"InnoDB: Error on page %lu in index %s table %s\n",
					buf_frame_get_page_no(page),
					index->name, index->table_name);

					buf_page_print(father_page);
					buf_page_print(right_father_page);
					buf_page_print(page);
					buf_page_print(right_page);
				}

				if (buf_frame_get_page_no(right_father_page)
				   != btr_page_get_next(father_page, &mtr)) {

					ret = FALSE;
					fprintf(stderr,
			"InnoDB: node pointer 3 to the right page is wrong\n");

	 				fprintf(stderr,
			"InnoDB: Error on page %lu in index %s table %s\n",
					buf_frame_get_page_no(page),
					index->name, index->table_name);

					buf_page_print(father_page);
					buf_page_print(right_father_page);
					buf_page_print(page);
					buf_page_print(right_page);
				}
			}					
		}
	}

node_ptr_fails:
	mtr_commit(&mtr);

	if (right_page_no != FIL_NULL) {
		mtr_start(&mtr);
	
		page = btr_page_get(space, right_page_no, RW_X_LATCH, &mtr);

		goto loop;
	}

	return(ret);
}

/******************************************************************
Checks the consistency of an index tree. */

ibool
btr_validate_tree(
/*==============*/
				/* out: TRUE if ok */
	dict_tree_t*	tree)	/* in: tree */
{
	mtr_t	mtr;
	page_t*	root;
	ulint	i;
	ulint	n;

	mtr_start(&mtr);
	mtr_x_lock(dict_tree_get_lock(tree), &mtr);

	root = btr_root_get(tree, &mtr);
	n = btr_page_get_level(root, &mtr);

	for (i = 0; i <= n; i++) {
		
		if (!btr_validate_level(tree, n - i)) {

			mtr_commit(&mtr);

			return(FALSE);
		}
	}

	mtr_commit(&mtr);

	return(TRUE);
}

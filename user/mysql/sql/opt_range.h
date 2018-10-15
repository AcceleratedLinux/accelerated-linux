/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

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


/* classes to use when handling where clause */

#ifndef _opt_range_h
#define _opt_range_h

#ifdef __GNUC__
#pragma interface			/* gcc class implementation */
#endif

#define NO_MIN_RANGE	1
#define NO_MAX_RANGE	2
#define NEAR_MIN	4
#define NEAR_MAX	8
#define UNIQUE_RANGE	16
#define EQ_RANGE	32
#define NULL_RANGE	64

typedef struct st_key_part {
  uint16 key,part,part_length;
  uint8  null_bit;
  Field *field;
} KEY_PART;


class QUICK_RANGE :public Sql_alloc {
 public:
  char *min_key,*max_key;
  uint16 min_length,max_length,flag;
#ifdef HAVE_purify
  uint16 dummy;					/* Avoid warnings on 'flag' */
#endif
  QUICK_RANGE();				/* Full range */
  QUICK_RANGE(const char *min_key_arg,uint min_length_arg,
	      const char *max_key_arg,uint max_length_arg,
	      uint flag_arg)
    : min_key((char*) sql_memdup(min_key_arg,min_length_arg+1)),
      max_key((char*) sql_memdup(max_key_arg,max_length_arg+1)),
      min_length((uint16) min_length_arg),
      max_length((uint16) max_length_arg),
      flag((uint16) flag_arg)
    {
#ifdef HAVE_purify
      dummy=0;
#endif
    }
};


class QUICK_SELECT {
public:
  bool next,dont_free;
  int error;
  uint index,max_used_key_length;
  TABLE *head;
  handler *file;
  byte    *record;
  List<QUICK_RANGE> ranges;
  List_iterator<QUICK_RANGE> it;
  QUICK_RANGE *range;
  MEM_ROOT alloc;

  KEY_PART *key_parts;
  ha_rows records;
  double read_time;

  QUICK_SELECT(THD *thd, TABLE *table,uint index_arg,bool no_alloc=0);
  virtual ~QUICK_SELECT();
  void reset(void) { next=0; it.rewind(); }
  int init() { return error=file->index_init(index); }
  virtual int get_next();
  virtual bool reverse_sorted() { return 0; }
  int cmp_next(QUICK_RANGE *range);
  bool unique_key_range();
};


class QUICK_SELECT_DESC: public QUICK_SELECT
{
public:
  QUICK_SELECT_DESC(QUICK_SELECT *q, uint used_key_parts);
  int get_next();
  bool reverse_sorted() { return 1; }
private:
  int cmp_prev(QUICK_RANGE *range);
  bool range_reads_after_key(QUICK_RANGE *range);
#ifdef NOT_USED
  bool test_if_null_range(QUICK_RANGE *range, uint used_key_parts);
#endif
  void reset(void) { next=0; rev_it.rewind(); }
  List<QUICK_RANGE> rev_ranges;
  List_iterator<QUICK_RANGE> rev_it;
};


class SQL_SELECT :public Sql_alloc {
 public:
  QUICK_SELECT *quick;		// If quick-select used
  COND		*cond;		// where condition
  TABLE	*head;
  IO_CACHE file;		// Positions to used records
  ha_rows records;		// Records in use if read from file
  double read_time;		// Time to read rows
  key_map quick_keys;		// Possible quick keys
  key_map needed_reg;		// Possible quick keys after prev tables.
  table_map const_tables,read_tables;
  bool	free_cond;

  SQL_SELECT();
  ~SQL_SELECT();
  bool check_quick(THD *thd, bool force_quick_range= 0,
		   ha_rows limit= HA_POS_ERROR)
  { return test_quick_select(thd, ~0L,0,limit, force_quick_range) < 0; }
  inline bool skipp_record() { return cond ? cond->val_int() == 0 : 0; }
  int test_quick_select(THD *thd, key_map keys, table_map prev_tables,
			ha_rows limit, bool force_quick_range=0);
};

QUICK_SELECT *get_quick_select_for_ref(THD *thd, TABLE *table,
				       struct st_table_ref *ref);

#endif

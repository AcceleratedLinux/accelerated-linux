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


#ifdef __GNUC__
#pragma interface			/* gcc class implementation */
#endif

/* class for the the heap handler */

#include <heap.h>

class ha_heap: public handler
{
  HP_INFO *file;

 public:
  ha_heap(TABLE *table): handler(table), file(0) {}
  ~ha_heap() {}
  const char *table_type() const { return "HEAP"; }
  const char *index_type(uint inx)
  {
    return ((table->key_info[inx].algorithm == HA_KEY_ALG_BTREE) ? "BTREE" :
	    "HASH");
  }
  const char **bas_ext() const;
  ulong table_flags() const
  {
    return (HA_READ_RND_SAME | HA_NO_INDEX | HA_KEYPOS_TO_RNDPOS |
	    HA_NO_BLOBS | HA_NULL_KEY | HA_REC_NOT_IN_SEQ |
	    HA_NO_AUTO_INCREMENT);
  }
  ulong index_flags(uint inx) const
  {
    return ((table->key_info[inx].algorithm == HA_KEY_ALG_BTREE) ?
	    (HA_READ_NEXT | HA_READ_PREV | HA_READ_ORDER) :
	    (HA_ONLY_WHOLE_INDEX | HA_WRONG_ASCII_ORDER |
	     HA_NOT_READ_PREFIX_LAST));
  }
  uint max_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_keys()          const { return MAX_KEY; }
  uint max_key_parts()     const { return MAX_REF_PARTS; }
  uint max_key_length()    const { return HA_MAX_REC_LENGTH; }
  double scan_time() { return (double) (records+deleted) / 20.0+10; }
  double read_time(uint index, uint ranges, ha_rows rows)
  { return (double) rows /  20.0+1; }
  virtual bool fast_key_read() { return 1;}

  int open(const char *name, int mode, uint test_if_locked);
  int close(void);
  int write_row(byte * buf);
  int update_row(const byte * old_data, byte * new_data);
  int delete_row(const byte * buf);
  int index_read(byte * buf, const byte * key,
		 uint key_len, enum ha_rkey_function find_flag);
  int index_read_idx(byte * buf, uint idx, const byte * key,
		     uint key_len, enum ha_rkey_function find_flag);
  int index_next(byte * buf);
  int index_prev(byte * buf);
  int index_first(byte * buf);
  int index_last(byte * buf);
  int rnd_init(bool scan=1);
  int rnd_next(byte *buf);
  int rnd_pos(byte * buf, byte *pos);
  void position(const byte *record);
  void info(uint);
  int extra(enum ha_extra_function operation);
  int reset(void);
  int external_lock(THD *thd, int lock_type);
  int delete_all_rows(void);
  ha_rows records_in_range(int inx, const byte *start_key,uint start_key_len,
			   enum ha_rkey_function start_search_flag,
			   const byte *end_key,uint end_key_len,
			   enum ha_rkey_function end_search_flag);
  int delete_table(const char *from);
  int rename_table(const char * from, const char * to);
  int create(const char *name, TABLE *form, HA_CREATE_INFO *create_info);

  THR_LOCK_DATA **store_lock(THD *thd, THR_LOCK_DATA **to,
			     enum thr_lock_type lock_type);

};

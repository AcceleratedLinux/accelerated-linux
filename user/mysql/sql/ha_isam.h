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

/* class for the the myisam handler */

#include <nisam.h>

class ha_isam: public handler
{
  N_INFO *file;
  /* We need this as table_flags() may change after open() */
  ulong int_table_flags;

 public:
  ha_isam(TABLE *table)
    :handler(table), file(0),
    int_table_flags(HA_READ_RND_SAME | HA_KEYPOS_TO_RNDPOS | HA_LASTKEY_ORDER |
		    HA_KEY_READ_WRONG_STR | HA_DUPP_POS |
		    HA_NOT_DELETE_WITH_CACHE)
  {}
  ~ha_isam() {}
  const char *table_type() const { return "ISAM"; }
  const char *index_type(uint key_number) { return "BTREE"; }
  const char **bas_ext() const;
  ulong table_flags() const { return int_table_flags; }
  uint max_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_keys()          const { return N_MAXKEY; }
  uint max_key_parts()     const { return N_MAXKEY_SEG; }
  uint max_key_length()    const { return N_MAX_KEY_LENGTH; }
  uint min_record_length(uint options) const;
  bool low_byte_first() const { return 0; }

  int open(const char *name, int mode, uint test_if_locked);
  int close(void);
  int write_row(byte * buf);
  int update_row(const byte * old_data, byte * new_data);
  int delete_row(const byte * buf);
  int index_read(byte * buf, const byte * key,
		 uint key_len, enum ha_rkey_function find_flag);
  int index_read_idx(byte * buf, uint idx, const byte * key,
		     uint key_len, enum ha_rkey_function find_flag);
  int index_read_last(byte * buf, const byte * key, uint key_len);
  int index_next(byte * buf);
  int index_prev(byte * buf);
  int index_first(byte * buf);
  int index_last(byte * buf);
  int rnd_init(bool scan=1);
  int rnd_next(byte *buf);
  int rnd_pos(byte * buf, byte *pos);
  void position(const byte *record);
  my_off_t row_position() { return nisam_position(file); }
  void info(uint);
  int extra(enum ha_extra_function operation);
  int reset(void);
  int external_lock(THD *thd, int lock_type);
  ha_rows records_in_range(int inx,
			   const byte *start_key,uint start_key_len,
			   enum ha_rkey_function start_search_flag,
			   const byte *end_key,uint end_key_len,
			   enum ha_rkey_function end_search_flag);

  int create(const char *name, TABLE *form, HA_CREATE_INFO *create_info);
  THR_LOCK_DATA **store_lock(THD *thd, THR_LOCK_DATA **to,
			     enum thr_lock_type lock_type);
};

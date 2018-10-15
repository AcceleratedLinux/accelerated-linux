/* Copyright (C) 2000 MySQL AB

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

#ifndef _my_bitmap_h_
#define _my_bitmap_h_

#include <my_pthread.h>

#define MY_BIT_NONE (~(uint) 0)

typedef struct st_bitmap
{
  uchar *bitmap;
  uint bitmap_size;
  my_bool thread_safe; /* set if several threads access the bitmap */
  /*
     mutex will be acquired for the duration of each bitmap operation if
     thread_safe flag is set. Otherwise, we optimize by not acquiring the
     mutex
   */
#ifdef THREAD
  pthread_mutex_t mutex;
#endif
} MY_BITMAP;

#ifdef	__cplusplus
extern "C" {
#endif
  extern my_bool bitmap_init(MY_BITMAP *bitmap, uint bitmap_size,
			     my_bool thread_safe);
  extern void bitmap_free(MY_BITMAP *bitmap);
  extern void bitmap_set_bit(MY_BITMAP *bitmap, uint bitmap_bit);
  extern uint bitmap_set_next(MY_BITMAP *bitmap);
  extern void bitmap_set_all(MY_BITMAP* bitmap);
  extern my_bool bitmap_is_set(MY_BITMAP* bitmap, uint bitmap_bit);
  extern void bitmap_clear_all(MY_BITMAP* bitmap);
  extern void bitmap_clear_bit(MY_BITMAP *bitmap, uint bitmap_bit);
#ifdef	__cplusplus
}
#endif

#endif /* _my_bitmap_h_ */

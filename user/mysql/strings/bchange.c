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

/*  File   : bchange.c
    Author : Michael widenius
    Updated: 1987-03-20
    Defines: bchange()

    bchange(dst, old_length, src, new_length, tot_length)
    replaces old_length characters at dst to new_length characters from
    src in a buffer with tot_length bytes.
*/

#include <my_global.h>
#include "m_string.h"

void bchange(register char *dst, uint old_length, register const char *src,
	     uint new_length, uint tot_length)
{
  uint rest=tot_length-old_length;
  if (old_length < new_length)
    bmove_upp(dst+rest+new_length,dst+tot_length,rest);
  else
    bmove(dst+new_length,dst+old_length,rest);
  memcpy(dst,src,new_length);
}

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

#include "mysys_priv.h"
#include <stdarg.h>

/*
  Malloc many pointers at the same time

  SYNOPSIS
    my_multi_malloc()
      myFlags	Flags
      ... 	Multiple arguments terminated by null ptr

	ptr, length
	ptr, length
	NULL
*/ 

gptr my_multi_malloc(myf myFlags, ...)
{
  va_list args;
  char **ptr,*start,*res;
  uint tot_length,length;
  DBUG_ENTER("my_multi_malloc");

  va_start(args,myFlags);
  tot_length=0;
  while ((ptr=va_arg(args, char **)))
  {
    length=va_arg(args,uint);
    tot_length+=ALIGN_SIZE(length);
  }
  va_end(args);

  if (!(start=(char *) my_malloc(tot_length,myFlags)))
    DBUG_RETURN(0); /* purecov: inspected */

  va_start(args,myFlags);
  res=start;
  while ((ptr=va_arg(args, char **)))
  {
    *ptr=res;
    length=va_arg(args,uint);
    res+=ALIGN_SIZE(length);
  }
  va_end(args);
  DBUG_RETURN((gptr) start);
}

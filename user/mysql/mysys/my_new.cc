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

/*
  This is a replacement of new/delete operators to be used when compiling
  with gcc 3.0.x to avoid including libstdc++
*/

#include "mysys_priv.h"

#ifdef USE_MYSYS_NEW  

void *operator new (size_t sz)
{
  return (void *) malloc (sz ? sz+1 : sz);
}

void *operator new[] (size_t sz)
{
  return (void *) malloc (sz ? sz+1 : sz);
}

void operator delete (void *ptr)
{
  if (ptr)
    free(ptr);
}

void operator delete[] (void *ptr) throw ()
{
  if (ptr)
    free(ptr);
}

#endif /* USE_MYSYS_NEW */


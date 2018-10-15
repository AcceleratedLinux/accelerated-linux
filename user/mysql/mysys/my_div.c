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

/*
  Get filename of file

  SYNOPSIS
    my_filename()
      fd	File descriptor
*/

my_string my_filename(File fd)
{
  DBUG_ENTER("my_filename");
  if (fd >= MY_NFILE)
    DBUG_RETURN((char*) "UNKNOWN");
  if (fd >= 0 && my_file_info[fd].type != UNOPEN)
  {
    DBUG_RETURN(my_file_info[fd].name);
  }
  else
    DBUG_RETURN((char*) "UNOPENED");	/* Debug message */
}

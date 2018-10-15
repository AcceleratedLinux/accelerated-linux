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
#include <m_string.h>

	/* Functions definied in this file */

uint dirname_length(const char *name)
{
  register my_string pos,gpos;
#ifdef FN_DEVCHAR
  if ((pos=(char*)strrchr(name,FN_DEVCHAR)) == 0)
#endif
    pos=(char*) name-1;

  gpos= pos++;
  for ( ; *pos ; pos++)				/* Find last FN_LIBCHAR */
    if (*pos == FN_LIBCHAR || *pos == '/'
#ifdef FN_C_AFTER_DIR
	|| *pos == FN_C_AFTER_DIR || *pos == FN_C_AFTER_DIR_2
#endif
	)
      gpos=pos;
  return ((uint) (uint) (gpos+1-(char*) name));
}


	/* Gives directory part of filename. Directory ends with '/' */
	/* Returns length of directory part */

uint dirname_part(my_string to, const char *name)
{
  uint length;
  DBUG_ENTER("dirname_part");
  DBUG_PRINT("enter",("'%s'",name));

  length=dirname_length(name);
  convert_dirname(to, name, name+length);
  DBUG_RETURN(length);
} /* dirname */


/*
  Convert directory name to use under this system

  SYNPOSIS
    convert_dirname()
    to				Store result here
    from			Original filename
    from_end			Pointer at end of filename (normally end \0)

  IMPLEMENTATION
    If MSDOS converts '/' to '\'
    If VMS converts '<' to '[' and '>' to ']'
    Adds a FN_LIBCHAR to end if the result string if there isn't one
    and the last isn't dev_char.
    Copies data from 'from' until ASCII(0) for until from == from_end
    If you want to use the whole 'from' string, just send NullS as the
    last argument.

    If the result string is larger than FN_REFLEN -1, then it's cut.

  RETURN
   Returns pointer to end \0 in to
*/

#ifndef FN_DEVCHAR
#define FN_DEVCHAR '\0'				/* For easier code */
#endif

char *convert_dirname(char *to, const char *from, const char *from_end)
{
  char *to_org=to;

  /* We use -2 here, becasue we need place for the last FN_LIBCHAR */
  if (!from_end || (from_end - from) > FN_REFLEN-2)
    from_end=from+FN_REFLEN -2;

#if FN_LIBCHAR != '/' || defined(FN_C_BEFORE_DIR_2)
  {
    for (; *from && from != from_end; from++)
    {
      if (*from == '/')
	*to++= FN_LIBCHAR;
#ifdef FN_C_BEFORE_DIR_2
      else if (*from == FN_C_BEFORE_DIR_2)
	*to++= FN_C_BEFORE_DIR;
      else if (*from == FN_C_AFTER_DIR_2)
	*to++= FN_C_AFTER_DIR;
#endif
      else
	*to++= *from;
    }
    *to=0;
  }
#else
  /* This is ok even if to == from, becasue we need to cut the string */
  to= strmake(to, from, (uint) (from_end-from));
#endif

  /* Add FN_LIBCHAR to the end of directory path */
  if (to != to_org && (to[-1] != FN_LIBCHAR && to[-1] != FN_DEVCHAR))
  {
    *to++=FN_LIBCHAR;
    *to=0;
  }
#ifdef FN_UPPER_CASE
  caseup_str(to_org);
#endif
#ifdef FN_LOWER_CASE
  casedn_str(to_org);
#endif
  return to;					/* Pointer to end of dir */
} /* convert_dirname */

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

/*
  Return a pointer to the extension of the filename.

  SYNOPSIS
    fn_ext()
    name		Name of file

  DESCRIPTION
    The extension is defined as everything after the first extension character
    (normally '.') after the directory name.

  RETURN VALUES
    Pointer to to the extension character. If there isn't any extension,
    points at the end ASCII(0) of the filename.
*/

my_string fn_ext(const char *name)
{
  register my_string pos,gpos;
  DBUG_ENTER("fn_ext");
  DBUG_PRINT("mfunkt",("name: '%s'",name));

#if defined(FN_DEVCHAR) || defined(FN_C_AFTER_DIR)
  {
    char buff[FN_REFLEN];
    gpos=(my_string) name+dirname_part(buff,(char*) name);
  }
#else
  if (!(gpos=strrchr(name,FNLIBCHAR)))
    gpos=name;
#endif
  pos=strchr(gpos,FN_EXTCHAR);
  DBUG_RETURN (pos ? pos : strend(gpos));
} /* fn_ext */

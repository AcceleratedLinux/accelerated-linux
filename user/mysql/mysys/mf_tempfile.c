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
#include "my_static.h"
#include "mysys_err.h"
#include <errno.h>
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef HAVE_TEMPNAM
#if !defined(MSDOS) && !defined(OS2) && !defined(__NETWARE__)
extern char **environ;
#endif
#endif

/*
  Create a temporary file in a given directory
  This function should be used instead of my_tempnam() !
*/

File create_temp_file(char *to, const char *dir, const char *prefix,
		      int mode __attribute__((unused)),
		      myf MyFlags __attribute__((unused)))
{
  File file= -1;
  DBUG_ENTER("create_temp_file");
#if defined(_MSC_VER)
  {
    char temp[FN_REFLEN],*end,*res,**old_env,*temp_env[1];
    old_env=environ;
    if (dir)
    {
      end=strend(dir)-1;
      if (!dir[0])
      {				/* Change empty string to current dir */
	to[0]= FN_CURLIB;
	to[1]= 0;
	dir=to;
      }
      else if (*end == FN_DEVCHAR)
      {				/* Get current dir for drive */
	_fullpath(temp,dir,FN_REFLEN);
	dir=to;
      }
      else if (*end == FN_LIBCHAR && dir < end && end[-1] != FN_DEVCHAR)
      {
	strmake(to,dir,(uint) (end-dir));	/* Copy and remove last '\' */
	dir=to;
      }
      environ=temp_env;		/* Force use of dir (dir not checked) */
      temp_env[0]=0;
    }
    if ((res=tempnam((char*) dir,(char *) prefix)))
    {
      strmake(to,res,FN_REFLEN-1);
      (*free)(res);
      file=my_create(to,0, mode, MyFlags);
    }
    environ=old_env;
  }
#elif defined(_ZTC__)
  if (!dir)
    dir=getenv("TMPDIR");
  if ((res=tempnam((char*) dir,(char *) prefix)))
  {
    strmake(to,res,FN_REFLEN-1);
    (*free)(res);
    file=my_create(to, 0, mode, MyFlags);
  }
#elif defined(HAVE_MKSTEMP)
  {
    char prefix_buff[30];
    uint pfx_len;
    File org_file;

    pfx_len= (uint) (strmov(strnmov(prefix_buff,
				    prefix ? prefix : "tmp.",
				    sizeof(prefix_buff)-7),"XXXXXX") -
		     prefix_buff);
    if (!dir && ! (dir =getenv("TMPDIR")))
      dir=P_tmpdir;
    if (strlen(dir)+ pfx_len > FN_REFLEN-2)
    {
      errno=my_errno= ENAMETOOLONG;
      return 1;
    }
    strmov(convert_dirname(to,dir,NullS),prefix_buff);
    org_file=mkstemp(to);
    file=my_register_filename(org_file, to, FILE_BY_MKSTEMP,
			      EE_CANTCREATEFILE, MyFlags);
    /* If we didn't manage to register the name, remove the temp file */
    if (org_file >= 0 && file < 0)
    {
      int tmp=my_errno;
      (void) my_delete(to, MYF(MY_WME | ME_NOINPUT));
      my_errno=tmp;
    }
  }
#elif defined(HAVE_TEMPNAM)
  {
    char *res,**old_env,*temp_env[1];
    if (dir && !dir[0])
    {				/* Change empty string to current dir */
      to[0]= FN_CURLIB;
      to[1]= 0;
      dir=to;
    }
#ifdef OS2
    /* changing environ variable doesn't work with VACPP */
    char  buffer[256];
    sprintf( buffer, "TMP=%s", dir);
    /* remove ending backslash */
    if (buffer[strlen(buffer)-1] == '\\')
       buffer[strlen(buffer)-1] = '\0';
    putenv( buffer);
#elif !defined(__NETWARE__)
    old_env= (char**) environ;
    if (dir)
    {				/* Don't use TMPDIR if dir is given */
      environ=(const char**) temp_env;
      temp_env[0]=0;
    }
#endif
    if ((res=tempnam((char*) dir, (char*) prefix)))
    {    
      strmake(to,res,FN_REFLEN-1);
      (*free)(res);
      file=my_create(to,0,
		     (int) (O_RDWR | O_BINARY | O_TRUNC |
			    O_TEMPORARY | O_SHORT_LIVED),
		     MYF(MY_WME));

    }
    else
    {
      DBUG_PRINT("error",("Got error: %d from tempnam",errno));
    }
#if !defined(OS2) && !defined(__NETWARE__)
    environ=(const char**) old_env;
#endif
  }
#else
  {
    register long uniq;
    register int length;
    my_string pos,end_pos;
    /* Make an unique number */
    pthread_mutex_lock(&THR_LOCK_open);
    uniq= ((long) getpid() << 20) + (long) _my_tempnam_used++ ;
    pthread_mutex_unlock(&THR_LOCK_open);
    if (!dir && !(dir=getenv("TMPDIR")))	/* Use this if possibly */
      dir=P_tmpdir;			/* Use system default */
    length=strlen(dir)+strlen(pfx)+1;

    DBUG_PRINT("test",("mallocing %d byte",length+8+sizeof(TMP_EXT)+1));
    if (length+8+sizeof(TMP_EXT)+1 > FN_REFLENGTH)
      errno=my_errno= ENAMETOOLONG;
    else
    {
      end_pos=strmov(to,dir);
      if (end_pos != to && end_pos[-1] != FN_LIBCHAR)
	*end_pos++=FN_LIBCHAR;
      end_pos=strmov(end_pos,pfx);

      for (length=0 ; length < 8 && uniq ; length++)
      {
	*end_pos++= _dig_vec[(int) (uniq & 31)];
	uniq >>= 5;
      }
      (void) strmov(end_pos,TMP_EXT);
      file=my_create(to,0,
		     (int) (O_RDWR | O_BINARY | O_TRUNC |
			    O_TEMPORARY | O_SHORT_LIVED),
		     MYF(MY_WME));
    }
  }
#endif
  if (file >= 0)
    thread_safe_increment(my_tmp_file_created,&THR_LOCK_open);
  DBUG_RETURN(file);
}

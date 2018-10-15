/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2004 Tenable Network Security
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 /*
  * This file contains all the "unsafe" functions found in NASL
  */
#include <includes.h>

#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"  

#include "strutils.h"
#include "nasl_cmd_exec.h"
#include "nasl_debug.h"

static  pid_t pid = 0;
static  void (*old_sig_t)() = NULL, (*old_sig_i)() = NULL, (*old_sig_c) = NULL;

static void
sig_h()
{
  if (pid > 0)
    (void) kill(pid, SIGKILL);
}

static void	sig_c()
{
  if (pid > 0)
    (void) waitpid(pid, NULL, WNOHANG);
}

tree_cell*
nasl_pread(lex_ctxt * lexic)
{
  tree_cell	*retc = NULL, *a;
  anon_nasl_var	*v;
  nasl_array	*av;
  int		i, j, n, sz, sz2, cd, nice;
  char		** args = NULL, *cmd, *str, *str2, buf[8192];
  FILE		*fp;
#ifdef MAXPATHLEN
  char		cwd[MAXPATHLEN], newdir[MAXPATHLEN];
#else
  char		cwd[1024], newdir[1024];
#endif

 if ( check_authenticated(lexic) < 0 ) return NULL;
  
  if (pid != 0)
    {
      nasl_perror(lexic, "nasl_pread is not reentrant!\n");
      return NULL;
    }

  a = get_variable_by_name(lexic, "argv");
  cmd = get_str_local_var_by_name(lexic, "cmd");
  if (cmd == NULL || a == NULL || (v = a->x.ref_val) == NULL)
    {
      nasl_perror(lexic, "pread() usage: cmd:..., argv:...\n");
      return NULL;
    }

  nice = get_int_local_var_by_name(lexic, "nice", 0);
  
  if (v->var_type == VAR2_ARRAY)
    av = &v->v.v_arr;
  else
    {
      nasl_perror(lexic, "pread: argv element must be an array (0x%x)\n",
		  v->var_type);
      return NULL;
    }

  cd = get_int_local_var_by_name(lexic, "cd", 0);

  cwd[0] = '\0';
  if (cd)
    {
      char	*p;

      if (cmd[0] == '/')
	{
	  strncpy(newdir, cmd, sizeof(newdir)-1);
	  p = strrchr(newdir, '/');
	  if (p != newdir) *p = '\0';
	}
      else
	{
	  p = find_in_path(cmd, 0);
	  if (p != NULL)
	    strncpy(newdir, p, sizeof(newdir)-1);
	  else
	    {
	      nasl_perror(lexic, "pread: '%s' not found in $PATH\n", cmd);
	      return NULL;
	    }
	  
	}
      newdir[sizeof(newdir)-1] = '\0';

      if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
	  nasl_perror(lexic, "pread(): getcwd: %s\n", strerror(errno));
	  *cwd = '\0';
	}

      if (chdir(newdir) < 0)
	{
	  nasl_perror(lexic, "pread: could not chdir to %s\n", newdir);
	  return NULL;
	}
      if (cmd[0] != '/' && strlen(newdir) + strlen(cmd) + 1 < sizeof(newdir))
	{
	  strcat(newdir, "/");
	  strcat(newdir, cmd);
	  cmd = newdir;
	}
    }

  if (av->hash_elt != NULL)
    nasl_perror(lexic, "pread: named elements in 'cmd' are ignored!\n");
  n = av->max_idx;
  args = emalloc(sizeof(char**) * (n+2)); /* Last arg is NULL */
  for (j= 0, i = 0; i < n; i ++)
    {
      str = (char*)var2str(av->num_elt[i]);
      if (str != NULL)
	args[j++] = estrdup(str);
    }
  args[j++] = NULL;

  old_sig_t = signal(SIGTERM, sig_h);
  old_sig_i = signal(SIGINT, sig_h);
  old_sig_c = signal(SIGCHLD, sig_c);

  fp = nessus_popen4((const char*)cmd, args, &pid, nice);

  for (i = 0; i < n; i ++)
    efree(&args[i]);
  efree(&args);
  if (fp != NULL)
    {
      sz = 0; str = emalloc(1);

      errno = 0;
      while ((n = fread(buf, 1, sizeof(buf), fp)) > 0 || errno == EINTR ) /* && kill(pid, 0) >= 0)*/
	{
          if ( errno == EINTR ) 
          {
           errno = 0;
	   continue;
	  }
	  sz2 = sz + n;
	  str2 = erealloc(str, sz2);
	  if (str2 == NULL)
	    {
	      nasl_perror(lexic, "nasl_pread: erealloc failed\n");
	      break;
	    }
	  str = str2;
	  memcpy(str + sz, buf, n);
	  sz = sz2;
	}
      if (ferror(fp) && errno != EINTR )
	nasl_perror(lexic, "nasl_pread: fread(): %s\n", strerror(errno));

      (void) nessus_pclose(fp, pid);
      pid = 0;

      if (*cwd != '\0')
	if (chdir(cwd) < 0)
	  nasl_perror(lexic, "pread(): chdir(%s): %s\n", cwd, strerror(errno));

      retc = alloc_typed_cell(CONST_DATA);
      retc->x.str_val = str;
      retc->size = sz;
    }

  signal(SIGINT, old_sig_i);
  signal(SIGTERM, old_sig_t);
  signal(SIGCHLD, old_sig_c);

  return retc;
}

tree_cell*
nasl_find_in_path(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*cmd;


 if ( check_authenticated(lexic) < 0 ) return NULL;


  cmd = get_str_var_by_num(lexic, 0);
  if (cmd == NULL)
    {
      nasl_perror(lexic, "find_in_path() usage: cmd\n");
      return NULL;
    }

  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = (find_in_path(cmd, 0) != NULL);
  return retc;
}

/*
 * Not a command, but dangerous anyway
 */

tree_cell*
nasl_fread(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*fname;
  struct stat	st;
  char		*buf, *p;
  int		alen, len, n;
  FILE		*fp;

 if ( check_authenticated(lexic) < 0 ) return NULL;
    
  fname = get_str_var_by_num(lexic, 0);
  if (fname == NULL)
    {
      nasl_perror(lexic, "fread: need one argument (file name)\n");
      return NULL;
    }
  
  if (stat(fname, &st) < 0)
    {
      nasl_perror(lexic, "fread: stat(%s): %s\n", fname, strerror(errno));
      return NULL;
    }

  fp = fopen(fname, "r");
  if (fp == NULL)
    {
      nasl_perror(lexic, "fread: %s: %s\n", fname, strerror(errno));
      return NULL;
    }

  alen = st.st_size + 1;
  buf = emalloc(alen);
  if (buf == NULL)
    {
      nasl_perror(lexic, "fread: cannot malloc %d bytes\n", alen);
      goto error;
    }

  len = 0;
  while ((n = fread(buf + len, 1, alen - len, fp)) > 0)
    {
      len += n;
      if (alen <= len)
	{
	  alen += 4096;
	  p = erealloc(buf, alen);
	  if (p == NULL)
	    {
	      nasl_perror(lexic, "fread: cannot realloc %d bytes\n", alen);
	      goto error;
	    }
	  buf = p;
	}
    }

  buf[len] ='\0';
  if (alen > len+1)
    {
      p = erealloc(buf, len+1);
      if (p != NULL) buf = p;
    }

  retc = alloc_typed_cell(CONST_DATA);
  retc->size = len;
  retc->x.str_val = buf;
  fclose(fp);
  return retc;
  
 error:
  efree(&buf);
  fclose(fp);
  return NULL;
}

/*
 * Not a command, but dangerous anyway
 */

tree_cell*
nasl_unlink(lex_ctxt * lexic)
{
  char		*fname;

 if (check_authenticated(lexic) < 0) return NULL;
    
  fname = get_str_var_by_num(lexic, 0);
  if (fname == NULL)
    {
      nasl_perror(lexic, "unlink: need one argument (file name)\n");
      return NULL;
    }

  if (unlink(fname) < 0)
    {
      nasl_perror(lexic, "unlink(%s): %s\n", fname, strerror(errno));
      return NULL;
    }
  /* No need to return a value */
  return FAKE_CELL;
}

/* Definitely dangerous too */

tree_cell*
nasl_fwrite(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*content, *fname;
  int		len, i, x;
  FILE		*fp;

  if (check_authenticated(lexic) < 0)
    {
      nasl_perror(lexic, "fwrite may only be called by an authenticated script\n");
      return NULL;
    }
    
  content = get_str_local_var_by_name(lexic, "data");
  fname = get_str_local_var_by_name(lexic, "file");
  if (content == NULL || fname == NULL)
    {
      nasl_perror(lexic, "fwrite: need two arguments 'data' and 'file'\n");
      return NULL;
    }
  len = get_var_size_by_name(lexic, "data");
  
  fp = fopen(fname, "w");
  if (fp == NULL)
    {
      nasl_perror(lexic, "fwrite: %s: %s\n", fname, strerror(errno));
      return NULL;
    }
  for (i = 0; i < len; )
    {
      x = fwrite(content + i, 1, len - i, fp);
      if (x > 0)
	i += x;
      else
	{
	  nasl_perror(lexic, "fwrite: %s: %s\n", fname, strerror(errno));
	  (void) fclose(fp);
	  unlink(fname);
	  return NULL;
	}
    }

  if (fclose(fp) < 0)
    {
      nasl_perror(lexic, "fwrite: %s: %s\n", fname, strerror(errno));
      unlink(fname);
      return NULL;
    }
  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = len;
  return retc;
}



tree_cell*
nasl_get_tmp_dir(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*p, path[MAXPATHLEN];
  int		warn = 0;

 if (check_authenticated(lexic) < 0) return NULL;
 snprintf(path, sizeof(path), "%s/nessus/tmp/", NESSUS_STATE_DIR);
 if (access(path, R_OK|W_OK|X_OK) < 0)
     {
     nasl_perror(lexic, "get_tmp_dir(): %s not available - check your Nessus installation\n", path);
     return NULL;
     }
     
 retc = alloc_typed_cell(CONST_DATA);
 retc->x.str_val = strdup(path);
 retc->size = strlen(retc->x.str_val);
 return retc;
}


/*
 *  File access functions : Dangerous
 */


tree_cell*
nasl_file_stat(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*fname;
  struct stat	st;

  if ( check_authenticated(lexic) < 0 ) return NULL;
 
  fname = get_str_var_by_num(lexic, 0);
  if (fname == NULL)
    {
      nasl_perror(lexic, "file_stat: need one argument (file name)\n");
      return NULL;
    }
  
  if (stat(fname, &st) < 0)
      return NULL;

  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = (int)st.st_size;
  return retc;
}


tree_cell*
nasl_file_open(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*fname, *mode;
  int		fd; 
  int		imode;

  if ( check_authenticated(lexic) < 0 ) return NULL;
 
  fname = get_str_local_var_by_name(lexic, "name");
  if (fname == NULL)
    {
      nasl_perror(lexic, "file_open: need file name argument\n");
      return NULL;
    }
 
  mode = get_str_local_var_by_name(lexic, "mode");
  if (mode == NULL)
    {
      nasl_perror(lexic, "file_open: need file mode argument\n");
      return NULL;
    }

  if ( strcmp(mode, "r") == 0 ) 
	imode = O_RDONLY;
  else if ( strcmp(mode, "w") == 0 )
	imode = O_WRONLY|O_CREAT;
  else if ( strcmp(mode, "w+") == 0 )
	imode = O_WRONLY|O_TRUNC|O_CREAT;
   else if ( strcmp(mode, "a") == 0 )
	imode = O_WRONLY|O_APPEND|O_CREAT;
   else if ( strcmp(mode, "a+") == 0 )
	imode = O_RDWR|O_APPEND|O_CREAT;

  fd = open(fname, imode, 0600);
  if ( fd < 0 )
    {
      nasl_perror(lexic, "file_open: %s: %s\n", fname, strerror(errno));
      return NULL;
    }

  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = fd;
  return retc;
}


tree_cell*
nasl_file_close(lex_ctxt * lexic)
{
  tree_cell	*retc;
  int		fd;

  if ( check_authenticated(lexic) < 0 ) return NULL;
 
  fd = get_int_var_by_num(lexic, 0, -1);
  if ( fd < 0 )
    {
      nasl_perror(lexic, "file_close: need file pointer argument\n");
      return NULL;
    }
 
  if (close(fd) < 0)
    {
      nasl_perror(lexic, "file_close: %s\n", strerror(errno));
      return NULL;
    }

  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = 0;
  return retc;
}



tree_cell*
nasl_file_read(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*buf;
  int		len;
  int		fd;
  int           flength;
  int           n;

  if ( check_authenticated(lexic) < 0 ) return NULL;


 
  fd = get_int_local_var_by_name(lexic, "fp", -1);
  if (fd < 0 )
    {
      nasl_perror(lexic, "file_read: need file pointer argument\n");
      return NULL;
    }
 
  flength = get_int_local_var_by_name(lexic, "length", 0);

  buf = emalloc(flength + 1);
  if (buf == NULL)
    {
      nasl_perror(lexic, "file_read: cannot malloc %d bytes\n", flength);
      goto error;
    }

  for ( n = 0 ; n < flength ; )
  { 
   int e;
   errno = 0;
   e  = read(fd, buf + n, flength - n);
   if ( e < 0 && errno == EINTR ) continue;
   else if ( e <= 0 ) break;
   else n += e;
  }
  
  retc = alloc_typed_cell(CONST_DATA);
  retc->size = n;
  retc->x.str_val = buf;
  return retc;
 error:
  efree(&buf);
  return NULL;
}



tree_cell*
nasl_file_write(lex_ctxt * lexic)
{
  tree_cell	*retc;
  char		*content;
  int		len;
  int		fd;
  int 		n;

  if ( check_authenticated(lexic) < 0 ) return NULL;

  content = get_str_local_var_by_name(lexic, "data");
  fd = get_int_local_var_by_name(lexic, "fp", -1);
  if (content == NULL || fd < 0 )
    {
      nasl_perror(lexic, "file_write: need two arguments 'fp' and 'data'\n");
      return NULL;
    }
  len = get_var_size_by_name(lexic, "data");
 

  for ( n = 0 ; n < len ;  )
  { 
   int e;
   errno = 0;
   e = write(fd, content + n ,len - n);
   if ( e < 0 && errno == EINTR ) continue;
   else if ( e <= 0 ) 
     {
      nasl_perror(lexic, "file_write: write() failed - %s\n", strerror(errno));
      break;
     }
   else n += e ;
  }
   


  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = n;
  return retc;
}


tree_cell*
nasl_file_seek(lex_ctxt * lexic)
{
  tree_cell	*retc;
  int		len;
  int		fd;
  int           foffset;

  if ( check_authenticated(lexic) < 0 ) return NULL;




  foffset = get_int_local_var_by_name(lexic, "offset", 0);
  fd = get_int_local_var_by_name(lexic, "fp", -1);
  if (fd < 0 )
    {
      nasl_perror(lexic, "file_seek: need one arguments 'fp'\n");
      return NULL;
    }

  if ( lseek(fd, foffset, SEEK_SET) < 0 )
    {
      nasl_perror(lexic, "fseek: %s\n", strerror(errno));
      return NULL;
    }

  retc = alloc_typed_cell(CONST_INT);
  retc->x.i_val = 0;
  return retc;
}


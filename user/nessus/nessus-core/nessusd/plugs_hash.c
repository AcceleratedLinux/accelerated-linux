/* Nessus
 * Copyright (C) 1998 - 2006 Tenable Network Security, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
 *
 * This module computes the hash of the plugins (and the hash of the
 * hashes of the plugins)
 *
 */

#include <includes.h>
#include "./md5.h"
#include "users.h"
#include "log.h"

char *
file_hash(fname)
  char * fname;
{
 struct stat st;
 int fd = open(fname, O_RDONLY);
 char * content;
 int len;

 if(fd < 0)
  return NULL;
  
 fstat(fd, &st);
 
 len = (int)st.st_size;
 content = mmap(NULL,len, PROT_READ, MAP_SHARED,fd, 0);
 if(content &&
    (content != MAP_FAILED))
    {
     char * ret = md5sum(content, len);
     munmap(content, len);
     close(fd);
     return ret;
    }
 return NULL;
}


/*
 * Returns a hash of each plugin hash
 */
static void
dir_plugins_hash(ctx, dirname)
 md5_ctx * ctx;
 char * dirname;
{
 DIR * dir;
 struct dirent * dp;
 
 
 if(!dirname)
  return;
    
 dir = opendir(dirname);
 if(!dir)
 {
  log_write("plugins_hash(): could not open %s - %s\n",
  			dirname,
			strerror(errno));
  return;
 }
 

 while((dp = readdir(dir)))
 {
  char fullname[PATH_MAX + 1];
  char * tmp;
  if((strlen(dirname) + strlen(dp->d_name) + 1) >
     (sizeof(fullname) - 1))
     {
     log_write("plugins_hash(): filename too long\n");
     continue;
     }

   if(dp->d_name[0] == '.')continue; /* Skip .dot files */

   bzero(fullname, sizeof(fullname));
   strcat(fullname, dirname);
   strcat(fullname, "/");
   strcat(fullname, dp->d_name);
   tmp = file_hash(fullname);
   if(tmp != NULL)
    { 
    md5update(ctx, tmp, strlen(tmp));
    efree(&tmp);
   }
  }
  closedir(dir);
}


/*
 * returns the hash of the hashes of the plugins in the
 * plugins dir + plugins in the user home dir
 */
char * 
plugins_hash(globals)
 struct arglist * globals;
{
 struct arglist * preferences = arg_get_value(globals,"preferences");
 char *dir  = arg_get_value(preferences, "plugins_folder");
 char *uhome;
 md5_ctx * ctx;
 char * ret;
 
 ctx = md5init();
 dir_plugins_hash(ctx, dir);
 uhome = user_home(globals);
 dir = emalloc(strlen(uhome) + strlen("/plugins") + 1);
 sprintf(dir, "%s/plugins", uhome);
 efree(&uhome);
 dir_plugins_hash(ctx, dir);
 efree(&dir);
 ret = md5final(ctx);
 md5free(ctx);
 return ret;
}


static void plugins_send_md5_byid(globals)
 struct arglist * globals;
{
 struct arglist * plugins = arg_get_value(globals, "plugins");


 auth_printf(globals, "SERVER <|> PLUGINS_MD5\n");
 
 if( plugins == NULL )
	return;

 while( plugins->next != NULL )
 {
  struct arglist * args = plugins->value;
  char * fname = plug_get_path(args);
  int id = plug_get_id(args);
  char * md5   = file_hash(fname);
  auth_printf(globals, "%d <|> %s\n", id, md5);
  efree(&md5);
  plugins = plugins->next;
 }
 auth_printf(globals, "<|> SERVER\n");
}


static void plugins_send_md5_byname(struct arglist * globals)
{ 
 struct arglist * preferences = arg_get_value(globals,"preferences");
 char * dirname  = arg_get_value(preferences, "plugins_folder");
 DIR * dir;
 struct dirent * dp;

 if( dirname == NULL )
	return;

 dir = opendir(dirname);
 if( dir == NULL )
	return;

 auth_printf(globals, "SERVER <|> PLUGINS_MD5\n");
 while ( (dp = readdir(dir)) != NULL )
 {
  char fullname[PATH_MAX + 1];
  char * tmp;

  if(dp->d_name[0] == '.')
	continue;

  if(strlen(dirname) + strlen(dp->d_name) > (sizeof(fullname) - 2))
	continue;

  snprintf(fullname, sizeof(fullname), "%s/%s", dirname, dp->d_name);
  tmp = file_hash(fullname);
  if( tmp != NULL )
   {
    auth_printf(globals, "%s <|> %s\n", dp->d_name, tmp);
    efree(&tmp);
   }
 }
 closedir(dir);
 auth_printf(globals, "<|> SERVER\n");
}



void plugins_send_md5(struct arglist * globals)
{
 ntp_caps * caps = arg_get_value(globals, "ntp_caps");
 if( caps == NULL )
  return;
 if(caps->md5_by_name)
	plugins_send_md5_byname(globals);
 else
	plugins_send_md5_byid(globals);
}

 


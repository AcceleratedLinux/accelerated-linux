
/***************************************************************************
 * filespace.c -- a simple mechanism for storing dynamic amounts of data   *
 * in a simple to use, and quick to append-to structure.                   *
 *                                                                         *
 ***********************IMPORTANT NSOCK LICENSE TERMS***********************
 *                                                                         *
 * The nsock parallel socket event library is (C) 1999-2006 Insecure.Com   *
 * LLC This library is free software; you may redistribute and/or          *
 * modify it under the terms of the GNU General Public License as          *
 * published by the Free Software Foundation; Version 2.  This guarantees  *
 * your right to use, modify, and redistribute this software under certain *
 * conditions.  If this license is unacceptable to you, Insecure.Com LLC   *
 * may be willing to sell alternative licenses (contact                    *
 * sales@insecure.com ).                                                   *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included Copying.OpenSSL file, and distribute linked      *
 * combinations including the two. You must obey the GNU GPL in all        *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         * 
 * If you received these files with a written license agreement stating    *
 * terms other than the (GPL) terms above, then that alternative license   *
 * agreement takes precedence over this comment.                          *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to fyodor@insecure.org for possible incorporation into the main         *
 * distribution.  By sending these changes to Fyodor or one the            *
 * insecure.org development mailing lists, it is assumed that you are      *
 * offering Fyodor and Insecure.Com LLC the unlimited, non-exclusive right *
 * to reuse, modify, and relicense the code.  Nmap will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).  We also occasionally relicense the    *
 * code to third parties as discussed above.  If you wish to specify       *
 * special license conditions of your contributions, just say so when you  *
 * send them.                                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details (                               *
 * http://www.gnu.org/copyleft/gpl.html ).                                 *
 *                                                                         *
 ***************************************************************************/

/* $Id: filespace.c 3870 2006-08-25 01:47:53Z fyodor $ */

#include "filespace.h"

#include <string.h>

/* Assumes space for fs has already been allocated */
int filespace_init(struct filespace *fs, int initial_size) {
  
  memset(fs, 0, sizeof(struct filespace));
  if (initial_size == 0)
    initial_size = 1024;
  fs->current_alloc = initial_size;
  fs->str = (char *) malloc(fs->current_alloc);
  if (!fs->str) return -1;
  fs->str[0] = '\0';
  fs->pos = fs->str;
  return 0;
}

/* Used when you want to start over with a filespace you have been
   using (it sets the length to zero and the pointers to the beginning
   of memory , etc */
int fs_clear(struct filespace *fs) {
  fs->current_size = 0;
  fs->pos = fs->str;
  fs->str[0] = '\0'; /* Not necessary, possible help with debugging */
  return 0;
}


int fs_free(struct filespace *fs) {
  if (fs->str) free(fs->str);
  fs->current_alloc = fs->current_size = 0;
  fs->pos = fs->str = NULL;
  return 0;
}

/* Prepend an n-char string to a filespace */
int fs_prepend(char *str, int len, struct filespace *fs)  {
char *tmpstr;

if (len < 0) return -1;
if (len == 0) return 0;

  if (fs->current_alloc - fs->current_size < len + 2) {
    fs->current_alloc = (int) (fs->current_alloc * 1.4 + 1 );
    fs->current_alloc += 100 + len;
    tmpstr = (char *) malloc(fs->current_alloc);
    if (!tmpstr)
      return -1;
    memcpy(tmpstr, fs->str, fs->current_size);
    fs->pos = (fs->pos - fs->str) + tmpstr;
    if (fs->str) free(fs->str);
    fs->str = tmpstr; 
  }
  if (fs->current_size > 0)   
    memmove(fs->str + len, fs->str, fs->current_size);
  memcpy(fs->str, str, len);

  fs->current_size += len;
  fs->str[fs->current_size] = '\0';
  return 0;
}













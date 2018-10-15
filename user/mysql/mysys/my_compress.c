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

/* Written by Sinisa Milivojevic <sinisa@mysql.com> */

#include <my_global.h>
#ifdef HAVE_COMPRESS
#include <my_sys.h>
#ifndef SCO
#include <m_string.h>
#endif
#include <zlib.h>

/*
** This replaces the packet with a compressed packet
** Returns 1 on error
** *complen is 0 if the packet wasn't compressed
*/

my_bool my_compress(byte *packet, ulong *len, ulong *complen)
{
  DBUG_ENTER("my_compress");
  if (*len < MIN_COMPRESS_LENGTH)
  {
    *complen=0;
    DBUG_PRINT("note",("Packet too short: Not compressed"));
  }
  else
  {
    byte *compbuf=my_compress_alloc(packet,len,complen);
    if (!compbuf)
      DBUG_RETURN(*complen ? 0 : 1);
    memcpy(packet,compbuf,*len);
    my_free(compbuf,MYF(MY_WME));						  }
  DBUG_RETURN(0);
}


byte *my_compress_alloc(const byte *packet, ulong *len, ulong *complen)
{
  byte *compbuf;
  *complen=  *len * 120 / 100 + 12;
  if (!(compbuf= (byte *) my_malloc(*complen,MYF(MY_WME))))
    return 0;					/* Not enough memory */
  if (compress((Bytef*) compbuf,(ulong *) complen, (Bytef*) packet,
	       (uLong) *len ) != Z_OK)
  {
    my_free(compbuf,MYF(MY_WME));
    return 0;
  }
  if (*complen >= *len)
  {
    *complen= 0;
    my_free(compbuf, MYF(MY_WME));
    DBUG_PRINT("note",("Packet got longer on compression; Not compressed"));
    return 0;
  }
  swap(ulong, *len, *complen);			/* *len is now packet length */
  return compbuf;
}


my_bool my_uncompress (byte *packet, ulong *len, ulong *complen)
{
  DBUG_ENTER("my_uncompress");
  if (*complen)					/* If compressed */
  {
    byte *compbuf= (byte *) my_malloc(*complen,MYF(MY_WME));
    int error;
    if (!compbuf)
      DBUG_RETURN(1);				/* Not enough memory */
    if ((error=uncompress((Bytef*) compbuf, complen, (Bytef*) packet, *len))
	!= Z_OK)
    {						/* Probably wrong packet */
      DBUG_PRINT("error",("Can't uncompress packet, error: %d",error));
      my_free(compbuf, MYF(MY_WME));
      DBUG_RETURN(1);
    }
    *len= *complen;
    memcpy(packet, compbuf, *len);
    my_free(compbuf, MYF(MY_WME));
  }
  DBUG_RETURN(0);
}
#endif /* HAVE_COMPRESS */

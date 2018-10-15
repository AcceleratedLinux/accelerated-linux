/* UNIX RFCNB (RFC1001/RFC1002) NetBIOS implementation

   Version 1.0
   RFCNB Common Structures etc Defines

   Copyright (C) Richard Sharpe 1996

*/

/*
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* A data structure we need */

typedef struct RFCNB_Pkt {

  char * data;			/* The data in this portion */
  int len;
  struct RFCNB_Pkt *next;

} RFCNB_Pkt;



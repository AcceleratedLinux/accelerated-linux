/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

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
 This is the header file for code which implements the Secure
 Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 April 17, 1995.

 Many of the variable names in this code, especially the
 single character names, were used because those were the names
 used in the publication.

 Please read the file sha1.c for more information.

 Modified 2002 by Peter Zaitsev to better follow MySQL standards
*/


enum sha_result_codes
{
  SHA_SUCCESS = 0,
  SHA_NULL,		/* Null pointer parameter */
  SHA_INPUT_TOO_LONG,	/* input data too long */
  SHA_STATE_ERROR	/* called Input after Result */
};

#define SHA1_HASH_SIZE 20 /* Hash size in bytes */

/*
  This structure will hold context information for the SHA-1
  hashing operation
*/

typedef struct SHA1_CONTEXT
{
  ulonglong  Length;		/* Message length in bits      */
  uint32 Intermediate_Hash[SHA1_HASH_SIZE/4]; /* Message Digest  */
  int Computed;			/* Is the digest computed?	   */
  int Corrupted;		/* Is the message digest corrupted? */
  int16 Message_Block_Index;	/* Index into message block array   */
  uint8 Message_Block[64];	/* 512-bit message blocks      */
} SHA1_CONTEXT;

/*
  Function Prototypes
*/

C_MODE_START

int sha1_reset( SHA1_CONTEXT* );
int sha1_input( SHA1_CONTEXT*, const uint8 *, unsigned int );
int sha1_result( SHA1_CONTEXT* , uint8 Message_Digest[SHA1_HASH_SIZE] );

C_MODE_END

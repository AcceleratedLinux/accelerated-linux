/*
 * lib/crypto/des/string2key.c
 *
 * based on lib/crypto/des/string2key.c from MIT V5 
 * and on lib/des/afs_string_to_key.c from UMD.
 * constructed by Mark Eichin, Cygnus Support, 1995.
 * made thread-safe by Ken Raeburn, MIT, 2001.
 */

/*
 * Copyright 2001 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

/*
 * Copyright (C) 1998 by the FundsXpress, INC.
 * 
 * All rights reserved.
 * 
 * Export of this software from the United States of America may require
 * a specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of FundsXpress. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  FundsXpress makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "k5-int.h"
#include "des_int.h"
#include <ctype.h>

#define afs_crypt mit_afs_crypt
char *afs_crypt (const char *, const char *, char *);

#undef min
#define min(a,b) ((a)>(b)?(b):(a))

krb5_error_code
mit_afs_string_to_key (krb5_keyblock *keyblock, const krb5_data *data,
		       const krb5_data *salt)
{
  /* totally different approach from MIT string2key. */
  /* much of the work has already been done by the only caller 
     which is mit_des_string_to_key; in particular, *keyblock is already 
     set up. */
  
    char *realm = salt->data;
    unsigned int i, j;
    krb5_octet *key = keyblock->contents;

    if (data->length <= 8) {
      /* One block only.  Run afs_crypt and use the first eight
	 returned bytes after the copy of the (fixed) salt.

	 Since the returned bytes are alphanumeric, the output is
	 limited to 2**48 possibilities; for each byte, only 64
	 possible values can be used.  */
      unsigned char password[9]; /* trailing nul for crypt() */
      char afs_crypt_buf[16];

      memset (password, 0, sizeof (password));
      memcpy (password, realm, min (salt->length, 8));
      for (i=0; i<8; i++)
	if (isupper(password[i]))
	  password[i] = tolower(password[i]);
      for (i=0; i<data->length; i++)
	password[i] ^= data->data[i];
      for (i=0; i<8; i++)
	if (password[i] == '\0')
	  password[i] = 'X';
      password[8] = '\0';
      /* Out-of-bounds salt characters are equivalent to a salt string
	 of "p1".  */
      strncpy((char *) key,
	      (char *) afs_crypt((char *) password, "#~", afs_crypt_buf) + 2,
	      8);
      for (i=0; i<8; i++)
	key[i] <<= 1;
      /* now fix up key parity again */
      mit_des_fixup_key_parity(key);
      /* clean & free the input string */
      memset(password, 0, (size_t) sizeof(password));
    } else {
      /* Multiple blocks.  Do a CBC checksum, twice, and use the
	 result as the new key.  */
      mit_des_cblock ikey, tkey;
      mit_des_key_schedule key_sked;
      unsigned int pw_len = salt->length+data->length;
      unsigned char *password = malloc(pw_len+1);
      if (!password) return ENOMEM;

      /* Some bound checks from the original code are elided here as
	 the malloc above makes sure we have enough storage. */
      memcpy (password, data->data, data->length);
      for (i=data->length, j = 0; j < salt->length; i++, j++) {
	password[i] = realm[j];
	if (isupper(password[i]))
	  password[i] = tolower(password[i]);
      }
	
      memcpy (ikey, "kerberos", sizeof(ikey));
      memcpy (tkey, ikey, sizeof(tkey));
      mit_des_fixup_key_parity (tkey);
      (void) mit_des_key_sched (tkey, key_sked);
      (void) mit_des_cbc_cksum (password, tkey, i, key_sked, ikey);

      memcpy (ikey, tkey, sizeof(ikey));
      mit_des_fixup_key_parity (tkey);
      (void) mit_des_key_sched (tkey, key_sked);
      (void) mit_des_cbc_cksum (password, key, i, key_sked, ikey);
	
      /* erase key_sked */
      memset((char *)key_sked, 0,sizeof(key_sked));

      /* now fix up key parity again */
      mit_des_fixup_key_parity(key);
      
      /* clean & free the input string */
      memset(password, 0, (size_t) pw_len);
      free(password);
    }
#if 0
    /* must free here because it was copied for this special case */
    free(salt->data);
#endif
    return 0;
}


/* Portions of this code:
   Copyright 1989 by the Massachusetts Institute of Technology
   */
 
/*
 * Copyright (c) 1990 Regents of The University of Michigan.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appears in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * The University of Michigan not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission. This software is supplied as
 * is without expressed or implied warranties of any kind.
 *
 *	ITD Research Systems
 *	University of Michigan
 *	535 W. William Street
 *	Ann Arbor, Michigan
 *	+1-313-936-2652
 *	netatalk@terminator.cc.umich.edu
 */

static void krb5_afs_crypt_setkey (char*, char*, char(*)[48]);
static void krb5_afs_encrypt (char*,char*,char (*)[48]);

/*
 * Initial permutation,
 */
static const char	IP[] = {
	58,50,42,34,26,18,10, 2,
	60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6,
	64,56,48,40,32,24,16, 8,
	57,49,41,33,25,17, 9, 1,
	59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5,
	63,55,47,39,31,23,15, 7,
};
 
/*
 * Final permutation, FP = IP^(-1)
 */
static const char	FP[] = {
	40, 8,48,16,56,24,64,32,
	39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30,
	37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28,
	35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26,
	33, 1,41, 9,49,17,57,25,
};
 
/*
 * Permuted-choice 1 from the key bits to yield C and D.
 * Note that bits 8,16... are left out: They are intended for a parity check.
 */
static const char	PC1_C[] = {
	57,49,41,33,25,17, 9,
	 1,58,50,42,34,26,18,
	10, 2,59,51,43,35,27,
	19,11, 3,60,52,44,36,
};
 
static const char	PC1_D[] = {
	63,55,47,39,31,23,15,
	 7,62,54,46,38,30,22,
	14, 6,61,53,45,37,29,
	21,13, 5,28,20,12, 4,
};
 
/*
 * Sequence of shifts used for the key schedule.
 */
static const char	shifts[] = {
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,
};
 
/*
 * Permuted-choice 2, to pick out the bits from
 * the CD array that generate the key schedule.
 */
static const char	PC2_C[] = {
	14,17,11,24, 1, 5,
	 3,28,15, 6,21,10,
	23,19,12, 4,26, 8,
	16, 7,27,20,13, 2,
};
 
static const char	PC2_D[] = {
	41,52,31,37,47,55,
	30,40,51,45,33,48,
	44,49,39,56,34,53,
	46,42,50,36,29,32,
};
 
/*
 * The E bit-selection table.
 */
static const char	e[] = {
	32, 1, 2, 3, 4, 5,
	 4, 5, 6, 7, 8, 9,
	 8, 9,10,11,12,13,
	12,13,14,15,16,17,
	16,17,18,19,20,21,
	20,21,22,23,24,25,
	24,25,26,27,28,29,
	28,29,30,31,32, 1,
};
 
/*
 * P is a permutation on the selected combination
 * of the current L and key.
 */
static const char	P[] = {
	16, 7,20,21,
	29,12,28,17,
	 1,15,23,26,
	 5,18,31,10,
	 2, 8,24,14,
	32,27, 3, 9,
	19,13,30, 6,
	22,11, 4,25,
};
 
/*
 * The 8 selection functions.
 * For some reason, they give a 0-origin
 * index, unlike everything else.
 */
static const char	S[8][64] = {
	{14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	  0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	  4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	 15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13},
 
	{15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	  3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	  0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	 13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9},
 
	{10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	 13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	 13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	  1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12},
 
	{ 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	 13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	 10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	  3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14},
 
	{ 2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	 14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	  4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	 11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3},
 
	{12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	 10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	  9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	  4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13},
 
	{ 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	 13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	  1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	  6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12},
 
	{13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	  1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	  7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	  2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11},
};
 
 
char *afs_crypt(const char *pw, const char *salt,
		/* must be at least 16 bytes */
		char *iobuf)
{
	int i, j, c;
	int temp;
	char block[66];
	char E[48];
	/*
	 * The key schedule.
	 * Generated from the key.
	 */
	char KS[16][48];
 
	for(i=0; i<66; i++)
		block[i] = 0;
	for(i=0; (c= *pw) && i<64; pw++){
		for(j=0; j<7; j++, i++)
			block[i] = (c>>(6-j)) & 01;
		i++;
	}
	
	krb5_afs_crypt_setkey(block, E, KS);

	for(i=0; i<66; i++)
		block[i] = 0;

	for(i=0;i<2;i++){
		c = *salt++;
		iobuf[i] = c;
		if(c>'Z') c -= 6;
		if(c>'9') c -= 7;
		c -= '.';
		for(j=0;j<6;j++){
			if((c>>j) & 01){
				temp = E[6*i+j];
				E[6*i+j] = E[6*i+j+24];
				E[6*i+j+24] = temp;
				}
			}
		}
	
	for(i=0; i<25; i++)
		krb5_afs_encrypt(block,E,KS);
	
	for(i=0; i<11; i++){
		c = 0;
		for(j=0; j<6; j++){
			c <<= 1;
			c |= block[6*i+j];
			}
		c += '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		iobuf[i+2] = c;
	}
	iobuf[i+2] = 0;
	if(iobuf[1]==0)
		iobuf[1] = iobuf[0];
	return(iobuf);
}

/*
 * Set up the key schedule from the key.
 */
 
static void krb5_afs_crypt_setkey(char *key, char *E, char (*KS)[48])
{
	register int i, j, k;
	int t;
	/*
	 * The C and D arrays used to calculate the key schedule.
	 */
	char C[28], D[28];
 
	/*
	 * First, generate C and D by permuting
	 * the key.  The low order bit of each
	 * 8-bit char is not used, so C and D are only 28
	 * bits apiece.
	 */
	for (i=0; i<28; i++) {
		C[i] = key[PC1_C[i]-1];
		D[i] = key[PC1_D[i]-1];
	}
	/*
	 * To generate Ki, rotate C and D according
	 * to schedule and pick up a permutation
	 * using PC2.
	 */
	for (i=0; i<16; i++) {
		/*
		 * rotate.
		 */
		for (k=0; k<shifts[i]; k++) {
			t = C[0];
			for (j=0; j<28-1; j++)
				C[j] = C[j+1];
			C[27] = t;
			t = D[0];
			for (j=0; j<28-1; j++)
				D[j] = D[j+1];
			D[27] = t;
		}
		/*
		 * get Ki. Note C and D are concatenated.
		 */
		for (j=0; j<24; j++) {
			KS[i][j] = C[PC2_C[j]-1];
			KS[i][j+24] = D[PC2_D[j]-28-1];
		}
	}
 
#if 0
	for(i=0;i<48;i++) {
		E[i] = e[i];
	}
#else
	memcpy(E, e, 48);
#endif
}
 
/*
 * The payoff: encrypt a block.
 */
 
static void krb5_afs_encrypt(char *block, char *E, char (*KS)[48])
{
	const long edflag = 0;
	int i, ii;
	int t, j, k;
	char tempL[32];
	char f[32];
	/*
	 * The current block, divided into 2 halves.
	 */
	char L[64];
	char *const R = &L[32];
	/*
	 * The combination of the key and the input, before selection.
	 */
	char preS[48];

	/*
	 * First, permute the bits in the input
	 */
	for (j=0; j<64; j++)
		L[j] = block[IP[j]-1];
	/*
	 * Perform an encryption operation 16 times.
	 */
	for (ii=0; ii<16; ii++) {
		/*
		 * Set direction
		 */
		if (edflag)
			i = 15-ii;
		else
			i = ii;
		/*
		 * Save the R array,
		 * which will be the new L.
		 */
#if 0
		for (j=0; j<32; j++)
			tempL[j] = R[j];
#else
		memcpy(tempL, R, 32);
#endif
		/*
		 * Expand R to 48 bits using the E selector;
		 * exclusive-or with the current key bits.
		 */
		for (j=0; j<48; j++)
			preS[j] = R[E[j]-1] ^ KS[i][j];
		/*
		 * The pre-select bits are now considered
		 * in 8 groups of 6 bits each.
		 * The 8 selection functions map these
		 * 6-bit quantities into 4-bit quantities
		 * and the results permuted
		 * to make an f(R, K).
		 * The indexing into the selection functions
		 * is peculiar; it could be simplified by
		 * rewriting the tables.
		 */
		for (j=0; j<8; j++) {
			t = 6*j;
			k = S[j][(preS[t+0]<<5)+
				(preS[t+1]<<3)+
				(preS[t+2]<<2)+
				(preS[t+3]<<1)+
				(preS[t+4]<<0)+
				(preS[t+5]<<4)];
			t = 4*j;
				f[t+0] = (k>>3)&01;
				f[t+1] = (k>>2)&01;
				f[t+2] = (k>>1)&01;
				f[t+3] = (k>>0)&01;
		}
		/*
		 * The new R is L ^ f(R, K).
		 * The f here has to be permuted first, though.
		 */
		for (j=0; j<32; j++)
			R[j] = L[j] ^ f[P[j]-1];
		/*
		 * Finally, the new L (the original R)
		 * is copied back.
		 */
#if 0
		for (j=0; j<32; j++)
			L[j] = tempL[j];
#else
		memcpy(L, tempL, 32);
#endif
	}
	/*
	 * The output L and R are reversed.
	 */
	for (j=0; j<32; j++) {
		t = L[j];
		L[j] = R[j];
		R[j] = t;
	}
	/*
	 * The final output
	 * gets the inverse permutation of the very original.
	 */
	for (j=0; j<64; j++)
		block[j] = L[FP[j]-1];
}

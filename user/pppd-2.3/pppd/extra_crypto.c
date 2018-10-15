/*
 * Copyright (c) Tim Hockin, Cobalt Networks Inc. and others
 *
 * crypto routines used by multiple c files 
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "extra_crypto.h"
#include "pppd.h"
#include "md4.h"
#ifndef USE_CRYPT
#include <des/des.h>
#endif

/* quick wrapper for easy md4 */
void
md4(unsigned char *from, int from_len, unsigned char *to)
{
  MD4_CTX Context;

#ifndef __NetBSD__
  from_len <<= 3; /* bytes->bits */
#endif

  MD4Init(&Context);
  MD4Update(&Context, from, from_len);
  MD4Final(to, &Context);
}


/* Microsoft LANMAN Password hashing */
static u_char *MSStdText = (u_char *)"KGS!@#$%"; /* key from rasapi32.dll */

void
LmPasswordHash(char *password, int len, char *hash)
{
    int	i;
    u_char up_pass[MAX_NT_PASSWORD]; /* max is actually 14 */

    /* LANMan password is case insensitive */
    BZERO(up_pass, sizeof(up_pass));
    for (i = 0; i < len; i++)
       up_pass[i] = (u_char)toupper(up_pass[i]);
    DesEncrypt(MSStdText, up_pass + 0, hash + 0);
    DesEncrypt(MSStdText, up_pass + 7, hash + 8);
}

void
NtPasswordHash(char *secret, int secret_len, unsigned char *hash)
{
    int	i;
    u_char unicodePassword[MAX_NT_PASSWORD * 2];

    /* Initialize the Unicode version of the secret (== password). */
    /* This implicitly supports 8-bit ISO8859/1 characters. */
    BZERO(unicodePassword, sizeof(unicodePassword));
    for (i = 0; i < secret_len; i++)
	unicodePassword[i * 2] = (u_char)secret[i];

    /* Unicode is 2 bytes per char */
    md4(unicodePassword, secret_len * 2, hash);
}


static u_char Get7Bits(unsigned char *input, int startBit)
{
    register unsigned int       word;

    word  = (unsigned)input[startBit / 8] << 8;
    word |= (unsigned)input[startBit / 8 + 1];

    word >>= 15 - (startBit % 8 + 7);

    return word & 0xFE;
}


static void MakeKey(unsigned char *key, unsigned char *des_key)
{
    des_key[0] = Get7Bits(key,  0);
    des_key[1] = Get7Bits(key,  7);
    des_key[2] = Get7Bits(key, 14);
    des_key[3] = Get7Bits(key, 21);
    des_key[4] = Get7Bits(key, 28);
    des_key[5] = Get7Bits(key, 35);
    des_key[6] = Get7Bits(key, 42);
    des_key[7] = Get7Bits(key, 49);

#ifndef USE_CRYPT
    des_set_odd_parity((des_cblock *)des_key);
#endif
}


#ifdef USE_CRYPT
/* in == 8-byte string (expanded version of the 56-bit key)
 * out == 64-byte string where each byte is either 1 or 0
 * Note that the low-order "bit" is always ignored by by setkey()
 */
static void Expand(unsigned char *in, unsigned char *out)
{
        int j, c;
        int i;

        for(i = 0; i < 64; in++){
		c = *in;
                for(j = 7; j >= 0; j--)
                        *out++ = (c >> j) & 01;
                i += 8;
        }
}

/* The inverse of Expand
 */
static void Collapse(unsigned char *in, unsigned char *out)
{
        int j;
        int i;
	unsigned int c;

	for (i = 0; i < 64; i += 8, out++) {
	    c = 0;
	    for (j = 7; j >= 0; j--, in++)
		c |= *in << j;
	    *out = c & 0xff;
	}
}
void
DesEncrypt(unsigned char *clear, unsigned char *key, unsigned char *cipher)
{
    u_char des_key[8];
    u_char crypt_key[66];
    u_char des_input[66];

    MakeKey(key, des_key);

    Expand(des_key, crypt_key);
    setkey(crypt_key);

    Expand(clear, des_input);
    encrypt(des_input, 0);
    Collapse(des_input, cipher);
}
#else /* don't USE_CRYPT */
void
DesEncrypt(unsigned char *clear, unsigned char *key, unsigned char *cipher)
{
    des_cblock		des_key;
    des_key_schedule	key_schedule;

    MakeKey(key, des_key);

    des_set_key(&des_key, key_schedule);

    des_ecb_encrypt((des_cblock *)clear, (des_cblock *)cipher, key_schedule, 1);
}
#endif /* USE_CRYPT */



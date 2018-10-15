/*
 * mppe - Mucking with PpP Encription
 *
 * Copyright (c) 1995 �rp�d Magoss�nyi
 * All rights reserved.
 *
 * Copyright (c) 1999 Tim Hockin, Cobalt Networks Inc.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Pedro Roque Marques.  The name of the author may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef MPPE

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include "pppd.h"
#include "chap.h"
#include "fsm.h"
#include "ccp.h"
#include "md4.h" 
#include "sha.h"
#include "chap_ms.h"
#include "extra_crypto.h"

static void
mppe_get_start_key __P((unsigned char *, unsigned char *, unsigned char *));
static void
mppe_get_master_key __P((unsigned char *, unsigned char *, unsigned char *));
static void
GetAsymetricStartKey __P((unsigned char *, unsigned char *, int, int, int));

unsigned char mppe_master_send_key_40[8];
unsigned char mppe_master_recv_key_40[8];
unsigned char mppe_master_send_key_128[16];
unsigned char mppe_master_recv_key_128[16];
unsigned int mppe_allowed = 0;

/*
 * Pads used in key derivation - from sha1dgst.c
 */
static unsigned char  SHApad1[40] =
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char  SHApad2[40] =
  {0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2};


/* This is used with chap-ms (v1) */
void
mppe_gen_master_key(char *secret, int secret_len, unsigned char *challenge)
{
    unsigned char PasswordHash[MD4_SIGNATURE_SIZE];
    unsigned char PasswordHashHash[MD4_SIGNATURE_SIZE];

    /* 40 bit */
    LmPasswordHash(secret, secret_len, PasswordHash);
    BCOPY(PasswordHash, mppe_master_send_key_40, 8);
    BCOPY(mppe_master_send_key_40, mppe_master_recv_key_40, 8);

    /* 128 bit */
    NtPasswordHash(secret, secret_len, PasswordHash);
    md4(PasswordHash, sizeof(PasswordHash), PasswordHashHash);
    mppe_get_start_key(challenge, PasswordHashHash, mppe_master_send_key_128);
    BCOPY(mppe_master_send_key_128, mppe_master_recv_key_128, 16);

    mppe_allowed = 1;
}


/* This is used with chap-ms-v2 (per MS' draft RFC) - 2 different keys */
void
mppe_gen_master_key_v2(char *secret, int secret_len, unsigned char *response, 
			int is_server)
{
    unsigned char PasswordHash[MD4_SIGNATURE_SIZE];
    unsigned char PasswordHashHash[MD4_SIGNATURE_SIZE];
    unsigned char MasterKey[MD4_SIGNATURE_SIZE];

    /* 128 bit - 2 keys */
    NtPasswordHash(secret, secret_len, PasswordHash);
    md4(PasswordHash, sizeof(PasswordHash), PasswordHashHash);
    mppe_get_master_key(PasswordHashHash, response, MasterKey);
    GetAsymetricStartKey(MasterKey, mppe_master_send_key_128, 16,1, is_server);
    GetAsymetricStartKey(MasterKey, mppe_master_recv_key_128, 16,0, is_server);

    /* 40 bit - 2 keys */
    BCOPY(mppe_master_send_key_128, mppe_master_send_key_40, 8);
    BCOPY(mppe_master_recv_key_128, mppe_master_recv_key_40, 8);

    mppe_allowed = 1;
}


static void
mppe_get_start_key(unsigned char *Challenge, unsigned char *NtPasswordHashHash,
			unsigned char *InitialSessionKey)
{
    unsigned char Digest[SHA_DIGEST_LENGTH];
    SHA_CTX Context;
    
    SHA1_Init(&Context);
    SHA1_Update(&Context, NtPasswordHashHash, 16);
    SHA1_Update(&Context, NtPasswordHashHash, 16);
    SHA1_Update(&Context, Challenge, 8);
    SHA1_Final(Digest, &Context);
    BCOPY(Digest, InitialSessionKey, 16);
}

static void
mppe_get_master_key(unsigned char *PasswordHashHash, unsigned char *NtResponse,
			unsigned char *MasterKey)
{
    unsigned char Digest[SHA_DIGEST_LENGTH];
    SHA_CTX Context;
    static char Magic1[] = "This is the MPPE Master Key";
    
    BZERO(Digest, sizeof(Digest));
    
    SHA1_Init(&Context);
    SHA1_Update(&Context, PasswordHashHash, 16);
    SHA1_Update(&Context, NtResponse, 24);
    SHA1_Update(&Context, Magic1, sizeof(Magic1) - 1);
    SHA1_Final(Digest, &Context);
    
    BCOPY(Digest, MasterKey, 16);
}

static void
GetAsymetricStartKey(unsigned char *MasterKey, unsigned char *SessionKey, 
			int SessionKeyLength, int IsSend, int IsServer)
{
    unsigned char Digest[SHA_DIGEST_LENGTH];
    SHA_CTX Context;
    char *s;
    static char Magic2[] = "On the client side, this is the send key; on the server side, it is the receive key.";
    static char Magic3[] = "On the client side, this is the receive key; on the server side, it is the send key.";
    
    BZERO(Digest, sizeof(Digest));
    if(IsSend)
    {
      if(IsServer)
        s = Magic3;
      else
        s = Magic2;
    }
    else
    {
      if(IsServer)
        s = Magic2;
      else
        s = Magic3;
    }
    
    SHA1_Init(&Context);
    SHA1_Update(&Context, MasterKey, 16);
    SHA1_Update(&Context, SHApad1, 40);
    SHA1_Update(&Context, s, 84);
    SHA1_Update(&Context, SHApad2, 40);
    SHA1_Final(Digest, &Context);
    BCOPY(Digest, SessionKey, SessionKeyLength);
}

/*
 * Functions called from config options
 */
int
setmppe_40(char **argv)
{   
    ccp_allowoptions[0].mppe = ccp_wantoptions[0].mppe = 1;
    ccp_allowoptions[0].mppe_40 = ccp_wantoptions[0].mppe_40 = 1;
    return 1;
}

int
setnomppe_40(char **argv)
{   
    ccp_allowoptions[0].mppe_40 = ccp_wantoptions[0].mppe_40 = 0;
    return 1;
}

int
setmppe_128(char **argv)
{   
    ccp_allowoptions[0].mppe = ccp_wantoptions[0].mppe = 1;
    ccp_allowoptions[0].mppe_128 = ccp_wantoptions[0].mppe_128 = 1;
    return 1;
}

int
setnomppe_128(char **argv)
{   
    ccp_allowoptions[0].mppe_128 = ccp_wantoptions[0].mppe_128 = 0;
    return 1;
}

int
setmppe_stateless(char **argv)
{
    ccp_allowoptions[0].mppe_stateless = ccp_wantoptions[0].mppe_stateless = 1;
    return 1;
}

int
setnomppe_stateless(char **argv)
{
    ccp_allowoptions[0].mppe_stateless = ccp_wantoptions[0].mppe_stateless = 0;
    return 1;
}
#endif /* MPPE */

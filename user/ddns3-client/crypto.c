/*
 *	DDNS v3 Client
 *
 *		Author:		Alan Yates <alany@ay.com.au>
 *		Version:	$Id: crypto.c,v 1.1.1.1 2002/07/19 11:47:20 alany Exp $
 */
#define _XOPEN_SOURCE
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include "md5.h"

static char
*hex(char *s, int len) {
	int i;
	char *buf = (char *) calloc((len * 2) + 1, sizeof(char));

	for(i = 0; i < len; i++)
		sprintf(&(buf[i*2]), "%.2x", s[i] & 0xff);
	buf[len*2] = 0;

	return buf;
}

char
*ddns3_crypto_md5hash(char *s, int len) {
        MD5_CTX context;
        char *buf = (char *) calloc(MD5_DIGEST_CHARS, sizeof(char));
        char *str = (char *) calloc(len, sizeof(char));

        memcpy(str, s, len);
        MD5Init(&context);
        MD5Update(&context, str, len);
        MD5Final(buf, &context);

        free(str);
	str = hex(buf, MD5_DIGEST_CHARS);
	free(buf);
        return str;
}

#ifndef WIN32
char
*ddns3_crypto_crypt(char *key, char *salt) {
	char *buf = (char *) calloc(14, sizeof(char));
	
	memcpy(buf, crypt(key, salt), 13);
	buf[13] = 0;

	return buf;
}
#endif

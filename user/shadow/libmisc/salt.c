/*
 * salt.c - generate a random salt string for crypt()
 *
 * Written by Marek Michalkiewicz <marekm@i17linuxb.ists.pwr.wroc.pl>,
 * public domain.
 */

#include <config.h>

#ident "$Id: salt.c,v 1.10 2005/08/31 17:24:58 kloczek Exp $"

#include <sys/time.h>
#include <stdlib.h>
#include "prototypes.h"
#include "defines.h"
#include "getdef.h"
/*
 * Generate 8 base64 ASCII characters of random salt.  If MD5_CRYPT_ENAB
 * in /etc/login.defs is "yes", the salt string will be prefixed by "$1$"
 * (magic) and pw_encrypt() will execute the MD5-based FreeBSD-compatible
 * version of crypt() instead of the standard one.
 */
char *crypt_make_salt (void)
{
	struct timeval tv;
	static char result[40];

	result[0] = '\0';
#ifndef USE_PAM
	if (getdef_bool ("MD5_CRYPT_ENAB")) {
		strcpy (result, "$1$");	/* magic for the new MD5 crypt() */
	}
#endif

	/*
	 * Generate 8 chars of salt, the old crypt() will use only first 2.
	 */
	gettimeofday (&tv, (struct timezone *) 0);
	strcat (result, l64a (tv.tv_usec));
	strcat (result, l64a (tv.tv_sec + getpid () + clock ()));

	if (strlen (result) > 3 + 8)	/* magic+salt */
		result[11] = '\0';

	return result;
}

#ifndef HAVE_A64L
/* This code was lifted from glibc and is GPL licensed */

/* Conversion table.  */
static const char conv_table[64] =
{
  '.', '/', '0', '1', '2', '3', '4', '5',
  '6', '7', '8', '9', 'A', 'B', 'C', 'D',
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
  'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
  's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

char *l64a (n)
     long int n;
{
  unsigned long int m = (unsigned long int) n;
  static char result[7];
  int cnt;

  if (m == 0l)
    /* The value for N == 0 is defined to be the empty string. */
    return (char *) "";

  result[6] = '\0';

  for (cnt = 5; m > 0; --cnt)
    {
      result[cnt] = conv_table[m & 0x3f];
      m >>= 6;
    }

  return &result[cnt + 1];
}
#endif

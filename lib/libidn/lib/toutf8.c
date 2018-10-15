/* toutf8.c	Convert strings from system locale into UTF-8.
 * Copyright (C) 2002, 2003, 2004  Simon Josefsson
 *
 * This file is part of GNU Libidn.
 *
 * GNU Libidn is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GNU Libidn is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GNU Libidn; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "stringprep.h"

#ifdef _LIBC
# define HAVE_ICONV 1
# define LOCALE_WORKS 1
# define ICONV_CONST
#endif

#ifdef HAVE_ICONV
# include <iconv.h>

# if LOCALE_WORKS
#  include <langinfo.h>
#  include <locale.h>
# endif

# ifdef _LIBC
#  define stringprep_locale_charset() nl_langinfo (CODESET)
# else
/**
 * stringprep_locale_charset:
 *
 * Find out current locale charset.  The function respect the CHARSET
 * environment variable, but typically uses nl_langinfo(CODESET) when
 * it is supported.  It fall back on "ASCII" if CHARSET isn't set and
 * nl_langinfo isn't supported or return anything.
 *
 * Note that this function return the application's locale's preferred
 * charset (or thread's locale's preffered charset, if your system
 * support thread-specific locales).  It does not return what the
 * system may be using.  Thus, if you receive data from external
 * sources you cannot in general use this function to guess what
 * charset it is encoded in.  Use stringprep_convert from the external
 * representation into the charset returned by this function, to have
 * data in the locale encoding.
 *
 * Return value: Return the character set used by the current locale.
 *   It will never return NULL, but use "ASCII" as a fallback.
 **/
const char *
stringprep_locale_charset (void)
{
  const char *charset = getenv ("CHARSET");	/* flawfinder: ignore */

  if (charset && *charset)
    return charset;

#  ifdef LOCALE_WORKS
  charset = nl_langinfo (CODESET);

  if (charset && *charset)
    return charset;
#  endif

  return "ASCII";
}
# endif

/**
 * stringprep_convert:
 * @str: input zero-terminated string.
 * @to_codeset: name of destination character set.
 * @from_codeset: name of origin character set, as used by @str.
 *
 * Convert the string from one character set to another using the
 * system's iconv() function.
 *
 * Return value: Returns newly allocated zero-terminated string which
 *   is @str transcoded into to_codeset.
 **/
char *
stringprep_convert (const char *str,
		    const char *to_codeset, const char *from_codeset)
{
  iconv_t cd;
  char *dest;
  char *outp;
  ICONV_CONST char *p;
  size_t inbytes_remaining;
  size_t outbytes_remaining;
  size_t err;
  size_t outbuf_size;
  int have_error = 0;

  if (strcmp (to_codeset, from_codeset) == 0)
    {
      char *q;
      q = malloc (strlen (str) + 1);
      if (!q)
	return NULL;
      return strcpy (q, str);
    }

  cd = iconv_open (to_codeset, from_codeset);

  if (cd == (iconv_t) - 1)
    return NULL;

  p = (ICONV_CONST char *) str;

  inbytes_remaining = strlen (p);
  /* Guess the maximum length the output string can have.  */
  outbuf_size = (inbytes_remaining + 1) * 5;

  outp = dest = malloc (outbuf_size);
  if (dest == NULL)
    goto out;
  outbytes_remaining = outbuf_size - 1;	/* -1 for NUL */

again:

  err = iconv (cd, (ICONV_CONST char **) &p, &inbytes_remaining,
	       &outp, &outbytes_remaining);

  if (err == (size_t) - 1)
    {
      switch (errno)
	{
	case EINVAL:
	  /* Incomplete text, do not report an error */
	  break;

	case E2BIG:
	  {
	    size_t used = outp - dest;
	    char *newdest;

	    outbuf_size *= 2;
	    newdest = realloc (dest, outbuf_size);
	    if (newdest == NULL)
	      {
		have_error = 1;
		goto out;
	      }
	    dest = newdest;

	    outp = dest + used;
	    outbytes_remaining = outbuf_size - used - 1; /* -1 for NUL */

	    goto again;
	  }
	  break;

	case EILSEQ:
	  have_error = 1;
	  break;

	default:
	  have_error = 1;
	  break;
	}
    }

  *outp = '\0';

  if (*p != '\0')
    have_error = 1;

 out:
  iconv_close (cd);

  if (have_error)
    {
      free (dest);
      dest = NULL;
    }

  return dest;
}

#else /* HAVE_ICONV */

const char *
stringprep_locale_charset ()
{
  return "ASCII";
}

char *
stringprep_convert (const char *str,
		    const char *to_codeset, const char *from_codeset)
{
  char *p;
  fprintf (stderr, "libidn: warning: libiconv not installed, cannot "
	   "convert data to UTF-8\n");
  p = malloc (strlen (str) + 1);
  if (!p)
    return NULL;
  strcpy (p, str);
  return p;
}

#endif /* HAVE_ICONV */

/**
 * stringprep_locale_to_utf8:
 * @str: input zero terminated string.
 *
 * Convert string encoded in the locale's character set into UTF-8 by
 * using stringprep_convert().
 *
 * Return value: Returns newly allocated zero-terminated string which
 *   is @str transcoded into UTF-8.
 **/
char *
stringprep_locale_to_utf8 (const char *str)
{
  return stringprep_convert (str, "UTF-8", stringprep_locale_charset ());
}

/**
 * stringprep_utf8_to_locale:
 * @str: input zero terminated string.
 *
 * Convert string encoded in UTF-8 into the locale's character set by
 * using stringprep_convert().
 *
 * Return value: Returns newly allocated zero-terminated string which
 *   is @str transcoded into the locale's character set.
 **/
char *
stringprep_utf8_to_locale (const char *str)
{
  return stringprep_convert (str, stringprep_locale_charset (), "UTF-8");
}

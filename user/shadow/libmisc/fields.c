/*
 * Copyright 1990, Julianne Frances Haugh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config.h>

#ident "$Id: fields.c,v 1.7 2005/08/31 17:24:57 kloczek Exp $"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "prototypes.h"
/*
 * valid_field - insure that a field contains all legal characters
 *
 * The supplied field is scanned for non-printing and other illegal
 * characters.  If any illegal characters are found, valid_field
 * returns -1.  Zero is returned for success.
 */
int valid_field (const char *field, const char *illegal)
{
	const char *cp;

	for (cp = field;
	     *cp && isprint (*cp & 0x7F) && !strchr (illegal, *cp); cp++);

	if (*cp)
		return -1;
	else
		return 0;
}

/*
 * change_field - change a single field if a new value is given.
 *
 * prompt the user with the name of the field being changed and the
 * current value.
 */

void change_field (char *buf, size_t maxsize, const char *prompt)
{
	char newf[200];
	char *cp;

	if (maxsize > sizeof (newf))
		maxsize = sizeof (newf);

	printf ("\t%s [%s]: ", prompt, buf);
	if (fgets (newf, maxsize, stdin) != newf)
		return;

	if (!(cp = strchr (newf, '\n')))
		return;
	*cp = '\0';

	if (newf[0]) {
		/*
		 * Remove leading and trailing whitespace.  This also
		 * makes it possible to change the field to empty, by
		 * entering a space.  --marekm
		 */

		while (--cp >= newf && isspace (*cp));
		*++cp = '\0';

		cp = newf;
		while (*cp && isspace (*cp))
			cp++;

		strncpy (buf, cp, maxsize - 1);
		buf[maxsize - 1] = '\0';
	}
}

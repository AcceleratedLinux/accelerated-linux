/*
 * Copyright (C) 2004-2007  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2000, 2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: journalprint.c,v 1.12 2007/06/18 23:47:26 tbox Exp $ */

/*! \file */
#include <config.h>

#include <isc/mem.h>
#include <isc/util.h>

#include <dns/journal.h>
#include <dns/types.h>

#include <stdlib.h>

int
main(int argc, char **argv) {
	char *file;
	isc_mem_t *mctx = NULL;

	if (argc != 2) {
		printf("usage: %s journal\n", argv[0]);
		return(1);
	}

	file = argv[1];

	RUNTIME_CHECK(isc_mem_create(0, 0, &mctx) == ISC_R_SUCCESS);

	RUNTIME_CHECK(dns_journal_print(mctx, file, stdout) == ISC_R_SUCCESS);
	isc_mem_detach(&mctx);
	return(0);
}

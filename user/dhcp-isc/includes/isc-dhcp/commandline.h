/*
 * Copyright (C) 2004-2007  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 1999-2001  Internet Software Consortium.
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

/* $Id: commandline.h,v 1.2 2007/11/16 11:04:11 shane Exp $ */

#ifndef ISC_COMMANDLINE_H
#define ISC_COMMANDLINE_H 1

/*! \file isc/commandline.h */

#include <isc-dhcp/boolean.h>
#include <isc-dhcp/lang.h>
/*#include <isc-dhcp/platform.h>*/

/*% Index into parent argv vector. */
extern int isc_commandline_index;
/*% Character checked for validity. */
extern int isc_commandline_option;
/*% Argument associated with option. */
extern char *isc_commandline_argument;
/*% For printing error messages. */
extern char *isc_commandline_progname;
/*% Print error message. */
extern isc_boolean_t isc_commandline_errprint;
/*% Reset getopt. */
extern isc_boolean_t isc_commandline_reset;

ISC_LANG_BEGINDECLS

/*% parse command line */
int
isc_commandline_parse(int argc, char * const *argv, const char *options);

ISC_LANG_ENDDECLS

#endif /* ISC_COMMANDLINE_H */

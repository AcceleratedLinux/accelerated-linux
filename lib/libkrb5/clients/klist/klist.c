/*
 * clients/klist/klist.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
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
 * 
 *
 * List out the contents of your credential cache or keytab.
 */

#include "autoconf.h"
#include <krb5.h>
#include <com_err.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <time.h>
/* Need definition of INET6 before network headers, for IRIX.  */
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif

#ifndef _WIN32
#define GET_PROGNAME(x) (strrchr((x), '/') ? strrchr((x), '/')+1 : (x))
#else
#define GET_PROGNAME(x) max(max(strrchr((x), '/'), strrchr((x), '\\')) + 1,(x))
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#endif

extern int optind;

int show_flags = 0, show_time = 0, status_only = 0, show_keys = 0;
int show_etype = 0, show_addresses = 0, no_resolve = 0, print_version = 0;
char *defname;
char *progname;
krb5_int32 now;
unsigned int timestamp_width;

krb5_context kcontext;

char * etype_string (krb5_enctype );
void show_credential (krb5_creds *);
	
void do_ccache (char *);
void do_keytab (char *);
void printtime (time_t);
void one_addr (krb5_address *);
void fillit (FILE *, unsigned int, int);

#define DEFAULT 0
#define CCACHE 1
#define KEYTAB 2

static void usage()
{
#define KRB_AVAIL_STRING(x) ((x)?"available":"not available")

    fprintf(stderr, "Usage: %s [-e] [-V] [[-c] [-f] [-s] [-a [-n]]] %s",
	     progname, "[-k [-t] [-K]] [name]\n"); 
    fprintf(stderr, "\t-c specifies credentials cache\n");
    fprintf(stderr, "\t-k specifies keytab\n");
    fprintf(stderr, "\t   (Default is credentials cache)\n");
    fprintf(stderr, "\t-e shows the encryption type\n");
    fprintf(stderr, "\t-V shows the Kerberos version and exits\n");
    fprintf(stderr, "\toptions for credential caches:\n");
    fprintf(stderr, "\t\t-f shows credentials flags\n");
    fprintf(stderr, "\t\t-s sets exit status based on valid tgt existence\n");
    fprintf(stderr, "\t\t-a displays the address list\n");
    fprintf(stderr, "\t\t\t-n do not reverse-resolve\n");
    fprintf(stderr, "\toptions for keytabs:\n");
    fprintf(stderr, "\t\t-t shows keytab entry timestamps\n");
    fprintf(stderr, "\t\t-K shows keytab entry DES keys\n");
    exit(1);
}

int
main(argc, argv)
    int argc;
    char **argv;
{
    int c;
    char *name;
    int mode;

    progname = GET_PROGNAME(argv[0]);

    name = NULL;
    mode = DEFAULT;
    /* V=version so v can be used for verbose later if desired.  */
    while ((c = getopt(argc, argv, "fetKsnack45V")) != -1) {
	switch (c) {
	case 'f':
	    show_flags = 1;
	    break;
	case 'e':
	    show_etype = 1;
	    break;
	case 't':
	    show_time = 1;
	    break;
	case 'K':
	    show_keys = 1;
	    break;
	case 's':
	    status_only = 1;
	    break;
	case 'n':
	    no_resolve = 1;
	    break;
	case 'a':
	    show_addresses = 1;
	    break;
	case 'c':
	    if (mode != DEFAULT) usage();
	    mode = CCACHE;
	    break;
	case 'k':
	    if (mode != DEFAULT) usage();
	    mode = KEYTAB;
	    break;
	case '4':
	    fprintf(stderr, "Kerberos 4 is no longer supported\n");
	    exit(3);
	    break;
	case '5':
	    break;
	case 'V':
	    print_version = 1;
	    break;
	default:
	    usage();
	    break;
	}
    }

    if (no_resolve && !show_addresses) {
	usage();
    }

    if (mode == DEFAULT || mode == CCACHE) {
	if (show_time || show_keys)
	    usage();
    } else {
	if (show_flags || status_only || show_addresses)
	    usage();
    }

    if (argc - optind > 1) {
	fprintf(stderr, "Extra arguments (starting with \"%s\").\n",
		argv[optind+1]);
	usage();
    }

    if (print_version) {
	printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	exit(0);
    }

    name = (optind == argc-1) ? argv[optind] : 0;

    now = time(0);
    {
	char tmp[BUFSIZ];

	if (!krb5_timestamp_to_sfstring(now, tmp, 20, (char *) NULL) ||
	    !krb5_timestamp_to_sfstring(now, tmp, sizeof(tmp), 
					(char *) NULL))
	    timestamp_width = (int) strlen(tmp);
	else
	    timestamp_width = 15;
    }

    {
	krb5_error_code retval;
	retval = krb5_init_context(&kcontext);
	if (retval) {
	    com_err(progname, retval, "while initializing krb5");
	    exit(1);
	}

	if (mode == DEFAULT || mode == CCACHE)
	    do_ccache(name);
	else
	    do_keytab(name);
    }

    return 0;
}    

void do_keytab(name)
   char *name;
{
     krb5_keytab kt;
     krb5_keytab_entry entry;
     krb5_kt_cursor cursor;
     char buf[BUFSIZ]; /* hopefully large enough for any type */
     char *pname;
     int code;
     
     if (name == NULL) {
	  if ((code = krb5_kt_default(kcontext, &kt))) {
	       com_err(progname, code, "while getting default keytab");
	       exit(1);
	  }
     } else {
	  if ((code = krb5_kt_resolve(kcontext, name, &kt))) {
	       com_err(progname, code, "while resolving keytab %s",
		       name);
	       exit(1);
	  }
     }

     if ((code = krb5_kt_get_name(kcontext, kt, buf, BUFSIZ))) {
	  com_err(progname, code, "while getting keytab name");
	  exit(1);
     }

     printf("Keytab name: %s\n", buf);
     
     if ((code = krb5_kt_start_seq_get(kcontext, kt, &cursor))) {
	  com_err(progname, code, "while starting keytab scan");
	  exit(1);
     }

     if (show_time) {
	  printf("KVNO Timestamp");
	  fillit(stdout, timestamp_width - sizeof("Timestamp") + 2, (int) ' ');
	  printf("Principal\n");
	  printf("---- ");
	  fillit(stdout, timestamp_width, (int) '-');
	  printf(" ");
	  fillit(stdout, 78 - timestamp_width - sizeof("KVNO"), (int) '-');
	  printf("\n");
     } else {
	  printf("KVNO Principal\n");
	  printf("---- --------------------------------------------------------------------------\n");
     }
     
     while ((code = krb5_kt_next_entry(kcontext, kt, &entry, &cursor)) == 0) {
	  if ((code = krb5_unparse_name(kcontext, entry.principal, &pname))) {
	       com_err(progname, code, "while unparsing principal name");
	       exit(1);
	  }
	  printf("%4d ", entry.vno);
	  if (show_time) {
	       printtime(entry.timestamp);
	       printf(" ");
	  }
	  printf("%s", pname);
	  if (show_etype)
	      printf(" (%s) " , etype_string(entry.key.enctype));
	  if (show_keys) {
	       printf(" (0x");
	       {
		    int i;
		    for (i = 0; i < entry.key.length; i++)
			 printf("%02x", entry.key.contents[i]);
	       }
	       printf(")");
	  }
	  printf("\n");
	  krb5_free_unparsed_name(kcontext, pname);
     }
     if (code && code != KRB5_KT_END) {
	  com_err(progname, code, "while scanning keytab");
	  exit(1);
     }
     if ((code = krb5_kt_end_seq_get(kcontext, kt, &cursor))) {
	  com_err(progname, code, "while ending keytab scan");
	  exit(1);
     }
     exit(0);
}
void do_ccache(name)
   char *name;
{
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ;
    krb5_flags flags;
    krb5_error_code code;
    int	exit_status = 0;
	    
    if (status_only)
	/* exit_status is set back to 0 if a valid tgt is found */
	exit_status = 1;

    if (name == NULL) {
	if ((code = krb5_cc_default(kcontext, &cache))) {
	    if (!status_only)
		com_err(progname, code, "while getting default ccache");
	    exit(1);
	    }
    } else {
	if ((code = krb5_cc_resolve(kcontext, name, &cache))) {
	    if (!status_only)
		com_err(progname, code, "while resolving ccache %s",
			name);
	    exit(1);
	}
    }
 
    flags = 0;				/* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
	if (code == KRB5_FCC_NOFILE) {
	    if (!status_only) {
		com_err(progname, code, "(ticket cache %s:%s)",
			krb5_cc_get_type(kcontext, cache),
			krb5_cc_get_name(kcontext, cache));
#ifdef KRB5_KRB4_COMPAT
		if (name == NULL)
		    do_v4_ccache(0);
#endif
	    }
	} else {
	    if (!status_only)
		com_err(progname, code,
			"while setting cache flags (ticket cache %s:%s)",
			krb5_cc_get_type(kcontext, cache),
			krb5_cc_get_name(kcontext, cache));
	}
	exit(1);
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
	if (!status_only)
	    com_err(progname, code, "while retrieving principal name");
	exit(1);
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
	if (!status_only)
	    com_err(progname, code, "while unparsing principal name");
	exit(1);
    }
    if (!status_only) {
	printf("Ticket cache: %s:%s\nDefault principal: %s\n\n",
	       krb5_cc_get_type(kcontext, cache),
	       krb5_cc_get_name(kcontext, cache), defname);
	fputs("Valid starting", stdout);
	fillit(stdout, timestamp_width - sizeof("Valid starting") + 3,
	       (int) ' ');
	fputs("Expires", stdout);
	fillit(stdout, timestamp_width - sizeof("Expires") + 3,
	       (int) ' ');
	fputs("Service principal\n", stdout);
    }
    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
	if (!status_only)
	    com_err(progname, code, "while starting to retrieve tickets");
	exit(1);
    }
    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
	if (status_only) {
	    if (exit_status && creds.server->length == 2 &&
		strcmp(creds.server->realm.data, princ->realm.data) == 0 &&
		strcmp((char *)creds.server->data[0].data, "krbtgt") == 0 &&
		strcmp((char *)creds.server->data[1].data,
		       princ->realm.data) == 0 && 
		creds.times.endtime > now)
		exit_status = 0;
	} else {
	    show_credential(&creds);
	}
	krb5_free_cred_contents(kcontext, &creds);
    }
    if (code == KRB5_CC_END) {
	if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
	    if (!status_only)
		com_err(progname, code, "while finishing ticket retrieval");
	    exit(1);
	}
	flags = KRB5_TC_OPENCLOSE;	/* turns on OPENCLOSE mode */
	if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
	    if (!status_only)
		com_err(progname, code, "while closing ccache");
	    exit(1);
	}
#ifdef KRB5_KRB4_COMPAT
	if (name == NULL && !status_only)
	    do_v4_ccache(0);
#endif
	exit(exit_status);
    } else {
	if (!status_only)
	    com_err(progname, code, "while retrieving a ticket");
	exit(1);
    }	
}

char *
etype_string(enctype)
    krb5_enctype enctype;
{
    static char buf[100];
    krb5_error_code retval;
    
    if ((retval = krb5_enctype_to_string(enctype, buf, sizeof(buf)))) {
	/* XXX if there's an error != EINVAL, I should probably report it */
	snprintf(buf, sizeof(buf), "etype %d", enctype);
    }

    return buf;
}

static char *
flags_string(cred)
    register krb5_creds *cred;
{
    static char buf[32];
    int i = 0;
	
    if (cred->ticket_flags & TKT_FLG_FORWARDABLE)
	buf[i++] = 'F';
    if (cred->ticket_flags & TKT_FLG_FORWARDED)
	buf[i++] = 'f';
    if (cred->ticket_flags & TKT_FLG_PROXIABLE)
	buf[i++] = 'P';
    if (cred->ticket_flags & TKT_FLG_PROXY)
	buf[i++] = 'p';
    if (cred->ticket_flags & TKT_FLG_MAY_POSTDATE)
	buf[i++] = 'D';
    if (cred->ticket_flags & TKT_FLG_POSTDATED)
	buf[i++] = 'd';
    if (cred->ticket_flags & TKT_FLG_INVALID)
	buf[i++] = 'i';
    if (cred->ticket_flags & TKT_FLG_RENEWABLE)
	buf[i++] = 'R';
    if (cred->ticket_flags & TKT_FLG_INITIAL)
	buf[i++] = 'I';
    if (cred->ticket_flags & TKT_FLG_HW_AUTH)
	buf[i++] = 'H';
    if (cred->ticket_flags & TKT_FLG_PRE_AUTH)
	buf[i++] = 'A';
    if (cred->ticket_flags & TKT_FLG_TRANSIT_POLICY_CHECKED)
	buf[i++] = 'T';
    if (cred->ticket_flags & TKT_FLG_OK_AS_DELEGATE)
	buf[i++] = 'O';		/* D/d are taken.  Use short strings?  */
    if (cred->ticket_flags & TKT_FLG_ANONYMOUS)
	buf[i++] = 'a';
    buf[i] = '\0';
    return(buf);
}

void 
printtime(tv)
    time_t tv;
{
    char timestring[BUFSIZ];
    char fill;

    fill = ' ';
    if (!krb5_timestamp_to_sfstring((krb5_timestamp) tv,
				    timestring,
				    timestamp_width+1,
				    &fill)) {
	printf(timestring);
    }
}

void
show_credential(cred)
    register krb5_creds * cred;
{
    krb5_error_code retval;
    krb5_ticket *tkt;
    char *name, *sname, *flags;
    int	extra_field = 0;

    retval = krb5_unparse_name(kcontext, cred->client, &name);
    if (retval) {
	com_err(progname, retval, "while unparsing client name");
	return;
    }
    retval = krb5_unparse_name(kcontext, cred->server, &sname);
    if (retval) {
	com_err(progname, retval, "while unparsing server name");
	krb5_free_unparsed_name(kcontext, name);
	return;
    }
    if (!cred->times.starttime)
	cred->times.starttime = cred->times.authtime;

    printtime(cred->times.starttime);
    putchar(' '); putchar(' ');
    printtime(cred->times.endtime);
    putchar(' '); putchar(' ');

    printf("%s\n", sname);

    if (strcmp(name, defname)) {
	    printf("\tfor client %s", name);
	    extra_field++;
    }
    
    if (cred->times.renew_till) {
	if (!extra_field)
		fputs("\t",stdout);
	else
		fputs(", ",stdout);
	fputs("renew until ", stdout);
	printtime(cred->times.renew_till);
	extra_field += 2;
    }

    if (extra_field > 3) {
	fputs("\n", stdout);
	extra_field = 0;
    }

    if (show_flags) {
	flags = flags_string(cred);
	if (flags && *flags) {
	    if (!extra_field)
		fputs("\t",stdout);
	    else
		fputs(", ",stdout);
	    printf("Flags: %s", flags);
	    extra_field++;
	}
    }

    if (extra_field > 2) {
	fputs("\n", stdout);
	extra_field = 0;
    }

    if (show_etype) {
	retval = krb5_decode_ticket(&cred->ticket, &tkt);
	if (retval)
	    goto err_tkt;

	if (!extra_field)
	    fputs("\t",stdout);
	else
	    fputs(", ",stdout);
	printf("Etype (skey, tkt): %s, ",
	       etype_string(cred->keyblock.enctype));
	printf("%s ",
	       etype_string(tkt->enc_part.enctype));
	extra_field++;

    err_tkt:
	if (tkt != NULL)
	    krb5_free_ticket(kcontext, tkt);
    }

    /* if any additional info was printed, extra_field is non-zero */
    if (extra_field)
	putchar('\n');


    if (show_addresses) {
	if (!cred->addresses || !cred->addresses[0]) {
	    printf("\tAddresses: (none)\n");
	} else {
	    int i;

	    printf("\tAddresses: ");
	    one_addr(cred->addresses[0]);

	    for (i=1; cred->addresses[i]; i++) {
		printf(", ");
		one_addr(cred->addresses[i]);
	    }

	    printf("\n");
	}
    }

    krb5_free_unparsed_name(kcontext, name);
    krb5_free_unparsed_name(kcontext, sname);
}

#include "port-sockets.h"
#include "socket-utils.h" /* for ss2sin etc */
#include "fake-addrinfo.h"

void one_addr(a)
    krb5_address *a;
{
    struct sockaddr_storage ss;
    int err;
    char namebuf[NI_MAXHOST];

    memset (&ss, 0, sizeof (ss));

    switch (a->addrtype) {
    case ADDRTYPE_INET:
	if (a->length != 4) {
	broken:
	    printf ("broken address (type %d length %d)",
		    a->addrtype, a->length);
	    return;
	}
	{
	    struct sockaddr_in *sinp = ss2sin (&ss);
	    sinp->sin_family = AF_INET;
#ifdef HAVE_SA_LEN
	    sinp->sin_len = sizeof (struct sockaddr_in);
#endif
	    memcpy (&sinp->sin_addr, a->contents, 4);
	}
	break;
#ifdef KRB5_USE_INET6
    case ADDRTYPE_INET6:
	if (a->length != 16)
	    goto broken;
	{
	    struct sockaddr_in6 *sin6p = ss2sin6 (&ss);
	    sin6p->sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
	    sin6p->sin6_len = sizeof (struct sockaddr_in6);
#endif
	    memcpy (&sin6p->sin6_addr, a->contents, 16);
	}
	break;
#endif
    default:
	printf ("unknown addrtype %d", a->addrtype);
	return;
    }

    namebuf[0] = 0;
    err = getnameinfo (ss2sa (&ss), socklen (ss2sa (&ss)),
		       namebuf, sizeof (namebuf), 0, 0,
		       no_resolve ? NI_NUMERICHOST : 0U);
    if (err) {
	printf ("unprintable address (type %d, error %d %s)", a->addrtype, err,
		gai_strerror (err));
	return;
    }
    printf ("%s", namebuf);
}

void
fillit(f, num, c)
    FILE		*f;
    unsigned int	num;
    int			c;
{
    int i;

    for (i=0; i<num; i++)
	fputc(c, f);
}

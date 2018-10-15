/*
 * Copyright 1993 OpenVision Technologies, Inc., All Rights Reserved.
 *
 * $Id: keytab.c 19709 2007-07-12 23:35:24Z raeburn $
 * $Source$
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

#if !defined(lint) && !defined(__CODECENTER__)
static char *rcsid = "$Header$";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "k5-int.h"
#include <kadm5/admin.h>
#include <adm_proto.h>
#include "kadmin.h"

static int add_principal(void *lhandle, char *keytab_str, krb5_keytab keytab,
			 krb5_boolean keepold,
			 int n_ks_tuple, krb5_key_salt_tuple *ks_tuple,
			 char *princ_str);
static int remove_principal(char *keytab_str, krb5_keytab keytab, char
			    *princ_str, char *kvno_str);
static char *etype_string(krb5_enctype enctype);

static int quiet;

#ifdef KADMIN_LOCAL
static int norandkey;
#endif

static void add_usage()
{
#ifdef KADMIN_LOCAL
     fprintf(stderr, "Usage: ktadd [-k[eytab] keytab] [-q] [-e keysaltlist] [-norandkey] [principal | -glob princ-exp] [...]\n");
#else
     fprintf(stderr, "Usage: ktadd [-k[eytab] keytab] [-q] [-e keysaltlist] [principal | -glob princ-exp] [...]\n");
#endif
}
     
static void rem_usage()
{
     fprintf(stderr, "Usage: ktremove [-k[eytab] keytab] [-q] principal [kvno|\"all\"|\"old\"]\n");
}

static int process_keytab(krb5_context my_context, char **keytab_str,
		   krb5_keytab *keytab) 
{
     int code;
     
     if (*keytab_str == NULL) {
	  /* XXX krb5_defkeyname is an internal library global and
             should go away */
	  if (! (*keytab_str = strdup(krb5_defkeyname))) {
	       com_err(whoami, ENOMEM, "while creating keytab name");
	       return 1;
	  }
	  code = krb5_kt_default(my_context, keytab);
	  if (code != 0) {
	       com_err(whoami, code, "while opening default keytab");
	       free(*keytab_str);
	       return 1;
	  }
     } else {
	  if (strchr(*keytab_str, ':') != NULL) {
	       *keytab_str = strdup(*keytab_str);
	       if (*keytab_str == NULL) {
		    com_err(whoami, ENOMEM, "while creating keytab name");
		    return 1;
	       }
	  } else {
	       if (asprintf(keytab_str, "WRFILE:%s", *keytab_str) < 0) {
		   *keytab_str = NULL;
		   com_err(whoami, ENOMEM, "while creating keytab name");
		   return 1;
	       }
	  }
	  
	  code = krb5_kt_resolve(my_context, *keytab_str, keytab);
	  if (code != 0) {
	       com_err(whoami, code, "while resolving keytab %s", *keytab_str);
	       free(keytab_str);
	       return 1;
	  }
     }
     
     return 0;
}

     
void kadmin_keytab_add(int argc, char **argv)
{
     krb5_keytab keytab = 0;
     char *keytab_str = NULL, **princs;
     int code, num, i;
     krb5_error_code retval;
     int n_ks_tuple = 0;
     krb5_boolean keepold = FALSE;
     krb5_key_salt_tuple *ks_tuple = NULL;

     argc--; argv++;
     quiet = 0;
#ifdef KADMIN_LOCAL
     norandkey = 0;
#endif
     while (argc) {
	  if (strncmp(*argv, "-k", 2) == 0) {
	       argc--; argv++;
	       if (!argc || keytab_str) {
		    add_usage();
		    return;
	       }
	       keytab_str = *argv;
	  } else if (strcmp(*argv, "-q") == 0) {
	       quiet++;
#ifdef KADMIN_LOCAL
        } else if (strcmp(*argv, "-norandkey") == 0) {
             norandkey++;
#endif
	  } else if (strcmp(*argv, "-e") == 0) {
	       argc--;
	       if (argc < 1) {
		    add_usage();
		    return;
	       }
	       retval = krb5_string_to_keysalts(*++argv, ", \t", ":.-", 0,
						&ks_tuple, &n_ks_tuple);
	       if (retval) {
		    com_err("ktadd", retval, "while parsing keysalts %s",
			    *argv);

		    return;
	       }
	  } else
	       break;
	  argc--; argv++;
     }

     if (argc == 0) {
	  add_usage();
	  return;
     }

#ifdef KADMIN_LOCAL
     if (norandkey && ks_tuple) {
       fprintf(stderr, "cannot specify keysaltlist when not changing key\n");
       return;
     }
#endif

     if (process_keytab(context, &keytab_str, &keytab))
	  return;
     
     while (*argv) {
	  if (strcmp(*argv, "-glob") == 0) {
	       if (*++argv == NULL) {
		    add_usage();
		    break;
	       }
	       
	       code = kadm5_get_principals(handle, *argv, &princs, &num);
	       if (code) {
		    com_err(whoami, code, "while expanding expression \"%s\".",
			    *argv);
		    argv++;
		    continue;
	       }
	       
	       for (i = 0; i < num; i++) 
		    (void) add_principal(handle, keytab_str, keytab,
					 keepold, n_ks_tuple, ks_tuple,
					 princs[i]); 
	       kadm5_free_name_list(handle, princs, num);
	  } else
	       (void) add_principal(handle, keytab_str, keytab,
				    keepold, n_ks_tuple, ks_tuple,
				    *argv);
	  argv++;
     }
	  
     code = krb5_kt_close(context, keytab);
     if (code != 0)
	  com_err(whoami, code, "while closing keytab");

     free(keytab_str);
}

void kadmin_keytab_remove(int argc, char **argv)
{
     krb5_keytab keytab = 0;
     char *keytab_str = NULL;
     int code;

     argc--; argv++;
     quiet = 0;
     while (argc) {
	  if (strncmp(*argv, "-k", 2) == 0) {
	       argc--; argv++;
	       if (!argc || keytab_str) {
		    rem_usage();
		    return;
	       }
	       keytab_str = *argv;
	  } else if (strcmp(*argv, "-q") == 0) {
	       quiet++;
	  } else
	       break;
	  argc--; argv++;
     }

     if (argc != 1 && argc != 2) {
	  rem_usage();
	  return;
     }
     if (process_keytab(context, &keytab_str, &keytab))
	  return;

     (void) remove_principal(keytab_str, keytab, argv[0], argv[1]);

     code = krb5_kt_close(context, keytab);
     if (code != 0)
	  com_err(whoami, code, "while closing keytab");

     free(keytab_str);
}

static 
int add_principal(void *lhandle, char *keytab_str, krb5_keytab keytab,
		  krb5_boolean keepold, int n_ks_tuple,
		  krb5_key_salt_tuple *ks_tuple,
		  char *princ_str) 
{
     kadm5_principal_ent_rec princ_rec;
     krb5_principal princ;
     krb5_keytab_entry new_entry;
     krb5_keyblock *keys;
     int code, nkeys, i;

     (void) memset((char *)&princ_rec, 0, sizeof(princ_rec));

     princ = NULL;
     keys = NULL;
     nkeys = 0;

     code = krb5_parse_name(context, princ_str, &princ);
     if (code != 0) {
	  com_err(whoami, code, "while parsing -add principal name %s",
		  princ_str);
	  goto cleanup;
     }

#ifdef KADMIN_LOCAL
     if (norandkey)
       code = kadm5_get_principal_keys(handle, princ, &keys, &nkeys);
     else
#endif
     if (keepold || ks_tuple != NULL) {
	 code = kadm5_randkey_principal_3(lhandle, princ,
					  keepold, n_ks_tuple, ks_tuple,
					  &keys, &nkeys);
     } else {
	 code = kadm5_randkey_principal(lhandle, princ, &keys, &nkeys);
     }
     if (code != 0) {
	  if (code == KADM5_UNK_PRINC) {
	       fprintf(stderr, "%s: Principal %s does not exist.\n",
		       whoami, princ_str);
	  } else
	       com_err(whoami, code, "while changing %s's key",
		       princ_str);
	  goto cleanup;
     }

     code = kadm5_get_principal(lhandle, princ, &princ_rec,
				KADM5_PRINCIPAL_NORMAL_MASK);
     if (code != 0) {
	  com_err(whoami, code, "while retrieving principal");
	  goto cleanup;
     }

     for (i = 0; i < nkeys; i++) {
	  memset((char *) &new_entry, 0, sizeof(new_entry));
	  new_entry.principal = princ;
	  new_entry.key = keys[i];
	  new_entry.vno = princ_rec.kvno;

	  code = krb5_kt_add_entry(context, keytab, &new_entry);
	  if (code != 0) {
	       com_err(whoami, code, "while adding key to keytab");
	       (void) kadm5_free_principal_ent(lhandle, &princ_rec);
	       goto cleanup;
	  }

	  if (!quiet)
	       printf("Entry for principal %s with kvno %d, "
		      "encryption type %s added to keytab %s.\n",
		      princ_str, princ_rec.kvno,
		      etype_string(keys[i].enctype), keytab_str);
     }

     code = kadm5_free_principal_ent(lhandle, &princ_rec);
     if (code != 0) {
	  com_err(whoami, code, "while freeing principal entry");
	  goto cleanup;
     }

cleanup:
     if (nkeys) {
	  for (i = 0; i < nkeys; i++)
	       krb5_free_keyblock_contents(context, &keys[i]);
	  free(keys);
     }
     if (princ)
	  krb5_free_principal(context, princ);

     return code;
}

int remove_principal(char *keytab_str, krb5_keytab keytab, char
		     *princ_str, char *kvno_str) 
{
     krb5_principal princ;
     krb5_keytab_entry entry;
     krb5_kt_cursor cursor;
     enum { UNDEF, SPEC, HIGH, ALL, OLD } mode;
     int code, did_something;
     krb5_kvno kvno;

     code = krb5_parse_name(context, princ_str, &princ);
     if (code != 0) {
	  com_err(whoami, code, "while parsing principal name %s",
		  princ_str);
	  return code;
     }

     mode = UNDEF;
     if (kvno_str == NULL) {
	  mode = HIGH;
	  kvno = 0;
     } else if (strcmp(kvno_str, "all") == 0) {
	  mode = ALL;
	  kvno = 0;
     } else if (strcmp(kvno_str, "old") == 0) {
	  mode = OLD;
	  kvno = 0;
     } else {
	  mode = SPEC;
	  kvno = atoi(kvno_str);
     }

     /* kvno is set to specified value for SPEC, 0 otherwise */
     code = krb5_kt_get_entry(context, keytab, princ, kvno, 0, &entry);
     if (code != 0) {
	  if (code == ENOENT) {
	       fprintf(stderr, "%s: Keytab %s does not exist.\n",
		       whoami, keytab_str);
	  } else if (code == KRB5_KT_NOTFOUND) {
	       if (mode != SPEC)
		    fprintf(stderr, "%s: No entry for principal "
			    "%s exists in keytab %s\n",
			    whoami, princ_str, keytab_str);
	       else
		    fprintf(stderr, "%s: No entry for principal "
			    "%s with kvno %d exists in keytab "
			    "%s.\n", whoami, princ_str, kvno,
			    keytab_str);
	  } else {
	       com_err(whoami, code, "while retrieving highest kvno "
		       "from keytab");
	  }
	  return code;
     }

     /* set kvno to spec'ed value for SPEC, highest kvno otherwise */
     kvno = entry.vno;
     krb5_kt_free_entry(context, &entry);

     code = krb5_kt_start_seq_get(context, keytab, &cursor);
     if (code != 0) {
	  com_err(whoami, code, "while starting keytab scan");
	  return code;
     }

     did_something = 0;
     while ((code = krb5_kt_next_entry(context, keytab, &entry, &cursor)) == 0) {
	  if (krb5_principal_compare(context, princ, entry.principal) &&
	      ((mode == ALL) ||
	       (mode == SPEC && entry.vno == kvno) ||
	       (mode == OLD && entry.vno != kvno) ||
	       (mode == HIGH && entry.vno == kvno))) {

	       /*
		* Ack!  What a kludge... the scanning functions lock
		* the keytab so entries cannot be removed while they
		* are operating.
		*/
	       code = krb5_kt_end_seq_get(context, keytab, &cursor);
	       if (code != 0) {
		    com_err(whoami, code, "while temporarily ending "
			    "keytab scan");
		    return code;
	       }
	       code = krb5_kt_remove_entry(context, keytab, &entry);
	       if (code != 0) {
		    com_err(whoami, code, "while deleting entry from keytab");
		    return code;
	       }
	       code = krb5_kt_start_seq_get(context, keytab, &cursor);
	       if (code != 0) {
		    com_err(whoami, code, "while restarting keytab scan");
		    return code;
	       }

	       did_something++;
	       if (!quiet)
		    printf("Entry for principal %s with kvno %d "
			   "removed from keytab %s.\n", 
			   princ_str, entry.vno, keytab_str);
	  }
	  krb5_kt_free_entry(context, &entry);
     }
     if (code && code != KRB5_KT_END) {
	  com_err(whoami, code, "while scanning keytab");
	  return code;
     }
     if ((code = krb5_kt_end_seq_get(context, keytab, &cursor))) {
	  com_err(whoami, code, "while ending keytab scan");
	  return code;
     }

     /*
      * If !did_someting then mode must be OLD or we would have
      * already returned with an error.  But check it anyway just to
      * prevent unexpected error messages...
      */
     if (!did_something && mode == OLD) {
	  fprintf(stderr, "%s: There is only one entry for principal "
		  "%s in keytab %s\n", whoami, princ_str, keytab_str);
	  return 1;
     }
     
     return 0;
}

/*
 * etype_string(enctype): return a string representation of the
 * encryption type.  XXX copied from klist.c; this should be a
 * library function, or perhaps just #defines
 */
static char *etype_string(enctype)
    krb5_enctype enctype;
{
    static char buf[100];
    krb5_error_code ret;

    if ((ret = krb5_enctype_to_string(enctype, buf, sizeof(buf))))
	snprintf(buf, sizeof(buf), "etype %d", enctype);

    return buf;
}

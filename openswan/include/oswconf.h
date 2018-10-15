/* misc functions to get compile time and runtime options
 * Copyright (C) 2005 Michael Richardson <mcr@xelerance.com>
 * Copyright (C) 2009 Paul Wouters <paul@xelerance.com>
 * Copyright (C) 2009 Avesh Agarwal <avagarwa@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

#ifndef _OSW_CONF_H
#define _OSW_CONF_H

#include "constants.h"

#ifdef HAVE_LIBNSS
# include <nss.h>
# include <pk11pub.h>
#endif

struct paththing {
  char    *path;
  size_t   path_space;
};


struct osw_conf_options {
    char *rootdir;                /* default is "" --- used for testing */
    char *confdir;                /* "/etc" */
    char *conffile;               /* "/etc/ipsec.conf" */
    char *confddir;               /* "/etc/ipsec.d" */
    char *vardir;                 /* "/var/run/pluto" */
    char *policies_dir;           /* "/etc/ipsec.d/policies" */
    char *acerts_dir;             /* "/etc/ipsec.d/acerts" */
    char *cacerts_dir;            /* "/etc/ipsec.d/cacerts" */
    char *crls_dir;               /* "/etc/ipsec.d/crls" */    
    char *private_dir;            /* "/etc/ipsec.d/private" */
    char *certs_dir;              /* "/etc/ipsec.d/certs" */
    char *aacerts_dir;            /* "/etc/ipsec.d/aacerts" */
    char *ocspcerts_dir;          /* "/etc/ipsec.d/ocspcerts" */
};

#ifdef HAVE_LIBNSS
typedef struct {
    enum {
      PW_NONE = 0,      /* no password */
      PW_FROMFILE = 1,  /* password data in a text file */
      PW_PLAINTEXT = 2, /* password data in the clear in memory buffer */
      PW_EXTERNAL = 3   /* external source, user will be prompted */
    } source ;
    char *data;
} secuPWData;
#endif

extern const struct osw_conf_options *osw_init_options(void);
extern void osw_conf_free_oco(void);
extern const struct osw_conf_options *osw_init_ipsecdir(const char *ipsec_dir);
extern const struct osw_conf_options *osw_init_rootdir(const char *root_dir);

#ifdef HAVE_LIBNSS
extern secuPWData *osw_return_nss_password_file_info(void);
extern char *getNSSPassword(PK11SlotInfo *slot, PRBool retry, void *arg);
extern bool Pluto_IsFIPS(void);
#endif

#endif /* _OSW_ALLOC_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: pluto
 * End:
 */

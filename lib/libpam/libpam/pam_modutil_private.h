#ifndef PAMMODUTIL_PRIVATE_H
#define PAMMODUTIL_PRIVATE_H

/*
 * $Id: pam_modutil_private.h,v 1.1 2005/09/21 10:00:58 t8m Exp $
 *
 * Copyright (c) 2001 Andrew Morgan <morgan@kernel.org>
 */

#include "config.h"

#include <security/_pam_macros.h>
#include <security/pam_modules.h>
#include <security/pam_modutil.h>

#define PWD_INITIAL_LENGTH     0x100
#define PWD_ABSURD_PWD_LENGTH  0x8000

extern void
pam_modutil_cleanup(pam_handle_t *pamh, void *data,
                    int error_status);

#endif /* PAMMODUTIL_PRIVATE_H */

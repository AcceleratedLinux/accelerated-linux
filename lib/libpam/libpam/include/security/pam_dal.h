
/* DAL extensions to libpam */

#ifndef _SECURITY__PAM_DAL_H_
#define _SECURITY__PAM_DAL_H_

#include <security/_pam_types.h>
#include <sys/types.h>

extern int PAM_NONNULL((1,2))
pam_dal_set_groupname_from_attr(pam_handle_t *pamh,
				const char *groupattr,
				size_t groupattr_len);

#endif /* _SECURITY__PAM_DAL_H_ */

#ifndef _OPENGEAR_SESSION_H_
#define _OPENGEAR_SESSION_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_session(const char *cookie);
const char *get_username();
bool get_auth_success();
char *get_session_hash(const char *usage);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_SESSION_H_ */

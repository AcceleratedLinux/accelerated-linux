#ifndef _PORTUSERS_H
#define	_PORTUSERS_H

#include <stdint.h>
#include <stdbool.h>
#include <opengear/queue.h>

#ifdef	__cplusplus
extern "C" {
#endif

/** A structure containing all statistics for all ports */
typedef struct portuser {
	int port;
	char name[64];
	char label[64];
	TAILQ_ENTRY(portuser) list;
} portuser_t;

typedef TAILQ_HEAD(portusers, portuser) portusers_t;

extern bool opengear_get_port_users(portusers_t *);

extern char *opengear_get_port_users_for_port(int port);

extern int opengear_disconnect_port_users(const char *name, const int port, int *count);

#ifdef	__cplusplus
}
#endif

#endif	/* _PORTUSERS_H */


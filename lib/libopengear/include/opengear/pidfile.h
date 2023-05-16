#ifndef _OPENGEAR_PIDFILE_H_
#define _OPENGEAR_PIDFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OG_LOCKFILE_PATH "/var/lock/"
#define OG_PIDFILE_PATH "/var/run/"

/* Pidfiles are at OG_PIDFILE_PATH/name.pid, or can 
 * be at any path with the _path version of the routines*/
int opengear_pidfile_kill(const char *name, int sig);
int opengear_pidfile_create(const char *name);
void opengear_pidfile_clear(const char *name);
int opengear_pidfile_valid(const char *name);

/* Lockfiles are at OG_LOCKFILE_PATH/name.lck, or can
 * be at any path with the _path versions of the routines */
int opengear_lockfile_kill(const char *name, int sig);
int opengear_lockfile_create(const char *name);
void opengear_lockfile_clear(const char *name);
int opengear_lockfile_valid(const char *name);

/* Name is used for logging, can be NULL */
int opengear_pidfile_kill_path(const char *name, const char *path, int sig);
int opengear_pidfile_create_path(const char *path);
void opengear_pidfile_clear_path(const char *path);
int opengear_pidfile_valid_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_PIDFILE_H_ */

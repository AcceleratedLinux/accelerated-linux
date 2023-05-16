#define _GNU_SOURCE
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include <opengear/pidfile.h>

int opengear_pidfile_kill(const char *name, int sig)
{
	char *path = NULL;
        int path_valid = asprintf(&path,  OG_PIDFILE_PATH "%s.pid", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for pidfile path");	
		return (-1);
	}

	success = opengear_pidfile_kill_path(name, path, sig);
	free(path);
	return success;
}

int opengear_lockfile_kill(const char *name, int sig)
{
	char *path = NULL;
        int path_valid = asprintf(&path, OG_LOCKFILE_PATH "%s.lck", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for lockfile path");	
		return (-1);
	}

	success = opengear_pidfile_kill_path(name, path, sig);
	free(path);
	return success;
}

int opengear_pidfile_kill_path(const char *name, const char *path, int sig)
{
        struct stat statbuf;
	const char *default_name = "process";
        if (stat(path, &statbuf) != 0) {
                syslog(LOG_DEBUG, "Failed to signal %s: no such file '%s'",
                        name, path);
                return (-1);
        }
	if (name != NULL) {
		default_name = name;
	}

        FILE *pidfile;
        pidfile = fopen(path, "r");
        if (pidfile == NULL) {
                syslog(LOG_ERR, "Failed to signal %s: could not open file '%s'",
                        default_name, path);
                return (-1);
        }
        pid_t pid;
        if (fscanf(pidfile, "%d", &pid) == EOF) {
                syslog(LOG_ERR, "Failed to signal %s: "
                                "error reading pid from file '%s'",
                        default_name, path);
                errno = EINVAL;
                fclose(pidfile);
                return (-1);
        }
        if (pid == -1 || pid == 0 || pid == 1) {
                syslog(LOG_ERR, "Failed to signal %s: invalid pid '%d'",
                        default_name, pid);
                errno = EINVAL;
                fclose(pidfile);
                return (-1);
        }

        int rc = kill((pid_t)pid, sig);
        if (rc != 0) {
                syslog(LOG_ERR, "Failed to signal %s pid '%d': kill returned %d",
                        default_name, pid, rc);
        }
        fclose(pidfile);
        return (rc);
}

int opengear_pidfile_create(const char *name)
{
	char *path = NULL;
        int path_valid = asprintf(&path, OG_PIDFILE_PATH "%s.pid", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for pidfile path");	
		return (-1);
	}
	success = opengear_pidfile_create_path(path);
	free(path);
	return success;
}

int opengear_lockfile_create(const char *name)
{
	char *path = NULL;
        int path_valid = asprintf(&path, OG_LOCKFILE_PATH "%s.lck", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for lockfile path");	
		return (-1);
	}
	success = opengear_pidfile_create_path(path);
	free(path);
	return success;
}

int opengear_pidfile_create_path(const char *path)
{
        FILE *pidfile;
        pidfile = fopen(path, "w");
        if (pidfile == NULL) {
                syslog(LOG_ERR, "Could not open file '%s'",
                        path);
                return (-1);
        }
        pid_t pid = getpid();
        fprintf(pidfile, "%d", pid);
        fclose(pidfile);
        return 0;
}

void opengear_pidfile_clear(const char *name)
{
	char * path = NULL;
        int path_valid = asprintf(&path, OG_PIDFILE_PATH "%s.pid", name);
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for pidfile path");	
		return;
	}
	opengear_pidfile_clear_path(path);
	free(path);
	return;
}

void opengear_lockfile_clear(const char *name)
{
	char * path = NULL;
        int path_valid = asprintf(&path, OG_LOCKFILE_PATH "%s.lck", name);
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for lockfile path");	
		return;
	}
	opengear_pidfile_clear_path(path);
	free(path);
	return;
}

void opengear_pidfile_clear_path(const char *path)
{
        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
                unlink(path);
        }
	return;
}

int opengear_pidfile_valid(const char *name)
{
	char * path = NULL;
        int path_valid = asprintf(&path, OG_PIDFILE_PATH "%s.pid", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for pidfile path");	
		return (-1);
	}

	success = opengear_pidfile_valid_path(path);
	free(path);
	return success;
}

int opengear_lockfile_valid(const char *name)
{
	char * path = NULL;
        int path_valid = asprintf(&path, OG_LOCKFILE_PATH "%s.lck", name);
	int success = 0;
	if (path_valid < 0) {
		syslog(LOG_ERR, "Could not allocate memory for lockfile path");	
		return (-1);
	}

	success = opengear_pidfile_valid_path(path);
	free(path);
	return success;
}

int opengear_pidfile_valid_path(const char *path)
{
        FILE *pidfile;
        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
                pid_t pid;
                pidfile = fopen(path, "r");
                if (pidfile == NULL) {
                        unlink(path);
                        return (-1);
                }
                if (fscanf(pidfile, "%d", &pid) == EOF) {
                        fclose(pidfile);
                        unlink(path);
                        return (-1);
                }
                if (pid == -1 || pid == 0 || pid == 1) {
                        fclose(pidfile);
                        unlink(path);
                        return (-1);
                }
                if (kill(pid, 0) == 0) {
                        /* Still valid */
                        fclose(pidfile);
                        return 0; 
                } else {
                        fclose(pidfile);
                        unlink(path);
                        return (-1);

                }
        }
        return -1; 
}

/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */

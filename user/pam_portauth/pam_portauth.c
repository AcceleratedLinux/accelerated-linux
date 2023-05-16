#define _GNU_SOURCE	/* getline() */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslog.h>
#include <stdarg.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_misc.h>			/* D() for debug */

#define USERPORT_MAX 256

static char * pam_module_name = "pam_portauth";

/*
 * PAM authentication module that will authenticate username 'unauth'
 * if OG-PMSHELL is set to one of the names in /etc/config/portauth.
 */

/* Test if entry string appears as a line by itself in /etc/config/portauth */
static _Bool
in_portauth(const char *entry)
{
	FILE *portauth = fopen("/etc/config/portauth", "r");
	if (!portauth) {
		return 0;
	}

	_Bool found = 0;
	char *line = NULL;
	size_t linesz = 0;
	while (getline(&line, &linesz, portauth) != -1) {
		/* Remove trailing newline */
		if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
			line[strlen(line) - 1] = '\0';
		}

		if (strcmp(line, entry) == 0) {
			found = 1;
			break;
		}
	}
	free(line);
	fclose(portauth);

	return found;
}

static void _pam_log(int prio, char *format, ...)
{
        va_list args;
        char buffer[1024];

        va_start(args, format);
        vsprintf(buffer, format, args);
        /* don't do openlog or closelog, but put our name in to be friendly */
        syslog(prio, "%s: %s", pam_module_name, buffer);
        va_end(args);
}

/* Creates the "user:port" string from PAM_USER and OG-PMSHELL */
static _Bool
make_userport(pam_handle_t *pamh, char userport[static USERPORT_MAX])
{
	int error;
	const char *username;
	const char *og_pmshell;
	int n;

	/* Expect a bare username to have been supplied */
	username = NULL;
	error = pam_get_item(pamh, PAM_USER, (const void **)&username);
	if (error != PAM_SUCCESS) {
		return 0;
	}
	D(("username=%s", username));

	/* Expect OG-PMSHELL to be have been set in the PAM environment */
	og_pmshell = pam_getenv(pamh, "OG-PMSHELL");
	D(("og_pmshell=%s", og_pmshell));
	if (!og_pmshell || !*og_pmshell) {
		return 0;
	}

	/* Construct <user>:<port> string */
	n = snprintf(userport, USERPORT_MAX, "%s:%s", username, og_pmshell);
	if (n >= USERPORT_MAX) {
		return 0;		/* username too long */
	}
	D(("userport=%s", userport));

	return 1;
}

/*
 * Handle pam.conf "auth" directives referring to this module.
 * Returns PAM_SUCCESS if the user does not require authentication,
 * Returns PAM_DENIED otherwise.
 */
PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	char userport[USERPORT_MAX];

	D(("pam_sm_authenticate"));

	if (!make_userport(pamh, userport))
		return PAM_IGNORE;

	/* Check if <user>:<port> is permitted */
	if (in_portauth(userport)) {
		_pam_log(LOG_INFO, "%s: permit %s", pam_module_name, userport);
		pam_set_data(pamh, pam_module_name, "OK", NULL);
		return PAM_SUCCESS;
	} else {
		_pam_log(LOG_INFO, "%s: deny %s", pam_module_name, userport);
		return PAM_PERM_DENIED;
	}
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_IGNORE;
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	char userport[USERPORT_MAX];

	if (!make_userport(pamh, userport))
		return PAM_IGNORE;
	return in_portauth(userport) ? PAM_SUCCESS : PAM_PERM_DENIED;
}

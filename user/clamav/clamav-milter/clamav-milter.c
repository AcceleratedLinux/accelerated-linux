/*
 *  Copyright (C)2008 Sourcefire, Inc.
 *
 *  Author: aCaB <acab@clamav.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#ifdef USE_SYSLOG
#include <syslog.h>
#endif
#include <time.h>
#include <libmilter/mfapi.h>

#include "clamav.h"

#include "shared/output.h"
#include "shared/optparser.h"
#include "shared/misc.h"

#include "connpool.h"
#include "netcode.h"
#include "clamfi.h"
#include "whitelist.h"

struct smfiDesc descr;

int main(int argc, char **argv) {
    char *my_socket, *pt;
    const struct optstruct *opt;
    struct optstruct *opts;
    time_t currtime;
    int ret;

    memset(&descr, 0, sizeof(struct smfiDesc));
    descr.xxfi_name = "ClamAV";			/* filter name */
    descr.xxfi_version = SMFI_VERSION;		/* milter version */
    descr.xxfi_flags = SMFIF_QUARANTINE;	/* flags */
    descr.xxfi_connect = clamfi_connect;	/* connection info filter */
    descr.xxfi_envfrom = clamfi_envfrom;	/* envelope sender filter */
    descr.xxfi_envrcpt = clamfi_envrcpt;	/* envelope recipient filter */
    descr.xxfi_header = clamfi_header;		/* header filter */
    descr.xxfi_body = clamfi_body;		/* body block */
    descr.xxfi_eom = clamfi_eom;		/* end of message */
    descr.xxfi_abort = clamfi_abort;		/* message aborted */

    opts = optparse(NULL, argc, argv, 1, OPT_MILTER, 0, NULL);
    if (!opts) {
	mprintf("!Can't parse command line options\n");
	return 1;
    }

    if(optget(opts, "help")->enabled) {
	printf("Usage: %s [-c <config-file>]\n\n", argv[0]);
	printf("    --help                   -h       Show this help\n");
	printf("    --version                -V       Show version and exit\n");
	printf("    --config-file <file>     -c       Read configuration from file\n\n");
	optfree(opts);
	return 0;
    }

    if(opts->filename) {
	int x;
	for(x = 0; opts->filename[x]; x++)
	    mprintf("^Ignoring option %s\n", opts->filename[x]);
    }

    if(optget(opts, "version")->enabled) {
	printf("clamav-milter %s\n", get_version());
	optfree(opts);
	return 0;
    }

    pt = strdup(optget(opts, "config-file")->strarg);
    if((opts = optparse(pt, 0, NULL, 1, OPT_MILTER, 0, opts)) == NULL) {
	printf("%s: cannot parse config file %s\n", argv[0], pt);
	free(pt);
	return 1;
    }
    free(pt);

    if((opt = optget(opts, "Chroot"))->enabled) {
	if(chdir(opt->strarg) != 0) {
	    logg("!Cannot change directory to %s\n", opt->strarg);
	    return 1;
	}
	if(chroot(opt->strarg) != 0) {
	    logg("!chroot to %s failed. Are you root?\n", opt->strarg);
	    return 1;
	}
    }

    if(geteuid() == 0 && (opt = optget(opts, "User"))->enabled) {
        struct passwd *user = NULL;
	if((user = getpwnam(opt->strarg)) == NULL) {
	    fprintf(stderr, "ERROR: Can't get information about user %s.\n", opt->strarg);
	    optfree(opts);
	    return 1;
	}

	if(optget(opts, "AllowSupplementaryGroups")->enabled) {
#ifdef HAVE_INITGROUPS
	    if(initgroups(opt->strarg, user->pw_gid)) {
		fprintf(stderr, "ERROR: initgroups() failed.\n");
		optfree(opts);
		return 1;
	    }
#else
	    mprintf("!AllowSupplementaryGroups: initgroups() is not available, please disable AllowSupplementaryGroups\n");
	    optfree(opts);
	    return 1;
#endif
	} else {
#ifdef HAVE_SETGROUPS
	    if(setgroups(1, &user->pw_gid)) {
		fprintf(stderr, "ERROR: setgroups() failed.\n");
		optfree(opts);
		return 1;
	    }
#endif
	}

	if(setgid(user->pw_gid)) {
	    fprintf(stderr, "ERROR: setgid(%d) failed.\n", (int) user->pw_gid);
	    optfree(opts);
	    return 1;
	}

	if(setuid(user->pw_uid)) {
	    fprintf(stderr, "ERROR: setuid(%d) failed.\n", (int) user->pw_uid);
	    optfree(opts);
	    return 1;
	}
    }

    logg_lock = !optget(opts, "LogFileUnlock")->enabled;
    logg_time = optget(opts, "LogTime")->enabled;
    logg_size = optget(opts, "LogFileMaxSize")->numarg;
    logg_verbose = mprintf_verbose = optget(opts, "LogVerbose")->enabled;

    if((opt = optget(opts, "LogFile"))->enabled) {
	logg_file = opt->strarg;
	if(strlen(logg_file) < 2 || logg_file[0] != '/') {
	    fprintf(stderr, "ERROR: LogFile requires full path.\n");
	    logg_close();
	    optfree(opts);
	    return 1;
	}
    } else
	logg_file = NULL;

#if defined(USE_SYSLOG) && !defined(C_AIX)
    if(optget(opts, "LogSyslog")->enabled) {
	int fac;

	opt = optget(opts, "LogFacility");
	if((fac = logg_facility(opt->strarg)) == -1) {
	    logg("!LogFacility: %s: No such facility.\n", opt->strarg);
	    logg_close();
	    optfree(opts);
	    return 1;
	}

	openlog("clamav-milter", LOG_PID, fac);
	logg_syslog = 1;
    }
#endif

    time(&currtime);
    if(logg("#+++ Started at %s", ctime(&currtime))) {
	fprintf(stderr, "ERROR: Can't initialize the internal logger\n");
	logg_close();
	optfree(opts);
	return 1;
    }
    if((opt = optget(opts, "TemporaryDirectory"))->enabled)
	tempdir = opt->strarg;

    if(localnets_init(opts) || init_actions(opts)) {
	logg_close();
	optfree(opts);
	return 1;
    }

    if((opt = optget(opts, "Whitelist"))->enabled && whitelist_init(opt->strarg)) {
	localnets_free();
	logg_close();
	optfree(opts);
	return 1;
    }

    if((opt = optget(opts, "SkipAuthenticated"))->enabled && smtpauth_init(opt->strarg)) {
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }

    pt = optget(opts, "AddHeader")->strarg;
    if(strcasecmp(pt, "No")) {
	char myname[255];

	if(!gethostname(myname, sizeof(myname))) {
	    myname[sizeof(myname)-1] = '\0';
	    snprintf(xvirushdr, sizeof(xvirushdr), "clamav-milter %s at %s", get_version(), myname);
	    xvirushdr[sizeof(xvirushdr)-1] = '\0';
	} else {
	    snprintf(xvirushdr, sizeof(xvirushdr), "clamav-milter %s", get_version());
	    xvirushdr[sizeof(xvirushdr)-1] = '\0';
	}

	descr.xxfi_flags |= SMFIF_ADDHDRS;

	if(strcasecmp(pt, "Add")) { /* Replace or Yes */
	    descr.xxfi_flags |= SMFIF_CHGHDRS;
	    addxvirus = 1;
	} else { /* Add */
	    addxvirus = 2;
	}
    }
    
    if(!(my_socket = optget(opts, "MilterSocket")->strarg)) {
	logg("!Please configure the MilterSocket directive\n");
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }

    if(!optget(opts, "Foreground")->enabled) {
	if(daemonize() == -1) {
	    logg("!daemonize() failed\n");
	    localnets_free();
	    whitelist_free();
	    cpool_free();
	    logg_close();
	    optfree(opts);
	    return 1;
	}
	if(chdir("/") == -1)
	    logg("^Can't change current working directory to root\n");
    }

    if(smfi_setconn(my_socket) == MI_FAILURE) {
	logg("!smfi_setconn failed\n");
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }
    if(smfi_register(descr) == MI_FAILURE) {
	logg("!smfi_register failed\n");
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }
    opt = optget(opts, "FixStaleSocket");
    if(smfi_opensocket(opt->enabled) == MI_FAILURE) {
	logg("!Failed to create socket %s\n", my_socket);
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }

    maxfilesize = optget(opts, "MaxFileSize")->numarg;
    readtimeout = optget(opts, "ReadTimeout")->numarg;

    cpool_init(opts);
    if (!cp) {
	logg("!Failed to init the socket pool\n");
	localnets_free();
	whitelist_free();
	logg_close();
	optfree(opts);
	return 1;
    }	

    if((opt = optget(opts, "PidFile"))->enabled) {
	FILE *fd;
	mode_t old_umask = umask(0006);

	if((fd = fopen(opt->strarg, "w")) == NULL) {
	    logg("!Can't save PID in file %s\n", opt->strarg);
	} else {
	    if (fprintf(fd, "%u", (unsigned int)getpid())<0) {
	    	logg("!Can't save PID in file %s\n", opt->strarg);
	    }
	    fclose(fd);
	}
	umask(old_umask);
    }

    ret = smfi_main();

    optfree(opts);

    logg_close();
    cpool_free();
    localnets_free();
    whitelist_free();

    return ret;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * tab-width: 8
 * End: 
 * vim: set cindent smartindent autoindent softtabstop=4 shiftwidth=4 tabstop=8: 
 */

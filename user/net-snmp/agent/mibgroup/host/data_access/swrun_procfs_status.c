/*
 * swrun_procfs_linux.c:
 *     hrSWRunTable data access:
 *     /proc/{pid}/status interface - Linux
 */
#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_LINUX_TASKS_H
#include <linux/tasks.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/data_access/swrun.h>

/* ---------------------------------------------------------------------
 */
void
netsnmp_arch_swrun_init(void)
{
#ifdef HAVE_LINUX_TASKS_H
    extern int _swrun_max = NR_TASKS;   /* from <linux/tasks.h> */
#endif
    
    return;
}

/* ---------------------------------------------------------------------
 */
int
netsnmp_arch_swrun_container_load( netsnmp_container *container, u_int flags)
{
    DIR                 *procdir = NULL;
    struct dirent       *procentry_p;
    FILE                *fp;
    int                  pid, rc, i;
    char                 buf[BUFSIZ], buf2[BUFSIZ], *cp;
    netsnmp_swrun_entry *entry;
    
    procdir = opendir("/proc");
    if ( NULL == procdir ) {
        snmp_log( LOG_ERR, "Failed to open /proc" );
        return -1;
    }

    /*
     * Walk through the list of processes in the /proc tree
     */
    while ( NULL != (procentry_p = readdir( procdir ))) {
        pid = atoi( procentry_p->d_name );
        if ( 0 == pid )
            continue;   /* Presumably '.' or '..' */

        entry = netsnmp_swrun_entry_create(pid);
        if (NULL == entry)
            continue;   /* error already logged by function */
        rc = CONTAINER_INSERT(container, entry);

        /*
         * Now extract the interesting information
         *   from the various /proc{PID}/ interface files
         */

        /*
         *   Name:  process name
         */
        snprintf( buf2, BUFSIZ, "/proc/%d/status", pid );
        fp = fopen( buf2, "r" );
        memset(buf, 0, sizeof(buf));
        fgets( buf, BUFSIZ-1, fp );
        fclose(fp);

        for ( cp = buf; *cp != ':'; cp++ )
            ;
        while (isspace(*(++cp)))	/* Skip ':' and following spaces */
            ;
        entry->hrSWRunName_len = snprintf(entry->hrSWRunName,
                                   sizeof(entry->hrSWRunName)-1, "%s", cp);
        if ( '\n' == entry->hrSWRunName[ entry->hrSWRunName_len-1 ]) {
            entry->hrSWRunName[ entry->hrSWRunName_len-1 ] = '\0';
            entry->hrSWRunName_len--;           /* Stamp on trailing newline */
        }

        /*
         *  Command Line:
         *     argv[0] '\0' argv[1] '\0' ....
         */
        snprintf( buf2, BUFSIZ, "/proc/%d/cmdline", pid );
        fp = fopen( buf2, "r" );
        memset(buf, 0, sizeof(buf));
        cp = fgets( buf, BUFSIZ-1, fp );
        fclose(fp);

        if ( cp ) {
            /*
             *     argv[0]   is hrSWRunPath
             */ 
            entry->hrSWRunPath_len = snprintf(entry->hrSWRunPath,
                                       sizeof(entry->hrSWRunPath)-1, "%s", buf);
            /*
             * Stitch together argv[1..] to construct hrSWRunParameters
             */
            cp = buf + entry->hrSWRunPath_len+1;
            while ( 1 ) {
                while (*cp)
                    cp++;
                if ( '\0' == *(cp+1))
                    break;      /* '\0''\0' => End of command line */
                *cp = ' ';
            }
            entry->hrSWRunParameters_len = snprintf(entry->hrSWRunParameters,
                                             sizeof(entry->hrSWRunParameters)-1,
                                             "%s", buf + entry->hrSWRunPath_len+1);
        } else {
            memcpy(entry->hrSWRunPath, entry->hrSWRunName, entry->hrSWRunName_len);
            entry->hrSWRunPath_len       = entry->hrSWRunName_len;
            entry->hrSWRunParameters_len = 0;
        }
 
        /*
         * XXX - No information regarding system processes vs applications
         */
        entry->hrSWRunType = HRSWRUNTYPE_APPLICATION;

        /*
         *   {xxx} {xxx} STATUS  {xxx}*10  UTIME STIME  {xxx}*8 RSS
         */
        snprintf( buf, BUFSIZ, "/proc/%d/stat", pid );
        fp = fopen( buf, "r" );
        fgets( buf, BUFSIZ-1, fp );
        fclose(fp);

        cp = buf;
        while ( ' ' != *(cp++))    /* Skip first field */
            ;
        while ( ' ' != *(cp++))    /* Skip second field */
            ;
        
        switch (*cp) {
        case 'R':  entry->hrSWRunStatus = HRSWRUNSTATUS_RUNNING;
                   break;
        case 'S':  entry->hrSWRunStatus = HRSWRUNSTATUS_RUNNABLE;
                   break;
        case 'D':
        case 'T':  entry->hrSWRunStatus = HRSWRUNSTATUS_NOTRUNNABLE;
                   break;
        case 'Z':
        default:   entry->hrSWRunStatus = HRSWRUNSTATUS_INVALID;
                   break;
        }
        for (i=10; i; i--) {   /* Skip STATUS + 10 fields */
            while (' ' != *(cp++))
                ;
            cp++;
        }
        entry->hrSWRunPerfCPU  = atoi( cp );   /*  utime */
        while ( ' ' != *(cp++))
            ;
        cp++;				   /* Skip utime */
        entry->hrSWRunPerfCPU += atoi( cp );   /* +stime */

        for (i=8; i; i--) {   /* Skip stime + 8 fields */
            while (' ' != *(cp++))
                ;
            cp++;
        }
        entry->hrSWRunPerfMem  = atoi( cp );   /*  rss */
        entry->hrSWRunPerfMem *= (getpagesize()/1024);  /* in kB */
    }
    closedir( procdir );

    DEBUGMSGTL(("swrun:load:arch"," loaded %d entries\n",
                CONTAINER_SIZE(container)));

    return 0;
}

/*
 * swrun_kinfo.c:
 *     hrSWRunTable data access:
 *     kvm_getprocs() interface - FreeBSD, NetBSD, OpenBSD
 *
 * NB: later FreeBSD uses a different kinfo_proc structure
 */
#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef HAVE_KVM_H
#include <kvm.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_USER_H
#include <sys/user.h>
#endif
#ifdef HAVE_UVM_UVM_EXTERNAL_H
#include <uvm/uvm_external.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/data_access/swrun.h>
#include "kernel.h"


#if defined(freebsd5) && __FreeBSD_version >= 500014
    /*
     * later FreeBSD kinfo_proc field names
     */
#define SWRUN_K_STAT	ki_stat
#define SWRUN_K_PID	ki_pid
#define SWRUN_K_COMM	ki_comm
#define SWRUN_K_FLAG	ki_flag
#define SWRUN_K_CLASS	ki_pri.pri_class

#else
    /*
     * early FreeBSD, NetBSD, OpenBSD kinfo_proc field names
     */
#define SWRUN_K_STAT	kp_proc.p_stat
#define SWRUN_K_PID	kp_proc.p_pid
#define SWRUN_K_COMM	kp_proc.p_comm
#define SWRUN_K_FLAG	kp_proc.p_flag
/*      SWRUN_K_CLASS	not defined     */
#endif

/*
 *  Define dummy values if not already provided by the system
 */

#ifndef SRUN
#define SRUN	200	/* Defined by FreeBSD/OpenBSD, missing in  NetBSD */
#endif
#ifndef SACTIVE
#define SACTIVE	201	/* Defined by  NetBSD, missing in FreeBSD/OpenBSD */
#endif
#ifndef SSLEEP
#define SSLEEP	202	/* Defined by FreeBSD/OpenBSD, missing in  NetBSD */
#endif
#ifndef SWAIT
#define SWAIT	203	/* Defined by FreeBSD, missing in  NetBSD/OpenBSD */
#endif
#ifndef SSTOP
#define SSTOP	204	/* Defined by FreeBSD/NetBSD/OpenBSD */
#endif
#ifndef SLOCK
#define SLOCK	205	/* Defined by FreeBSD, missing in NetBSD/OpenBSD */
#endif
#ifndef SIDL
#define SIDL	206	/* Defined by FreeBSD/NetBSD/OpenBSD */
#endif
#ifndef SZOMB
#define SZOMB	207	/* Defined by FreeBSD/NetBSD/OpenBSD */
#endif
#ifndef SDEAD
#define SDEAD	208	/* Defined by OpenBSD, missing in FreeBSD/NetBSD */
#endif
#ifndef SONPROC
#define SONPROC	209	/* Defined by OpenBSD, missing in FreeBSD/NetBSD */
#endif

/* ---------------------------------------------------------------------
 */
void
netsnmp_arch_swrun_init(void)
{
#if NETSNMP_CAN_USE_SYSCTL && defined(CTL_KERN) && defined(KERN_MAXPROC)
    extern int _swrun_max;
    int max_size = sizeof(_swrun_max);
    int maxproc_mib[] = { CTL_KERN, KERN_MAXPROC };
    sysctl(maxproc_mib, 2, &_swrun_max, &max_size, NULL, 0);
#endif
    
    return;
}

/* ---------------------------------------------------------------------
 */
int
netsnmp_arch_swrun_container_load( netsnmp_container *container, u_int flags)
{
    struct kinfo_proc   *proc_table;
    int                  nprocs, i, rc;
    char                 buf[BUFSIZ], **argv, *cp;
    char                *name, *path;
    netsnmp_swrun_entry *entry;

    if ( 0 == kd ) {
        DEBUGMSGTL(("swrun:load:arch"," Can't query kvm info\n"));
        return 1;     /* No handle for retrieving process table */
    }
    proc_table = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nprocs );
    for ( i=0 ; i<nprocs; i++ ) {
        if ( 0 == proc_table[i].SWRUN_K_STAT )
            continue;
        entry = netsnmp_swrun_entry_create(proc_table[i].SWRUN_K_PID);
        if (NULL == entry)
            continue;   /* error already logged by function */
        rc = CONTAINER_INSERT(container, entry);

        /*
         * There are two possible sources for the command being run:
         *   - SWRUN_K_COMM  (from the proc_table entry directly)
         *   - running kvm_getargv on the process entry.
         *
         * We'll use argv[0] (if set) for hrSWRunPath,
         *   since that might potentially contain the
         *   absolute path to the running binary.
         * We'll use SWRUN_K_COMM for hrSWRunName,
         *   and as an alternative for hrSWRunPath
         */
        argv = kvm_getargv( kd, &(proc_table[i]), 0);

        entry->hrSWRunName_len = snprintf(entry->hrSWRunName,
                                   sizeof(entry->hrSWRunName)-1,
                                          "%s", proc_table[i].SWRUN_K_COMM);

        if ( argv && *argv)
            entry->hrSWRunPath_len = snprintf(entry->hrSWRunPath,
                                       sizeof(entry->hrSWRunPath)-1,
                                              "%s", argv[0]);
        else {
            memcpy( entry->hrSWRunPath, entry->hrSWRunName,
                                        entry->hrSWRunName_len );
            entry->hrSWRunPath_len = entry->hrSWRunName_len;
        }

        /*
         * Stitch together the rest of argv[] to build hrSWRunParameters
         *
         * Note:
         *   We add a separating space before each argv[] parameter,
         *   *including* the first one.  So we need to skip this
         *   leading space (buf[0]) when setting hrSWRunParameters.
         * This is also why we cleared the first *two* characters
         *   in the buffer initially. If there were no command-line
         *   arguments, then buf[1] would still be a null string.
         */
        buf[0] = '\0';
        buf[1] = '\0';
        if (argv)
            argv++;    /* Skip argv[0] */
        while ( argv && *argv ) {
            strcat(buf, " ");
            strcat(buf, *argv);
            argv++;
        }
        entry->hrSWRunParameters_len = snprintf(entry->hrSWRunParameters,
                                         sizeof(entry->hrSWRunParameters)-1,
                                          "%s", buf+1);

        entry->hrSWRunType = (P_SYSTEM & proc_table[i].SWRUN_K_FLAG) 
#ifdef SWRUN_K_CLASS
                             ? ((PRI_ITHD == proc_table[i].SWRUN_K_CLASS)
                                ? 3  /* device driver    */
                                : 2  /* operating system */
                               )
#else
                             ? 2  /* operating system */
#endif
                             : 4  /*  application     */
                             ;

        switch (proc_table[i].SWRUN_K_STAT) {
        case SRUN:    entry->hrSWRunStatus = HRSWRUNSTATUS_RUNNING;
                      break;
        case SSLEEP:
        case SWAIT:   entry->hrSWRunStatus = HRSWRUNSTATUS_RUNNABLE;
                      break;
        case SSTOP:
        case SLOCK:   entry->hrSWRunStatus = HRSWRUNSTATUS_NOTRUNNABLE;
                      break;
        case SIDL:
        case SZOMB:
        default:      entry->hrSWRunStatus = HRSWRUNSTATUS_INVALID;
                      break;
        }
        
#if defined(freebsd5) && __FreeBSD_version >= 500014
# ifdef NOT_DEFINED
   Apparently following these pointers triggers a SIG10 error

        entry->hrSWRunPerfCPU  = proc_table[i].ki_paddr->p_uticks;
        entry->hrSWRunPerfCPU += proc_table[i].ki_paddr->p_sticks;
        entry->hrSWRunPerfCPU += proc_table[i].ki_paddr->p_iticks;
        entry->hrSWRunPerfMem  = proc_table[i].ki_vmspace->vm_tsize;
        entry->hrSWRunPerfMem += proc_table[i].ki_vmspace->vm_ssize;
        entry->hrSWRunPerfMem += proc_table[i].ki_vmspace->vm_dsize;
        entry->hrSWRunPerfMem *= (getpagesize()/1024);  /* in kB */
# endif
        entry->hrSWRunPerfCPU  = proc_table[i].ki_runtime / 100000;
        entry->hrSWRunPerfMem  = proc_table[i].ki_size / 1024;;
#else
        /*
         * early FreeBSD, NetBSD, OpenBSD
         */
        entry->hrSWRunPerfCPU  = proc_table[i].kp_proc.p_uticks;
        entry->hrSWRunPerfCPU += proc_table[i].kp_proc.p_sticks;
        entry->hrSWRunPerfCPU += proc_table[i].kp_proc.p_iticks;
        entry->hrSWRunPerfMem  = proc_table[i].kp_eproc.e_vm.vm_tsize;
        entry->hrSWRunPerfMem += proc_table[i].kp_eproc.e_vm.vm_ssize;
        entry->hrSWRunPerfMem += proc_table[i].kp_eproc.e_vm.vm_dsize;
        entry->hrSWRunPerfMem *= (getpagesize() / 1024);
#endif
    }
    /*
     * 'proc_table' is owned by the kvm library,
     *   so shouldn't be freed here.
     */

    DEBUGMSGTL(("swrun:load:arch"," loaded %d entries\n",
                CONTAINER_SIZE(container)));

    return 0;
}

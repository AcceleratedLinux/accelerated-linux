/* Native debugging support for GNU/Linux (LWP layer).

   Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
   Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "target.h"

#include <signal.h>

/* Structure describing an LWP.  This is public only for the purposes
   of ALL_LWPS; target-specific code should generally not access it
   directly.  */

struct lwp_info
{
  /* The process id of the LWP.  This is a combination of the LWP id
     and overall process id.  */
  ptid_t ptid;

  /* Non-zero if this LWP is cloned.  In this context "cloned" means
     that the LWP is reporting to its parent using a signal other than
     SIGCHLD.  */
  int cloned;

  /* Non-zero if we sent this LWP a SIGSTOP (but the LWP didn't report
     it back yet).  */
  int signalled;

  /* Non-zero if this LWP is stopped.  */
  int stopped;

  /* Non-zero if this LWP will be/has been resumed.  Note that an LWP
     can be marked both as stopped and resumed at the same time.  This
     happens if we try to resume an LWP that has a wait status
     pending.  We shouldn't let the LWP run until that wait status has
     been processed, but we should not report that wait status if GDB
     didn't try to let the LWP run.  */
  int resumed;

  /* If non-zero, a pending wait status.  */
  int status;

  /* Non-zero if we were stepping this LWP.  */
  int step;

  /* Non-zero si_signo if this LWP stopped with a trap.  si_addr may
     be the address of a hardware watchpoint.  */
  struct siginfo siginfo;

  /* If WAITSTATUS->KIND != TARGET_WAITKIND_SPURIOUS, the waitstatus
     for this LWP's last event.  This may correspond to STATUS above,
     or to a local variable in lin_lwp_wait.  */
  struct target_waitstatus waitstatus;

  /* Next LWP in list.  */
  struct lwp_info *next;
};

/* The global list of LWPs, for ALL_LWPS.  Unlike the threads list,
   there is always at least one LWP on the list while the GNU/Linux
   native target is active.  */
extern struct lwp_info *lwp_list;

/* Iterate over the PTID each active thread (light-weight process).  There
   must be at least one.  */
#define ALL_LWPS(LP, PTID)						\
  for ((LP) = lwp_list, (PTID) = (LP)->ptid;				\
       (LP) != NULL;							\
       (LP) = (LP)->next, (PTID) = (LP) ? (LP)->ptid : (PTID))

/* Attempt to initialize libthread_db.  */
void check_for_thread_db (void);

/* Tell the thread_db layer what native target operations to use.  */
void thread_db_init (struct target_ops *);

/* Find process PID's pending signal set from /proc/pid/status.  */
void linux_proc_pending_signals (int pid, sigset_t *pending, sigset_t *blocked, sigset_t *ignored);

/* linux-nat functions for handling fork events.  */
extern void linux_enable_event_reporting (ptid_t ptid);

extern int lin_lwp_attach_lwp (ptid_t ptid);

/* Iterator function for lin-lwp's lwp list.  */
struct lwp_info *iterate_over_lwps (int (*callback) (struct lwp_info *, 
						     void *), 
				    void *data);

/* Create a prototype generic GNU/Linux target.  The client can
   override it with local methods.  */
struct target_ops * linux_target (void);

/* Create a generic GNU/Linux target using traditional 
   ptrace register access.  */
struct target_ops *
linux_trad_target (CORE_ADDR (*register_u_offset)(struct gdbarch *, int, int));

/* Register the customized GNU/Linux target.  This should be used
   instead of calling add_target directly.  */
void linux_nat_add_target (struct target_ops *);

/* Register a method to call whenever a new thread is attached.  */
void linux_nat_set_new_thread (struct target_ops *, void (*) (ptid_t));

/* Update linux-nat internal state when changing from one fork
   to another.  */
void linux_nat_switch_fork (ptid_t new_ptid);

/* Return the saved siginfo associated with PTID.  */
struct siginfo *linux_nat_get_siginfo (ptid_t ptid);

/* Data structures associated with breakpoints in GDB.
   Copyright (C) 1992, 93, 94, 95, 96, 98, 1999 Free Software Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if !defined (BREAKPOINT_H)
#define BREAKPOINT_H 1

#include "frame.h"
#include "value.h"

/* This is the maximum number of bytes a breakpoint instruction can take.
   Feel free to increase it.  It's just used in a few places to size
   arrays that should be independent of the target architecture.  */

#define	BREAKPOINT_MAX	16

/* Type of breakpoint. */
/* FIXME In the future, we should fold all other breakpoint-like things into
   here.  This includes:

   * single-step (for machines where we have to simulate single stepping)
      (probably, though perhaps it is better for it to look as much as
      possible like a single-step to wait_for_inferior).  */

enum bptype {
  bp_none = 0,                  /* Eventpoint has been deleted. */
  bp_breakpoint,		/* Normal breakpoint */
  bp_hardware_breakpoint,	/* Hardware assisted breakpoint */
  bp_until,			/* used by until command */
  bp_finish,			/* used by finish command */
  bp_watchpoint,		/* Watchpoint */
  bp_hardware_watchpoint,	/* Hardware assisted watchpoint */
  bp_read_watchpoint,		/* read watchpoint, (hardware assisted) */
  bp_access_watchpoint,		/* access watchpoint, (hardware assisted) */
  bp_longjmp,			/* secret breakpoint to find longjmp() */
  bp_longjmp_resume,		/* secret breakpoint to escape longjmp() */

  /* Used by wait_for_inferior for stepping over subroutine calls, for
     stepping over signal handlers, and for skipping prologues.  */
  bp_step_resume,

  /* Used by wait_for_inferior for stepping over signal handlers.  */
  bp_through_sigtramp,

  /* Used to detect when a watchpoint expression has gone out of
     scope.  These breakpoints are usually not visible to the user.

     This breakpoint has some interesting properties:

       1) There's always a 1:1 mapping between watchpoints
       on local variables and watchpoint_scope breakpoints.

       2) It automatically deletes itself and the watchpoint it's
       associated with when hit.

       3) It can never be disabled.  */
  bp_watchpoint_scope,

  /* The breakpoint at the end of a call dummy.  */
  /* FIXME: What if the function we are calling longjmp()s out of the
     call, or the user gets out with the "return" command?  We currently
     have no way of cleaning up the breakpoint in these (obscure) situations.
     (Probably can solve this by noticing longjmp, "return", etc., it's
     similar to noticing when a watchpoint on a local variable goes out
     of scope (with hardware support for watchpoints)).  */
  bp_call_dummy,

  /* Some dynamic linkers (HP, maybe Solaris) can arrange for special
     code in the inferior to run when significant events occur in the
     dynamic linker (for example a library is loaded or unloaded).

     By placing a breakpoint in this magic code GDB will get control
     when these significant events occur.  GDB can then re-examine
     the dynamic linker's data structures to discover any newly loaded
     dynamic libraries.  */
  bp_shlib_event,
 
  /* These breakpoints are used to implement the "catch load" command
     on platforms whose dynamic linkers support such functionality.  */
  bp_catch_load,
 
  /* These breakpoints are used to implement the "catch unload" command
     on platforms whose dynamic linkers support such functionality.  */
  bp_catch_unload,
 
  /* These are not really breakpoints, but are catchpoints that
     implement the "catch fork", "catch vfork" and "catch exec" commands
     on platforms whose kernel support such functionality.  (I.e.,
     kernels which can raise an event when a fork or exec occurs, as
     opposed to the debugger setting breakpoints on functions named
     "fork" or "exec".) */
  bp_catch_fork,
  bp_catch_vfork,
  bp_catch_exec,

  /* These are catchpoints to implement "catch catch" and "catch throw"
     commands for C++ exception handling. */
  bp_catch_catch,
  bp_catch_throw
 
};

/* States of enablement of breakpoint. */

enum enable { 
  disabled,           /* The eventpoint is inactive, and cannot trigger. */
  enabled,            /* The eventpoint is active, and can trigger. */
  shlib_disabled,     /* The eventpoint's address is within an unloaded solib.
                         The eventpoint will be automatically enabled & reset
                         when that solib is loaded. */
  call_disabled       /* The eventpoint has been disabled while a call into
                         the inferior is "in flight", because some eventpoints
                         interfere with the implementation of a call on some
                         targets.  The eventpoint will be automatically enabled
                         & reset when the call "lands" (either completes, or
                         stops at another eventpoint). */
};


/* Disposition of breakpoint.  Ie: what to do after hitting it. */

enum bpdisp {
  del,				/* Delete it */
  del_at_next_stop,		/* Delete at next stop, whether hit or not */
  disable,			/* Disable it */
  donttouch			/* Leave it alone */
};

/* Note that the ->silent field is not currently used by any commands
   (though the code is in there if it was to be, and set_raw_breakpoint
   does set it to 0).  I implemented it because I thought it would be
   useful for a hack I had to put in; I'm going to leave it in because
   I can see how there might be times when it would indeed be useful */

/* This is for a breakpoint or a watchpoint.  */

struct breakpoint
{
  struct breakpoint *next;
  /* Type of breakpoint. */
  enum bptype type;
  /* Zero means disabled; remember the info but don't break here.  */
  enum enable enable;
  /* What to do with this breakpoint after we hit it. */
  enum bpdisp disposition;
  /* Number assigned to distinguish breakpoints.  */
  int number;

  /* Address to break at, or NULL if not a breakpoint.  */
  CORE_ADDR address;

  /* Line number of this address.  Only matters if address is
     non-NULL.  */

  int line_number;

  /* Source file name of this address.  Only matters if address is
     non-NULL.  */

  char *source_file;

  /* Non-zero means a silent breakpoint (don't print frame info
     if we stop here). */
  unsigned char silent;
  /* Number of stops at this breakpoint that should
     be continued automatically before really stopping.  */
  int ignore_count;
  /* "Real" contents of byte where breakpoint has been inserted.
     Valid only when breakpoints are in the program.  Under the complete
     control of the target insert_breakpoint and remove_breakpoint routines.
     No other code should assume anything about the value(s) here.  */
  char shadow_contents[BREAKPOINT_MAX];
  /* Nonzero if this breakpoint is now inserted.  Only matters if address
     is non-NULL.  */
  char inserted;
  /* Nonzero if this is not the first breakpoint in the list
     for the given address.  Only matters if address is non-NULL.  */
  char duplicate;
  /* Chain of command lines to execute when this breakpoint is hit.  */
  struct command_line *commands;
  /* Stack depth (address of frame).  If nonzero, break only if fp
     equals this.  */
  CORE_ADDR frame;
  /* Conditional.  Break only if this expression's value is nonzero.  */
  struct expression *cond;

  /* String we used to set the breakpoint (malloc'd).  Only matters if
     address is non-NULL.  */
  char *addr_string;
  /* Language we used to set the breakpoint.  */
  enum language language;
  /* Input radix we used to set the breakpoint.  */
  int input_radix;
  /* String form of the breakpoint condition (malloc'd), or NULL if there
     is no condition.  */
  char *cond_string;
  /* String form of exp (malloc'd), or NULL if none.  */
  char *exp_string;

  /* The expression we are watching, or NULL if not a watchpoint.  */
  struct expression *exp;
  /* The largest block within which it is valid, or NULL if it is
     valid anywhere (e.g. consists just of global symbols).  */
  struct block *exp_valid_block;
  /* Value of the watchpoint the last time we checked it.  */
  value_ptr val;

  /* Holds the value chain for a hardware watchpoint expression.  */
  value_ptr val_chain;

  /* Holds the address of the related watchpoint_scope breakpoint
     when using watchpoints on local variables (might the concept
     of a related breakpoint be useful elsewhere, if not just call
     it the watchpoint_scope breakpoint or something like that. FIXME).  */
  struct breakpoint *related_breakpoint; 

  /* Holds the frame address which identifies the frame this watchpoint
     should be evaluated in, or NULL if the watchpoint should be evaluated
     on the outermost frame.  */
  CORE_ADDR watchpoint_frame;

  /* Thread number for thread-specific breakpoint, or -1 if don't care */
  int thread;

  /* Count of the number of times this breakpoint was taken, dumped
     with the info, but not used for anything else.  Useful for
     seeing how many times you hit a break prior to the program
     aborting, so you can back up to just before the abort.  */
  int hit_count;

  /* Filename of a dynamically-linked library (dll), used for bp_catch_load
     and bp_catch_unload (malloc'd), or NULL if any library is significant.  */
  char *  dll_pathname;
 
  /* Filename of a dll whose state change (e.g., load or unload)
     triggered this catchpoint.  This field is only vaid immediately
     after this catchpoint has triggered.  */
  char *  triggered_dll_pathname;
 
  /* Process id of a child process whose forking triggered this catchpoint.
     This field is only vaid immediately after this catchpoint has triggered.  */
  int  forked_inferior_pid;
 
  /* Filename of a program whose exec triggered this catchpoint.  This
     field is only vaid immediately after this catchpoint has triggered.  */
  char *  exec_pathname;
  
  asection *section;
};

/* The following stuff is an abstract data type "bpstat" ("breakpoint status").
   This provides the ability to determine whether we have stopped at a
   breakpoint, and what we should do about it.  */

typedef struct bpstats *bpstat;

/* Interface:  */
/* Clear a bpstat so that it says we are not at any breakpoint.
   Also free any storage that is part of a bpstat.  */
extern void bpstat_clear PARAMS ((bpstat *));

/* Return a copy of a bpstat.  Like "bs1 = bs2" but all storage that
   is part of the bpstat is copied as well.  */
extern bpstat bpstat_copy PARAMS ((bpstat));

extern bpstat bpstat_stop_status PARAMS ((CORE_ADDR *, int));

/* This bpstat_what stuff tells wait_for_inferior what to do with a
   breakpoint (a challenging task).  */

enum bpstat_what_main_action {
  /* Perform various other tests; that is, this bpstat does not
     say to perform any action (e.g. failed watchpoint and nothing
     else).  */
  BPSTAT_WHAT_KEEP_CHECKING,

  /* Rather than distinguish between noisy and silent stops here, it
     might be cleaner to have bpstat_print make that decision (also
     taking into account stop_print_frame and source_only).  But the
     implications are a bit scary (interaction with auto-displays, etc.),
     so I won't try it.  */
     
  /* Stop silently.  */
  BPSTAT_WHAT_STOP_SILENT,

  /* Stop and print.  */
  BPSTAT_WHAT_STOP_NOISY,

  /* Remove breakpoints, single step once, then put them back in and
     go back to what we were doing.  It's possible that this should be
     removed from the main_action and put into a separate field, to more
     cleanly handle BPSTAT_WHAT_CLEAR_LONGJMP_RESUME_SINGLE.  */
  BPSTAT_WHAT_SINGLE,

  /* Set longjmp_resume breakpoint, remove all other breakpoints,
     and continue.  The "remove all other breakpoints" part is required
     if we are also stepping over another breakpoint as well as doing
     the longjmp handling.  */
  BPSTAT_WHAT_SET_LONGJMP_RESUME,

  /* Clear longjmp_resume breakpoint, then handle as
     BPSTAT_WHAT_KEEP_CHECKING.  */
  BPSTAT_WHAT_CLEAR_LONGJMP_RESUME,

  /* Clear longjmp_resume breakpoint, then handle as BPSTAT_WHAT_SINGLE.  */
  BPSTAT_WHAT_CLEAR_LONGJMP_RESUME_SINGLE,

  /* Clear step resume breakpoint, and keep checking.  */
  BPSTAT_WHAT_STEP_RESUME,

  /* Clear through_sigtramp breakpoint, muck with trap_expected, and keep
     checking.  */
  BPSTAT_WHAT_THROUGH_SIGTRAMP,

  /* Check the dynamic linker's data structures for new libraries, then
     keep checking.  */
  BPSTAT_WHAT_CHECK_SHLIBS,

  /* Check the dynamic linker's data structures for new libraries, then
     resume out of the dynamic linker's callback, stop and print.  */
  BPSTAT_WHAT_CHECK_SHLIBS_RESUME_FROM_HOOK,

  /* This is just used to keep track of how many enums there are.  */
  BPSTAT_WHAT_LAST
};

struct bpstat_what {
  enum bpstat_what_main_action main_action;

  /* Did we hit a call dummy breakpoint?  This only goes with a main_action
     of BPSTAT_WHAT_STOP_SILENT or BPSTAT_WHAT_STOP_NOISY (the concept of
     continuing from a call dummy without popping the frame is not a
     useful one).  */
  int call_dummy;
};

/* Tell what to do about this bpstat.  */
struct bpstat_what bpstat_what PARAMS ((bpstat));

/* Find the bpstat associated with a breakpoint.  NULL otherwise. */
bpstat bpstat_find_breakpoint PARAMS ((bpstat, struct breakpoint *));

/* Find a step_resume breakpoint associated with this bpstat.
   (If there are multiple step_resume bp's on the list, this function
   will arbitrarily pick one.)

   It is an error to use this function if BPSTAT doesn't contain a
   step_resume breakpoint.

   See wait_for_inferior's use of this function.
   */
extern struct breakpoint *
bpstat_find_step_resume_breakpoint PARAMS ((bpstat));

/* Nonzero if a signal that we got in wait() was due to circumstances
   explained by the BS.  */
/* Currently that is true if we have hit a breakpoint, or if there is
   a watchpoint enabled.  */
#define bpstat_explains_signal(bs) ((bs) != NULL)

/* Nonzero if we should step constantly (e.g. watchpoints on machines
   without hardware support).  This isn't related to a specific bpstat,
   just to things like whether watchpoints are set.  */
extern int bpstat_should_step PARAMS ((void));

/* Nonzero if there are enabled hardware watchpoints. */
extern int bpstat_have_active_hw_watchpoints PARAMS ((void));

/* Print a message indicating what happened.  Returns nonzero to
   say that only the source line should be printed after this (zero
   return means print the frame as well as the source line).  */
extern int bpstat_print PARAMS ((bpstat));

/* Return the breakpoint number of the first breakpoint we are stopped
   at.  *BSP upon return is a bpstat which points to the remaining
   breakpoints stopped at (but which is not guaranteed to be good for
   anything but further calls to bpstat_num).
   Return 0 if passed a bpstat which does not indicate any breakpoints.  */
extern int bpstat_num PARAMS ((bpstat *));

/* Perform actions associated with having stopped at *BSP.  Actually, we just
   use this for breakpoint commands.  Perhaps other actions will go here
   later, but this is executed at a late time (from the command loop).  */
extern void bpstat_do_actions PARAMS ((bpstat *));

/* Modify BS so that the actions will not be performed.  */
extern void bpstat_clear_actions PARAMS ((bpstat));

/* Given a bpstat that records zero or more triggered eventpoints, this
   function returns another bpstat which contains only the catchpoints
   on that first list, if any.
   */
extern void bpstat_get_triggered_catchpoints PARAMS ((bpstat, bpstat *));
 
/* Implementation:  */
struct bpstats
{
  /* Linked list because there can be two breakpoints at the
     same place, and a bpstat reflects the fact that both have been hit.  */
  bpstat next;
  /* Breakpoint that we are at.  */
  struct breakpoint *breakpoint_at;
  /* Commands left to be done.  */
  struct command_line *commands;
  /* Old value associated with a watchpoint.  */
  value_ptr old_val;

  /* Nonzero if this breakpoint tells us to print the frame.  */
  char print;

  /* Nonzero if this breakpoint tells us to stop.  */
  char stop;

  /* Function called by bpstat_print to print stuff associated with
     this element of the bpstat chain.  Returns 0 or 1 just like
     bpstat_print, or -1 if it can't deal with it.  */
  int (*print_it) PARAMS((bpstat bs));
};

enum inf_context
{
  inf_starting,
  inf_running,
  inf_exited
};


/* Prototypes for breakpoint-related functions.  */

#ifdef __STDC__		/* Forward declarations for prototypes */
struct frame_info;
#endif

extern int breakpoint_here_p PARAMS ((CORE_ADDR));

extern int breakpoint_inserted_here_p PARAMS ((CORE_ADDR));

extern int frame_in_dummy PARAMS ((struct frame_info *));

extern int breakpoint_thread_match PARAMS ((CORE_ADDR, int));

extern void until_break_command PARAMS ((char *, int));

extern void breakpoint_re_set PARAMS ((void));

extern void breakpoint_re_set_thread PARAMS ((struct breakpoint *));

extern int ep_is_exception_catchpoint PARAMS ((struct breakpoint *));

extern struct breakpoint *set_momentary_breakpoint
  PARAMS ((struct symtab_and_line, struct frame_info *, enum bptype));

extern void set_ignore_count PARAMS ((int, int, int));

extern void set_default_breakpoint PARAMS ((int, CORE_ADDR, struct symtab *, int));

extern void mark_breakpoints_out PARAMS ((void));

extern void breakpoint_init_inferior PARAMS ((enum inf_context));

extern void delete_breakpoint PARAMS ((struct breakpoint *));

extern void breakpoint_auto_delete PARAMS ((bpstat));

extern void breakpoint_clear_ignore_counts PARAMS ((void));

extern void break_command PARAMS ((char *, int));

extern void tbreak_command PARAMS ((char *, int));

extern int insert_breakpoints PARAMS ((void));

extern int remove_breakpoints PARAMS ((void));

/* This function can be used to physically insert eventpoints from the
   specified traced inferior process, without modifying the breakpoint
   package's state.  This can be useful for those targets which support
   following the processes of a fork() or vfork() system call, when both
   of the resulting two processes are to be followed.  */
extern int reattach_breakpoints PARAMS ((int));

/* This function can be used to update the breakpoint package's state
   after an exec() system call has been executed.

   This function causes the following:

     - All eventpoints are marked "not inserted".
     - All eventpoints with a symbolic address are reset such that
       the symbolic address must be reevaluated before the eventpoints
       can be reinserted.
     - The solib breakpoints are explicitly removed from the breakpoint
       list.
     - A step-resume breakpoint, if any, is explicitly removed from the
       breakpoint list.
     - All eventpoints without a symbolic address are removed from the
       breakpoint list. */
extern void update_breakpoints_after_exec PARAMS ((void));

/* This function can be used to physically remove hardware breakpoints
   and watchpoints from the specified traced inferior process, without
   modifying the breakpoint package's state.  This can be useful for
   those targets which support following the processes of a fork() or
   vfork() system call, when one of the resulting two processes is to
   be detached and allowed to run free.
 
   It is an error to use this function on the process whose id is
   inferior_pid.  */
extern int detach_breakpoints PARAMS ((int));
 
extern void enable_longjmp_breakpoint PARAMS ((void));

extern void disable_longjmp_breakpoint PARAMS ((void));

extern void set_longjmp_resume_breakpoint PARAMS ((CORE_ADDR,
						   struct frame_info *));
/* These functions respectively disable or reenable all currently
   enabled watchpoints.  When disabled, the watchpoints are marked
   call_disabled.  When reenabled, they are marked enabled.

   The intended client of these functions is infcmd.c\run_stack_dummy.

   The inferior must be stopped, and all breakpoints removed, when
   these functions are used.

   The need for these functions is that on some targets (e.g., HP-UX),
   gdb is unable to unwind through the dummy frame that is pushed as
   part of the implementation of a call command.  Watchpoints can
   cause the inferior to stop in places where this frame is visible,
   and that can cause execution control to become very confused.

   Note that if a user sets breakpoints in an interactively call
   function, the call_disabled watchpoints will have been reenabled
   when the first such breakpoint is reached.  However, on targets
   that are unable to unwind through the call dummy frame, watches
   of stack-based storage may then be deleted, because gdb will
   believe that their watched storage is out of scope.  (Sigh.) */
extern void
disable_watchpoints_before_interactive_call_start PARAMS ((void));

extern void
enable_watchpoints_after_interactive_call_stop PARAMS ((void));

 
extern void clear_breakpoint_hit_counts PARAMS ((void));

/* The following are for displays, which aren't really breakpoints, but
   here is as good a place as any for them.  */

extern void disable_current_display PARAMS ((void));

extern void do_displays PARAMS ((void));

extern void disable_display PARAMS ((int));

extern void clear_displays PARAMS ((void));

extern void disable_breakpoint PARAMS ((struct breakpoint *));

extern void enable_breakpoint PARAMS ((struct breakpoint *));

extern void create_solib_event_breakpoint PARAMS ((CORE_ADDR));

extern void remove_solib_event_breakpoints PARAMS ((void));

extern void disable_breakpoints_in_shlibs PARAMS ((int silent));

extern void re_enable_breakpoints_in_shlibs PARAMS ((void));

extern void create_solib_load_event_breakpoint PARAMS ((char *, int, char *, char *));
 
extern void create_solib_unload_event_breakpoint PARAMS ((char *, int, char *, char *));
 
extern void create_fork_event_catchpoint PARAMS ((int, char *));
 
extern void create_vfork_event_catchpoint PARAMS ((int, char *));

extern void create_exec_event_catchpoint PARAMS ((int, char *));
 
/* This function returns TRUE if ep is a catchpoint. */
extern int ep_is_catchpoint PARAMS ((struct breakpoint *));
 
/* This function returns TRUE if ep is a catchpoint of a
   shared library (aka dynamically-linked library) event,
   such as a library load or unload. */
extern int ep_is_shlib_catchpoint PARAMS ((struct breakpoint *));
 
extern struct breakpoint *set_breakpoint_sal PARAMS ((struct symtab_and_line));

#endif /* !defined (BREAKPOINT_H) */

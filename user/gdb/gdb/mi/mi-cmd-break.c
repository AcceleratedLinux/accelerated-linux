/* MI Command Set - breakpoint and watchpoint commands.
   Copyright (C) 2000, 2001, 2002, 2007, 2008 Free Software Foundation, Inc.
   Contributed by Cygnus Solutions (a Red Hat company).

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

#include "defs.h"
#include "mi-cmds.h"
#include "ui-out.h"
#include "mi-out.h"
#include "breakpoint.h"
#include "gdb_string.h"
#include "mi-getopt.h"
#include "gdb-events.h"
#include "gdb.h"
#include "exceptions.h"

enum
  {
    FROM_TTY = 0
  };

/* Output a single breakpoint. */

static void
breakpoint_notify (int b)
{
  gdb_breakpoint_query (uiout, b, NULL);
}


struct gdb_events breakpoint_hooks =
{
  breakpoint_notify,
  breakpoint_notify,
  breakpoint_notify,
};


enum bp_type
  {
    REG_BP,
    HW_BP,
    REGEXP_BP
  };

/* Implements the -break-insert command.
   See the MI manual for the list of possible options.  */

enum mi_cmd_result
mi_cmd_break_insert (char *command, char **argv, int argc)
{
  char *address = NULL;
  enum bp_type type = REG_BP;
  int temp_p = 0;
  int thread = -1;
  int ignore_count = 0;
  char *condition = NULL;
  int pending = 0;
  struct gdb_exception e;
  struct gdb_events *old_hooks;
  enum opt
    {
      HARDWARE_OPT, TEMP_OPT /*, REGEXP_OPT */ , CONDITION_OPT,
      IGNORE_COUNT_OPT, THREAD_OPT, PENDING_OPT
    };
  static struct mi_opt opts[] =
  {
    {"h", HARDWARE_OPT, 0},
    {"t", TEMP_OPT, 0},
    {"c", CONDITION_OPT, 1},
    {"i", IGNORE_COUNT_OPT, 1},
    {"p", THREAD_OPT, 1},
    {"f", PENDING_OPT, 0},
    { 0, 0, 0 }
  };

  /* Parse arguments. It could be -r or -h or -t, <location> or ``--''
     to denote the end of the option list. */
  int optind = 0;
  char *optarg;
  while (1)
    {
      int opt = mi_getopt ("mi_cmd_break_insert", argc, argv, opts, &optind, &optarg);
      if (opt < 0)
	break;
      switch ((enum opt) opt)
	{
	case TEMP_OPT:
	  temp_p = 1;
	  break;
	case HARDWARE_OPT:
	  type = HW_BP;
	  break;
#if 0
	case REGEXP_OPT:
	  type = REGEXP_BP;
	  break;
#endif
	case CONDITION_OPT:
	  condition = optarg;
	  break;
	case IGNORE_COUNT_OPT:
	  ignore_count = atol (optarg);
	  break;
	case THREAD_OPT:
	  thread = atol (optarg);
	  break;
	case PENDING_OPT:
	  pending = 1;
	  break;
	}
    }

  if (optind >= argc)
    error (_("mi_cmd_break_insert: Missing <location>"));
  if (optind < argc - 1)
    error (_("mi_cmd_break_insert: Garbage following <location>"));
  address = argv[optind];

  /* Now we have what we need, let's insert the breakpoint! */
  old_hooks = deprecated_set_gdb_event_hooks (&breakpoint_hooks);
  /* Make sure we restore hooks even if exception is thrown.  */
  TRY_CATCH (e, RETURN_MASK_ALL)
    {
      switch (type)
	{
	case REG_BP:
	  set_breakpoint (address, condition,
			  0 /*hardwareflag */ , temp_p,
			  thread, ignore_count,
			  pending);
	  break;
	case HW_BP:
	  set_breakpoint (address, condition,
			  1 /*hardwareflag */ , temp_p,
			  thread, ignore_count,
			  pending);
	  break;
#if 0
	case REGEXP_BP:
	  if (temp_p)
	    error (_("mi_cmd_break_insert: Unsupported tempoary regexp breakpoint"));
	  else
	    rbreak_command_wrapper (address, FROM_TTY);
	  return MI_CMD_DONE;
	  break;
#endif
	default:
	  internal_error (__FILE__, __LINE__,
			  _("mi_cmd_break_insert: Bad switch."));
	}
    }
  deprecated_set_gdb_event_hooks (old_hooks);
  if (e.reason < 0)
    throw_exception (e);

  return MI_CMD_DONE;
}

enum wp_type
{
  REG_WP,
  READ_WP,
  ACCESS_WP
};

/* Insert a watchpoint. The type of watchpoint is specified by the
   first argument: 
   -break-watch <expr> --> insert a regular wp.  
   -break-watch -r <expr> --> insert a read watchpoint.
   -break-watch -a <expr> --> insert an access wp. */

enum mi_cmd_result
mi_cmd_break_watch (char *command, char **argv, int argc)
{
  char *expr = NULL;
  enum wp_type type = REG_WP;
  enum opt
    {
      READ_OPT, ACCESS_OPT
    };
  static struct mi_opt opts[] =
  {
    {"r", READ_OPT, 0},
    {"a", ACCESS_OPT, 0},
    { 0, 0, 0 }
  };

  /* Parse arguments. */
  int optind = 0;
  char *optarg;
  while (1)
    {
      int opt = mi_getopt ("mi_cmd_break_watch", argc, argv, opts, &optind, &optarg);
      if (opt < 0)
	break;
      switch ((enum opt) opt)
	{
	case READ_OPT:
	  type = READ_WP;
	  break;
	case ACCESS_OPT:
	  type = ACCESS_WP;
	  break;
	}
    }
  if (optind >= argc)
    error (_("mi_cmd_break_watch: Missing <expression>"));
  if (optind < argc - 1)
    error (_("mi_cmd_break_watch: Garbage following <expression>"));
  expr = argv[optind];

  /* Now we have what we need, let's insert the watchpoint! */
  switch (type)
    {
    case REG_WP:
      watch_command_wrapper (expr, FROM_TTY);
      break;
    case READ_WP:
      rwatch_command_wrapper (expr, FROM_TTY);
      break;
    case ACCESS_WP:
      awatch_command_wrapper (expr, FROM_TTY);
      break;
    default:
      error (_("mi_cmd_break_watch: Unknown watchpoint type."));
    }
  return MI_CMD_DONE;
}

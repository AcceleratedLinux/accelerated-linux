/* MI Command Set.

   Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008
   Free Software Foundation, Inc.

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

/* Work in progress.  */

#include "defs.h"
#include "target.h"
#include "inferior.h"
#include "gdb_string.h"
#include "exceptions.h"
#include "top.h"
#include "gdbthread.h"
#include "mi-cmds.h"
#include "mi-parse.h"
#include "mi-getopt.h"
#include "mi-console.h"
#include "ui-out.h"
#include "mi-out.h"
#include "interps.h"
#include "event-loop.h"
#include "event-top.h"
#include "gdbcore.h"		/* For write_memory().  */
#include "value.h"
#include "regcache.h"
#include "gdb.h"
#include "frame.h"
#include "mi-main.h"

#include <ctype.h>
#include <sys/time.h>

#if defined HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_GETRUSAGE
struct rusage rusage;
#endif

enum
  {
    FROM_TTY = 0
  };

/* Enumerations of the actions that may result from calling
   captured_mi_execute_command.  */

enum captured_mi_execute_command_actions
  {
    EXECUTE_COMMAND_DISPLAY_PROMPT,
    EXECUTE_COMMAND_SUPRESS_PROMPT
  };

/* This structure is used to pass information from captured_mi_execute_command
   to mi_execute_command.  */
struct captured_mi_execute_command_args
{
  /* This return result of the MI command (output).  */
  enum mi_cmd_result rc;

  /* What action to perform when the call is finished (output).  */
  enum captured_mi_execute_command_actions action;

  /* The command context to be executed (input).  */
  struct mi_parse *command;
};

int mi_debug_p;
struct ui_file *raw_stdout;

/* This is used to pass the current command timestamp
   down to continuation routines.  */
static struct mi_timestamp *current_command_ts;

static int do_timings = 0;

/* The token of the last asynchronous command.  */
static char *last_async_command;
static char *previous_async_command;
char *mi_error_message;

extern void _initialize_mi_main (void);
static enum mi_cmd_result mi_cmd_execute (struct mi_parse *parse);

static void mi_execute_cli_command (const char *cmd, int args_p,
				    const char *args);
static enum mi_cmd_result mi_execute_async_cli_command (char *mi, char *args, int from_tty);

static void mi_exec_async_cli_cmd_continuation (struct continuation_arg *arg);

static int register_changed_p (int regnum, struct regcache *,
			       struct regcache *);
static int get_register (int regnum, int format);

/* Command implementations.  FIXME: Is this libgdb?  No.  This is the MI
   layer that calls libgdb.  Any operation used in the below should be
   formalized.  */

static void timestamp (struct mi_timestamp *tv);

static void print_diff_now (struct mi_timestamp *start);
static void print_diff (struct mi_timestamp *start, struct mi_timestamp *end);

enum mi_cmd_result
mi_cmd_gdb_exit (char *command, char **argv, int argc)
{
  /* We have to print everything right here because we never return.  */
  if (last_async_command)
    fputs_unfiltered (last_async_command, raw_stdout);
  fputs_unfiltered ("^exit\n", raw_stdout);
  mi_out_put (uiout, raw_stdout);
  /* FIXME: The function called is not yet a formal libgdb function.  */
  quit_force (NULL, FROM_TTY);
  return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_exec_run (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("run", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_next (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("next", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_next_instruction (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("nexti", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_step (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("step", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_step_instruction (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("stepi", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_finish (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("finish", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_until (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("until", args, from_tty);
}

enum mi_cmd_result
mi_cmd_exec_return (char *args, int from_tty)
{
  /* This command doesn't really execute the target, it just pops the
     specified number of frames. */
  if (*args)
    /* Call return_command with from_tty argument equal to 0 so as to
       avoid being queried.  */
    return_command (args, 0);
  else
    /* Call return_command with from_tty argument equal to 0 so as to
       avoid being queried.  */
    return_command (NULL, 0);

  /* Because we have called return_command with from_tty = 0, we need
     to print the frame here.  */
  print_stack_frame (get_selected_frame (NULL), 1, LOC_AND_ADDRESS);

  return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_exec_continue (char *args, int from_tty)
{
  /* FIXME: Should call a libgdb function, not a cli wrapper.  */
  return mi_execute_async_cli_command ("continue", args, from_tty);
}

/* Interrupt the execution of the target.  Note how we must play around
   with the token variables, in order to display the current token in
   the result of the interrupt command, and the previous execution
   token when the target finally stops.  See comments in
   mi_cmd_execute.  */
enum mi_cmd_result
mi_cmd_exec_interrupt (char *args, int from_tty)
{
  if (!target_executing)
    {
      mi_error_message = xstrprintf ("mi_cmd_exec_interrupt: Inferior not executing.");
      return MI_CMD_ERROR;
    }
  interrupt_target_command (args, from_tty);
  if (last_async_command)
    fputs_unfiltered (last_async_command, raw_stdout);
  fputs_unfiltered ("^done", raw_stdout);
  xfree (last_async_command);
  if (previous_async_command)
    last_async_command = xstrdup (previous_async_command);
  xfree (previous_async_command);
  previous_async_command = NULL;
  mi_out_put (uiout, raw_stdout);
  mi_out_rewind (uiout);
  fputs_unfiltered ("\n", raw_stdout);
  return MI_CMD_QUIET;
}

enum mi_cmd_result
mi_cmd_thread_select (char *command, char **argv, int argc)
{
  enum gdb_rc rc;

  if (argc != 1)
    {
      mi_error_message = xstrprintf ("mi_cmd_thread_select: USAGE: threadnum.");
      return MI_CMD_ERROR;
    }
  else
    rc = gdb_thread_select (uiout, argv[0], &mi_error_message);

  if (rc == GDB_RC_FAIL)
    return MI_CMD_ERROR;
  else
    return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_thread_list_ids (char *command, char **argv, int argc)
{
  enum gdb_rc rc;

  if (argc != 0)
    {
      mi_error_message = xstrprintf ("mi_cmd_thread_list_ids: No arguments required.");
      return MI_CMD_ERROR;
    }
  else
    rc = gdb_list_thread_ids (uiout, &mi_error_message);

  if (rc == GDB_RC_FAIL)
    return MI_CMD_ERROR;
  else
    return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_data_list_register_names (char *command, char **argv, int argc)
{
  int regnum, numregs;
  int i;
  struct cleanup *cleanup;

  /* Note that the test for a valid register must include checking the
     gdbarch_register_name because gdbarch_num_regs may be allocated for
     the union of the register sets within a family of related processors.
     In this case, some entries of gdbarch_register_name will change depending
     upon the particular processor being debugged.  */

  numregs = gdbarch_num_regs (current_gdbarch)
	    + gdbarch_num_pseudo_regs (current_gdbarch);

  cleanup = make_cleanup_ui_out_list_begin_end (uiout, "register-names");

  if (argc == 0)		/* No args, just do all the regs.  */
    {
      for (regnum = 0;
	   regnum < numregs;
	   regnum++)
	{
	  if (gdbarch_register_name (current_gdbarch, regnum) == NULL
	      || *(gdbarch_register_name (current_gdbarch, regnum)) == '\0')
	    ui_out_field_string (uiout, NULL, "");
	  else
	    ui_out_field_string (uiout, NULL,
				 gdbarch_register_name
				   (current_gdbarch, regnum));
	}
    }

  /* Else, list of register #s, just do listed regs.  */
  for (i = 0; i < argc; i++)
    {
      regnum = atoi (argv[i]);
      if (regnum < 0 || regnum >= numregs)
	{
	  do_cleanups (cleanup);
	  mi_error_message = xstrprintf ("bad register number");
	  return MI_CMD_ERROR;
	}
      if (gdbarch_register_name (current_gdbarch, regnum) == NULL
	  || *(gdbarch_register_name (current_gdbarch, regnum)) == '\0')
	ui_out_field_string (uiout, NULL, "");
      else
	ui_out_field_string (uiout, NULL,
			     gdbarch_register_name (current_gdbarch, regnum));
    }
  do_cleanups (cleanup);
  return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_data_list_changed_registers (char *command, char **argv, int argc)
{
  static struct regcache *this_regs = NULL;
  struct regcache *prev_regs;
  int regnum, numregs, changed;
  int i;
  struct cleanup *cleanup;

  /* The last time we visited this function, the current frame's register
     contents were saved in THIS_REGS.  Move THIS_REGS over to PREV_REGS,
     and refresh THIS_REGS with the now-current register contents.  */

  prev_regs = this_regs;
  this_regs = frame_save_as_regcache (get_selected_frame (NULL));
  cleanup = make_cleanup_regcache_xfree (prev_regs);

  /* Note that the test for a valid register must include checking the
     gdbarch_register_name because gdbarch_num_regs may be allocated for
     the union of the register sets within a family of related processors.
     In this  case, some entries of gdbarch_register_name will change depending
     upon the particular processor being debugged.  */

  numregs = gdbarch_num_regs (current_gdbarch)
	    + gdbarch_num_pseudo_regs (current_gdbarch);

  make_cleanup_ui_out_list_begin_end (uiout, "changed-registers");

  if (argc == 0)		/* No args, just do all the regs.  */
    {
      for (regnum = 0;
	   regnum < numregs;
	   regnum++)
	{
	  if (gdbarch_register_name (current_gdbarch, regnum) == NULL
	      || *(gdbarch_register_name (current_gdbarch, regnum)) == '\0')
	    continue;
	  changed = register_changed_p (regnum, prev_regs, this_regs);
	  if (changed < 0)
	    {
	      do_cleanups (cleanup);
	      mi_error_message = xstrprintf ("mi_cmd_data_list_changed_registers: Unable to read register contents.");
	      return MI_CMD_ERROR;
	    }
	  else if (changed)
	    ui_out_field_int (uiout, NULL, regnum);
	}
    }

  /* Else, list of register #s, just do listed regs.  */
  for (i = 0; i < argc; i++)
    {
      regnum = atoi (argv[i]);

      if (regnum >= 0
	  && regnum < numregs
	  && gdbarch_register_name (current_gdbarch, regnum) != NULL
	  && *gdbarch_register_name (current_gdbarch, regnum) != '\000')
	{
	  changed = register_changed_p (regnum, prev_regs, this_regs);
	  if (changed < 0)
	    {
	      do_cleanups (cleanup);
	      mi_error_message = xstrprintf ("mi_cmd_data_list_register_change: Unable to read register contents.");
	      return MI_CMD_ERROR;
	    }
	  else if (changed)
	    ui_out_field_int (uiout, NULL, regnum);
	}
      else
	{
	  do_cleanups (cleanup);
	  mi_error_message = xstrprintf ("bad register number");
	  return MI_CMD_ERROR;
	}
    }
  do_cleanups (cleanup);
  return MI_CMD_DONE;
}

static int
register_changed_p (int regnum, struct regcache *prev_regs,
		    struct regcache *this_regs)
{
  struct gdbarch *gdbarch = get_regcache_arch (this_regs);
  gdb_byte prev_buffer[MAX_REGISTER_SIZE];
  gdb_byte this_buffer[MAX_REGISTER_SIZE];

  /* Registers not valid in this frame return count as unchanged.  */
  if (!regcache_valid_p (this_regs, regnum))
    return 0;

  /* First time through or after gdbarch change consider all registers as
     changed.  Same for registers not valid in the previous frame.  */
  if (!prev_regs || get_regcache_arch (prev_regs) != gdbarch
      || !regcache_valid_p (prev_regs, regnum))
    return 1;

  /* Get register contents and compare.  */
  regcache_cooked_read (prev_regs, regnum, prev_buffer);
  regcache_cooked_read (this_regs, regnum, this_buffer);

  return memcmp (prev_buffer, this_buffer,
		 register_size (gdbarch, regnum)) != 0;
}

/* Return a list of register number and value pairs.  The valid
   arguments expected are: a letter indicating the format in which to
   display the registers contents.  This can be one of: x (hexadecimal), d
   (decimal), N (natural), t (binary), o (octal), r (raw).  After the
   format argumetn there can be a sequence of numbers, indicating which
   registers to fetch the content of.  If the format is the only argument,
   a list of all the registers with their values is returned.  */
enum mi_cmd_result
mi_cmd_data_list_register_values (char *command, char **argv, int argc)
{
  int regnum, numregs, format, result;
  int i;
  struct cleanup *list_cleanup, *tuple_cleanup;

  /* Note that the test for a valid register must include checking the
     gdbarch_register_name because gdbarch_num_regs may be allocated for
     the union of the register sets within a family of related processors.
     In this case, some entries of gdbarch_register_name will change depending
     upon the particular processor being debugged.  */

  numregs = gdbarch_num_regs (current_gdbarch)
	    + gdbarch_num_pseudo_regs (current_gdbarch);

  if (argc == 0)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_list_register_values: Usage: -data-list-register-values <format> [<regnum1>...<regnumN>]");
      return MI_CMD_ERROR;
    }

  format = (int) argv[0][0];

  list_cleanup = make_cleanup_ui_out_list_begin_end (uiout, "register-values");

  if (argc == 1)	    /* No args, beside the format: do all the regs.  */
    {
      for (regnum = 0;
	   regnum < numregs;
	   regnum++)
	{
	  if (gdbarch_register_name (current_gdbarch, regnum) == NULL
	      || *(gdbarch_register_name (current_gdbarch, regnum)) == '\0')
	    continue;
	  tuple_cleanup = make_cleanup_ui_out_tuple_begin_end (uiout, NULL);
	  ui_out_field_int (uiout, "number", regnum);
	  result = get_register (regnum, format);
	  if (result == -1)
	    {
	      do_cleanups (list_cleanup);
	      return MI_CMD_ERROR;
	    }
	  do_cleanups (tuple_cleanup);
	}
    }

  /* Else, list of register #s, just do listed regs.  */
  for (i = 1; i < argc; i++)
    {
      regnum = atoi (argv[i]);

      if (regnum >= 0
	  && regnum < numregs
	  && gdbarch_register_name (current_gdbarch, regnum) != NULL
	  && *gdbarch_register_name (current_gdbarch, regnum) != '\000')
	{
	  tuple_cleanup = make_cleanup_ui_out_tuple_begin_end (uiout, NULL);
	  ui_out_field_int (uiout, "number", regnum);
	  result = get_register (regnum, format);
	  if (result == -1)
	    {
	      do_cleanups (list_cleanup);
	      return MI_CMD_ERROR;
	    }
	  do_cleanups (tuple_cleanup);
	}
      else
	{
	  do_cleanups (list_cleanup);
	  mi_error_message = xstrprintf ("bad register number");
	  return MI_CMD_ERROR;
	}
    }
  do_cleanups (list_cleanup);
  return MI_CMD_DONE;
}

/* Output one register's contents in the desired format.  */
static int
get_register (int regnum, int format)
{
  gdb_byte buffer[MAX_REGISTER_SIZE];
  int optim;
  int realnum;
  CORE_ADDR addr;
  enum lval_type lval;
  static struct ui_stream *stb = NULL;

  stb = ui_out_stream_new (uiout);

  if (format == 'N')
    format = 0;

  frame_register (get_selected_frame (NULL), regnum, &optim, &lval, &addr,
		  &realnum, buffer);

  if (optim)
    {
      mi_error_message = xstrprintf ("Optimized out");
      return -1;
    }

  if (format == 'r')
    {
      int j;
      char *ptr, buf[1024];

      strcpy (buf, "0x");
      ptr = buf + 2;
      for (j = 0; j < register_size (current_gdbarch, regnum); j++)
	{
	  int idx = gdbarch_byte_order (current_gdbarch) == BFD_ENDIAN_BIG ? j
	  : register_size (current_gdbarch, regnum) - 1 - j;
	  sprintf (ptr, "%02x", (unsigned char) buffer[idx]);
	  ptr += 2;
	}
      ui_out_field_string (uiout, "value", buf);
      /*fputs_filtered (buf, gdb_stdout); */
    }
  else
    {
      val_print (register_type (current_gdbarch, regnum), buffer, 0, 0,
		 stb->stream, format, 1, 0, Val_pretty_default);
      ui_out_field_stream (uiout, "value", stb);
      ui_out_stream_delete (stb);
    }
  return 1;
}

/* Write given values into registers. The registers and values are
   given as pairs.  The corresponding MI command is 
   -data-write-register-values <format> [<regnum1> <value1>...<regnumN> <valueN>]*/
enum mi_cmd_result
mi_cmd_data_write_register_values (char *command, char **argv, int argc)
{
  int numregs, i;
  char format;

  /* Note that the test for a valid register must include checking the
     gdbarch_register_name because gdbarch_num_regs may be allocated for
     the union of the register sets within a family of related processors.
     In this case, some entries of gdbarch_register_name will change depending
     upon the particular processor being debugged.  */

  numregs = gdbarch_num_regs (current_gdbarch)
	    + gdbarch_num_pseudo_regs (current_gdbarch);

  if (argc == 0)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_write_register_values: Usage: -data-write-register-values <format> [<regnum1> <value1>...<regnumN> <valueN>]");
      return MI_CMD_ERROR;
    }

  format = (int) argv[0][0];

  if (!target_has_registers)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_write_register_values: No registers.");
      return MI_CMD_ERROR;
    }

  if (!(argc - 1))
    {
      mi_error_message = xstrprintf ("mi_cmd_data_write_register_values: No regs and values specified.");
      return MI_CMD_ERROR;
    }

  if ((argc - 1) % 2)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_write_register_values: Regs and vals are not in pairs.");
      return MI_CMD_ERROR;
    }

  for (i = 1; i < argc; i = i + 2)
    {
      int regnum = atoi (argv[i]);

      if (regnum >= 0 && regnum < numregs
	  && gdbarch_register_name (current_gdbarch, regnum)
	  && *gdbarch_register_name (current_gdbarch, regnum))
	{
	  LONGEST value;

	  /* Get the value as a number.  */
	  value = parse_and_eval_address (argv[i + 1]);

	  /* Write it down.  */
	  regcache_cooked_write_signed (get_current_regcache (), regnum, value);
	}
      else
	{
	  mi_error_message = xstrprintf ("bad register number");
	  return MI_CMD_ERROR;
	}
    }
  return MI_CMD_DONE;
}

/* Evaluate the value of the argument.  The argument is an
   expression. If the expression contains spaces it needs to be
   included in double quotes.  */
enum mi_cmd_result
mi_cmd_data_evaluate_expression (char *command, char **argv, int argc)
{
  struct expression *expr;
  struct cleanup *old_chain = NULL;
  struct value *val;
  struct ui_stream *stb = NULL;

  stb = ui_out_stream_new (uiout);

  if (argc != 1)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_evaluate_expression: Usage: -data-evaluate-expression expression");
      ui_out_stream_delete (stb);
      return MI_CMD_ERROR;
    }

  expr = parse_expression (argv[0]);

  old_chain = make_cleanup (free_current_contents, &expr);

  val = evaluate_expression (expr);

  /* Print the result of the expression evaluation.  */
  val_print (value_type (val), value_contents (val),
	     value_embedded_offset (val), VALUE_ADDRESS (val),
	     stb->stream, 0, 0, 0, 0);

  ui_out_field_stream (uiout, "value", stb);
  ui_out_stream_delete (stb);

  do_cleanups (old_chain);

  return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_target_download (char *args, int from_tty)
{
  char *run;
  struct cleanup *old_cleanups = NULL;

  run = xstrprintf ("load %s", args);
  old_cleanups = make_cleanup (xfree, run);
  execute_command (run, from_tty);

  do_cleanups (old_cleanups);
  return MI_CMD_DONE;
}

/* Connect to the remote target.  */
enum mi_cmd_result
mi_cmd_target_select (char *args, int from_tty)
{
  char *run;
  struct cleanup *old_cleanups = NULL;

  run = xstrprintf ("target %s", args);
  old_cleanups = make_cleanup (xfree, run);

  /* target-select is always synchronous.  Once the call has returned
     we know that we are connected.  */
  /* NOTE: At present all targets that are connected are also
     (implicitly) talking to a halted target.  In the future this may
     change.  */
  execute_command (run, from_tty);

  do_cleanups (old_cleanups);

  /* Issue the completion message here.  */
  if (last_async_command)
    fputs_unfiltered (last_async_command, raw_stdout);
  fputs_unfiltered ("^connected", raw_stdout);
  mi_out_put (uiout, raw_stdout);
  mi_out_rewind (uiout);
  fputs_unfiltered ("\n", raw_stdout);
  do_exec_cleanups (ALL_CLEANUPS);
  return MI_CMD_QUIET;
}

/* DATA-MEMORY-READ:

   ADDR: start address of data to be dumped.
   WORD-FORMAT: a char indicating format for the ``word''.  See 
   the ``x'' command.
   WORD-SIZE: size of each ``word''; 1,2,4, or 8 bytes.
   NR_ROW: Number of rows.
   NR_COL: The number of colums (words per row).
   ASCHAR: (OPTIONAL) Append an ascii character dump to each row.  Use
   ASCHAR for unprintable characters.

   Reads SIZE*NR_ROW*NR_COL bytes starting at ADDR from memory and
   displayes them.  Returns:

   {addr="...",rowN={wordN="..." ,... [,ascii="..."]}, ...}

   Returns: 
   The number of bytes read is SIZE*ROW*COL. */

enum mi_cmd_result
mi_cmd_data_read_memory (char *command, char **argv, int argc)
{
  struct cleanup *cleanups = make_cleanup (null_cleanup, NULL);
  CORE_ADDR addr;
  long total_bytes;
  long nr_cols;
  long nr_rows;
  char word_format;
  struct type *word_type;
  long word_size;
  char word_asize;
  char aschar;
  gdb_byte *mbuf;
  int nr_bytes;
  long offset = 0;
  int optind = 0;
  char *optarg;
  enum opt
    {
      OFFSET_OPT
    };
  static struct mi_opt opts[] =
  {
    {"o", OFFSET_OPT, 1},
    { 0, 0, 0 }
  };

  while (1)
    {
      int opt = mi_getopt ("mi_cmd_data_read_memory", argc, argv, opts,
			   &optind, &optarg);
      if (opt < 0)
	break;
      switch ((enum opt) opt)
	{
	case OFFSET_OPT:
	  offset = atol (optarg);
	  break;
	}
    }
  argv += optind;
  argc -= optind;

  if (argc < 5 || argc > 6)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_read_memory: Usage: ADDR WORD-FORMAT WORD-SIZE NR-ROWS NR-COLS [ASCHAR].");
      return MI_CMD_ERROR;
    }

  /* Extract all the arguments. */

  /* Start address of the memory dump.  */
  addr = parse_and_eval_address (argv[0]) + offset;
  /* The format character to use when displaying a memory word.  See
     the ``x'' command. */
  word_format = argv[1][0];
  /* The size of the memory word.  */
  word_size = atol (argv[2]);
  switch (word_size)
    {
    case 1:
      word_type = builtin_type_int8;
      word_asize = 'b';
      break;
    case 2:
      word_type = builtin_type_int16;
      word_asize = 'h';
      break;
    case 4:
      word_type = builtin_type_int32;
      word_asize = 'w';
      break;
    case 8:
      word_type = builtin_type_int64;
      word_asize = 'g';
      break;
    default:
      word_type = builtin_type_int8;
      word_asize = 'b';
    }
  /* The number of rows.  */
  nr_rows = atol (argv[3]);
  if (nr_rows <= 0)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_read_memory: invalid number of rows.");
      return MI_CMD_ERROR;
    }
  /* Number of bytes per row.  */
  nr_cols = atol (argv[4]);
  if (nr_cols <= 0)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_read_memory: invalid number of columns.");
      return MI_CMD_ERROR;
    }
  /* The un-printable character when printing ascii.  */
  if (argc == 6)
    aschar = *argv[5];
  else
    aschar = 0;

  /* Create a buffer and read it in.  */
  total_bytes = word_size * nr_rows * nr_cols;
  mbuf = xcalloc (total_bytes, 1);
  make_cleanup (xfree, mbuf);

  nr_bytes = target_read (&current_target, TARGET_OBJECT_MEMORY, NULL,
			  mbuf, addr, total_bytes);
  if (nr_bytes <= 0)
    {
      do_cleanups (cleanups);
      mi_error_message = xstrdup ("Unable to read memory.");
      return MI_CMD_ERROR;
    }

  /* Output the header information.  */
  ui_out_field_core_addr (uiout, "addr", addr);
  ui_out_field_int (uiout, "nr-bytes", nr_bytes);
  ui_out_field_int (uiout, "total-bytes", total_bytes);
  ui_out_field_core_addr (uiout, "next-row", addr + word_size * nr_cols);
  ui_out_field_core_addr (uiout, "prev-row", addr - word_size * nr_cols);
  ui_out_field_core_addr (uiout, "next-page", addr + total_bytes);
  ui_out_field_core_addr (uiout, "prev-page", addr - total_bytes);

  /* Build the result as a two dimentional table.  */
  {
    struct ui_stream *stream = ui_out_stream_new (uiout);
    struct cleanup *cleanup_list_memory;
    int row;
    int row_byte;
    cleanup_list_memory = make_cleanup_ui_out_list_begin_end (uiout, "memory");
    for (row = 0, row_byte = 0;
	 row < nr_rows;
	 row++, row_byte += nr_cols * word_size)
      {
	int col;
	int col_byte;
	struct cleanup *cleanup_tuple;
	struct cleanup *cleanup_list_data;
	cleanup_tuple = make_cleanup_ui_out_tuple_begin_end (uiout, NULL);
	ui_out_field_core_addr (uiout, "addr", addr + row_byte);
	/* ui_out_field_core_addr_symbolic (uiout, "saddr", addr + row_byte); */
	cleanup_list_data = make_cleanup_ui_out_list_begin_end (uiout, "data");
	for (col = 0, col_byte = row_byte;
	     col < nr_cols;
	     col++, col_byte += word_size)
	  {
	    if (col_byte + word_size > nr_bytes)
	      {
		ui_out_field_string (uiout, NULL, "N/A");
	      }
	    else
	      {
		ui_file_rewind (stream->stream);
		print_scalar_formatted (mbuf + col_byte, word_type, word_format,
					word_asize, stream->stream);
		ui_out_field_stream (uiout, NULL, stream);
	      }
	  }
	do_cleanups (cleanup_list_data);
	if (aschar)
	  {
	    int byte;
	    ui_file_rewind (stream->stream);
	    for (byte = row_byte; byte < row_byte + word_size * nr_cols; byte++)
	      {
		if (byte >= nr_bytes)
		  {
		    fputc_unfiltered ('X', stream->stream);
		  }
		else if (mbuf[byte] < 32 || mbuf[byte] > 126)
		  {
		    fputc_unfiltered (aschar, stream->stream);
		  }
		else
		  fputc_unfiltered (mbuf[byte], stream->stream);
	      }
	    ui_out_field_stream (uiout, "ascii", stream);
	  }
	do_cleanups (cleanup_tuple);
      }
    ui_out_stream_delete (stream);
    do_cleanups (cleanup_list_memory);
  }
  do_cleanups (cleanups);
  return MI_CMD_DONE;
}

/* DATA-MEMORY-WRITE:

   COLUMN_OFFSET: optional argument. Must be preceeded by '-o'. The
   offset from the beginning of the memory grid row where the cell to
   be written is.
   ADDR: start address of the row in the memory grid where the memory
   cell is, if OFFSET_COLUMN is specified.  Otherwise, the address of
   the location to write to.
   FORMAT: a char indicating format for the ``word''.  See 
   the ``x'' command.
   WORD_SIZE: size of each ``word''; 1,2,4, or 8 bytes
   VALUE: value to be written into the memory address.

   Writes VALUE into ADDR + (COLUMN_OFFSET * WORD_SIZE).

   Prints nothing.  */
enum mi_cmd_result
mi_cmd_data_write_memory (char *command, char **argv, int argc)
{
  CORE_ADDR addr;
  char word_format;
  long word_size;
  /* FIXME: ezannoni 2000-02-17 LONGEST could possibly not be big
     enough when using a compiler other than GCC.  */
  LONGEST value;
  void *buffer;
  struct cleanup *old_chain;
  long offset = 0;
  int optind = 0;
  char *optarg;
  enum opt
    {
      OFFSET_OPT
    };
  static struct mi_opt opts[] =
  {
    {"o", OFFSET_OPT, 1},
    { 0, 0, 0 }
  };

  while (1)
    {
      int opt = mi_getopt ("mi_cmd_data_write_memory", argc, argv, opts,
			   &optind, &optarg);
      if (opt < 0)
	break;
      switch ((enum opt) opt)
	{
	case OFFSET_OPT:
	  offset = atol (optarg);
	  break;
	}
    }
  argv += optind;
  argc -= optind;

  if (argc != 4)
    {
      mi_error_message = xstrprintf ("mi_cmd_data_write_memory: Usage: [-o COLUMN_OFFSET] ADDR FORMAT WORD-SIZE VALUE.");
      return MI_CMD_ERROR;
    }

  /* Extract all the arguments.  */
  /* Start address of the memory dump.  */
  addr = parse_and_eval_address (argv[0]);
  /* The format character to use when displaying a memory word.  See
     the ``x'' command.  */
  word_format = argv[1][0];
  /* The size of the memory word. */
  word_size = atol (argv[2]);

  /* Calculate the real address of the write destination.  */
  addr += (offset * word_size);

  /* Get the value as a number.  */
  value = parse_and_eval_address (argv[3]);
  /* Get the value into an array.  */
  buffer = xmalloc (word_size);
  old_chain = make_cleanup (xfree, buffer);
  store_signed_integer (buffer, word_size, value);
  /* Write it down to memory.  */
  write_memory (addr, buffer, word_size);
  /* Free the buffer.  */
  do_cleanups (old_chain);

  return MI_CMD_DONE;
}

enum mi_cmd_result
mi_cmd_enable_timings (char *command, char **argv, int argc)
{
  if (argc == 0)
    do_timings = 1;
  else if (argc == 1)
    {
      if (strcmp (argv[0], "yes") == 0)
	do_timings = 1;
      else if (strcmp (argv[0], "no") == 0)
	do_timings = 0;
      else
	goto usage_error;
    }
  else
    goto usage_error;
    
  return MI_CMD_DONE;

 usage_error:
  error ("mi_cmd_enable_timings: Usage: %s {yes|no}", command);
  return MI_CMD_ERROR;
}

enum mi_cmd_result
mi_cmd_list_features (char *command, char **argv, int argc)
{
  if (argc == 0)
    {
      struct cleanup *cleanup = NULL;
      cleanup = make_cleanup_ui_out_list_begin_end (uiout, "features");      

      ui_out_field_string (uiout, NULL, "frozen-varobjs");
      ui_out_field_string (uiout, NULL, "pending-breakpoints");
      
      do_cleanups (cleanup);

      return MI_CMD_DONE;
    }

  error ("-list-features should be passed no arguments");
  return MI_CMD_ERROR;
}
 
/* Execute a command within a safe environment.
   Return <0 for error; >=0 for ok.

   args->action will tell mi_execute_command what action
   to perfrom after the given command has executed (display/supress
   prompt, display error). */

static void
captured_mi_execute_command (struct ui_out *uiout, void *data)
{
  struct captured_mi_execute_command_args *args =
    (struct captured_mi_execute_command_args *) data;
  struct mi_parse *context = args->command;

  struct mi_timestamp cmd_finished;

  switch (context->op)
    {

    case MI_COMMAND:
      /* A MI command was read from the input stream.  */
      if (mi_debug_p)
	/* FIXME: gdb_???? */
	fprintf_unfiltered (raw_stdout, " token=`%s' command=`%s' args=`%s'\n",
			    context->token, context->command, context->args);
      /* FIXME: cagney/1999-09-25: Rather than this convoluted
         condition expression, each function should return an
         indication of what action is required and then switch on
         that.  */
      args->action = EXECUTE_COMMAND_DISPLAY_PROMPT;

      if (do_timings)
	current_command_ts = context->cmd_start;

      args->rc = mi_cmd_execute (context);

      if (do_timings)
          timestamp (&cmd_finished);

      if (!target_can_async_p () || !target_executing)
	{
	  /* Print the result if there were no errors.

	     Remember that on the way out of executing a command, you have
	     to directly use the mi_interp's uiout, since the command could 
	     have reset the interpreter, in which case the current uiout 
	     will most likely crash in the mi_out_* routines.  */
	  if (args->rc == MI_CMD_DONE)
	    {
	      fputs_unfiltered (context->token, raw_stdout);
	      fputs_unfiltered ("^done", raw_stdout);
	      mi_out_put (uiout, raw_stdout);
	      mi_out_rewind (uiout);
	      /* Have to check cmd_start, since the command could be
		 -enable-timings.  */
	      if (do_timings && context->cmd_start)
		  print_diff (context->cmd_start, &cmd_finished);
	      fputs_unfiltered ("\n", raw_stdout);
	    }
	  else if (args->rc == MI_CMD_ERROR)
	    {
	      if (mi_error_message)
		{
		  fputs_unfiltered (context->token, raw_stdout);
		  fputs_unfiltered ("^error,msg=\"", raw_stdout);
		  fputstr_unfiltered (mi_error_message, '"', raw_stdout);
		  xfree (mi_error_message);
		  mi_error_message = NULL;
		  fputs_unfiltered ("\"\n", raw_stdout);
		}
	      mi_out_rewind (uiout);
	    }
	  else
	    mi_out_rewind (uiout);
	}
      else if (sync_execution)
	{
	  /* Don't print the prompt. We are executing the target in
	     synchronous mode.  */
	  args->action = EXECUTE_COMMAND_SUPRESS_PROMPT;
	  return;
	}
      break;

    case CLI_COMMAND:
      {
	char *argv[2];
	/* A CLI command was read from the input stream.  */
	/* This "feature" will be removed as soon as we have a
	   complete set of mi commands.  */
	/* Echo the command on the console.  */
	fprintf_unfiltered (gdb_stdlog, "%s\n", context->command);
	/* Call the "console" interpreter.  */
	argv[0] = "console";
	argv[1] = context->command;
	args->rc = mi_cmd_interpreter_exec ("-interpreter-exec", argv, 2);

	/* If we changed interpreters, DON'T print out anything.  */
	if (current_interp_named_p (INTERP_MI)
	    || current_interp_named_p (INTERP_MI1)
	    || current_interp_named_p (INTERP_MI2)
	    || current_interp_named_p (INTERP_MI3))
	  {
	    if (args->rc == MI_CMD_DONE)
	      {
		fputs_unfiltered (context->token, raw_stdout);
		fputs_unfiltered ("^done", raw_stdout);
		mi_out_put (uiout, raw_stdout);
		mi_out_rewind (uiout);
		fputs_unfiltered ("\n", raw_stdout);
		args->action = EXECUTE_COMMAND_DISPLAY_PROMPT;
	      }
	    else if (args->rc == MI_CMD_ERROR)
	      {
		if (mi_error_message)
		  {
		    fputs_unfiltered (context->token, raw_stdout);
		    fputs_unfiltered ("^error,msg=\"", raw_stdout);
		    fputstr_unfiltered (mi_error_message, '"', raw_stdout);
		    xfree (mi_error_message);
		    mi_error_message = NULL;
		    fputs_unfiltered ("\"\n", raw_stdout);
		  }
		mi_out_rewind (uiout);
	      }
	    else
	      mi_out_rewind (uiout);
	  }
	break;
      }

    }

  return;
}


void
mi_execute_command (char *cmd, int from_tty)
{
  struct mi_parse *command;
  struct captured_mi_execute_command_args args;
  struct ui_out *saved_uiout = uiout;

  /* This is to handle EOF (^D). We just quit gdb.  */
  /* FIXME: we should call some API function here.  */
  if (cmd == 0)
    quit_force (NULL, from_tty);

  command = mi_parse (cmd);

  if (command != NULL)
    {
      struct gdb_exception result;

      if (do_timings)
	{
	  command->cmd_start = (struct mi_timestamp *)
	    xmalloc (sizeof (struct mi_timestamp));
	  timestamp (command->cmd_start);
	}

      /* FIXME: cagney/1999-11-04: Can this use of catch_exceptions either
         be pushed even further down or even eliminated?  */
      args.command = command;
      result = catch_exception (uiout, captured_mi_execute_command, &args,
				RETURN_MASK_ALL);
      exception_print (gdb_stderr, result);

      if (args.action == EXECUTE_COMMAND_SUPRESS_PROMPT)
	{
	  /* The command is executing synchronously.  Bail out early
	     suppressing the finished prompt.  */
	  mi_parse_free (command);
	  return;
	}
      if (result.reason < 0)
	{
	  /* The command execution failed and error() was called
	     somewhere.  */
	  fputs_unfiltered (command->token, raw_stdout);
	  fputs_unfiltered ("^error,msg=\"", raw_stdout);
	  if (result.message == NULL)
	    fputs_unfiltered ("unknown error", raw_stdout);
	  else
	      fputstr_unfiltered (result.message, '"', raw_stdout);
	  fputs_unfiltered ("\"\n", raw_stdout);
	  mi_out_rewind (uiout);
	}
      mi_parse_free (command);
    }

  fputs_unfiltered ("(gdb) \n", raw_stdout);
  gdb_flush (raw_stdout);
  /* Print any buffered hook code.  */
  /* ..... */
}

static enum mi_cmd_result
mi_cmd_execute (struct mi_parse *parse)
{
  free_all_values ();

  if (parse->cmd->argv_func != NULL
      || parse->cmd->args_func != NULL)
    {
      /* FIXME: We need to save the token because the command executed
         may be asynchronous and need to print the token again.
         In the future we can pass the token down to the func
         and get rid of the last_async_command.  */
      /* The problem here is to keep the token around when we launch
         the target, and we want to interrupt it later on.  The
         interrupt command will have its own token, but when the
         target stops, we must display the token corresponding to the
         last execution command given.  So we have another string where
         we copy the token (previous_async_command), if this was
         indeed the token of an execution command, and when we stop we
         print that one.  This is possible because the interrupt
         command, when over, will copy that token back into the
         default token string (last_async_command).  */

      if (target_executing)
	{
	  if (!previous_async_command)
	    previous_async_command = xstrdup (last_async_command);
	  if (strcmp (parse->command, "exec-interrupt"))
	    {
	      fputs_unfiltered (parse->token, raw_stdout);
	      fputs_unfiltered ("^error,msg=\"", raw_stdout);
	      fputs_unfiltered ("Cannot execute command ", raw_stdout);
	      fputstr_unfiltered (parse->command, '"', raw_stdout);
	      fputs_unfiltered (" while target running", raw_stdout);
	      fputs_unfiltered ("\"\n", raw_stdout);
	      return MI_CMD_ERROR;
	    }
	}
      last_async_command = xstrdup (parse->token);
      make_exec_cleanup (free_current_contents, &last_async_command);
      /* FIXME: DELETE THIS! */
      if (parse->cmd->args_func != NULL)
	return parse->cmd->args_func (parse->args, 0 /*from_tty */ );
      return parse->cmd->argv_func (parse->command, parse->argv, parse->argc);
    }
  else if (parse->cmd->cli.cmd != 0)
    {
      /* FIXME: DELETE THIS. */
      /* The operation is still implemented by a cli command.  */
      /* Must be a synchronous one.  */
      mi_execute_cli_command (parse->cmd->cli.cmd, parse->cmd->cli.args_p,
			      parse->args);
      return MI_CMD_DONE;
    }
  else
    {
      /* FIXME: DELETE THIS.  */
      fputs_unfiltered (parse->token, raw_stdout);
      fputs_unfiltered ("^error,msg=\"", raw_stdout);
      fputs_unfiltered ("Undefined mi command: ", raw_stdout);
      fputstr_unfiltered (parse->command, '"', raw_stdout);
      fputs_unfiltered (" (missing implementation)", raw_stdout);
      fputs_unfiltered ("\"\n", raw_stdout);
      return MI_CMD_ERROR;
    }
}

/* FIXME: This is just a hack so we can get some extra commands going.
   We don't want to channel things through the CLI, but call libgdb directly.
   Use only for synchronous commands.  */

void
mi_execute_cli_command (const char *cmd, int args_p, const char *args)
{
  if (cmd != 0)
    {
      struct cleanup *old_cleanups;
      char *run;
      if (args_p)
	run = xstrprintf ("%s %s", cmd, args);
      else
	run = xstrdup (cmd);
      if (mi_debug_p)
	/* FIXME: gdb_???? */
	fprintf_unfiltered (gdb_stdout, "cli=%s run=%s\n",
			    cmd, run);
      old_cleanups = make_cleanup (xfree, run);
      execute_command ( /*ui */ run, 0 /*from_tty */ );
      do_cleanups (old_cleanups);
      return;
    }
}

enum mi_cmd_result
mi_execute_async_cli_command (char *mi, char *args, int from_tty)
{
  struct cleanup *old_cleanups;
  char *run;
  char *async_args;

  if (target_can_async_p ())
    {
      async_args = (char *) xmalloc (strlen (args) + 2);
      make_exec_cleanup (free, async_args);
      strcpy (async_args, args);
      strcat (async_args, "&");
      run = xstrprintf ("%s %s", mi, async_args);
      make_exec_cleanup (free, run);
      add_continuation (mi_exec_async_cli_cmd_continuation, NULL);
      old_cleanups = NULL;
    }
  else
    {
      run = xstrprintf ("%s %s", mi, args);
      old_cleanups = make_cleanup (xfree, run);
    }

  if (!target_can_async_p ())
    {
      /* NOTE: For synchronous targets asynchronous behavour is faked by
         printing out the GDB prompt before we even try to execute the
         command.  */
      if (last_async_command)
	fputs_unfiltered (last_async_command, raw_stdout);
      fputs_unfiltered ("^running\n", raw_stdout);
      fputs_unfiltered ("(gdb) \n", raw_stdout);
      gdb_flush (raw_stdout);
    }
  else
    {
      /* FIXME: cagney/1999-11-29: Printing this message before
         calling execute_command is wrong.  It should only be printed
         once gdb has confirmed that it really has managed to send a
         run command to the target.  */
      if (last_async_command)
	fputs_unfiltered (last_async_command, raw_stdout);
      fputs_unfiltered ("^running\n", raw_stdout);
    }

  execute_command ( /*ui */ run, 0 /*from_tty */ );

  if (!target_can_async_p ())
    {
      /* Do this before doing any printing.  It would appear that some
         print code leaves garbage around in the buffer.  */
      do_cleanups (old_cleanups);
      /* If the target was doing the operation synchronously we fake
         the stopped message.  */
      if (last_async_command)
	fputs_unfiltered (last_async_command, raw_stdout);
      fputs_unfiltered ("*stopped", raw_stdout);
      mi_out_put (uiout, raw_stdout);
      mi_out_rewind (uiout);
      if (do_timings)
      	print_diff_now (current_command_ts);
      fputs_unfiltered ("\n", raw_stdout);
      return MI_CMD_QUIET;
    }
  return MI_CMD_DONE;
}

void
mi_exec_async_cli_cmd_continuation (struct continuation_arg *arg)
{
  if (last_async_command)
    fputs_unfiltered (last_async_command, raw_stdout);
  fputs_unfiltered ("*stopped", raw_stdout);
  mi_out_put (uiout, raw_stdout);
  fputs_unfiltered ("\n", raw_stdout);
  fputs_unfiltered ("(gdb) \n", raw_stdout);
  gdb_flush (raw_stdout);
  do_exec_cleanups (ALL_CLEANUPS);
}

void
mi_load_progress (const char *section_name,
		  unsigned long sent_so_far,
		  unsigned long total_section,
		  unsigned long total_sent,
		  unsigned long grand_total)
{
  struct timeval time_now, delta, update_threshold;
  static struct timeval last_update;
  static char *previous_sect_name = NULL;
  int new_section;
  struct ui_out *saved_uiout;

  /* This function is called through deprecated_show_load_progress
     which means uiout may not be correct.  Fix it for the duration
     of this function.  */
  saved_uiout = uiout;

  if (current_interp_named_p (INTERP_MI)
      || current_interp_named_p (INTERP_MI2))
    uiout = mi_out_new (2);
  else if (current_interp_named_p (INTERP_MI1))
    uiout = mi_out_new (1);
  else if (current_interp_named_p (INTERP_MI3))
    uiout = mi_out_new (3);
  else
    return;

  update_threshold.tv_sec = 0;
  update_threshold.tv_usec = 500000;
  gettimeofday (&time_now, NULL);

  delta.tv_usec = time_now.tv_usec - last_update.tv_usec;
  delta.tv_sec = time_now.tv_sec - last_update.tv_sec;

  if (delta.tv_usec < 0)
    {
      delta.tv_sec -= 1;
      delta.tv_usec += 1000000L;
    }

  new_section = (previous_sect_name ?
		 strcmp (previous_sect_name, section_name) : 1);
  if (new_section)
    {
      struct cleanup *cleanup_tuple;
      xfree (previous_sect_name);
      previous_sect_name = xstrdup (section_name);

      if (last_async_command)
	fputs_unfiltered (last_async_command, raw_stdout);
      fputs_unfiltered ("+download", raw_stdout);
      cleanup_tuple = make_cleanup_ui_out_tuple_begin_end (uiout, NULL);
      ui_out_field_string (uiout, "section", section_name);
      ui_out_field_int (uiout, "section-size", total_section);
      ui_out_field_int (uiout, "total-size", grand_total);
      do_cleanups (cleanup_tuple);
      mi_out_put (uiout, raw_stdout);
      fputs_unfiltered ("\n", raw_stdout);
      gdb_flush (raw_stdout);
    }

  if (delta.tv_sec >= update_threshold.tv_sec &&
      delta.tv_usec >= update_threshold.tv_usec)
    {
      struct cleanup *cleanup_tuple;
      last_update.tv_sec = time_now.tv_sec;
      last_update.tv_usec = time_now.tv_usec;
      if (last_async_command)
	fputs_unfiltered (last_async_command, raw_stdout);
      fputs_unfiltered ("+download", raw_stdout);
      cleanup_tuple = make_cleanup_ui_out_tuple_begin_end (uiout, NULL);
      ui_out_field_string (uiout, "section", section_name);
      ui_out_field_int (uiout, "section-sent", sent_so_far);
      ui_out_field_int (uiout, "section-size", total_section);
      ui_out_field_int (uiout, "total-sent", total_sent);
      ui_out_field_int (uiout, "total-size", grand_total);
      do_cleanups (cleanup_tuple);
      mi_out_put (uiout, raw_stdout);
      fputs_unfiltered ("\n", raw_stdout);
      gdb_flush (raw_stdout);
    }

  xfree (uiout);
  uiout = saved_uiout;
}

static void 
timestamp (struct mi_timestamp *tv)
  {
    long usec;
    gettimeofday (&tv->wallclock, NULL);
#ifdef HAVE_GETRUSAGE
    getrusage (RUSAGE_SELF, &rusage);
    tv->utime.tv_sec = rusage.ru_utime.tv_sec;
    tv->utime.tv_usec = rusage.ru_utime.tv_usec;
    tv->stime.tv_sec = rusage.ru_stime.tv_sec;
    tv->stime.tv_usec = rusage.ru_stime.tv_usec;
#else
    usec = get_run_time ();
    tv->utime.tv_sec = usec/1000000L;
    tv->utime.tv_usec = usec - 1000000L*tv->utime.tv_sec;
    tv->stime.tv_sec = 0;
    tv->stime.tv_usec = 0;
#endif
  }

static void 
print_diff_now (struct mi_timestamp *start)
  {
    struct mi_timestamp now;
    timestamp (&now);
    print_diff (start, &now);
  }

static long 
timeval_diff (struct timeval start, struct timeval end)
  {
    return ((end.tv_sec - start.tv_sec) * 1000000L)
      + (end.tv_usec - start.tv_usec);
  }

static void 
print_diff (struct mi_timestamp *start, struct mi_timestamp *end)
  {
    fprintf_unfiltered
      (raw_stdout,
       ",time={wallclock=\"%0.5f\",user=\"%0.5f\",system=\"%0.5f\"}", 
       timeval_diff (start->wallclock, end->wallclock) / 1000000.0, 
       timeval_diff (start->utime, end->utime) / 1000000.0, 
       timeval_diff (start->stime, end->stime) / 1000000.0);
  }

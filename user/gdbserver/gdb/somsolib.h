/* HP SOM Shared library declarations for GDB, the GNU Debugger.
   Copyright (C) 1992 Free Software Foundation, Inc.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Written by the Center for Software Science at the Univerity of Utah
and by Cygnus Support.  */

#ifdef __STDC__		/* Forward decl's for prototypes */
struct target_ops;
struct objfile;
struct section_offsets;
#endif

/* Called to add symbols from a shared library to gdb's symbol table. */

#define SOLIB_ADD(filename, from_tty, targ) \
    som_solib_add (filename, from_tty, targ)

extern void
som_solib_add PARAMS ((char *, int, struct target_ops *));

extern CORE_ADDR
som_solib_get_got_by_pc PARAMS ((CORE_ADDR));

extern int
som_solib_section_offsets PARAMS ((struct objfile *, struct section_offsets *));

/* Function to be called when the inferior starts up, to discover the names
   of shared libraries that are dynamically linked, the base addresses to
   which they are linked, and sufficient information to read in their symbols
   at a later time. */

#define SOLIB_CREATE_INFERIOR_HOOK(PID)	som_solib_create_inferior_hook()

extern void
som_solib_create_inferior_hook PARAMS((void));

/* Function to be called to remove the connection between debugger and
   dynamic linker that was established by SOLIB_CREATE_INFERIOR_HOOK.
   (This operation does not remove shared library information from
   the debugger, as CLEAR_SOLIB does.)
   */
#define SOLIB_REMOVE_INFERIOR_HOOK(PID) som_solib_remove_inferior_hook(PID)

extern void
som_solib_remove_inferior_hook PARAMS((int));

/* This function is called by the "catch load" command.  It allows
   the debugger to be notified by the dynamic linker when a specified
   library file (or any library file, if filename is NULL) is loaded.
   */
#define SOLIB_CREATE_CATCH_LOAD_HOOK(pid,tempflag, filename,cond_string) \
   som_solib_create_catch_load_hook (pid, tempflag, filename, cond_string)

extern void
som_solib_create_catch_load_hook PARAMS((int, int, char *, char *));

/* This function is called by the "catch unload" command.  It allows
   the debugger to be notified by the dynamic linker when a specified
   library file (or any library file, if filename is NULL) is unloaded.
   */
#define SOLIB_CREATE_CATCH_UNLOAD_HOOK(pid,tempflag,filename, cond_string) \
   som_solib_create_catch_unload_hook (pid, tempflag, filename, cond_string)

extern void
som_solib_create_catch_unload_hook PARAMS((int, int, char *, char *));

/* This function returns TRUE if the dynamic linker has just reported
   a load of a library.

   This function must be used only when the inferior has stopped in
   the dynamic linker hook, or undefined results are guaranteed.
   */
#define SOLIB_HAVE_LOAD_EVENT(pid) \
   som_solib_have_load_event (pid)

extern int
som_solib_have_load_event PARAMS((int));

/* This function returns a pointer to the string representation of the
   pathname of the dynamically-linked library that has just been loaded.

   This function must be used only when SOLIB_HAVE_LOAD_EVENT is TRUE,
   or undefined results are guaranteed.

   This string's contents are only valid immediately after the inferior
   has stopped in the dynamic linker hook, and becomes invalid as soon
   as the inferior is continued.  Clients should make a copy of this
   string if they wish to continue the inferior and then access the string.
   */
#define SOLIB_LOADED_LIBRARY_PATHNAME(pid) \
   som_solib_loaded_library_pathname (pid)

extern char *
som_solib_loaded_library_pathname PARAMS((int));

/* This function returns TRUE if the dynamic linker has just reported
   an unload of a library.

   This function must be used only when the inferior has stopped in
   the dynamic linker hook, or undefined results are guaranteed.
   */
#define SOLIB_HAVE_UNLOAD_EVENT(pid) \
   som_solib_have_unload_event (pid)

extern int
som_solib_have_unload_event PARAMS((int));

/* This function returns a pointer to the string representation of the
   pathname of the dynamically-linked library that has just been unloaded.

   This function must be used only when SOLIB_HAVE_UNLOAD_EVENT is TRUE,
   or undefined results are guaranteed.

   This string's contents are only valid immediately after the inferior
   has stopped in the dynamic linker hook, and becomes invalid as soon
   as the inferior is continued.  Clients should make a copy of this
   string if they wish to continue the inferior and then access the string.
   */
#define SOLIB_UNLOADED_LIBRARY_PATHNAME(pid) \
   som_solib_unloaded_library_pathname (pid)

extern char *
som_solib_unloaded_library_pathname PARAMS((int));

/* This function returns TRUE if pc is the address of an instruction that
   lies within the dynamic linker (such as the event hook, or the dld
   itself).

   This function must be used only when a dynamic linker event has been
   caught, and the inferior is being stepped out of the hook, or undefined
   results are guaranteed.
   */
#define SOLIB_IN_DYNAMIC_LINKER(pid,pc) \
   som_solib_in_dynamic_linker (pid, pc)

extern int
som_solib_in_dynamic_linker PARAMS((int, CORE_ADDR));

/* This function must be called when the inferior is killed, and the program
   restarted.  This is not the same as CLEAR_SOLIB, in that it doesn't discard
   any symbol tables.

   Presently, this functionality is not implemented.
   */
#define SOLIB_RESTART() \
   som_solib_restart ()

extern void
som_solib_restart PARAMS((void));

/* If we can't set a breakpoint, and it's in a shared library, just
   disable it.  */

#define DISABLE_UNSETTABLE_BREAK(addr)	(som_solib_address(addr) != NULL)

extern char *
som_solib_address PARAMS ((CORE_ADDR));		/* somsolib.c */

/* If ADDR lies in a shared library, return its name.  */

#define PC_SOLIB(addr)	som_solib_address (addr)

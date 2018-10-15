/* Handle shared libraries for GDB, the GNU Debugger.

   Copyright (C) 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008
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

#include "defs.h"

#include <sys/types.h>
#include <fcntl.h>
#include "gdb_string.h"
#include "symtab.h"
#include "bfd.h"
#include "symfile.h"
#include "objfiles.h"
#include "exceptions.h"
#include "gdbcore.h"
#include "command.h"
#include "target.h"
#include "frame.h"
#include "gdb_regex.h"
#include "inferior.h"
#include "environ.h"
#include "language.h"
#include "gdbcmd.h"
#include "completer.h"
#include "filenames.h"		/* for DOSish file names */
#include "exec.h"
#include "solist.h"
#include "observer.h"
#include "readline/readline.h"

/* Architecture-specific operations.  */

/* Per-architecture data key.  */
static struct gdbarch_data *solib_data;

static void *
solib_init (struct obstack *obstack)
{
  struct target_so_ops **ops;

  ops = OBSTACK_ZALLOC (obstack, struct target_so_ops *);
  *ops = current_target_so_ops;
  return ops;
}

static struct target_so_ops *
solib_ops (struct gdbarch *gdbarch)
{
  struct target_so_ops **ops = gdbarch_data (gdbarch, solib_data);
  return *ops;
}

/* Set the solib operations for GDBARCH to NEW_OPS.  */

void
set_solib_ops (struct gdbarch *gdbarch, struct target_so_ops *new_ops)
{
  struct target_so_ops **ops = gdbarch_data (gdbarch, solib_data);
  *ops = new_ops;
}


/* external data declarations */

/* FIXME: gdbarch needs to control this variable, or else every
   configuration needs to call set_solib_ops.  */
struct target_so_ops *current_target_so_ops;

/* local data declarations */

static struct so_list *so_list_head;	/* List of known shared objects */

/* Local function prototypes */

/* If non-empty, this is a search path for loading non-absolute shared library
   symbol files.  This takes precedence over the environment variables PATH
   and LD_LIBRARY_PATH.  */
static char *solib_search_path = NULL;
static void
show_solib_search_path (struct ui_file *file, int from_tty,
			struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("\
The search path for loading non-absolute shared library symbol files is %s.\n"),
		    value);
}

/*

   GLOBAL FUNCTION

   solib_open -- Find a shared library file and open it.

   SYNOPSIS

   int solib_open (char *in_patname, char **found_pathname);

   DESCRIPTION

   Global variable GDB_SYSROOT is used as a prefix directory
   to search for shared libraries if they have an absolute path.

   Global variable SOLIB_SEARCH_PATH is used as a prefix directory
   (or set of directories, as in LD_LIBRARY_PATH) to search for all
   shared libraries if not found in GDB_SYSROOT.

   Search algorithm:
   * If there is a gdb_sysroot and path is absolute:
   *   Search for gdb_sysroot/path.
   * else
   *   Look for it literally (unmodified).
   * Look in SOLIB_SEARCH_PATH.
   * If available, use target defined search function.
   * If gdb_sysroot is NOT set, perform the following two searches:
   *   Look in inferior's $PATH.
   *   Look in inferior's $LD_LIBRARY_PATH.
   *   
   * The last check avoids doing this search when targetting remote
   * machines since gdb_sysroot will almost always be set.

   RETURNS

   file handle for opened solib, or -1 for failure.  */

int
solib_open (char *in_pathname, char **found_pathname)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);
  int found_file = -1;
  char *temp_pathname = NULL;
  char *p = in_pathname;
  int gdb_sysroot_is_empty;

  gdb_sysroot_is_empty = (gdb_sysroot == NULL || *gdb_sysroot == 0);

  if (! IS_ABSOLUTE_PATH (in_pathname) || gdb_sysroot_is_empty)
    temp_pathname = in_pathname;
  else
    {
      int prefix_len = strlen (gdb_sysroot);

      /* Remove trailing slashes from absolute prefix.  */
      while (prefix_len > 0
	     && IS_DIR_SEPARATOR (gdb_sysroot[prefix_len - 1]))
	prefix_len--;

      /* Cat the prefixed pathname together.  */
      temp_pathname = alloca (prefix_len + strlen (in_pathname) + 1);
      strncpy (temp_pathname, gdb_sysroot, prefix_len);
      temp_pathname[prefix_len] = '\0';
      strcat (temp_pathname, in_pathname);
    }

  /* Now see if we can open it.  */
  found_file = open (temp_pathname, O_RDONLY | O_BINARY, 0);

  /* We try to find the library in various ways.  After each attempt
     (except for the one above), either found_file >= 0 and
     temp_pathname is a malloc'd string, or found_file < 0 and
     temp_pathname does not point to storage that needs to be
     freed.  */

    if (found_file < 0)
      temp_pathname = NULL;
    else
      temp_pathname = xstrdup (temp_pathname);

  /* If the search in gdb_sysroot failed, and the path name is
     absolute at this point, make it relative.  (openp will try and open the
     file according to its absolute path otherwise, which is not what we want.)
     Affects subsequent searches for this solib.  */
  if (found_file < 0 && IS_ABSOLUTE_PATH (in_pathname))
    {
      /* First, get rid of any drive letters etc.  */
      while (!IS_DIR_SEPARATOR (*in_pathname))
        in_pathname++;

      /* Next, get rid of all leading dir separators.  */
      while (IS_DIR_SEPARATOR (*in_pathname))
        in_pathname++;
    }
  
  /* If not found, search the solib_search_path (if any).  */
  if (found_file < 0 && solib_search_path != NULL)
    found_file = openp (solib_search_path, OPF_TRY_CWD_FIRST,
			in_pathname, O_RDONLY | O_BINARY, 0, &temp_pathname);
  
  /* If not found, next search the solib_search_path (if any) for the basename
     only (ignoring the path).  This is to allow reading solibs from a path
     that differs from the opened path.  */
  if (found_file < 0 && solib_search_path != NULL)
    found_file = openp (solib_search_path, OPF_TRY_CWD_FIRST,
                        lbasename (in_pathname), O_RDONLY | O_BINARY, 0,
                        &temp_pathname);

  /* If not found, try to use target supplied solib search method */
  if (found_file < 0 && ops->find_and_open_solib)
    found_file = ops->find_and_open_solib (in_pathname, O_RDONLY | O_BINARY,
					   &temp_pathname);

  /* If not found, next search the inferior's $PATH environment variable. */
  if (found_file < 0 && gdb_sysroot_is_empty)
    found_file = openp (get_in_environ (inferior_environ, "PATH"),
			OPF_TRY_CWD_FIRST, in_pathname, O_RDONLY | O_BINARY, 0,
			&temp_pathname);

  /* If not found, next search the inferior's $LD_LIBRARY_PATH 
     environment variable. */
  if (found_file < 0 && gdb_sysroot_is_empty)
    found_file = openp (get_in_environ (inferior_environ, "LD_LIBRARY_PATH"),
			OPF_TRY_CWD_FIRST, in_pathname, O_RDONLY | O_BINARY, 0,
			&temp_pathname);

  /* Done.  If not found, tough luck.  Return found_file and 
     (optionally) found_pathname.  */
  if (temp_pathname)
    {
      if (found_pathname != NULL)
	*found_pathname = temp_pathname;
      else
	xfree (temp_pathname);
    }
  return found_file;
}


/*

   LOCAL FUNCTION

   solib_map_sections -- open bfd and build sections for shared lib

   SYNOPSIS

   static int solib_map_sections (struct so_list *so)

   DESCRIPTION

   Given a pointer to one of the shared objects in our list
   of mapped objects, use the recorded name to open a bfd
   descriptor for the object, build a section table, and then
   relocate all the section addresses by the base address at
   which the shared object was mapped.

   FIXMES

   In most (all?) cases the shared object file name recorded in the
   dynamic linkage tables will be a fully qualified pathname.  For
   cases where it isn't, do we really mimic the systems search
   mechanism correctly in the below code (particularly the tilde
   expansion stuff?).
 */

static int
solib_map_sections (void *arg)
{
  struct so_list *so = (struct so_list *) arg;	/* catch_errors bogon */
  char *filename;
  char *scratch_pathname;
  int scratch_chan;
  struct section_table *p;
  struct cleanup *old_chain;
  bfd *abfd;

  filename = tilde_expand (so->so_name);

  old_chain = make_cleanup (xfree, filename);
  scratch_chan = solib_open (filename, &scratch_pathname);

  if (scratch_chan < 0)
    {
      perror_with_name (filename);
    }

  /* Leave scratch_pathname allocated.  abfd->name will point to it.  */
  abfd = bfd_fopen (scratch_pathname, gnutarget, FOPEN_RB, scratch_chan);
  if (!abfd)
    {
      close (scratch_chan);
      error (_("Could not open `%s' as an executable file: %s"),
	     scratch_pathname, bfd_errmsg (bfd_get_error ()));
    }

  /* Leave bfd open, core_xfer_memory and "info files" need it.  */
  so->abfd = abfd;
  bfd_set_cacheable (abfd, 1);

  /* copy full path name into so_name, so that later symbol_file_add
     can find it */
  if (strlen (scratch_pathname) >= SO_NAME_MAX_PATH_SIZE)
    error (_("Full path name length of shared library exceeds SO_NAME_MAX_PATH_SIZE in so_list structure."));
  strcpy (so->so_name, scratch_pathname);

  if (!bfd_check_format (abfd, bfd_object))
    {
      error (_("\"%s\": not in executable format: %s."),
	     scratch_pathname, bfd_errmsg (bfd_get_error ()));
    }
  if (build_section_table (abfd, &so->sections, &so->sections_end))
    {
      error (_("Can't find the file sections in `%s': %s"),
	     bfd_get_filename (abfd), bfd_errmsg (bfd_get_error ()));
    }

  for (p = so->sections; p < so->sections_end; p++)
    {
      struct target_so_ops *ops = solib_ops (current_gdbarch);

      /* Relocate the section binding addresses as recorded in the shared
         object's file by the base address to which the object was actually
         mapped. */
      ops->relocate_section_addresses (so, p);

      /* If the target didn't provide information about the address
	 range of the shared object, assume we want the location of
	 the .text section.  */
      if (so->addr_low == 0 && so->addr_high == 0
	  && strcmp (p->the_bfd_section->name, ".text") == 0)
	{
	  so->addr_low = p->addr;
	  so->addr_high = p->endaddr;
	}
    }

  /* Free the file names, close the file now.  */
  do_cleanups (old_chain);

  return (1);
}

/* LOCAL FUNCTION

   free_so --- free a `struct so_list' object

   SYNOPSIS

   void free_so (struct so_list *so)

   DESCRIPTION

   Free the storage associated with the `struct so_list' object SO.
   If we have opened a BFD for SO, close it.  

   The caller is responsible for removing SO from whatever list it is
   a member of.  If we have placed SO's sections in some target's
   section table, the caller is responsible for removing them.

   This function doesn't mess with objfiles at all.  If there is an
   objfile associated with SO that needs to be removed, the caller is
   responsible for taking care of that.  */

void
free_so (struct so_list *so)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);
  char *bfd_filename = 0;

  if (so->sections)
    xfree (so->sections);
      
  if (so->abfd)
    {
      bfd_filename = bfd_get_filename (so->abfd);
      if (! bfd_close (so->abfd))
	warning (_("cannot close \"%s\": %s"),
		 bfd_filename, bfd_errmsg (bfd_get_error ()));
    }

  if (bfd_filename)
    xfree (bfd_filename);

  ops->free_so (so);

  xfree (so);
}


/* Return address of first so_list entry in master shared object list.  */
struct so_list *
master_so_list (void)
{
  return so_list_head;
}


/* A small stub to get us past the arg-passing pinhole of catch_errors.  */

static int
symbol_add_stub (void *arg)
{
  struct so_list *so = (struct so_list *) arg;  /* catch_errs bogon */
  struct section_addr_info *sap;

  /* Have we already loaded this shared object?  */
  ALL_OBJFILES (so->objfile)
    {
      if (strcmp (so->objfile->name, so->so_name) == 0)
	return 1;
    }

  sap = build_section_addr_info_from_section_table (so->sections,
                                                    so->sections_end);

  so->objfile = symbol_file_add (so->so_name, so->from_tty,
				 sap, 0, OBJF_SHARED);
  free_section_addr_info (sap);

  return (1);
}

/* Read in symbols for shared object SO.  If FROM_TTY is non-zero, be
   chatty about it.  Return non-zero if any symbols were actually
   loaded.  */

int
solib_read_symbols (struct so_list *so, int from_tty)
{
  if (so->symbols_loaded)
    {
      if (from_tty)
	printf_unfiltered (_("Symbols already loaded for %s\n"), so->so_name);
    }
  else if (so->abfd == NULL)
    {
      if (from_tty)
	printf_unfiltered (_("Symbol file not found for %s\n"), so->so_name);
    }
  else
    {
      if (catch_errors (symbol_add_stub, so,
			"Error while reading shared library symbols:\n",
			RETURN_MASK_ALL))
	{
	  if (from_tty)
	    printf_unfiltered (_("Loaded symbols for %s\n"), so->so_name);
	  so->symbols_loaded = 1;
	  return 1;
	}
    }

  return 0;
}

/* LOCAL FUNCTION

   update_solib_list --- synchronize GDB's shared object list with inferior's

   SYNOPSIS

   void update_solib_list (int from_tty, struct target_ops *TARGET)

   Extract the list of currently loaded shared objects from the
   inferior, and compare it with the list of shared objects currently
   in GDB's so_list_head list.  Edit so_list_head to bring it in sync
   with the inferior's new list.

   If we notice that the inferior has unloaded some shared objects,
   free any symbolic info GDB had read about those shared objects.

   Don't load symbolic info for any new shared objects; just add them
   to the list, and leave their symbols_loaded flag clear.

   If FROM_TTY is non-null, feel free to print messages about what
   we're doing.

   If TARGET is non-null, add the sections of all new shared objects
   to TARGET's section table.  Note that this doesn't remove any
   sections for shared objects that have been unloaded, and it
   doesn't check to see if the new shared objects are already present in
   the section table.  But we only use this for core files and
   processes we've just attached to, so that's okay.  */

static void
update_solib_list (int from_tty, struct target_ops *target)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);
  struct so_list *inferior = ops->current_sos();
  struct so_list *gdb, **gdb_link;

  /* If we are attaching to a running process for which we 
     have not opened a symbol file, we may be able to get its 
     symbols now!  */
  if (attach_flag &&
      symfile_objfile == NULL)
    catch_errors (ops->open_symbol_file_object, &from_tty, 
		  "Error reading attached process's symbol file.\n",
		  RETURN_MASK_ALL);

  /* GDB and the inferior's dynamic linker each maintain their own
     list of currently loaded shared objects; we want to bring the
     former in sync with the latter.  Scan both lists, seeing which
     shared objects appear where.  There are three cases:

     - A shared object appears on both lists.  This means that GDB
     knows about it already, and it's still loaded in the inferior.
     Nothing needs to happen.

     - A shared object appears only on GDB's list.  This means that
     the inferior has unloaded it.  We should remove the shared
     object from GDB's tables.

     - A shared object appears only on the inferior's list.  This
     means that it's just been loaded.  We should add it to GDB's
     tables.

     So we walk GDB's list, checking each entry to see if it appears
     in the inferior's list too.  If it does, no action is needed, and
     we remove it from the inferior's list.  If it doesn't, the
     inferior has unloaded it, and we remove it from GDB's list.  By
     the time we're done walking GDB's list, the inferior's list
     contains only the new shared objects, which we then add.  */

  gdb = so_list_head;
  gdb_link = &so_list_head;
  while (gdb)
    {
      struct so_list *i = inferior;
      struct so_list **i_link = &inferior;

      /* Check to see whether the shared object *gdb also appears in
	 the inferior's current list.  */
      while (i)
	{
	  if (ops->same)
	    {
	      if (ops->same (gdb, i))
		break;
	    }
	  else
	    {
	      if (! strcmp (gdb->so_original_name, i->so_original_name))
		break;	      
	    }

	  i_link = &i->next;
	  i = *i_link;
	}

      /* If the shared object appears on the inferior's list too, then
         it's still loaded, so we don't need to do anything.  Delete
         it from the inferior's list, and leave it on GDB's list.  */
      if (i)
	{
	  *i_link = i->next;
	  free_so (i);
	  gdb_link = &gdb->next;
	  gdb = *gdb_link;
	}

      /* If it's not on the inferior's list, remove it from GDB's tables.  */
      else
	{
	  /* Notify any observer that the shared object has been
	     unloaded before we remove it from GDB's tables.  */
	  observer_notify_solib_unloaded (gdb);

	  *gdb_link = gdb->next;

	  /* Unless the user loaded it explicitly, free SO's objfile.  */
	  if (gdb->objfile && ! (gdb->objfile->flags & OBJF_USERLOADED))
	    free_objfile (gdb->objfile);

	  /* Some targets' section tables might be referring to
	     sections from so->abfd; remove them.  */
	  remove_target_sections (gdb->abfd);

	  free_so (gdb);
	  gdb = *gdb_link;
	}
    }

  /* Now the inferior's list contains only shared objects that don't
     appear in GDB's list --- those that are newly loaded.  Add them
     to GDB's shared object list.  */
  if (inferior)
    {
      struct so_list *i;

      /* Add the new shared objects to GDB's list.  */
      *gdb_link = inferior;

      /* Fill in the rest of each of the `struct so_list' nodes.  */
      for (i = inferior; i; i = i->next)
	{
	  i->from_tty = from_tty;

	  /* Fill in the rest of the `struct so_list' node.  */
	  catch_errors (solib_map_sections, i,
			"Error while mapping shared library sections:\n",
			RETURN_MASK_ALL);

	  /* If requested, add the shared object's sections to the TARGET's
	     section table.  Do this immediately after mapping the object so
	     that later nodes in the list can query this object, as is needed
	     in solib-osf.c.  */
	  if (target)
	    {
	      int count = (i->sections_end - i->sections);
	      if (count > 0)
		{
		  int space = target_resize_to_sections (target, count);
		  memcpy (target->to_sections + space,
			  i->sections,
			  count * sizeof (i->sections[0]));
		}
	    }

	  /* Notify any observer that the shared object has been
             loaded now that we've added it to GDB's tables.  */
	  observer_notify_solib_loaded (i);
	}
    }
}

/* Return non-zero if SO is the libpthread shared library.

   Uses a fairly simplistic heuristic approach where we check
   the file name against "/libpthread".  This can lead to false
   positives, but this should be good enough in practice.  */

static int
libpthread_solib_p (struct so_list *so)
{
  return (strstr (so->so_name, "/libpthread") != NULL);
}

/* GLOBAL FUNCTION

   solib_add -- read in symbol info for newly added shared libraries

   SYNOPSIS

   void solib_add (char *pattern, int from_tty, struct target_ops
   *TARGET, int readsyms)

   DESCRIPTION

   Read in symbolic information for any shared objects whose names
   match PATTERN.  (If we've already read a shared object's symbol
   info, leave it alone.)  If PATTERN is zero, read them all.

   If READSYMS is 0, defer reading symbolic information until later
   but still do any needed low level processing.

   FROM_TTY and TARGET are as described for update_solib_list, above.  */

void
solib_add (char *pattern, int from_tty, struct target_ops *target, int readsyms)
{
  struct so_list *gdb;

  if (pattern)
    {
      char *re_err = re_comp (pattern);

      if (re_err)
	error (_("Invalid regexp: %s"), re_err);
    }

  update_solib_list (from_tty, target);

  /* Walk the list of currently loaded shared libraries, and read
     symbols for any that match the pattern --- or any whose symbols
     aren't already loaded, if no pattern was given.  */
  {
    int any_matches = 0;
    int loaded_any_symbols = 0;

    for (gdb = so_list_head; gdb; gdb = gdb->next)
      if (! pattern || re_exec (gdb->so_name))
	{
          /* Normally, we would read the symbols from that library
             only if READSYMS is set.  However, we're making a small
             exception for the pthread library, because we sometimes
             need the library symbols to be loaded in order to provide
             thread support (x86-linux for instance).  */
          const int add_this_solib =
            (readsyms || libpthread_solib_p (gdb));

	  any_matches = 1;
	  if (add_this_solib && solib_read_symbols (gdb, from_tty))
	    loaded_any_symbols = 1;
	}

    if (from_tty && pattern && ! any_matches)
      printf_unfiltered
	("No loaded shared libraries match the pattern `%s'.\n", pattern);

    if (loaded_any_symbols)
      {
	struct target_so_ops *ops = solib_ops (current_gdbarch);

	/* Getting new symbols may change our opinion about what is
	   frameless.  */
	reinit_frame_cache ();

	ops->special_symbol_handling ();
      }
  }
}


/*

   LOCAL FUNCTION

   info_sharedlibrary_command -- code for "info sharedlibrary"

   SYNOPSIS

   static void info_sharedlibrary_command ()

   DESCRIPTION

   Walk through the shared library list and print information
   about each attached library.
 */

static void
info_sharedlibrary_command (char *ignore, int from_tty)
{
  struct so_list *so = NULL;	/* link map state variable */
  int header_done = 0;
  int addr_width;

  /* "0x", a little whitespace, and two hex digits per byte of pointers.  */
  addr_width = 4 + (gdbarch_ptr_bit (current_gdbarch) / 4);

  update_solib_list (from_tty, 0);

  for (so = so_list_head; so; so = so->next)
    {
      if (so->so_name[0])
	{
	  if (!header_done)
	    {
	      printf_unfiltered ("%-*s%-*s%-12s%s\n", addr_width, "From",
				 addr_width, "To", "Syms Read",
				 "Shared Object Library");
	      header_done++;
	    }

	  printf_unfiltered ("%-*s", addr_width,
			     so->addr_high != 0
			       ? hex_string_custom (
			           (LONGEST) so->addr_low,
	                           addr_width - 4)
			       : "");
	  printf_unfiltered ("%-*s", addr_width,
			     so->addr_high != 0
			       ? hex_string_custom (
			           (LONGEST) so->addr_high,
	                           addr_width - 4)
			       : "");
	  printf_unfiltered ("%-12s", so->symbols_loaded ? "Yes" : "No");
	  printf_unfiltered ("%s\n", so->so_name);
	}
    }
  if (so_list_head == NULL)
    {
      printf_unfiltered (_("No shared libraries loaded at this time.\n"));
    }
}

/*

   GLOBAL FUNCTION

   solib_address -- check to see if an address is in a shared lib

   SYNOPSIS

   char * solib_address (CORE_ADDR address)

   DESCRIPTION

   Provides a hook for other gdb routines to discover whether or
   not a particular address is within the mapped address space of
   a shared library.

   For example, this routine is called at one point to disable
   breakpoints which are in shared libraries that are not currently
   mapped in.
 */

char *
solib_address (CORE_ADDR address)
{
  struct so_list *so = 0;	/* link map state variable */

  for (so = so_list_head; so; so = so->next)
    {
      struct section_table *p;

      for (p = so->sections; p < so->sections_end; p++)
	{
	  if (p->addr <= address && address < p->endaddr)
	    return (so->so_name);
	}
    }

  return (0);
}

/* Called by free_all_symtabs */

void
clear_solib (void)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);

  /* This function is expected to handle ELF shared libraries.  It is
     also used on Solaris, which can run either ELF or a.out binaries
     (for compatibility with SunOS 4), both of which can use shared
     libraries.  So we don't know whether we have an ELF executable or
     an a.out executable until the user chooses an executable file.

     ELF shared libraries don't get mapped into the address space
     until after the program starts, so we'd better not try to insert
     breakpoints in them immediately.  We have to wait until the
     dynamic linker has loaded them; we'll hit a bp_shlib_event
     breakpoint (look for calls to create_solib_event_breakpoint) when
     it's ready.

     SunOS shared libraries seem to be different --- they're present
     as soon as the process begins execution, so there's no need to
     put off inserting breakpoints.  There's also nowhere to put a
     bp_shlib_event breakpoint, so if we put it off, we'll never get
     around to it.

     So: disable breakpoints only if we're using ELF shared libs.  */
  if (exec_bfd != NULL
      && bfd_get_flavour (exec_bfd) != bfd_target_aout_flavour)
    disable_breakpoints_in_shlibs ();

  while (so_list_head)
    {
      struct so_list *so = so_list_head;
      so_list_head = so->next;
      if (so->abfd)
	remove_target_sections (so->abfd);
      free_so (so);
    }

  ops->clear_solib ();
}

/* GLOBAL FUNCTION

   solib_create_inferior_hook -- shared library startup support

   SYNOPSIS

   void solib_create_inferior_hook ()

   DESCRIPTION

   When gdb starts up the inferior, it nurses it along (through the
   shell) until it is ready to execute it's first instruction.  At this
   point, this function gets called via expansion of the macro
   SOLIB_CREATE_INFERIOR_HOOK.  */

void
solib_create_inferior_hook (void)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);
  ops->solib_create_inferior_hook();
}

/* GLOBAL FUNCTION

   in_solib_dynsym_resolve_code -- check to see if an address is in
                                   dynamic loader's dynamic symbol
				   resolution code

   SYNOPSIS

   int in_solib_dynsym_resolve_code (CORE_ADDR pc)

   DESCRIPTION

   Determine if PC is in the dynamic linker's symbol resolution
   code.  Return 1 if so, 0 otherwise.
*/

int
in_solib_dynsym_resolve_code (CORE_ADDR pc)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);
  return ops->in_dynsym_resolve_code (pc);
}

/*

   LOCAL FUNCTION

   sharedlibrary_command -- handle command to explicitly add library

   SYNOPSIS

   static void sharedlibrary_command (char *args, int from_tty)

   DESCRIPTION

 */

static void
sharedlibrary_command (char *args, int from_tty)
{
  dont_repeat ();
  solib_add (args, from_tty, (struct target_ops *) 0, 1);
}

/* LOCAL FUNCTION

   no_shared_libraries -- handle command to explicitly discard symbols
   from shared libraries.

   DESCRIPTION

   Implements the command "nosharedlibrary", which discards symbols
   that have been auto-loaded from shared libraries.  Symbols from
   shared libraries that were added by explicit request of the user
   are not discarded.  Also called from remote.c.  */

void
no_shared_libraries (char *ignored, int from_tty)
{
  objfile_purge_solibs ();
  clear_solib ();
}

static void
reload_shared_libraries (char *ignored, int from_tty,
			 struct cmd_list_element *e)
{
  no_shared_libraries (NULL, from_tty);
  solib_add (NULL, from_tty, NULL, auto_solib_add);
}

static void
show_auto_solib_add (struct ui_file *file, int from_tty,
		     struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("Autoloading of shared library symbols is %s.\n"),
		    value);
}


/* Handler for library-specific lookup of global symbol NAME in OBJFILE.  Call
   the library-specific handler if it is installed for the current target.  */

struct symbol *
solib_global_lookup (const struct objfile *objfile,
		     const char *name,
		     const char *linkage_name,
		     const domain_enum domain,
		     struct symtab **symtab)
{
  struct target_so_ops *ops = solib_ops (current_gdbarch);

  if (ops->lookup_lib_global_symbol != NULL)
    return ops->lookup_lib_global_symbol (objfile, name, linkage_name,
					  domain, symtab);
  return NULL;
}


extern initialize_file_ftype _initialize_solib; /* -Wmissing-prototypes */

void
_initialize_solib (void)
{
  struct cmd_list_element *c;

  solib_data = gdbarch_data_register_pre_init (solib_init);

  add_com ("sharedlibrary", class_files, sharedlibrary_command,
	   _("Load shared object library symbols for files matching REGEXP."));
  add_info ("sharedlibrary", info_sharedlibrary_command,
	    _("Status of loaded shared object libraries."));
  add_com ("nosharedlibrary", class_files, no_shared_libraries,
	   _("Unload all shared object library symbols."));

  add_setshow_boolean_cmd ("auto-solib-add", class_support,
			   &auto_solib_add, _("\
Set autoloading of shared library symbols."), _("\
Show autoloading of shared library symbols."), _("\
If \"on\", symbols from all shared object libraries will be loaded\n\
automatically when the inferior begins execution, when the dynamic linker\n\
informs gdb that a new library has been loaded, or when attaching to the\n\
inferior.  Otherwise, symbols must be loaded manually, using `sharedlibrary'."),
			   NULL,
			   show_auto_solib_add,
			   &setlist, &showlist);

  add_setshow_filename_cmd ("sysroot", class_support,
			    &gdb_sysroot, _("\
Set an alternate system root."), _("\
Show the current system root."), _("\
The system root is used to load absolute shared library symbol files.\n\
For other (relative) files, you can add directories using\n\
`set solib-search-path'."),
			    reload_shared_libraries,
			    NULL,
			    &setlist, &showlist);

  add_alias_cmd ("solib-absolute-prefix", "sysroot", class_support, 0,
		 &setlist);
  add_alias_cmd ("solib-absolute-prefix", "sysroot", class_support, 0,
		 &showlist);

  add_setshow_optional_filename_cmd ("solib-search-path", class_support,
				     &solib_search_path, _("\
Set the search path for loading non-absolute shared library symbol files."), _("\
Show the search path for loading non-absolute shared library symbol files."), _("\
This takes precedence over the environment variables PATH and LD_LIBRARY_PATH."),
				     reload_shared_libraries,
				     show_solib_search_path,
				     &setlist, &showlist);
}

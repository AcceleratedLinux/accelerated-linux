/* modprobe.c: insert a module into the kernel, intelligently.
    Copyright (C) 2001  Rusty Russell.
    Copyright (C) 2002, 2003  Rusty Russell, IBM Corporation.

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#define _GNU_SOURCE /* asprintf */

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <elf.h>
#include <getopt.h>
#include <fnmatch.h>
#include <asm/unistd.h>
#include <sys/wait.h>
#include <syslog.h>

#define streq(a,b) (strcmp((a),(b)) == 0)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include "zlibsupport.h"
#include "list.h"
#include "backwards_compat.c"

extern long init_module(void *, unsigned long, const char *);
extern long delete_module(const char *, unsigned int);

struct module {
	struct list_head list;
	char *modname;
	char filename[0];
};

#ifndef MODULE_DIR
#define MODULE_DIR "/lib/modules"
#endif

typedef void (*errfn_t)(const char *fmt, ...);

/* Do we use syslog or stderr for messages? */
static int log;

static void message(const char *prefix, const char *fmt, va_list *arglist)
{
	char *buf, *buf2;

	vasprintf(&buf, fmt, *arglist);
	asprintf(&buf2, "%s%s", prefix, buf);

	if (log)
		syslog(LOG_NOTICE, "%s", buf2);
	else
		fprintf(stderr, "%s", buf2);
	free(buf2);
	free(buf);
}

static int warned = 0;
static void warn(const char *fmt, ...)
{
	va_list arglist;
	warned++;
	va_start(arglist, fmt);
	message("WARNING: ", fmt, &arglist);
	va_end(arglist);
}

static void fatal(const char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	message("FATAL: ", fmt, &arglist);
	va_end(arglist);
	exit(1);
}


static void grammar(const char *cmd, const char *filename, unsigned int line)
{
	warn("%s line %u: ignoring bad line starting with '%s'\n",
	     filename, line, cmd);
}

static void *do_nofail(void *ptr, const char *file, int line, const char *expr)
{
	if (!ptr) {
		fatal("Memory allocation failure %s line %d: %s.\n",
		      file, line, expr);
	}
	return ptr;
}

#define NOFAIL(ptr)	do_nofail((ptr), __FILE__, __LINE__, #ptr)

static void print_usage(const char *progname)
{
	fprintf(stderr,
		"Usage: %s [-v] [-V] [-C config-file] [-n] [-i] [-q] [-b] [-o <modname>] <modname> [parameters...]\n"
		"%s -r [-n] [-i] [-v] <modulename> ...\n"
		"%s -l -t <dirname> [ -a <modulename> ...]\n",
		progname, progname, progname);
	exit(1);
}

static int fgetc_wrapped(FILE *file, unsigned int *linenum)
{
	for (;;) {
	  	int ch = fgetc(file);
		if (ch != '\\')
			return ch;
		ch = fgetc(file);
		if (ch != '\n')
			return ch;
		if (linenum)
			(*linenum)++;
	}
}

static char *getline_wrapped(FILE *file, unsigned int *linenum)
{
	int size = 1024;
	int i = 0;
	char *buf = NOFAIL(malloc(size));
	for(;;) {
		int ch = fgetc_wrapped(file, linenum);
		if (i == size) {
			size *= 2;
			buf = NOFAIL(realloc(buf, size));
		}
		if (ch < 0 && i == 0) {
			free(buf);
			return NULL;
		}
		if (ch < 0 || ch == '\n') {
			if (linenum)
				(*linenum)++;
			buf[i] = '\0';
			return NOFAIL(realloc(buf, i+1));
		}
		buf[i++] = ch;
	}
}

static struct module *find_module(const char *filename, struct list_head *list)
{
	struct module *i;

	list_for_each_entry(i, list, list) {
		if (strcmp(i->filename, filename) == 0)
			return i;
	}
	return NULL;
}

/* Convert filename to the module name.  Works if filename == modname, too. */
static void filename2modname(char *modname, const char *filename)
{
	const char *afterslash;
	unsigned int i;

	afterslash = strrchr(filename, '/');
	if (!afterslash)
		afterslash = filename;
	else
		afterslash++;

	/* Convert to underscores, stop at first . */
	for (i = 0; afterslash[i] && afterslash[i] != '.'; i++) {
		if (afterslash[i] == '-')
			modname[i] = '_';
		else
			modname[i] = afterslash[i];
	}
	modname[i] = '\0';
}

static int lock_file(const char *filename)
{
	int fd = open(filename, O_RDWR, 0);

	if (fd >= 0) {
		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		fcntl(fd, F_SETLKW, &lock);
	} else
		/* Read-only filesystem?  There goes locking... */
		fd = open(filename, O_RDONLY, 0);
	return fd;
}

static void unlock_file(int fd)
{
	/* Valgrind is picky... */
	close(fd);
}

static void add_module(char *filename, int namelen, struct list_head *list)
{
	struct module *mod;

	/* If it's a duplicate: move it to the end, so it gets
	   inserted where it is *first* required. */
	mod = find_module(filename, list);
	if (mod)
		list_del(&mod->list);
	else {
		/* No match.  Create a new module. */
		mod = NOFAIL(malloc(sizeof(struct module) + namelen + 1));
		memcpy(mod->filename, filename, namelen);
		mod->filename[namelen] = '\0';
		mod->modname = NOFAIL(malloc(namelen + 1));
		filename2modname(mod->modname, mod->filename);
	}

	list_add_tail(&mod->list, list);
}

/* Compare len chars of a to b, with _ and - equivalent. */
static int modname_equal(const char *a, const char *b, unsigned int len)
{
	unsigned int i;

	if (strlen(b) != len)
		return 0;

	for (i = 0; i < len; i++) {
		if ((a[i] == '_' || a[i] == '-')
		    && (b[i] == '_' || b[i] == '-'))
			continue;
		if (a[i] != b[i])
			return 0;
	}
	return 1;
}

/* Fills in list of modules if this is the line we want. */
static int add_modules_dep_line(char *line,
				const char *name,
				struct list_head *list)
{
	char *ptr;
	int len;
	char *modname;

	/* Ignore lines without : or which start with a # */
	ptr = index(line, ':');
	if (ptr == NULL || line[strspn(line, "\t ")] == '#')
		return 0;

	/* Is this the module we are looking for? */
	*ptr = '\0';
	if (strrchr(line, '/'))
		modname = strrchr(line, '/') + 1;
	else
		modname = line;

	len = strlen(modname);
	if (strchr(modname, '.'))
		len = strchr(modname, '.') - modname;
	if (!modname_equal(modname, name, len))
		return 0;

	/* Create the list. */
	add_module(line, ptr - line, list);

	ptr++;
	for(;;) {
		char *dep_start;
		ptr += strspn(ptr, " \t");
		if (*ptr == '\0')
			break;
		dep_start = ptr;
		ptr += strcspn(ptr, " \t");
		add_module(dep_start, ptr - dep_start, list);
	}
	return 1;
}

static void read_depends(const char *dirname,
			 const char *start_name,
			 struct list_head *list)
{
	char *modules_dep_name;
	char *line;
	FILE *modules_dep;
	int done = 0;

	asprintf(&modules_dep_name, "%s/%s", dirname, "modules.dep");
	modules_dep = fopen(modules_dep_name, "r");
	if (!modules_dep)
		fatal("Could not load %s: %s\n",
		      modules_dep_name, strerror(errno));

	/* Stop at first line, as we can have duplicates (eg. symlinks
           from boot/ */
	while (!done && (line = getline_wrapped(modules_dep, NULL)) != NULL) {
		done = add_modules_dep_line(line, start_name, list);
		free(line);
	}
	fclose(modules_dep);
	free(modules_dep_name);
}

/* We use error numbers in a loose translation... */
static const char *insert_moderror(int err)
{
	switch (err) {
	case ENOEXEC:
		return "Invalid module format";
	case ENOENT:
		return "Unknown symbol in module, or unknown parameter (see dmesg)";
	case ENOSYS:
		return "Kernel does not have module support";
	default:
		return strerror(err);
	}
}

static const char *remove_moderror(int err)
{
	switch (err) {
	case ENOENT:
		return "No such module";
	case ENOSYS:
		return "Kernel does not have module unloading support";
	default:
		return strerror(err);
	}
}

/* Is module in /proc/modules?  If so, fill in usecount if not NULL. 
   0 means no, 1 means yes, -1 means unknown.
 */
static int module_in_kernel(const char *modname, unsigned int *usecount)
{
	FILE *proc_modules;
	char *line;

again:
	/* Might not be mounted yet.  Don't fail. */
	proc_modules = fopen("/proc/modules", "r");
	if (!proc_modules)
		return -1;

	while ((line = getline_wrapped(proc_modules, NULL)) != NULL) {
		char *entry = strtok(line, " \n");

		if (entry && streq(entry, modname)) {
			/* If it exists, usecount is the third entry. */
			if (!strtok(NULL, " \n"))
				goto out;

			if (!(entry = strtok(NULL, " \n"))) /* usecount */
				goto out;
			else
				if (usecount)
					*usecount = atoi(entry);

			/* Followed by - then status. */
			if (strtok(NULL, " \n")
			    && (entry = strtok(NULL, " \n")) != NULL) {
				/* Locking will fail on ro fs, we might hit
				 * cases where module is in flux.  Spin. */
				if (streq(entry, "Loading")
				    || streq(entry, "Unloading")) {
					usleep(100000);
					free(line);
					fclose(proc_modules);
					goto again;
				}
			}

		out:
			free(line);
			fclose(proc_modules);
			return 1;
		}
		free(line);
	}
	fclose(proc_modules);
	return 0;
}

static void replace_modname(struct module *module,
			    void *mem, unsigned long len,
			    const char *oldname, const char *newname)
{
	char *p;

	/* 64 - sizeof(unsigned long) - 1 */
	if (strlen(newname) > 55)
		fatal("New name %s is too long\n", newname);

	/* Find where it is in the module structure.  Don't assume layout! */
	for (p = mem; p < (char *)mem + len - strlen(oldname); p++) {
		if (memcmp(p, oldname, strlen(oldname)) == 0) {
			strcpy(p, newname);
			return;
		}
	}

	warn("Could not find old name in %s to replace!\n", module->filename);
}

static void *get_section32(void *file,
			   unsigned long size,
			   const char *name,
			   unsigned long *secsize)
{
	Elf32_Ehdr *hdr = file;
	Elf32_Shdr *sechdrs = file + hdr->e_shoff;
	const char *secnames;
	unsigned int i;

	/* Too short? */
	if (size < sizeof(*hdr))
		return NULL;
	if (size < hdr->e_shoff + hdr->e_shnum * sizeof(sechdrs[0]))
		return NULL;
	if (size < sechdrs[hdr->e_shstrndx].sh_offset)
		return NULL;
		
	secnames = file + sechdrs[hdr->e_shstrndx].sh_offset;
	for (i = 1; i < hdr->e_shnum; i++)
		if (strcmp(secnames + sechdrs[i].sh_name, name) == 0) {
			*secsize = sechdrs[i].sh_size;
			return file + sechdrs[i].sh_offset;
		}
	return NULL;
}

static void *get_section64(void *file,
			   unsigned long size,
			   const char *name,
			   unsigned long *secsize)
{
	Elf64_Ehdr *hdr = file;
	Elf64_Shdr *sechdrs = file + hdr->e_shoff;
	const char *secnames;
	unsigned int i;

	/* Too short? */
	if (size < sizeof(*hdr))
		return NULL;
	if (size < hdr->e_shoff + hdr->e_shnum * sizeof(sechdrs[0]))
		return NULL;
	if (size < sechdrs[hdr->e_shstrndx].sh_offset)
		return NULL;
		
	secnames = file + sechdrs[hdr->e_shstrndx].sh_offset;
	for (i = 1; i < hdr->e_shnum; i++)
		if (strcmp(secnames + sechdrs[i].sh_name, name) == 0) {
			*secsize = sechdrs[i].sh_size;
			return file + sechdrs[i].sh_offset;
		}
	return NULL;
}

static int elf_ident(void *mod, unsigned long size)
{
	/* "\177ELF" <byte> where byte = 001 for 32-bit, 002 for 64 */
	char *ident = mod;

	if (size < EI_CLASS || memcmp(mod, ELFMAG, SELFMAG) != 0)
		return ELFCLASSNONE;
	return ident[EI_CLASS];
}

static void *get_section(void *file,
			 unsigned long size,
			 const char *name,
			 unsigned long *secsize)
{
	switch (elf_ident(file, size)) {
	case ELFCLASS32:
		return get_section32(file, size, name, secsize);
	case ELFCLASS64:
		return get_section64(file, size, name, secsize);
	default:
		return NULL;
	}
}

static void rename_module(struct module *module,
			  void *mod,
			  unsigned long len,
			  const char *newname)
{
	void *modstruct;
	unsigned long modstruct_len;

	/* Old-style */
	modstruct = get_section(mod, len, ".gnu.linkonce.this_module",
				&modstruct_len);
	/* New-style */
	if (!modstruct)
		modstruct = get_section(mod, len, "__module", &modstruct_len);
	if (!modstruct)
		warn("Could not find module name to change in %s\n",
		     module->filename);
	else
		replace_modname(module, modstruct, modstruct_len,
				module->modname, newname);
}

/* Kernel told to ignore these sections if SHF_ALLOC not set. */
static void invalidate_section32(void *mod, const char *secname)
{
	Elf32_Ehdr *hdr = mod;
	Elf32_Shdr *sechdrs = mod + hdr->e_shoff;
	const char *secnames = mod + sechdrs[hdr->e_shstrndx].sh_offset;
	unsigned int i;

	for (i = 1; i < hdr->e_shnum; i++)
		if (strcmp(secnames+sechdrs[i].sh_name, secname) == 0)
			sechdrs[i].sh_flags &= ~SHF_ALLOC;
}

static void invalidate_section64(void *mod, const char *secname)
{
	Elf64_Ehdr *hdr = mod;
	Elf64_Shdr *sechdrs = mod + hdr->e_shoff;
	const char *secnames = mod + sechdrs[hdr->e_shstrndx].sh_offset;
	unsigned int i;

	for (i = 1; i < hdr->e_shnum; i++)
		if (strcmp(secnames+sechdrs[i].sh_name, secname) == 0)
			sechdrs[i].sh_flags &= ~(unsigned long long)SHF_ALLOC;
}

static void strip_section(struct module *module,
			  void *mod,
			  unsigned long len,
			  const char *secname)
{
	switch (elf_ident(mod, len)) {
	case ELFCLASS32:
		invalidate_section32(mod, secname);
		break;
	case ELFCLASS64:
		invalidate_section64(mod, secname);
		break;
	default:
		warn("Unknown module format in %s: not forcing version\n",
		     module->filename);
	}
}

static const char *next_string(const char *string, unsigned long *secsize)
{
	/* Skip non-zero chars */
	while (string[0]) {
		string++;
		if ((*secsize)-- <= 1)
			return NULL;
	}

	/* Skip any zero padding. */
	while (!string[0]) {
		string++;
		if ((*secsize)-- <= 1)
			return NULL;
	}
	return string;
}

static void clear_magic(struct module *module, void *mod, unsigned long len)
{
	const char *p;
	unsigned long modlen;

	/* Old-style: __vermagic section */
	strip_section(module, mod, len, "__vermagic");

	/* New-style: in .modinfo section */
	for (p = get_section(mod, len, ".modinfo", &modlen);
	     p;
	     p = next_string(p, &modlen)) {
		if (strncmp(p, "vermagic=", strlen("vermagic=")) == 0) {
			memset((char *)p, 0, strlen(p));
			return;
		}
	}
}

struct module_options
{
	struct module_options *next;
	char *modulename;
	char *options;
};

struct module_command
{
	struct module_command *next;
	char *modulename;
	char *command;
};

struct module_alias
{
	struct module_alias *next;
	char *module;
};

struct module_blacklist
{
	struct module_blacklist *next;
	char *modulename;
};

/* Link in a new option line from the config file. */
static struct module_options *
add_options(const char *modname,
	    const char *option,
	    struct module_options *options)
{
	struct module_options *new;
	char *tab; 

	new = NOFAIL(malloc(sizeof(*new)));
	new->modulename = NOFAIL(strdup(modname));
	new->options = NOFAIL(strdup(option));
	/* We can handle tabs, kernel can't. */
	for (tab = strchr(new->options, '\t'); tab; tab = strchr(tab, '\t'))
		*tab = ' ';
	new->next = options;
	return new;
}

/* Link in a new install line from the config file. */
static struct module_command *
add_command(const char *modname,
	       const char *command,
	       struct module_command *commands)
{
	struct module_command *new;

	new = NOFAIL(malloc(sizeof(*new)));
	new->modulename = NOFAIL(strdup(modname));
	new->command = NOFAIL(strdup(command));
	new->next = commands;
	return new;
}

/* Link in a new alias line from the config file. */
static struct module_alias *
add_alias(const char *modname, struct module_alias *aliases)
{
	struct module_alias *new;

	new = NOFAIL(malloc(sizeof(*new)));
	new->module = NOFAIL(strdup(modname));
	new->next = aliases;
	return new;
}

/* Link in a new blacklist line from the config file. */
static struct module_blacklist *
add_blacklist(const char *modname, struct module_blacklist *blacklist)
{
	struct module_blacklist *new;

	new = NOFAIL(malloc(sizeof(*new)));
	new->modulename = NOFAIL(strdup(modname));
	new->next = blacklist;
	return new;
}

/* Find blacklist commands if any. */
static  int
find_blacklist(const char *modname, const struct module_blacklist *blacklist)
{
	while (blacklist) {
		if (strcmp(blacklist->modulename, modname) == 0)
			return 1;
		blacklist = blacklist->next;
	}
	return 0;
}

/* return a new alias list, with backlisted elems filtered out */
static struct module_alias *
apply_blacklist(const struct module_alias *aliases,
		const struct module_blacklist *blacklist)
{
	struct module_alias *result = NULL;
	while (aliases) {
		char *modname = aliases->module;
		if (!find_blacklist(modname, blacklist))
			result = add_alias(modname, result);
		aliases = aliases->next;
	}
	return result;
}

/* Find install commands if any. */
static const char *find_command(const char *modname,
				const struct module_command *commands)
{
	while (commands) {
		if (fnmatch(commands->modulename, modname, 0) == 0)
			return commands->command;
		commands = commands->next;
	}
	return NULL;
}

static char *append_option(char *options, const char *newoption)
{
	options = NOFAIL(realloc(options, strlen(options) + 1
				 + strlen(newoption) + 1));
	if (strlen(options)) strcat(options, " ");
	strcat(options, newoption);
	return options;
}

/* Add to options */
static char *add_extra_options(const char *modname,
			       char *optstring,
			       const struct module_options *options)
{
	while (options) {
		if (strcmp(options->modulename, modname) == 0)
			optstring = append_option(optstring, options->options);
		options = options->next;
	}
	return optstring;
}

/* If we don't flush, then child processes print before we do */
static void verbose_printf(int verbose, const char *fmt, ...)
{
	va_list arglist;

	if (verbose) {
		va_start(arglist, fmt);
		vprintf(fmt, arglist);
		fflush(stdout);
		va_end(arglist);
	}
}

/* Do an install/remove command: replace $CMDLINE_OPTS if it's specified. */
static void do_command(const char *modname,
		       const char *command,
		       int verbose, int dry_run,
		       errfn_t error,
		       const char *type,
		       const char *cmdline_opts)
{
	int ret;
	char *p, *replaced_cmd = NOFAIL(strdup(command));

	while ((p = strstr(replaced_cmd, "$CMDLINE_OPTS")) != NULL) {
		char *new;
		asprintf(&new, "%.*s%s%s",
			 p - replaced_cmd, replaced_cmd, cmdline_opts,
			 p + strlen("$CMDLINE_OPTS"));
		NOFAIL(new);
		free(replaced_cmd);
		replaced_cmd = new;
	}

	verbose_printf(verbose, "%s %s\n", type, replaced_cmd);
	if (dry_run)
		return;

	setenv("MODPROBE_MODULE", modname, 1);
	ret = system(replaced_cmd);
	if (ret == -1 || WEXITSTATUS(ret))
		error("Error running %s command for %s\n", type, modname);
	free(replaced_cmd);
}

/* Actually do the insert.  Frees second arg. */
static void insmod(struct list_head *list,
		   char *optstring,
		   const char *newname,
		   int first_time,
		   errfn_t error,
		   int dry_run,
		   int verbose,
		   const struct module_options *options,
		   const struct module_command *commands,
		   int ignore_commands,
		   int ignore_proc,
		   int strip_vermagic,
		   int strip_modversion,
		   const char *cmdline_opts)
{
	int ret, fd;
	unsigned long len;
	void *map;
	const char *command;
	struct module *mod = list_entry(list->next, struct module, list);

	/* Take us off the list. */
	list_del(&mod->list);

	/* Do things we (or parent) depend on first, but don't die if
	 * they fail. */
	if (!list_empty(list)) {
		insmod(list, NOFAIL(strdup("")), NULL, 0, warn,
		       dry_run, verbose, options, commands, 0, ignore_proc,
		       strip_vermagic, strip_modversion, cmdline_opts);
	}

	/* Lock before we look, in case it's initializing. */
	fd = lock_file(mod->filename);
	if (fd < 0) {
		error("Could not open '%s': %s\n",
		      mod->filename, strerror(errno));
		goto out_optstring;
	}

	/* Don't do ANYTHING if already in kernel. */
	if (!ignore_proc
	    && module_in_kernel(newname ?: mod->modname, NULL) == 1) {
		if (first_time)
			error("Module %s already in kernel.\n",
			      newname ?: mod->modname);
		goto out_unlock;
	}

	command = find_command(mod->modname, commands);
	if (command && !ignore_commands) {
		/* It might recurse: unlock. */
		unlock_file(fd);
		do_command(mod->modname, command, verbose, dry_run, error,
			   "install", cmdline_opts);
		goto out_optstring;
	}

	map = grab_fd(fd, &len);
	if (!map) {
		error("Could not read '%s': %s\n",
		      mod->filename, strerror(errno));
		goto out_unlock;
	}

	/* Rename it? */
	if (newname)
		rename_module(mod, map, len, newname);

	if (strip_modversion)
		strip_section(mod, map, len, "__versions");
	if (strip_vermagic)
		clear_magic(mod, map, len);

	/* Config file might have given more options */
	optstring = add_extra_options(mod->modname, optstring, options);

	verbose_printf(verbose, "insmod %s %s\n", mod->filename, optstring);

	if (dry_run)
		goto out;

	ret = init_module(map, len, optstring);
	if (ret != 0) {
		if (errno == EEXIST) {
			if (first_time)
				error("Module %s already in kernel.\n",
				      newname ?: mod->modname);
			goto out_unlock;
		}
		error("Error inserting %s (%s): %s\n",
		      mod->modname, mod->filename, insert_moderror(errno));
	}
 out:
	release_file(map, len);
 out_unlock:
	unlock_file(fd);
 out_optstring:
	free(optstring);
	return;
}

/* Do recursive removal. */
static void rmmod(struct list_head *list,
		  const char *name,
		  int first_time,
		  errfn_t error,
		  int dry_run,
		  int verbose,
		  struct module_command *commands,
		  int ignore_commands,
		  int ignore_inuse,
		  const char *cmdline_opts)
{
	const char *command;
	unsigned int usecount = 0;
	int lock;
	struct module *mod = list_entry(list->next, struct module, list);

	/* Take first one off the list. */
	list_del(&mod->list);

	/* Ignore failure; it's best effort here. */
	lock = lock_file(mod->filename);

	if (!name)
		name = mod->modname;

	/* Even if renamed, find commands to orig. name. */
	command = find_command(mod->modname, commands);
	if (command && !ignore_commands) {
		/* It might recurse: unlock. */
		unlock_file(lock);
		do_command(mod->modname, command, verbose, dry_run, error,
			   "remove", cmdline_opts);
		goto remove_rest_no_unlock;
	}

	if (module_in_kernel(name, &usecount) == 0)
		goto nonexistent_module;

	if (usecount != 0) {
		if (!ignore_inuse)
			error("Module %s is in use.\n", name);
		goto remove_rest;
	}

	verbose_printf(verbose, "rmmod %s\n", mod->filename);

	if (dry_run)
		goto remove_rest;

	if (delete_module(name, O_EXCL) != 0) {
		if (errno == ENOENT)
			goto nonexistent_module;
		error("Error removing %s (%s): %s\n",
		      name, mod->filename,
		      remove_moderror(errno));
	}

 remove_rest:
	unlock_file(lock);
 remove_rest_no_unlock:
	/* Now do things we depend. */
	if (!list_empty(list))
		rmmod(list, NULL, 0, warn, dry_run, verbose, commands,
		      0, 1, cmdline_opts);
	return;

nonexistent_module:
	if (first_time)
		fatal("Module %s is not in kernel.\n", mod->modname);
	goto remove_rest;
}

/* Does path contain directory(s) subpath? */
static int type_matches(const char *path, const char *subpath)
{
	char *subpath_with_slashes;
	int ret;

	asprintf(&subpath_with_slashes, "/%s/", subpath);
	NOFAIL(subpath_with_slashes);

	ret = (strstr(path, subpath_with_slashes) != NULL);
	free(subpath_with_slashes);
	return ret;
}

static char *underscores(char *string)
{
	if (string) {
		unsigned int i;
		for (i = 0; string[i]; i++)
			if (string[i] == '-')
				string[i] = '_';
	}
	return string;
}

static int do_wildcard(const char *dirname,
		       const char *type,
		       const char *wildcard)
{
	char modules_dep_name[strlen(dirname) + sizeof("modules.dep") + 1];
	char *line, *wcard;
	FILE *modules_dep;

	/* Canonicalize wildcard */
	wcard = strdup(wildcard);
	underscores(wcard);

	sprintf(modules_dep_name, "%s/%s", dirname, "modules.dep");
	modules_dep = fopen(modules_dep_name, "r");
	if (!modules_dep)
		fatal("Could not load %s: %s\n",
		      modules_dep_name, strerror(errno));

	while ((line = getline_wrapped(modules_dep, NULL)) != NULL) {
		char *ptr;

		/* Ignore lines without : or which start with a # */
		ptr = strchr(line, ':');
		if (ptr == NULL || line[strspn(line, "\t ")] == '#')
			goto next;
		*ptr = '\0';

		/* "type" must match complete directory component(s). */
		if (!type || type_matches(line, type)) {
			char modname[strlen(line)+1];

			filename2modname(modname, line);
			if (fnmatch(wcard, modname, 0) == 0)
				printf("%s\n", line);
		}
	next:
		free(line);
	}

	free(wcard);
	return 0;
}

static char *strsep_skipspace(char **string, char *delim)
{
	if (!*string)
		return NULL;
	*string += strspn(*string, delim);
	return strsep(string, delim);
}

/* Recursion */
static int read_config(const char *filename,
		       const char *name,
		       int dump_only,
		       int removing,
		       struct module_options **options,
		       struct module_command **commands,
		       struct module_alias **alias,
		       struct module_blacklist **blacklist);

/* FIXME: Maybe should be extended to "alias a b [and|or c]...". --RR */
static int read_config_file(const char *filename,
			    const char *name,
			    int dump_only,
			    int removing,
			    struct module_options **options,
			    struct module_command **commands,
			    struct module_alias **aliases,
			    struct module_blacklist **blacklist)
{
	char *line;
	unsigned int linenum = 0;
	FILE *cfile;

	cfile = fopen(filename, "r");
	if (!cfile)
		return 0;

	while ((line = getline_wrapped(cfile, &linenum)) != NULL) {
		char *ptr = line;
		char *cmd, *modname;

		if (dump_only)
			printf("%s\n", line);

		cmd = strsep_skipspace(&ptr, "\t ");
		if (cmd == NULL || cmd[0] == '#' || cmd[0] == '\0')
			continue;

		if (strcmp(cmd, "alias") == 0) {
			char *wildcard
				= underscores(strsep_skipspace(&ptr, "\t "));
			char *realname
				= underscores(strsep_skipspace(&ptr, "\t "));

			if (!wildcard || !realname)
				grammar(cmd, filename, linenum);
			else if (fnmatch(wildcard,name,0) == 0)
				*aliases = add_alias(realname, *aliases);
		} else if (strcmp(cmd, "include") == 0) {
			struct module_alias *newalias = NULL;
			char *newfilename;

			newfilename = strsep_skipspace(&ptr, "\t ");
			if (!newfilename)
				grammar(cmd, filename, linenum);
			else {
				if (!read_config(newfilename, name,
						 dump_only, removing,
						 options, commands, &newalias,
						 blacklist))
					warn("Failed to open included"
					      " config file %s: %s\n",
					      newfilename, strerror(errno));

				/* Files included override aliases,
				   etc that was already set ... */
				if (newalias)
					*aliases = newalias;
			}
		} else if (strcmp(cmd, "options") == 0) {
			modname = strsep_skipspace(&ptr, "\t ");
			if (!modname || !ptr)
				grammar(cmd, filename, linenum);
			else {
				ptr += strspn(ptr, "\t ");
				*options = add_options(underscores(modname),
						       ptr, *options);
			}
		} else if (strcmp(cmd, "install") == 0) {
			modname = strsep_skipspace(&ptr, "\t ");
			if (!modname || !ptr)
				grammar(cmd, filename, linenum);
			else if (!removing) {
				ptr += strspn(ptr, "\t ");
				*commands = add_command(underscores(modname),
							ptr, *commands);
			}
		} else if (strcmp(cmd, "blacklist") == 0) {
			modname = strsep_skipspace(&ptr, "\t ");
			if (!modname)
				grammar(cmd, filename, linenum);
			else if (!removing) {
				*blacklist = add_blacklist(underscores(modname),
							*blacklist);
			}
		} else if (strcmp(cmd, "remove") == 0) {
			modname = strsep_skipspace(&ptr, "\t ");
			if (!modname || !ptr)
				grammar(cmd, filename, linenum);
			else if (removing) {
				ptr += strspn(ptr, "\t ");
				*commands = add_command(underscores(modname),
							ptr, *commands);
			}
		} else
			grammar(cmd, filename, linenum);

		free(line);
	}
	fclose(cfile);
	return 1;
}

/* Simple format, ignore lines starting with #, one command per line.
   Returns true or false. */
static int read_config(const char *filename,
		       const char *name,
		       int dump_only,
		       int removing,
		       struct module_options **options,
		       struct module_command **commands,
		       struct module_alias **aliases,
		       struct module_blacklist **blacklist)
{
	DIR *dir;
	int ret = 0;

	/* Reiser4 has file/directory duality: treat it as both. */
	dir = opendir(filename);
	if (dir) {
		struct dirent *i;
		while ((i = readdir(dir)) != NULL) {
			if (!streq(i->d_name,".") && !streq(i->d_name,"..")) {
				char sub[strlen(filename) + 1
					 + strlen(i->d_name) + 1];

				sprintf(sub, "%s/%s", filename, i->d_name);
				if (!read_config(sub, name,
						 dump_only, removing, options,
						 commands, aliases, blacklist))
					warn("Failed to open"
					     " config file %s: %s\n",
					     sub, strerror(errno));
			}
		}
		closedir(dir);
		ret = 1;
	}

	if (read_config_file(filename, name, dump_only, removing,
			     options, commands, aliases, blacklist))
		ret = 1;

	return ret;
}

static const char *default_configs[] = 
{
	"/etc/modprobe.conf",
	"/etc/modprobe.d",
};

static void read_toplevel_config(const char *filename,
				 const char *name,
				 int dump_only,
				 int removing,
				 struct module_options **options,
				 struct module_command **commands,
				 struct module_alias **aliases,
				 struct module_blacklist **blacklist)
{
	unsigned int i;

	if (filename) {
		if (!read_config(filename, name, dump_only, removing,
				 options, commands, aliases, blacklist))
			fatal("Failed to open config file %s: %s\n",
			      filename, strerror(errno));
		return;
	}

	/* Try defaults. */
	for (i = 0; i < ARRAY_SIZE(default_configs); i++) {
		if (read_config(default_configs[i], name, dump_only, removing,
				options, commands, aliases, blacklist))
			return;
	}
}

static void add_to_env_var(const char *option)
{
	const char *oldenv;

	if ((oldenv = getenv("MODPROBE_OPTIONS")) != NULL) {
		char *newenv;
		asprintf(&newenv, "%s %s", oldenv, option);
		setenv("MODPROBE_OPTIONS", newenv, 1);
	} else
		setenv("MODPROBE_OPTIONS", option, 1);
}

/* Prepend options from environment. */
static char **merge_args(char *args, char *argv[], int *argc)
{
	char *arg, *argstring;
	char **newargs = NULL;
	unsigned int i, num_env = 0;

	if (!args)
		return argv;

	argstring = NOFAIL(strdup(args));
	for (arg = strtok(argstring, " "); arg; arg = strtok(NULL, " ")) {
		num_env++;
		newargs = NOFAIL(realloc(newargs,
					 sizeof(newargs[0])
					 * (num_env + *argc + 1)));
		newargs[num_env] = arg;
	}

	/* Append commandline args */
	newargs[0] = argv[0];
	for (i = 1; i <= *argc; i++)
		newargs[num_env+i] = argv[i];

	*argc += num_env;
	return newargs;
}

static char *gather_options(char *argv[])
{
	char *optstring = NOFAIL(strdup(""));

	/* Rest is module options */
	while (*argv) {
		/* Quote value if it contains spaces. */
		unsigned int eq = strcspn(*argv, "=");

		if (strchr(*argv+eq, ' ') && !strchr(*argv, '"')) {
			char quoted[strlen(*argv) + 3];
			(*argv)[eq] = '\0';
			sprintf(quoted, "%s=\"%s\"", *argv, *argv+eq+1);
			optstring = append_option(optstring, quoted);
		} else
			optstring = append_option(optstring, *argv);
		argv++;
	}
	return optstring;
}

static void handle_module(const char *modname,
			  struct list_head *todo_list,
			  const char *newname,
			  int remove,
			  char *options,
			  int first_time,
			  errfn_t error,
			  int dry_run,
			  int verbose,
			  struct module_options *modoptions,
			  struct module_command *commands,
			  int ignore_commands,
			  int ignore_proc,
			  int strip_vermagic,
			  int strip_modversion,
			  int unknown_silent,
			  const char *cmdline_opts)
{
	if (list_empty(todo_list)) {
		const char *command;

		/* The dependencies have to be real modules, but
		   handle case where the first is completely bogus. */
		command = find_command(modname, commands);
		if (command && !ignore_commands) {
			do_command(modname, command, verbose, dry_run, error,
				   remove ? "remove":"install", cmdline_opts);
			return;
		}

		if (unknown_silent)
			exit(1);
		error("Module %s not found.\n", modname);
		return;
	}

	if (remove)
		rmmod(todo_list, newname, first_time, error, dry_run, verbose,
		      commands, ignore_commands, 0, cmdline_opts);
	else
		insmod(todo_list, NOFAIL(strdup(options)), newname,
		       first_time, error, dry_run, verbose, modoptions,
		       commands, ignore_commands, ignore_proc, strip_vermagic,
		       strip_modversion, cmdline_opts);
}

static struct option options[] = { { "verbose", 0, NULL, 'v' },
				   { "version", 0, NULL, 'V' },
				   { "config", 1, NULL, 'C' },
				   { "name", 1, NULL, 'o' },
				   { "remove", 0, NULL, 'r' },
				   { "showconfig", 0, NULL, 'c' },
				   { "autoclean", 0, NULL, 'k' },
				   { "quiet", 0, NULL, 'q' },
				   { "show", 0, NULL, 'n' },
				   { "dry-run", 0, NULL, 'n' },
				   { "syslog", 0, NULL, 's' },
				   { "type", 1, NULL, 't' },
				   { "list", 0, NULL, 'l' },
				   { "all", 0, NULL, 'a' },
				   { "ignore-install", 0, NULL, 'i' },
				   { "ignore-remove", 0, NULL, 'i' },
				   { "force", 0, NULL, 'f' },
				   { "force-vermagic", 0, NULL, 1 },
				   { "force-modversion", 0, NULL, 2 },
				   { "set-version", 1, NULL, 'S' },
				   { "show-depends", 0, NULL, 'D' },
				   { "first-time", 0, NULL, 3 },
				   { "use-blacklist", 0, NULL, 'b' },
				   { NULL, 0, NULL, 0 } };

#define MODPROBE_DEVFSD_CONF "/etc/modprobe.devfs"

/* This is a horrible hack to allow devfsd, which calls modprobe with
   -C /etc/modules.conf or /etc/modules.devfs, to work.  FIXME. */
/* Modern devfsd or variants should use -q explicitly in 2.6. */
static int is_devfs_call(char *argv[])
{
	unsigned int i;

	/* Look for "/dev" arg */
	for (i = 1; argv[i]; i++) {
		if (strncmp(argv[i], "/dev/", 5) == 0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct utsname buf;
	struct stat statbuf;
	int opt;
	int dump_only = 0;
	int dry_run = 0;
	int remove = 0;
	int verbose = 0;
	int unknown_silent = 0;
	int list_only = 0;
	int all = 0;
	int ignore_commands = 0;
	int strip_vermagic = 0;
	int strip_modversion = 0;
	int ignore_proc = 0;
	int first_time = 0;
	int use_blacklist = 0;
	unsigned int i, num_modules;
	char *type = NULL;
	const char *config = NULL;
	char *dirname, *optstring;
	char *newname = NULL;
	char *aliasfilename, *symfilename;
	errfn_t error = fatal;

	/* Prepend options from environment. */
	argv = merge_args(getenv("MODPROBE_OPTIONS"), argv, &argc);

	/* --set-version overrides version, and disables backwards compat. */
	for (opt = 1; opt < argc; opt++)
		if (strncmp(argv[opt],"--set-version",strlen("--set-version"))
		    == 0)
			break;

	if (opt == argc)
		try_old_version("modprobe", argv);

	uname(&buf);
	while ((opt = getopt_long(argc, argv, "vVC:o:rknqQsclt:aifb", options, NULL)) != -1){
		switch (opt) {
		case 'v':
			add_to_env_var("-v");
			verbose = 1;
			break;
		case 'V':
			puts(PACKAGE " version " VERSION);
			exit(0);
		case 'S':
			strncpy(buf.release, optarg, sizeof(buf.release));
			buf.release[sizeof(buf.release)-1] = '\0';
			break;
		case 'C':
			if (is_devfs_call(argv)) {
				if (streq("/etc/modules.devfs", optarg)) {
					config = MODPROBE_DEVFSD_CONF;
					add_to_env_var("-C");
					add_to_env_var(config);
					/* Fall thru to -q */
				} else if (streq("/etc/modules.conf", optarg))
					/* Ignore config, fall thru to -q */
					;
				else {
					/* False alarm.  Treat as normal. */
					config = optarg;
					add_to_env_var("-C");
					add_to_env_var(config);
					break;
				}
			} else {
				config = optarg;
				add_to_env_var("-C");
				add_to_env_var(config);
				break;
			}
		case 'q':
			unknown_silent = 1;
			add_to_env_var("-q");
			break;
		case 'D':
			dry_run = 1;
			ignore_proc = 1;
			verbose = 1;
			add_to_env_var("-D");
			break;
		case 'o':
			newname = optarg;
			break;
		case 'r':
			remove = 1;
			break;
		case 'c':
			dump_only = 1;
			break;
		case 't':
			type = optarg;
			break;
		case 'l':
			list_only = 1;
			break;
		case 'a':
			all = 1;
			error = warn;
			break;
		case 'k':
			/* FIXME: This should actually do something */
			break;
		case 'n':
			dry_run = 1;
			break;
		case 's':
			add_to_env_var("-s");
			log = 1;
			break;
		case 'i':
			ignore_commands = 1;
			break;
		case 'f':
			strip_vermagic = 1;
			strip_modversion = 1;
			break;
		case 'b':
			use_blacklist = 1;
			break;
		case 1:
			strip_vermagic = 1;
			break;
		case 2:
			strip_modversion = 1;
			break;
		case 3:
			first_time = 1;
			break;
		default:
			print_usage(argv[0]);
		}
	}

	/* If stderr not open, go to syslog */
	if (log || fstat(STDERR_FILENO, &statbuf) != 0) {
		openlog("modprobe", LOG_CONS, LOG_DAEMON);
		log = 1;
	}

	if (argc < optind + 1 && !dump_only && !list_only && !remove)
		print_usage(argv[0]);

	dirname = NOFAIL(malloc(strlen(buf.release) + sizeof(MODULE_DIR) + 1));
	sprintf(dirname, "%s/%s", MODULE_DIR, buf.release);
	aliasfilename = NOFAIL(malloc(strlen(dirname)
				      + sizeof("/modules.alias")));
	sprintf(aliasfilename, "%s/modules.alias", dirname);
	symfilename = NOFAIL(malloc(strlen(dirname)
				    + sizeof("/modules.symbols")));
	sprintf(symfilename, "%s/modules.symbols", dirname);

	/* Old-style -t xxx wildcard?  Only with -l. */
	if (list_only) {
		if (optind+1 < argc)
			fatal("Can't have multiple wildcards\n");
		/* fprintf(stderr, "man find\n"); return 1; */
		return do_wildcard(dirname, type, argv[optind]?:"*");
	}
	if (type)
		fatal("-t only supported with -l");

	if (dump_only) {
		struct module_command *commands = NULL;
		struct module_options *modoptions = NULL;
		struct module_alias *aliases = NULL;
		struct module_blacklist *blacklist = NULL;

		read_toplevel_config(config, "", 1, 0,
			     &modoptions, &commands, &aliases, &blacklist);
		read_config(aliasfilename, "", 1, 0,&modoptions, &commands,
			    &aliases, &blacklist);
		read_config(symfilename, "", 1, 0, &modoptions, &commands,
			    &aliases, &blacklist);
		exit(0);
	}

	if (remove || all) {
		num_modules = argc - optind;
		optstring = NOFAIL(strdup(""));
	} else {
		num_modules = 1;
		optstring = gather_options(argv+optind+1);
	}

	/* num_modules is always 1 except for -r or -a. */
	for (i = 0; i < num_modules; i++) {
		struct module_command *commands = NULL;
		struct module_options *modoptions = NULL;
		struct module_alias *aliases = NULL;
		struct module_blacklist *blacklist = NULL;
		LIST_HEAD(list);
		char *modulearg = argv[optind + i];

		/* Convert name we are looking for */
		underscores(modulearg);

		/* Returns the resolved alias, options */
		read_toplevel_config(config, modulearg, 0,
		     remove, &modoptions, &commands, &aliases, &blacklist);

		/* No luck?  Try symbol names, if starts with symbol:. */
		if (!aliases
		    && strncmp(modulearg, "symbol:", strlen("symbol:")) == 0)
			read_config(symfilename, modulearg, 0,
			    remove, &modoptions, &commands,
			    	&aliases, &blacklist);

		if (!aliases) {
			/* We only use canned aliases as last resort. */
			read_depends(dirname, modulearg, &list);

			if (list_empty(&list)
			    && !find_command(modulearg, commands))
			{
				read_config(aliasfilename, modulearg, 0,
					    remove, &modoptions, &commands,
					    &aliases, &blacklist);
				aliases = apply_blacklist(aliases, blacklist);
			}
		}

		if (aliases) {
			errfn_t err = error;

			/* More than one alias?  Don't bail out on failure. */
			if (aliases->next)
				err = warn;
			while (aliases) {
				/* Add the options for this alias. */
				char *opts = NOFAIL(strdup(optstring));
				opts = add_extra_options(modulearg,
							 opts, modoptions);

				read_depends(dirname, aliases->module, &list);
				handle_module(aliases->module, &list, newname,
					      remove, opts, first_time, err,
					      dry_run, verbose, modoptions,
					      commands, ignore_commands,
					      ignore_proc, strip_vermagic,
					      strip_modversion,
					      unknown_silent,
					      optstring);

				aliases = aliases->next;
				INIT_LIST_HEAD(&list);
			}
		} else {
			if (use_blacklist
			    && find_blacklist(modulearg, blacklist))
				continue;

			handle_module(modulearg, &list, newname, remove,
				      optstring, first_time, error, dry_run,
				      verbose, modoptions, commands,
				      ignore_commands, ignore_proc,
				      strip_vermagic, strip_modversion,
				      unknown_silent, optstring);
		}
	}
	if (log)
		closelog();

	return 0;
}

/* Objective-C language support definitions for GDB, the GNU debugger.

   Copyright (C) 1992, 2005, 2007, 2008 Free Software Foundation, Inc.

   Contributed by Apple Computer, Inc.

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

#if !defined(OBJC_LANG_H)
#define OBJC_LANG_H

struct stoken;

struct value;
struct block;

extern int objc_parse (void);		/* Defined in c-exp.y */

extern void objc_error (char *);	/* Defined in c-exp.y */

extern CORE_ADDR lookup_objc_class     (char *classname);
extern CORE_ADDR lookup_child_selector (char *methodname);

extern char *objc_demangle (const char *mangled, int options);

extern int find_objc_msgcall (CORE_ADDR pc, CORE_ADDR *new_pc);

extern char *parse_selector (char *method, char **selector);

extern char *parse_method (char *method, char *type, 
			   char **class, char **category, 
			   char **selector);

extern char *find_imps (struct symtab *symtab, struct block *block,
			char *method, struct symbol **syms, 
			unsigned int *nsym, unsigned int *ndebug);

extern struct value *value_nsstring (char *ptr, int len);

/* for parsing Objective C */
extern void start_msglist (void);
extern void add_msglist (struct stoken *str, int addcolon);
extern int end_msglist (void);

struct symbol *lookup_struct_typedef (char *name, struct block *block,
				      int noerr);

#endif

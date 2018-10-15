/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2003 Michel Arboi and Renaud Deraison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Renaud Deraison and Michel Arboi
 * give permission to link the code of this program with any
 * version of the OpenSSL library which is distributed under a
 * license identical to that listed in the included COPYING.OpenSSL
 * file, and distribute linked combinations including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 *
 */
#ifndef NASL_LEX_CTXT_H_INCLUDED
#define NASL_LEX_CTXT_H_INCLUDED


typedef struct struct_lex_ctxt {
  struct struct_lex_ctxt	*up_ctxt;
  tree_cell	*ret_val;	/* return value or exit flag */
  unsigned	fct_ctxt :1;	/* This is a function context */
  unsigned	break_flag : 1;	/* Break from loop */
  unsigned	cont_flag : 1;	/* Next iteration in loop */
  unsigned	authenticated:1;/* Authenticated script */
  unsigned 	always_authenticated:1;
  struct arglist	*script_infos;
  int			recv_timeout;
  /* Named variables hash set + anonymous variables array */
  nasl_array	ctx_vars;
  /* Functions hash set */
  nasl_func	*functions[FUNC_NAME_HASH];
} lex_ctxt;

#define NASL_COMPAT_LEX_CTXT	"NASL compat lex context"

lex_ctxt	*init_empty_lex_ctxt(void);
void		free_lex_ctxt(lex_ctxt*);
lex_ctxt	*get_top_level_ctxt(lex_ctxt*);

void		dump_ctxt(lex_ctxt*);

nasl_func	*get_func_ref_by_name(lex_ctxt*, const char*);
tree_cell	*decl_nasl_func(lex_ctxt*, tree_cell*);
nasl_func	*insert_nasl_func(lex_ctxt*, const char*, tree_cell*);
tree_cell*	nasl_func_call(lex_ctxt*, const nasl_func*, tree_cell*);

tree_cell	*get_variable_by_name(lex_ctxt*, const char*);
tree_cell	*get_array_elem(lex_ctxt*, const char* /*array name*/, tree_cell*);
anon_nasl_var*	add_numbered_var_to_ctxt(lex_ctxt*, int, tree_cell*);
named_nasl_var*	add_named_var_to_ctxt(lex_ctxt*, const char*, tree_cell*);
tree_cell*	nasl_read_var_ref(lex_ctxt*, tree_cell*);
tree_cell* 	nasl_incr_variable(lex_ctxt*, tree_cell* , int , int );
tree_cell*	nasl_return(lex_ctxt*, tree_cell*);

tree_cell*  	decl_local_variables(lex_ctxt*, tree_cell*);
tree_cell*  	decl_global_variables(lex_ctxt*, tree_cell*);

tree_cell* 	cell2atom(lex_ctxt*, tree_cell*);


int		get_int_var_by_num(lex_ctxt*, int, int);
char*		get_str_var_by_num(lex_ctxt*, int);
int		get_int_var_by_name(lex_ctxt*, const char*, int);
int		get_int_local_var_by_name(lex_ctxt*, const char*, int);
char*		get_str_var_by_name(lex_ctxt*, const char*);
char*		get_str_local_var_by_name(lex_ctxt*, const char*);

int 		get_var_size_by_name(lex_ctxt*, const char*);
int 		get_local_var_size_by_name(lex_ctxt*, const char*);
int 		get_local_var_type_by_name(lex_ctxt *, const char *);


int		get_var_size_by_num(lex_ctxt*, int);
int	 	get_var_type_by_num(lex_ctxt*, int);
#endif

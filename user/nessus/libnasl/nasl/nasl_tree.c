/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2004 Tenable Network Security
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
 */
#include <includes.h>
#include "nasl_regex.h"

#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"

#include "nasl_debug.h"

tree_cell*
alloc_tree_cell(int lnb, char * s)
{
  tree_cell	*p = malloc(sizeof(tree_cell));
  int		i;

  if (p == NULL)
    {
      perror("malloc");
      abort();
    }
  p->type = 0;
  p->size = 0;
  p->line_nb = lnb;
  p->x.str_val = s;
  p->ref_count = 1;
  for (i = 0; i < 4;i ++)
    p->link[i] = NULL;
  return p;
}

tree_cell*
alloc_typed_cell(int typ)
{
  tree_cell	*c = alloc_tree_cell(0, NULL);
  c->type = typ;
  return c;
}
  
tree_cell*
alloc_RE_cell(int lnb, int t, tree_cell *l, char *re_str)
{
  regex_t	*re = emalloc(sizeof(regex_t));
  int	e;

  tree_cell *c = alloc_tree_cell(lnb, NULL);
  c->type = t;			/* We could check the type... */
  c->link[0] = l;
  c->link[1] = FAKE_CELL;
  e = nasl_regcomp(re, re_str, REG_EXTENDED|REG_NOSUB|REG_ICASE);
  if (! e)
    c->x.ref_val = re;
  else
    {
      nasl_perror(NULL, "Line %d: Cannot compile regex: %s (error = %d)\n",
		  lnb, re_str, e);
      efree(&re);
    }
  free(re_str);
  return c;
}

tree_cell*
alloc_expr_cell(int lnb, int t, tree_cell *l, tree_cell *r)
{
  tree_cell *c = alloc_tree_cell(lnb, NULL);
  c->type = t;
  c->link[0] = l;
  c->link[1] = r;
  return c;
}

tree_cell*
dup_cell(const tree_cell* tc)
{
  tree_cell	*r;
  int		i;

  if (tc == NULL)
    return NULL;
  else if (tc == FAKE_CELL)
    return FAKE_CELL;

  r  = alloc_tree_cell(tc->line_nb, NULL);
  r->type = tc->type;
  r->size = tc->size;
  switch (tc->type)
    {
    case CONST_STR:
    case CONST_DATA:
      r->x.str_val = emalloc(tc->size);
      memcpy(r->x.str_val, tc->x.str_val, tc->size);
      break;
    default:
      r->x = tc->x;
      break;
    }

  for (i = 0; i < 4; i ++)
    r->link[i] = dup_cell(tc->link[i]);
  return r;
}

static void
free_tree(tree_cell *c)
{
  int			i;
  nasl_array		*a;

  if (c == NULL || c == FAKE_CELL)
    return;
#if 0
  nasl_dump_tree(c);
#endif
  for (i = 0; i < 4; i ++)
    if (c->link[i] != NULL)
      deref_cell(c->link[i]);
  if (c->x.str_val != NULL)
    switch(c->type)
      {
      case CONST_STR:
      case CONST_DATA:
#ifdef SCRATCH_FREED_MEMORY
	if (c->size > 0)
	  memset(c->x.str_val, 0xFF, c->size);
#endif
	efree(&c->x.str_val);
	break;

      case CONST_REGEX:
      case COMP_RE_MATCH:
      case COMP_RE_NOMATCH:
	if (c->x.ref_val != NULL)
	  {
	    nasl_regfree(c->x.ref_val);
	    efree(&c->x.ref_val);
	  }
	break;

      case DYN_ARRAY:
	a = c->x.ref_val;
	if (a != NULL)
	  {
	    free_array(a);
	    efree(&c->x.ref_val);
	  }
	break;

      case NODE_FUN_DEF:
      case NODE_FUN_CALL:
      case NODE_VAR:
      case NODE_DECL:
      case NODE_ARG:
      case NODE_ARRAY_EL:
      case NODE_FOREACH:
	efree(&c->x.str_val);
	break;
      }
#ifdef SCRATCH_FREED_MEMORY
  memset(c, 0xFF, sizeof(*c));
#endif
  efree(&c);
}

void
ref_cell(tree_cell* c)
{
  if (c == NULL || c == FAKE_CELL)
    return;
  c->ref_count ++;
  if (c->ref_count < 0)
    {
      nasl_perror(NULL, "ref_cell: ref count is negative!\n");
      nasl_dump_tree(c);
      abort();
    }
}

void
deref_cell(tree_cell* c)
{
  if (c == NULL || c == FAKE_CELL)
    return;
  if (-- c->ref_count <= 0)
    free_tree(c);
}

/* Debug */

static char * node_names[] = { 
  "NODE_EMPTY",
  "NODE_IF_ELSE",
  "NODE_INSTR_L",
  "NODE_FOR",
  "NODE_WHILE",
  "NODE_FOREACH",
  "NODE_REPEAT_UNTIL",
  "NODE_REPEATED",
  "NODE_FUN_DEF",
  "NODE_FUN_CALL",
  "NODE_DECL",
  "NODE_ARG",
  "NODE_RETURN",
  "NODE_BREAK",
  "NODE_CONTINUE",

  "NODE_ARRAY_EL",
  "NODE_AFF",
  "NODE_VAR",
  "NODE_LOCAL",
  "NODE_GLOBAL",
  "NODE_PLUS_EQ",
  "NODE_MINUS_EQ",
  "NODE_MULT_EQ",
  "NODE_DIV_EQ",
  "NODE_MODULO_EQ",

  "NODE_L_SHIFT_EQ",
  "NODE_R_SHIFT_EQ",
  "NODE_R_USHIFT_EQ",
  "EXPR_AND",
  "EXPR_OR",
  "EXPR_NOT",

  "EXPR_PLUS",
  "EXPR_MINUS",
  "EXPR_U_MINUS",
  "EXPR_MULT",
  "EXPR_DIV",
  "EXPR_MODULO",
  "EXPR_EXPO",

  "EXPR_BIT_AND",
  "EXPR_BIT_OR",
  "EXPR_BIT_XOR",
  "EXPR_BIT_NOT",
  "EXPR_INCR",
  "EXPR_DECR",
  "EXPR_L_SHIFT",
  "EXPR_R_SHIFT",
  "EXPR_R_USHIFT",

  "COMP_MATCH",
  "COMP_NOMATCH",
  "COMP_RE_MATCH",
  "COMP_RE_NOMATCH",

  "COMP_LT",
  "COMP_LE",
  "COMP_EQ",
  "COMP_NE",
  "COMP_GT",
  "COMP_GE",
  "CONST_INT",
  "CONST_STR",
  "CONST_DATA",
  "CONST_REGEX",

  "ARRAY_ELEM",

  "REF_VAR",
  "REF_ARRAY",
  "DYN_ARRAY"
};

static void
prefix(int n, int i)
{
  int	j;
  for (j = 0; j < n; j ++)
    putchar(' ');
  if (i <= 0)
    fputs("   ", stdout);
  else
    printf("%d: ", i);
}

char*
dump_cell_val(const tree_cell* c)
{
  static char	txt[80];

  if (c == NULL)
    return "NULL";
  else if (c == FAKE_CELL)
     return "FAKE";
  else
    switch(c->type)
      {
      case CONST_INT:
	snprintf(txt, sizeof(txt), "%d", c->x.i_val);
	break;
      case CONST_STR:
      case CONST_DATA:		/* Beurk */
	if (c->size >= sizeof(txt) + 2)
	  {
	    snprintf(txt, sizeof(txt), "\"%s", c->x.str_val);
	    strcpy(txt + (sizeof(txt) - 5), "...\"");
	  }
	else
	  snprintf(txt, sizeof(txt), "\"%s\"", c->x.str_val);
	break;
      default:
	snprintf(txt, sizeof(txt), "???? (%s)", nasl_type_name(c->type));
	break;
      }
  return txt;
}

static void
dump_tree(const tree_cell* c, int n, int idx)
{
  int	i;

  if (c == NULL)
    return;

  prefix(n, idx);

  if (c == FAKE_CELL)
    {
      puts("* FAKE *");
      return;
    }

  if (c->line_nb > 0)
    printf("L%d: ", c->line_nb);

#if 0
  if ((int) c < 0x1000)
    {
      printf("* INVALID PTR 0x%x *\n", (int) c);
      return;
    }
#endif
  if (c->type < 0 || c->type >= sizeof(node_names) / sizeof(node_names[0]))
    printf("* UNKNOWN %d (0x%x)*\n", c->type, c->type);
  else
    printf("%s (%d)\n", node_names[c->type],c->type);


  prefix(n, idx);
  printf("Ref_count=%d", c->ref_count);
  if (c->size > 0)
    {
      /*prefix(n, idx);*/
      printf("\tSize=%d (0x%x)", c->size, c->size);
    }
  putchar('\n');

  switch(c->type)
    {
    case CONST_INT:
      prefix(n, 0);
      printf("Val=%d\n", c->x.i_val);
      break;

    case CONST_STR:
    case CONST_DATA:
    case NODE_VAR:
    case NODE_FUN_DEF:
    case NODE_FUN_CALL:
    case NODE_DECL:
    case NODE_ARG:
    case NODE_ARRAY_EL:
    case ARRAY_ELEM:
      prefix(n, 0);
      if (c->x.str_val == NULL)
	printf("Val=(null)\n");
      else
	printf("Val=\"%s\"\n", c->x.str_val);
      break;
    case REF_VAR:
      prefix(n, 0);
      if (c->x.ref_val == NULL)
	printf("Ref=(null)\n");
      else
	{
	  named_nasl_var	*v = c->x.ref_val;
	  printf("Ref=(type=%d, name=%s, value=%s)\n",
		 v->u.var_type, v->var_name != NULL ? v->var_name : "(null)",
		 var2str(&v->u));
	}
      break;

    case REF_ARRAY:
    case DYN_ARRAY:
      break;
    }

  for (i = 0; i < 4; i ++)
    {
      dump_tree(c->link[i], n+3, i+1);
    }
}

const char*
nasl_type_name(int t)
{
  static char	txt4[4][32];	/*  This function may be called 4 times in the same expression */
  static int	i = 0;
  char	*txt;

  if (++ i > 4) i = 0;
  txt = txt4[i];

  if (t >= 0 || t < sizeof(node_names) / sizeof(node_names[0]))
    snprintf(txt, 32, "%s (%d)", node_names[t], t);
  else
    snprintf(txt, 32, "*UNKNOWN* (%d)", t);
  return txt;
}
 

void
nasl_dump_tree(const tree_cell* c)
{
  printf("^^^^ %08x ^^^^^\n", (int) c);
  if (c == NULL)
    puts("NULL CELL");
  else if (c == FAKE_CELL)
    puts ("FAKE CELL");
  else
    dump_tree(c, 0, 0);
  printf("vvvvvvvvvvvvvvvvvv\n");
}

char*
get_line_nb(const tree_cell* c)
{
  static char txt[32];
  if (c == NULL || c == FAKE_CELL || c->line_nb <= 0)
    return "";
  snprintf(txt, sizeof(txt), " at or near line %d ", c->line_nb);
  return txt;
}


int
nasl_is_leaf(const tree_cell* pc)
{
  if (pc == NULL || pc == FAKE_CELL)
    return 1;
  switch(pc->type)
    {
    case CONST_INT:
    case CONST_STR:
    case CONST_DATA:
    case REF_ARRAY:
    case DYN_ARRAY:
      return 1;
    default:
      return 0;
    }
  /*NOTREACHED*/
}

int
cell_type(const tree_cell* c)
{
  if (c == NULL || c== FAKE_CELL)
    return 0;
  else
    return c->type;
}




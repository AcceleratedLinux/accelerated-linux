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

#include "nasl.h"
#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"
#include "preparse.h"
#include "nasl_server.h"

#include "nasl_debug.h"
#include "strutils.h"
#include "nasl_init.h"

#ifndef NASL_DEBUG
#define NASL_DEBUG 0
#endif

extern int naslparse( naslctxt * );


int
check_authenticated( lex_ctxt * lexic )
{
 if ( lexic->authenticated == 1 ) return 0;
 else {
	nasl_perror(lexic, "A non-authenticated script attempted to use an authenticated function - returning NULL\n");
	return -1;
      }
}


static int
cell2bool(lex_ctxt* lexic, tree_cell* c)
{
  tree_cell	*c2;
  int		flag;


  if (c == NULL || c == FAKE_CELL)
    return 0;
  
  switch (c->type)
    {
    case CONST_INT:
      return c->x.i_val != 0;
    case CONST_STR:
    case CONST_DATA:
      if (c->size == 0)
	return 0;
      if(c->x.str_val[0] == '0' && c->size == 1)
	{
	  /* 
	   * This gives the same semantics as Perl ("0" is false), 
	   * but I do not agree with it. 
	   * This piece of code is here from the begining of NASL2; it 
	   * probably fixed some compatibility issue with old 
	   * quick & dirty scripts. 
	   * I added this warning to check if we can switch to a
	   * simpler behaviour (empty string = false, not empty = true)
	   */
	  nasl_perror(lexic, "cell2boll: string '0' is FALSE\n");
	  return 0;
	}
     return 1;

    case REF_ARRAY:
    case DYN_ARRAY:
      nasl_perror(lexic, "cell2bool: converting array to boolean does not make sense!\n");
      return 1;

    default:
      c2 = nasl_exec(lexic, c);
      flag = cell2bool(lexic, c2);
      deref_cell(c2);
      return flag;
    }
}

static int
cvt_bool(lex_ctxt* lexic, tree_cell* c)
{
  int	flag;

#if 0
  nasl_dump_tree(c);
#endif
  flag = cell2bool(lexic, c);
  /* free(c); free_tree(c); */
  return flag;
}


static int
cell2int3(lex_ctxt* lexic, tree_cell* c, int warn)
{
  tree_cell	*c2 = NULL;
  int		x;
  char		*p = NULL;

  if (c == NULL || c == FAKE_CELL) /*  Do not SEGV on undefined variables */
    return 0;

  switch(c->type)
    {
    case CONST_INT:
      return c->x.i_val;

    case CONST_STR:
    case CONST_DATA:
      x = strtol(c->x.str_val, &p, 0);
      if (*p != '\0' && warn)
      if (warn)
	nasl_perror(lexic, "Converting a non numeric string to integer does not make sense in this context");
      return x;

    default:
      c2 = nasl_exec(lexic, c);
      x = cell2int3(lexic, c2, warn);
      deref_cell(c2);
      return x;
    }
}

static int
cell2int(lex_ctxt* lexic, tree_cell* c)
{
  return cell2int3(lexic, c, 0);
}

static int
cell2intW(lex_ctxt* lexic, tree_cell* c)
{
  return cell2int3(lexic, c, 1);
}

static tree_cell*
int2cell(int x)
{
  tree_cell	*c = alloc_expr_cell(0, CONST_INT, NULL, NULL);
  c->x.i_val = x;
  return c;
}

static tree_cell*
bool2cell(int x)
{
  return int2cell(x != 0);
}

static char*
cell2str(lex_ctxt* lexic, tree_cell* c)
{
  char	* p;
  tree_cell	*c2;
  nasl_array	*a;

  if (c == NULL || c == FAKE_CELL)
    {
#if NASL_DEBUG > 0
      nasl_perror(lexic, "Cannot convert NULL or FAKE cell to string\n");
#endif
      return NULL;
    }
      
  switch(c->type)
    {
    case CONST_INT:
      p = malloc(16);
      if (p != NULL)
	snprintf(p, 16, "%d", c->x.i_val);
      return p;
      
    case CONST_STR:
    case CONST_DATA:
      if ( c->x.str_val == NULL)
	p = estrdup("");
      else
	p = nasl_strndup(c->x.str_val, c->size);
      return p;

    case REF_ARRAY:
    case DYN_ARRAY:
      a = c->x.ref_val;
      p = (char*)array2str(a);
      return estrdup(p);

    default:
      c2 = nasl_exec(lexic, c);
      p = cell2str(lexic, c2);
      deref_cell(c2);
      if (p == NULL)
	p = estrdup("");
      return p;
    }
}


#ifdef DEPRECAT_CODE
char*
cell2str_and_size(lex_ctxt* lexic, tree_cell* c, int * sz)
{
  char	* p;
  tree_cell	*c2;

  if (c == NULL || c == FAKE_CELL)
    {
#if NASL_DEBUG > 0
      nasl_perror(lexic, "Cannot convert NULL or FAKE cell to string\n");
#endif
      return NULL;
    }
      
  switch(c->type)
    {
    case CONST_INT:
      p = malloc(16);
      if ( p == NULL ) 
        return NULL;
      snprintf(p, 16, "%d", c->x.i_val);
      if(sz != NULL)*sz = strlen(p);
      return p;
      
    case CONST_STR:
    case CONST_DATA:
      if ( c->x.str_val == NULL)
	p = estrdup("");
      else
	p = nasl_strndup(c->x.str_val, c->size);
      if(sz != NULL)*sz = c->size;
      return p;
      
    default:
      c2 = nasl_exec(lexic, c);
      p = cell2str_and_size(lexic, c2, sz);
      deref_cell(c2);
      if (p == NULL)
      	{
	p = estrdup("");
      	if(sz != NULL)*sz = 0;
	}
      return p;
    }
}
#endif

/* cell2atom returns a 'referenced' cell */	
tree_cell*
cell2atom(lex_ctxt* lexic, tree_cell* c1)
{
  tree_cell	*c2 = NULL, *ret = NULL;
  if (c1 == NULL || c1 == FAKE_CELL)
    return c1;

  switch(c1->type)
    {
    case CONST_INT:
    case CONST_STR:
    case CONST_DATA:
    case REF_ARRAY:
    case DYN_ARRAY:
      ref_cell(c1);
      return c1;
    default:
      c2 = nasl_exec(lexic, c1);
      ret = cell2atom(lexic, c2);
      deref_cell(c2);
      return ret;
    }
}

int
cell_cmp(lex_ctxt* lexic, tree_cell* c1, tree_cell* c2)
{
  int		flag, x1, x2, typ, typ1, typ2;
  char		*s1, *s2;
  int		len_s1, len_s2, len_min;


#if NASL_DEBUG >= 0
  if (c1 == NULL || c1 == FAKE_CELL)
    nasl_perror(lexic, "cell_cmp: c1 == NULL !\n");
  if (c2 == NULL || c2 == FAKE_CELL)
    nasl_perror(lexic, "cell_cmp: c2 == NULL !\n");
#endif

  /* We first convert the cell to atomic types */
  c1 = cell2atom(lexic, c1);
  c2 = cell2atom(lexic, c2);

  /*
   * Comparing anything to something else which is entirely different 
   * may lead to unpredictable results.
   * Here are the rules:
   * 1. No problem with same types, although we do not compare arrays yet
   * 2. No problem with CONST_DATA / CONST_STR
   * 3. When an integer is compared to a string, the integer is converted
   * 4. When NULL is compared to an integer, it is converted to 0
   * 5. When NULL is compared to a string, it is converted to ""
   * 6. NULL is "smaller" than anything else (i.e. an array)
   * Anything else is an error
   */
  typ1 = cell_type(c1);
  typ2 = cell_type(c2);

  if (typ1 == 0 && typ2 == 0)	/* Two NULL */
    {
      deref_cell(c1); deref_cell(c2); 
      return 0;
    }

  if (typ1 == typ2)		/* Same type, no problem */
    typ = typ1;
  else if ((typ1 == CONST_DATA || typ1 == CONST_STR) &&
	   (typ2 == CONST_DATA || typ2 == CONST_STR))
    typ = CONST_DATA;		/* Same type in fact (string) */
  /* We convert an integer into a string before compare */
  else if ((typ1 == CONST_INT && (typ2 == CONST_DATA || typ2 == CONST_STR)) ||
	   (typ2 == CONST_INT && (typ1 == CONST_DATA || typ1 == CONST_STR)) )
    {
#if NASL_DEBUG > 0
      nasl_perror(lexic, "cell_cmp: converting integer to string\n");
#endif
      typ = CONST_DATA;
    }
  else if (typ1 == 0)		/* 1st argument is null */
    if (typ2 == CONST_INT || typ2 == CONST_DATA || typ2 == CONST_STR)
      typ = typ2;		/* We convert it to 0 or "" */
    else
      {
	deref_cell(c1); deref_cell(c2); 
	return -1;		/* NULL is smaller than anything else */
      }
  else if (typ2 == 0)		/* 2nd argument is null */
    if (typ1 == CONST_INT || typ1 == CONST_DATA || typ1 == CONST_STR)
      typ = typ1;		/* We convert it to 0 or "" */
    else
      {
	deref_cell(c1); deref_cell(c2); 
	return 1;		/* Anything else is greater than NULL  */
      }
  else
    {
      nasl_perror(lexic, "cell_cmp: comparing %s and %s does not make sense\n", nasl_type_name(typ1), nasl_type_name(typ2));
      deref_cell(c1); deref_cell(c2); 
      return 0;
    } 

  switch (typ)
    {
    case CONST_INT:
      x1 = cell2int(lexic, c1);
      x2 = cell2int(lexic, c2);
      deref_cell(c1); deref_cell(c2); 
      return x1 - x2;

    case CONST_STR:
    case CONST_DATA:
      s1 = cell2str(lexic, c1);
      if (typ1 == CONST_STR || typ1 == CONST_DATA)
	len_s1 = c1->size;
      else if (s1 == NULL)
	len_s1 = 0;
      else
	len_s1 = strlen(s1);

      s2 = cell2str(lexic, c2);
      if (typ2 == CONST_STR || typ2 == CONST_DATA)
	len_s2 = c2->size;
      else if (s2 == NULL)
	len_s2 = 0;
      else
	len_s2 = strlen(s2);
       		  
      len_min = len_s1 < len_s2 ? len_s1 : len_s2;
      flag = 0;

      if (len_min > 0)
	flag = memcmp(s1, s2, len_min);
      if (flag == 0)
	flag = len_s1 - len_s2;
       	
      efree(&s1); efree(&s2); 
      deref_cell(c1); deref_cell(c2); 
      return flag;

    case REF_ARRAY:
    case DYN_ARRAY:
      fprintf(stderr, "cell_cmp: cannot compare arrays yet\n");
      deref_cell(c1); deref_cell(c2); 
      return 0;

    default:
      fprintf(stderr, "cell_cmp: don't known how to compare %s and %s\n",
	      nasl_type_name(typ1), nasl_type_name(typ2));
      deref_cell(c1); deref_cell(c2); 
      return 0;
    }
}

FILE*	nasl_trace_fp = NULL;

lex_ctxt* truc = NULL;

static void
nasl_dump_expr(FILE* fp, const tree_cell* c)
{
  if (c == NULL)
    fprintf(fp, "NULL");
  else if  (c == FAKE_CELL)
    fprintf(fp, "FAKE");
  else
    switch(c->type)
      {
      case NODE_VAR:
	fprintf(fp, "%s", c->x.str_val);
	break;
      case EXPR_AND:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") && (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_OR:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") || (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_NOT:
	fprintf(fp, "! (");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ")");
	break;
      case EXPR_PLUS:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") + (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_MINUS:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") - (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case EXPR_INCR:
	if (c->link[0] == NULL)
	  {
	    fprintf(fp, " ++");
	    nasl_dump_expr(fp, c->link[1]);
	  }
	else
	  {
	    nasl_dump_expr(fp, c->link[0]);
	    fprintf(fp, "++ ");
	  }
	break;
      case EXPR_DECR:
	if (c->link[0] == NULL)
	  {
	    fprintf(fp, " --");
	    nasl_dump_expr(fp, c->link[1]);
	  }
	else
	  {
	    nasl_dump_expr(fp, c->link[0]);
	    fprintf(fp, "-- ");
	  }
	break;
	    
      case EXPR_EXPO:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") ** (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case EXPR_U_MINUS:
	fprintf(fp, " - (");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ")");
	break;

      case EXPR_MULT:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") * (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_DIV:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") / (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_MODULO:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") %% (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_BIT_AND:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") & (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_BIT_OR:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") | (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_BIT_XOR:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") ^ (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_BIT_NOT:
	fprintf(fp, "~ (");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ")");
	break;
      case EXPR_L_SHIFT:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") << (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_R_SHIFT:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") >> (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case EXPR_R_USHIFT:
	fprintf(fp, "(");
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, ") >>> (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;
      case COMP_MATCH:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " >< ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case COMP_NOMATCH:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " >!< ");
	nasl_dump_expr(fp, c->link[1]);
	break;

      case COMP_RE_MATCH:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " =~ ");
	nasl_dump_expr(fp, c->link[1]);
	break;

      case COMP_RE_NOMATCH:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " !~ ");
	nasl_dump_expr(fp, c->link[1]);
	break;

      case COMP_LT:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " < ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case COMP_LE:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " <= ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case COMP_GT:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " > ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case COMP_GE:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " >= ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case COMP_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " == ");
	nasl_dump_expr(fp, c->link[1]);
	break;
      case CONST_INT:
	fprintf(fp, "%d", c->x.i_val);
	break;
      case CONST_STR:
      case CONST_DATA:
	fprintf(fp, "\"%s\"", c->x.str_val);
	break;

      case NODE_ARRAY_EL:
	fprintf(fp, "%s[", c->x.str_val);
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "]");
	break; 

      case NODE_FUN_CALL:
	fprintf(fp, "%s(...)", c->x.str_val);
	break;
	
      case NODE_AFF:
	nasl_dump_expr(fp, c->link[0]);
	putc('=', fp);
	nasl_dump_expr(fp, c->link[1]);
	break;

      case NODE_PLUS_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "+= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_MINUS_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "-= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_MULT_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "*= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_DIV_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "/= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_MODULO_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, "%%= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_L_SHIFT_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " <<= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_R_SHIFT_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " >>= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      case NODE_R_USHIFT_EQ:
	nasl_dump_expr(fp, c->link[0]);
	fprintf(fp, " >>>= (");
	nasl_dump_expr(fp, c->link[1]);
	fprintf(fp, ")");
	break;

      default:
	fprintf(fp, "*%d*", c->type);
	break;      
      }
}

static void
nasl_short_dump(FILE* fp, const tree_cell* c)
{
  if (c == NULL || c == FAKE_CELL)
    return;

  switch (c->type)
    {
    case NODE_IF_ELSE:
      fprintf(fp, "NASL:%04d> if (", c->line_nb);
      nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, ") { ... }");
      if (c->link[2] != NULL)  fprintf(fp, " else { ... }");
      putc('\n', fp);
      break;

    case NODE_FOR:
      fprintf(fp, "NASL:%04d> for (", c->line_nb); nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, "; "); nasl_dump_expr(fp, c->link[1]);
      fprintf(fp, "; "); nasl_dump_expr(fp, c->link[2]);
      fprintf(fp, ") { ... }\n");
      break;

    case NODE_WHILE:
      fprintf(fp, "NASL:%04d> while (", c->line_nb);
      nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, ") { ... }\n");
      break;

    case NODE_FOREACH:
      fprintf(fp, "NASL:%04d> foreach %s (", c->line_nb, c->x.str_val);
      nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, ") { ... }\n");
      break;

    case NODE_REPEAT_UNTIL:
      fprintf(fp, "NASL:%04d> repeat { ... } until (", c->line_nb);
      nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, ")\n");
      break;

    case NODE_REPEATED:
      fprintf(fp, "NASL:%04d> ... x ", c->line_nb); 
      nasl_dump_expr(fp, c->link[1]);
      putc('\n', fp);
      break;

    case NODE_RETURN:
      fprintf(fp, "NASL:%04d> return ", c->line_nb);
      nasl_dump_expr(fp, c->link[0]);
      fprintf(fp, ";\n");
      break;

    case NODE_BREAK:
      fprintf(fp, "NASL:%04d> break\n", c->line_nb);
      break;

    case NODE_CONTINUE:
      fprintf(fp, "NASL:%04d> continue\n", c->line_nb);
      break;

    case NODE_AFF:
    case NODE_PLUS_EQ:
    case NODE_MINUS_EQ:
    case NODE_MULT_EQ:
    case NODE_DIV_EQ:
    case NODE_MODULO_EQ:
    case NODE_R_SHIFT_EQ:
    case NODE_R_USHIFT_EQ:
    case NODE_L_SHIFT_EQ:
      fprintf(fp, "NASL:%04d> ", c->line_nb);
      nasl_dump_expr(fp, c);
      fprintf(fp, ";\n");
      break;

    case NODE_FUN_CALL:
      fprintf(fp, "NASL:%04d> %s(...)\n", c->line_nb, c->x.str_val);
      break;

    case NODE_LOCAL:
      fprintf(fp, "NASL:%04d> local_var ...\n", c->line_nb);
      break;

    case NODE_GLOBAL:
      fprintf(fp, "NASL:%04d> global_var ...\n", c->line_nb);
      break;
    }
}


static int
expo(int x, int y)
{
  int	z;

  if (y == 0)
    return 1;
  else if (y < 0)
    if (x == 1)
      return 1;
    else
      return 0;
  else if (y == 1)
    return x;

  z = expo(x, y /2);
  if (y % 2 == 0)
    return z * z;
  else
    return x * z * z;
}

tree_cell*
nasl_exec(lex_ctxt* lexic, tree_cell* st)
{
  tree_cell	*ret = NULL, *ret2 = NULL, *tc1 = NULL, *tc2 = NULL, *tc3 = NULL, *idx = NULL, *args;
  int		flag, x, y, z;
  char		*s1 = NULL, *s2 = NULL, *s3 = NULL, *p = NULL;
  char		*p1, *p2;
  int		len1, len2;
  nasl_func	*pf = NULL;
  int		i, n;
  unsigned long sz;


#if 0
  nasl_dump_tree(st);      /* See rt.value, rt.type, rt.length */
#endif

  /* return */
  if (lexic->ret_val != NULL)
    {
      ref_cell(lexic->ret_val);
      return lexic->ret_val;
    }

  /* break or continue */
  if (lexic->break_flag || lexic->cont_flag)
    return FAKE_CELL;

  if (st == FAKE_CELL)
    return FAKE_CELL;

  if (st == NULL)
    {
#if NASL_DEBUG > 0
      nasl_perror(lexic, "nasl_exec: st == NULL\n");
#endif
      return NULL;
    }

  if (nasl_trace_fp != NULL)
    nasl_short_dump(nasl_trace_fp, st);

  switch(st->type)
    {
    case NODE_IF_ELSE:
      ret = nasl_exec(lexic, st->link[0]);
#ifdef STOP_AT_FIRST_ERROR
      if (ret == NULL)
	return NULL;
#endif
      if (cvt_bool(lexic, ret))
	ret2 = nasl_exec(lexic, st->link[1]);
      else
	if (st->link[2] != NULL) /* else branch */
	  ret2 = nasl_exec(lexic, st->link[2]);
	else			/* No else */
	  ret2 = FAKE_CELL;
      deref_cell(ret);
      return ret2;

    case NODE_INSTR_L:	/* Block. [0] = first instr, [1] = tail */
      ret = nasl_exec(lexic, st->link[0]);
#if NASL_DEBUG > 1
      if (ret == NULL)
	nasl_perror(lexic, "Instruction failed. Going on in block\n");
#endif
      if (st->link[1] == NULL || lexic->break_flag || lexic->cont_flag)
	return ret;
      deref_cell(ret);
      ret = nasl_exec(lexic, st->link[1]);
      return ret;
	
    case NODE_FOR:
      /* [0] = start expr, [1] = cond, [2] = end_expr, [3] = block */
      ret2 = nasl_exec(lexic, st->link[0]);
#ifdef STOP_AT_FIRST_ERROR
      if (ret2 == NULL)
	return NULL;
#endif
      deref_cell(ret2);
      for (;;)
	{
	  /* Break the loop if 'return' */
	  if (lexic->ret_val != NULL)
	    {
	      ref_cell(lexic->ret_val);
	      return lexic->ret_val;
	    }

	  /* condition */
	  if ((ret = nasl_exec(lexic, st->link[1])) == NULL)
	    return NULL;	/* We can return here, as NULL is false */
	  flag = cvt_bool(lexic, ret);
	  deref_cell(ret);
	  if (! flag)
	    break;
	  /* block */
	  ret = nasl_exec(lexic, st->link[3]);
#ifdef STOP_AT_FIRST_ERROR
	  if (ret == NULL)
	    return NULL;
#endif
	  deref_cell(ret);

	  /* break */
	  if (lexic->break_flag)
	    {
	      lexic->break_flag = 0;
	      return FAKE_CELL;
	    }

	  lexic->cont_flag = 0;	/* No need to test if set */

	  /* end expression */
	  ret = nasl_exec(lexic, st->link[2]);
#ifdef STOP_AT_FIRST_ERROR
	  if (ret == NULL)
	    return NULL;
#endif
	  deref_cell(ret); 
	}
      return FAKE_CELL;

    case NODE_WHILE:
      /* [0] = cond, [1] = block */
      for (;;)
	{
	  /* return? */
	  if (lexic->ret_val != NULL)
	    {
	      ref_cell(lexic->ret_val);
	      return lexic->ret_val;
	    }
	  /* Condition */
	  if ((ret = nasl_exec(lexic, st->link[0])) == NULL)
	    return NULL;	/* NULL is false */
	  flag = cvt_bool(lexic, ret);
	  deref_cell(ret);
	  if (! flag)
	    break;
	  /* Block */
	  ret = nasl_exec(lexic, st->link[1]);
#ifdef STOP_AT_FIRST_ERROR
	  if (ret == NULL)
	    return NULL;
#endif	  
	  deref_cell(ret);

	  /* break */
	  if (lexic->break_flag)
	    {
	      lexic->break_flag = 0;
	      return FAKE_CELL;
	    }
	  lexic->cont_flag = 0;
	}
      return FAKE_CELL;

    case NODE_REPEAT_UNTIL:
      /* [0] = block, [1] = cond  */
      for (;;)
	{
	  /* return? */
	  if (lexic->ret_val != NULL)
	    {
	      ref_cell(lexic->ret_val);
	      return lexic->ret_val;
	    }
	  /* Block */
	  ret = nasl_exec(lexic, st->link[0]);
#ifdef STOP_AT_FIRST_ERROR
	  if (ret == NULL)
	    return NULL;
#endif
	  deref_cell(ret);

	  /* break */
	  if (lexic->break_flag)
	    {
	      lexic->break_flag = 0;
	      return FAKE_CELL;
	    }
	  lexic->cont_flag = 0;

	  /* Condition */
	  ret = nasl_exec(lexic, st->link[1]);
#ifdef STOP_AT_FIRST_ERROR
	  if (ret == NULL)
	    return NULL;
#endif
	  flag = cvt_bool(lexic, ret);
	  deref_cell(ret);
	  if (flag)
	    break;
	}
      return FAKE_CELL;

    case NODE_FOREACH:
      /* str_val = index name, [0] = array, [1] = block */
      {
	nasl_iterator	ai;
	tree_cell	*v, *a, *val;

	v = get_variable_by_name(lexic, st->x.str_val);
	if (v == NULL)
	  return NULL;		/* We cannot go on if we have no variable to iterate */
	a = nasl_exec(lexic, st->link[0]); 
	ai = nasl_array_iterator(a);
	while ((val = nasl_iterate_array(&ai)) != NULL)
	  {
	    tc1 = nasl_affect(v, val);
	    ret = nasl_exec(lexic, st->link[1]);
	    deref_cell(val);
	    deref_cell(tc1);
#ifdef STOP_AT_FIRST_ERROR
	    if (ret == NULL) 
	      break;
#endif
	    deref_cell(ret);

	    /* return */
	    if (lexic->ret_val != NULL)
	      break;
	    /* break */
	    if (lexic->break_flag)
	      {
		lexic->break_flag = 0;
		break;
	      }
	    lexic->cont_flag = 0;
	  }
	deref_cell(a);
	deref_cell(v);
      }
      return FAKE_CELL;

    case NODE_FUN_DEF:
      /* x.str_val = function name, [0] = argdecl, [1] = block */
      ret = decl_nasl_func(lexic, st);
      return ret;

    case NODE_FUN_CALL:
      pf = get_func_ref_by_name(lexic, st->x.str_val);
      if (pf == NULL)
	{
	  nasl_perror(lexic, "Undefined function '%s'\n", st->x.str_val);
	  return NULL;
	}
      args = st->link[0];
#if 0
      printf("****************\n");
      nasl_dump_tree(args);
      printf("****************\n");
#endif
      ret = nasl_func_call(lexic, pf, args);
      return ret;

    case NODE_REPEATED:
      n = cell2intW(lexic, st->link[1]);
      if (n <= 0)
	return NULL;
	
#ifdef STOP_AT_FIRST_ERROR	
      for (tc1 = NULL, i = 1; i <= n; i ++)
	{
	  deref_cell(tc1);
	  if ((tc1 = nasl_exec(lexic, st->link[0])) == NULL)
	    return NULL;
	}
      return tc1;
#else
      for (i = 1; i <= n; i ++)
	{
	  tc1 = nasl_exec(lexic, st->link[0]);
	  deref_cell(tc1);
	}
      return FAKE_CELL;
#endif

      /*
       * I wonder... 
       * Will nasl_exec be really called with NODE_EXEC or NODE_ARG?
       */
    case NODE_DECL:		/* Used in function declarations */
      /* [0] = next arg in list */
      /* TBD? */
      return st;		/* ? */

    case NODE_ARG:		/* Used function calls */
      /* val = name can be NULL, [0] = val, [1] = next arg */
      ret = nasl_exec(lexic, st->link[0]);	/* Is this wise? */
      return ret;

    case NODE_RETURN:
      /* [0] = ret val */
      ret = nasl_return(lexic, st->link[0]);
      return ret;

    case NODE_BREAK:
      lexic->break_flag = 1;
      return FAKE_CELL;

    case NODE_CONTINUE:
      lexic->cont_flag = 1;
      return FAKE_CELL;

    case NODE_ARRAY_EL:		/* val = array name, [0] = index */
      idx = cell2atom(lexic, st->link[0]);
      ret = get_array_elem(lexic, st->x.str_val, idx);
      deref_cell(idx);
      return ret;

    case NODE_AFF:
      /* [0] = lvalue, [1] = rvalue */
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      ret = nasl_affect(tc1, tc2);
      deref_cell(tc1);		/* Must free VAR_REF */
      deref_cell(ret);
      return tc2;		/* So that "a = b = e;" works */

    case NODE_PLUS_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_PLUS, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;		/* So that "a = b += e;" works */
      
    case NODE_MINUS_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_MINUS, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;		/* So that "a = b -= e;" works */
      
    case NODE_MULT_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_MULT, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_DIV_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_DIV, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_MODULO_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_MODULO, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_L_SHIFT_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_L_SHIFT, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_R_SHIFT_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_R_SHIFT, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_R_USHIFT_EQ:
      tc1 = nasl_exec(lexic, st->link[0]);
      tc2 = nasl_exec(lexic, st->link[1]);
      tc3 = alloc_expr_cell(0, EXPR_R_USHIFT, tc1, tc2);
      ret2 = nasl_exec(lexic, tc3);
      ret = nasl_affect(tc1, ret2);
      deref_cell(tc3);		/* Frees tc1 and tc2 */
      deref_cell(ret);
      return ret2;
      
    case NODE_VAR:
      /* val = variable name */
      ret = get_variable_by_name(lexic, st->x.str_val);
      return ret;

    case NODE_LOCAL:		/* [0] = argdecl */
      ret = decl_local_variables(lexic, st->link[0]);
      return ret;

    case NODE_GLOBAL:		/* [0] = argdecl */
      ret = decl_global_variables(lexic, st->link[0]);
      return ret;

    case EXPR_AND:
      x = cell2bool(lexic, st->link[0]);
      if(! x)
	return bool2cell(0);
      
      y = cell2bool(lexic, st->link[1]);
      return bool2cell(y);
     

    case EXPR_OR:
      x = cell2bool(lexic, st->link[0]);
      if(x)
       return bool2cell(x);
      y = cell2bool(lexic, st->link[1]);
      return bool2cell(y);

    case EXPR_NOT:
      x = cell2bool(lexic, st->link[0]);
      return bool2cell(! x);

    case EXPR_INCR:
    case EXPR_DECR:
      x =  (st->type == EXPR_INCR) ? 1 : -1;
      if (st->link[0] == NULL)
	{
	  y = 1;		/* pre */
	  tc1 = st->link[1];
	}
      else
	{
	  y = 0;		/* post */
	  tc1 = st->link[0];
	}
      tc2 = nasl_exec(lexic, tc1);
      if (tc2 == NULL)
	return NULL;
      ret = nasl_incr_variable(lexic, tc2, y, x);
      deref_cell(tc2);
      return ret;

      if (st->link[0] == NULL)
	ret = nasl_incr_variable(lexic, st->link[1], 1, 1);
      else
	ret = nasl_incr_variable(lexic, st->link[1], 0, 1);
      break;

    case EXPR_PLUS:
      s1 = s2 = NULL;
      tc1 = cell2atom(lexic, st->link[0]);
#ifdef STOP_AT_FIRST_ERROR
      if (tc1 == NULL || tc1 == FAKE_CELL)
	return NULL;
#endif
      tc2 = cell2atom(lexic, st->link[1]);
      if (tc2 == NULL || tc2 == FAKE_CELL)
	{
#ifdef STOP_AT_FIRST_ERROR
	  deref_cell(tc1);
	  return NULL;
#else
	  return tc1;
#endif
	}

      if (tc1 == NULL || tc1 == FAKE_CELL)
	return tc2;

      /*
       * Anything added to a string is converted to a string
       * Otherwise anything added to an intger is converted into an integer
       */
      if (tc1->type == CONST_DATA || tc2->type == CONST_DATA)
	flag = CONST_DATA;
      else if (tc1->type == CONST_STR || tc2->type == CONST_STR)
	flag = CONST_STR;
      else if (tc1->type == CONST_INT || tc2->type == CONST_INT)
	flag = CONST_INT;
      else
	flag = NODE_EMPTY;
#if NASL_DEBUG > 0
      if ((flag == CONST_DATA || flag == CONST_STR) && 
	  (tc1->type == CONST_INT || tc2->type == CONST_INT))
	nasl_perror(lexic, "Horrible type conversion (int -> str) for operator + %s\n", get_line_nb(st));
#endif
      switch (flag)
	{
	case CONST_INT:
	  x = tc1->x.i_val;
	  y = cell2int(lexic, tc2);
	  ret = int2cell(x + y);
	  break;

	case CONST_STR:
	case CONST_DATA:
	  s1 = s2 = NULL;
	  if (tc1->type == CONST_STR || tc1->type == CONST_DATA)
	    len1 = tc1->size;
	  else
	    {
	      s1 = cell2str(lexic, tc1);
	      len1 = (s1 == NULL ? 0 : strlen(s1));
	    }

	  if (tc2->type == CONST_STR || tc2->type == CONST_DATA)
	    len2 = tc2->size;
	  else
	    {
	      s2 = cell2str(lexic, tc2);
	      len2 = (s2 == NULL ? 0 : strlen(s2));
	    }

	  sz = len1 + len2;
	  s3 = emalloc(sz);
	  if (len1 > 0)
	    memcpy(s3, s1 != NULL ? s1 : tc1->x.str_val, len1);
	  if (len2 > 0)
	    memcpy(s3 + len1, s2 != NULL ? s2 : tc2->x.str_val, len2);
	  efree(&s1); efree(&s2);
	  ret = alloc_tree_cell(0, s3);
	  ret->type = flag;
	  ret->size = sz;
	  break;

	default:
	  ret = NULL;
	  break;
	}
      deref_cell(tc1);
      deref_cell(tc2);
      return ret;

    case EXPR_MINUS:		/* Infamous duplicated code */
      s1 = s2 = NULL;
      tc1 = cell2atom(lexic, st->link[0]);
#ifdef STOP_AT_FIRST_ERROR
      if (tc1 == NULL || tc1 == FAKE_CELL)
	return NULL;
#endif
      tc2 = cell2atom(lexic, st->link[1]);
      if (tc2 == NULL || tc2 == FAKE_CELL)
	{
#ifdef STOP_AT_FIRST_ERROR
	  deref_cell(tc1);
	  return NULL;
#else
	  return tc1;
#endif
	}

      if (tc1 == NULL || tc1 == FAKE_CELL)
	{
	  if (tc2->type == CONST_INT)
	    {
	      y = cell2int(lexic, tc2);
	      ret = int2cell(- y);
	    }
	  else
	    ret = NULL;
	  deref_cell(tc2);
	  return ret;
	}

      /*
       * Anything substracted from a string is converted to a string
       * Otherwise anything substracted from integer is converted into an
       * integer
       */
      if (tc1->type == CONST_DATA || tc2->type == CONST_DATA)
	flag = CONST_DATA;
      else if (tc1->type == CONST_STR || tc2->type == CONST_STR)
	flag = CONST_STR;
      else if (tc1->type == CONST_INT || tc2->type == CONST_INT)
	flag = CONST_INT;
      else
	flag = NODE_EMPTY;
#if NASL_DEBUG > 0
      if ((flag == CONST_DATA || flag == CONST_STR) && 
	  (tc1->type == CONST_INT || tc2->type == CONST_INT))
	nasl_perror(lexic, "Horrible type conversion (int -> str) for operator - %s\n", get_line_nb(st));
#endif
      switch (flag)
	{
	case CONST_INT:
	  x = cell2int(lexic, tc1);
	  y = cell2int(lexic, tc2);
	  ret = int2cell(x - y);
	  break;

	case CONST_STR:
	case CONST_DATA:
	  if (tc1->type == CONST_STR || tc1->type == CONST_DATA)
	    {
	      p1 = tc1->x.str_val;
	      len1 = tc1->size;
	    }
	  else
	    {
	      p1 = s1 = cell2str(lexic, tc1);
	      len1 = (s1 == NULL ? 0 : strlen(s1));
	    }
	      
	  if (tc2->type == CONST_STR || tc2->type == CONST_DATA)
	    {
	      p2 = tc2->x.str_val;
	      len2 = tc2->size;
	    }
	  else
	    {
	      p2 = s2 = cell2str(lexic, tc2);
	      len2 = (s2 == NULL ? 0 : strlen(s2));
	    }

	  if (len2 == 0 || len1 < len2 || 
	      (p = (char*)nasl_memmem(p1, len1,  p2, len2)) == NULL)
	    {
	      s3 = emalloc(len1);
	      memcpy(s3, p1, len1);
	      ret = alloc_tree_cell(0, s3);
	      ret->type = flag;
	      ret->size = len1;
	    }
	  else
	    {
	      sz = len1 - len2;
	      if (sz <= 0)
		{
		  sz = 0;
		  s3 = estrdup("");
		}
	      else
		{
		  s3 = emalloc(sz);
		  if (p - p1 > 0)
		    memcpy(s3, p1, p - p1);
		  if (sz > p - p1)
		    memcpy(s3 + (p - p1), p + len2, sz - (p - p1));
		}
	      ret = alloc_tree_cell(0, s3);
	      ret->size = sz;
	      ret->type = flag;
	    }

	  efree(&s1); efree(&s2);
	 break;

	default:
	  ret = NULL;
	  break;
	}
      deref_cell(tc1);
      deref_cell(tc2);
      return ret;
    
    case EXPR_MULT:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(x * y);

    case EXPR_DIV:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      if( y != 0 )
       return int2cell(x / y);
      else
       return int2cell(0);
       
    case EXPR_EXPO:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(expo(x, y));

    case EXPR_MODULO:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      if( y != 0)
       return int2cell(x % y);
      else
       return int2cell(0);

    case EXPR_BIT_AND:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(x & y);

    case EXPR_BIT_OR:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(x | y);

    case EXPR_BIT_XOR:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(x ^ y);

    case EXPR_BIT_NOT:
      x = cell2intW(lexic, st->link[0]);
      return int2cell(~ x);

    case EXPR_U_MINUS:
      x = cell2intW(lexic, st->link[0]);
      return int2cell(- x);

      /* TBD: Handle shift for strings and arrays */
    case EXPR_L_SHIFT:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
      return int2cell(x << y);

    case EXPR_R_SHIFT:		/* arithmetic right shift */
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
#if NASL_DEBUG > 0
      if (y < 0)
	nasl_perror(lexic, "Warning: Negative count in right shift!\n");
#endif
      z = x >> y;
#ifndef __GNUC__
      if (x < 0 && z >= 0)	/* Fix it */
	{
#if NASL_DEBUG > 1
	  nasl_perror(lexic, "Warning: arithmetic right shift is buggy! Fixing...\n");
#endif
	  z |= (~0) << (sizeof(x) * 8 - y);
	}
#endif
      return int2cell(z);

    case EXPR_R_USHIFT:
      x = cell2intW(lexic, st->link[0]);
      y = cell2intW(lexic, st->link[1]);
#if NASL_DEBUG > 0
      if (y < 0)
	nasl_perror(lexic, "Warning: Negative count in right shift!\n");
#endif
      z = (unsigned)x >> (unsigned)y;
#ifndef __GNUC__
      if (x < 0 && z <= 0)	/* Fix it! */
	{
#if NASL_DEBUG > 1
	  nasl_perror(lexic, "Warning: Logical right shift is buggy! Fixing...\n");
#endif
	  z &= ~((~0) << (sizeof(x) * 8 - y));
	}
#endif
      return int2cell(z);

    case COMP_MATCH:
    case COMP_NOMATCH:
      tc1 = cell2atom(lexic, st->link[0]); 
      tc2 = cell2atom(lexic, st->link[1]); 
      s1 = s2 = NULL;

      if (tc1 == NULL || tc1 == FAKE_CELL)
	{
	  p1 = ""; 
	  len1 = 0;
	}
      else if (tc1->type == CONST_STR || tc1->type == CONST_DATA)
	{
	  p1 = tc1->x.str_val;
	  len1 = tc1->size;
	}
      else
	{
#if NASL_DEBUG > 0
	  nasl_perror(lexic, "Horrible type conversion (%s -> str) for operator >< or >!< %s\n", nasl_type_name(tc1->type), get_line_nb(st));
#endif
	  p1 = s1 = cell2str(lexic, tc1);
	  len1 = strlen(s1);
	}

      if (tc2 == NULL || tc2 == FAKE_CELL)
	{
	  p2 = "";
	  len2 = 0;
	}
      else if (tc2->type == CONST_STR || tc2->type == CONST_DATA)
	{
	  p2 = tc2->x.str_val;
	  len2 = tc2->size;
	}
      else
	{
#if NASL_DEBUG > 0
	  nasl_perror(lexic, "Horrible type conversion (%s -> str) for operator >< or >!< %s\n", nasl_type_name(tc2->type), get_line_nb(st));
#endif
	  p2 = s2 = cell2str(lexic, tc2);
	  len2 = strlen(s2);
	}

      if(len1 <= len2)		
      	flag = ((void*)nasl_memmem(p2, len2, p1, len1) != NULL);
      else
      	flag = 0;
	
      efree(&s1); efree(&s2);
      deref_cell(tc1);
      deref_cell(tc2);
      if (st->type == COMP_MATCH)
	return bool2cell(flag);
      else
	return bool2cell(! flag);

    case COMP_RE_MATCH:
    case COMP_RE_NOMATCH:
      if (st->x.ref_val == NULL)
	{
	  nasl_perror(lexic, "nasl_exec: bad regex at or near line %d\n",
		  st->line_nb);
	  return NULL;
	}
      s1 = cell2str(lexic, st->link[0]);
      if (s1 == NULL)
	return 0;
      flag = nasl_regexec(st->x.ref_val, s1, 0, NULL, 0);
      free(s1);
      if (st->type == COMP_RE_MATCH)
	return bool2cell(flag != REG_NOMATCH);
      else
	return bool2cell(flag == REG_NOMATCH);

    case COMP_LT:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) < 0);

    case COMP_LE:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) <= 0);

    case COMP_EQ:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) == 0);

    case COMP_NE:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) != 0);

    case COMP_GT:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) > 0);

    case COMP_GE:
      return bool2cell(cell_cmp(lexic, st->link[0], st->link[1]) >= 0);

    case REF_ARRAY:
    case DYN_ARRAY:
    case CONST_INT:
    case CONST_STR:
    case CONST_DATA:
      ref_cell(st);	/* nasl_exec returns a cell that should be deref-ed */
      return st;

    case REF_VAR:
      ret = nasl_read_var_ref(lexic, st);
      return ret;

    default:
      nasl_perror(lexic, "nasl_exec: unhandled node type %d\n", st->type);
      abort();
      return NULL;
    }

  deref_cell(ret);
  deref_cell(ret2);
  return NULL;
}

/*
 * Note that "mode" is now a bit field, instead of a simple boolean
 *
 * bit #0 (1) is "description"
 * Bit #1 (2) is "parse only"
 */
extern tree_cell*	nasl_lint(lex_ctxt*, tree_cell*);

int
execute_nasl_script(struct arglist * script_infos, const char* name, const char * cache_dir, int mode)
{
  naslctxt	ctx;
  nasl_func	*pf;
  int		err = 0;
  tree_cell	*ret;
  lex_ctxt	*lexic;
  char 	 	old_dir[MAXPATHLEN+1];
  char		*newdir;
  char		*old;
  tree_cell	tc;
  struct arglist*	prefs = arg_get_value(script_infos, "preferences");
  char		*str;
  int		to;
  char * basename;

#ifdef ENABLE_PLUGIN_SERVER
  char * cached_script = NULL;
  unsigned int cached_script_len = 0;
#endif
  
  srand48(getpid() + getppid() + (long)time(NULL));

  old_dir[sizeof(old_dir) - 1] = '\0';
  getcwd(old_dir, sizeof(old_dir) - 1);
#if NASL_DEBUG > 2
  nasl_trace_fp = stderr;
#endif
 if((old = arg_get_value(script_infos, "script_name")) == NULL)
   arg_add_value(script_infos, "script_name", ARG_STRING, strlen(name), estrdup(name));
 else
   { 
   efree(&old);
   arg_set_value(script_infos, "script_name", strlen(name), estrdup(name));
  }
 
 newdir = strrchr(name, '/');
 if(newdir != NULL)
 {
	 char dir[MAXPATHLEN+1];
	 char *s;
	 dir[sizeof(dir) - 1] = '\0';
	 strncpy(dir, name, sizeof(dir) - 1);
	 s = strrchr(dir, '/');
	 s[0] = '\0';
	 chdir(dir);
	 basename = newdir + 1;
 }
 else basename = (char*)name;

 bzero(&ctx, sizeof(ctx));
 if ( mode & NASL_ALWAYS_SIGNED )
	ctx.always_authenticated = 1;

 
#ifdef ENABLE_PLUGIN_SERVER
 if (  nasl_index_fetch(basename, &cached_script, &cached_script_len) >= 0 )
 {
  if ( nasl_load_parsed_tree_buf(&ctx, cached_script, cached_script_len, basename) < 0 )  
  {
   printf("Could not load plugin\n");
   efree(&cached_script);
   chdir(old_dir);
   return -1;
  }
  efree(&cached_script);
 }
 else
#endif
 {
 if (nasl_load_or_parse(&ctx, name, basename, cache_dir) < 0 )
  {
    chdir(old_dir);
    return -1;
  }
 }


#if NASL_DEBUG > 4
 nasl_dump_tree(ctx.tree);
#endif
 lexic = init_empty_lex_ctxt();
 lexic->script_infos = script_infos;

 if ( mode & NASL_ALWAYS_SIGNED )
 	lexic->authenticated = 1;
 else
 	lexic->authenticated = ctx.authenticated;
 
 str = arg_get_value(prefs, "checks_read_timeout");
 if( str != NULL )
 	to = atoi(str);
 else
 	to = 5;
	
 if(to <= 0)to = 5;
 
 lexic->recv_timeout = to;

 init_nasl_library(lexic);

 if (mode & NASL_LINT)
   {
     if (nasl_lint(lexic, ctx.tree) == NULL)
       err --;
   }
 else
 if (! (mode & NASL_EXEC_PARSE_ONLY))
   {
     char	*p;

     bzero(&tc, sizeof(tc));
     tc.type = CONST_INT;
     tc.x.i_val = (mode & NASL_COMMAND_LINE) != 0;
     add_named_var_to_ctxt(lexic, "COMMAND_LINE", &tc);

     bzero(&tc, sizeof(tc));
     tc.type = CONST_INT;
     tc.x.i_val = (mode & NASL_EXEC_DESCR) != 0;
     add_named_var_to_ctxt(lexic, "description", &tc);

     tc.type = CONST_DATA;
     p = strrchr(name, '/');
     if (p == NULL) p = (char*)name; else p ++;
     tc.x.str_val = p;
     tc.size = strlen(p);
     add_named_var_to_ctxt(lexic, "SCRIPT_NAME", &tc);

     truc = (lex_ctxt*)ctx.tree;
     if ((ret = nasl_exec(lexic, ctx.tree)) == NULL)
       err = -1;
     else
       deref_cell(ret);

     if ((pf = get_func_ref_by_name(lexic, "on_exit")) != NULL)
       nasl_func_call(lexic, pf, NULL);
   }

#if NASL_DEBUG > 2
 {
   struct rusage	ru;

   if (getrusage(RUSAGE_SELF, &ru) < 0)
     perror("getrusage");
   else
     {
       nasl_perror(lexic, 
		   "rusage: utime=%d.%03d stime=%d.%03d minflt=%d majflt=%d nswap=%d\n",
		   ru.ru_utime.tv_sec, ru.ru_utime.tv_usec / 1000,
		   ru.ru_stime.tv_sec, ru.ru_stime.tv_usec / 1000,
		   ru.ru_minflt, ru.ru_majflt, ru.ru_nswap);
     }
 }
#endif

#if NASL_DEBUG > 3
 nasl_dump_tree(ctx.tree);
#endif

 chdir(old_dir);
 if ( mode & NASL_EXEC_DONT_CLEANUP ) return err;

 nasl_clean_ctx(&ctx);
 free_lex_ctxt(lexic);


 return err;
}



/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <my_global.h>
#include <m_string.h>
#include <stdlib.h>
#include <my_getopt.h>
#include <assert.h>
#include <my_sys.h>
#include <mysys_err.h>

static int findopt(char *optpat, uint length,
		   const struct my_option **opt_res,
		   char **ffname);
my_bool getopt_compare_strings(const char *s,
			       const char *t,
			       uint length);
static longlong getopt_ll(char *arg, const struct my_option *optp, int *err);
static ulonglong getopt_ull(char *arg, const struct my_option *optp,
			    int *err);
static void init_variables(const struct my_option *options);
static int setval(const struct my_option *opts, char *argument,
		  my_bool set_maximum_value);

/*
  The following three variables belong to same group and the number and
  order of their arguments must correspond to each other.
*/
static const char *special_opt_prefix[]=
{"skip", "disable", "enable", "maximum", "loose", 0};
static const uint special_opt_prefix_lengths[]=
{ 4,      7,         6,        7,         5,      0};
enum enum_special_opt
{ OPT_SKIP, OPT_DISABLE, OPT_ENABLE, OPT_MAXIMUM, OPT_LOOSE};

char *disabled_my_option= (char*) "0";

/* 
   This is a flag that can be set in client programs. 0 means that
   my_getopt will not print error messages, but the client should do
   it by itself
*/

my_bool my_getopt_print_errors= 1;


/* 
  function: handle_options

  Sort options; put options first, until special end of options (--), or
  until end of argv. Parse options; check that the given option matches with
  one of the options in struct 'my_option', return error in case of ambiguous
  or unknown option. Check that option was given an argument if it requires
  one. Call function 'get_one_option()' once for each option.
*/

int handle_options(int *argc, char ***argv, 
		   const struct my_option *longopts, 
		   my_bool (*get_one_option)(int,
					     const struct my_option *,
					     char *))
{
  uint opt_found, argvpos= 0, length, i;
  my_bool end_of_options= 0, must_be_var, set_maximum_value, special_used,
          option_is_loose;
  char *progname= *(*argv), **pos, **pos_end, *optend, *prev_found;
  const struct my_option *optp;
  int error;

  LINT_INIT(opt_found);
  (*argc)--; /* Skip the program name */
  (*argv)++; /*      --- || ----      */
  init_variables(longopts);

  for (pos= *argv, pos_end=pos+ *argc; pos != pos_end ; pos++)
  {
    char *cur_arg= *pos;
    if (cur_arg[0] == '-' && cur_arg[1] && !end_of_options) /* must be opt */
    {
      char *argument=    0;
      must_be_var=       0;
      set_maximum_value= 0;
      special_used=      0;
      option_is_loose=   0;

      cur_arg++;		/* skip '-' */
      if (*cur_arg == '-' || *cur_arg == 'O') /* check for long option, */
      {                                       /* --set-variable, or -O  */
	if (*cur_arg == 'O')
	{
	  must_be_var= 1;

	  if (!(*++cur_arg))	/* If not -Ovar=# */
	  {
	    /* the argument must be in next argv */
	    if (!*++pos)
	    {
	      if (my_getopt_print_errors)
		fprintf(stderr, "%s: Option '-O' requires an argument\n",
			progname);
	      return EXIT_ARGUMENT_REQUIRED;
	    }
	    cur_arg= *pos;
	    (*argc)--;
	  }
	}
	else if (!getopt_compare_strings(cur_arg, "-set-variable", 13))
	{
	  must_be_var= 1;
	  if (cur_arg[13] == '=')
	  {
	    cur_arg+= 14;
	    if (!*cur_arg)
	    {
	      if (my_getopt_print_errors)
		fprintf(stderr,
			"%s: Option '--set-variable' requires an argument\n",
			progname);
	      return EXIT_ARGUMENT_REQUIRED;
	    }
	  }
	  else if (cur_arg[14]) /* garbage, or another option. break out */
	    must_be_var= 0;
	  else
	  {
	    /* the argument must be in next argv */
	    if (!*++pos)
	    {
	      if (my_getopt_print_errors)
		fprintf(stderr,
			"%s: Option '--set-variable' requires an argument\n",
			progname);
	      return EXIT_ARGUMENT_REQUIRED;
	    }
	    cur_arg= *pos;
	    (*argc)--;
	  }
	}
	else if (!must_be_var)
	{
	  if (!*++cur_arg)	/* skip the double dash */
	  {
	    /* '--' means end of options, look no further */
	    end_of_options= 1;
	    (*argc)--;
	    continue;
	  }
	}
	optend= strcend(cur_arg, '=');
	length= optend - cur_arg;
	if (*optend == '=')
	  optend++;
	else
	  optend=0;

	/*
	  Find first the right option. Return error in case of an ambiguous,
	  or unknown option
	*/
	optp= longopts;
	if (!(opt_found= findopt(cur_arg, length, &optp, &prev_found)))
	{
	  /*
	    Didn't find any matching option. Let's see if someone called
	    option with a special option prefix
	  */
	  if (!must_be_var)
	  {
	    if (optend)
	      must_be_var= 1; /* option is followed by an argument */
	    for (i= 0; special_opt_prefix[i]; i++)
	    {
	      if (!getopt_compare_strings(special_opt_prefix[i], cur_arg,
					  special_opt_prefix_lengths[i]) &&
		  cur_arg[special_opt_prefix_lengths[i]] == '-')
	      {
		/*
		  We were called with a special prefix, we can reuse opt_found
		*/
		special_used= 1;
		cur_arg+= (special_opt_prefix_lengths[i] + 1);
		if (i == OPT_LOOSE)
		  option_is_loose= 1;
		if ((opt_found= findopt(cur_arg, length -
					(special_opt_prefix_lengths[i] + 1),
					&optp, &prev_found)))
		{
		  if (opt_found > 1)
		  {
		    if (my_getopt_print_errors)
		      fprintf(stderr,
			      "%s: ambiguous option '--%s-%s' (--%s-%s)\n",
			      progname, special_opt_prefix[i], cur_arg,
			      special_opt_prefix[i], prev_found);
		    return EXIT_AMBIGUOUS_OPTION;
		  }
		  switch (i) {
		  case OPT_SKIP:
		  case OPT_DISABLE: /* fall through */
		    /*
		      double negation is actually enable again,
		      for example: --skip-option=0 -> option = TRUE
		    */
		    optend= (optend && *optend == '0' && !(*(optend + 1))) ?
		      (char*) "1" : disabled_my_option;
		    break;
		  case OPT_ENABLE:
		    optend= (optend && *optend == '0' && !(*(optend + 1))) ?
 		      disabled_my_option : (char*) "1";
		    break;
		  case OPT_MAXIMUM:
		    set_maximum_value= 1;
		    must_be_var= 1;
		    break;
		  }
		  break; /* break from the inner loop, main loop continues */
		}
	      }
	    }
	  }
	  if (!opt_found)
	  {
	    if (must_be_var)
	    {
	      if (my_getopt_print_errors)
		fprintf(stderr,
			"%s: %s: unknown variable '%s'\n", progname,
			option_is_loose ? "WARNING" : "ERROR", cur_arg);
	      if (!option_is_loose)
		return EXIT_UNKNOWN_VARIABLE;
	    }
	    else
	    {
	      if (my_getopt_print_errors)
		fprintf(stderr,
			"%s: %s: unknown option '--%s'\n", progname,
			option_is_loose ? "WARNING" : "ERROR", cur_arg);
	      if (!option_is_loose)
		return EXIT_UNKNOWN_OPTION;
	    }
	    if (option_is_loose)
	    {
	      (*argc)--;
	      continue;
	    }
	  }
	}
	if (opt_found > 1)
	{
	  if (must_be_var)
	  {
	    if (my_getopt_print_errors)
	      fprintf(stderr, "%s: variable prefix '%s' is not unique\n",
		      progname, cur_arg);
	    return EXIT_VAR_PREFIX_NOT_UNIQUE;
	  }
	  else
	  {
	    if (my_getopt_print_errors)
	      fprintf(stderr, "%s: ambiguous option '--%s' (%s, %s)\n",
		      progname, cur_arg, prev_found, optp->name);
	    return EXIT_AMBIGUOUS_OPTION;
	  }
	}
	if (must_be_var && optp->var_type == GET_NO_ARG)
	{
	  if (my_getopt_print_errors)
	    fprintf(stderr, "%s: option '%s' cannot take an argument\n",
		    progname, optp->name);
	  return EXIT_NO_ARGUMENT_ALLOWED;
	}
	if (optp->arg_type == NO_ARG)
	{
	  if (optend && optp->var_type != GET_BOOL)
	  {
	    if (my_getopt_print_errors)
	      fprintf(stderr, "%s: option '--%s' cannot take an argument\n",
		      progname, optp->name);
	    return EXIT_NO_ARGUMENT_ALLOWED;
	  }
	  if (optp->var_type == GET_BOOL)
	  {
	    /*
	      Set bool to 1 if no argument or if the user has used
	      --enable-'option-name'.
	      *optend was set to '0' if one used --disable-option
	      */
	    *((my_bool*) optp->value)= 	(my_bool) (!optend || *optend == '1');
	    (*argc)--;	    
	    get_one_option(optp->id, optp, argument);
	    continue;
	  }
	  argument= optend;
	}
	else if (optp->arg_type == OPT_ARG && optp->var_type == GET_BOOL)
	{
	  if (optend == disabled_my_option)
	    *((my_bool*) optp->value)= (my_bool) 0;
	  else
	  {
	    if (!optend) /* No argument -> enable option */
	      *((my_bool*) optp->value)= (my_bool) 1;
	    else /* If argument differs from 0, enable option, else disable */
	      *((my_bool*) optp->value)= (my_bool) atoi(optend) != 0;
	  }
	  (*argc)--;	    
	  continue;
	}
	else if (optp->arg_type == REQUIRED_ARG && !optend)
	{
	  /* Check if there are more arguments after this one */
	  if (!*++pos)
	  {
	    if (my_getopt_print_errors)
	      fprintf(stderr, "%s: option '--%s' requires an argument\n",
		      progname, optp->name);
	    return EXIT_ARGUMENT_REQUIRED;
	  }
	  argument= *pos;
	  (*argc)--;
	}
	else
	  argument= optend;
      }
      else  /* must be short option */
      {
	for (optend= cur_arg; *optend; optend++)
	{
	  opt_found= 0;
	  for (optp= longopts; optp->id; optp++)
	  {
	    if (optp->id == (int) (uchar) *optend)
	    {
	      /* Option recognized. Find next what to do with it */
	      opt_found= 1;
	      if (optp->var_type == GET_BOOL && optp->arg_type == NO_ARG)
	      {
		*((my_bool*) optp->value)= (my_bool) 1;
		get_one_option(optp->id, optp, argument);
		continue;
	      }
	      else if (optp->arg_type == REQUIRED_ARG ||
		       optp->arg_type == OPT_ARG)
	      {
		if (*(optend + 1))
		{
		  /* The rest of the option is option argument */
		  argument= optend + 1;
		  /* This is in effect a jump out of the outer loop */
		  optend= (char*) " ";
		}
		else if (optp->arg_type == REQUIRED_ARG)
		{
		  /* Check if there are more arguments after this one */
		  if (!*++pos)
		  {
		    if (my_getopt_print_errors)
		      fprintf(stderr,
			      "%s: option '-%c' requires an argument\n",
			      progname, optp->id);
		    return EXIT_ARGUMENT_REQUIRED;
		  }
		  argument= *pos;
		  (*argc)--;
		  /* the other loop will break, because *optend + 1 == 0 */
		}
	      }
	      if ((error= setval(optp, argument, set_maximum_value)))
	      {
		fprintf(stderr,
			"%s: Error while setting value '%s' to '%s'\n",
			progname, argument, optp->name);
		return error;
	      }
	      get_one_option(optp->id, optp, argument);
	      break;
	    }
	  }
	  if (!opt_found)
	  {
	    if (my_getopt_print_errors)
	      fprintf(stderr,
		      "%s: unknown option '-%c'\n", progname, *optend);
	    return EXIT_UNKNOWN_OPTION;
	  }
	}
	(*argc)--; /* option handled (short), decrease argument count */
	continue;
      }
      if ((error= setval(optp, argument, set_maximum_value)))
      {
	fprintf(stderr,
		"%s: Error while setting value '%s' to '%s'\n",
		progname, argument, optp->name);
	return error;
      }
      get_one_option(optp->id, optp, argument);

      (*argc)--; /* option handled (short�or�long), decrease argument count */
    }
    else /* non-option found */
      (*argv)[argvpos++]= cur_arg;
  }
  /*
    Destroy the first, already handled option, so that programs that look
    for arguments in 'argv', without checking 'argc', know when to stop.
    Items in argv, before the destroyed one, are all non-option -arguments
    to the program, yet to be (possibly) handled.
  */
  (*argv)[argvpos]= 0;
  return 0;
}

/*
  function: setval

  Arguments: opts, argument
  Will set the option value to given value
*/

static int setval(const struct my_option *opts, char *argument,
		  my_bool set_maximum_value)
{
  int err= 0;

  if (opts->value && argument)
  {
    gptr *result_pos= ((set_maximum_value) ?
		       opts->u_max_value : opts->value);

    if (!result_pos)
      return EXIT_NO_PTR_TO_VARIABLE;

    switch (opts->var_type) {
    case GET_INT:
    case GET_UINT:           /* fall through */
      *((int*) result_pos)= (int) getopt_ll(argument, opts, &err);
      break;
    case GET_LONG:
    case GET_ULONG:          /* fall through */
      *((long*) result_pos)= (long) getopt_ll(argument, opts, &err);
      break;
    case GET_LL:
      *((longlong*) result_pos)= getopt_ll(argument, opts, &err);
      break;
    case GET_ULL:
      *((ulonglong*) result_pos)= getopt_ull(argument, opts, &err);
      break;
    case GET_STR:
      *((char**) result_pos)= argument;
      break;
    case GET_STR_ALLOC:
      if ((*((char**) result_pos)))
	my_free((*(char**) result_pos), MYF(MY_WME | MY_FAE));
      if (!(*((char**) result_pos)= my_strdup(argument, MYF(MY_WME))))
	return EXIT_OUT_OF_MEMORY;
      break;
    default:    /* dummy default to avoid compiler warnings */
      break;
    }
    if (err)
      return EXIT_UNKNOWN_SUFFIX;
  }
  return 0;
}

/* 
  function: findopt

  Arguments: opt_pattern, length of opt_pattern, opt_struct, first found
  name (ffname)

  Go through all options in the my_option struct. Return number
  of options found that match the pattern and in the argument
  list the option found, if any. In case of ambiguous option, store
  the name in ffname argument
*/

static int findopt(char *optpat, uint length,
		   const struct my_option **opt_res,
		   char **ffname)
{
  int count;
  struct my_option *opt= (struct my_option *) *opt_res;

  for (count= 0; opt->name; opt++)
  {
    if (!getopt_compare_strings(opt->name, optpat, length)) /* match found */
    {
      (*opt_res)= opt;
      if (!count)
	*ffname= (char *) opt->name;	/* We only need to know one prev */
      if (!opt->name[length])		/* Exact match */
	return 1;
      count++;
    }
  }
  return count;
}


/* 
  function: compare_strings

  Works like strncmp, other than 1.) considers '-' and '_' the same.
  2.) Returns -1 if strings differ, 0 if they are equal
*/

my_bool getopt_compare_strings(register const char *s, register const char *t,
			       uint length)
{
  char const *end= s + length;
  for (;s != end ; s++, t++)
  {
    if ((*s != '-' ? *s : '_') != (*t != '-' ? *t : '_'))
      return 1;
  }
  return 0;
}

/*
  function: eval_num_suffix

  Transforms a number with a suffix to real number. Suffix can
  be k|K for kilo, m|M for mega or g|G for giga.
*/

static longlong eval_num_suffix (char *argument, int *error, char *option_name)
{
  char *endchar;
  longlong num;
  
  *error= 0;
  num= strtoll(argument, &endchar, 10);
  if (*endchar == 'k' || *endchar == 'K')
    num*= 1024L;
  else if (*endchar == 'm' || *endchar == 'M')
    num*= 1024L * 1024L;
  else if (*endchar == 'g' || *endchar == 'G')
    num*= 1024L * 1024L * 1024L;
  else if (*endchar)
  {
    fprintf(stderr,
	    "Unknown suffix '%c' used for variable '%s' (value '%s')\n",
	    *endchar, option_name, argument);
    *error= 1;
    return 0;
  }
  return num;
}

/* 
  function: getopt_ll

  Evaluates and returns the value that user gave as an argument
  to a variable. Recognizes (case insensitive) K as KILO, M as MEGA
  and G as GIGA bytes. Some values must be in certain blocks, as
  defined in the given my_option struct, this function will check
  that those values are honored.
  In case of an error, set error value in *err.
*/

static longlong getopt_ll(char *arg, const struct my_option *optp, int *err)
{
  longlong num;
  
  num= eval_num_suffix(arg, err, (char*) optp->name);
  if (num < (longlong) optp->min_value)
    num= (longlong) optp->min_value;
  else if (num > 0 && (ulonglong) num > (ulonglong) (ulong) optp->max_value
	   && optp->max_value) /* if max value is not set -> no upper limit */
    num= (longlong) (ulong) optp->max_value;
  num= ((num - (longlong) optp->sub_size) / (optp->block_size ?
					     (ulonglong) optp->block_size :
					     1L));
  return (longlong) (num * (optp->block_size ? (ulonglong) optp->block_size :
			    1L));
}

/*
  function: getopt_ull

  This is the same as getopt_ll, but is meant for unsigned long long
  values.
*/

static ulonglong getopt_ull(char *arg, const struct my_option *optp, int *err)
{
  ulonglong num;

  num= eval_num_suffix(arg, err, (char*) optp->name);  
  return getopt_ull_limit_value(num, optp);
}


ulonglong getopt_ull_limit_value(ulonglong num, const struct my_option *optp)
{
  if ((ulonglong) num > (ulonglong) optp->max_value &&
      optp->max_value) /* if max value is not set -> no upper limit */
    num= (ulonglong) optp->max_value;
  if (optp->block_size > 1)
  {
    num/= (ulonglong) optp->block_size;
    num*= (ulonglong) optp->block_size;
  }
  if (num < (ulonglong) optp->min_value)
    num= (ulonglong) optp->min_value;
  return num;
}

/* 
  function: init_variables

  initialize all variables to their default values
*/

static void init_variables(const struct my_option *options)
{
  for (; options->name; options++)
  {
    if (options->value)
    {
      switch (options->var_type) {
      case GET_BOOL:
	if (options->u_max_value)
	  *((my_bool*) options->u_max_value)= (my_bool) options->max_value;
	*((my_bool*) options->value)= (my_bool) options->def_value;
	break;
      case GET_INT:
	if (options->u_max_value)
	  *((int*) options->u_max_value)= (int) options->max_value;
	*((int*) options->value)= (int) options->def_value;
	break;
      case GET_UINT:
	if (options->u_max_value)
	  *((uint*) options->u_max_value)= (uint) options->max_value;
	*((uint*) options->value)= (uint) options->def_value;
	break;
      case GET_LONG:
	if (options->u_max_value)
	  *((long*) options->u_max_value)= (long) options->max_value;
	*((long*) options->value)= (long) options->def_value;
	break;
      case GET_ULONG:
	if (options->u_max_value)
	  *((ulong*) options->u_max_value)= (ulong) options->max_value;
	*((ulong*) options->value)= (ulong) options->def_value;
	break;
      case GET_LL:
	if (options->u_max_value)
	  *((longlong*) options->u_max_value)= (longlong) options->max_value;
	*((longlong*) options->value)= (longlong) options->def_value;
	break;
      case GET_ULL:
	if (options->u_max_value)
	  *((ulonglong*) options->u_max_value)= (ulonglong) options->max_value;
	*((ulonglong*) options->value)=  (ulonglong) options->def_value;
	break;
      default: /* dummy default to avoid compiler warnings */
	break;
      }
    }
  }
}


/*
  function: my_print_options

  Print help for all options and variables.
*/

void my_print_help(const struct my_option *options)
{
  uint col, name_space= 22, comment_space= 57;
  const char *line_end;
  const struct my_option *optp;

  for (optp= options; optp->id; optp++)
  {
    if (optp->id < 256)
    {
      printf("  -%c%s", optp->id, strlen(optp->name) ? ", " : "  ");
      col= 6;
    }
    else
    {
      printf("  ");
      col= 2;
    }
    if (strlen(optp->name))
    {
      printf("--%s", optp->name);
      col+= 2 + strlen(optp->name);
      if (optp->var_type == GET_STR || optp->var_type == GET_STR_ALLOC)
      {
	printf("%s=name%s ", optp->arg_type == OPT_ARG ? "[" : "",
	       optp->arg_type == OPT_ARG ? "]" : "");
	col+= (optp->arg_type == OPT_ARG) ? 8 : 6;
      }
      else if (optp->var_type == GET_NO_ARG || optp->var_type == GET_BOOL)
      {
	putchar(' ');
	col++;
      }
      else
      {
	printf("%s=#%s ", optp->arg_type == OPT_ARG ? "[" : "",
	       optp->arg_type == OPT_ARG ? "]" : "");
	col+= (optp->arg_type == OPT_ARG) ? 5 : 3;
      }
      if (col > name_space && optp->comment && *optp->comment)
      {
	putchar('\n');
	col= 0;
      }
    }
    for (; col < name_space; col++)
      putchar(' ');
    if (optp->comment && *optp->comment)
    {
      const char *comment= optp->comment, *end= strend(comment);

      while ((uint) (end - comment) > comment_space)
      {
	for (line_end= comment + comment_space; *line_end != ' '; line_end--);
	for (; comment != line_end; comment++)
	  putchar(*comment);
	comment++; /* skip the space, as a newline will take it's place now */
	putchar('\n');
	for (col= 0; col < name_space; col++)
	  putchar(' ');
      }
      printf("%s", comment);
    }
    putchar('\n');
  }
}


/*
  function: my_print_options

  Print variables.
*/

void my_print_variables(const struct my_option *options)
{
  uint name_space= 34, length;
  char buff[255];
  const struct my_option *optp;

  printf("\nVariables (--variable-name=value)\n");
  printf("and boolean options {FALSE|TRUE}  Value (after reading options)\n");
  printf("--------------------------------- -----------------------------\n");
  for (optp= options; optp->id; optp++)
  {
    if (optp->value)
    {
      printf("%s", optp->name);
      length= strlen(optp->name);
      for (; length < name_space; length++)
	putchar(' ');
      switch (optp->var_type) {
      case GET_STR:
      case GET_STR_ALLOC:                    /* fall through */
	printf("%s\n", *((char**) optp->value) ? *((char**) optp->value) :
	       "(No default value)");
	break;
      case GET_BOOL:
	printf("%s\n", *((my_bool*) optp->value) ? "TRUE" : "FALSE");
	break;
      case GET_INT:
	printf("%d\n", *((int*) optp->value));
	break;
      case GET_UINT:
	printf("%d\n", *((uint*) optp->value));
	break;
      case GET_LONG:
	printf("%lu\n", *((long*) optp->value));
	break;
      case GET_ULONG:
	printf("%lu\n", *((ulong*) optp->value));
	break;
      case GET_LL:
	printf("%s\n", llstr(*((longlong*) optp->value), buff));
	break;
      case GET_ULL:
	longlong2str(*((ulonglong*) optp->value), buff, 10);
	printf("%s\n", buff);
	break;
      default: /* dummy default to avoid compiler warnings */
	break;
      }
    }
  }
}

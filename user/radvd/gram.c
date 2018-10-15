
/*  A Bison parser, made from gram.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_INTERFACE	257
#define	T_PREFIX	258
#define	STRING	259
#define	NUMBER	260
#define	SIGNEDNUMBER	261
#define	DECIMAL	262
#define	SWITCH	263
#define	IPV6ADDR	264
#define	INFINITY	265
#define	T_AdvSendAdvert	266
#define	T_MaxRtrAdvInterval	267
#define	T_MinRtrAdvInterval	268
#define	T_AdvManagedFlag	269
#define	T_AdvOtherConfigFlag	270
#define	T_AdvLinkMTU	271
#define	T_AdvReachableTime	272
#define	T_AdvRetransTimer	273
#define	T_AdvCurHopLimit	274
#define	T_AdvDefaultLifetime	275
#define	T_AdvSourceLLAddress	276
#define	T_AdvOnLink	277
#define	T_AdvAutonomous	278
#define	T_AdvValidLifetime	279
#define	T_AdvPreferredLifetime	280
#define	T_AdvRouterAddr	281
#define	T_AdvHomeAgentFlag	282
#define	T_AdvIntervalOpt	283
#define	T_AdvHomeAgentInfo	284
#define	T_Base6to4Interface	285
#define	T_UnicastOnly	286
#define	T_HomeAgentPreference	287
#define	T_HomeAgentLifetime	288
#define	T_BAD_TOKEN	289

#line 16 "gram.y"

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

extern struct Interface *IfaceList;
struct Interface *iface = NULL;
struct AdvPrefix *prefix = NULL;

extern char *conf_file;
extern int num_lines;
extern char *yytext;
extern int sock;

static void cleanup(void);
static void yyerror(char *msg);

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ABORT	do { cleanup(); YYABORT; } while (0);


#line 92 "gram.y"
typedef union {
	int			num;
	int			snum;
	double			dec;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		109
#define	YYFLAG		-32768
#define	YYNTBASE	40

#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    39,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    38,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    36,     2,    37,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,    11,    14,    16,    19,    20,    22,    25,
    27,    31,    35,    39,    43,    47,    51,    55,    59,    63,
    67,    71,    75,    79,    83,    87,    91,    95,    99,   103,
   107,   109,   112,   118,   123,   124,   126,   129,   131,   135,
   139,   143,   147,   151,   155,   157
};

static const short yyrhs[] = {    40,
    41,     0,    41,     0,    42,    36,    44,    37,    38,     0,
     3,    43,     0,     5,     0,    45,    48,     0,     0,    46,
     0,    46,    47,     0,    47,     0,    14,     6,    38,     0,
    13,     6,    38,     0,    14,     8,    38,     0,    13,     8,
    38,     0,    12,     9,    38,     0,    15,     9,    38,     0,
    16,     9,    38,     0,    17,     6,    38,     0,    18,     6,
    38,     0,    19,     6,    38,     0,    21,     6,    38,     0,
    20,     6,    38,     0,    22,     9,    38,     0,    29,     9,
    38,     0,    30,     9,    38,     0,    28,     9,    38,     0,
    33,     6,    38,     0,    33,     7,    38,     0,    34,     6,
    38,     0,    32,     9,    38,     0,    49,     0,    48,    49,
     0,    50,    36,    51,    37,    38,     0,     4,    10,    39,
     6,     0,     0,    52,     0,    52,    53,     0,    53,     0,
    23,     9,    38,     0,    24,     9,    38,     0,    27,     9,
    38,     0,    25,    54,    38,     0,    26,    54,    38,     0,
    31,    43,    38,     0,     6,     0,    11,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   104,   105,   108,   144,   159,   166,   172,   173,   176,   177,
   180,   184,   188,   192,   196,   200,   204,   208,   212,   216,
   220,   224,   228,   232,   236,   240,   244,   248,   252,   256,
   262,   266,   273,   304,   327,   328,   330,   331,   334,   338,
   342,   346,   350,   354,   362,   366
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_INTERFACE",
"T_PREFIX","STRING","NUMBER","SIGNEDNUMBER","DECIMAL","SWITCH","IPV6ADDR","INFINITY",
"T_AdvSendAdvert","T_MaxRtrAdvInterval","T_MinRtrAdvInterval","T_AdvManagedFlag",
"T_AdvOtherConfigFlag","T_AdvLinkMTU","T_AdvReachableTime","T_AdvRetransTimer",
"T_AdvCurHopLimit","T_AdvDefaultLifetime","T_AdvSourceLLAddress","T_AdvOnLink",
"T_AdvAutonomous","T_AdvValidLifetime","T_AdvPreferredLifetime","T_AdvRouterAddr",
"T_AdvHomeAgentFlag","T_AdvIntervalOpt","T_AdvHomeAgentInfo","T_Base6to4Interface",
"T_UnicastOnly","T_HomeAgentPreference","T_HomeAgentLifetime","T_BAD_TOKEN",
"'{'","'}'","';'","'/'","grammar","ifacedef","ifacehead","name","ifaceparams",
"optional_ifacevlist","ifacevlist","ifaceval","prefixlist","prefixdef","prefixhead",
"optional_prefixplist","prefixplist","prefixparms","number_or_infinity", NULL
};
#endif

static const short yyr1[] = {     0,
    40,    40,    41,    42,    43,    44,    45,    45,    46,    46,
    47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
    47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
    48,    48,    49,    50,    51,    51,    52,    52,    53,    53,
    53,    53,    53,    53,    54,    54
};

static const short yyr2[] = {     0,
     2,     1,     5,     2,     1,     2,     0,     1,     2,     1,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     1,     2,     5,     4,     0,     1,     2,     1,     3,     3,
     3,     3,     3,     3,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     2,     0,     5,     4,     1,     7,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     8,    10,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     6,    31,     0,     9,    15,    12,    14,    11,    13,
    16,    17,    18,    19,    20,    22,    21,    23,    26,    24,
    25,    30,    27,    28,    29,     3,     0,    32,    35,     0,
     0,     0,     0,     0,     0,     0,     0,    36,    38,    34,
     0,     0,    45,    46,     0,     0,     0,     0,     0,    37,
    39,    40,    42,    43,    41,    44,    33,     0,     0
};

static const short yydefgoto[] = {     2,
     3,     4,     6,    26,    27,    28,    29,    52,    53,    54,
    87,    88,    89,    95
};

static const short yypact[] = {    31,
    24,    12,-32768,    -1,-32768,-32768,-32768,   -12,    27,     5,
    22,    28,    29,    33,    34,    35,    36,    37,    38,    39,
    40,    41,    42,    26,    46,     7,    49,   -12,-32768,    16,
    17,    18,    19,    20,    21,    23,    25,    30,    32,    43,
    44,    45,    47,    48,    50,    51,    52,    53,    54,    55,
    56,    49,-32768,     9,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    57,-32768,     0,    58,
    60,    62,     8,     8,    63,    24,    61,     0,-32768,-32768,
    59,    64,-32768,-32768,    65,    66,    67,    68,    69,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    73,-32768
};

static const short yypgoto[] = {-32768,
    72,-32768,   -40,-32768,-32768,-32768,    71,-32768,    10,-32768,
-32768,-32768,   -28,   -19
};


#define	YYLAST		107


static const short yytable[] = {     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
    31,   108,    32,    93,     1,    20,    21,    22,    94,    23,
    24,    25,    81,    82,    83,    84,    85,    33,     5,    34,
    86,    47,    48,     1,     8,    30,    35,    36,    37,    38,
    39,    40,    41,    50,    79,    98,    42,    43,    44,    45,
    46,    49,    51,    56,    57,    58,    59,    60,    61,   100,
    62,    78,    63,    90,    96,    77,     0,    64,    91,    65,
    92,    97,   109,     7,     0,     0,     0,     0,     0,     0,
    66,    67,    68,     0,    69,    70,     0,    71,    72,    73,
    74,    75,    76,     0,     0,    80,   101,    99,    55,     0,
     0,   102,   103,   104,   105,   106,   107
};

static const short yycheck[] = {    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     6,     0,     8,     6,     3,    28,    29,    30,    11,    32,
    33,    34,    23,    24,    25,    26,    27,     6,     5,     8,
    31,     6,     7,     3,    36,     9,     9,     9,     6,     6,
     6,     6,     6,    37,    36,    86,     9,     9,     9,     9,
     9,     6,     4,    38,    38,    38,    38,    38,    38,    88,
    38,    52,    38,     6,    84,    10,    -1,    38,     9,    38,
     9,     9,     0,     2,    -1,    -1,    -1,    -1,    -1,    -1,
    38,    38,    38,    -1,    38,    38,    -1,    38,    38,    38,
    38,    38,    38,    -1,    -1,    39,    38,    37,    28,    -1,
    -1,    38,    38,    38,    38,    38,    38
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 109 "gram.y"
{
			struct Interface *iface2;

			iface2 = IfaceList;
			while (iface2)
			{
				if (!strcmp(iface2->Name, iface->Name))
				{
					log(LOG_ERR, "duplicate interface "
						"definition for %s", iface->Name);

					ABORT;
				}
				iface2 = iface2->next;
			}			

			if (check_device(sock, iface) < 0)
				ABORT;
			if (setup_deviceinfo(sock, iface) < 0)
				ABORT;
			if (check_iface(iface) < 0)
				ABORT;
			if (setup_linklocal_addr(sock, iface) < 0)
				ABORT;
			if (setup_allrouters_membership(sock, iface) < 0)
				ABORT;

			iface->next = IfaceList;
			IfaceList = iface;

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", iface->Name);

			iface = NULL;
		;
    break;}
case 4:
#line 145 "gram.y"
{
			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				log(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->Name, yyvsp[0].str, IFNAMSIZ-1);
			iface->Name[IFNAMSIZ-1] = '\0';
		;
    break;}
case 5:
#line 160 "gram.y"
{
			/* check vality */
			yyval.str = yyvsp[0].str;
		;
    break;}
case 6:
#line 167 "gram.y"
{
			iface->AdvPrefixList = yyvsp[0].pinfo;
		;
    break;}
case 11:
#line 181 "gram.y"
{
			iface->MinRtrAdvInterval = yyvsp[-1].num;
		;
    break;}
case 12:
#line 185 "gram.y"
{
			iface->MaxRtrAdvInterval = yyvsp[-1].num;
		;
    break;}
case 13:
#line 189 "gram.y"
{
			iface->MinRtrAdvInterval = yyvsp[-1].dec;
		;
    break;}
case 14:
#line 193 "gram.y"
{
			iface->MaxRtrAdvInterval = yyvsp[-1].dec;
		;
    break;}
case 15:
#line 197 "gram.y"
{
			iface->AdvSendAdvert = yyvsp[-1].bool;
		;
    break;}
case 16:
#line 201 "gram.y"
{
			iface->AdvManagedFlag = yyvsp[-1].bool;
		;
    break;}
case 17:
#line 205 "gram.y"
{
			iface->AdvOtherConfigFlag = yyvsp[-1].bool;
		;
    break;}
case 18:
#line 209 "gram.y"
{
			iface->AdvLinkMTU = yyvsp[-1].num;
		;
    break;}
case 19:
#line 213 "gram.y"
{
			iface->AdvReachableTime = yyvsp[-1].num;
		;
    break;}
case 20:
#line 217 "gram.y"
{
			iface->AdvRetransTimer = yyvsp[-1].num;
		;
    break;}
case 21:
#line 221 "gram.y"
{
			iface->AdvDefaultLifetime = yyvsp[-1].num;
		;
    break;}
case 22:
#line 225 "gram.y"
{
			iface->AdvCurHopLimit = yyvsp[-1].num;
		;
    break;}
case 23:
#line 229 "gram.y"
{
			iface->AdvSourceLLAddress = yyvsp[-1].bool;
		;
    break;}
case 24:
#line 233 "gram.y"
{
			iface->AdvIntervalOpt = yyvsp[-1].bool;
		;
    break;}
case 25:
#line 237 "gram.y"
{
			iface->AdvHomeAgentInfo = yyvsp[-1].bool;
		;
    break;}
case 26:
#line 241 "gram.y"
{
			iface->AdvHomeAgentFlag = yyvsp[-1].bool;
		;
    break;}
case 27:
#line 245 "gram.y"
{
			iface->HomeAgentPreference = yyvsp[-1].num;
		;
    break;}
case 28:
#line 249 "gram.y"
{
			iface->HomeAgentPreference = yyvsp[-1].snum;
		;
    break;}
case 29:
#line 253 "gram.y"
{
			iface->HomeAgentLifetime = yyvsp[-1].num;
		;
    break;}
case 30:
#line 257 "gram.y"
{
			iface->UnicastOnly = yyvsp[-1].bool;
		;
    break;}
case 31:
#line 263 "gram.y"
{
			yyval.pinfo = yyvsp[0].pinfo;
		;
    break;}
case 32:
#line 267 "gram.y"
{
			yyvsp[0].pinfo->next = yyvsp[-1].pinfo;
			yyval.pinfo = yyvsp[0].pinfo;
		;
    break;}
case 33:
#line 274 "gram.y"
{
			unsigned int dst;

			if (prefix->AdvPreferredLifetime >
			    prefix->AdvValidLifetime)
			{
				log(LOG_ERR, "AdvValidLifeTime must be "
					"greater than AdvPreferredLifetime in %s, line %d", 
					conf_file, num_lines);
				ABORT;
			}

			if( prefix->if6to4[0] )
			{
				if (get_v4addr(prefix->if6to4, &dst) < 0)
				{
					log(LOG_ERR, "interface %s has no IPv4 addresses, disabling 6to4 prefix", prefix->if6to4 );
					prefix->enabled = 0;
				} else
				{
					*((uint16_t *)(prefix->Prefix.s6_addr)) = htons(0x2002);
					memcpy( prefix->Prefix.s6_addr + 2, &dst, sizeof( dst ) );
				}
			}

			yyval.pinfo = prefix;
			prefix = NULL;
		;
    break;}
case 34:
#line 305 "gram.y"
{
			prefix = malloc(sizeof(struct AdvPrefix));
			
			if (prefix == NULL) {
				log(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if (yyvsp[0].num > MAX_PrefixLen)
			{
				log(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			prefix->PrefixLen = yyvsp[0].num;

			memcpy(&prefix->Prefix, yyvsp[-2].addr, sizeof(struct in6_addr));
		;
    break;}
case 39:
#line 335 "gram.y"
{
			prefix->AdvOnLinkFlag = yyvsp[-1].bool;
		;
    break;}
case 40:
#line 339 "gram.y"
{
			prefix->AdvAutonomousFlag = yyvsp[-1].bool;
		;
    break;}
case 41:
#line 343 "gram.y"
{
			prefix->AdvRouterAddr = yyvsp[-1].bool;
		;
    break;}
case 42:
#line 347 "gram.y"
{
			prefix->AdvValidLifetime = yyvsp[-1].num;
		;
    break;}
case 43:
#line 351 "gram.y"
{
			prefix->AdvPreferredLifetime = yyvsp[-1].num;
		;
    break;}
case 44:
#line 355 "gram.y"
{
			dlog(LOG_DEBUG, 4, "using interface %s for 6to4", yyvsp[-1].str);
			strncpy(prefix->if6to4, yyvsp[-1].str, IFNAMSIZ-1);
			prefix->if6to4[IFNAMSIZ-1] = '\0';
		;
    break;}
case 45:
#line 363 "gram.y"
{
                                yyval.num = yyvsp[0].num; 
                        ;
    break;}
case 46:
#line 367 "gram.y"
{
                                yyval.num = (uint32_t)~0;
                        ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 372 "gram.y"


static
void cleanup(void)
{
	if (iface)
		free(iface);
	
	if (prefix)
		free(prefix);
}

static void
yyerror(char *msg)
{
	cleanup();
	log(LOG_ERR, "%s in %s, line %d: %s", msg, conf_file, num_lines, yytext);
}

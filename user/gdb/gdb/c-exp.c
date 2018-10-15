/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     FLOAT = 259,
     DECFLOAT = 260,
     STRING = 261,
     NAME = 262,
     TYPENAME = 263,
     NAME_OR_INT = 264,
     STRUCT = 265,
     CLASS = 266,
     UNION = 267,
     ENUM = 268,
     SIZEOF = 269,
     UNSIGNED = 270,
     COLONCOLON = 271,
     TEMPLATE = 272,
     ERROR = 273,
     SIGNED_KEYWORD = 274,
     LONG = 275,
     SHORT = 276,
     INT_KEYWORD = 277,
     CONST_KEYWORD = 278,
     VOLATILE_KEYWORD = 279,
     DOUBLE_KEYWORD = 280,
     VARIABLE = 281,
     ASSIGN_MODIFY = 282,
     TRUEKEYWORD = 283,
     FALSEKEYWORD = 284,
     ABOVE_COMMA = 285,
     OROR = 286,
     ANDAND = 287,
     NOTEQUAL = 288,
     EQUAL = 289,
     GEQ = 290,
     LEQ = 291,
     RSH = 292,
     LSH = 293,
     DECREMENT = 294,
     INCREMENT = 295,
     UNARY = 296,
     ARROW = 297,
     BLOCKNAME = 298,
     FILENAME = 299
   };
#endif
#define INT 258
#define FLOAT 259
#define DECFLOAT 260
#define STRING 261
#define NAME 262
#define TYPENAME 263
#define NAME_OR_INT 264
#define STRUCT 265
#define CLASS 266
#define UNION 267
#define ENUM 268
#define SIZEOF 269
#define UNSIGNED 270
#define COLONCOLON 271
#define TEMPLATE 272
#define ERROR 273
#define SIGNED_KEYWORD 274
#define LONG 275
#define SHORT 276
#define INT_KEYWORD 277
#define CONST_KEYWORD 278
#define VOLATILE_KEYWORD 279
#define DOUBLE_KEYWORD 280
#define VARIABLE 281
#define ASSIGN_MODIFY 282
#define TRUEKEYWORD 283
#define FALSEKEYWORD 284
#define ABOVE_COMMA 285
#define OROR 286
#define ANDAND 287
#define NOTEQUAL 288
#define EQUAL 289
#define GEQ 290
#define LEQ 291
#define RSH 292
#define LSH 293
#define DECREMENT 294
#define INCREMENT 295
#define UNARY 296
#define ARROW 297
#define BLOCKNAME 298
#define FILENAME 299




/* Copy the first part of user declarations.  */
#line 40 "c-exp.y"


#include "defs.h"
#include "gdb_string.h"
#include <ctype.h>
#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "c-lang.h"
#include "bfd.h" /* Required by objfiles.h.  */
#include "symfile.h" /* Required by objfiles.h.  */
#include "objfiles.h" /* For have_full_symbols and have_partial_symbols */
#include "charset.h"
#include "block.h"
#include "cp-support.h"
#include "dfp.h"

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in gdb.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list. */

#define	yymaxdepth c_maxdepth
#define	yyparse	c_parse
#define	yylex	c_lex
#define	yyerror	c_error
#define	yylval	c_lval
#define	yychar	c_char
#define	yydebug	c_debug
#define	yypact	c_pact	
#define	yyr1	c_r1			
#define	yyr2	c_r2			
#define	yydef	c_def		
#define	yychk	c_chk		
#define	yypgo	c_pgo		
#define	yyact	c_act		
#define	yyexca	c_exca
#define yyerrflag c_errflag
#define yynerrs	c_nerrs
#define	yyps	c_ps
#define	yypv	c_pv
#define	yys	c_s
#define	yy_yys	c_yys
#define	yystate	c_state
#define	yytmp	c_tmp
#define	yyv	c_v
#define	yy_yyv	c_yyv
#define	yyval	c_val
#define	yylloc	c_lloc
#define yyreds	c_reds		/* With YYDEBUG defined */
#define yytoks	c_toks		/* With YYDEBUG defined */
#define yyname	c_name		/* With YYDEBUG defined */
#define yyrule	c_rule		/* With YYDEBUG defined */
#define yylhs	c_yylhs
#define yylen	c_yylen
#define yydefred c_yydefred
#define yydgoto	c_yydgoto
#define yysindex c_yysindex
#define yyrindex c_yyrindex
#define yygindex c_yygindex
#define yytable	 c_yytable
#define yycheck	 c_yycheck

#ifndef YYDEBUG
#define	YYDEBUG 1		/* Default to yydebug support */
#endif

#define YYFPRINTF parser_fprintf

int yyparse (void);

static int yylex (void);

void yyerror (char *);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 125 "c-exp.y"
typedef union YYSTYPE {
    LONGEST lval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val_int;
    struct {
      DOUBLEST dval;
      struct type *type;
    } typed_val_float;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_decfloat;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct type **tvec;
    int *ivec;
  } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 272 "c-exp.c.tmp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 153 "c-exp.y"

/* YYSTYPE gets defined by %union */
static int parse_number (char *, int, int, YYSTYPE *);


/* Line 214 of yacc.c.  */
#line 288 "c-exp.c.tmp"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC xmalloc
# endif

/* The parser invokes alloca or xmalloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  97
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   771

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  31
/* YYNRULES -- Number of rules. */
#define YYNRULES  160
/* YYNRULES -- Number of states. */
#define YYNSTATES  242

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    62,     2,     2,     2,    52,    38,     2,
      58,    65,    50,    48,    30,    49,    56,    51,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    68,     2,
      41,    32,    42,    33,    47,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    57,     2,    64,    37,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    66,    36,    67,    63,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    31,    34,    35,    39,    40,
      43,    44,    45,    46,    53,    54,    55,    59,    60,    61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    15,    18,    21,
      24,    27,    30,    33,    36,    39,    42,    45,    48,    52,
      56,    61,    65,    69,    74,    79,    80,    86,    88,    89,
      91,    95,    97,   101,   106,   111,   115,   119,   123,   127,
     131,   135,   139,   143,   147,   151,   155,   159,   163,   167,
     171,   175,   179,   183,   187,   191,   197,   201,   205,   207,
     209,   211,   213,   215,   217,   222,   224,   226,   228,   230,
     232,   236,   240,   244,   249,   251,   254,   256,   259,   261,
     262,   266,   268,   270,   272,   273,   275,   278,   280,   283,
     285,   289,   292,   294,   297,   299,   302,   306,   309,   313,
     315,   317,   319,   321,   323,   326,   330,   333,   337,   341,
     345,   348,   351,   355,   360,   364,   368,   373,   377,   382,
     386,   391,   394,   398,   401,   405,   408,   412,   414,   417,
     420,   423,   426,   429,   432,   434,   437,   439,   445,   448,
     451,   453,   457,   459,   461,   463,   465,   467,   471,   473,
     478,   481,   484,   486,   488,   490,   492,   494,   496,   498,
     500
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      70,     0,    -1,    72,    -1,    71,    -1,    90,    -1,    73,
      -1,    72,    30,    73,    -1,    50,    73,    -1,    38,    73,
      -1,    49,    73,    -1,    48,    73,    -1,    62,    73,    -1,
      63,    73,    -1,    54,    73,    -1,    53,    73,    -1,    73,
      54,    -1,    73,    53,    -1,    14,    73,    -1,    73,    59,
      98,    -1,    73,    59,    80,    -1,    73,    59,    50,    73,
      -1,    73,    56,    98,    -1,    73,    56,    80,    -1,    73,
      56,    50,    73,    -1,    73,    57,    72,    64,    -1,    -1,
      73,    58,    74,    76,    65,    -1,    66,    -1,    -1,    73,
      -1,    76,    30,    73,    -1,    67,    -1,    75,    76,    77,
      -1,    75,    90,    77,    73,    -1,    58,    90,    65,    73,
      -1,    58,    72,    65,    -1,    73,    47,    73,    -1,    73,
      50,    73,    -1,    73,    51,    73,    -1,    73,    52,    73,
      -1,    73,    48,    73,    -1,    73,    49,    73,    -1,    73,
      46,    73,    -1,    73,    45,    73,    -1,    73,    40,    73,
      -1,    73,    39,    73,    -1,    73,    44,    73,    -1,    73,
      43,    73,    -1,    73,    41,    73,    -1,    73,    42,    73,
      -1,    73,    38,    73,    -1,    73,    37,    73,    -1,    73,
      36,    73,    -1,    73,    35,    73,    -1,    73,    34,    73,
      -1,    73,    33,    73,    68,    73,    -1,    73,    32,    73,
      -1,    73,    27,    73,    -1,     3,    -1,     9,    -1,     4,
      -1,     5,    -1,    79,    -1,    26,    -1,    14,    58,    90,
      65,    -1,     6,    -1,    28,    -1,    29,    -1,    60,    -1,
      61,    -1,    78,    16,    98,    -1,    78,    16,    98,    -1,
      91,    16,    98,    -1,    91,    16,    63,    98,    -1,    80,
      -1,    16,    98,    -1,    99,    -1,    47,     7,    -1,    97,
      -1,    -1,    82,    81,    82,    -1,    83,    -1,    97,    -1,
      84,    -1,    -1,    50,    -1,    50,    86,    -1,    38,    -1,
      38,    86,    -1,    87,    -1,    58,    86,    65,    -1,    87,
      88,    -1,    88,    -1,    87,    89,    -1,    89,    -1,    57,
      64,    -1,    57,     3,    64,    -1,    58,    65,    -1,    58,
      94,    65,    -1,    95,    -1,     8,    -1,    22,    -1,    20,
      -1,    21,    -1,    20,    22,    -1,    20,    19,    22,    -1,
      20,    19,    -1,    19,    20,    22,    -1,    15,    20,    22,
      -1,    20,    15,    22,    -1,    20,    15,    -1,    20,    20,
      -1,    20,    20,    22,    -1,    20,    20,    19,    22,    -1,
      20,    20,    19,    -1,    19,    20,    20,    -1,    19,    20,
      20,    22,    -1,    15,    20,    20,    -1,    15,    20,    20,
      22,    -1,    20,    20,    15,    -1,    20,    20,    15,    22,
      -1,    21,    22,    -1,    21,    19,    22,    -1,    21,    19,
      -1,    15,    21,    22,    -1,    21,    15,    -1,    21,    15,
      22,    -1,    25,    -1,    20,    25,    -1,    10,    98,    -1,
      11,    98,    -1,    12,    98,    -1,    13,    98,    -1,    15,
      93,    -1,    15,    -1,    19,    93,    -1,    19,    -1,    17,
      98,    41,    90,    42,    -1,    84,    91,    -1,    91,    84,
      -1,    92,    -1,    91,    16,    98,    -1,     8,    -1,    22,
      -1,    20,    -1,    21,    -1,    90,    -1,    94,    30,    90,
      -1,    91,    -1,    95,    85,    86,    85,    -1,    23,    24,
      -1,    24,    23,    -1,    96,    -1,    23,    -1,    24,    -1,
       7,    -1,    60,    -1,     8,    -1,     9,    -1,     7,    -1,
      60,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   237,   237,   238,   241,   248,   249,   254,   258,   262,
     266,   270,   274,   278,   282,   286,   290,   294,   298,   304,
     312,   316,   322,   330,   334,   341,   338,   348,   352,   355,
     359,   363,   366,   373,   379,   385,   391,   395,   399,   403,
     407,   411,   415,   419,   423,   427,   431,   435,   439,   443,
     447,   451,   455,   459,   463,   467,   471,   475,   481,   488,
     499,   506,   513,   516,   520,   528,   553,   560,   569,   577,
     583,   594,   610,   624,   649,   650,   684,   741,   747,   748,
     751,   754,   755,   759,   760,   763,   765,   767,   769,   771,
     774,   776,   781,   788,   790,   794,   796,   800,   802,   814,
     818,   820,   822,   824,   826,   828,   830,   832,   834,   836,
     838,   840,   842,   844,   846,   848,   850,   852,   854,   856,
     858,   860,   862,   864,   866,   868,   870,   872,   874,   876,
     879,   882,   885,   888,   890,   892,   894,   899,   903,   905,
     907,   955,   980,   981,   987,   993,  1002,  1007,  1014,  1015,
    1019,  1020,  1023,  1027,  1029,  1033,  1034,  1035,  1036,  1039,
    1040
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "DECFLOAT", "STRING",
  "NAME", "TYPENAME", "NAME_OR_INT", "STRUCT", "CLASS", "UNION", "ENUM",
  "SIZEOF", "UNSIGNED", "COLONCOLON", "TEMPLATE", "ERROR",
  "SIGNED_KEYWORD", "LONG", "SHORT", "INT_KEYWORD", "CONST_KEYWORD",
  "VOLATILE_KEYWORD", "DOUBLE_KEYWORD", "VARIABLE", "ASSIGN_MODIFY",
  "TRUEKEYWORD", "FALSEKEYWORD", "','", "ABOVE_COMMA", "'='", "'?'",
  "OROR", "ANDAND", "'|'", "'^'", "'&'", "NOTEQUAL", "EQUAL", "'<'", "'>'",
  "GEQ", "LEQ", "RSH", "LSH", "'@'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "DECREMENT", "INCREMENT", "UNARY", "'.'", "'['", "'('", "ARROW",
  "BLOCKNAME", "FILENAME", "'!'", "'~'", "']'", "')'", "'{'", "'}'", "':'",
  "$accept", "start", "type_exp", "exp1", "exp", "@1", "lcurly", "arglist",
  "rcurly", "block", "variable", "qualified_name", "space_identifier",
  "const_or_volatile", "cv_with_space_id",
  "const_or_volatile_or_space_identifier_noopt",
  "const_or_volatile_or_space_identifier", "abs_decl", "direct_abs_decl",
  "array_mod", "func_mod", "type", "typebase", "qualified_type",
  "typename", "nonempty_typelist", "ptype", "const_and_volatile",
  "const_or_volatile_noopt", "name", "name_not_typename", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
      44,   285,    61,    63,   286,   287,   124,    94,    38,   288,
     289,    60,    62,   290,   291,   292,   293,    64,    43,    45,
      42,    47,    37,   294,   295,   296,    46,    91,    40,   297,
     298,   299,    33,   126,    93,    41,   123,   125,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    69,    70,    70,    71,    72,    72,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    74,    73,    75,    76,    76,
      76,    77,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    78,    78,
      78,    79,    80,    80,    79,    79,    79,    81,    82,    82,
      83,    84,    84,    85,    85,    86,    86,    86,    86,    86,
      87,    87,    87,    87,    87,    88,    88,    89,    89,    90,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    92,    93,    93,    93,    93,    94,    94,    95,    95,
      96,    96,    97,    97,    97,    98,    98,    98,    98,    99,
      99
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     3,
       4,     3,     3,     4,     4,     0,     5,     1,     0,     1,
       3,     1,     3,     4,     4,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     1,     1,
       3,     3,     3,     4,     1,     2,     1,     2,     1,     0,
       3,     1,     1,     1,     0,     1,     2,     1,     2,     1,
       3,     2,     1,     2,     1,     2,     3,     2,     3,     1,
       1,     1,     1,     1,     2,     3,     2,     3,     3,     3,
       2,     2,     3,     4,     3,     3,     4,     3,     4,     3,
       4,     2,     3,     2,     3,     2,     3,     1,     2,     2,
       2,     2,     2,     2,     1,     2,     1,     5,     2,     2,
       1,     3,     1,     1,     1,     1,     1,     3,     1,     4,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      79,    58,    60,    61,    65,   159,   100,    59,     0,     0,
       0,     0,    79,   134,     0,     0,   136,   102,   103,   101,
     153,   154,   127,    63,    66,    67,    79,    79,    79,    79,
      79,    79,    79,   160,    69,    79,    79,    27,     0,     3,
       2,     5,    28,     0,    62,    74,     0,    81,    79,     4,
     148,   140,    99,   152,    82,    76,   155,   157,   158,   156,
     129,   130,   131,   132,    79,    17,    79,   142,   144,   145,
     143,   133,    75,     0,   144,   145,   135,   110,   106,   111,
     104,   128,   125,   123,   121,   150,   151,     8,    10,     9,
       7,    14,    13,     0,     0,    11,    12,     1,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    16,    15,    79,    79,    25,    79,    29,     0,     0,
       0,     0,    79,   138,     0,   139,    83,     0,     0,   117,
     108,   124,    79,   115,   107,   109,   105,   119,   114,   112,
     126,   122,    35,    79,     6,    57,    56,     0,    54,    53,
      52,    51,    50,    45,    44,    48,    49,    47,    46,    43,
      42,    36,    40,    41,    37,    38,    39,   157,    79,    22,
      21,     0,    28,    79,    19,    18,    79,    31,    32,    79,
      71,    77,    80,    78,     0,     0,    72,    87,    85,     0,
      79,    84,    89,    92,    94,    64,   118,     0,   148,   116,
     120,   113,    34,    79,    23,    24,     0,    20,    30,    33,
     141,    73,    88,    86,     0,    95,    97,     0,   146,     0,
     149,    79,    91,    93,   137,    55,    26,    96,    90,    79,
      98,   147
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    38,    39,    93,    41,   182,    42,   128,   188,    43,
      44,    45,   132,    46,    47,    48,   137,   201,   202,   203,
     204,   228,    66,    51,    71,   229,    52,    53,    54,    60,
      55
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -142
static const short yypact[] =
{
     268,  -142,  -142,  -142,  -142,  -142,  -142,  -142,     7,     7,
       7,     7,   332,    33,     7,     7,    64,   116,   125,  -142,
     -11,    13,  -142,  -142,  -142,  -142,   268,   268,   268,   268,
     268,   268,   268,    24,  -142,   268,   268,  -142,    46,  -142,
      20,   529,   204,    72,  -142,  -142,    68,  -142,   290,  -142,
     105,  -142,   443,  -142,    73,  -142,  -142,  -142,  -142,  -142,
    -142,  -142,  -142,  -142,   268,   438,    15,  -142,    54,   102,
    -142,  -142,  -142,    93,    67,  -142,  -142,   115,   117,   219,
    -142,  -142,   132,   147,  -142,  -142,  -142,   438,   438,   438,
     438,   438,   438,   -21,   101,   438,   438,  -142,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,  -142,  -142,   419,   268,  -142,   463,   529,   -22,   106,
       7,   175,     6,   109,    53,  -142,  -142,   127,   118,   165,
    -142,  -142,   290,   167,  -142,  -142,  -142,   170,   176,  -142,
    -142,  -142,  -142,   268,   529,   529,   529,   492,   581,   605,
     628,   650,   671,   690,   690,   458,   458,   458,   458,   315,
     315,   702,   712,   712,   438,   438,   438,   221,   268,  -142,
    -142,    -6,   204,   268,  -142,  -142,   268,  -142,  -142,   268,
     183,  -142,  -142,  -142,     7,     7,   148,   144,   110,    -1,
     138,     2,   122,  -142,  -142,   396,  -142,   162,   109,  -142,
    -142,  -142,   438,   268,   438,  -142,   -18,   438,   529,   438,
    -142,  -142,  -142,  -142,   142,  -142,  -142,   157,  -142,   -13,
    -142,    58,  -142,  -142,  -142,   556,  -142,  -142,  -142,   290,
    -142,  -142
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -142,  -142,  -142,     3,    -8,  -142,  -142,    61,   107,  -142,
    -142,    -9,  -142,   103,  -142,   -15,    45,  -141,  -142,    47,
      48,     1,     0,  -142,   231,  -142,  -142,  -142,   123,    -4,
    -142
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -142
static const short yytable[] =
{
      50,    49,   224,    40,    65,    61,    62,    63,   186,    98,
      72,    73,   186,    85,    56,    57,    58,   239,    87,    88,
      89,    90,    91,    92,    98,    20,    21,    95,    96,    20,
      21,   134,    50,    94,   127,   135,    86,   136,    20,    21,
     -68,    67,    50,   129,   152,   187,    97,   236,   133,   -79,
      98,   135,   240,    68,    69,    70,   222,   223,   215,   227,
      56,    57,    58,   225,    50,   138,     6,    59,     8,     9,
      10,    11,    67,    13,   139,    15,   140,    16,    17,    18,
      19,    20,    21,    22,    74,    75,    70,   143,   130,   144,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,    59,   179,   131,   195,   184,   135,   180,
     -78,   134,   185,   226,   141,   194,   190,   181,    20,    21,
     196,    77,    20,    21,   142,    78,    79,   145,    80,   146,
      82,    81,   208,   207,    83,   212,     6,    84,     8,     9,
      10,    11,   -79,    13,   150,    15,   -79,    16,    17,    18,
      19,    20,    21,    22,  -141,   197,   153,   199,   200,   151,
     214,  -141,  -141,   187,   127,   217,   197,   198,   218,   199,
     231,   219,   191,   205,   199,   200,   136,   206,   198,   209,
     220,   221,   210,   135,   198,   199,   200,   212,   211,   -70,
     208,   199,   200,   226,   234,   235,   237,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,   238,    16,    17,    18,    19,    20,    21,    22,
      23,   208,    24,    25,   147,   192,   189,  -100,   148,   208,
     241,   149,    26,   216,  -100,  -100,   230,    76,     0,   232,
     233,   -79,    27,    28,    29,   193,     0,    30,    31,     0,
       0,     0,    32,     0,    33,    34,    35,    36,  -100,     0,
      37,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,     0,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,    25,     6,     0,
       8,     9,    10,    11,     0,    13,    26,    15,     0,    16,
      17,    18,    19,    20,    21,    22,    27,    28,    29,     0,
       0,    30,    31,     0,     0,     0,    32,     0,    33,    34,
      35,    36,     0,     0,    37,     1,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
       0,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,   115,   116,   117,   118,   119,   120,   121,   122,
      26,   123,   124,   125,   126,     0,     0,     0,     0,     0,
      27,    28,    29,     0,     0,    30,    31,     0,     0,     0,
      64,     0,    33,    34,    35,    36,     0,     0,    37,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,     0,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,    25,    56,   177,    58,     8,
       9,    10,    11,     0,    13,     0,    15,     0,    16,    17,
      18,    19,    20,    21,    22,     0,     0,     0,     0,    30,
      31,     0,     0,     0,    32,     0,    33,    34,    35,    36,
       0,     0,    37,     0,     0,     0,    20,    21,     0,   178,
      56,   177,    58,     8,     9,    10,    11,     0,    13,    59,
      15,   -84,    16,    17,    18,    19,    20,    21,    22,     0,
     -79,   121,   122,   -84,   123,   124,   125,   126,     0,     0,
     -84,   -84,     0,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   183,   123,   124,   125,   126,     0,    99,
       0,     0,     0,    59,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,     0,   123,   124,
     125,   126,     0,     0,     0,     0,    99,     0,     0,     0,
     213,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,     0,   123,   124,   125,   126,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,     0,   123,   124,   125,   126,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,     0,   123,   124,   125,
     126,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
       0,   123,   124,   125,   126,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     0,   123,   124,   125,   126,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,   123,   124,   125,   126,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,     0,   123,   124,   125,
     126,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,   123,   124,   125,   126,
     116,   117,   118,   119,   120,   121,   122,     0,   123,   124,
     125,   126,   118,   119,   120,   121,   122,     0,   123,   124,
     125,   126
};

static const short yycheck[] =
{
       0,     0,     3,     0,    12,     9,    10,    11,    30,    30,
      14,    15,    30,    24,     7,     8,     9,    30,    26,    27,
      28,    29,    30,    31,    30,    23,    24,    35,    36,    23,
      24,    16,    32,    32,    42,    50,    23,    52,    23,    24,
      16,     8,    42,    42,    65,    67,     0,    65,    48,    47,
      30,    66,    65,    20,    21,    22,   197,   198,    64,   200,
       7,     8,     9,    64,    64,    64,     8,    60,    10,    11,
      12,    13,     8,    15,    20,    17,    22,    19,    20,    21,
      22,    23,    24,    25,    20,    21,    22,    20,    16,    22,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    60,   123,    47,    63,   126,   133,   123,
      47,    16,   126,    65,    22,    16,   130,   124,    23,    24,
     134,    15,    23,    24,    41,    19,    20,    22,    22,    22,
      15,    25,   142,   142,    19,   153,     8,    22,    10,    11,
      12,    13,    47,    15,    22,    17,    47,    19,    20,    21,
      22,    23,    24,    25,    16,    38,    65,    57,    58,    22,
     178,    23,    24,    67,   182,   183,    38,    50,   186,    57,
      58,   189,     7,    65,    57,    58,   201,    22,    50,    22,
     194,   195,    22,   208,    50,    57,    58,   205,    22,    16,
     200,    57,    58,    65,    42,   213,    64,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    65,    19,    20,    21,    22,    23,    24,    25,
      26,   231,    28,    29,    15,   132,   129,    16,    19,   239,
     239,    22,    38,   182,    23,    24,   201,    16,    -1,   202,
     202,    47,    48,    49,    50,   132,    -1,    53,    54,    -1,
      -1,    -1,    58,    -1,    60,    61,    62,    63,    47,    -1,
      66,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,     8,    -1,
      10,    11,    12,    13,    -1,    15,    38,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    48,    49,    50,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,    -1,    60,    61,
      62,    63,    -1,    -1,    66,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    47,    48,    49,    50,    51,    52,    53,    54,
      38,    56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      48,    49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,
      58,    -1,    60,    61,    62,    63,    -1,    -1,    66,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,     7,     8,     9,    10,
      11,    12,    13,    -1,    15,    -1,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    -1,    60,    61,    62,    63,
      -1,    -1,    66,    -1,    -1,    -1,    23,    24,    -1,    50,
       7,     8,     9,    10,    11,    12,    13,    -1,    15,    60,
      17,    38,    19,    20,    21,    22,    23,    24,    25,    -1,
      47,    53,    54,    50,    56,    57,    58,    59,    -1,    -1,
      57,    58,    -1,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    50,    56,    57,    58,    59,    -1,    27,
      -1,    -1,    -1,    60,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      68,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    57,    58,
      59,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    57,    58,    59,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    57,    58,
      59,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    19,    20,    21,    22,
      23,    24,    25,    26,    28,    29,    38,    48,    49,    50,
      53,    54,    58,    60,    61,    62,    63,    66,    70,    71,
      72,    73,    75,    78,    79,    80,    82,    83,    84,    90,
      91,    92,    95,    96,    97,    99,     7,     8,     9,    60,
      98,    98,    98,    98,    58,    73,    91,     8,    20,    21,
      22,    93,    98,    98,    20,    21,    93,    15,    19,    20,
      22,    25,    15,    19,    22,    24,    23,    73,    73,    73,
      73,    73,    73,    72,    90,    73,    73,     0,    30,    27,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    56,    57,    58,    59,    73,    76,    90,
      16,    47,    81,    91,    16,    84,    84,    85,    90,    20,
      22,    22,    41,    20,    22,    22,    22,    15,    19,    22,
      22,    22,    65,    65,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,     8,    50,    80,
      98,    72,    74,    50,    80,    98,    30,    67,    77,    77,
      98,     7,    82,    97,    16,    63,    98,    38,    50,    57,
      58,    86,    87,    88,    89,    65,    22,    90,    91,    22,
      22,    22,    73,    68,    73,    64,    76,    73,    73,    73,
      98,    98,    86,    86,     3,    64,    65,    86,    90,    94,
      85,    58,    88,    89,    42,    73,    65,    64,    65,    30,
      65,    90
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to xreallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to xreallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 242 "c-exp.y"
    { write_exp_elt_opcode(OP_TYPE);
			  write_exp_elt_type(yyvsp[0].tval);
			  write_exp_elt_opcode(OP_TYPE);}
    break;

  case 6:
#line 250 "c-exp.y"
    { write_exp_elt_opcode (BINOP_COMMA); }
    break;

  case 7:
#line 255 "c-exp.y"
    { write_exp_elt_opcode (UNOP_IND); }
    break;

  case 8:
#line 259 "c-exp.y"
    { write_exp_elt_opcode (UNOP_ADDR); }
    break;

  case 9:
#line 263 "c-exp.y"
    { write_exp_elt_opcode (UNOP_NEG); }
    break;

  case 10:
#line 267 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PLUS); }
    break;

  case 11:
#line 271 "c-exp.y"
    { write_exp_elt_opcode (UNOP_LOGICAL_NOT); }
    break;

  case 12:
#line 275 "c-exp.y"
    { write_exp_elt_opcode (UNOP_COMPLEMENT); }
    break;

  case 13:
#line 279 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREINCREMENT); }
    break;

  case 14:
#line 283 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREDECREMENT); }
    break;

  case 15:
#line 287 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTINCREMENT); }
    break;

  case 16:
#line 291 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTDECREMENT); }
    break;

  case 17:
#line 295 "c-exp.y"
    { write_exp_elt_opcode (UNOP_SIZEOF); }
    break;

  case 18:
#line 299 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 19:
#line 305 "c-exp.y"
    { /* exp->type::name becomes exp->*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 20:
#line 313 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 21:
#line 317 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 22:
#line 323 "c-exp.y"
    { /* exp.type::name becomes exp.*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 23:
#line 331 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 24:
#line 335 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUBSCRIPT); }
    break;

  case 25:
#line 341 "c-exp.y"
    { start_arglist (); }
    break;

  case 26:
#line 343 "c-exp.y"
    { write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL); }
    break;

  case 27:
#line 349 "c-exp.y"
    { start_arglist (); }
    break;

  case 29:
#line 356 "c-exp.y"
    { arglist_len = 1; }
    break;

  case 30:
#line 360 "c-exp.y"
    { arglist_len++; }
    break;

  case 31:
#line 364 "c-exp.y"
    { yyval.lval = end_arglist () - 1; }
    break;

  case 32:
#line 367 "c-exp.y"
    { write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_ARRAY); }
    break;

  case 33:
#line 374 "c-exp.y"
    { write_exp_elt_opcode (UNOP_MEMVAL);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_MEMVAL); }
    break;

  case 34:
#line 380 "c-exp.y"
    { write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;

  case 35:
#line 386 "c-exp.y"
    { }
    break;

  case 36:
#line 392 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REPEAT); }
    break;

  case 37:
#line 396 "c-exp.y"
    { write_exp_elt_opcode (BINOP_MUL); }
    break;

  case 38:
#line 400 "c-exp.y"
    { write_exp_elt_opcode (BINOP_DIV); }
    break;

  case 39:
#line 404 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REM); }
    break;

  case 40:
#line 408 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ADD); }
    break;

  case 41:
#line 412 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUB); }
    break;

  case 42:
#line 416 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LSH); }
    break;

  case 43:
#line 420 "c-exp.y"
    { write_exp_elt_opcode (BINOP_RSH); }
    break;

  case 44:
#line 424 "c-exp.y"
    { write_exp_elt_opcode (BINOP_EQUAL); }
    break;

  case 45:
#line 428 "c-exp.y"
    { write_exp_elt_opcode (BINOP_NOTEQUAL); }
    break;

  case 46:
#line 432 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LEQ); }
    break;

  case 47:
#line 436 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GEQ); }
    break;

  case 48:
#line 440 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LESS); }
    break;

  case 49:
#line 444 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GTR); }
    break;

  case 50:
#line 448 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_AND); }
    break;

  case 51:
#line 452 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_XOR); }
    break;

  case 52:
#line 456 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_IOR); }
    break;

  case 53:
#line 460 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_AND); }
    break;

  case 54:
#line 464 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_OR); }
    break;

  case 55:
#line 468 "c-exp.y"
    { write_exp_elt_opcode (TERNOP_COND); }
    break;

  case 56:
#line 472 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN); }
    break;

  case 57:
#line 476 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode (yyvsp[-1].opcode);
			  write_exp_elt_opcode (BINOP_ASSIGN_MODIFY); }
    break;

  case 58:
#line 482 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (yyvsp[0].typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)(yyvsp[0].typed_val_int.val));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 59:
#line 489 "c-exp.y"
    { YYSTYPE val;
			  parse_number (yyvsp[0].ssym.stoken.ptr, yyvsp[0].ssym.stoken.length, 0, &val);
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (val.typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)val.typed_val_int.val);
			  write_exp_elt_opcode (OP_LONG);
			}
    break;

  case 60:
#line 500 "c-exp.y"
    { write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (yyvsp[0].typed_val_float.type);
			  write_exp_elt_dblcst (yyvsp[0].typed_val_float.dval);
			  write_exp_elt_opcode (OP_DOUBLE); }
    break;

  case 61:
#line 507 "c-exp.y"
    { write_exp_elt_opcode (OP_DECFLOAT);
			  write_exp_elt_type (yyvsp[0].typed_val_decfloat.type);
			  write_exp_elt_decfloatcst (yyvsp[0].typed_val_decfloat.val);
			  write_exp_elt_opcode (OP_DECFLOAT); }
    break;

  case 64:
#line 521 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type (current_gdbarch)->builtin_int);
			  CHECK_TYPEDEF (yyvsp[-1].tval);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 65:
#line 529 "c-exp.y"
    { /* C strings are converted into array constants with
			     an explicit null byte added at the end.  Thus
			     the array upper bound is the string length.
			     There is no such thing in C as a completely empty
			     string. */
			  char *sp = yyvsp[0].sval.ptr; int count = yyvsp[0].sval.length;
			  while (count-- > 0)
			    {
			      write_exp_elt_opcode (OP_LONG);
			      write_exp_elt_type (builtin_type (current_gdbarch)->builtin_char);
			      write_exp_elt_longcst ((LONGEST)(*sp++));
			      write_exp_elt_opcode (OP_LONG);
			    }
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type (current_gdbarch)->builtin_char);
			  write_exp_elt_longcst ((LONGEST)'\0');
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) (yyvsp[0].sval.length));
			  write_exp_elt_opcode (OP_ARRAY); }
    break;

  case 66:
#line 554 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (builtin_type (current_gdbarch)->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 1);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 67:
#line 561 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (builtin_type (current_gdbarch)->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 0);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 68:
#line 570 "c-exp.y"
    {
			  if (yyvsp[0].ssym.sym)
			    yyval.bval = SYMBOL_BLOCK_VALUE (yyvsp[0].ssym.sym);
			  else
			    error ("No file or function \"%s\".",
				   copy_name (yyvsp[0].ssym.stoken));
			}
    break;

  case 69:
#line 578 "c-exp.y"
    {
			  yyval.bval = yyvsp[0].bval;
			}
    break;

  case 70:
#line 584 "c-exp.y"
    { struct symbol *tem
			    = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					     VAR_DOMAIN, (int *) NULL,
					     (struct symtab **) NULL);
			  if (!tem || SYMBOL_CLASS (tem) != LOC_BLOCK)
			    error ("No function \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));
			  yyval.bval = SYMBOL_BLOCK_VALUE (tem); }
    break;

  case 71:
#line 595 "c-exp.y"
    { struct symbol *sym;
			  sym = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					       VAR_DOMAIN, (int *) NULL,
					       (struct symtab **) NULL);
			  if (sym == 0)
			    error ("No symbol \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));

			  write_exp_elt_opcode (OP_VAR_VALUE);
			  /* block_found is set by lookup_symbol.  */
			  write_exp_elt_block (block_found);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); }
    break;

  case 72:
#line 611 "c-exp.y"
    {
			  struct type *type = yyvsp[-2].tval;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 73:
#line 625 "c-exp.y"
    {
			  struct type *type = yyvsp[-3].tval;
			  struct stoken tmp_token;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  tmp_token.ptr = (char*) alloca (yyvsp[0].sval.length + 2);
			  tmp_token.length = yyvsp[0].sval.length + 1;
			  tmp_token.ptr[0] = '~';
			  memcpy (tmp_token.ptr+1, yyvsp[0].sval.ptr, yyvsp[0].sval.length);
			  tmp_token.ptr[tmp_token.length] = 0;

			  /* Check for valid destructor name.  */
			  destructor_name_p (tmp_token.ptr, type);
			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (tmp_token);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 75:
#line 651 "c-exp.y"
    {
			  char *name = copy_name (yyvsp[0].sval);
			  struct symbol *sym;
			  struct minimal_symbol *msymbol;

			  sym =
			    lookup_symbol (name, (const struct block *) NULL,
					   VAR_DOMAIN, (int *) NULL,
					   (struct symtab **) NULL);
			  if (sym)
			    {
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      break;
			    }

			  msymbol = lookup_minimal_symbol (name, NULL, NULL);
			  if (msymbol != NULL)
			    {
			      write_exp_msymbol (msymbol,
						 lookup_function_type (builtin_type (current_gdbarch)->builtin_int),
						 builtin_type (current_gdbarch)->builtin_int);
			    }
			  else
			    if (!have_full_symbols () && !have_partial_symbols ())
			      error ("No symbol table is loaded.  Use the \"file\" command.");
			    else
			      error ("No symbol \"%s\" in current context.", name);
			}
    break;

  case 76:
#line 685 "c-exp.y"
    { struct symbol *sym = yyvsp[0].ssym.sym;

			  if (sym)
			    {
			      if (symbol_read_needs_frame (sym))
				{
				  if (innermost_block == 0 ||
				      contained_in (block_found, 
						    innermost_block))
				    innermost_block = block_found;
				}

			      write_exp_elt_opcode (OP_VAR_VALUE);
			      /* We want to use the selected frame, not
				 another more inner frame which happens to
				 be in the same block.  */
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			    }
			  else if (yyvsp[0].ssym.is_a_field_of_this)
			    {
			      /* C++: it hangs off of `this'.  Must
			         not inadvertently convert from a method call
				 to data ref.  */
			      if (innermost_block == 0 || 
				  contained_in (block_found, innermost_block))
				innermost_block = block_found;
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			      write_exp_string (yyvsp[0].ssym.stoken);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			    }
			  else
			    {
			      struct minimal_symbol *msymbol;
			      char *arg = copy_name (yyvsp[0].ssym.stoken);

			      msymbol =
				lookup_minimal_symbol (arg, NULL, NULL);
			      if (msymbol != NULL)
				{
				  write_exp_msymbol (msymbol,
						     lookup_function_type (builtin_type (current_gdbarch)->builtin_int),
						     builtin_type (current_gdbarch)->builtin_int);
				}
			      else if (!have_full_symbols () && !have_partial_symbols ())
				error ("No symbol table is loaded.  Use the \"file\" command.");
			      else
				error ("No symbol \"%s\" in current context.",
				       copy_name (yyvsp[0].ssym.stoken));
			    }
			}
    break;

  case 77:
#line 742 "c-exp.y"
    { push_type_address_space (copy_name (yyvsp[0].ssym.stoken));
		  push_type (tp_space_identifier);
		}
    break;

  case 85:
#line 764 "c-exp.y"
    { push_type (tp_pointer); yyval.voidval = 0; }
    break;

  case 86:
#line 766 "c-exp.y"
    { push_type (tp_pointer); yyval.voidval = yyvsp[0].voidval; }
    break;

  case 87:
#line 768 "c-exp.y"
    { push_type (tp_reference); yyval.voidval = 0; }
    break;

  case 88:
#line 770 "c-exp.y"
    { push_type (tp_reference); yyval.voidval = yyvsp[0].voidval; }
    break;

  case 90:
#line 775 "c-exp.y"
    { yyval.voidval = yyvsp[-1].voidval; }
    break;

  case 91:
#line 777 "c-exp.y"
    {
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			}
    break;

  case 92:
#line 782 "c-exp.y"
    {
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			  yyval.voidval = 0;
			}
    break;

  case 93:
#line 789 "c-exp.y"
    { push_type (tp_function); }
    break;

  case 94:
#line 791 "c-exp.y"
    { push_type (tp_function); }
    break;

  case 95:
#line 795 "c-exp.y"
    { yyval.lval = -1; }
    break;

  case 96:
#line 797 "c-exp.y"
    { yyval.lval = yyvsp[-1].typed_val_int.val; }
    break;

  case 97:
#line 801 "c-exp.y"
    { yyval.voidval = 0; }
    break;

  case 98:
#line 803 "c-exp.y"
    { free (yyvsp[-1].tvec); yyval.voidval = 0; }
    break;

  case 100:
#line 819 "c-exp.y"
    { yyval.tval = yyvsp[0].tsym.type; }
    break;

  case 101:
#line 821 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_int; }
    break;

  case 102:
#line 823 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long; }
    break;

  case 103:
#line 825 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_short; }
    break;

  case 104:
#line 827 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long; }
    break;

  case 105:
#line 829 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long; }
    break;

  case 106:
#line 831 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long; }
    break;

  case 107:
#line 833 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long; }
    break;

  case 108:
#line 835 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long; }
    break;

  case 109:
#line 837 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long; }
    break;

  case 110:
#line 839 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long; }
    break;

  case 111:
#line 841 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 112:
#line 843 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 113:
#line 845 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 114:
#line 847 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 115:
#line 849 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 116:
#line 851 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_long; }
    break;

  case 117:
#line 853 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long_long; }
    break;

  case 118:
#line 855 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long_long; }
    break;

  case 119:
#line 857 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long_long; }
    break;

  case 120:
#line 859 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_long_long; }
    break;

  case 121:
#line 861 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_short; }
    break;

  case 122:
#line 863 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_short; }
    break;

  case 123:
#line 865 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_short; }
    break;

  case 124:
#line 867 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_short; }
    break;

  case 125:
#line 869 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_short; }
    break;

  case 126:
#line 871 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_short; }
    break;

  case 127:
#line 873 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_double; }
    break;

  case 128:
#line 875 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_long_double; }
    break;

  case 129:
#line 877 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 130:
#line 880 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 131:
#line 883 "c-exp.y"
    { yyval.tval = lookup_union (copy_name (yyvsp[0].sval),
					     expression_context_block); }
    break;

  case 132:
#line 886 "c-exp.y"
    { yyval.tval = lookup_enum (copy_name (yyvsp[0].sval),
					    expression_context_block); }
    break;

  case 133:
#line 889 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 134:
#line 891 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_unsigned_int; }
    break;

  case 135:
#line 893 "c-exp.y"
    { yyval.tval = lookup_signed_typename (TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 136:
#line 895 "c-exp.y"
    { yyval.tval = builtin_type (current_gdbarch)->builtin_int; }
    break;

  case 137:
#line 900 "c-exp.y"
    { yyval.tval = lookup_template_type(copy_name(yyvsp[-3].sval), yyvsp[-1].tval,
						    expression_context_block);
			}
    break;

  case 138:
#line 904 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[0].tval); }
    break;

  case 139:
#line 906 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-1].tval); }
    break;

  case 141:
#line 956 "c-exp.y"
    {
		  struct type *type = yyvsp[-2].tval;
		  struct type *new_type;
		  char *ncopy = alloca (yyvsp[0].sval.length + 1);

		  memcpy (ncopy, yyvsp[0].sval.ptr, yyvsp[0].sval.length);
		  ncopy[yyvsp[0].sval.length] = '\0';

		  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
		      && TYPE_CODE (type) != TYPE_CODE_UNION
		      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
		    error ("`%s' is not defined as an aggregate type.",
			   TYPE_NAME (type));

		  new_type = cp_lookup_nested_type (type, ncopy,
						    expression_context_block);
		  if (new_type == NULL)
		    error ("No type \"%s\" within class or namespace \"%s\".",
			   ncopy, TYPE_NAME (type));
		  
		  yyval.tval = new_type;
		}
    break;

  case 143:
#line 982 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "int";
		  yyval.tsym.stoken.length = 3;
		  yyval.tsym.type = builtin_type (current_gdbarch)->builtin_int;
		}
    break;

  case 144:
#line 988 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "long";
		  yyval.tsym.stoken.length = 4;
		  yyval.tsym.type = builtin_type (current_gdbarch)->builtin_long;
		}
    break;

  case 145:
#line 994 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "short";
		  yyval.tsym.stoken.length = 5;
		  yyval.tsym.type = builtin_type (current_gdbarch)->builtin_short;
		}
    break;

  case 146:
#line 1003 "c-exp.y"
    { yyval.tvec = (struct type **) xmalloc (sizeof (struct type *) * 2);
		  yyval.ivec[0] = 1;	/* Number of types in vector */
		  yyval.tvec[1] = yyvsp[0].tval;
		}
    break;

  case 147:
#line 1008 "c-exp.y"
    { int len = sizeof (struct type *) * (++(yyvsp[-2].ivec[0]) + 1);
		  yyval.tvec = (struct type **) xrealloc ((char *) yyvsp[-2].tvec, len);
		  yyval.tvec[yyval.ivec[0]] = yyvsp[0].tval;
		}
    break;

  case 149:
#line 1016 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-3].tval); }
    break;

  case 152:
#line 1024 "c-exp.y"
    { push_type (tp_const);
			  push_type (tp_volatile); 
			}
    break;

  case 153:
#line 1028 "c-exp.y"
    { push_type (tp_const); }
    break;

  case 154:
#line 1030 "c-exp.y"
    { push_type (tp_volatile); }
    break;

  case 155:
#line 1033 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 156:
#line 1034 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 157:
#line 1035 "c-exp.y"
    { yyval.sval = yyvsp[0].tsym.stoken; }
    break;

  case 158:
#line 1036 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;


    }

/* Line 1000 of yacc.c.  */
#line 2477 "c-exp.c.tmp"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1050 "c-exp.y"


/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (p, len, parsed_float, putithere)
     char *p;
     int len;
     int parsed_float;
     YYSTYPE *putithere;
{
  /* FIXME: Shouldn't these be unsigned?  We don't deal with negative values
     here, and we do kind of silly things like cast to unsigned.  */
  LONGEST n = 0;
  LONGEST prevn = 0;
  ULONGEST un;

  int i = 0;
  int c;
  int base = input_radix;
  int unsigned_p = 0;

  /* Number of "L" suffixes encountered.  */
  int long_p = 0;

  /* We have found a "L" or "U" suffix.  */
  int found_suffix = 0;

  ULONGEST high_bit;
  struct type *signed_type;
  struct type *unsigned_type;

  if (parsed_float)
    {
      /* It's a float since it contains a point or an exponent.  */
      char *s = xmalloc (len);
      int num = 0;	/* number of tokens scanned by scanf */
      char saved_char = p[len];

      p[len] = 0;	/* null-terminate the token */

      /* If it ends at "df", "dd" or "dl", take it as type of decimal floating
         point.  Return DECFLOAT.  */

      if (p[len - 2] == 'd' && p[len - 1] == 'f')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = builtin_type (current_gdbarch)->builtin_decfloat;
	  decimal_from_string (putithere->typed_val_decfloat.val, 4, p);
	  p[len] = saved_char;
	  return (DECFLOAT);
	}

      if (p[len - 2] == 'd' && p[len - 1] == 'd')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = builtin_type (current_gdbarch)->builtin_decdouble;
	  decimal_from_string (putithere->typed_val_decfloat.val, 8, p);
	  p[len] = saved_char;
	  return (DECFLOAT);
	}

      if (p[len - 2] == 'd' && p[len - 1] == 'l')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = builtin_type (current_gdbarch)->builtin_declong;
	  decimal_from_string (putithere->typed_val_decfloat.val, 16, p);
	  p[len] = saved_char;
	  return (DECFLOAT);
	}

      num = sscanf (p, "%" DOUBLEST_SCAN_FORMAT "%s",
		    &putithere->typed_val_float.dval, s);
      p[len] = saved_char;	/* restore the input stream */

      if (num == 1)
	putithere->typed_val_float.type = 
	  builtin_type (current_gdbarch)->builtin_double;

      if (num == 2 )
	{
	  /* See if it has any float suffix: 'f' for float, 'l' for long 
	     double.  */
	  if (!strcasecmp (s, "f"))
	    putithere->typed_val_float.type = 
	      builtin_type (current_gdbarch)->builtin_float;
	  else if (!strcasecmp (s, "l"))
	    putithere->typed_val_float.type = 
	      builtin_type (current_gdbarch)->builtin_long_double;
	  else
	    {
	      free (s);
	      return ERROR;
	    }
	}

      free (s);
      return FLOAT;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (p[0] == '0')
    switch (p[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    p += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    p += 2;
	    base = 10;
	    len -= 2;
	  }
	break;

      default:
	base = 8;
	break;
      }

  while (len-- > 0)
    {
      c = *p++;
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      if (c != 'l' && c != 'u')
	n *= base;
      if (c >= '0' && c <= '9')
	{
	  if (found_suffix)
	    return ERROR;
	  n += i = c - '0';
	}
      else
	{
	  if (base > 10 && c >= 'a' && c <= 'f')
	    {
	      if (found_suffix)
		return ERROR;
	      n += i = c - 'a' + 10;
	    }
	  else if (c == 'l')
	    {
	      ++long_p;
	      found_suffix = 1;
	    }
	  else if (c == 'u')
	    {
	      unsigned_p = 1;
	      found_suffix = 1;
	    }
	  else
	    return ERROR;	/* Char not a digit */
	}
      if (i >= base)
	return ERROR;		/* Invalid digit in this base */

      /* Portably test for overflow (only works for nonzero values, so make
	 a second check for zero).  FIXME: Can't we just make n and prevn
	 unsigned and avoid this?  */
      if (c != 'l' && c != 'u' && (prevn >= n) && n != 0)
	unsigned_p = 1;		/* Try something unsigned */

      /* Portably test for unsigned overflow.
	 FIXME: This check is wrong; for example it doesn't find overflow
	 on 0x123456789 when LONGEST is 32 bits.  */
      if (c != 'l' && c != 'u' && n != 0)
	{	
	  if ((unsigned_p && (ULONGEST) prevn >= (ULONGEST) n))
	    error ("Numeric constant too large.");
	}
      prevn = n;
    }

  /* An integer constant is an int, a long, or a long long.  An L
     suffix forces it to be long; an LL suffix forces it to be long
     long.  If not forced to a larger size, it gets the first type of
     the above that it fits in.  To figure out whether it fits, we
     shift it right and see whether anything remains.  Note that we
     can't shift sizeof (LONGEST) * HOST_CHAR_BIT bits or more in one
     operation, because many compilers will warn about such a shift
     (which always produces a zero result).  Sometimes gdbarch_int_bit
     or gdbarch_long_bit will be that big, sometimes not.  To deal with
     the case where it is we just always shift the value more than
     once, with fewer bits each time.  */

  un = (ULONGEST)n >> 2;
  if (long_p == 0
      && (un >> (gdbarch_int_bit (current_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_int_bit (current_gdbarch) - 1);

      /* A large decimal (not hex or octal) constant (between INT_MAX
	 and UINT_MAX) is a long or unsigned long, according to ANSI,
	 never an unsigned int, but this code treats it as unsigned
	 int.  This probably should be fixed.  GCC gives a warning on
	 such constants.  */

      unsigned_type = builtin_type (current_gdbarch)->builtin_unsigned_int;
      signed_type = builtin_type (current_gdbarch)->builtin_int;
    }
  else if (long_p <= 1
	   && (un >> (gdbarch_long_bit (current_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_long_bit (current_gdbarch) - 1);
      unsigned_type = builtin_type (current_gdbarch)->builtin_unsigned_long;
      signed_type = builtin_type (current_gdbarch)->builtin_long;
    }
  else
    {
      int shift;
      if (sizeof (ULONGEST) * HOST_CHAR_BIT 
	  < gdbarch_long_long_bit (current_gdbarch))
	/* A long long does not fit in a LONGEST.  */
	shift = (sizeof (ULONGEST) * HOST_CHAR_BIT - 1);
      else
	shift = (gdbarch_long_long_bit (current_gdbarch) - 1);
      high_bit = (ULONGEST) 1 << shift;
      unsigned_type = builtin_type (current_gdbarch)->builtin_unsigned_long_long;
      signed_type = builtin_type (current_gdbarch)->builtin_long_long;
    }

   putithere->typed_val_int.val = n;

   /* If the high bit of the worked out type is set then this number
      has to be unsigned. */

   if (unsigned_p || (n & high_bit)) 
     {
       putithere->typed_val_int.type = unsigned_type;
     }
   else 
     {
       putithere->typed_val_int.type = signed_type;
     }

   return INT;
}

struct token
{
  char *operator;
  int token;
  enum exp_opcode opcode;
};

static const struct token tokentab3[] =
  {
    {">>=", ASSIGN_MODIFY, BINOP_RSH},
    {"<<=", ASSIGN_MODIFY, BINOP_LSH}
  };

static const struct token tokentab2[] =
  {
    {"+=", ASSIGN_MODIFY, BINOP_ADD},
    {"-=", ASSIGN_MODIFY, BINOP_SUB},
    {"*=", ASSIGN_MODIFY, BINOP_MUL},
    {"/=", ASSIGN_MODIFY, BINOP_DIV},
    {"%=", ASSIGN_MODIFY, BINOP_REM},
    {"|=", ASSIGN_MODIFY, BINOP_BITWISE_IOR},
    {"&=", ASSIGN_MODIFY, BINOP_BITWISE_AND},
    {"^=", ASSIGN_MODIFY, BINOP_BITWISE_XOR},
    {"++", INCREMENT, BINOP_END},
    {"--", DECREMENT, BINOP_END},
    {"->", ARROW, BINOP_END},
    {"&&", ANDAND, BINOP_END},
    {"||", OROR, BINOP_END},
    {"::", COLONCOLON, BINOP_END},
    {"<<", LSH, BINOP_END},
    {">>", RSH, BINOP_END},
    {"==", EQUAL, BINOP_END},
    {"!=", NOTEQUAL, BINOP_END},
    {"<=", LEQ, BINOP_END},
    {">=", GEQ, BINOP_END}
  };

/* Read one token, getting characters through lexptr.  */

static int
yylex ()
{
  int c;
  int namelen;
  unsigned int i;
  char *tokstart;
  char *tokptr;
  int tempbufindex;
  static char *tempbuf;
  static int tempbufsize;
  char * token_string = NULL;
  int class_prefix = 0;
   
 retry:

  /* Check if this is a macro invocation that we need to expand.  */
  if (! scanning_macro_expansion ())
    {
      char *expanded = macro_expand_next (&lexptr,
                                          expression_macro_lookup_func,
                                          expression_macro_lookup_baton);

      if (expanded)
        scan_macro_expansion (expanded);
    }

  prev_lexptr = lexptr;

  tokstart = lexptr;
  /* See if it is a special token of length 3.  */
  for (i = 0; i < sizeof tokentab3 / sizeof tokentab3[0]; i++)
    if (strncmp (tokstart, tokentab3[i].operator, 3) == 0)
      {
	lexptr += 3;
	yylval.opcode = tokentab3[i].opcode;
	return tokentab3[i].token;
      }

  /* See if it is a special token of length 2.  */
  for (i = 0; i < sizeof tokentab2 / sizeof tokentab2[0]; i++)
    if (strncmp (tokstart, tokentab2[i].operator, 2) == 0)
      {
	lexptr += 2;
	yylval.opcode = tokentab2[i].opcode;
	return tokentab2[i].token;
      }

  switch (c = *tokstart)
    {
    case 0:
      /* If we were just scanning the result of a macro expansion,
         then we need to resume scanning the original text.
         Otherwise, we were already scanning the original text, and
         we're really done.  */
      if (scanning_macro_expansion ())
        {
          finished_macro_expansion ();
          goto retry;
        }
      else
        return 0;

    case ' ':
    case '\t':
    case '\n':
      lexptr++;
      goto retry;

    case '\'':
      /* We either have a character constant ('0' or '\177' for example)
	 or we have a quoted symbol reference ('foo(int,int)' in C++
	 for example). */
      lexptr++;
      c = *lexptr++;
      if (c == '\\')
	c = parse_escape (&lexptr);
      else if (c == '\'')
	error ("Empty character constant.");
      else if (! host_char_to_target (c, &c))
        {
          int toklen = lexptr - tokstart + 1;
          char *tok = alloca (toklen + 1);
          memcpy (tok, tokstart, toklen);
          tok[toklen] = '\0';
          error ("There is no character corresponding to %s in the target "
                 "character set `%s'.", tok, target_charset ());
        }

      yylval.typed_val_int.val = c;
      yylval.typed_val_int.type = builtin_type (current_gdbarch)->builtin_char;

      c = *lexptr++;
      if (c != '\'')
	{
	  namelen = skip_quoted (tokstart) - tokstart;
	  if (namelen > 2)
	    {
	      lexptr = tokstart + namelen;
	      if (lexptr[-1] != '\'')
		error ("Unmatched single quote.");
	      namelen -= 2;
	      tokstart++;
	      goto tryname;
	    }
	  error ("Invalid character constant.");
	}
      return INT;

    case '(':
      paren_depth++;
      lexptr++;
      return c;

    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      lexptr++;
      return c;

    case ',':
      if (comma_terminates
          && paren_depth == 0
          && ! scanning_macro_expansion ())
	return 0;
      lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (lexptr[1] < '0' || lexptr[1] > '9')
	goto symbol;		/* Nope, must be a symbol. */
      /* FALL THRU into number case.  */

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	/* It's a number.  */
	int got_dot = 0, got_e = 0, toktype;
	char *p = tokstart;
	int hex = input_radix > 10;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T' || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	for (;; ++p)
	  {
	    /* This test includes !hex because 'e' is a valid hex digit
	       and thus does not indicate a floating point number when
	       the radix is hex.  */
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    /* This test does not include !hex, because a '.' always indicates
	       a decimal floating point number regardless of the radix.  */
	    else if (!got_dot && *p == '.')
	      got_dot = 1;
	    else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		     && (*p == '-' || *p == '+'))
	      /* This is the sign of the exponent, not the end of the
		 number.  */
	      continue;
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
				  && (*p < 'A' || *p > 'Z')))
	      break;
	  }
	toktype = parse_number (tokstart, p - tokstart, got_dot|got_e, &yylval);
        if (toktype == ERROR)
	  {
	    char *err_copy = (char *) alloca (p - tokstart + 1);

	    memcpy (err_copy, tokstart, p - tokstart);
	    err_copy[p - tokstart] = 0;
	    error ("Invalid number \"%s\".", err_copy);
	  }
	lexptr = p;
	return toktype;
      }

    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '@':
    case '<':
    case '>':
    case '[':
    case ']':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      lexptr++;
      return c;

    case '"':

      /* Build the gdb internal form of the input string in tempbuf,
	 translating any standard C escape forms seen.  Note that the
	 buffer is null byte terminated *only* for the convenience of
	 debugging gdb itself and printing the buffer contents when
	 the buffer contains no embedded nulls.  Gdb does not depend
	 upon the buffer being null byte terminated, it uses the length
	 string instead.  This allows gdb to handle C strings (as well
	 as strings in other languages) with embedded null bytes */

      tokptr = ++tokstart;
      tempbufindex = 0;

      do {
        char *char_start_pos = tokptr;

	/* Grow the static temp buffer if necessary, including allocating
	   the first one on demand. */
	if (tempbufindex + 1 >= tempbufsize)
	  {
	    tempbuf = (char *) xrealloc (tempbuf, tempbufsize += 64);
	  }
	switch (*tokptr)
	  {
	  case '\0':
	  case '"':
	    /* Do nothing, loop will terminate. */
	    break;
	  case '\\':
	    tokptr++;
	    c = parse_escape (&tokptr);
	    if (c == -1)
	      {
		continue;
	      }
	    tempbuf[tempbufindex++] = c;
	    break;
	  default:
	    c = *tokptr++;
            if (! host_char_to_target (c, &c))
              {
                int len = tokptr - char_start_pos;
                char *copy = alloca (len + 1);
                memcpy (copy, char_start_pos, len);
                copy[len] = '\0';

                error ("There is no character corresponding to `%s' "
                       "in the target character set `%s'.",
                       copy, target_charset ());
              }
            tempbuf[tempbufindex++] = c;
	    break;
	  }
      } while ((*tokptr != '"') && (*tokptr != '\0'));
      if (*tokptr++ != '"')
	{
	  error ("Unterminated string in expression.");
	}
      tempbuf[tempbufindex] = '\0';	/* See note above */
      yylval.sval.ptr = tempbuf;
      yylval.sval.length = tempbufindex;
      lexptr = tokptr;
      return (STRING);
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error ("Invalid character '%c' in expression.", c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '<');)
    {
      /* Template parameter lists are part of the name.
	 FIXME: This mishandles `print $a<4&&$a>3'.  */

      if (c == '<')
	{ 
               /* Scan ahead to get rest of the template specification.  Note
                  that we look ahead only when the '<' adjoins non-whitespace
                  characters; for comparison expressions, e.g. "a < b > c",
                  there must be spaces before the '<', etc. */
               
               char * p = find_template_name_end (tokstart + namelen);
               if (p)
                 namelen = p - tokstart;
               break;
	}
      c = tokstart[++namelen];
    }

  /* The token "if" terminates the expression and is NOT removed from
     the input stream.  It doesn't count if it appears in the
     expansion of a macro.  */
  if (namelen == 2
      && tokstart[0] == 'i'
      && tokstart[1] == 'f'
      && ! scanning_macro_expansion ())
    {
      return 0;
    }

  lexptr += namelen;

  tryname:

  /* Catch specific keywords.  Should be done with a data structure.  */
  switch (namelen)
    {
    case 8:
      if (strncmp (tokstart, "unsigned", 8) == 0)
	return UNSIGNED;
      if (current_language->la_language == language_cplus
	  && strncmp (tokstart, "template", 8) == 0)
	return TEMPLATE;
      if (strncmp (tokstart, "volatile", 8) == 0)
	return VOLATILE_KEYWORD;
      break;
    case 6:
      if (strncmp (tokstart, "struct", 6) == 0)
	return STRUCT;
      if (strncmp (tokstart, "signed", 6) == 0)
	return SIGNED_KEYWORD;
      if (strncmp (tokstart, "sizeof", 6) == 0)
	return SIZEOF;
      if (strncmp (tokstart, "double", 6) == 0)
	return DOUBLE_KEYWORD;
      break;
    case 5:
      if (current_language->la_language == language_cplus)
        {
          if (strncmp (tokstart, "false", 5) == 0)
            return FALSEKEYWORD;
          if (strncmp (tokstart, "class", 5) == 0)
            return CLASS;
        }
      if (strncmp (tokstart, "union", 5) == 0)
	return UNION;
      if (strncmp (tokstart, "short", 5) == 0)
	return SHORT;
      if (strncmp (tokstart, "const", 5) == 0)
	return CONST_KEYWORD;
      break;
    case 4:
      if (strncmp (tokstart, "enum", 4) == 0)
	return ENUM;
      if (strncmp (tokstart, "long", 4) == 0)
	return LONG;
      if (current_language->la_language == language_cplus)
          {
            if (strncmp (tokstart, "true", 4) == 0)
              return TRUEKEYWORD;
          }
      break;
    case 3:
      if (strncmp (tokstart, "int", 3) == 0)
	return INT_KEYWORD;
      break;
    default:
      break;
    }

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  if (*tokstart == '$')
    {
      write_dollar_variable (yylval.sval);
      return VARIABLE;
    }
  
  /* Use token-type BLOCKNAME for symbols that happen to be defined as
     functions or symtabs.  If this is not so, then ...
     Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
  {
    char *tmp = copy_name (yylval.sval);
    struct symbol *sym;
    int is_a_field_of_this = 0;
    int hextype;

    sym = lookup_symbol (tmp, expression_context_block,
			 VAR_DOMAIN,
			 current_language->la_language == language_cplus
			 ? &is_a_field_of_this : (int *) NULL,
			 (struct symtab **) NULL);
    /* Call lookup_symtab, not lookup_partial_symtab, in case there are
       no psymtabs (coff, xcoff, or some future change to blow away the
       psymtabs once once symbols are read).  */
    if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
      {
	yylval.ssym.sym = sym;
	yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	return BLOCKNAME;
      }
    else if (!sym)
      {				/* See if it's a file name. */
	struct symtab *symtab;

	symtab = lookup_symtab (tmp);

	if (symtab)
	  {
	    yylval.bval = BLOCKVECTOR_BLOCK (BLOCKVECTOR (symtab), STATIC_BLOCK);
	    return FILENAME;
	  }
      }

    if (sym && SYMBOL_CLASS (sym) == LOC_TYPEDEF)
        {
	  /* NOTE: carlton/2003-09-25: There used to be code here to
	     handle nested types.  It didn't work very well.  See the
	     comment before qualified_type for more info.  */
	  yylval.tsym.type = SYMBOL_TYPE (sym);
	  return TYPENAME;
        }
    yylval.tsym.type
      = language_lookup_primitive_type_by_name (current_language,
						current_gdbarch, tmp);
    if (yylval.tsym.type != NULL)
      return TYPENAME;

    /* Input names that aren't symbols but ARE valid hex numbers,
       when the input radix permits them, can be names or numbers
       depending on the parse.  Note we support radixes > 16 here.  */
    if (!sym && 
        ((tokstart[0] >= 'a' && tokstart[0] < 'a' + input_radix - 10) ||
         (tokstart[0] >= 'A' && tokstart[0] < 'A' + input_radix - 10)))
      {
 	YYSTYPE newlval;	/* Its value is ignored.  */
	hextype = parse_number (tokstart, namelen, 0, &newlval);
	if (hextype == INT)
	  {
	    yylval.ssym.sym = sym;
	    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	    return NAME_OR_INT;
	  }
      }

    /* Any other kind of symbol */
    yylval.ssym.sym = sym;
    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
    return NAME;
  }
}

void
yyerror (msg)
     char *msg;
{
  if (prev_lexptr)
    lexptr = prev_lexptr;

  error ("A %s in expression, near `%s'.", (msg ? msg : "error"), lexptr);
}



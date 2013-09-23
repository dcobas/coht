/* A Bison parser, made from conv.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	INCLUDE	257
# define	DEFINE	258
# define	TYPEDEF	259
# define	STRUCT	260
# define	UNION	261
# define	ENUM	262
# define	UNSIGNED	263
# define	SIZEOF	264
# define	IF	265
# define	IFDEF	266
# define	IFNDEF	267
# define	ELSE	268
# define	ENDIF	269
# define	CHAR	270
# define	SHORT	271
# define	INT	272
# define	LONG	273
# define	FLOAT	274
# define	DOUBLE	275
# define	STRING	276
# define	SHL	277
# define	SHR	278
# define	EQ	279
# define	E_O_F	280
# define	NUM	281
# define	TYPE	282
# define	STRU	283
# define	UNIO	284
# define	ENU	285
# define	SYM	286

#line 1 "conv.y"
      /******************** C declaration section */
#include "convP.h"

#line 7 "conv.y"
#ifndef YYSTYPE
typedef union { /*define the possible token types */
 int     Num;    /*number EG constant expression */
 symbol *Sym;    /*symbol table reference EG identifier */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		157
#define	YYFLAG		-32768
#define	YYNTBASE	55

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 286 ? yytranslate[x] : 96)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      33,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    36,    34,     2,    47,    39,     2,
      41,    42,    45,    43,    54,    44,     2,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    35,
      37,    53,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    49,     2,    50,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,    38,    52,    48,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     2,     5,     7,     9,    12,    14,    17,    21,
      24,    27,    30,    33,    35,    37,    40,    44,    45,    47,
      49,    51,    53,    55,    56,    58,    60,    64,    68,    72,
      77,    79,    83,    87,    89,    93,    97,    99,   103,   107,
     111,   113,   115,   120,   124,   127,   130,   132,   134,   137,
     139,   141,   143,   145,   147,   149,   151,   153,   155,   158,
     162,   164,   166,   168,   170,   171,   173,   177,   179,   181,
     184,   189,   190,   195,   199,   202,   203,   204,   209,   210,
     214,   218,   221,   222,   223,   228,   232,   235,   236,   238,
     239,   244,   248,   252,   253,   255,   258,   260,   264,   268,
     270,   273
};
static const short yyrhs[] =
{
      56,     0,    55,    56,     0,    33,     0,    26,     0,    34,
      57,     0,    58,     0,     1,    33,     0,     4,    61,    62,
       0,     3,    60,     0,    13,    61,     0,    12,    61,     0,
      11,    63,     0,    14,     0,    15,     0,    59,    35,     0,
      58,    59,    35,     0,     0,    69,     0,    76,     0,    36,
       0,    37,     0,    32,     0,     0,    63,     0,    64,     0,
      63,    38,    64,     0,    63,    39,    64,     0,    63,    40,
      64,     0,    41,    70,    42,    63,     0,    65,     0,    64,
      23,    65,     0,    64,    24,    65,     0,    66,     0,    65,
      43,    66,     0,    65,    44,    66,     0,    67,     0,    66,
      45,    67,     0,    66,    46,    67,     0,    66,    47,    67,
       0,    27,     0,    32,     0,    10,    41,    68,    42,     0,
      41,    63,    42,     0,    44,    67,     0,    48,    67,     0,
      70,     0,    61,     0,    70,    73,     0,    71,     0,    72,
       0,    16,     0,    17,     0,    18,     0,    19,     0,    20,
       0,    21,     0,    22,     0,     9,    71,     0,     9,    33,
      71,     0,    85,     0,    82,     0,    78,     0,    28,     0,
       0,    74,     0,    73,    94,    74,     0,    75,     0,    61,
       0,    45,    75,     0,    75,    49,    63,    50,     0,     0,
       5,    77,    70,    75,     0,     6,    86,    79,     0,     6,
      29,     0,     0,     0,    89,    80,    81,    90,     0,     0,
      69,    95,    81,     0,     7,    86,    83,     0,     7,    30,
       0,     0,     0,    89,    84,    81,    90,     0,     8,    86,
      87,     0,     8,    31,     0,     0,    32,     0,     0,    89,
      88,    92,    90,     0,    91,    51,    91,     0,    91,    52,
      91,     0,     0,    33,     0,    91,    33,     0,    93,     0,
      92,    94,    93,     0,    61,    53,    63,     0,    61,     0,
      54,    91,     0,    35,    91,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    33,    34,    36,    37,    38,    39,    40,    42,    43,
      44,    45,    46,    47,    48,    50,    51,    53,    54,    55,
      58,    59,    61,    63,    64,    66,    67,    68,    69,    70,
      72,    73,    74,    76,    77,    78,    80,    81,    82,    83,
      85,    86,    87,    88,    89,    90,    92,    93,    96,    98,
      99,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     111,   112,   113,   114,   116,   117,   118,   120,   122,   123,
     124,   127,   127,   130,   131,   133,   134,   134,   136,   137,
     140,   141,   143,   144,   144,   147,   148,   150,   151,   153,
     153,   155,   157,   159,   160,   161,   163,   164,   166,   167,
     169,   171
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "INCLUDE", "DEFINE", "TYPEDEF", "STRUCT", 
  "UNION", "ENUM", "UNSIGNED", "SIZEOF", "IF", "IFDEF", "IFNDEF", "ELSE", 
  "ENDIF", "CHAR", "SHORT", "INT", "LONG", "FLOAT", "DOUBLE", "STRING", 
  "SHL", "SHR", "EQ", "E_O_F", "NUM", "TYPE", "STRU", "UNIO", "ENU", 
  "SYM", "'\\n'", "'#'", "';'", "'\\\"'", "'<'", "'|'", "'&'", "'^'", 
  "'('", "')'", "'+'", "'-'", "'*'", "'/'", "'%'", "'~'", "'['", "']'", 
  "'{'", "'}'", "'='", "','", "prog", "line", "preproc", "cmdlist", "cmd", 
  "filename", "name", "expr", "exp", "aexp", "mexp", "term", "prime", 
  "typed", "vardef", "type", "scalar", "compound", "varlist", "varl", 
  "var", "typedef", "@1", "struct", "structb", "@2", "vardeflist", 
  "union", "unionb", "@3", "enum", "aname", "enumb", "@4", "bopen", 
  "bclose", "nextline", "enumlist", "enumer", "comma", "scolon", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    55,    55,    56,    56,    56,    56,    56,    57,    57,
      57,    57,    57,    57,    57,    58,    58,    59,    59,    59,
      60,    60,    61,    62,    62,    63,    63,    63,    63,    63,
      64,    64,    64,    65,    65,    65,    66,    66,    66,    66,
      67,    67,    67,    67,    67,    67,    68,    68,    69,    70,
      70,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      72,    72,    72,    72,    73,    73,    73,    74,    75,    75,
      75,    77,    76,    78,    78,    79,    80,    79,    81,    81,
      82,    82,    83,    84,    83,    85,    85,    86,    86,    88,
      87,    89,    90,    91,    91,    91,    92,    92,    93,    93,
      94,    95
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     1,     2,     1,     1,     2,     1,     2,     3,     2,
       2,     2,     2,     1,     1,     2,     3,     0,     1,     1,
       1,     1,     1,     0,     1,     1,     3,     3,     3,     4,
       1,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       1,     1,     4,     3,     2,     2,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       1,     1,     1,     1,     0,     1,     3,     1,     1,     2,
       4,     0,     4,     3,     2,     0,     0,     4,     0,     3,
       3,     2,     0,     0,     4,     3,     2,     0,     1,     0,
       4,     3,     3,     0,     1,     2,     1,     3,     3,     1,
       2,     2
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     0,    71,    87,    87,    87,     0,    51,    52,    53,
      54,    55,    56,    57,     4,    63,     3,     0,     0,     1,
       6,     0,    18,    64,    49,    50,    19,    62,    61,    60,
       7,     0,    74,    88,    75,    81,    82,    86,    93,     0,
      58,     0,     0,     0,     0,     0,    13,    14,     5,     2,
       0,    15,    22,     0,    68,    48,    65,    67,     0,    94,
      73,    76,     0,    80,    83,    85,    89,    59,    20,    21,
       9,    23,     0,    40,    41,     0,     0,     0,    12,    25,
      30,    33,    36,    11,    10,    16,    69,    93,     0,     0,
      72,    78,    95,    93,    78,     0,     8,    24,     0,     0,
       0,     0,    44,    45,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,    66,     0,     0,    93,    91,
      93,    99,    93,    96,    47,     0,    46,    43,     0,    26,
      27,    28,    31,    32,    34,    35,    37,    38,    39,    70,
      93,    78,    77,     0,    84,     0,    90,     0,    42,    29,
     101,    79,    93,    98,    97,    92,     0,     0
};

static const short yydefgoto[] =
{
      18,    19,    48,    20,    21,    70,    54,    96,    99,    79,
      80,    81,    82,   125,    22,    23,    24,    25,    55,    56,
      57,    26,    31,    27,    60,    91,   118,    28,    63,    94,
      29,    34,    65,    95,    61,   142,    62,   122,   123,    88,
     141
};

static const short yypact[] =
{
     148,    33,-32768,    14,    22,    17,   188,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,   243,   117,-32768,
     206,    -4,-32768,    -3,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,   223,-32768,-32768,   -17,-32768,   -17,-32768,    57,     5,
  -32768,   -19,    72,     3,    72,    72,-32768,-32768,-32768,-32768,
       6,-32768,-32768,    -3,-32768,    76,-32768,    83,    -3,-32768,
  -32768,-32768,   -14,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,     3,   103,-32768,-32768,    55,    68,    68,    54,    36,
      58,    95,-32768,-32768,-32768,-32768,    83,    57,    -3,     3,
      83,   223,-32768,    57,   223,    72,-32768,    54,   171,    89,
     104,     3,-32768,-32768,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,   114,-32768,    41,   113,    57,   114,
      57,   105,   -21,-32768,-32768,   118,-32768,-32768,     3,    36,
      36,    36,    58,    58,    95,    95,-32768,-32768,-32768,-32768,
      57,   223,-32768,   -24,-32768,     3,-32768,    72,-32768,    54,
     114,-32768,    57,    54,-32768,   114,   159,-32768
};

static const short yypgoto[] =
{
  -32768,   143,-32768,-32768,   142,-32768,   -42,-32768,   -39,     9,
     -10,    -2,     8,-32768,   -84,   -30,     0,-32768,-32768,    75,
     -38,-32768,-32768,-32768,-32768,-32768,   -86,-32768,-32768,-32768,
  -32768,   106,-32768,-32768,    31,   -34,   -82,-32768,    24,    50,
  -32768
};


#define	YYLAST		258


static const short yytable[] =
{
      71,    58,    83,    84,    78,   114,    40,   117,   120,    92,
     117,   119,    59,    72,     6,    86,    59,    68,    69,    92,
      90,     7,     8,     9,    10,    11,    12,    13,   152,    52,
      73,    51,    97,    87,   -93,    74,   143,    93,   143,    67,
     143,    85,    53,    32,    75,   100,    33,    76,    37,    33,
     116,    77,    35,   121,    33,   151,   124,   117,   150,   107,
     108,     3,     4,     5,     6,    72,    30,    64,   126,    66,
     155,     7,     8,     9,    10,    11,    12,    13,    72,   104,
     105,   106,    73,    15,   102,   103,   144,    74,   146,   149,
      59,   139,   104,   105,   106,    73,    75,   132,   133,    76,
      74,   109,   110,    77,    52,   121,   153,   134,   135,   101,
      36,    38,    76,   129,   130,   131,    77,   156,     1,   136,
     137,   138,     2,     3,     4,     5,     6,   104,   105,   106,
      87,   127,    89,     7,     8,     9,    10,    11,    12,    13,
     111,   112,   113,    14,    98,    15,   128,    92,   140,     1,
      16,    17,   -17,     2,     3,     4,     5,     6,   145,   157,
     148,    49,    50,   115,     7,     8,     9,    10,    11,    12,
      13,   154,   147,     0,    14,     0,    15,     3,     4,     5,
       6,    16,    17,   -17,     0,     0,     0,     7,     8,     9,
      10,    11,    12,    13,     0,     0,     0,     6,     0,    15,
       0,     0,     0,    52,     7,     8,     9,    10,    11,    12,
      13,     2,     3,     4,     5,     6,     0,     0,     0,     0,
       0,    39,     7,     8,     9,    10,    11,    12,    13,     3,
       4,     5,     6,     0,    15,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    41,    42,     0,     0,
       0,    15,     0,     0,    43,    44,    45,    46,    47
};

static const short yycheck[] =
{
      42,    31,    44,    45,    43,    87,     6,    91,    94,    33,
      94,    93,    33,    10,     9,    53,    33,    36,    37,    33,
      58,    16,    17,    18,    19,    20,    21,    22,    52,    32,
      27,    35,    71,    54,    51,    32,   118,    51,   120,    39,
     122,    35,    45,    29,    41,    75,    32,    44,    31,    32,
      89,    48,    30,    95,    32,   141,    98,   141,   140,    23,
      24,     6,     7,     8,     9,    10,    33,    36,    98,    38,
     152,    16,    17,    18,    19,    20,    21,    22,    10,    38,
      39,    40,    27,    28,    76,    77,   120,    32,   122,   128,
      33,    50,    38,    39,    40,    27,    41,   107,   108,    44,
      32,    43,    44,    48,    32,   147,   145,   109,   110,    41,
       4,     5,    44,   104,   105,   106,    48,     0,     1,   111,
     112,   113,     5,     6,     7,     8,     9,    38,    39,    40,
      54,    42,    49,    16,    17,    18,    19,    20,    21,    22,
      45,    46,    47,    26,    41,    28,    42,    33,    35,     1,
      33,    34,    35,     5,     6,     7,     8,     9,    53,     0,
      42,    18,    20,    88,    16,    17,    18,    19,    20,    21,
      22,   147,   122,    -1,    26,    -1,    28,     6,     7,     8,
       9,    33,    34,    35,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    21,    22,    -1,    -1,    -1,     9,    -1,    28,
      -1,    -1,    -1,    32,    16,    17,    18,    19,    20,    21,
      22,     5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    33,    16,    17,    18,    19,    20,    21,    22,     6,
       7,     8,     9,    -1,    28,    -1,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    21,    22,     3,     4,    -1,    -1,
      -1,    28,    -1,    -1,    11,    12,    13,    14,    15
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
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
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


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
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
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
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


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
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
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

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

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
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
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
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
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

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
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
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
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
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 4:
#line 37 "conv.y"
{if (Eof()) return; ;
    break;}
case 7:
#line 40 "conv.y"
{yyerrok; ;
    break;}
case 8:
#line 42 "conv.y"
{Define(yyvsp[-1].Sym,yyvsp[0].Num); ;
    break;}
case 9:
#line 43 "conv.y"
{Include(); ;
    break;}
case 10:
#line 44 "conv.y"
{IfDef(yyvsp[0].Sym,IFNDEF);;
    break;}
case 11:
#line 45 "conv.y"
{IfDef(yyvsp[0].Sym,IFDEF); ;
    break;}
case 12:
#line 46 "conv.y"
{IfDef(yyvsp[0].Num,IF);    ;
    break;}
case 13:
#line 47 "conv.y"
{IfDef(0L,ELSE);  ;
    break;}
case 14:
#line 48 "conv.y"
{IfDef(0L,ENDIF); ;
    break;}
case 20:
#line 58 "conv.y"
{SetFileName('\"'); ;
    break;}
case 21:
#line 59 "conv.y"
{SetFileName('>' ); ;
    break;}
case 22:
#line 61 "conv.y"
{nameptr=yyvsp[0].Sym; ;
    break;}
case 23:
#line 63 "conv.y"
{yyval.Num=0; ;
    break;}
case 26:
#line 67 "conv.y"
{yyval.Num= yyvsp[-2].Num | yyvsp[0].Num;;
    break;}
case 27:
#line 68 "conv.y"
{yyval.Num= yyvsp[-2].Num + yyvsp[0].Num;;
    break;}
case 28:
#line 69 "conv.y"
{yyval.Num= yyvsp[-2].Num ^ yyvsp[0].Num;;
    break;}
case 29:
#line 70 "conv.y"
{yyval.Num= yyvsp[0].Num; ;
    break;}
case 31:
#line 73 "conv.y"
{yyval.Num= yyvsp[-2].Num << yyvsp[0].Num; ;
    break;}
case 32:
#line 74 "conv.y"
{yyval.Num= yyvsp[-2].Num >> yyvsp[0].Num; ;
    break;}
case 34:
#line 77 "conv.y"
{yyval.Num= yyvsp[-2].Num + yyvsp[0].Num;;
    break;}
case 35:
#line 78 "conv.y"
{yyval.Num= yyvsp[-2].Num - yyvsp[0].Num;;
    break;}
case 37:
#line 81 "conv.y"
{yyval.Num= yyvsp[-2].Num * yyvsp[0].Num;;
    break;}
case 38:
#line 82 "conv.y"
{if (yyvsp[0].Num) yyval.Num= yyvsp[-2].Num / yyvsp[0].Num; else printf("Divide by 0 ");;
    break;}
case 39:
#line 83 "conv.y"
{if (yyvsp[0].Num) yyval.Num= yyvsp[-2].Num % yyvsp[0].Num; else printf("Divide by 0 ");;
    break;}
case 41:
#line 86 "conv.y"
{yyval.Num= yyvsp[0].Sym->Val; if (yyvsp[0].Sym->Type!=NUM) Error("Constant expected:%s",yyvsp[0].Sym->Name); ;
    break;}
case 42:
#line 87 "conv.y"
{yyval.Num= SizeOf(yyvsp[-1].Sym);;
    break;}
case 43:
#line 88 "conv.y"
{yyval.Num= yyvsp[-1].Num;;
    break;}
case 44:
#line 89 "conv.y"
{yyval.Num= - yyvsp[0].Num;;
    break;}
case 45:
#line 90 "conv.y"
{yyval.Num= ~ yyvsp[0].Num;;
    break;}
case 48:
#line 96 "conv.y"
{SetType(yyvsp[-1].Sym); ;
    break;}
case 49:
#line 98 "conv.y"
{SetType(yyvsp[0].Sym); ;
    break;}
case 50:
#line 99 "conv.y"
{SetType(yyvsp[0].Sym); ;
    break;}
case 58:
#line 108 "conv.y"
{yyval.Sym= yyvsp[0].Sym; ;
    break;}
case 59:
#line 109 "conv.y"
{yyval.Sym= yyvsp[0].Sym; ;
    break;}
case 64:
#line 116 "conv.y"
{NoVarList(); ;
    break;}
case 67:
#line 120 "conv.y"
{VarInList(yyvsp[0].Sym); ;
    break;}
case 68:
#line 122 "conv.y"
{SetVar(yyvsp[0].Sym); ;
    break;}
case 69:
#line 123 "conv.y"
{SetPtrLevel(yyval.Sym= yyvsp[0].Sym); ;
    break;}
case 70:
#line 124 "conv.y"
{SetDimension(yyvsp[-3].Sym,yyvsp[-1].Num); ;
    break;}
case 71:
#line 127 "conv.y"
{Typedef=1;;
    break;}
case 72:
#line 127 "conv.y"
{TypeDef(yyvsp[0].Sym); ;
    break;}
case 73:
#line 130 "conv.y"
{yyval.Sym= typtr; ;
    break;}
case 74:
#line 131 "conv.y"
{yyval.Sym= yyvsp[0].Sym; ;
    break;}
case 75:
#line 133 "conv.y"
{MarkBlock(StructType);;
    break;}
case 76:
#line 134 "conv.y"
{OpenBlock(StructType);;
    break;}
case 80:
#line 140 "conv.y"
{yyval.Sym= typtr; ;
    break;}
case 81:
#line 141 "conv.y"
{yyval.Sym= yyvsp[0].Sym; ;
    break;}
case 82:
#line 143 "conv.y"
{MarkBlock(UnionType);;
    break;}
case 83:
#line 144 "conv.y"
{OpenBlock(UnionType);;
    break;}
case 85:
#line 147 "conv.y"
{yyval.Sym= typtr; ;
    break;}
case 86:
#line 148 "conv.y"
{yyval.Sym= yyvsp[0].Sym; ;
    break;}
case 87:
#line 150 "conv.y"
{nameptr= NULL;;
    break;}
case 88:
#line 151 "conv.y"
{nameptr= yyvsp[0].Sym;  ;
    break;}
case 89:
#line 153 "conv.y"
{OpenBlock(EnumType);;
    break;}
case 92:
#line 157 "conv.y"
{CloseBlock();;
    break;}
case 98:
#line 166 "conv.y"
{SetEnum(yyvsp[-2].Sym,0,yyvsp[0].Num); ;
    break;}
case 99:
#line 167 "conv.y"
{SetEnum(yyvsp[0].Sym,1, 0); ;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
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

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 174 "conv.y"


#include "convP.c"

/* --conv.y-- */

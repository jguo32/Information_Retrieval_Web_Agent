extern char *malloc(), *realloc();

# line 7 "sym.y"
#include <stdio.h>
#include "SYM.h"

static	int	val_id, yyii, yyij;
static	char	**val_names, *field;
char	*strtok();
static  int	val_names_size=20;
static	bool	val_fill;
bool	read_prior = FALSE;
static	double	*probs;
static	int	max_prob;
static	int	error_count=0;
static	int	next_ut;
static	double	flt;
static  int     nextattr;
static  int     addattr;
static double	fetch_float();

extern FILE *yyin;
int yylineno=1;


# line 33 "sym.y"
typedef union  {
	Context  *tk_context;
	int	 tk_int;
	char 	 *tk_string;
	} YYSTYPE;
# define IDENTIFIER 257
# define STRING 258
# define CONTINUOUS 259
# define AND 260
# define ONLYIF 261
# define NEVER 262
# define CONTEXTS 263
# define STRATIFY 264
# define UTILITIES 265
# define ASFOR 266
# define PRIOR 267
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 508 "sym.y"


parse_symbols(fp)
FILE *fp;
{
	nextattr = 0;
	ncattrs = 0;
	nsattrs = 0;
	ndattrs = 0;
	decattr = -1;
	yyin = fp;
	val_names = mems_alloc(char *, val_names_size+1);
	return !yyparse();
}

yyerror(s,a1,a2,a3)
char *s,*a1,*a2,*a3;
{
	char errstr[256];

	error_count++;
	sprintf(errstr,s,a1,a2,a3);
	fprintf(stderr,".attr file line %3d: %s\n",yylineno,errstr);
}

static double	fetch_float(str)
char	*str;
{
	double	temp, strtod();
	char	*ptr;
	temp = strtod(str,&ptr);
	if (!ptr) 
		yyerror("float expected instead of %s",str);
	return temp;
}

static assign_val_id_to_unord()
{	
	st[nextattr].u.unord = mem_alloc(unord_sym);
	st[nextattr].u.unord->vals = val_id;
	st[nextattr].u.unord->subsetting = SB_OFF;
	sym_fill(nextattr) = val_fill;
	if ( val_id ) {
    	    unordstrs(nextattr) = mems_alloc(char *,val_id);
   	    for (yyii=0; yyii<val_id; yyii++) {
	        unordstrs(nextattr)[yyii] = val_names[yyii];
	      }
	    unordhash(nextattr) =
		fill_hash(&unordstrs(nextattr), val_id);
	    if ( used_hash(unordhash(nextattr)) != val_id )
		yyerror("duplicate attribute value names");
	} else {
    	    unordstrs(nextattr) = 0;
    	    unordhash(nextattr) = 0;
	}
	for (yyii=1; yyii<addattr; yyii++) {
	    st[nextattr+yyii].u.unord = 
	        st[nextattr].u.unord ;
	    st[nextattr+yyii].sflags = st[nextattr].sflags;
	}	
}
int yyexca[] ={
-1, 0,
	257, 10,
	58, 10,
	-2, 0,
-1, 1,
	0, -1,
	-2, 0,
-1, 2,
	0, 1,
	257, 10,
	263, 1,
	264, 1,
	265, 1,
	267, 1,
	58, 10,
	-2, 0,
-1, 13,
	0, 4,
	263, 4,
	264, 4,
	265, 4,
	267, 4,
	-2, 0,
	};
# define YYNPROD 54
# define YYLAST 219
int yyact[]={

    27,    17,    18,    19,    76,    20,    42,    43,    54,    50,
    61,    57,    30,    31,    27,    11,    93,    47,    91,    90,
    89,    31,    86,    83,    82,    41,     5,    38,    46,    29,
    52,    63,    79,    78,    80,    74,    69,    67,    35,    34,
    33,    67,    32,    94,    87,    75,    71,    65,    51,    70,
    44,     9,    66,    67,    73,    68,    38,    72,    37,    28,
    23,    22,     3,    49,    48,     7,    81,    59,    40,    53,
    36,    21,    64,     8,     4,    16,    15,    14,    13,    10,
     6,     2,    62,     1,     0,    39,     0,     0,     0,     0,
     0,     0,    45,     0,     0,     0,     0,     0,     0,    84,
     0,     0,     0,     0,    85,     0,     0,    64,    58,    88,
    92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    26,     0,    24,     0,     0,     0,
     0,     0,    55,    25,    60,    56,     0,     0,    26,     0,
     0,     0,     0,     0,    12,     0,     0,     0,    77 };
int yypact[]={

  -230, -1000,  -230, -1000, -1000,     5, -1000, -1000,   -43, -1000,
  -262,   -63, -1000,  -244, -1000, -1000, -1000,   -16,   -18,   -19,
   -20,    18,    12,   -49, -1000,  -232, -1000, -1000, -1000,  -255,
     4, -1000,  -244,  -240,  -240,  -249,     2,   -55,   -52,   -17,
   -53, -1000,  -236,     1, -1000, -1000,    -7,    11,   -10,     9,
     0, -1000,    16,    10, -1000,   -26, -1000, -1000, -1000, -1000,
    -1, -1000,   -42, -1000,   -28, -1000, -1000,  -233,  -234, -1000,
  -240, -1000, -1000,   -55,  -235,    -2, -1000,  -236,  -237,  -238,
  -239,  -240, -1000, -1000,     9, -1000, -1000,  -241, -1000, -1000,
 -1000, -1000,    -3, -1000, -1000 };
int yypgo[]={

     0,    83,    31,    82,    29,    81,    80,    79,    78,    77,
    76,    75,    62,    74,    73,    71,    70,    30,    69,    61,
    68,    67,    59,    28,    66,    64 };
int yyr1[]={

     0,     6,     1,     7,     7,     7,     7,     7,     5,     5,
    13,    12,    12,    14,    14,    16,    16,    17,    17,    18,
    18,    18,    15,    15,    20,    15,    15,    21,    21,    21,
    19,    19,    19,    19,     8,     8,    22,    22,    22,     3,
     3,     4,     2,     2,     2,     2,    24,     9,    11,    10,
    25,    25,    23,    23 };
int yyr2[]={

     0,     1,     7,     0,     4,     4,     4,     4,     2,     4,
     1,    13,     5,     5,     0,     6,     0,     6,     2,     3,
     3,     7,     3,     7,     1,     7,     5,     9,     3,     1,
     3,     7,     7,     3,     6,     4,     9,     7,     5,     3,
     7,     3,     7,     7,     7,     3,     1,    15,     9,     9,
     3,     7,     7,     7 };
int yychk[]={

 -1000,    -1,    -5,   -12,   -13,   256,    -6,   -12,   -14,    46,
    -7,    58,   257,    -8,    -9,   -10,   -11,   263,   264,   265,
   267,   -15,   -19,   123,   259,   266,   257,    63,   -22,    -4,
   256,   257,    58,    58,    58,    58,   -16,    40,    44,   -19,
   -20,   257,   261,   262,    46,   -22,   -23,   257,   -25,   -23,
   258,    46,   -17,   -18,    63,   257,   257,    63,   125,   -21,
   257,    63,    -3,    -2,    -4,    46,    59,    44,    44,    46,
    59,    46,    41,    44,    61,    46,    46,   260,    61,    60,
    62,   -24,   257,   257,   -23,   -17,   257,    46,    -2,   257,
   257,   257,   -23,   257,    46 };
int yydef[]={

    -2,    -2,    -2,     8,    14,     0,     3,     9,     0,    12,
     2,     0,    13,    -2,     5,     6,     7,     0,     0,     0,
     0,    16,    22,     0,    24,     0,    30,    33,    35,     0,
     0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
    29,    26,     0,     0,    38,    34,     0,     0,     0,    50,
     0,    11,     0,    18,    19,    20,    31,    32,    23,    25,
     0,    28,     0,    39,    45,    37,    46,     0,     0,    49,
     0,    48,    15,     0,     0,     0,    36,     0,     0,     0,
     0,     0,    53,    52,    51,    17,    21,     0,    40,    42,
    43,    44,     0,    27,    47 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"IDENTIFIER",	257,
	"STRING",	258,
	"CONTINUOUS",	259,
	"AND",	260,
	"ONLYIF",	261,
	"NEVER",	262,
	"CONTEXTS",	263,
	"STRATIFY",	264,
	"UTILITIES",	265,
	"ASFOR",	266,
	"PRIOR",	267,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"symbols : attribute_defs",
	"symbols : attribute_defs other_defs",
	"other_defs : /* empty */",
	"other_defs : other_defs contexts",
	"other_defs : other_defs stratify",
	"other_defs : other_defs utilities",
	"other_defs : other_defs prior",
	"attribute_defs : attribute_def",
	"attribute_defs : attribute_defs attribute_def",
	"attribute_def : /* empty */",
	"attribute_def : identifier_list ':' attribute_vals attr_qualifiers '.'",
	"attribute_def : error '.'",
	"identifier_list : identifier_list IDENTIFIER",
	"identifier_list : /* empty */",
	"attr_qualifiers : '(' attr_qualifier_list ')'",
	"attr_qualifiers : /* empty */",
	"attr_qualifier_list : attr_qualifier ',' attr_qualifier_list",
	"attr_qualifier_list : attr_qualifier",
	"attr_qualifier : '?'",
	"attr_qualifier : IDENTIFIER",
	"attr_qualifier : IDENTIFIER '=' IDENTIFIER",
	"attribute_vals : discrete_vals",
	"attribute_vals : '{' discrete_vals '}'",
	"attribute_vals : CONTINUOUS",
	"attribute_vals : CONTINUOUS min_max",
	"attribute_vals : ASFOR IDENTIFIER",
	"min_max : IDENTIFIER '.' '.' IDENTIFIER",
	"min_max : '?'",
	"min_max : /* empty */",
	"discrete_vals : IDENTIFIER",
	"discrete_vals : discrete_vals ',' IDENTIFIER",
	"discrete_vals : discrete_vals ',' '?'",
	"discrete_vals : '?'",
	"contexts : CONTEXTS ':' context_defn",
	"contexts : contexts context_defn",
	"context_defn : attrid ONLYIF context '.'",
	"context_defn : attrid NEVER '.'",
	"context_defn : error '.'",
	"context : context_lit",
	"context : context AND context_lit",
	"attrid : IDENTIFIER",
	"context_lit : attrid '=' IDENTIFIER",
	"context_lit : attrid '<' IDENTIFIER",
	"context_lit : attrid '>' IDENTIFIER",
	"context_lit : attrid",
	"stratify : STRATIFY ':' probabilities ';'",
	"stratify : STRATIFY ':' probabilities ';' probabilities '.'",
	"prior : PRIOR ':' STRING '.'",
	"utilities : UTILITIES ':' utility_list '.'",
	"utility_list : probabilities",
	"utility_list : utility_list ';' probabilities",
	"probabilities : IDENTIFIER ',' IDENTIFIER",
	"probabilities : probabilities ',' IDENTIFIER",
};
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					(void)printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						(void)printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						(void)printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			(void)printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 49 "sym.y"
{
		  if ( decattr < 0 ) {
                	yyerror("no decision attribute found");
			decattr = 0;
		  }
        	  nvalsattr = 2;
        	  foraattrs(yyii)
                      if ( !num_type(yyii)  && unordvals(yyii) > nvalsattr )
                        nvalsattr = unordvals(yyii);
        	  st_ndecs = unordvals(decattr);
		} break;
case 2:
# line 61 "sym.y"
{
		  sfree(val_names);
		  if (error_count>0)
			return 1;
		} break;
case 10:
# line 79 "sym.y"
{
			addattr = 0;
		} break;
case 11:
# line 83 "sym.y"
{
		        nextattr += addattr;
		} break;
case 12:
# line 87 "sym.y"
{
	    	    	yyerror("can't read attribute definition");
		} break;
case 13:
# line 93 "sym.y"
{
			/*
			 *     attribute called "class" is the 
			 *     decision attribute
			 */
			if ( nextattr+addattr>=MAXATTRS ) {
	    	    		yyerror("too many attributes");
				return 1;
			}
			name(nextattr+addattr) = yypvt[-0].tk_string;
			if ( strcmp(yypvt[-0].tk_string, "class") == 0 ||
				 strcmp(yypvt[-0].tk_string, "Class") == 0 ) {
                            type(nextattr+addattr) = DECISION;
                            decattr = nextattr+addattr;
			} else
                            type(nextattr+addattr) = DISCRETE;
			unkns(nextattr+addattr) = FALSE;
			sym_fill(nextattr+addattr) = FALSE;
			onlyif(nextattr+addattr) = Cnil;
			addattr++;
		} break;
case 19:
# line 125 "sym.y"
{
		        for ( yyii=0; yyii<addattr; yyii++)
				unkns(nextattr+yyii) = TRUE;
		} break;
case 20:
# line 131 "sym.y"
{
		        if ( strcmp(yypvt[-0].tk_string,"class") )
			       yyerror("unrecognized attribute qualifier");
			sfree(yypvt[-0].tk_string);
			if ( addattr > 1 )
				yyerror("multiple class attributes");
	    	    	if ( type(nextattr)== DISCRETE )
			        /* assigned as discrete, so decrement */
				ndattrs--;
	    	    	if ( type(nextattr)!= DISCRETE  &&
	    	    			type(nextattr)!= DECISION )
				yyerror("class attribute must be discrete");
                        type(nextattr) = DECISION;
			sindex(nextattr) = 0;
			decattr = nextattr;
		} break;
case 21:
# line 149 "sym.y"
{
		        if ( !strcmp(yypvt[-2].tk_string,"subset") && !sym_fill(nextattr) ) {
			  if ( unordvals(nextattr) == 2 )
			    yyerror("subsetting meaningless for binary attr.");
			  if ( !strcmp(yypvt[-0].tk_string,SB_FULL_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_FULL; 
			  else if ( !strcmp(yypvt[-0].tk_string,SB_ONE_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_ONE; 
			  else if ( !strcmp(yypvt[-0].tk_string,SB_REST_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_REST; 
			  else
			    yyerror("unknown option for subset flag");
			} else
			    yyerror("unknown flag in attribute qualifier");
			sfree(yypvt[-2].tk_string);
			sfree(yypvt[-0].tk_string);
		} break;
case 22:
# line 169 "sym.y"
{	
			for ( yyii=0; yyii<addattr; yyii++) {
			    if ( type(nextattr+yyii) == DECISION ) {
                                sindex(nextattr+yyii) = 0;
                            } else
                                sindex(nextattr+yyii) = (ndattrs++) + 1;
			}
			assign_val_id_to_unord();
		} break;
case 23:
# line 179 "sym.y"
{	
			if ( val_id > SYMSETMAX )
				/*
				 *   keeps the final bit for SDKNOW
				 */
				yyerror("too many values for set type");
			for ( yyii=0; yyii<addattr; yyii++) {
                                type(nextattr+yyii) = SETTYPE;
				sindex(nextattr+yyii) = nsattrs++;
			}
			assign_val_id_to_unord();
		} break;
case 24:
# line 192 "sym.y"
{
			st[nextattr].u.ord = mem_alloc(ord_sym);
		} break;
case 25:
# line 196 "sym.y"
{	
			sindex(nextattr) = ncattrs++;
			type(nextattr) = yypvt[-2].tk_int;
			for (yyii=1; yyii<addattr; yyii++) {
				sindex(nextattr+yyii) = ncattrs++;
				type(nextattr+yyii) = yypvt[-2].tk_int;
			        st[nextattr+yyii].u.ord = st[val_id].u.ord;
				st[nextattr+yyii].sflags = st[nextattr].sflags;
			}
		} break;
case 26:
# line 207 "sym.y"
{	
		        val_id = getattribute(yypvt[-0].tk_string);
			if ( val_id < 0 ) {
				yyerror("identifier %s not already seen",yypvt[-0].tk_string);
				val_id = 0;
			}
			sfree(yypvt[-0].tk_string);
			for ( yyii=0; yyii<addattr; yyii++) {
			  st[nextattr+yyii].sflags = st[val_id].sflags;
			  if ( num_type(val_id) ) {
			    sindex(nextattr+yyii) = ncattrs++;
			    type(nextattr+yyii) = type(val_id);
			    st[nextattr+yyii].u.ord = st[val_id].u.ord;
			  } else {
			    if ( type(val_id) == SETTYPE )
				sindex(nextattr) = nsattrs++;
			    else
				sindex(nextattr) = (ndattrs++) + 1;
			    type(nextattr+yyii) = type(val_id);
			    if ( type(val_id) == DECISION  )
				 type(nextattr+yyii) = DISCRETE;
			    st[nextattr+yyii].u.unord = 
				st[val_id].u.unord;
			  }
			}
		} break;
case 27:
# line 236 "sym.y"
{	
	    		getmin(nextattr) = fetch_float(yypvt[-3].tk_string); sfree(yypvt[-3].tk_string);
	    		getmax(nextattr) = fetch_float(yypvt[-0].tk_string); sfree(yypvt[-0].tk_string);
		} break;
case 28:
# line 241 "sym.y"
{	
			sym_fill(nextattr) = TRUE;
	    		getmin(nextattr) = FLOATMAX;
	    		getmax(nextattr) = -FLOATMAX;
		} break;
case 29:
# line 247 "sym.y"
{	
	    		getmin(nextattr) = -FLOATMAX;
	    		getmax(nextattr) = FLOATMAX;
		} break;
case 30:
# line 255 "sym.y"
{
			val_id = 1; 
			val_fill = FALSE;
			val_names[0] = (char *) yypvt[-0].tk_string; 
		 } break;
case 31:
# line 261 "sym.y"
{
			if ( val_id >= val_names_size  ) {
				val_names_size += 20;
				val_names = mems_realloc(val_names,char *, 
						val_names_size+1);
			}
	    		val_names[val_id++] = (char *) yypvt[-0].tk_string; 
		} break;
case 32:
# line 270 "sym.y"
{
			val_fill = TRUE;
		} break;
case 33:
# line 274 "sym.y"
{
			val_fill = TRUE;
			val_id = 0;
		} break;
case 36:
# line 284 "sym.y"
{
			  onlyif(yypvt[-3].tk_int) = yypvt[-1].tk_context;
			} break;
case 37:
# line 288 "sym.y"
{
			  onlyif(yypvt[-2].tk_int) = mem_alloc(Context);
			  onlyif(yypvt[-2].tk_int)->coutcome = DONTKNOW;
			  onlyif(yypvt[-2].tk_int)->ctest.attr = decattr;
			} break;
case 38:
# line 294 "sym.y"
{
	    	    	yyerror("can't read context definition");
		} break;
case 39:
# line 300 "sym.y"
{ yyval.tk_context = yypvt[-0].tk_context; 
		  yypvt[-0].tk_context->cnext = 0; } break;
case 40:
# line 303 "sym.y"
{
                  yyval.tk_context = yypvt[-0].tk_context;
                  yypvt[-0].tk_context->cnext = yypvt[-2].tk_context;
                } break;
case 41:
# line 310 "sym.y"
{ if ( (yyval.tk_int = getattribute(yypvt[-0].tk_string)) == -1)
		  {
			yyerror("Unknown attribute %s",yypvt[-0].tk_string);
			yyval.tk_int = 0;
		  }
		  sfree(yypvt[-0].tk_string);
		} break;
case 42:
# line 320 "sym.y"
{ int d;
		  if ( (d = lookupval(yypvt[-0].tk_string,yypvt[-2].tk_int)) == DONTKNOW) {
			yyerror("illegal value of %s for %s",
				yypvt[-0].tk_string,name(yypvt[-2].tk_int));
			d = DONTKNOW;
		  }
		  sfree(yypvt[-0].tk_string);
		  if (type(yypvt[-2].tk_int) != DISCRETE)
		      yyerror("Attribute %s shouldn't be in an equality test",
				name(yypvt[-2].tk_int));
		  yyval.tk_context = mem_alloc(Context);
		  yyval.tk_context->ctest.attr = yypvt[-2].tk_int;
		  yyval.tk_context->ctest.no = unordvals(yypvt[-2].tk_int);
		  null_flags(yyval.tk_context->ctest.tsflags);
		  set_flag(yyval.tk_context->ctest.tsflags,attr);
		  yyval.tk_context->coutcome = d;
		} break;
case 43:
# line 338 "sym.y"
{ if (!num_type(yypvt[-2].tk_int))
			yyerror("attribute %s cannot compare",name(yypvt[-2].tk_int));
		  flt = fetch_float(yypvt[-0].tk_string); sfree(yypvt[-0].tk_string);
		  if (flt < getmin(yypvt[-2].tk_int) || flt > getmax(yypvt[-2].tk_int)) {
			yyerror("Value of %g out of range for %s",
					flt,name(yypvt[-2].tk_int));
		  }
		  yyval.tk_context = mem_alloc(Context);
		  yyval.tk_context->ctest.attr = yypvt[-2].tk_int;
		  yyval.tk_context->ctest.no = 2;
                  null_flags(yyval.tk_context->ctest.tsflags);
                  set_flag(yyval.tk_context->ctest.tsflags,cut);
		  yyval.tk_context->ctest.tval.cut = flt;
		  yyval.tk_context->coutcome = LESSTHAN;
		} break;
case 44:
# line 354 "sym.y"
{ if (!num_type(yypvt[-2].tk_int))
			yyerror("attribute %s cannot compare",name(yypvt[-2].tk_int));
		  flt = fetch_float(yypvt[-0].tk_string); sfree(yypvt[-0].tk_string);
		  if (flt < getmin(yypvt[-2].tk_int) || flt > getmax(yypvt[-2].tk_int))
				yyerror("Value of %g out of range for %s",
					flt,name(yypvt[-2].tk_int));
		  yyval.tk_context = mem_alloc(Context);
	          yyval.tk_context = mem_alloc(Context);
                  yyval.tk_context->ctest.attr = yypvt[-2].tk_int;
                  yyval.tk_context->ctest.no = 2;
		  null_flags(yyval.tk_context->ctest.tsflags);
                  set_flag(yyval.tk_context->ctest.tsflags,cut);
                  yyval.tk_context->ctest.tval.cut = flt;
                  yyval.tk_context->coutcome = GREATERTHAN;
		} break;
case 45:
# line 370 "sym.y"
{ yyval.tk_context = mem_alloc(Context);
		  yyval.tk_context->ctest.attr = yypvt[-0].tk_int;
		  yyval.tk_context->coutcome = DONTKNOW;
		} break;
case 46:
# line 377 "sym.y"
{
			if (max_prob != ndecs ) 
	    	 	     yyerror("incorrect number of true probabilities");
			true_prob = (double *) salloc(ndecs*sizeof(double));
			flt = 0.0;
			fordecs(yyii) {
			    if ( probs[yyii] <= 0.0 )
	    	 	        yyerror("true probabilities must be positive");
			    flt += true_prob[yyii] = probs[yyii];
			}
			if ( 0.99999 > flt || 1.00001 < flt )
	    	 	    yyerror("true probabilities sum to %lg", flt);
			else
			    fordecs(yyii) 
				probs[yyii] /= flt;
		} break;
case 47:
# line 394 "sym.y"
{
			if (max_prob != ndecs ) 
	    	 	     yyerror("incorrect number of stratified probabilities");
			strat_prob = (double *) salloc(ndecs*sizeof(double));
			flt = 0.0;
			fordecs(yyii) {
			    if ( probs[yyii] <= 0.0 )
	    	 	        yyerror("stratified probabilities must be positive");
			    flt += strat_prob[yyii] = probs[yyii];
			}
			if ( 0.99999 > flt || 1.00001 < flt )
	    	 	     yyerror("stratified probabilities sum to %lg", flt);
			else
			    fordecs(yyii)
				strat_prob[yyii] /= flt;
		} break;
case 48:
# line 413 "sym.y"
{
#define nextfield()   {  if ( field[2] ) field = field+2; else field = strtok((char*)0," \t"); }
			if ( read_prior )
			    for (   field = strtok(yypvt[-1].tk_string," \t");
                		field;
                		field = strtok((char*)0," \t")   ) {
				if ( field[0] != '-' || !field[1] ) {	
					yyerror("prior option %s not preceeded with a '-'",field);
					continue;
				}
				switch ( field[1] ) {
				case 'N' :
					if ( field[2] )
						yyerror("prior option overlaid on -N");
					prior_opts((int)'N',"");
					break;
				case 'A' :
					nextfield();
					prior_opts((int)'A',field);
					break;
				case 'P' :
					nextfield();
					prior_opts((int)'P',field);
					break;
				case 'd' :
					nextfield();
					prior_opts((int)'d',field);
					break;
				}
			}
		} break;
case 49:
# line 446 "sym.y"
{	
			if ( next_ut != ndecs ) 
	    	 	     yyerror("incorrect number of utilities");
			/*
			 * 	for binary class case
			 *	utilities transform to cut-off probabilities
			 */
			if (ndecs == 2) 
			    cut_prob = (utilities[0][0]-utilities[1][0]) /
				( utilities[0][0]-utilities[1][0] +
				  utilities[1][1]-utilities[0][1] );
			util_max = -FLOATMAX;
			util_min = FLOATMAX;
			fordecs(yyii) fordecs(yyij) {
				if ( utilities[yyii][yyij] > util_max )
					util_max = utilities[yyii][yyij];
				if ( utilities[yyii][yyij] < util_min )
                                        util_min = utilities[yyii][yyij];
			}
			
		} break;
case 50:
# line 468 "sym.y"
{
			if (max_prob != ndecs ) 
	    	 	     yyerror("incorrect number of utilities");
			utilities = (float **) salloc(ndecs*sizeof(float*));
			utilities[0] = (float *) salloc(ndecs*sizeof(float));
			fordecs(yyii)
				utilities[0][yyii] = probs[yyii];
			next_ut = 1;
		} break;
case 51:
# line 478 "sym.y"
{
			if ( max_prob != ndecs || next_ut>=ndecs ) 
	    	 	     yyerror("incorrect number of utilities");
			utilities[next_ut] = (float *) salloc(ndecs*sizeof(float));
			fordecs(yyii)
				utilities[next_ut][yyii] = probs[yyii];
			next_ut++;
		} break;
case 52:
# line 490 "sym.y"
{
			if ( !probs )
				probs = mems_alloc(double,ndecs);
			max_prob = 2; 
			probs[0] = fetch_float(yypvt[-2].tk_string);  sfree(yypvt[-2].tk_string);
	    	 	probs[1] = fetch_float(yypvt[-0].tk_string);  sfree(yypvt[-0].tk_string);
		 } break;
case 53:
# line 498 "sym.y"
{
			if ( max_prob >= ndecs ) {
	    	    		yyerror("too many probabilities");
				val_id = ndecs-1;
			}
	    		probs[max_prob++] = fetch_float(yypvt[-0].tk_string);  
			sfree(yypvt[-0].tk_string);
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}

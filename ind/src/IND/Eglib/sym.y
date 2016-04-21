/*
 *   IND 1.0 released 9/15/91   contact: wray@ptolemy.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 244-17, Moffett Field, CA 94035
 */
%{
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

%}


%start	symbols

%union {
	Context  *tk_context;
	int	 tk_int;
	char 	 *tk_string;
	}

%token	<tk_string>	IDENTIFIER STRING
%token	<tk_int>	CONTINUOUS
%token	AND ONLYIF NEVER CONTEXTS 
%token  STRATIFY UTILITIES ASFOR  PRIOR

%type	<tk_context>	context_lit context 
%type	<tk_int>	attrid

%%
symbols	:	attribute_defs 
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
		}
		other_defs
		{
		  sfree(val_names);
		  if (error_count>0)
			return 1;
		}
	;

other_defs :	/* nothing */
	|	other_defs contexts
	|	other_defs stratify
	|	other_defs utilities
	|	other_defs prior
	;

attribute_defs : attribute_def
	|	attribute_defs attribute_def
	;
attribute_def :	/*  nothing */
		{
			addattr = 0;
		}
		identifier_list ':' attribute_vals attr_qualifiers '.'
	    	{
		        nextattr += addattr;
		}
	|	error '.'
		{
	    	    	yyerror("can't read attribute definition");
		}
	;

identifier_list :	identifier_list IDENTIFIER
		{
			/*
			 *     attribute called "class" is the 
			 *     decision attribute
			 */
			if ( nextattr+addattr>=MAXATTRS ) {
	    	    		yyerror("too many attributes");
				return 1;
			}
			name(nextattr+addattr) = $2;
			if ( strcmp($2, "class") == 0 ||
				 strcmp($2, "Class") == 0 ) {
                            type(nextattr+addattr) = DECISION;
                            decattr = nextattr+addattr;
			} else
                            type(nextattr+addattr) = DISCRETE;
			unkns(nextattr+addattr) = FALSE;
			sym_fill(nextattr+addattr) = FALSE;
			onlyif(nextattr+addattr) = Cnil;
			addattr++;
		}
	 |
	 ;

attr_qualifiers :	'(' attr_qualifier_list ')'
	 |	
	 ;
attr_qualifier_list :	attr_qualifier ',' attr_qualifier_list
	 |		attr_qualifier
	 ;

attr_qualifier :	'?' 
		{
		        for ( yyii=0; yyii<addattr; yyii++)
				unkns(nextattr+yyii) = TRUE;
		}
         |      IDENTIFIER
		/*     	this attribute is the class  */
		{
		        if ( strcmp($1,"class") )
			       yyerror("unrecognized attribute qualifier");
			sfree($1);
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
		}
         |      IDENTIFIER '=' IDENTIFIER
		/*     	set options  */
		{
		        if ( !strcmp($1,"subset") && !sym_fill(nextattr) ) {
			  if ( unordvals(nextattr) == 2 )
			    yyerror("subsetting meaningless for binary attr.");
			  if ( !strcmp($3,SB_FULL_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_FULL; 
			  else if ( !strcmp($3,SB_ONE_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_ONE; 
			  else if ( !strcmp($3,SB_REST_STR) ) 
			    st[nextattr].u.unord->subsetting = SB_REST; 
			  else
			    yyerror("unknown option for subset flag");
			} else
			    yyerror("unknown flag in attribute qualifier");
			sfree($1);
			sfree($3);
		}
	 ;

attribute_vals : discrete_vals 
		{	
			for ( yyii=0; yyii<addattr; yyii++) {
			    if ( type(nextattr+yyii) == DECISION ) {
                                sindex(nextattr+yyii) = 0;
                            } else
                                sindex(nextattr+yyii) = (ndattrs++) + 1;
			}
			assign_val_id_to_unord();
		}
	|       '{' discrete_vals '}'
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
		}
	|	CONTINUOUS 
		{
			st[nextattr].u.ord = mem_alloc(ord_sym);
		}
		min_max
		{	
			sindex(nextattr) = ncattrs++;
			type(nextattr) = $1;
			for (yyii=1; yyii<addattr; yyii++) {
				sindex(nextattr+yyii) = ncattrs++;
				type(nextattr+yyii) = $1;
			        st[nextattr+yyii].u.ord = st[val_id].u.ord;
				st[nextattr+yyii].sflags = st[nextattr].sflags;
			}
		}
	|	ASFOR IDENTIFIER 
		{	
		        val_id = getattribute($2);
			if ( val_id < 0 ) {
				yyerror("identifier %s not already seen",$2);
				val_id = 0;
			}
			sfree($2);
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
		}
	;

min_max : 	IDENTIFIER '.' '.' IDENTIFIER
		{	
	    		getmin(nextattr) = fetch_float($1); sfree($1);
	    		getmax(nextattr) = fetch_float($4); sfree($4);
		}
	|	'?'
		{	
			sym_fill(nextattr) = TRUE;
	    		getmin(nextattr) = FLOATMAX;
	    		getmax(nextattr) = -FLOATMAX;
		}
	|
		{	
	    		getmin(nextattr) = -FLOATMAX;
	    		getmax(nextattr) = FLOATMAX;
		}
	;

/*  read discrete value names into "val_names"  */
discrete_vals :	IDENTIFIER 
	   	{
			val_id = 1; 
			val_fill = FALSE;
			val_names[0] = (char *) $1; 
		 }
	|	discrete_vals ',' IDENTIFIER
	    	{
			if ( val_id >= val_names_size  ) {
				val_names_size += 20;
				val_names = mems_realloc(val_names,char *, 
						val_names_size+1);
			}
	    		val_names[val_id++] = (char *) $3; 
		}
	|	discrete_vals ',' '?'
	    	{
			val_fill = TRUE;
		}
	|	'?'
		{
			val_fill = TRUE;
			val_id = 0;
		}
	;

contexts	:	CONTEXTS ':' context_defn
		|	contexts context_defn
		;
context_defn	:	attrid ONLYIF context '.'
			{
			  onlyif($1) = $3;
			}
		|	attrid NEVER '.'
			{
			  onlyif($1) = mem_alloc(Context);
			  onlyif($1)->coutcome = DONTKNOW;
			  onlyif($1)->ctest.attr = decattr;
			}
		|	error '.'
		{
	    	    	yyerror("can't read context definition");
		}
		;

context	: context_lit
                { $$ = $1; 
		  $1->cnext = 0; }
        | context AND context_lit
                {
                  $$ = $3;
                  $3->cnext = $1;
                }
        ;

attrid	: IDENTIFIER
		{ if ( ($$ = getattribute($1)) == -1)
		  {
			yyerror("Unknown attribute %s",$1);
			$$ = 0;
		  }
		  sfree($1);
		}
	;

context_lit : attrid '=' IDENTIFIER
		{ int d;
		  if ( (d = lookupval($3,$1)) == DONTKNOW) {
			yyerror("illegal value of %s for %s",
				$3,name($1));
			d = DONTKNOW;
		  }
		  sfree($3);
		  if (type($1) != DISCRETE)
		      yyerror("Attribute %s shouldn't be in an equality test",
				name($1));
		  $$ = mem_alloc(Context);
		  $$->ctest.attr = $1;
		  $$->ctest.no = unordvals($1);
		  null_flags($$->ctest.tsflags);
		  set_flag($$->ctest.tsflags,attr);
		  $$->coutcome = d;
		}
	| attrid '<' IDENTIFIER
		{ if (!num_type($1))
			yyerror("attribute %s cannot compare",name($1));
		  flt = fetch_float($3); sfree($3);
		  if (flt < getmin($1) || flt > getmax($1)) {
			yyerror("Value of %g out of range for %s",
					flt,name($1));
		  }
		  $$ = mem_alloc(Context);
		  $$->ctest.attr = $1;
		  $$->ctest.no = 2;
                  null_flags($$->ctest.tsflags);
                  set_flag($$->ctest.tsflags,cut);
		  $$->ctest.tval.cut = flt;
		  $$->coutcome = LESSTHAN;
		}
	| attrid '>' IDENTIFIER
		{ if (!num_type($1))
			yyerror("attribute %s cannot compare",name($1));
		  flt = fetch_float($3); sfree($3);
		  if (flt < getmin($1) || flt > getmax($1))
				yyerror("Value of %g out of range for %s",
					flt,name($1));
		  $$ = mem_alloc(Context);
	          $$ = mem_alloc(Context);
                  $$->ctest.attr = $1;
                  $$->ctest.no = 2;
		  null_flags($$->ctest.tsflags);
                  set_flag($$->ctest.tsflags,cut);
                  $$->ctest.tval.cut = flt;
                  $$->coutcome = GREATERTHAN;
		}
	| attrid
		{ $$ = mem_alloc(Context);
		  $$->ctest.attr = $1;
		  $$->coutcome = DONTKNOW;
		}
	;

stratify : STRATIFY ':' probabilities ';' 
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
		}
		probabilities '.'
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
		}
	

prior 	:  PRIOR ':' STRING  '.'
		{
#define nextfield()   {  if ( field[2] ) field = field+2; else field = strtok((char*)0," \t"); }
			if ( read_prior )
			    for (   field = strtok($3," \t");
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
		}

utilities : UTILITIES ':' utility_list '.'
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
			
		}
utility_list :  probabilities
		{
			if (max_prob != ndecs ) 
	    	 	     yyerror("incorrect number of utilities");
			utilities = (float **) salloc(ndecs*sizeof(float*));
			utilities[0] = (float *) salloc(ndecs*sizeof(float));
			fordecs(yyii)
				utilities[0][yyii] = probs[yyii];
			next_ut = 1;
		}
	|	utility_list ';' probabilities
		{
			if ( max_prob != ndecs || next_ut>=ndecs ) 
	    	 	     yyerror("incorrect number of utilities");
			utilities[next_ut] = (float *) salloc(ndecs*sizeof(float));
			fordecs(yyii)
				utilities[next_ut][yyii] = probs[yyii];
			next_ut++;
		}

		
/*  read list of ndecs continuous values into "probs"  */
probabilities :	IDENTIFIER ',' IDENTIFIER 
	   	{
			if ( !probs )
				probs = mems_alloc(double,ndecs);
			max_prob = 2; 
			probs[0] = fetch_float($1);  sfree($1);
	    	 	probs[1] = fetch_float($3);  sfree($3);
		 }
	|	probabilities ',' IDENTIFIER
	    	{
			if ( max_prob >= ndecs ) {
	    	    		yyerror("too many probabilities");
				val_id = ndecs-1;
			}
	    		probs[max_prob++] = fetch_float($3);  
			sfree($3);
		}
	;

%%

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

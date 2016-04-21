/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *	Symbol table routines for the IND system.
 *	The routines given here create, and allow you to access
 *	a symbol table containing information about the attributes.
 *
 *	The following information is stored about each attribute:
 *		- type (discrete, decision, numeric)
 *		- name
 *		- max and min values for numeric attributes
 *		- number of possible values and the name of
 *		  each value for discrete/decision attributes.
 *
 *	The following routines are available-
 *		create(order) : creates the symbol table and
 *			returns the number of attributes per
 *			example.
 *
 *		type(attr) : returns the type of attr
 *
 *		getmin(attr), getmax(attr) : return the min and
 *			max possible values for a cts attribute.
 *			Fail if called with non continuous attribute.
 *
 *		lookupval(val, attr) : if val is the name of a possible
 *			value of attr, return the number of the value,
 *			else return DONTKNOW.
 *			This applies only to non-cts attributes.
 *
 *		name(attr) : return the name of attr.
 *
 *		nvals(attr) : return number of values of attr. (#defined)
 *
 *		ndecs : return number of decision classes.
 *
 *		find(attr, val) : return name of attr's val.
 *
 *		getattribute(name) : return the number for the attr name.
 */

#define  SYMBOL
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "SYM.h"
#include "sets.h"
/*  #define  DEBUG  */

char *strcpy();

Symbol st[MAXATTRS];	/* The symbol table */
Header ld;		/* the loaded header file  */
double	*true_prob=0;	/* true class probab.s if found in ".attr" */
double	*strat_prob=0;	/* sample class probab.s if found in ".attr" */
double	*strat_true=0;	/* ratio of above (to save recalculating) */
float	**utilities=0;	/* utilities stored here if found in ".attr" */
float   util_max, util_min;  /*  of above  */
float	cut_prob=0.5;	/* choose pos. binary class if prob over this */
int	nvalsattr;	/* max. number of values for attribute */
int	nvalstest;	/* max. number of outcomes for test */
int     st_ndecs;       /*  no of decisions  */
bool	rev_strat_flg;	/* do reverse stratification when doing stats */
int     sym_checking=0; /*  checking level for reading egs:
			 *	1 = query user with each new problem
			 *	2 = accept everything as valid extension
			 */

/*
 *	Create a symbol table and read the information into it.
 *	The names of the attributes are in the order file, in the 
 *	form
 *		name \t <U|D> \t number of possible vals
 *	for discrete (U) or decision (D) attributes
 *	OR
 *		name \t <C|N|E|S|P> \t min \t max
 *	for continuous attributes.
 *	The possible value names for each non-cts attribute must be
 *	stored in a file with the same name as the attribute. These
 *	values must be SORTED.
 */

FILE	*fp;			/* Used for the order file	   */

void create(order)
string order;
{
	int	res;
	int	i;
	ld.h_magicno = MAGICNO;
	fp = efopen(order, "r");
	res = parse_symbols(fp);
	if ( num_type(decattr) )
		uerror("decision attribute is not discrete\n","");
	/*      size of eg. storage in aligned chars */
	egsize = eg_storage;
	/*      array offset for set/continuous attrs - see "sets.h"   */
        forattrs(i) {
	     if ( num_type(i) )
		sindex(i) += eg_coffset;
	     else if ( set_type(i) )
		sindex(i) += eg_soffset;
	}
	no_sample(&ld.h_sampler);
#	ifdef DEBUG
	{
		int	j;
		fprintf(stdrep, "ndattrs = %d\n", ndattrs );
		fprintf(stdrep, "ncattrs = %d\n", ncattrs );
		fprintf(stdrep, "nsattrs = %d\n", nsattrs );
		fprintf(stdrep, "nattrs = %d\n", nattrs );
		fprintf(stdrep, "eg_storage = %d\n", eg_storage );
		fprintf(stdrep, "eg_coffset = %d\n", eg_coffset );
		fprintf(stdrep, "eg_soffset = %d\n", eg_soffset );
		fprintf(stdrep, "Symbol Table\n");
		fprintf(stdrep, "============\n");
		for(i=0; i<=nattrs; i++) {
			fprintf(stdrep, "%s\t%c\t%d", 
				name(i), type(i), sindex(i));
			if ( num_type(i) )
				fprintf(stdrep, "\t%g\t%g\n", 
				   getmin(i), getmax(i) );
			else {
				fprintf(stdrep, "\t%d\n\t", unordvals(i) );
				for (j=0; j<unordvals(i); j++)
				   fprintf(stdrep, " %s", unordvalname(i,j) );
				fprintf(stdrep, "\n\t");
				for (j=0; j<unordvals(i); j++)
					fprintf(stdrep, " %s", 
					   unordvalname(i,unordind(i)[j] ) );
				fprintf(stdrep, "\n");
			}
		}
	}
#	endif
	if (!res )
		uerror("Parse of attribute file failed\n","");
	if ( strat_prob && true_prob ) {
		int i;
		strat_true = (double *) salloc(ndecs*sizeof(double));
		fordecs(i)
			strat_true[i] = strat_prob[i] / true_prob[i];
	}
}

/*
 *        assume a lookup to the string hash table has just failed,
 *        now add the value!
 */
extend_attr(val,attr)
char  *val;
int   attr;
{
      char   *cpy_val = mems_alloc(char,strlen(val)+1);
      strcpy(cpy_val,val);
      unordvals(attr)++;
      if ( unordstrs(attr) ) {
	unordstrs(attr) = mems_realloc(unordstrs(attr),char*,unordvals(attr));
        unordstrs(attr)[unordvals(attr)-1] = cpy_val;
        add_hash(unordhash(attr),unordvals(attr)-1);
      } else {
	unordstrs(attr) = mems_alloc(char*,1);
        unordstrs(attr)[0] = cpy_val;
        unordhash(attr) = fill_hash(&unordstrs(attr), 1);
      }
}
  
#ifdef DUPLI
/*
 *        assume a lookup to the string hash table has just failed,
 *        now add the value!
 */
duplic_attr(val,attr,old_val)
char  *val;
int   attr;
int   old_val;
{
      char   *cpy_val = mems_alloc(char,strlen(val)+1);
      error("duplic_attr() not written","");
      strcpy(cpy_val,val);
      unordstrs(attr) = mems_realloc(unordstrs(attr),char*,unordvals(attr));
      unordstrs(attr)[unordvals(attr)-1] = cpy_val;
      add_hash(unordhash(attr),unordvals(attr)-1);
}
#endif
     
/*
 *	Symbol table access routines.
 *	Many have been #defined in SYM.h for efficiency reasons.
 */

char   *enc_set_delim = ",} \t";

/*
 *	find the index represented by the string "val" for attribute;
 *	if some problem, then take appropriate action,
 *	add new attribute value, change given, ignore, etc.
 */
int strchkval(val, attr)
char  *   val;
int      attr;
{
	char  *strtok();
	char  act, buf[150];
	int  bit;
        if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	    if ( sym_fill(attr) ) {
	      if ( sym_checking==1 ) {
	        fprintf(stdrep,"\nUnrecognized value \"%s\"", val);
	        fprintf(stdrep," for attribute \"%s\"\n", name(attr));
	again:
	        fprintf(stdrep,"Action ('h' for help): ");
		fflush(stdrep);
	        scanf(" %c",&act);
	        switch ( act ) {
	        case 'h' :
	        case '?' :
	        case 'H' :
	      	  fprintf(stdrep,"a = accept as new value \n");
	      	  fprintf(stdrep,"c new = change to value \"new\"\n");
#ifdef DUPLI
	      	  fprintf(stdrep,"d new = change this and future occurrences to value \"new\"\n");
#endif
	      	  fprintf(stdrep,"i = take this value as an error\n");
	      	  fprintf(stdrep,"l = list current values\n");
	          goto again;
		case 'a' :
		case 'A' :
		  extend_attr(val,attr);
		  return unordvals(attr)-1;
		case 'c' :
		case 'C' :
	          scanf("%s",&buf[0]);
          	  if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	      	    fprintf(stdrep,"don't recognise value %s\n",&buf[0]);
		    goto again;
		  }
		  return bit;
#ifdef DUPLI
		case 'd' :
		case 'D' :
	          scanf("%s",&buf[0]);
          	  if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	      	    fprintf(stdrep,"don't recognise value %s\n",&buf[0]);
		    goto again;
		  }
		  duplic_attr(val,attr,bit);
		  return bit;
#endif
		case 'i' :
		case 'I' :
		  return DONTKNOW;
		case 'l' :
		case 'L' :
              	  fprintf(stdrep,"%s: ",name(attr));
		  print_anames(stdrep, attr, 2+strlen(name(attr)) );
		  goto again;
	        default :
		  goto again;
		}
	      } else if ( sym_checking==2 ) {
		extend_attr(val,attr);
		return unordvals(attr)-1;
	      } else
		return DONTKNOW;
	    } else
	      return DONTKNOW;
	} else
	    return bit;
}
/*
 *	find the set represented by the string "val" for attribute;
 *	if some problem, then return unknown,  i.e. no checking
 */
bitset lookupset(val, attr)
char  *   val;
int      attr;
{
	bitset set;
	char  *strtok();
	int	bit;
	set_copy(set,EMPTY_SET);
	/*
	 *	first skip over initial '{'
	 */
	for (  ;  *val && *val != '{' ; val++) ;
	if ( *val )
		val++;
	for ( val = strtok(val,enc_set_delim); val ;
           	val = strtok(0,enc_set_delim) ) {
          if ( (bit = lookupval(val, attr)) == DONTKNOW) 
		return SDKNOW;
	  else
		set_bit(set, bit);
	}
	return set;
}

/*
 *	find the set represented by the string "val" for attribute;
 *	if some problem, then take appropriate action,
 *	add new set element, change item, ignore, etc.
 */
bitset strchkset(val, attr)
char  *   val;
int      attr;
{
	bitset set;
	char  *strtok();
	char  act, buf[150];
	int	bit;
	set_copy(set,EMPTY_SET);
	/*
	 *	first skip over initial '{'
	 */
	for (  ;  *val && *val != '{' ; val++) ;
	if ( *val )
		val++;
	for ( val = strtok(val,enc_set_delim); val ;
           	val = strtok(0,enc_set_delim) ) {
          if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	    if ( sym_fill(attr) ) {
	      if ( sym_checking==1 ) {
	        fprintf(stdrep,"\nUnrecognized set element \"%s\"", val);
	        fprintf(stdrep," for attribute \"%s\"\n", name(attr));
	again:
	        fprintf(stdrep,"Action ('h' for help): ");
		fflush(stdrep);
	        scanf(" %c",&act);
	        switch ( act ) {
	        case 'h' :
	        case 'H' :
	        case '?' :
	      	  fprintf(stdrep,"a = accept as new set element \n");
	      	  fprintf(stdrep,"c new = change to set element \"new\"\n");
#ifdef DUPLI
	      	  fprintf(stdrep,"d new = change this and future occurrences to set element \"new\"\n");
#endif
	      	  fprintf(stdrep,"i = take this set element as an error\n");
	      	  fprintf(stdrep,"l = list current set elements\n");
	          goto again;
		case 'a' :
	        case 'A' :
		  extend_attr(val,attr);
		  set_bit(set,unordvals(attr)-1);
		  break;
		case 'c' :
	        case 'C' :
	          scanf("%s",&buf[0]);
          	  if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	      	    fprintf(stdrep,"don't recognise set element %s\n",&buf[0]);
		    goto again;
		  }
		  set_bit(set,bit);
		  break;
#ifdef DUPLI
		case 'd' :
	        case 'D' :
	          scanf("%s",&buf[0]);
          	  if ( (bit = lookupval(val, attr)) == DONTKNOW) {
	      	    fprintf(stdrep,"don't recognise set element %s\n",&buf[0]);
		    goto again;
		  }
		  duplic_attr(val,attr,bit);
		  set_bit(set,bit);
		  break;
#endif
		case 'i' :
	        case 'I' :
		  return SDKNOW;
		case 'l' :
	        case 'L' :
              	  fprintf(stdrep,"%s: ",name(attr));
		  print_anames(stdrep, attr, 2+strlen(name(attr)) );
		  goto again;
		default:
		  goto again;
		}
	      } else if ( sym_checking==2 ) {
		extend_attr(val,attr);
		set_bit(set,unordvals(attr)-1);
	      } else
	        return SDKNOW;
	    } else
	      return SDKNOW;
	  } else
	    set_bit(set,bit);
	}
	return set;
}

/*
 *    display set on file "outf"
 */
write_set(outf, sval, attr)
FILE  *outf;
bitset sval;
int	attr;
{
        int   i;
	int   first=1;
        bitset temp;
        fprintf(outf, "{");
        forinset(i,sval,temp)
	  if ( first ) {
		first = 0;
		fprintf(outf, "%s", unordvalname(attr, i));
	  } else
            fprintf(outf, ",%s", unordvalname(attr, i));
        fprintf(outf, "}");
}

static	char	*dk_ = "?";

char
*find(attr, val)
int	attr;		/* assuming that  type(attr) != CTS,NORM,... */
int	val;
{
	if (val == DONTKNOW)
		return dk_;
	else if (val < 0 || val >= unordvals(attr))
	{	fprintf(stdrep, "attr %d = %d\n", attr, val);
		uerror("find: Illegal value for attribute\n","");
		return 0;
	}
	else
		return unordstrs(attr)[val];
}

int getattribute(astr)
char	*astr;
{
	register int	a;

		/*attributes not ordered, so must do linear search*/
	for (a = nattrs; (a>=0 && strcmp(astr,name(a))); --a);
	return a;
}

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <assert.h>
#include "SYM.h"
#include "sets.h"
/*  #define  DEBUG  */

int print_anames(fp,attr,strt_len)
FILE   *fp;
int  attr;
int  strt_len;
{
	int  i,j, ll;
	if ( !unordvals(attr) ) {
		fprintf(fp,"?");
		return strt_len+1;
	}
	if ( strt_len > 70 ) {
		fprintf(fp,"\n\t");
		strt_len = 8;
        }
        for (j=0; j<unordvals(attr)-1; ) {
	    if ( j ) {
		fprintf(fp,"\n\t");
	        ll = 8;
	    } else
		ll = strt_len;
            for (i=j; i<unordvals(attr)-1 && ll<80; i++) 
		ll += strlen(unordvalname(attr,i))+1;
            for (; j<i; j++) 
	        fprintf(fp,"%s,",unordvalname(attr,j));
	}
	if ( ll+strlen(unordvalname(attr,j))+1>= 80 ) {
		fprintf(fp,"\n\t");
		ll = 8;
	}
	ll += strlen(unordvalname(attr,j));
	fprintf(fp,"%s",unordvalname(attr,j));
	return ll;
}

/*
 *    prints the symbol table as it currently is
 *    in ".attr"  form
 *    NB.   hard codes lots of conventions in "sym.y"
 */
void print_sym(order)
char  *order;
{
	int	i, j, attr, next_attr;
	int     line_len;	/*  length of current output line */
	FILE    *fp;
	bool    start_quals;
	if ( !order || !*order )
	  fp = stdrep;
	else
	  fp = efopen(order, "w");
	for ( attr = 0 ; attr<=nattrs; attr=next_attr) {
	  line_len = 0;
	  /*
	   *   find which run of attributes to group together
	   */
	  for ( i=attr+1; i<=nattrs; i++) {
	    if ( num_type(i) ) {
	      if ( type(i) != type(attr) ||
		   st[i].u.ord!=st[attr].u.ord ||
		   int_flags(st[i].sflags)!=int_flags(st[attr].sflags) )
		  break;
	    } else if ( type(i) != type(attr) ||
		        st[i].u.unord!=st[attr].u.unord ||
		        int_flags(st[i].sflags)!=int_flags(st[attr].sflags) )
	      break;
	  }
	  next_attr = i;
	  /*
	   *       print the related attribute names
	   */
	  for (j=attr; j<next_attr; j++) {
	    if ( line_len > 73 ) {
	      fprintf(fp,"\n\t");
	      line_len = 8;
	    }
	    fprintf(fp,"%s ",name(j));
	    line_len += strlen(name(j)) + 1;
	  }
	  fprintf(fp,":\t");
	  line_len = ((line_len+9)/8 + 1 ) * 8;
	  if ( line_len > 70 ) {
	      fprintf(fp,"\n\t");
	      line_len = 8;
	  }
	  /*
	   *       find if previous attributes are equivalent
	   */
	  for ( i=0; i<attr; i++) {
	    if ( num_type(i) ) {
	      if ( type(i) == type(attr) &&
		   st[i].u.ord==st[attr].u.ord &&
		   int_flags(st[i].sflags)==int_flags(st[attr].sflags) )
		  break;
	    } else if ( type(i) == type(attr) &&
		        st[i].u.unord==st[attr].u.unord &&
		        int_flags(st[i].sflags)==int_flags(st[attr].sflags) )
	      break;
	  }
	  /*
	   *       print the attribute values
	   */
	  if ( i<attr ) {
	    fprintf(fp,"asfor %s", name(i));
	    line_len += 6 + strlen(name(i));
	  } else if ( type(attr)==DISCRETE || type(attr)==DECISION 
		       || type(attr)==SETTYPE )	 {
	    if ( set_type(attr) ) {
	      fprintf(fp,"{ ");
	      line_len += 2;
	    }
	    line_len = print_anames(fp,attr,line_len);
	    if ( set_type(attr) ) {
	      fprintf(fp," }");
	      line_len += 2;
	    }
	  } else if ( num_type(attr) ) {
	    fprintf(fp,"cont ");
	    line_len += 5;
	    if ( getmin(attr) > -FLOATMAX && getmax(attr) < FLOATMAX ) {
	      fprintf(fp,"%g..%g",  getmin(attr), getmax(attr));
	      line_len += 12;
	    }
	  } 
	  /*
	   *       print the attribute_qualifiers
	   */
	  if ( line_len > 70 ) {
	      fprintf(fp,"\n\t");
	      line_len = 8;
	  }
	  start_quals = FALSE;
	  if ( unkns(attr) ) {
	    fprintf(fp," (?");
	    start_quals = TRUE;
	  }
	  if ( type(attr) == DISCRETE && unordsubsets(attr) ) {
	      if (start_quals)
	        fprintf(fp,",");
	      else {
	        fprintf(fp," (");
	        start_quals = TRUE;
	      }
	      fprintf(fp,"subset=");
	      if ( unordsubsets(attr)==SB_FULL )
		fprintf(fp,SB_FULL_STR);
	      else if ( unordsubsets(attr)==SB_ONE )
		fprintf(fp,SB_ONE_STR);
	      else
		fprintf(fp,SB_REST_STR);
	  }
          if ( type(attr) == DECISION && strcmp(name(attr),"class") ) {
	    if (start_quals)
	      fprintf(fp,",class");
	    else {
	      fprintf(fp," (class");
	      start_quals = TRUE;
	    }
	  }
	  if (start_quals)
	      fprintf(fp,")");	 
	  fprintf(fp,".\n");	
        }
        fprintf(fp,"| rest of the attribute file, utilities+contexts, same\n");
}

/*
 *    prints the symbol table as it currently is
 *    in Quinlan's ".names"  form
 *    NB.   hard codes lots of conventions in "sym.y"
 */
void print_c4sym(order)
char  *order;
{
        int     attr;
        FILE    *fp;
        int     line_len;
        if ( !order || !*order )
          fp = stdrep;
        else
          fp = efopen(order, "w");
        line_len = print_anames(fp,decattr,0);
        fprintf(fp,".\n");
        for ( attr = 0 ; attr<=nattrs; attr++) {
          if ( attr == decattr)
                continue;
          fprintf(fp,"%s: ",name(attr));
          /*
           *       print the attribute values
           */
          if ( type(attr)==DISCRETE || type(attr)==DECISION
                       || type(attr)==SETTYPE )  {
            line_len = print_anames(fp,attr,0);
            if ( type(attr)==SETTYPE )
                uerror("no set types in C4.5","");
          } else if ( num_type(attr) ) {
            fprintf(fp,"continuous");
          }
          fprintf(fp,".\n");
        }
	if ( order && *order )
		fclose(fp);
}


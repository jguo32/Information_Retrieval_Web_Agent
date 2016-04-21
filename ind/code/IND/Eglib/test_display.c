/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#define  TESTED
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


/*
 *	These routines display the test structures defined in SYM.h.
 */

/*
 *    display the test alone
 */
display_test(tester,fp)
register  test_rec  *tester;
FILE	*fp;
{
	register	int i;
	register int	at;
	register	bitset temp;
	if ( !tester ) 
        	fprintf(fp,"%s ?", name(decattr));
	else 
	if ( cut_test(tester) ) {
                fprintf(fp, "%s <,>= %.5g ?", 
			name(tester->attr), tester->tval.cut);
	} else if ( set_test(tester) ) {
	  if ( subset_test(tester) ) {
	    ASSERT(clean_set(tester->tval.valset,unordvals(tester->attr)) )
	    fprintf(fp, "%s super ", name(tester->attr) );
	    write_set(fp, tester->tval.valset, (int)tester->attr);
	  } else {
	    ASSERT ( remdr_test(tester) );
	    set_clean(tester->tval.valset,unordvals(tester->attr));
            fprintf(fp, "%s inter ", name(tester->attr) );
            write_set(fp, tester->tval.valset, (int)tester->attr);
	  }
	} else if ( subset_test(tester) ) {
	  ASSERT( clean_set(tester->tval.valset,unordvals(tester->attr)) )
	  if ( remdr_test(tester) ) {
	    fprintf(fp,"%s = ", name(at = tester->attr));
	    foroutset(i,tester->tval.valset,temp,unordvals(tester->attr))
	      fprintf(fp,"%s,", unordvalname(at, i));
	    fprintf(fp,"or other ?");
	  } else {
	    fprintf(fp,"%s in ", name(at = tester->attr));
	    forinset(i,tester->tval.valset,temp)
	      fprintf(fp,"%s,", unordvalname(at, i));
	    fprintf(fp,"\b ");
	    fprintf(fp," ?");
	  } 
        } else if ( bigsubset_test(tester) ) 
	  if ( remdr_test(tester) ) {
	    fprintf(fp,"%s = ", name(at = tester->attr));
	    forunordvals(i,at) 
	      if ( ! bitarray_set(tester->tval.valbigset,i) )
		fprintf(fp,"%s,", unordvalname(at, i));
	    fprintf(fp," or others ?");
	  } else {
	    fprintf(fp,"%s in ", name(at = tester->attr));
	    forunordvals(i,at) 
	      if ( bitarray_set(tester->tval.valbigset,i) )
		fprintf(fp,"%s,", unordvalname(at, i));
	    fprintf(fp,"\b ");
	    fprintf(fp," ?");
	  }
	else {
	  fprintf(fp,"%s ?", name(tester->attr));
	}
}	

/*
 *    display the test and the outcome of the test
 */
display_outcome(tester,outcome,fp)
register  test_rec  *tester;
int	outcome;
FILE	*fp;
{
	register	int i;
	register	bitset temp;
	register int	at;
	if ( !tester ) {
        	fprintf(fp,"%s in ", name(decattr));
		forinset(i,tester->tval.valset,temp)
        		fprintf(fp,"%s,", unordvalname(decattr, i));
		fprintf(fp,"\b ");
        	fprintf(fp,": ");
	} else if ( cut_test(tester) )  {
                if (outcome==LESSTHAN)
                	fprintf(fp, "%s < %.5g: ", 
				name(tester->attr), tester->tval.cut);
                else
                	fprintf(fp, "%s >= %.5g: ", 
				name(tester->attr), tester->tval.cut);
	} else if ( set_test(tester) ) {
	  /*
	   *	SETTYPE test
	   */
	  fprintf(fp,"%s ", name(tester->attr) );
	  if ( subset_test(tester) ) {
		if ( outcome )
			fprintf(fp,"super ");
		else
			fprintf(fp,"~super ");
	  } else {
		ASSERT ( remdr_test(tester) );
		if ( outcome )
			fprintf(fp,"inter ");
		else
			fprintf(fp,"~inter ");
	  }
	  write_set(fp, tester->tval.valset, (int)tester->attr);
	  fprintf(fp,": ");
	} else if ( subset_test(tester) ) {
	  /*
	   *	DISCRETE test
	   */
	  if ( remdr_test(tester) ) 
                if ( outcome ) {
                       fprintf(fp,"%s = %s: ", name(tester->attr),
                               unordvalname(tester->attr, 
                                  getset_nth(tester->tval.valset,outcome) ) );

		} else {	    
        		fprintf(fp,"%s in ", name(at=tester->attr));
			forinset(i,tester->tval.valset,temp)
        			fprintf(fp,"%s,", unordvalname(at, i));
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		}
	  else 
		if ( outcome ) {
        		fprintf(fp,"%s in ", name(at=tester->attr));
			forinset(i,tester->tval.valset,temp)
        			fprintf(fp,"%s,", unordvalname(at, i));
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		} else {
        		fprintf(fp,"%s in ", name(at=tester->attr));
			foroutset(i,tester->tval.valset,temp, unordvals(at)) {
				if ( i>= unordvals(at) )
					break;
        			fprintf(fp,"%s,", unordvalname(at, i));
			}
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		}
	} else if ( bigsubset_test(tester) ) {
	  if ( remdr_test(tester) ) 
                if ( outcome ) {
                       fprintf(fp,"%s = %s: ", name(tester->attr),
                            unordvalname(tester->attr, 
                                bitarray_nth(tester->tval.valbigset,outcome) ));
		} else {	    
        		fprintf(fp,"%s in ", name(at=tester->attr));
			forunordvals(i,at) 
			  if ( bitarray_set(tester->tval.valbigset,i) )
        			fprintf(fp,"%s,", unordvalname(at, i));
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		}
	  else 
		if ( outcome ) {
        		fprintf(fp,"%s in ", name(at=tester->attr));
			forunordvals(i,at) 
			  if ( bitarray_set(tester->tval.valbigset,i) )
        			fprintf(fp,"%s,", unordvalname(at, i));
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		} else {
        		fprintf(fp,"%s in ", name(at=tester->attr));
			forunordvals(i,at) 
			  if ( ! bitarray_set(tester->tval.valbigset,i) )
        			fprintf(fp,"%s,", unordvalname(at, i));
			fprintf(fp,"\b ");
        		fprintf(fp,": ");
		}
	} else
        	fprintf(fp,"%s = %s: ", name(tester->attr),
			unordvalname(tester->attr, outcome));
}

/*
 *    display the outcome of the test, without showing the test
 */
display_result(tester,outcome,fp)
register  test_rec  *tester;
int	outcome;
FILE	*fp;
{
	if ( !tester )  {
        	fprintf(fp,"%s: ", unordvalname(decattr, outcome));
	} else if ( cut_test(tester) )  {
                if (outcome==LESSTHAN)
                	fprintf(fp, "<: ");
                else
                	fprintf(fp, "%>=: ");
	} else if ( set_test(tester) ) {
	  /*
	   *	SETTYPE test
	   */
	  if ( subset_test(tester) ) {
		if ( outcome )
			fprintf(fp,"super: ");
		else
			fprintf(fp,"~super: ");
	  } else {
		ASSERT ( remdr_test(tester) );
		if ( outcome )
			fprintf(fp,"inter: ");
		else
			fprintf(fp,"~inter: ");
	  }
	} else if ( !remdr_test(tester) && subsets_test(tester) ) {
		if ( outcome )
        		fprintf(fp,"in: ");
		else 
        		fprintf(fp,"out: ");
	} else if ( remdr_test(tester) ) {
	  if ( !outcome )
	    fprintf(fp,"other: ");
	  else if ( subset_test(tester) )
	    fprintf(fp,"%s: ", unordvalname(tester->attr, 
				    getset_nth(tester->tval.valset,outcome) ) );
	  else if ( bigsubset_test(tester) )
	    fprintf(fp,"%s: ", unordvalname(tester->attr, 
				 bitarray_nth(tester->tval.valbigset,outcome)));
	} else {
        	fprintf(fp,"%s: ", unordvalname(tester->attr, outcome));
	}
}


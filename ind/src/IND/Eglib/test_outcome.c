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
 *	returns outcome of test on attribute (see also #defines in SYM.h)
 *		-1 for unknown, 0-nvals otherwise
 */
int outcome(eg,tester)
register test_rec  *tester;
register egtype 	eg;
{
	int     unordval;
	float	ordval;
	int	card;
	if ( !tester ) 
		return class_val(eg);
	if ( cut_test(tester) ) 
		if ( (ordval=ord_val(eg,tester->attr))==FDKNOW )
			return -1;
	    	else return (ordval<tester->tval.cut) ?
				LESSTHAN : GREATERTHAN;
	else if ( set_test(tester) ) {
	    /*
	     *    SETTYPE test
	     */
	    if ( subset_test(tester) ) {
		/*
		 *  is tval.valset a subset of set_val
 		 */
		if ( set_sub(tester->tval.valset, set_val(eg,tester->attr),
			unordvals(tester->attr) )  )
			return 1;
		else
			return 0;
	    } else if ( remdr_test(tester) ) {
		/*
		 *  does tval.valset intersect with set_val
 		 */
		if ( set_inter(tester->tval.valset, set_val(eg,tester->attr)))
			return 1;
		else
			return 0;
	    }
	} else {
	    /*
	     *    DISCRETE test
	     */
	    if ( (unordval=unord_val(eg,tester->attr))==DONTKNOW )
			return -1;
	    else if ( subset_test(tester) ) {
	      if ( remdr_test(tester) )
	        if ( bit_set(tester->tval.valset,unordval) )
		    return 0;
	        else 
		    return unordval - 
			   set_cardless(tester->tval.valset,unordval) + 1;
	      else
		    return bit_set(tester->tval.valset,unordval) ? 1 : 0;
	    } else if ( bigsubset_test(tester) ) {
	      if ( remdr_test(tester) )
	        if ( bitarray_set(tester->tval.valbigset,unordval) )
		    return 0;
	        else  {
		    bitarray_cardless(card,tester->tval.valbigset,unordval);	
		    return unordval - card + 1;
	        }
	      else
		return bitarray_set(tester->tval.valbigset,unordval) ? 1 : 0;
	    } else
		return unordval;
	}
	return -1;
}


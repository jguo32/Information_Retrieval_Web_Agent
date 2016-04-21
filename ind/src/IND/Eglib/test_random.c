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
 *	These routines access the test structures defined in SYM.h.
 */

/* 
 *	assume outcome of test is unknown, assign value probabilistically
 *      according to proportions for tester,
 *      return outcome eventually assigned
 */
int random_outcome(tester)
register test_rec  *tester;
{
	register int	nv;
	if ( !tester ) 
		return -1;
	if ( (nv=outcomes(tester))==2 ) {
	  if ( frandom() <= tester->tprop.prop_true )
		 return 0;
	  else return 1;
	} else 
	  return vrandom(tester->tprop.prop_vec, nv);
}


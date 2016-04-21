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
 *	These routines handle memory access for the "test"
 */

/*
 *	return copy, if salloc fails, return 0
 */
test_rec *
copy_test(tester)
register test_rec *tester;
{
	test_rec  *new;
	register test_rec *tsp;
	
	if ( !tester )
		return (test_rec *)0;
	tsp = new = (test_rec *)salloc(sizeof(test_rec));
	if ( !tsp )
		return (test_rec *)0;
        memcpy(tsp,tester,sizeof(test_rec));
	if ( bigsubset_test(tester) )
	  tsp->tval.valbigset = copy_bitarray(tester->tval.valbigset);
	return new;
}

free_test(tester)
test_rec *tester;
{
	if ( bigsubset_test(tester) )
		  free_bitarray(tester->tval.valbigset);
	sfree(tester);
}


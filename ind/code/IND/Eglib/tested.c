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

/* #define DEBUG_TESTED */

#ifdef DEBUG_TESTED
#include <stdio.h>
#endif

/*
 *	These routines maintain arrays that tells which attributes have
 *	been tested in the current branch of the tree. Continuous attributes 
 *      are never counted as having been tested.
 *	The routines are
 *		add_test(tester,outcome,testing) -- indicate test made
 *		rem_test(tester,outcome,testing) -- unindicate test unmade
 *              merge_test(testing1,testing2)
 *		null_test() -- create, set all attributes to be untested.
 *		cannot_test(tester,testing) -- return true if tester can't be 
 *				tested either because its already tested or
 *				context restrictions prevent testing it
 *		is_tested(attr) -- return true if attr has been tested
 *				so no degrees-freedom left
 *		testchcs() -- return no. of possible tests at current level
 */

/*
 *	initialize test-value storage to look like an example, and return
 *	for use by local routine
 */
egtesttype null_test(testing)
egtesttype  testing;
{
        int  i;
	forattrs(i)  {
	  if ( !num_type(i) && !set_type(i) ) {
	    if ( !Cnull(onlyif(i)) && Cnever(onlyif(i)) )
	      /*   context always fails so set to any old value */
	      unord_val(testing,i) = 0;
	    else
	      unord_val(testing,i) = DONTKNOW;
	  }
	}
	return testing;
}
egtesttype init_test()
{
	egtesttype  testing;
        int  i;
        testing.unordp = (unordtype *) salloc((ndattrs+1)*sizeof(float));
	forattrs(i)  {
	  if ( !num_type(i) && !set_type(i) ) {
	    if ( !Cnull(onlyif(i)) && Cnever(onlyif(i)) )
	      /*   context always fails so set to any old value */
	      unord_val(testing,i) = 0;
	    else
	      unord_val(testing,i) = DONTKNOW;
	  }
	}
	return testing;
}

void add_test(tester,outcome,testing)
register test_rec  *tester;
int	outcome;
egtesttype  testing;
{
        int   card;
	if ( ! discrete_test(tester) )
		return;
	if ( attr_test(tester) ) {
	    unord_val(testing,tester->attr) = outcome;
	} else if ( subset_test(tester) )  {
	  if ( remdr_test(tester) ) {
	    if (outcome) {
	        unord_val(testing,tester->attr) = 
		  getset_nth(tester->tval.valset,outcome);
	    }
	  } else {
	    if (outcome) {
		if ( singleton(tester->tval.valset) )
		  unord_val(testing,tester->attr) = 
		    getset_nth(tester->tval.valset,1);
	    } else {
		if ( singleton(set_comp(tester->tval.valset,
				unordvals(tester->attr))) )
		  unord_val(testing,tester->attr) = 
		    getset_nth(tester->tval.valset,1);
	    }
	  }
	} else if ( bigsubset_test(tester) ) {
	  if ( remdr_test(tester) ) {
            if (outcome) {
                unord_val(testing,tester->attr) = 
		  bitarray_nth(tester->tval.valbigset,outcome);
	    }
	  } else {
	    if (outcome) {
		bitarray_card(card,tester->tval.valbigset);
		if ( card==1 )
		  unord_val(testing,tester->attr) = 
		    bitarray_nth(tester->tval.valbigset,1);
	    } 
	  }
	}
}

void rem_test(tester,outcome,testing)
test_rec	*tester;
int	outcome;
egtesttype  testing;
{
	  if ( ! discrete_test(tester) )
		return;
	  if ( Cnull(onlyif(tester->attr)) ) 
	    unord_val(testing,tester->attr) = DONTKNOW;
          else if ( !Cnever(onlyif(tester->attr)) )
	    unord_val(testing,tester->attr) = DONTKNOW;
}

egtesttype  merge_tests(testing1,testing2)
egtesttype  testing1, testing2;
{
  int  i;
  egtesttype newtesting = init_test();
  for ( i=1; i<=ndattrs; i++)
    if (unord_val_s(testing1,i)==unord_val_s(testing2,i))
	unord_val_s(newtesting,i)=unord_val_s(testing2,i);	
    else
	unord_val_s(newtesting,i) = DONTKNOW;
  return newtesting;
}

/* 
 *	returns TRUE when attribute 'a' has been testing 
 *		so no degrees of freedom left
 */
bool is_tested(a,testing)
register int	a;
egtesttype  testing;
{
	if ( num_type(a) || set_type(a) )
		return FALSE;
	else if (unord_val(testing,a)==DONTKNOW)
		return FALSE;
	else
		return TRUE;
}

/* 
 *	tests context against current node
 */
bool in_context(context,testing)	/* returns TRUE if context holds*/
Context *context;
egtesttype  testing;
{
	while ( context!= Cnil ) {
          if ( Cnever(context) ) return FALSE;
	  if ( context->coutcome != outcome(testing,&(context->ctest)) )
	    return FALSE;
	  context = context->cnext;
        }
        return TRUE;
}

/* 
 *	returns TRUE when attribute 'a' has been tested, or
 *	       when 'a' is 'out of context', or when no more spaces in "testing".
 */
bool cannot_test(tester,testing)
test_rec	*tester;
egtesttype  testing;
{
	register int a = tester->attr;
	if (is_tested(a,testing))
	    return TRUE;	/* has been used already - can't use again */
	else if ( onlyif(a) ) {
	    if ( in_context(onlyif(a),testing) )
		return FALSE;    /*  in context so OK  */
	    else
		return TRUE;	
	} else
	  return FALSE;    /*  no context applies  */
} 

/* 
 *	returns no. of possible tests at current level
 *		(currently a hack as doesn't consider context)
 */
int testchcs(testing)
egtesttype  testing;
{
  int i, testchoices;
  testchoices = nattrs;
  for ( i=1; i<=ndattrs; i++)
    if (unord_val_s(testing,i)!=DONTKNOW)
      testchoices--;
  return testchoices;
} 


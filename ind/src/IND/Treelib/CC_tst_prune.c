/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* cost complexity pruner using 1 SE rule with test set
 * (see CART p 79,309)
 *
 *	Author - Wray Buntine
 *
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/************************************************************************/
/*
 *	CC_test_prune(t, pf)    assumes xtra->errs already loaded
 */
CC_test_prune(t, pf, size)
ot_ptr	t;
float	pf;	/*  # std. devs. to add */
int	size;	/*  size of test set  */
{
	double	cost;
	double	sqrt();
	float  alph;
	/*
	 *	get error for pruning
	 */
	cost = CC_errprune(t)/size;
	if ( cost > 1.0 )
		error("sqrt domain in CC_test_prune\n","");
	cost += pf * sqrt(cost*(1.0-cost)/size);
	alph = alpha_min(t, cost*size);
	uncost_tree(t);
	CC_costprune(t, alph);
}

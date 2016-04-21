/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* prune -- tree pruner.
 *
 *	Author - Wray Buntine
 *
 */
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


/************************************************************************/
/*
 *	mincost_prune(t) --  prune node to minimum cost subtree
 *				returning cost achieved
 *			 --  not sensible for decision graphs since class
 *			     counts get screwed up when you remove a parent
 *			     from a join node
 *			 --  cost = errors if no utilities
 */
float
mincost_prune(t)
ot_ptr	t;
{
	short	i,j,best;
	bt_ptr	option;
	float	opt_cost, best_cost;

	if ( !t )
		return 0.0;
	if ( ttest(t) )
	{
		best_cost = FLOATMAX;;
		foroptions(option,i,t) {
		    opt_cost = 0.0;
		    foroutcomes(j,option->test) {
			opt_cost += mincost_prune(option->branches[j]);
		    }
		    if ( opt_cost < best_cost ) {
			best_cost = opt_cost;
			best = i;
		    }
		}
		rem_ooption(t,best);
		opt_cost = cost_val(t);
		if ( opt_cost < best_cost ) {
			force_leaf(t);
			best_cost = opt_cost;
		}
		return best_cost;
	}
	return cost_val(t);
}


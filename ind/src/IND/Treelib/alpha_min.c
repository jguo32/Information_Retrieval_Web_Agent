/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/*
 *	finds max. alpha s.t.  R_\alpha < cost
 *	assumes costs have been previously placed at t->xtra.cc->err
 */

static	float	max_alpha;
static	float	zero_cost;

static 
chk_cost(cost, recost, comp, alph)
float	cost, recost, comp, alph;
{
#ifdef   DEBUG
	fprintf(stdrep,"pruning at alpha = %lg to give cost = %lg, comp = %lg\n", 
			alph, cost, comp);
#endif
	if ( recost < zero_cost ) {
		max_alpha = alph;
	}
}

extern float
alpha_min(t, cost)
ot_ptr	t;
float	cost;
{
	max_alpha = 0.0;
	zero_cost = cost;
#ifdef   DEBUG
	fprintf(stdrep,"zero cost = %lg\n", cost);
#endif
	traverse_trees(t, chk_cost);
#ifdef   DEBUG
	fprintf(stdrep,"\talpha chosen = %lg\n", max_alpha);
#endif
	return max_alpha;
}



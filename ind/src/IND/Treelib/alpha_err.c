/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*
 *	add list of substitution cost estimates for R_\alpha minimising
 *	assumes costs have been previously placed at t->xtra.cc->err
 */

static float	*alphe_list;
static float	*cost_list;
static int	alphe_i;
static int	lesize;

static
adj_cost(cost, recost, comp, alph)
float	cost, recost, comp, alph;
{
	register	int i;
	static float		prev_cost;
	static float		prev_alph;
	if ( alph ) {
	    for (i = alphe_i; i < lesize && alphe_list[i] < alph ; i++)
		if ( alphe_list[i] >= prev_alph )
			cost_list[i] += prev_cost;
	    alphe_i = i;
	}
	prev_cost = recost;
	prev_alph = alph;
/*	fprintf(stdrep, "cost %g, recost %g, comp %g, alph %g\n",
		cost, recost, comp, alph);	*/
}

extern
alpha_cost(t, alist, clist, lsize)
ot_ptr	t;
float	*alist;		/*  alpha list to match   */
float	*clist;		/*  add costs here  */
int	lsize;		/*  size of above arrays  */
{
	alphe_list = alist;
	cost_list = clist;
	alphe_i = 0;
	lesize = lsize;
	traverse_trees(t, adj_cost);
#ifdef  DEBUG
	{
		int	i;
		fprintf(stdrep,"Alpha_list: ");
		for (i=0; i<=lsize; i++)
			fprintf(stdrep," %g",alphe_list[i]);
		fprintf(stdrep,"   Cost_list: ");
		for (i=0; i<=lsize; i++)
			fprintf(stdrep," %g",cost_list[i]);
		fprintf(stdrep,"\n");
	}
#endif
}


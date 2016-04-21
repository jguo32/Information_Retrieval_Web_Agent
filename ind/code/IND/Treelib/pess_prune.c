/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* tree pruner using Quinlan's pessemistic pruning
 * (see IJMMS 1987?)
 *	incompatible with option trees
 * and C4.5 pruning
 * (see Quinlan, 1992)
 *
 *	Author - Wray Buntine
 *
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

static	float prune_factor = 1.0;
static	float coeff = 1.0;
extern  float   Ztoprob();

/************************************************************************/
/*
 *	c45_prune(t)	--  prune the decision tree
 */
c45_prune(t, pf)
ot_ptr	t;
float	pf;
{
	float  c = 0.0,mc = 0.0;    /* dummy variables for dummy root */
        if (decision_graph_flag) 
              error("pessimistic pruning of dgraphs unimplemented","");
	prune_factor = pf;
	coeff = Ztoprob(pf);
	coeff *= coeff;
	_c45_prune(t,&c,&mc);
}

/************************************************************************/
/*
 *	pess_prune(t)	--  prune the decision tree
 */
pess_prune(t, pf)
ot_ptr	t;
float	pf;
{
	float  c = 0.0,mc = 0.0;    /* dummy variables for dummy root */
        if (decision_graph_flag) 
              error("pessimistic pruning of dgraphs unimplemented","");
	prune_factor = pf;
	_pess_prune(t,&c,&mc);
}

/************************************************************************/
/*
 *	pesse_prune(t)	--  prune the decision tree, but dont allow recursive
 *				pruning
 */
pesse_prune(t, pf)
ot_ptr	t;
float	pf;
{
	float  c = 0.0,mc = 0.0;    /* dummy variables for dummy root */
        if (decision_graph_flag) 
              error("pessimistic pruning of dgraphs unimplemented","");
	prune_factor = pf;
	_pesse_prune(t,&c,&mc);
}

/************************************************************************/
/*
 *	_pess_prune(t)	--  prune
 */
_pess_prune(t, lc, mlc)
ot_ptr	t;
float	*lc,*mlc;	/* counts and mis_counts for parent */
{
	int	i;
	float   n_lc=0.0, n_mlc=0.0;
	float std_error();

	if (!t)
		return;
	if ( ttest(t) )
	{
	    if ( toptions(t) ) 
		error("pessimistic pruning of options unimplemented","");
	    foroutcomes(i,t->option.o->test)
		    _pess_prune(t->option.o->branches[i], &n_lc, &n_mlc);
	    if ( cost_val(t) + 0.5 <
		     n_mlc + prune_factor*std_error(n_mlc,n_lc) )
		    force_leaf(t);
	}
	if (  tleaf(t) )
	{
		*mlc += cost_val(t) + 0.5;
		*lc += t->tot_count; 
	} else {
		*lc += n_lc;
		*mlc += n_mlc;
	}
}

/*
 *	Quinlan's AddErrs()
 *		calc. upper confidence limit for cost
 *		(i.e. errors for 0-1 cost matrix)
 *		where prune_factor = confidence factor
 *      NB.   slight difference to Quinlan's because of calc. of
 *	      Ztoprob() 
 */
static float  upper_cost(count,cost)
float	count, cost;
{
float  temp;
double log(), exp(), sqrt();
	if ( cost < EPSILON )
		return cost + count * (1.0-exp(log(prune_factor)/ count ) );
	else if ( cost < 0.9999 ) {
		temp = cost + count * (1.0-exp(log(prune_factor)/ count ));
		return temp + cost*(upper_cost(count,1.0) - temp);
	} else if ( cost + 0.5 >= count ) {
		return 0.67 * count + 0.33 * cost;
	} else {
		temp = (cost + 0.5 + coeff/2.0 +
			sqrt(coeff * ((cost+0.5)*(1.0-(cost+0.5)/count)
				+ coeff/4.0)) ) / (count + coeff);
		return count * temp;
	}
}


/************************************************************************/
/*
 *	_c45_prune(t)	--  prune
 */
_c45_prune(t, lc, mlc)
ot_ptr	t;
float	*lc,*mlc;	/* counts and mis_counts for parent */
{
	int	i;
	float   n_lc=0.0, n_mlc=0.0;

	if (!t)
		return;
	if ( ttest(t) )
	{
	    if ( toptions(t) ) 
		error("C4.5 pruning of options unimplemented","");
	    foroutcomes(i,t->option.o->test)
		    _c45_prune(t->option.o->branches[i], &n_lc, &n_mlc);
	    if ( upper_cost(t->tot_count,cost_val(t)) < n_mlc )
		    force_leaf(t);
	}
	if (  tleaf(t) )
	{
		*mlc += upper_cost(t->tot_count,cost_val(t));
		*lc += t->tot_count; 
	} else {
		*lc += n_lc;
		*mlc += n_mlc;
	}
}

/************************************************************************/
/*
 *	_pess_prune(t)	--  prune
 */
_pesse_prune(t, lc, mlc)
ot_ptr	t;
float	*lc,*mlc;	/* counts and mis_counts for parent */
{
	int	i;
	float   n_lc=0.0, n_mlc=0.0;
	float std_error();

	if (!t)
		return;
	if ( ttest(t) ) {
	    if ( toptions(t) ) 
		error("pessimistic pruning of options unimplemented","");
	    foroutcomes(i,t->option.o->test)
		    _pesse_prune(t->option.o->branches[i], &n_lc, &n_mlc);
	    if ( cost_val(t) + 0.5 <
		     n_mlc + prune_factor*std_error(n_mlc,n_lc) ) {
		    force_leaf(t);
	    }
	    *lc += n_lc;
	    *mlc += n_mlc;
	} else {
	    *lc += t->tot_count; 
	    *mlc += cost_val(t) + 0.5;
	}
}


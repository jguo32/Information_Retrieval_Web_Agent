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
 *       additive components for tree
 */
#define node_cost(t)     cc_cost_val(t)/CC_ntegs 
#define node_recost(t)   t->xtra.cc->err
#define node_leaf(t)     1.0
#define node_leafcost(t)     cost_val(t)/CC_ntegs + CC_alpha

/*
 *	cost complexity prune parameters
 */
static	float	CC_alpha;
static	float	CC_ntegs;

/*
 *	functions to compute additive stuff on tree, i.e. for CART utilities
 *		DONT WORK FOR OPTION TREES and rather inefficient
 */

/*
 *	compute additive function on tree
 *	NB.  prob. should just be float
 */

static	float	(*leaf_add)(), (*node_add)();

float
tree_add(t, leaf_fn, node_fn)
ot_ptr	t;
float	(*leaf_fn)(), (*node_fn)();
{
	float	_tree_add();
	leaf_add = leaf_fn;
	node_add = node_fn;
	return _tree_add(t);
}

extern float
_tree_add(t)
ot_ptr	t;
{
	int	i;
	float	sub;
	if ( !t )
		return 0.0;
	if ( ttest(t) ) {
		sub = (*node_add)(t);
		foroutcomes(i,t->option.o->test)
			sub += _tree_add(t->option.o->branches[i]);
		return sub;
	} else
		return (*leaf_add)(t);
}

/*
 *	prune tree to minimise additive function
 */

static	min_prune;	/*  set to 0 for no pruning  */

static float
_tree_min(t)
register ot_ptr	t;
{
	register int	i;
	register double	sub, lsub;
	if ( !t )
		return 0.0;
	if ( ttest(t) ) {
		sub = 0.0;
		foroutcomes(i,t->option.o->test)
			sub += _tree_min(t->option.o->branches[i]);
		if ( (lsub=(*leaf_add)(t)) <= sub ) {
			if (min_prune)
				force_leaf(t);
			return lsub;
		} else {
			return sub;
		}
	} else
		return (*leaf_add)(t);
}


static float
recost_val(t)
ot_ptr  t;
{
        return node_recost(t);
}

/*
 *	   calc. minim. errors tree on recosts 
 */
float CC_errprune(t)
ot_ptr	t;
{
	leaf_add =  recost_val;
	min_prune = 1;
	return _tree_min(t);
}


static float CC_leaf(t)
ot_ptr t;
{
	return  node_leafcost(t);
}

/*
 *	cost compl. pruning using trees resubstitution cost
 */
CC_costprune(t, alph)
ot_ptr	t;
float	alph;
{
	CC_alpha = alph;
	CC_ntegs = t->tot_count;
	leaf_add = CC_leaf;
	min_prune = 1;
	_tree_min(t);
}

/*
 *		traverse_trees(t, updt_fn)
 *	traverse all pruned subtrees in additive function order
 *	calling updt_fn() as each new "alpha" value found
 */

static	float	prev_alph;
#define EPSIL	1.0e-8

/*
 *        set this to "print_flags" (see TREE.h) for debugging info
 */
#ifdef DEBUG
tprint_flags	ccprint_flags;
#endif

traverse_trees(t, updt_fn)
ot_ptr	t;
int	(*updt_fn)();
{
	static int install_CC();
	register ot_ptr	this;
	register bt_ptr	option;
	int	i;
	CC_ntegs = t->tot_count;
	if ( tleaf(t) ) {
	    	updt_fn(node_cost(t), node_recost(t), node_leaf(t), 0.0);
		return;
	}
#ifdef DEBUG
	if ( int_flags(ccprint_flags)  ) {
		fprintf(stdrep,"Entry\n");
		print_tree(t,ccprint_flags,10000);
	}
#endif
	prev_alph = 0.0;
	install_CC(t);
	/*
 	 *	generate each increasing value of alpha and progressive totals
	 */
	while ( 1 ) {
	    /*
	     *	process this alpha
	     */
	    if ( prev_alph + EPSIL < t->xtra.cc->G ) {
	    	updt_fn(t->xtra.cc->S, t->xtra.cc->C, t->xtra.cc->N, prev_alph);
#ifdef DEBUG
		if ( int_flags(ccprint_flags) ) {
			fprintf(stdrep,"cost = %lg, recost = %g, comp = %lg, alpha = %lg\n",
				t->xtra.cc->S,  t->xtra.cc->C,
				t->xtra.cc->N, prev_alph);
			print_tree(t,ccprint_flags,10000);
		}
#endif
	        prev_alph = t->xtra.cc->G;
	    }
	    /*
	     *	travel down to next node for pruning
	     */
	    for ( this = t; this->xtra.cc->G < this->xtra.cc->g - EPSIL ; 
		  this = this->option.o->branches[(int)this->xtra.cc->best] ) ;
	    /*
	     *	travel up, to calc. next best node
	     */
	    this->xtra.cc->N = node_leaf(this);
	    this->xtra.cc->S = node_cost(this);
	    this->xtra.cc->C = node_recost(this);
	    this->xtra.cc->G = FLOATMAX;
	    set_flag(this->tflags,visited);
	    if ( this == t )
		break;
	    while ( first_parent(this) ) {
		this = oparent(this);
	    	this->xtra.cc->N = 0.0;
	    	this->xtra.cc->S = 0.0;
	    	this->xtra.cc->C = 0.0;
	    	this->xtra.cc->G = FLOATMAX;
	 	option = this->option.o;
		foroutcomes(i,option->test) {
	    	    this->xtra.cc->N += option->branches[i]->xtra.cc->N;
	    	    this->xtra.cc->S += option->branches[i]->xtra.cc->S;
	    	    this->xtra.cc->C += option->branches[i]->xtra.cc->C;
	    	    if ( this->xtra.cc->G > option->branches[i]->xtra.cc->G ) {
			this->xtra.cc->G = option->branches[i]->xtra.cc->G;
			this->xtra.cc->best = i;
		    }
		}
		this->xtra.cc->g = (node_cost(this) - this->xtra.cc->S) /
			(this->xtra.cc->N - 1.0);
		this->xtra.cc->G = Min(this->xtra.cc->g, this->xtra.cc->G);
	    }
	}
	updt_fn(t->xtra.cc->S,  t->xtra.cc->C, t->xtra.cc->N, prev_alph);
#ifdef DEBUG
	if ( int_flags(ccprint_flags) ) {
		fprintf(stdrep,"cost = %g, recost = %g, comp = %g, alpha = %lg\n",
			t->xtra.cc->S, t->xtra.cc->C, t->xtra.cc->N, prev_alph);
		print_tree(t,ccprint_flags,10000);
	}
#endif
	updt_fn(t->xtra.cc->S, t->xtra.cc->C, t->xtra.cc->N, FLOATMAX);
}

/*
 *	first for loop in ALG. 10.1 of CART p294
 */
static int
install_CC(this)
register	ot_ptr	this;
{
	int	depth, dep2;
	int	i;
	bt_ptr	option;

	if ( !ttest(this) ) {
	        set_flag(this->tflags,visited);
		this->xtra.cc->N = node_leaf(this);
		this->xtra.cc->C = node_recost(this);
		this->xtra.cc->S = node_cost(this);
		this->xtra.cc->g = 0.0;
		this->xtra.cc->G = FLOATMAX;
		this->xtra.cc->best = DONTKNOW;
		return 0;	
	}
	this->xtra.cc->N = 0.0;
	this->xtra.cc->S = 0.0;
	this->xtra.cc->C = 0.0;
	this->xtra.cc->G = FLOATMAX;
	this->xtra.cc->best = DONTKNOW;
	depth = 0;
	option = this->option.o;
	foroutcomes(i,option->test) {
	    dep2 = install_CC(option->branches[i]);
	    depth = Max( dep2, depth);
	    this->xtra.cc->N += option->branches[i]->xtra.cc->N;
	    this->xtra.cc->S += option->branches[i]->xtra.cc->S;
	    this->xtra.cc->C += option->branches[i]->xtra.cc->C;
	    if ( this->xtra.cc->G > option->branches[i]->xtra.cc->G ) {
		this->xtra.cc->G = option->branches[i]->xtra.cc->G;
		this->xtra.cc->best = i;
	    }
	}
	this->xtra.cc->g = (node_cost(this) - this->xtra.cc->S) /
			(this->xtra.cc->N - node_leaf(this));
	this->xtra.cc->G = Min(this->xtra.cc->g, this->xtra.cc->G);
	if ( !depth && this->xtra.cc->G <= 0.0 )  {
		/*
		 *	don't add neg. alpha nodes, prune immediately
		 */
	        set_flag(this->tflags,visited);
		this->xtra.cc->N = node_leaf(this);
		this->xtra.cc->S = node_cost(this);
		this->xtra.cc->C = node_recost(this);
		this->xtra.cc->g = 0.0;
		this->xtra.cc->G = FLOATMAX;
		this->xtra.cc->best = DONTKNOW;
	} else
	    depth++;
	return depth;
}




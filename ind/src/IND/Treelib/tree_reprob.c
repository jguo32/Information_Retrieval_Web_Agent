/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*
 *            These routines take an option tree with class counts
 *            at leaves and all interior nodes,
 *            and perform various statistical tricks to 
 *                1)  turn class counts to class probabilites
 *                       ot_rec.eg_count is modified in-situ
 *                2)  establish values for
 *                       ot_rec.lprob
 *                       t_rec.lprob,  t_rec.np.nprob
 *                    usually by "averaging"
 *            the spare space of "t_rec.xtra.pn" is used here
 */

extern	int	depth;
/*
 *            various  modifiers to the basic routines
 *            (values assigned are defaults from tprune.c)
 */
static	double	tot_weight;		/*  stores weight of entire tree */
static  double	prob_cutoff=0.01;
static	int	max_opt= 10;            /*   prune if more options that this */
static	bool	nopruneflg=FALSE;       /*   don't average over sub-trees,
					     just over different options  */
static	bool	probflg;                /*   set counts to probabilities  */
static  bool    modtree;                /*   allows tree to be pruned */
static  egtesttype  testing;

/******************************************************************************/
/*
 *	tree_reprob(t) -- adjust eg_counts to be class probabilities;
 *                        no averaging, no pruning, just do Laplacian
 *                        estimates at the leaves
 */
tree_reprob(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	if ( !t )
		return;
	set_leaf_prob(t->eg_count);
	restrat_fprob(t->eg_count);
	if ( tleaf(t) ) {
	    t->lprob = 1.0;
        } else {
	    t->lprob = 0.0;
	    foroptions(option,j,t) {
		foroutcomes(i,option->test)
			tree_reprob(option->branches[i]);
		option->np.nprop = 1.0/options(t);
	    }
	}
}


/******************************************************************************/
/*
 *	tree_avet(t) -- adjust eg_counts to be class probabilities;
 *			do full averaging of option tree, and possibly
 *                      prune low probability options and subtrees;
 *                      this implements sct. 6.5.4 of the thesis
 *                      
 */
double
tree_avet(t, prcut, maxo, noprun, ttesting)
ot_ptr	t;
float	prcut;	  /*   remove any option has branch proportion < this */
		  /*   if >=1, remove all but best option */
int	maxo;	  /*   any near-leaf node >= this many options is made leaf  */
bool	noprun;	  /*   true if don't wish to average pruned trees as well
				i.e.    for logical version    */
egtesttype  ttesting;    /*  use this record to keep "testing", its set on entry*/
{
	static double	_tree_subw();
	static void	_tree_ave();

	ASSERT ( ttesting.unordp )
	testing = ttesting;
	prob_cutoff = prcut;
	max_opt = maxo;
	nopruneflg = noprun;
	modtree = probflg = TRUE;
	tot_weight = _tree_subw(t);
	if ( !nopruneflg )
	    _tree_ave(t, tot_weight);
        return tot_weight;
}

/******************************************************************************/
/*
 *	Dtree_avet(t,) -- calc. 1st & 2nd derivative of posterior w.r.t alpha
 *                       results left in "xtra.pn"
 *			 used to maximise prior parameters
 */
void
Dtree_ave(t,t_d2sprob,t_dsprob)
ot_ptr	t;
double  *t_d2sprob, *t_dsprob;
{
	static double	_tree_subw();
	static void    _Dtree_ave();

	nopruneflg = FALSE;
	modtree = probflg = FALSE;
	tot_weight = _tree_subw(t);
	*t_d2sprob = *t_dsprob = 0.0;
	_Dtree_ave(t,t_d2sprob,t_dsprob);
}

static  float   subw_cut, leafw_cut;

/******************************************************************************/
/*
 *	tree_maxt(t) -- find max. posterior subtree in entire option tree,
 *                      then adjust counts, t->lprob, etc. as in
 *                      tree_reprob(),
 *                      (similar to "tree_avet()" with pr_cut > 1.0 )
 *	
 */
double
tree_maxt(t, lwcut, swcut, ttesting)
ot_ptr	t;
float	swcut;
float   lwcut;
egtesttype  ttesting;    /*  use this record to keep "testing", its set on entry*/
{
	static double	_tree_maxw();

	ASSERT ( ttesting.unordp )
	testing = ttesting;
	subw_cut = log((double)swcut);
	leafw_cut = log((double)lwcut);
	tot_weight = _tree_maxw(t);
        return tot_weight;
}

/**************************************************************************/
/*
 *          various static routines to help with above
 */


/*
 *	_tree_subw(t) -- put into subw form.
 *		i.e.
 *		puts leaf weight into "t->xtra.pn.lprob"
 *		puts weight of all pruned subtrees in "option->gain"
 *		puts sum of these in "t->xtra.pn.sprob"
 *               ALSO - "probflg" - set if want counts changed to probs
 *                    "nopruneflg" - set if dont want subtrees weighted as well
 *
 *           NB.  final pass is needed (see _tree_avet) to calculate t->lprob
 */
static double	
_tree_subw(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	bt_ptr	best;
	int	remvd;
	int	sprobnew;

	if ( !t )
		error("empty branch","");
	/*
	 *	set t->xtra.pn.lprob to be prob. of this node as leaf
	 *	and initialise node class probabilities
	 */
	t->lprob = 0.0;
	t->xtra.pn.lprob = leaf_prob(t,testing);
	if ( probflg ) calc_leaf_prob(t->eg_count);
	if ( tleaf(t) ) {
	        t->lprob = 1.0;
		return t->xtra.pn.sprob = t->xtra.pn.lprob;
	} else {
	  t->xtra.pn.sprob = -FLOATMAX;
	  foroptions(option,j,t) {
		/*
		 *  set options->gain to be sum of probs of all trees
		 *  with options as root (but not leaf options)
		 */
		option->gain = (option->np.nodew =	
				     node_weight(testing, option))
			       + option->nprob; 
		depth++;
		foroutcomes(i,option->test) {
		    add_test(option->test,i,testing);
		    option->gain += _tree_subw(option->branches[i]);
		    rem_test(option->test,i,testing);
		}
		depth--;
		/*
		 *	add these to node sprob
		 */
		if ( t->xtra.pn.sprob == -FLOATMAX )
		    t->xtra.pn.sprob = option->gain;
		else
		    t->xtra.pn.sprob =
			logsum(t->xtra.pn.sprob,option->gain);
	  }
	  /* 
	   *     at this point, but we're finished using np.nodew
	   */
	  foroptions(option,j,t)
	   	option->np.nprop = exp(option->gain-t->xtra.pn.sprob);
	  /*
	   *   averaging done, now do post pruning stuff
	   */
	  if ( modtree ) {
	    remvd = 0;
	    /*
	     *		remove any options with low probability
	     */
	    if ( toptions(t) ) {
	       	/*
		 *	only works because j counts downwards
		 */
		remvd = 0;
		if ( prob_cutoff < 1.0 ) {
		  foroptions(option,j,t) {
		    /*
		     *	 remove any options less than cutoff
		     */
		    if ( option->np.nprop < prob_cutoff ) {
			rem_option(t,j);
			remvd++;
			if ( !toptions(t) ) {
				if ( t->option.o->np.nprop < prob_cutoff ) {
					rem_option(t,0);
					remvd++;
				}
				break;
			}
		    }
		  }
		  if ( remvd ) {
		    /*
		     *	 adjusts probs for removed items
		     */
		    if ( !ttest(t) ) {
			return t->xtra.pn.sprob = t->xtra.pn.lprob;
		    }
		    sprobnew = 1;
	   	    foroptions(option,j,t) {
		   	if ( sprobnew ) {
		    	    t->xtra.pn.sprob = option->gain;
			    sprobnew = 0;
			} else
		    	    t->xtra.pn.sprob =
				logsum(t->xtra.pn.sprob,option->gain);
		    }
	    	    foroptions(option,j,t)
			option->np.nprop = 
			  exp(option->gain-t->xtra.pn.sprob);
		    }
		} else {
		  /*
		   *   find best option, remove all others
		   */
		  best = (bt_ptr) 0;
		  foroptions(option,j,t) {
			if ( !best || best->np.nprop < option->np.nprop )
			    best = option;
		  }
		  best->np.nprop = 1.0;
		  t->xtra.pn.sprob = best->gain;
	   	  foroptions(option,j,t) {
		        if ( option != best )
			  rem_option(t,j);
			if ( !toptions(t) )
			  break;
		  }
		}
	    }    
	    /*
	     *	 force leaf if all children are leaves and too many options
	     */
	    remvd = 0;
	    foroptions(option,j,t) {
		foroutcomes(i,option->test)
		    if ( !tleaf(option->branches[i]) ) {
			remvd = 1;
			break;
		    }
		if ( remvd )	break;
	    }
	    if ( remvd && options(t) > max_opt ) {
		force_leaf(t);
	        t->lprob = 1.0;
		return t->xtra.pn.sprob = t->xtra.pn.lprob;
	    }
	  }
	  if ( nopruneflg )
	        return t->xtra.pn.sprob;
	  else
	        return t->xtra.pn.sprob =
			logsum(t->xtra.pn.sprob,t->xtra.pn.lprob);
	}
}


/*
 *	_tree_maxw(t) -- find max. posterior subtree
 */
static double	
_tree_maxw(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	bt_ptr	best;

	if ( !t )
		error("empty branch","");
	/*
	 *	set t->xtra.pn.lprob to be prob. of this node as leaf
	 *	and initialise node class probabilities
	 */
	t->xtra.pn.lprob = leaf_prob(t,testing);
	calc_leaf_prob(t->eg_count);
	if ( tleaf(t) ) {
	        t->lprob = 1.0;
		return t->xtra.pn.sprob = t->xtra.pn.lprob;
	} else {
	    t->lprob = 0.0;
	    t->xtra.pn.sprob = -FLOATMAX;
	    best = (bt_ptr)0;
	    foroptions(option,j,t) {
		/*
		 *  set options->gain to be sum of probs of all trees
		 *  with options as root (but not leaf options)
		 */
		option->gain = (option->np.nodew = node_weight(testing, option))
				+ option->nprob; 
		depth++;
		foroutcomes(i,option->test) {
		    add_test(option->test,i,testing);
		    option->gain += _tree_maxw(option->branches[i]);
		    rem_test(option->test,i,testing);
		}
		depth--;
		/*
		 *	add these to node sprob
		 */
		if ( t->xtra.pn.sprob == -FLOATMAX )
		    t->xtra.pn.sprob = option->gain;
		else
		    t->xtra.pn.sprob =
			logsum(t->xtra.pn.sprob,option->gain);
		if ( !best || best->gain < option->gain )
		  best = option;
	    }
	    if ( best->gain - t->xtra.pn.sprob < subw_cut
		 ||  t->xtra.pn.lprob - best->gain > leafw_cut ) {
	      /*
	       *   force to be leaf
	       */
	      force_leaf(t);
	      t->lprob = 1.0;
	      return t->xtra.pn.sprob = t->xtra.pn.lprob;
	    } else {
	      /*
	       *   save best option
	       */
	      best->np.nprop = 1.0;
	      t->xtra.pn.sprob = best->gain;
	      foroptions(option,j,t) {
		if ( option != best )
		  rem_option(t,j);
		if ( !toptions(t) )
		  break;
	      }
	      return t->xtra.pn.sprob;
	    }
        }
}

/*
 *	_Dtree_ave(t) -- calc. its 1st & 2nd derivative w.r.t. palpha
 *
 */
static void
_Dtree_ave(t, addto_d2sprob, addto_dsprob)
ot_ptr	t;
double *addto_d2sprob, *addto_dsprob;
{
	int	i,j;
	bt_ptr	option;
	double	temp_prop;
	double	DLw;
	double  option_d2sprob, option_dsprob;
	double  t_dsprob;

	if ( !t )
		error("empty branch","");
	DLw = Dleaf_prob(t->eg_count);
	temp_prop = exp( t->xtra.pn.lprob - t->xtra.pn.sprob );
	t_dsprob = temp_prop * DLw;
	*addto_d2sprob += temp_prop * ( DLw * DLw + D2leaf_prob() );
	if ( !tleaf(t) ) {
	    foroptions(option,j,t) {
	        /*
		 *      calc. derivatives for subtrees
		 */
		depth++;
		option_d2sprob = 0.0;
		option_dsprob = 0.0;		
		foroutcomes(i,option->test) {
		    _Dtree_ave(option->branches[i], &option_d2sprob
						  , &option_dsprob);
		}
		depth--;
		/*
		 *	add these to intermediate calculations
		 */
		temp_prop = exp( option->gain - t->xtra.pn.sprob );
		t_dsprob += temp_prop * option_dsprob;
		*addto_d2sprob += temp_prop * ( option_d2sprob
		  		  + option_dsprob * option_dsprob );
		
	    }
	}
	*addto_d2sprob -= t_dsprob * t_dsprob;
	*addto_dsprob += t_dsprob;
}


/*
 *	_tree_ave(t, top_weight) -- take values in t->xtra.pn, and of parents,
 *                                  to calculate "t->lprob"
 */
static void
_tree_ave(t, top_weight)
ot_ptr	t;
double	top_weight;		/*  sum of "pn.sprob" for parents  */
{
	int	i,j;
	bt_ptr	option;
	
	if ( !t )
		error("empty branch","");
	top_weight -= t->xtra.pn.sprob;
	t->lprob = exp(top_weight+t->xtra.pn.lprob - tot_weight);
	if ( !tleaf(t) ) {
	    foroptions(option,j,t) {
	        foroutcomes(i, option->test)
		    _tree_ave(option->branches[i],
				top_weight+option->gain);
	    }
	}
}




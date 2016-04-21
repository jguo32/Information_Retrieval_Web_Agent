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

static  egtesttype  testing;
extern  depth;

/*
 *	tree_submaxw(t,dom) -- calc. subw or maxw for tree,  (extracted from _tree_subw)
 *                      but don't do anything else;
 *			assume "testing" is correctly set on entry;
 *			dom = 1 for maxw, 0 for subw						
 *	btree_submaxw(t,testing,dom) -- ditto for btree
 */
static unsigned char  do_maxw;
#define  combine(a,b)  (do_maxw? Max(a,b) : logsum((double)a,(double)b))

static double	
_tree_submaxw(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	double  subw, ssubw;

	if ( !t )
		error("empty branch","");
	if ( tleaf(t) || !tparent(t) )  {
		return leaf_prob(t,testing);
	}
	subw = leaf_prob(t,testing);
	foroptions(option,j,t) {
	  ssubw = (option->np.nodew = node_weight(testing, option))
			 + option->nprob; 
	  depth++;
	  foroutcomes(i,option->test) {
	    add_test(option->test,i,testing);
	    ssubw += _tree_submaxw(option->branches[i]);
	    rem_test(option->test,i,testing);
	  }
	  depth--;
	  subw = combine(subw,ssubw);
	}
	return subw;
}

static double	
_btree_submaxw(bt)
bt_ptr	bt;
{
	int	i;
	double  subw;
	if ( !bt )
		error("empty branch","");
	subw = (bt->np.nodew = node_weight(testing, bt))
			 + bt->nprob; 
	depth++;
	foroutcomes(i,bt->test) {
	    add_test(bt->test,i,testing);
	    subw += _tree_submaxw(bt->branches[i]);
	    rem_test(bt->test,i,testing);
	}
	depth--;
	return subw;
}

/*
 *	both routines calc. the log. probability of the subtree,
 *	the "testing" record is used throughout, and any "testing"
 *	records in the tree are preserved
 */
extern double	
tree_submaxw(t,ttesting,dom)
egtesttype  ttesting;
ot_ptr	t;
int dom;
{
	do_maxw = dom;
	testing = ttesting;
	return _tree_submaxw(t);
}

extern double	
btree_submaxw(bt,ttesting,dom)
egtesttype  ttesting;
bt_ptr	bt;
int dom;
{
	do_maxw = dom;
	testing = ttesting;
	return _btree_submaxw(bt);
}


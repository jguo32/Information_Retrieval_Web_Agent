/*	Copyright (C)  Wray Buntine 1990
/*
 *	RECURSIVE PARTITIONING SKELETON CODE
 */
/*
 *	Basic tree data structure
 *	(normally, all sorts of other statistics such as class counts
 *	 are stored as well)
 */

typedef struct
{
	int	test;		/*   attribute tested on or 0 for `none' */
	treeptr branch[];	/*   array of subtrees, for each outcome */
	float	probs[];	/*   array of class probabilities        */
}	tree;
typedef struct tree *treeptr;

/*
 *	Outer shell for the recursive partitioning algorithm
 */

treeptr  grow(egs, attr_set)
eg_type[]	egs;
attr_type[]	attr_set;
{
    treeptr	this;

    this = new_tree();
    if ( stopping(egs) )
    {
	/*
	 *	stop growing and construct class probabilities for leaf
	 */
        this.probs = prob_est(egs);
        this.test  = 0;
    } else {
	/*
	 *	choose best test using ``splitting rule''
	 */
    	this.test = splitting(egs, attr_set);
    	foreach outcome in outcomes(this.test)
    	{
	    /*
	     *	partition examples accordingly to test
  	     *	and grow subtrees recursively
	     */
            this.branch[outcome] = grow(
		subset(egs,this.test,outcome),
		set_remove(this.test,attr_set) );
	}
    }
}

eg_type[]  subset(egs,test,outcome),
eg_type[]  egs;
int	   test;
int	   outcome;
{
    /*
     *	return subset of ``egs'' having value ``outcome'' for ``test''
     */	
}

/*
 *	Outer shell for a standard recursive tree pruner attempting
 *	to minimise some additive function over the tree.
 *	leaf_prune_value()
 *	node_prune_value() :   used to compute the additive function
 */

int  prune(this)
treeptr	this;
{
    float    sub_tree_total;	/*   total of additive function for ``this'' */
    if ( this.test == 0 ) 
    {
	/*
	 *	tree is a leaf, so don't prune
	 */
    	return leaf_prune_value(this);
    } else {
	/*
	 *	tree is not a leaf,
	 *	so prune subtrees and compute additive function for
	 *	resultant subtree at same time
	 */
	sub_tree_total = node_prune_value(this);
    	foreach outcome in outcomes(this.test)
    	{
            sub_tree_total = sub_tree_total + prune(this.branch[outcome]);
	}
	if ( sub_tree_total < leaf_prune_value )
	{
	   /*	
	    *    pruning condition succeeds, so turn ``this'' to leaf
	    */
	    return leaf_prune_value(force_leaf(this));
	} else {
	   /*
	    *	 don't prune, just return value for subtree
	    */
	    return sub_tree_total;
	}
    }
}

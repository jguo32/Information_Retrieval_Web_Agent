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


static	ot_ptr	t_root;
static	(*updt)();
static	bool 	updt_leaf;	/*  1 if call update for internal nodes too */
bool 	tree_reclass_query=0;   /*  1 if query to find details of eg.  */
static void _tree_reclass();

/******************************************************************************/
/*
 *	tree_reclass(t, eg) -- find nodes for eg,
 *			       and call node_update();
 */
tree_reclass(t, eg, node_update, tr_updt_leaf)
ot_ptr	t;
egtype	eg;
int	(*node_update)();
int	tr_updt_leaf;	/*  1 if call update for internal nodes too */
{
	t_root = t;
	updt_leaf = tr_updt_leaf;
	updt = node_update;
	_tree_reclass(t, eg, (float)1.0);
}

/******************************************************************************/
/*
 *	_tree_reclass(t, eg, prop) -- find leaf nodes of eg,
 *			beginning at position t in the decision tree;
 *			prop is the prop. of the example to store
 *			update counts accordingly
 *	NB.   "send down all" doesn't make sense when classifying as it
 *		corresponds to taking 1/nvals average
 */
static void
_tree_reclass(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	register	int	i,j,v;
	int	nv;		/* number of possible values */
	register bt_ptr	option;
	float	*sub_prop;
	static	int	mf;
	static	float	mf_count;

	if ( tleaf(t) ) {
	    (*updt)(t,eg,prop);
	} else {
	    if ( updt_leaf )
		(*updt)(t,eg,prop);
	    foroptions(option,j,t) {
	        nv = outcomes(option->test);
	        if (tree_reclass_query )
			v = query_outcome(eg,option->test);
		else
			v = outcome(eg,option->test);
	        if ( v<0 ) {
        	    switch (Uflg)
       		    {
        	    case 5:         /*  send down even   */
			prop *= 1.0/nv;
        	    case 2:         /*  send down ALL   */
			for (i = 0; i < nv; i++)
			    _tree_reclass(option->branches[i], eg, prop);
			break;
		    case 1:	     /*   send down proportionally   */
			sub_prop = copy_fvec(subtree_prop(option),nv);
			for (i = 0; i < nv; i++)
			    _tree_reclass(option->branches[i], eg,
					prop*sub_prop[i]);
			break;
        	    case 3:        /*  send down most common  */
			mf = 0;
			mf_count = option->branches[0]->tot_count;
			for (i = 1; i < nv; i++)
			    if ( option->branches[i]->tot_count > mf_count ) {
				mf = i;
				mf_count = option->branches[i]->tot_count;
			    }
			_tree_reclass(option->branches[mf], eg, prop);
			break;
		    case 4:	     /*   send down one with prob.   */
				     /*   NB.  later assign. may clash!! */
			assign_subtree_prop(option);
			i = random_outcome(option->test);
			_tree_reclass(option->branches[i], eg, prop);
			break;
        	    default:         /*  ignore unknowns  */
			break;
		    }
	        } else
		   _tree_reclass(option->branches[v], eg, prop);
	    }
    	}
}

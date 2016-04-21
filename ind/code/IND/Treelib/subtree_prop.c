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


#define SUBTREE_ALPHA 0.5

/***************************************************************************/
/*
 *	subtree_prop(t) -- return tempor. vector of prop.s of subtrees of t
 */
float	*
subtree_prop(t)
bt_ptr	t;
{
	extern	double *strat_true;
	static	float	*prop=0;
	register	int i,j,nv = outcomes(t->test);
	float	sum=0.0;
	if ( !prop )
		prop = (float *) salloc(nvalsattr * sizeof(float));
	if ( !t->test )
		error("Node does not have subtree\n","");
	if (strat_true) {
		/*
		 *	reverse stratification
		 */
		float	*ts;
		for (j=0; j<nv; j++) {
		    prop[j] = 0.0;
		    if (t->branches[j]) {
		        ts = t->branches[j]->eg_count;
		        prop[j] = 0.0;
		        fordecs(i)
		            prop[j] += ts[i] / strat_true[i];
		        sum += prop[j];
		    } 
		}
	} else
		for (i=0; i<nv; i++)
		    if (t->branches[i])
		        sum += prop[i] =
				t->branches[i]->tot_count+SUBTREE_ALPHA;
		    else
			sum += prop[i] = SUBTREE_ALPHA;
	for (j=0; j<nv; j++) 
		prop[j] /= sum;
	return &prop[0];
}

/***********************************************************************
 * 
 *	Correct subtree proportions should be assigned
 *	when the tree is built, except for multi-outcome tests,
 *	which have proportions recalc. and stored in temp. array as
 *	needed.   This routine makes corrections as needed.
 *	It cannot work for complex tests, this would mean assigning
 *	things when test is constructed in choose.c.
 *	i.e.   unimplemented for complex tests
 */
assign_subtree_prop(t)
bt_ptr	t;
{
	if ( outcomes(t->test) > 2 && t->test->tprop.prop_vec ||
	      outcomes(t->test) == 2 && t->test->tprop.prop_true  )
		return;
	if ( outcomes(t->test) > 2)
		t->test->tprop.prop_vec = subtree_prop(t);
	else
		t->test->tprop.prop_true = (subtree_prop(t))[0];
}

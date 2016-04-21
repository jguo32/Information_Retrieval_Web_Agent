/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   Altered to free decision graphs Jon Oliver August 1992
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

bool   free_ccxtra=FALSE;		/*   set to TRUE to free cc */
bool   free_egxtra=FALSE;		/*   set to true to free .gr.egs */


/***************************************************************************/
/*
 *	free_btree(t) -- free memory for b tree,  leave join nodes
 *			alone if other parents are not being freed,
 *			but adjust parent list
 */
free_btree(t)
register bt_ptr	t;
{
	register int	i,j;
	ot_ptr   ot;

	if (t == NULL)
		return;
        foroutcomes(j,t->test) {
	    ot=t->branches[j];
	    if ( ot->num_parents>1 ) {
		/*
		 *    if ot has multiple parents, don't free ot, but
		 *    remove t from the list of parents
		 */
		ot->num_parents--;
		for ( i=ot->num_parents; ot->parents[i]!=t && i>=0; i--) ;
		ASSERT ( i>=0 );
	        ot->parents[i] = ot->parents[ot->num_parents];
		ot->parents[ot->num_parents] = 0;
	    } else 
	    	free_tree(ot);
	  }
	sfree(t->branches);
	free_test(t->test);
	sfree(t);
}


/***************************************************************************/
/*
 *	free_tree(t) -- free memory for tree, using free_btree, etc.
 */
free_tree(t)
ot_ptr	t;
{
	register int	i;
	bt_ptr		option;

	if (t == NULL)
		return;
	if ( ttest(t) ) {
	    foroptions(option,i,t) 
		free_btree(option);
	    if ( toptions(t) )
		sfree(t->option.s.o);
	}
       	if (t->testing.unordp) {
		sfree(t->testing.unordp);
	}
	if (t->parents)
		sfree(t->parents);
	if (t->eg_count)
		sfree(t->eg_count);
	if ( free_egxtra )
		free_set(t->xtra.gr.egs);
	if ( free_ccxtra )
		sfree(t->xtra.cc);
	sfree(t);
}

/***************************************************************************/
/*
 *      force_leaf(t) -- free tree to be leaf 
 */
force_leaf(t)
register ot_ptr   t;
{
	register int	i;
	bt_ptr		option;

	if (t == NULL)
		return;
	if ( ttest(t) ) {
	    foroptions(option,i,t) 
	        free_btree(option);
	    if ( toptions(t) )
		sfree(t->option.s.o);
	}
	t->option.o = 0;
	t->option.s.c = 0;
	unset_flag(t->tflags,test);
	unset_flag(t->tflags,optiont);
	set_flag(t->tflags,leaf);
}

static void do_force_leaves(t)
ot_ptr   t;
{
	if ( !ttest(t) ) 
		force_leaf(t);
}

/***************************************************************************/
/*
 *      force_leaves(t) -- go through tree recursively to make 
 *                         ungrown nodes leaves 
 */
force_leaves(t)
register ot_ptr   t;
{
	    traverse_onodes(t,do_force_leaves);
}

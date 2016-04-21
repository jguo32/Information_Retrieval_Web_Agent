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
 *	install "cc" record at "->xtra" in each node, 
 *	initialise best class, and do decision pruning, etc.
 *	returns best decision if its constant for all subtrees,
 *	(so needs pruning), or -1 if no best decision
 */
int
recost_tree(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	extern  bool free_ccxtra;
	/*
	 *    maintain best decision, while its constant across
	 *    substrees, uninitialised = -2, differs in subtrees = -1
	 */
	int	dec = -2;   
	if ( t ) {
	    t->xtra.cc = mem_alloc(CC_rec);
	    t->xtra.cc->err = 0.0;
	    t->xtra.cc->class = best_dec(t->eg_count);
	    if ( ttest(t) ) {
		foroptions(option,j,t) foroutcomes(i,option->test) {
			if ( dec == -2 )
			   dec = recost_tree(option->branches[i]);
			else if ( dec != recost_tree(option->branches[i]) )
			    dec = -1;
		}
		if ( dec >= 0 ) {
		    /*
		     *     all subtrees have constant best decision
		     *     so prune
		     */
		    free_ccxtra = TRUE;
		    force_leaf(t);
		    free_ccxtra = FALSE;
	    	    return t->xtra.cc->class ;
		}
		    else return -1;
	    } else
	    	return t->xtra.cc->class ;
	}
	   else return -1;
}

/*
 *	free the "xtra.cc" record
 */
uncost_tree(t)
ot_ptr	t;
{
	int	i,j;
	bt_ptr	option;
	if ( t ) {
	    sfree(t->xtra.cc);
	    if ( ttest(t) ) {
		foroptions(option,j,t) foroutcomes(i,option->test) 
			uncost_tree(option->branches[i]);
	    }
	}
}

/*
 *	update cc.err at "->xtra" for node
 */
static
recost_updt(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	if ( utilities ) {
	  t->xtra.cc->err += 
		prop*(util_max-utilities[t->xtra.cc->class][class_val(eg)])/
				(util_max-util_min);
	} else if ( t->xtra.cc->class != class_val(eg))
		t->xtra.cc->err += prop;
}


/*
 *	do recost of egs from file
 */
int
file_recost(fname, t)
char	*fname;
ot_ptr	t;
{
        egtype   eg;
	int	count;
        
	recost_tree(t);
	count = 0;
        while ( (eg = read_eg(fname)).unordp ) {
                tree_reclass(t, eg, recost_updt, 1);
		count++;
	}
	return count;
}

/*
 *	do recost of egs from set
 */
egs_recost(egs, t)
egset	*egs;
ot_ptr	t;
{
        egtype   eg;
        foregtype       egp;
        
	recost_tree(t);
        foreacheg(egs, eg, egp) {
                tree_reclass(t, eg, recost_updt, 1);
	}
}



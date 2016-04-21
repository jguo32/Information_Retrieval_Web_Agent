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
 *	assume leaf counts are currect and recount all others
 *      may cause trouble if unknowns where ignored during growing;
 *      may also cause trouble if options have different counts
 *	NB.. not really supported, used in tgendta.c
 */
chkcnt_tree(t)
ot_ptr	t;
{
	int	i,j,k;
	bt_ptr	option;
	if ( t ) {
	    if ( ttest(t) ) {
		foroptions(option,j,t) {
		  fordecs(k)
		    t->eg_count[k] = 0.0;
		  foroutcomes(i,option->test) {
		    chkcnt_tree(option->branches[i]);
		    fordecs(k)
		      t->eg_count[k] += option->branches[i]->eg_count[k];
		  }
		}		  
	    }
	    t->tot_count = 0.0;
	    fordecs(k)
	      t->tot_count += t->eg_count[k];
	  }
}


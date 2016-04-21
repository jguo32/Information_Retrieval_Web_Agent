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



/***************************************************************************/
/*
 *	rem_option(t, op) -- rem option with index op
 *	
 */
rem_option(t, op)
ot_ptr	t;
int	op;
{
	register int	i;
	register bt_ptr	option;
	if ( !ttest(t) )
		return;
	if ( !toptions(t) ) {
		force_leaf(t);
		return;
	}
	if ( toptions(t) && t->option.s.c > 2) {
		/*
		 *	decrease multiple options by 1
		 */
		free_btree(t->option.s.o[op]);
		for (i=op+1; i<t->option.s.c ; i++)
			t->option.s.o[i-1] = t->option.s.o[i];
		t->option.s.c--;
	} else {
		/*
		 *	have 2 options so convert to single
		 */
		unset_flag(t->tflags,optiont);
		option = t->option.s.o[op?0:1];
		free_btree(t->option.s.o[op]);
		sfree(t->option.s.o);
		t->option.o = option;
	}
}


/***************************************************************************/
/*
 *	rem_ooption(t, op) -- rem. all options except one with index op
 *	
 */
rem_ooption(t, op)
ot_ptr	t;
int	op;
{
	register int	j;
	register bt_ptr	option;
	if ( !ttest(t) || !toptions(t) ) {
		return;
	}
	/*
	 *	have >=2 options so convert to single
	 */
	foroptions(option, j, t) 
		if ( j!=op )
	    		free_btree(option);
	unset_flag(t->tflags,optiont);
	option = t->option.s.o[op];
	sfree(t->option.s.o);
	t->option.o = option;
}

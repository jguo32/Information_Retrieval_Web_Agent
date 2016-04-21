/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* prune -- tree pruner.
 *
 *	Author - Wray Buntine
 *
 */
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*  #define DEBUG_DECPRUNE  */

static void
dec_prune_node(t)
ot_ptr	t;
{
	int     i,j,dec = -1;
	bt_ptr  option;
        if ( ttest(t) ) {
#ifdef DEBUG_DECPRUNE
	    printf("check node %g egs and test ", t->tot_count);
	    display_test(t->option.o->test,stdout);
	    printf(", best decision %d : ", best_dec(t->eg_count));
            for (i = 0; i < ndecs; i++) 
	      printf("%g ",t->eg_count[i]);
	    printf("\n");
#endif
            foroptions(option,i,t) {
                foroutcomes(j,option->test) {
#ifdef DEBUG_DECPRUNE
	    if ( !tleaf(option->branches[j]) )	 
	      printf("  sub node %g egs and outcome ", 
		   option->branches[j]->tot_count);
	    else
	      printf("  sub leaf %g egs and outcome ", 
		   option->branches[j]->tot_count);
	    display_outcome(t->option.o->test,j,stdout);
	    printf(", best decision %d : ", 
		   best_dec(option->branches[j]->eg_count));
            for (i = 0; i < ndecs; i++) 
	      printf("%g ",option->branches[j]->eg_count[i]);
	    printf("\n");
#endif
		    if ( !tleaf(option->branches[j]) ) {
#ifdef DEBUG_DECPRUNE
			printf("  subnode is non leaf so quitting\n");
#endif
				return;
		    }
                    if ( dec>=0 ) {
		      if ( option->branches[j]->tot_count )
			 if ( dec != 
			      best_dec(option->branches[j]->eg_count) )
					return;
		    } else
		      if ( option->branches[j]->tot_count )
			 dec = best_dec(option->branches[j]->eg_count);
#ifdef DEBUG_DECPRUNE
			printf("  updated dec to %d\n",dec);
#endif
                }
		if ( dec>=0 ) {
			force_leaf(t);
#ifdef DEBUG_DECPRUNE
			printf("  made leaf and quitting\n");
#endif
		}
            }
	}
#ifdef DEBUG_DECPRUNE
	      else
			printf("  leaf");
	printf(" node left alone\n");
#endif
}

/************************************************************************/
/*
 *	dec_prune(t)	--  prune node if all subtrees make same decision
 */
int
dec_prune(t)
ot_ptr	t;
{
#ifdef DEBUG_DECPRUNE
  
#endif
	traverse_onodes_post(t, dec_prune_node);
}

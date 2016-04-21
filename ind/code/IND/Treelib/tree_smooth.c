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

extern	int	depth;
static  bool    decprune;


/******************************************************************************/
/*
 *	tree_smooth(t) -- smooth the tree produced by tree_ave(),
 *                       i.e.  install correct probabilities at leaves,
 *                        and maybe do decision pruning
 *                      
 */
void
tree_smooth(t, dprune)
ot_ptr	t;
bool   dprune;         /*   if set prune back constant decision sub-nodes */
{
        static void _tree_smooth();
        decprune = dprune;
	_tree_smooth(t);
}

/* 
 *    this debugging flag prevents all pruning but displays
 *      actions of what WOULD be done
 */
/* #define DEBUG_DECPRUNE  */

/*
 *	_tree_smooth(t, top_weight) -- take "t->lprob" to produce smoothed
 *                                     probability vectors then decision prune
 */
static void
_tree_smooth(t)
ot_ptr	t;
{
	int	i,j,k;
	bt_ptr	option;
	int     dec = -1;   /*  -1 = unassigned, -2 = forget best decision
			     *  else = common best decision for subnodes so for 
			     */
	float   *props;
	
        ASSERT ( t );
	if ( !no_parents(t) )
	  fordecs(i) 
	    t->eg_count[i] = t->lprob * t->eg_count[i] + oparent(t)->eg_count[i];
	else
	  fordecs(i) 
	    t->eg_count[i] *= t->lprob;
        if ( ! t->tot_count )
	  t->tot_count = 0.000000001;
	if ( !tleaf(t) ) {
#ifdef DEBUG_DECPRUNE
            printf("check node %g egs, lprob %g and test ", 
		   t->tot_count, t->lprob);
            display_test(t->option.o->test,stdout);
            printf(", best decision %d : ", best_dec(t->eg_count));
            for (k = 0; k < ndecs; k++)
              printf("%g ",t->eg_count[k]);
            printf("\n");
#endif
	    foroptions(option,j,t) {
#ifdef DEBUG_DECPRUNE
              printf("check test with prop. %g of ", option->np.nprop);
              display_test(option->test,stdout);
              printf("\n");
#endif
	        foroutcomes(i, option->test) {
#ifdef DEBUG_DECPRUNE
		  if ( !tleaf(option->branches[i]) )
		    printf("  presmooth sub node %g egs, lprob %g  and outcome ",
			   option->branches[i]->tot_count,
			   option->branches[i]->lprob );
		  else
		    printf("  presmooth sub leaf %g egs, lprob %g and outcome ",
			   option->branches[i]->tot_count,
			   option->branches[i]->lprob);
		  display_outcome(t->option.o->test,i,stdout);
		  printf(", best decision %d : ",
			 best_dec(option->branches[i]->eg_count));
		  for (k = 0; k < ndecs; k++)
		    printf("%g ",option->branches[i]->eg_count[k]);
		  printf("\n");
#endif
		    _tree_smooth(option->branches[i]);
#ifdef DEBUG_DECPRUNE
		  if ( !tleaf(option->branches[i]) )
		    printf("  postsmooth sub node %g egs, lprob %g  and outcome ",
			   option->branches[i]->tot_count,
			   option->branches[i]->lprob );
		  else
		    printf("  postsmooth sub leaf %g egs, lprob %g  and outcome ",
			   option->branches[i]->tot_count,
			   option->branches[i]->lprob);
		  display_outcome(t->option.o->test,i,stdout);
		  printf(", best decision %d : ",
			 best_dec(option->branches[i]->eg_count));
		  for (k = 0; k < ndecs; k++)
		    printf("%g ",option->branches[i]->eg_count[k]);
		  printf("\n");
#endif
		    if ( decprune && dec > -2 ) {
		      if ( tleaf(option->branches[i] ) ) {
			if ( dec>=0 ) {
			  if ( dec != best_dec(option->branches[i]->eg_count) )
			    dec = -2;
			} else if ( dec == -1 )
			  dec = best_dec(option->branches[i]->eg_count);
		      } else
			dec = -2;
		    }
		}
	    }
	    if ( decprune && dec >= 0 ) {
	      t->lprob = 1.0;
	      /*
	       *   decision prune this node, so smooth childrens probs
	       */
	      for (k = 0; k < ndecs; k++)
		  t->eg_count[k] = 0;
	      foroptions(option,j,t) {
		props = subtree_prop(option);
		/*
		 *    class probabilities are the average of their children
		 */
		foroutcomes(i, option->test) {
		  for (k = 0; k < ndecs; k++)
		    t->eg_count[k] += props[i] * option->np.nprop * 
		      option->branches[i]->eg_count[k];
		}
	      }
#ifndef DEBUG_DECPRUNE
	      force_leaf(t);
#endif
#ifdef DEBUG_DECPRUNE
	      printf("pruned node %g egs, lprob %g and test ", 
		   t->tot_count, t->lprob);
	      display_test(t->option.o->test,stdout);
	      printf(", best decision %d : ", best_dec(t->eg_count));
	      for (k = 0; k < ndecs; k++)
		printf("%g ",t->eg_count[k]);
	      printf("\n");
#endif
	    } else {
	      t->lprob = 0.0;
	    }	      
	} else
	      t->lprob = 1.0;
}



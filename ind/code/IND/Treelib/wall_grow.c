/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  #define W_DEBUG   */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

#define   MAX_W_DIFF  0.2

extern  int    timeout;

wall_grow(tree, egs, Wtree_cycles, W_mina)
ot_ptr  tree;
egset	*egs;
int	Wtree_cycles;
float   W_mina;
{
        double  fabs();
        /*
         *         stuff for wallace style growing cycle
	 */
        float    diff, old_palphaval, old_NR_palphaval;
        double    old_NR_sprob, old_sprob, oldold_palphaval;
	extern float    palphaval; 
	float   W_accur = 0.01;
	double  dsprob, d2sprob;
	void   Dtree_ave();

	  /*
	   *   wallace-style growing:
	   *     grow tree, then find maximum posterior value of 
	   *     hyper-parameter "palphaval",
	   *     then grow again, with new "palphaval", until little
	   *     change in "palphaval";
	   *     save checkpoints at each stage in case no improvement
	   */
	  old_sprob = -FLOATMAX;
	  old_palphaval = palphaval;
	  for ( ; Wtree_cycles && !timeout; Wtree_cycles-- ) {
	    maketree(tree, egs);
	    /*
	     *     do simple Newt.-Raphs to find best alpha
	     */
#ifdef  W_DEBUG
	    fprintf(stdrep, 
		    "Starting Wallace cycle, alpha = %lg\n",
		    palphaval);
#endif
	    oldold_palphaval = old_palphaval;
	    old_palphaval = palphaval;
	    old_NR_palphaval = palphaval + 1.0;
	    old_NR_sprob = FLOATMAX;
	    while ( fabs((double)(old_NR_palphaval - palphaval)) > W_accur 
		    && !timeout ) {
	      set_palpha(palphaval, tree->eg_count);
	      (void)Dtree_ave(tree,&dsprob,&d2sprob);
	      /*
	       *     if new value gave no improvement, do new split
	       */
	      if ( old_NR_sprob < FLOATMAX  &&
		  old_NR_sprob > tree->xtra.pn.sprob ) {
		palphaval = ( old_NR_palphaval + palphaval ) * 0.5 ;
	      } else {
		old_NR_sprob = tree->xtra.pn.sprob;
		old_NR_palphaval = palphaval;
		/*
		 *     else do N-R change with upperbound of MAX_W_DIFF
		 */
		if ( dsprob == 0.0 ) continue;
		if ( dsprob > 0.0 ) {
		  /*     must increase palphaval */
		  if ( d2sprob >= 0.0 )
		    palphaval += MAX_W_DIFF;
		  else {
		    diff = dsprob / d2sprob;
		    if ( diff > -MAX_W_DIFF )
		      palphaval -=  diff;
		    else
		      palphaval += MAX_W_DIFF;
		  }
	        } else {
		  /*     must decrease palphaval */
		  if ( d2sprob <= 0.0 )
		    palphaval -= MAX_W_DIFF;
		  else {
		    diff = dsprob / d2sprob;
		    if ( diff > -MAX_W_DIFF )
		      palphaval +=  diff;
		    else
		      palphaval -= MAX_W_DIFF;
		  }
	        }
		if ( palphaval < W_mina ) 
		  palphaval = W_mina;
	      }
#ifdef  W_DEBUG
	    fprintf(stdrep, 
		    "  Updating old alpha %lg, alpha = %lg, old sprob = %lg\n",
		        old_NR_palphaval, palphaval, old_NR_sprob);
#endif
	    }
	    if ( fabs((double)(old_palphaval - palphaval)) < W_accur )
	      break;
	    if ( tree->xtra.pn.sprob < old_sprob ) {
	      /*
	       *   no improvement from this cycle, so redo last
	       *   cycle and quit
	       */
	      palphaval = oldold_palphaval;
	      Wtree_cycles = 2;
	      old_sprob = -FLOATMAX;	 
	    } else
	      old_sprob = tree->xtra.pn.sprob;
	    if ( Wtree_cycles>1 ) {
	      /*
	       *    on next cycle, start with whole new tree
	       */
	      free_tree(tree);
	      tree = new_node((bt_ptr)0);
	      tree->tot_count = setsize(egs);
	    }
	  }
}
	

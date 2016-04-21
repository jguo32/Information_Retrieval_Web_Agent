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

static   double genprob;


/******************************************************************************/
/*
 *	tree_smpl(t) -- sample from the option tree to create a single
 *                       tree, assumes tree_avet() just been called on t
 */
double
tree_smpl(t)
ot_ptr  t;
{
static void _tree_smpl();
	genprob = 1.0;
	_tree_smpl(t);
	return genprob;
}

static void
_tree_smpl(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;
double fran;
	if ( !tleaf(t) ) {
	    if ( (fran=frandom()) < t->lprob ) {
		/*
		 *  make this a leaf! 
		 */
		force_leaf(t);
		genprob *= t->lprob;
	        t->lprob = 1.0;
	    } else {
		genprob *= 1.0-t->lprob;
	        t->lprob = 0.0;
		if ( options(t) > 1 ) {
		    fran = frandom();
                    /*
                     *      only works because j counts downwards
                     */
                    foroptions(option,j,t) {
			/*
			 *   this case if error accum. in 
			 *	substraction of fran
			 */
			if ( j==0 && options(t)==1 ) {
				fran = -2.0;
				break;
			}
			if ( fran > -1.0 )
			    if ( (fran-=option->np.nprop ) <= 0.0 ) 
				fran = -2.0;
			    else
				rem_option(t,j);
			else
                        	rem_option(t,j);
		    }
		    ASSERT ( options(t) == 1 && fran < -1.0 )
		    genprob *= t->option.o->np.nprop;
		    foroptions(option,j,t) 
                        foroutcomes(i, option->test) 
                            _tree_smpl(option->branches[i]);
		} else {
               	    foroptions(option,j,t) 
                	foroutcomes(i, option->test) 
			    _tree_smpl(option->branches[i]);
                }

	    }	      
	} else
	      t->lprob = 1.0;
}

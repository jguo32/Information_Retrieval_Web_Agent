/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* grow tree with cost complexity pruner using ? SE rule with test set
 * (see CART p 79,309)
 *
 *	Author - Wray Buntine
 *
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/************************************************************************
 *
 *	CC_test_grow(egs)
 */
CC_test_grow(tree, egs, prop, pf, seed)
ot_ptr	tree;
egset	*egs;
float	prop;
float	pf;
long	seed;
{
	extern int verbose;
	egset	*tegs, *gegs;

        if ( verbose ) {
            fprintf(stdrep,"TEST SET COST-COMPLEXITY PRUNING OPTIONS:\n");
            fprintf(stdrep,"the training set is %g of the lot;\n",prop);
            fprintf(stdrep,"using the %g-SE rule to prune;\n",pf);
            if ( seed )
                fprintf(stdrep,"data shuffled with seed %ld;\n",seed);
	    fprintf(stdrep,"\n");
        }
	if ( seed ) {
		srandom(seed);
	}
	shuffle_set(egs);
	partition(egs, prop, &gegs, &tegs);
	maketree(tree, gegs);
	egs_recost(tegs,tree);
	CC_test_prune(tree, pf, setsize(tegs));
	sfree(gegs);
	sfree(tegs);
}

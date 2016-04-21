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
 *	compute list of alphas for R_\alpha minimising trees
 */

static float	*alph_list;
static float	*cost_list;
static int     *size_list;
static int	alph_list_i;

static
add_alph(cost, recost, comp, alph)
float	cost, recost, comp, alph;
{
	alph_list[alph_list_i] = alph;    
	cost_list[alph_list_i] = cost;  
	size_list[alph_list_i++] = comp;  
/*	fprintf(stdrep, "cost %g, recost %g, comp %g, alph %g\n",
                cost, recost, comp, alph);	*/
}

extern 
alpha_list(t, alist, cstore, sstore)
ot_ptr	t;
float	*alist;		/*  alpha list to match   */
int	*sstore;	/*  list of matching sizes  */
float     *cstore;        /*  list of costs */
{
	alph_list = alist;
	cost_list = cstore;
	size_list = sstore;
	alph_list_i = 0;
	traverse_trees(t, add_alph);
#ifdef  DEBUG
	{
		int	i;
		fprintf(stdrep,"Alpha_list: ");
		for (i=0; i<alph_list_i; i++)
			fprintf(stdrep," %g",alph_list[i]);
		fprintf(stdrep,"\n");
	}
#endif
	return alph_list_i-1;
}

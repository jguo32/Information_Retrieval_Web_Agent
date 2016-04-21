/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* grow tree with cost complexity pruner using pf SE rule with CV
 * (see CART p 79,309)
 *
 *	Author - Wray Buntine
 *
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/* #define CHECK */
/* #define DEBUG */
#ifdef DEBUG
tprint_flags  cpf;
#endif
extern int verbose;

/************************************************************************
 *
 *	CC_CV_grow(egs)
 */
float CC_CV_grow(tree, egs, folds, pf, seed)
ot_ptr	tree;
egset	*egs;
int	folds;
float	pf;
long	seed;
{
	egset	*tegs, *gegs, *copyegs;
	int	i;
#ifdef DEBUG
	int	j;
#endif
	float	*alphas;
	float	the_cost , *costs, ESE, SE;
	int	alpha_size, max_i;
	int     *sizes;
	int	save_verbose = verbose;
	ot_ptr	t;
	double	sqrt();
	extern  int  timeout, intint;

#ifdef DEBUG
	null_flags(cpf);
        set_flag(cpf,ccxtra);
        set_flag(cpf,counts);
        set_flag(cpf,decis);
        set_flag(cpf,intnl);
#endif
	if ( folds > setsize(egs) )
	  folds = setsize(egs);
	/*
	 *  maketree()  sorts egs so for later partitioning, must keep unsorted
	 */
	if ( verbose ) {
	    fprintf(stdrep,"CROSS VALIDATION COST-COMPLEXITY PRUNING OPTIONS:\n");
	    fprintf(stdrep,"doing cross validation with %d folds;\n",folds);
	    fprintf(stdrep,"using the %g-SE rule to prune;\n",pf);
	    if ( seed )
		fprintf(stdrep,"data shuffled with seed %ld;\n",seed);
	     fprintf(stdrep,"\n");
	}
	if (seed ) {
		srandom(seed);
	}
	shuffle_set(egs);
	copyegs = copy_set(egs);
	maketree(tree, egs);
	recost_tree(tree);
	if ( timeout || intint ) myexit(24);
	sizes = make_ivec(tree_size(tree) + 5 - leaf_size(tree));
	alphas = make_fvec(tree_size(tree) + 5 - leaf_size(tree));
	costs = make_fvec(tree_size(tree) + 5 - leaf_size(tree));
	alpha_size = alpha_list(tree, alphas, costs, sizes);
	if ( !alpha_size ) {
	  alphas[0] = 0.0;
	  alpha_size = 1;
	}
	if ( verbose ) {
	    fprintf(stdrep,"CROSS VALIDATION COST-COMPLEXITY DETAILS:\n");
	    fprintf(stdrep,"complexity parameters: ");
	    pr_fvec(alphas,alpha_size);
	    fprintf(stdrep,"\n");
	    fprintf(stdrep,"tree sizes: ");
	    pr_ivec(sizes,alpha_size);
	    fprintf(stdrep,"\n");
	    fprintf(stdrep,"resubstitution costs: ");
	    pr_fvec(costs,alpha_size);
	    fprintf(stdrep,"\n");
	}
	sfree(sizes); 
	sfree(costs);
	costs = make_fvec(alpha_size+1);
	for (i=0; i<alpha_size-1; i++) 
		alphas[i] = sqrt(alphas[i]*alphas[i+1]);
	alphas[i] *= 1.1;
	if ( verbose ) {
	    fprintf(stdrep,"complexity parameters used internally: ");
	    pr_fvec(alphas,alpha_size+1);
	    fprintf(stdrep,"\n");
	}
	for (i=0; i<folds; i++) {
		t = new_node((bt_ptr)0);
		ithpartition(copyegs, i, folds, &gegs, &tegs);
		verbose = 0;
		maketree(t, gegs);
		verbose = save_verbose;
		if ( timeout || intint ) myexit(24);
#ifdef CHECK
                tree_fcheck(t);
		if ( tree_pcheck(t) > 0 ) error("parent pointers out in tree");
#endif       
		egs_recost(tegs,t);
		alpha_cost(t, alphas, costs, alpha_size+1);
#ifdef DEBUG
		if ( verbose>2 ) {
		  fprintf(stdrep,"test size = %d\n", setsize(tegs));
		  fprintf(stdrep,"costs: ");
		  pr_fvec(costs,alpha_size+1);
		  fprintf(stdrep,"\n");
		  if ( verbose>3 ) {
		    print_set(tegs);
		    print_tree(t,cpf,20);
		    if ( verbose>4 ) {
		      for (j=0; j<=alpha_size; j++)  {
		        CC_costprune(t, alphas[j]);
                	egs_recost(tegs,t);
			fprintf(stdrep,"alpha = %g\n", alphas[j]);
			print_tree(t,cpf,20);
		      }
		    }
		  }
		}
#endif
		uncost_tree(t);
		free_tree(t);
		free_set(gegs);
		free_set(tegs);
	}
	max_i = 0;
	for (i=0; i<alpha_size; i++)  {
		costs[i] /= setsize(egs);
		if ( costs[i] < costs[max_i] )
			max_i = i;
	}
	SE = sqrt(costs[max_i]*(1.0-costs[max_i])/setsize(egs));
	ESE = costs[max_i] + pf * SE;
	for (i=max_i; i<alpha_size; i++) 
		if ( costs[i] > ESE )
			break;
	i --;
	if ( verbose ) {
		fprintf(stdrep,"cross validated costs: ");
		pr_fvec(costs,alpha_size+1);
		fprintf(stdrep,"\n");
		fprintf(stdrep, "min cost parameter = %g\n", alphas[max_i]);
		fprintf(stdrep, "standard error in cost = %g\n", SE);
		fprintf(stdrep, "chosen parameter = %g\n\n", alphas[i]);
	}
	uncost_tree(tree);
	CC_costprune(tree, alphas[i]);
	sfree(alphas);
	the_cost = costs[i];
	sfree(costs);
	free_set(copyegs);
	return the_cost;
}

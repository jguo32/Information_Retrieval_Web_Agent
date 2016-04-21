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


static	ot_ptr	t_root;
static	float	*class_dec;
static  float   assigned;            /* amount of branch probability assigned
					to the class probability vector */

/******************************************************************************/
/*
 *	tree_class(t, tp, eg) -- return expected class probabilities
 *				   for standard class prob. tree
 *                  NB.  Zflg stuff OK for graphs cause a join
 *			 node MUST always have a non-zero count
 */
float	*
tree_class(t, eg)
ot_ptr	t;
egtype	eg;
{
	static int	_tree_class_updt_l();
	register	int	i;
	t_root = t;
	class_dec = make_fvec(ndecs);
	fordecs(i)
		class_dec[i]=0.0;
	tree_reclass(t, eg, _tree_class_updt_l, 0);
	return class_dec;
}

/******************************************************************************/
/*
 *	tree_class2(t, tp, eg) -- return expected class probability squared
 *				   for standard class prob. tree
 */
float	*
tree_class2(t, eg)
ot_ptr	t;
egtype	eg;
{
	static int	_tree_class2_updt_l();
	register	int	i;
	t_root = t;
	class_dec = make_fvec(ndecs);
	fordecs(i)
		class_dec[i]=0.0;
	tree_reclass(t, eg, _tree_class2_updt_l, 0);
	return class_dec;
}

/******************************************************************************/
/*
 *	tree_class_ave(t, tp, eg) -- return expected class probabilities
 *				   for averaged class prob. tree
 */
float	*
tree_class_ave(t, eg)
ot_ptr	t;
egtype	eg;
{
	static int	_tree_class_updt();
	register	int	i;
	t_root = t;
	class_dec = make_fvec(ndecs);
	fordecs(i)
		class_dec[i]=0.0;
	assigned = 0.0;
	tree_reclass(t, eg, _tree_class_updt, 1);
	if ( assigned < 0.95 || assigned > 1.05 )
	  fprintf(stderr,
                        "probability for example %d unnormalised (%lg)\n",
                                  negs, assigned );
	fordecs(i)
	    class_dec[i] /= assigned;
	return class_dec;
}

/******************************************************************************/
/*
 *	tree_class_ave(t, tp, eg) -- return expected class probabilities
 *				     squared
 *				   for averaged tree
 */
float	*
tree_class2_ave(t, eg)
ot_ptr	t;
egtype	eg;
{
	static int	_tree_class2_updt();
	register	int	i;
	t_root = t;
	class_dec = make_fvec(ndecs);
	fordecs(i)
		class_dec[i]=0.0;
	assigned = 0.0;
	tree_reclass(t, eg, _tree_class2_updt, 1);
	if ( assigned < 0.95 || assigned > 1.05 )
	  fprintf(stderr,
                        "probability for example %d unnormalised (%lg)\n",
                                  negs, assigned );
	fordecs(i)
	    class_dec[i] /= assigned;
	return class_dec;
}

/*******************************************************************************/
/*
 *      Routines for updating cummulative totals in "class_dec[]"
 *      Assume "t->tot_count" is total egs at node
 *      and t->eg_count" is expected class probability for node
 */
static
_tree_class_updt(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	register	int	i;
	assigned += prop *= t->lprob;
	if ( prop ) { 
	  if ( ! t->tot_count ) {
	    fordecs(i)
	      class_dec[i] += prop*palpha(0.0,i)/palphatot(0.0);
	  } else  { 
	    fordecs(i)
	      class_dec[i] += prop*t->eg_count[i];
	  }
	}
}

static
_tree_class2_updt(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	register	int	i;
	assigned += prop *= t->lprob;
	if ( prop ) {
	  if ( ! t->tot_count ) {
	    fordecs(i)
		class_dec[i] += prop * palpha(0.0,i) / palphatot(0.0) *
		  (palpha(0.0,i) + (palphatot(0.0)-palpha(0.0,i))/
		                (palphatot(0.0)+1.0)) / palphatot(0.0);
	  } else {
	    fordecs(i)
		class_dec[i] += prop * t->eg_count[i] *
		  (t->eg_count[i] + (1.0-t->eg_count[i])/
		       (t->tot_count+palphatot(t->tot_count)+1.0));
	  }
	}
}

static
_tree_class_updt_l(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	register	int	i;
	if ( !tleaf(t) )
		return;
	if ( ! t->tot_count )
	   if ( Zflg || no_parents(t) )
		t = t_root;
	   else
		t = oparent(t);
	fordecs(i)
		class_dec[i] += prop*t->eg_count[i];
}

static
_tree_class2_updt_l(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	register	int	i;
	if ( !tleaf(t) )
		return;
	if ( ! t->tot_count )
	   if ( Zflg || no_parents(t) )
		t = t_root;
	   else
		t = oparent(t);
	fordecs(i)
		class_dec[i] += prop * t->eg_count[i] *
		  (t->eg_count[i] + (1.0-t->eg_count[i])/
		      (t->tot_count+palphatot(t->tot_count)+1.0));
}

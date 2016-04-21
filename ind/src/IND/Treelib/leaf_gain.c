/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "DEFN.h"
#include "TREE.h"

static	double	this_prob;
static	float	counta[MAXCLSS];
static	float	prob[MAXCLSS];
static	double	tot, totr;
static	bool	probs_set;
/*
 *		if subsampling is done, sample has been
 *		decreased in size, this does the rescaling
 *		so "leaf_prob" etc. are still reasonable measures
 *		in terms of the log-prob score,
 */
static float	scale_up = 1.0;
static float	save_scale_up = 1.0;

set_scale_prob(val)
float   val;
{
	save_scale_up = scale_up = val;
}

scale_prob(flg)
int flg;
{
	if ( flg ) 
		scale_up = save_scale_up;
	else
		scale_up = 1.0;
}

/***************************************************************************/
/*
 *	leaf_prob(counts) -- return prob for these counts and store intermediate
 *			calc. in "this_prob" for later use by "set_leaf_stats()"
 *			or "leaf_gain()"
 *		NB.  similar code in set_leaf_prob() below
 */
float	
leaf_prob( t, testing)
ot_ptr t;
egtesttype  testing;
{
	float	*counts = t->eg_count;
	register	int j;
	register	double	prb;
	double	gamma();
	egtesttype   save_testing = t->testing;

	totr=tot=0.0;
	fordecs(j)  totr += counts[j];
	probs_set = 0;
	t->testing = testing;
	this_prob = leaf_weight(totr*scale_up, t);
	fordecs(j) {
		tot += counta[j] = prb = (counts[j]+palpha(totr,j))*scale_up;
		this_prob += gamma(prb);
	}
	t->testing = save_testing;
	return this_prob - gamma(tot);
}
/***************************************************************************/
/*
 *	log_beta(counts) -- like leaf_prob() but without leaf weights
 */
float	
log_beta(counts)
float	*counts;
{
	register	int j;
	register	double	prb, alph;
	double	gamma();

	totr=tot=0.0;
	fordecs(j)  totr += counts[j];
	fordecs(j) {
		tot += counta[j] = prb = counts[j]+(alph=palpha(totr,j));
		this_prob += gamma(prb) - gamma(alph);
	}
	return this_prob - gamma(tot) + gamma(palphatot(totr));
}

/***************************************************************************/
/*
 *	D2leaf_prob() -- assume "leaf_prob" has just been called
 *		determine 2nd derivative of leaf_prob w.r.t. palpha
 */
float	
D2leaf_prob()
{
	register	int j;
	double	this_Dprob;
	  
	this_Dprob = D2leaf_weight(totr*scale_up);
	fordecs(j) {
		this_Dprob += di2gamma(counta[j]);
	}
	return this_Dprob - ndecs * ndecs * di2gamma(tot);
}

/***************************************************************************/
/*
 *	Dleaf_prob(counts) -- 
 *		determine derivative of leaf_prob w.r.t. palpha
 *              calling with counts==0 uses previously stored values
 */
float	
Dleaf_prob(counts)
float   *counts;
{
	register	int j;
	double	this_Dprob;

	if ( counts) {
	  totr=tot=0.0;
	  fordecs(j) 
		totr += counts[j];
	  fordecs(j) 
		tot += counta[j] = (counts[j]+palpha(totr,j))*scale_up;
	}
	this_Dprob = Dleaf_weight(totr*scale_up);
	fordecs(j) {
		this_Dprob += digamma(counta[j]);
	}
	return this_Dprob - ndecs * digamma(tot);
}


/***************************************************************************/
/*
 *	calc_leaf_prob() -- calc. leaf class-probabilities from previous
 *			   assumes tot + counta already set
 *			   leave answer in "dv"
 */
calc_leaf_prob(dv)
float	*dv;
{
	register int i;
	if ( probs_set ) {
            if ( dv!=prob ) fordecs(i)
		dv[i] = prob[i];
	} else {
	    fordecs(i)
		dv[i] = counta[i]/tot;
	    if ( dv==prob)
		probs_set = 1;
	}
}

/***************************************************************************/
/*
 *	set_leaf_prob() -- calc. leaf class-probabilities from counts
 */
set_leaf_prob(counts)
float	*counts;
{
	register	int j;
	double	gamma();

	totr=tot=0.0;
	fordecs(j) 
		totr += counts[j];
	fordecs(j) 
		tot += counts[j] += palpha(totr,j);
	fordecs(j) 
		counts[j] /= tot;
}

/***************************************************************************/
/*
 *	leaf_error(counts) -- return error estimate for these counts
 */
leaf_errstd(counts,err,var)
float	*counts, *err, *var;
{
	register	int j;
	float	max=  -1.0, tot, prb;

	totr=tot=0.0;
	fordecs(j) 
		totr += counts[j];
	fordecs(j) {
		tot += prb = counts[j]+palpha(totr,j);
		if (prb > max)
			max = prb;
	}
	*err = (tot-max)/tot;
	*var = max*(tot-max)/tot/(tot+1.0)/tot;
}

/***************************************************************************/
/*
 *	vec_propstd(counts) -- return vector of estimates for these counts
 */
vec_propstd(counts,prop,var,n)
float	*counts, *prop, *var;
int	n;
{
	register	int j;
	float		prb;

	totr = tot = 0.0;
	fordecs(j) 
		totr += counts[j];
	for (j=n-1; j>=0; j--)
		tot += counts[j]+palpha(totr,j);
	for (j=n-1; j>=0; j--)  {
		prop[j] = (prb=counts[j]+palpha(totr,j))/tot;
		var[j] = prb*(tot-prb)/tot/(tot+1.0)/tot;
	}
}

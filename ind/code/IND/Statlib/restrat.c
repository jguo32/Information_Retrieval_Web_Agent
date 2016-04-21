/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"

/***************************************************************************/
/*
 *	restrat(dv) -- re-stratifies probability vector to remove
 *		       class stratification in sample.
 */
void restrat_fprob(dv)
float*	dv;
{
	if ( strat_true ) {
	    int	i;
	    double	sum;
	    /*
	     *	weight each prob. by "true_prob/strat_prob", then re-normalise
	     */
	    sum = 0.0;
	    fordecs(i)
		sum += dv[i] /= strat_true[i];
	    fordecs(i)
		dv[i] /= sum;
	}
}

/***************************************************************************/
/*
 *	strat(dv) -- stratify probability vector to add
 *		       class stratification .
 */
float	*
strat_fprob(dv)
float*	dv;
{
	if ( strat_true ) {
	    int	i;
	    double	sum;
	    /*
	     *	weight each prob. by "true_prob/strat_prob", then re-normalise
	     */
	    sum = 0.0;
	    fordecs(i)
		sum += dv[i] *= strat_true[i];
	    fordecs(i)
		dv[i] /= sum;
	}
	return	dv;
}


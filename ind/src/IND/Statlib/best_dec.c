/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"

/***************************************************************************/
/*
 *	best_dec(dv) -- return the decision in a vector of probs.
 *			with most utility
 */
int
best_dec(dv)
register float	*dv;
{
	register int	best_d;
	register float	best_u;
	register int	i,j;
	if ( utilities ) {
	    float	curr;
	    best_d = DONTKNOW;
	    best_u = -FLOATMAX;
	    fordecs(i) {
		curr = 0.0;
	        fordecs(j)
		    curr += dv[j] * utilities[i][j];
		if (curr > best_u)
		{
			best_u = curr;
			best_d = i;
		}
	    }
	    return best_d;
	} else {
	    best_d = 0;
	    best_u = dv[0];
	    fordecs(i)
		if (dv[i] > best_u)
		{
			best_u = dv[i];
			best_d = i;
		}
	    return best_d;
	}
}

/***************************************************************************/
/*
 *	best_util(dv) -- return the best utlity for prob. vectr dv
 */
float
best_util(dv)
register float	*dv;
{
	register float	best_u;
	register int	i,j;
	if ( utilities ) {
	    float	curr;
	    best_u = -FLOATMAX;
	    for (i = 0; i < ndecs; i++) {
		curr = 0.0;
	        for (j = ndecs-1; j>=0; j--) 
		    curr += dv[j] * utilities[i][j];
		if (curr > best_u) {
			best_u = curr;
		}
	    }
	    return best_u;
	} else {
	    best_u = dv[i];
	    for (i--; i >= 0; i--)
		if (dv[i] > best_u)
		{
			best_u = dv[i];
		}
	    return best_u;
	}
}


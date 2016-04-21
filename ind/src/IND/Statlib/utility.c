/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"


/***************************************************************************/
/*
 *	mtx_utility(freq) -- return the utility of decisions in matrix
 *			"freq"  (=  actual * predicted )
 */
float
mtx_utility(freq)
double	**freq;
{
	int	i,j;
	float	util = 0.0;
	if ( utilities ) {
	    fordecs(i)
	            fordecs(j)
		        util += freq[i][j] * utilities[i][j];
	    return util;
	} else {
	    fordecs(j)
		util += freq[i][i];
	    return util;
	}
}


/***************************************************************************/
/*
 *	prob_util(dv, dec) -- return the utility of making decision "dec"
 *			given prob. vector "dv"
 */
float
prob_util(dv, dec)
float*	dv;
int	dec;
{
	float	util = 0.0;
	if ( utilities ) {
	    int	j;
	    fordecs(j)
		    util += dv[j] * utilities[dec][j];
	    return util;
	} else {
	    return dv[dec];
	}
}


/***************************************************************************/
/*
 *	probn_util(dv, dec) -- return the utility of making decision "dec"
 *			given unnormalized prob. vector "dv"
 */
float
probn_util(dv, dec)
float*	dv;
int	dec;
{
	float	util = 0.0, normlz = 0.0;
	int	j;
	if ( utilities ) {
	    fordecs(j) {
		    util += dv[j] * utilities[dec][j];
		    normlz += dv[j];
	    }
	    return util/normlz;
	} else {
	    fordecs(j)
		normlz += dv[j];
	    return dv[dec]/normlz;
	}
}


/***************************************************************************/
/*
 *	utility_diff(dv) -- return the utility differential
 *			    for prob. vector "dv"
 */
float
utility_diff(dv)
float*	dv;
{
	float	min=1.0e+10, max=  -1.0e+10;
	int	dec;
	float	util;
	int	j;
	if ( utilities ) {
	    fordecs(dec) {
	    	util = 0.0;
	        fordecs(j)
		    util += dv[j] * utilities[dec][j];
		if ( util > max )
			max = util;
		else if ( util < min )
			min = util;
	    }
	    return max - min;
	} else {
	    fordecs(dec) {
		if ( dv[dec] > max )
			max = dv[dec];
		else if ( dv[dec] < min )
			min = dv[dec];
	    }
	    return max - min;
	}
}


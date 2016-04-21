/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "Lib.h"

/****************************************************************************/
/*
 *	count_fvec(d)	-  (Laplacian) convert vector of counts to probabilities
 */
count_fvec(d,dim,alpha)
register float*   d;
float  alpha;
int	dim;
{
	int     i;
	register double	tot=0.0;
	forless(i,dim)
		tot += d[i];
	if ( alpha > 0.0 || tot > 0.0 )
	    forless(i,dim)
		d[i] = (d[i] + alpha) / (tot + dim*alpha);
	else 
	    forless(i,dim)
		d[i] = 1.0 / dim;
}

/****************************************************************************/
/*
 *	norm_fvec(d)	-  convert vector of counts to probabilities
 */
norm_fvec(d,dim)
float*   d;
{
	register int     i;
	register double	tot=0.0;
	forless(i,dim)
		tot += d[i];
	if ( tot > 0.0 )
	    forless(i,dim)
		d[i] = d[i] / tot;
}

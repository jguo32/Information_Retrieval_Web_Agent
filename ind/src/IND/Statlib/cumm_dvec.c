/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include "Lib.h"
/****************************************************************************/
/*
 *	cumm_dvec(d)	-  return vector that is cummulative of input vector
 */
double	*cumm_dvec(n,d)
double   *d;		/*  a vector */
int	n;		/*  its length  */
{
	double	*nd = (double *) salloc(n*sizeof(double));
	int	i;
	nd[0] = d[0];
	for (i = 1; i < n; i++)
		nd[i] = nd[i-1] + d[i];
	return nd;
}

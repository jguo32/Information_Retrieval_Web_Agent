/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	copy_fvec(vec, dim) - make space for float vector, and copy vals
 */
float *
copy_fvec(vec, dim)
int	dim;
float	*vec;
{
	float	*m;
	int     j;
	m = (float *) salloc(dim * sizeof(float));
	for (j = 0; j < dim; j++)
		m[j] = vec[j];
	return m;
}

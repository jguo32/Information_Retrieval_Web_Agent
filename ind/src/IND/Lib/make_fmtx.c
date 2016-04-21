/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"
  
/****************************************************************************/
/*
 *	make_fmtx(dim1, dim2) - make space for float matrix, and zero
 */
float **
make_fmtx(dim1, dim2)
int	dim1, dim2;
{
	float	**m;
	int     i,j;
	m = (float **) salloc(dim1 * sizeof(float *));
	if (m) for (i = 0; i < dim1; i++) {
		if ( m[i] = (float *) salloc(dim2 * sizeof(float)) )
		    for (j = 0; j < dim2; j++)
			m[i][j] = 0.0;
		else return (float **) 0;
	}
	return m;
}

/****************************************************************************/
/*
 *	make_fvec(dim) - make space for float vector, and zero
 */
float *
make_fvec(dim)
int	dim;
{
	float	*m;
	int	i;
	if ( m = (float *) salloc(dim * sizeof(float)) )
          for (i = 0; i < dim; i++) 
             m[i] = 0.0;
	return m;
}

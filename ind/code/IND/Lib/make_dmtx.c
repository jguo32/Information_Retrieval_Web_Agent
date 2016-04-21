/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"

/****************************************************************************/
/*
 *	make_dmtx(dim1, dim2) - make space for double matrix, and zero
 */
double **
make_dmtx(dim1, dim2)
int	dim1, dim2;
{
	double	**m;
	int     i,j;
	if ( m = (double **) salloc(dim1 * sizeof(double *)) )
	    for (i = 0; i < dim1; i++) {
		if ( m[i] = (double *) salloc(dim2 * sizeof(double)) )
		    for (j = 0; j < dim2; j++)
			m[i][j] = 0.0;
		else
		    return (double **)0;
	    }
	return m;
}


/****************************************************************************/
/*
 *	make_dvec(dim) - make space for double vector, and zero
 */
double *
make_dvec(dim)
int	dim;
{
	double	*m;
	int     j;
	if ( m = (double *) salloc(dim * sizeof(double)) )
	    for (j = 0; j < dim; j++)
		m[j] = 0.0;
	return m;
}

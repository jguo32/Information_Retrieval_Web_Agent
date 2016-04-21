/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	make_imtx(dim1, dim2) - make space for int matrix, and zero
 */
int **
make_imtx(dim1, dim2)
int	dim1, dim2;
{
	int	**m;
	int     i,j;
	if ( m = (int **) salloc(dim1 * sizeof(int *)) )
	    for (i = 0; i < dim1; i++) {
		if ( m[i] = (int *) salloc(dim2 * sizeof(int)) )
		    for (j = 0; j < dim2; j++)
			m[i][j] = 0.0;
		else
		    return (int **)0;
	    }
	return m;
}


/****************************************************************************/
/*
 *	make_ivec(dim) - make space for int vector, and zero
 */
int *
make_ivec(dim)
int	dim;
{
	int	*m;
	int     j;
	if ( m = (int *) salloc(dim * sizeof(int)) )
	    for (j = 0; j < dim; j++)
		m[j] = 0.0;
	return m;
}


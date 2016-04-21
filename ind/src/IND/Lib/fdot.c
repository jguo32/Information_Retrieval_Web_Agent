/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>

/****************************************************************************/
/*
 *	dot_vec(v1,v2,dim)	- dot two vectors
 */
float
dot_fvec(v1,v2,dim)
float	*v1, *v2;
int	dim;
{
	int     i;
	float	dot = 0.0;
	if ( !v2 )
		v2 = v1;
	for (i = 0; i < dim; i++)
	 	dot += v1[i] * v2[i];
	return dot;
}

/****************************************************************************/
/*
 *	dot_mtx(m1,m2,dim)	- dot two matrices
 */
float
dot_fmtx(m1,m2,dim)
float	**m1, **m2;
int	dim;
{
	int     i,j;
	float	dot = 0.0;
	if ( !m2 )
		m2 = m1;
	for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	 	dot += m1[i][j] * m2[i][j];
	return dot;
}

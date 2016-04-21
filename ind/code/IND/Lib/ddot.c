/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	dot_vec(v1,v2,dim)	- dot two vectors
 */
double
dot_dvec(v1,v2,dim)
double	*v1, *v2;
int	dim;
{
	int     i;
	double	dot = 0.0;
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
double
dot_dmtx(m1,m2,dim)
double	**m1, **m2;
int	dim;
{
	int     i,j;
	double	dot = 0.0;
	if ( !m2 )
		m2 = m1;
	for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	 	dot += m1[i][j] * m2[i][j];
	return dot;
}

/****************************************************************************/
/*
 *	dot_mtx(m1,m2,dim)	- dot two matrices
 */
double *
dot_dmv(m1,v2,dim)
double	**m1, *v2;
int	dim;
{
	int     i,j;
	double	*dot;
	dot = make_dvec(dim);
	for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	 	dot[i] += m1[i][j] * v2[j];
	return dot;
}

/*
 *      normalise so euclidean size is 1
 */
norm_dvec(vec,dim)
double  *vec;
int     dim;
{
        double          grandsum=0.0, sqrt();
        register        int     i;
        for ( i=dim-1; i>=0; i-- )
                grandsum += vec[i] * vec[i];
        if ( !grandsum )
                error("trying to norm_dvec 0","");
        grandsum = sqrt(grandsum);
        for ( i=dim-1; i>=0; i-- )
                vec[i] /= grandsum;
}



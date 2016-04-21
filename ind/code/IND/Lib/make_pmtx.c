/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	make_pmtx(dim1, dim2) - make space for pointer matrix, and zero
 */
char ***
make_pmtx(dim1, dim2)
int	dim1, dim2;
{
	char	***m;
	int     i,j;


	if ( m = (char ***) salloc(dim1 * sizeof(char **)) )
	  {
	    for (i = 0; i < dim1; i++)
	      if ( m[i] = (char **) salloc(dim2 * sizeof(char *)) )
		for (j = 0; j < dim2; j++)
		  m[i][j] = (char *)0;
	    return  (char ***) 0;
	  }
	else
	  return m;
}


/****************************************************************************/
/*
 *	make_pvec(dim) - make space for int vector, and zero
 */
char **
make_pvec(dim)
int	dim;
{
	char	**m;
	int     j;
	if ( m = (char **) salloc(dim * sizeof(char *)) )
	    for (j = 0; j < dim; j++)
		m[j] = (char*)0;
	return m;
}


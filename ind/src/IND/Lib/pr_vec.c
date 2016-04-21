/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	prfvec(d)	- write out a float vec
 */
pr_fvec(d,s)
float*   d;
int	s;
{
	int     i;
	for (i = 0; i < s; i++)
		fprintf(stdrep," %g", d[i]);
}

/****************************************************************************/
/*
 *	prfvec(d)	- write out a float vec
 */
pr_fvecpl(d,s)
float*   d;
int	s;
{
	int     i;
	if (s>0) fprintf(stdrep,"%g", d[0]);
	for (i = 1; i < s; i++)
		fprintf(stdrep,"+%g", d[i]);
}

/****************************************************************************/
/*
 *	prfvec(d)	- write out a float vec
 */
pr_ivec(d,s)
int*   d;
int	s;
{
	int     i;
	for (i = 0; i < s; i++)
		fprintf(stdrep," %d", d[i]);
}

/****************************************************************************/
/*
 *	prdvec(d)	- write out a double vec
 */
pr_dvec(d,s)
double*   d;
int	s;
{
	int     i;
	for (i = 0; i < s; i++)
		fprintf(stdrep," %lg", d[i]);
}


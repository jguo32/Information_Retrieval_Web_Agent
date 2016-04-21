/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	prfmtx(d)	- write out a float mtx
 */
pr_fmtx(d,s)
float**   d;
int	s;
{
	register int     i,j;
	for (i = 0; i < s; i++) {
	    for (j = 0; j < s; j++)
		fprintf(stdrep," %g", d[i][j]);
	    fputc('\n',stdrep);
	}
}

/****************************************************************************/
/*
 *	prdmtx(d)	- write out a double mtx
 */
pr_dmtx(d,s)
double**   d;
int	s;
{
	register int     i,j;
	for (i = 0; i < s; i++) {
	    for (j = 0; j < s; j++)
		fprintf(stdrep," %lg", d[i][j]);
	    fputc('\n',stdrep);
	}
}


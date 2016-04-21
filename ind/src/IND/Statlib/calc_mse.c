/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"


/****************************************************************************/
/*
 *	calc_mse(cl, d) - calculate the mean squared error of a
 *			decision vector with true class "cl"
 */
double
calc_mse(cl, d)
int     cl;	/* true class  */
float*   d;	/* predicted probability matrix  */
{
	register int     i;
	register double  e = 0;
	fordecs(i)
		e +=	(i == cl) ? (1 - d[i]) * (1 - d[i]) : d[i] * d[i];
	return e;
}




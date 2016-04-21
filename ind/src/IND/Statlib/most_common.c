/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"


/***************************************************************************/
/*
 *	most_common(set) -- return the most common decision in a vector of
 *				counts.
 */
int
most_common(dv)
float*	dv;
{
	int	i;
	int	bdec = DONTKNOW;	/* The most popular decision	*/
	float	best = 0.0;		/* Num of egs with decision bdec*/

	fordecs(i)
		if (dv[i] > best)
		{
			best = dv[i];
			bdec = i;
		}
	return bdec;
}



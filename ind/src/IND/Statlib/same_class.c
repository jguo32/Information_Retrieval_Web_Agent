/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"


/***************************************************************************/
/*
 *	same_class(cvec) -- return true if all one class
 */
bool
same_class(dv)
register float*	dv;
{
	register int	i,j=0;
	fordecs(i)
		if ( dv[i] ) j++;
	return (j>1) ? 0 : 1;
}



/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"


/****************************************************************************/
/*
 *	shuffle_set(s)	-  shuffle set randomly
 */
shuffle_set(set)
egset 	*set;
{
	int     i,index;
	egtype  *egp, temp;
	float  *wtp, wtemp;
	egp = set->members;
	wtp = set->weights;
	for (i = 0; i < setsize(set)-1; i++ ) {
		index = random()%(setsize(set)-i);
		temp = *egp;
		*egp = *(egp+index);
		*(egp+index) = temp;
		egp++;
		if ( weighted(set) ) {
			wtemp = *wtp;
			*wtp = *(wtp+index);
			*(wtp+index) = wtemp;
			wtp++;
		}
	}	
}



/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

/*
 *	set all attribute values to unknown
 */
assign_unknown( eg)
egtype	eg;
{
	register int	j;
	forattrs(j)
	   if ( num_type(j) )
		ord_val(eg,j) = FDKNOW;
	   else if ( set_type(j) )
		set_copy(set_val(eg,j), SDKNOW);
	   else
		unord_val(eg,j) = DONTKNOW;
}


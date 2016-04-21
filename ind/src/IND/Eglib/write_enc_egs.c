/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

/*
 *	write encoded example file as a single block
 *	using the system header "ld"
 */
write_enc_egs(set,str)
char    *str;
egset  *set;
{
	FILE    *egfile, *efopen();
	egtype eg;
	foregtype egp;

	if ( !str || !*str )
		egfile = stdout;
	else
		egfile = efopen(str, "w");
	ld.h_magicno = MAGICNO;
        if (fwrite((char *)&ld,sizeof(Header),1,egfile) != 1)
                uerror("Could not write to example file", "");
	foreacheg(set,eg,egp)
		write_enc_eg(eg, egfile);
	if ( str && *str ) fclose(egfile);
}

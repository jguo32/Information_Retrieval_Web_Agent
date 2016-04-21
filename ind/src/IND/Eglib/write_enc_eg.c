/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

write_enc_eg(eg, outf)
egtype    eg;
FILE	*outf;
{
   	if ((fwrite((char *) eg.unordp, egsize, 1, outf)) < 1 )
       		uerror("Could not write to encoded example file","");
}


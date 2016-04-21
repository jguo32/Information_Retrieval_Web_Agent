/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


#include <math.h>
#include <Lib.h>

/*
 *        useful for debugging with math error problems
 *	  (get debugger to trap on "matherr"
 */
matherr(excp)
struct exception *excp;
{
  if ( excp->type == DOMAIN ) {
    error("%s: domain error",excp->name);
    return 0;
  }
  else return 1;
}

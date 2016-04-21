/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "math.h"
#include "Lib.h"


float *int_log=0;		/*  keeps array of log(int) values */

static float int_log_size=0;


/****************************************************************************/
/*
 *	init_int_log()    - initialize the array
 */
void
init_int_log(size)
int size;
{
        int i;
        if ( !int_log ) {
		/*
	 	 *	don't exist, so prepare int_log upto "size"
		 */
                int_log = mems_alloc(float,size+1);
                int_log_size = size;
                for (i=1; i<=size; i++) {
                        int_log[i] = log((double)i);
		}
        } else if ( int_log_size < size ) {
		/*
	 	 *	do exist, so extend int_log upto "size"
		 */
                int_log = mems_realloc(int_log,float,size+1);
                for (i=int_log_size+1; i<=size; i++) {
                        int_log[i] = log((double)i);
		}
                int_log_size = size;
        }
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "Lib.h"

void (*tmt_error)()=0;
int    timeout = 0;
int    intint  = 0;
extern  bool oflg;

/*
 *     on interrupt, set intint
 */
void sigint_handler()
{
    intint = 1;
    oflg = 1;
    printf("Interrupt,\n");
    return;
}

/*
 *     on timeout, set "timeout" 
 */
void sigxcpu_handler()
{
    if ( tmt_error )
      tmt_error();
    timeout = 1;
    return;
}

void null_handler()
{
    return;
}


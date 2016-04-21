/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "Lib.h"

/*
 *      on exit, kill any child processes (e.g. xgraphs)
 */

/*   storage of child processes */
typedef struct  cpl {
	int  proc;
	struct cpl *next;
} cpl;
static  cpl  *cpl_base = 0;

/*   add process id to store  */
void add_childproc(cp)
int	cp;
{
	cpl *cpl_new  = (cpl *) salloc(sizeof(cpl));
	cpl_new->next = cpl_base;
	cpl_new->proc = cp;
	cpl_base = cpl_new;
}	

/*   kill children  */
void kill_childproc()
{
	/*       kill any temp. files   */
  	/*        NB>   should delete specific files, incase multiple INDs */
	if ( cpl_base )  {
		system("rm ./INDtt*");
	}
        while ( cpl_base )  {
                (void)kill(cpl_base->proc, SIGTERM);
                cpl_base = cpl_base->next;
        }
}

void myexit(ex)
int  ex;
{
	kill_childproc();
	exit(ex);
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*
 *            This is a test routine to check Dtree_ave;
 *                  
 */

extern	int	depth;
/*
 *            various  modifiers to the basic routines
 *            (values assigned are defaults from tprune.c)
 */

/******************************************************************************/
/*
 *	check_Dtree(t) -- calc. 1st & 2nd derivative of posterior w.r.t alpha
 *                       results left in "xtra.pn"
 *			 used to maximise prior parameters
 */
check_Dtree(t)
ot_ptr	t;
{
	void Dtree_ave();
	double alphas[100];
	int   i,cnt;
	double  dsprob, d2sprob;

	fprintf(stdout, "Enter alpha stop, start, count: ");
	fscanf(stdin, "%lf %lf %d", alphas, alphas+1, &cnt);
        alphas[cnt-1] = alphas[1];
        for ( i=1; i<cnt-1; i++)
           alphas[i] = (alphas[cnt-1] - alphas[0]) / cnt * i + alphas[0];
	 
        for ( i=0; i<cnt; i++) {
          set_palpha(alphas[i],t->eg_count);
	  fprintf(stdout, "Alpha value: %g: ", palphatot((float)0));
	  d2sprob = dsprob = 0.0;
	  (void)Dtree_ave(t, &d2sprob, &dsprob);
	  fprintf(stdout, "%lg, %lg, %lg \n", t->xtra.pn.sprob, dsprob, d2sprob);
	}
	while( 1 ) {
	  fprintf(stdout, "Enter alpha: ");
	  fscanf(stdin, "%lf", alphas);
	  if ( alphas[0]<= 0.0 ) break;
	  fprintf(stdout, "Alpha value: %g: ", palphatot((float)0));
	  d2sprob = dsprob = 0.0;
	  (void)Dtree_ave(t, &d2sprob, &dsprob);
	  fprintf(stdout, "%lg, %lg, %lg \n", t->xtra.pn.sprob, dsprob, d2sprob);
	}
}

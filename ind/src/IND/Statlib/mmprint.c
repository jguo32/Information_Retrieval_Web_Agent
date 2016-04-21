/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"

#define FREQ

/***************************************************************************/
/*
 *	mmprint(tbl,size) -- print frequency square table "tbl"
 *		of size "size" on standard error.
 */
mmprint(tbl,size)
double	**tbl;
int	size;
{
	int   i,j;
	static double *row, *col;
	double tot = 0;

	if (!row) {
	    row = (double *)salloc(size*sizeof(double));
	    col = (double *)salloc(size*sizeof(double));
	}

	for (i=0; i<size; i++) {
	    row[i] = 0;		/*   holds predicted   */
	    col[i] = 0;		/*   holds actual   */
	}
	for (i=0; i<size; i++) {
	    for (j=0; j<size; j++) {
		row[i] += tbl[i][j];	
		col[j] += tbl[i][j];	
	    }
	    tot += row[i];
	}
	if ( !tot ) {
	    fprintf(stdrep, "    matrix all 0s !");
	    return;
	}
#if 0
	if ( rev_strat_flag ) {
	    double	strat_tot,row_prob, prob;
	    strat_tot = 0.0;
	    for (j=0; j<size; j++) 
		strat_tot += col[j]/strat_true[j];
	    for (i=0; i<size; i++) {
		row_prob = 0.0;
	        for (j=0; j<size; j++) {
		    row_prob += prob = tbl[i][j]/strat_true[j]/strat_tot;
		    fprintf(stdrep, "  %5lf", prob);
		}
	        fprintf(stdrep, " | %5lf", row_prob);
	        fprintf(stdrep,"\n");
	    }
	    for (j=0; j<size; j++)
	        fprintf(stdrep, "----------");
	    fprintf(stdrep, "----------\n");
	    for (j=0; j<size; j++)
	        fprintf(stdrep, "  %5lf", col[j]/strat_true[j]/strat_tot);
	    fprintf(stdrep, " | 1.00000\n");
	    for (j=0; j<size; j++)
	        fprintf(stdrep, "  %5lf", true_prob[j]);
	    fprintf(stdrep, " | 1.00000 (from '.attr')\n");
	} else
#endif
	{
	    for (i=0; i<size; i++) {
	        for (j=0; j<size; j++)
#ifdef FREQ
		    fprintf(stdrep, "  %5lf", tbl[i][j]/tot);
	        fprintf(stdrep, " | %5lf", row[i]/tot);
#else
		    fprintf(stdrep, "  %5lf", tbl[i][j]);
	        fprintf(stdrep, " | %5lf", row[i]);
#endif
	        fprintf(stdrep,"\n");
	    }
	    for (j=0; j<size; j++)
	        fprintf(stdrep, "----------");
	    fprintf(stdrep, "----------\n");
	    for (j=0; j<size; j++)
#ifdef FREQ
	        fprintf(stdrep, "  %5lf", col[j]/tot);
#else
	        fprintf(stdrep, "  %5lf", col[j]);
#endif
	    fprintf(stdrep, " | 1.00000\n");
	}
}


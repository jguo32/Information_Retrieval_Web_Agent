/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*
 *      flags for processing of graph wandering, etc.
 */
int Inference_flag = 0;
int Wander_flag = 0;
int Search_for_smooth = 0;
int Store_modifications_only = 0;
int Stochastic_Growth = 0;

extern  int Multiple_trees;
extern  int verbose;

/************************************************************************
 *      process command options for "dgraph"
 */

dgraph_opts(c,optarg)
int     c;
char    *optarg;
{
        extern  char   *strchr();
        switch (c)
        {
         case 'I':
		if (! Stochastic_Growth )
			uerror("'I' flag requires stochastic growth flag","");
                Inference_flag = 1;
                if (verbose > 2)
                        printf("Inference_flag = %d\n", Inference_flag );
                break;
         case 'w':
		if (! Stochastic_Growth )
			uerror("'w' flag requires stochastic growth flag","");
                Wander_flag = 1;
                if (verbose > 2)
                        printf("Wander\n");
                break;
         case 'x':
		if (! Stochastic_Growth )
			uerror("'x' flag requires stochastic growth flag","");
                Multiple_trees = 1;
                if (verbose > 2)
                        printf("output multiple trees\n");
                break;
         case 'b':
		if (! Stochastic_Growth )
			uerror("'b' flag requires stochastic growth flag","");
                Search_for_smooth = 1;
                if (verbose > 2)
                        printf("Search_for_smooth\n");
                break;
         case 'm':
		if (! Stochastic_Growth )
			uerror("'m' flag requires stochastic growth flag","");
                Store_modifications_only = 1;
                if (verbose > 2)
                        printf("Store_modifications_only \n");
                break;
         case 'D':
                if (Stochastic_Growth) {
                        decision_graph_flag = 1;
                	if (verbose > 2)
                                printf("Build a Decision Graph\n");
                } else {
                        Stochastic_Growth = 1;
                	if (verbose > 2)
                                printf("Stochastic_Growth\n");
                }
                break;
        default:
                error("incorrect option for dgraph","");
        }
}

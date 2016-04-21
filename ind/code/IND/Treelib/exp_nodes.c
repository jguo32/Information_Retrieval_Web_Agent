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


static  double  exp_leaves;
static  double  exp_depth_cnt;

static void exp_node(t,depth)
ot_ptr  t;
int     depth;
{
	exp_leaves += t->lprob;
	exp_depth_cnt += t->lprob*depth;
}


/******************************************************************************/
/*
 *	exp_nodes(t) -- return expected node count for averaged tree
 *
 */
float
exp_nodes(t)
ot_ptr	t;
{
  exp_depth_cnt = 0.0;
  exp_leaves = 0.0;
  traverse_onodes_depth(t,exp_node);
  return exp_leaves;
}
float
exp_depth(t)
ot_ptr	t;
{
  return exp_depth_cnt;
}

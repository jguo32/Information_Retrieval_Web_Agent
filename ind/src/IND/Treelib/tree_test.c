/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   Altered to free decision graphs Jon Oliver August 1992
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

extern  int depth;

/***************************************************************************/
/*
 *	tree_test()
 *			construct "testing" record for node by
 *                      traversing parents;
 *			and initialize "depth"
 *                      only use in trees or option trees
 */
tree_test(t,testing)
ot_ptr	t;
egtesttype  testing;
{
  int	i;
  bool  found;
  bt_ptr   option;

  depth = 0;
  for (;;) {
    if (t == NULL || decision_graph_flag)
      return ;
    if ( !(option = first_parent(t)) )
	return ;
    found = FALSE;
    depth++;
    foroutcomes(i,option->test) {
      if ( option->branches[i] == t ) {
	add_test(option->test,i,testing);
	found = TRUE;
	break;
      }
    }
    ASSERT ( found)
    t = option->parent;
  }
}

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      returns true (1) if t is a dgraph
 *		i.e. has some joins in it.
 */

#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

static int is_dgraph_flag;

static void is_dgraph_node(t)
ot_ptr t;
{
	if (t->num_parents > 1)
		is_dgraph_flag = 1;
}


is_dgraph(t)
ot_ptr t;
{
	is_dgraph_flag = 0;
	traverse_onodes2(t, is_dgraph_node);
	return (is_dgraph_flag);
}


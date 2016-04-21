/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* prune -- tree pruner.
 *
 *	Author - Wray Buntine
 *
 */
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

static int prune_depth;
static void depth_prune_node(t,depth)
ot_ptr    t;
int       depth;
{
	if ( depth >= prune_depth )
	     force_leaf(t);
}

/************************************************************************/
/*
 *	depth_prune(t,d)	--  prune by depth bound
 */
depth_prune(t, d)
ot_ptr	t;
int     d;
{
	prune_depth = d;
	traverse_onodes_depth(t, depth_prune_node);
}

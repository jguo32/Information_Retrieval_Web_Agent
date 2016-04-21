/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      path routines
 *		used to locate a node.
 *
 *	Used by wandering dgraphs
 *
 *	Reason: During initial growth of a dgraph join t1 and t2.
 *	During wandering, we may decide to explore the possibility
 *	of spliting t1.
 *	But the mod->node1 does NOT point to the node
 *	containg the join of t1 and t2. To find this node
 *	we use the path stored in the mod
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

ot_ptr locate_node_according_path(t, path)
ot_ptr t;
char *path;
{
ot_ptr ot;
bt_ptr bt;
int len, i, j;
	ot = t;
	len = strlen(path);
	for (j=0; j<len; j++) {
		i = path[j]-1;
		ASSERT( i>= 0)
		bt = ot->option.o;
		ASSERT(bt)
		ot = bt->branches[i];
		ASSERT(ot)
	}
	return(ot);
}

static char *create_path(t)
ot_ptr t;
{
ot_ptr ot;
bt_ptr bt;
int i, len, j;
char *res;


	len = calc_depth_site(t);
	res = mems_alloc(char,len+1);
	ot = t;
	res[len] = 0;
	for (j=len-1; j>=0; j--) {
		bt = ot->parents[0];
		ASSERT(bt)
		i = find_outcome_number(ot, bt)+1;
		ASSERT(i != 0)
		ASSERT(i <= 127)
		res[j] = i;
                ot = bt->parent;
		ASSERT(ot)

	}
	ASSERT (locate_node_according_path(ot, res) == t)
	return (res);
}

set_path(md)
Mod *md;
{
	switch(md->type_mod) {
		case SPLIT_MOD:
			md->path1 = create_path(md->node1);
			break;
		case JOIN_MOD:
			md->path1 = create_path(md->node1);
			md->path2 = create_path(md->node2);
			break;
		default:
			ASSERT(0);
	}
}


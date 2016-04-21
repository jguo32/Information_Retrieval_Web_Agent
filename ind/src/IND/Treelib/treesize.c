/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

static int Size_count;
static void count_node(t)
ot_ptr	t;
{
	Size_count ++;
}

tree_size(t)
ot_ptr	t;
{
	Size_count = 0;
	traverse_onodes(t, count_node);
	return (Size_count);
}

static void count_leaf_node(t)
ot_ptr	t;
{
	if (tleaf(t))
		Size_count ++;
}

leaf_size(t)
ot_ptr	t;
{
	Size_count = 0;
	traverse_onodes(t, count_leaf_node);
	return (Size_count);
}

static  int Max_opts;

static  void  maxopts_node(t)
ot_ptr   t;
{
        if ( options(t) > Max_opts )
                Max_opts = options(t);
}

int tree_maxopts(t)
ot_ptr  t;
{
        Max_opts = 1;
        traverse_onodes(t,maxopts_node);
        return Max_opts;
}

static int  Depth;
static void check_depth(t,depth)
ot_ptr    t;
int       depth;
{
	if ( depth > Depth )
		Depth = depth;
}

tree_depth(t)
ot_ptr  t;
{
	Depth = 0;
	traverse_onodes_depth(t,check_depth);
	return Depth;
}

static  float  ave_depth;
static void check_ave_depth(t,depth)
ot_ptr    t;
int       depth;
{
	if ( !ttest(t) ) {
		Size_count ++;
		ave_depth += depth;
	}
}

float tree_ave_depth(t)
ot_ptr  t;
{
	Size_count=0;
	ave_depth = 0;
	traverse_onodes_depth(t,check_ave_depth);
	if (  Size_count )
	    return ave_depth/Size_count;
	else 
	    return 0;
}


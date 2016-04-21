/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/*
 *	install "re_count" vector at "->xtra" in each node
 */
static void recnt_node(t)
ot_ptr	t;
{
	t->xtra.re_count = make_fvec(ndecs);
}

/*
 *	update "re_count" vector at "->xtra" for node
 */
static recnt_updt(t, eg, prop)
ot_ptr	t;
egtype	eg;
float	prop;
{
	t->xtra.re_count[class_val(eg)] += prop;
}

/*
 *       install count vector at (nodes)->xtra.re_count
 *	 and place counts for the examples in egs
 *	 as they would fall down the tree
 */
recnt_tree(t,egs)
ot_ptr	t;
egset   *egs;
{
	egtype   eg;
	foregtype  egp;
	traverse_onodes(t, recnt_node);
	foreacheg(egs, eg, egp)
		tree_reclass(t, eg, recnt_updt, 1);
}

/*
 *       install count vector at (nodes)->xtra.re_count
 *	 and place counts for the examples in egs
 *	 as they would fall down the tree
 *     +  read egs from file
 */
recnt_tree_f(t,eg_file)
ot_ptr	t;
char    *eg_file;
{
	egtype   eg;
	traverse_onodes(t, recnt_node);
        while ( (eg = read_eg(eg_file)).unordp )
                tree_reclass(t, eg, recnt_updt, 1);
}

/*
 *      fetch the count so stored as above
 */
float
recnt_val(t, ind)
ot_ptr	t;
int	ind;
{
	return t->xtra.re_count[ind];
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to
 *              (1) divide a graph into a tree section and a dgraph section
 *              (2) smooth a graph (instead of pruning)
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/***************************************************************
 *
 *	set_tree_section routines
 */

static void set_tree_section_node(t)
ot_ptr t;
{
ot_ptr ot;
bt_ptr option;
int i, j;
	t->tflags.b.tree_section = 1;
	ASSERT (decision_graph_flag)
	if ( ttest(t) ) {
	    foroptions(option, j, t) {
		foroutcomes(i, option->test) {
			ot = option->branches[i];
			if ( (ot->tflags.b.tree_section == 0) ||
				(ot->num_parents > 1) ) {
	 			t->tflags.b.tree_section = 0;
				return;
			}
		}
	    }
	}
}

set_tree_section(t)
ot_ptr t;
{
	traverse_onodes_post(t, set_tree_section_node);
}

/***************************************************************
 *
 *	graph_avet routines
 */

static float   Prcut;
static int     Maxo;
static bool    Noprun;

static void graph_avet_node( t)
ot_ptr  t;
{
bt_ptr  bt;
ot_ptr  ot;
	if (ttree_section(t)) {
		ASSERT (t->num_parents >= 1)
		if (t->num_parents == 1) {
			bt = t->parents[0];
			ASSERT (bt)
			ot = bt->parent;
			if ( ttree_section(ot) )
				return;
		}
		/*
		 *  WARNING :  this should really be setup right
		 */
		t->testing = init_test();
		tree_avet( t, Prcut, Maxo, Noprun, t->testing);
		sfree(t->testing.unordp);
		t->testing.unordp = 0;
	} else {
		t->lprob = 0.0;
	}
}

graph_avet( t, prcut, maxo, noprun)
ot_ptr  t;
float   prcut;    /*   remove any option has branch proportion < this */
                  /*   if >=1, remove all but best option */
int     maxo;     /*   any near-leaf node >= this many options is made leaf  */
bool     noprun;   /*   true if don't wish to average pruned trees as well
                                i.e.    for logical version    */
{
	Prcut = prcut;
	Maxo = maxo;
	Noprun = noprun;
	set_tree_section(t);
	traverse_onodes(t, graph_avet_node);
}

/***************************************************************
 *
 *	graph_smooth routines
 */

static bool    Decprune;

static void graph_smth_node( t)
ot_ptr  t;
{
bt_ptr  bt;
ot_ptr  ot;
	if (ttree_section(t)) {
		ASSERT (t->num_parents >= 1)
		if (t->num_parents == 1) {
			bt = t->parents[0];
			ASSERT (bt)
			ot = bt->parent;
			if ( ttree_section(ot) )
				return;
		}
		tree_smooth( t, Decprune);
	} else {
		t->lprob = 0.0;
	}
}

graph_smooth( t, decprune)
ot_ptr  t;
bool     decprune ;   /*   true to decision prune too */
{
	Decprune = decprune;
	set_tree_section(t);
	traverse_onodes(t, graph_smth_node);
}

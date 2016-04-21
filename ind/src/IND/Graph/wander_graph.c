/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to wander through graph space
 *
 *	Intro to wandering through graph space
 *	--------------------------------------
 *		wandering through dgraph space is different from wandering
 *		through tree space because any change is nolonger a local one
 *
 *		we number of the modifications as we grew the dgraph
 *			stored in the execution_order of each mod
 *		E.g.,
 *			split ROOT (3 way split)		1
 *				into nodes A, B and C
 *			split node A (2 way)	 		2
 *				into nodes D and E
 *			join nodes B and C			3
 *			. . .
 *
 *		if we wish to consider an alternate growth
 *		Eg split B (execution_order == 3) instead of joining it
 *		then we regrow	the graph up to the execution_order
 *		1 less than this (execution_order == 2)
 *
 *		- This is done in regrow_tree
 *
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

extern int verbose, depth_bound;
extern float Store_thresh_hold;
extern int  complex();
extern bool  oflg;

/************************************************************************
 *
 *      locate_active_execution
 *		locate the active mod for node n
 */

static Mod *locate_active_execution(lmods, n)
Mod *lmods;
int n;
{
Mod *md;
        md = (Mod *)0;
        while(lmods) {
                if ((lmods->execution_order == n) && (lmods->active_flag)) {
                        ASSERT (! md)
                        md = lmods;
                }
                lmods = lmods->next;
        }
        ASSERT (md)
        return(md);
}

/************************************************************************
 *
 *      replicate_mod
 *		locate the node for mod md and perform md
 */

bt_ptr make_btree();

replicate_mod(md, Root)
Mod *md;
ot_ptr Root;
{
ot_ptr node1, node2, jtree;
bt_ptr option;

#ifdef DEBUG_WANDER_GRAPH
printf("restore : ");
print_mod(md);
#endif

        switch (md->type_mod) {
                case COMPLEX_SPLIT_MOD:
			node1 = locate_node_according_path(Root, md->path1);
			ASSERT (! md->path2)
			ASSERT (node1 != md->node1)
			option = make_btree(md->option);
                        perform_split(node1, option, SPLIT_MOD, 0);
                        break;
                case COMPLEX_JOIN_MOD:
			node1 = locate_node_according_path(Root, md->path1);
			ASSERT (node1 != md->node1)
			node2 = locate_node_according_path(Root, md->path2);
			ASSERT (node2 != md->node2)
                        jtree = perform_join(node1, node2);
                        break;
                default:
                        ASSERT(0)
			break;
        }
}

/************************************************************************
 *
 *      regrow_tree
 *              see intro at the top of this file
 */

regrow_tree(t, lmods, md)
ot_ptr t;
Mod *lmods, *md;
{
int j;
Mod *m;
	for (j=1; j<md->execution_order; j++) {
		m = locate_active_execution(lmods, j);
		replicate_mod(m, t);
	}
	replicate_mod(md, t);
}

/************************************************************************
 *
 *      find_mods_node
 *		find all the mods (joins and splits) that could be done to t
 */

static Mod *find_lmods;
static ot_ptr find_root;

static void set_lock_node(t)
ot_ptr t;
{
	if (!ttest(t))
		tlock(t) = 1;
}

static void find_mods_node(t)
ot_ptr t;
{
	if (!ttest(t)) {
		find_lmods = make_join_mods(t, find_lmods, find_root);
		find_lmods = make_split_mods2(t, find_lmods, 1);
		tlock(t) = 0;
	}
}

/************************************************************************
 *
 *      wander_through_graph_space
 *
 *      Algorithm
 *      perform the following loop until timeout
 *      or ????????
 *              1) copy the tree
 *              2) call choose_mod t selects a complex mod (md)
 *              3) call regrow_tree to grow the dgraph up the point when
 *			md was due to be done
 *		4) call grow_probabilistically to grow the dgraph
 *			to full height
 *		5) if (ml_after < ml_before)
 *                      install new tree and continue wandering
 *                 else
 *                      re-install old tree and continue wandering
 */

static Mod *save_lmods_upto_execution_order();

ot_ptr wander_through_graph_space(copy_root, lmods, best_ml, best_t)
ot_ptr copy_root, best_t;
Mod *lmods;
float best_ml;
{
float ml;
int npossible_mods;
Mod *md, *active_md;
int cycle = 1;
ot_ptr ot;
static revise_node_pointers();

        ASSERT( ! lmods->prev )
again:

        cycle ++;
        if (verbose) {
                printf("CYCLE %d\n", cycle);
        }

        npossible_mods = set_choose_mod_flag(lmods, complex);
        if ( npossible_mods == 0 || !depth_bound ) {
                return( best_t );
        }

        ot = copy_otree(copy_root);
	ASSERT ( ot->xtra.gr.egs == 0 )
        ot->xtra.gr.egs = copy_set(best_t->xtra.gr.egs);
	ASSERT ( ot->testing.unordp == 0 )
        ot->testing = init_test();

        set_diff_savings();
        md = choose_mod(lmods);

#ifdef DEBUG_WANDER_GRAPH
printf("******** Trying ***********\n");
print_mod(md);
printf("***************************\n");
#endif

	regrow_tree(ot, lmods, md);

	find_lmods = (Mod *)0;
	find_root = ot;
	traverse_onodes(ot, set_lock_node);
	traverse_onodes3(ot, find_mods_node);
        find_lmods = grow_probabilistically(ot, find_lmods,
		md->execution_order);

	ml = message_length(ot, 1, 0);
	if ( oflg ) {
		interact(ot,ot->testing,7);
	}
	if (ml < Store_thresh_hold)
		store_good_tree(ot, ml);
	if (ml < best_ml ) {
                if ( verbose )
		    printf("CYCLE %d MESSAGE LENGTH = %8.3f\n", cycle, ml);

		lmods = save_lmods_upto_execution_order(lmods,
			md->execution_order);

if (verbose > 2) {
printf("****** find_lmods *********\n");
print_list_mods(find_lmods);
printf("***************************\n");
printf("****** lmods *********\n");
print_list_mods(lmods);
printf("***************************\n");
}
if (verbose > 1) {
printf("******** Trying ***********\n");
print_mod(md);
}

		revise_node_pointers(lmods, ot);
		lmods = concat(find_lmods, lmods);
		active_md = locate_active_execution(lmods, md->execution_order);

if (verbose > 2) {
printf("******** Active ***********\n");
print_mod(active_md);
}

		active_md->active_flag = 0;
		md->active_flag = 1;

if (verbose > 2) {
printf("****** after *********\n");
print_list_mods(lmods);
printf("***************************\n");
}
		best_ml = ml;
		release_tree_egs(best_t);
		best_t = ot;
	} else {
		release_tree_egs(ot);
		free_tree(ot);
	}
	goto again;
}

static Mod *save_lmods_upto_execution_order(lmods, execution_order)
Mod *lmods;
int execution_order;
{
Mod *tmp;
	tmp = lmods;
	while (tmp) {
		if (tmp->execution_order > execution_order)
			lmods = delete_mod(tmp, lmods);	
		tmp = tmp->next;
	}
	return(lmods);
}

static revise_node_pointers(lmods, root)
Mod *lmods;
ot_ptr root;
{
Mod *tmp;
ot_ptr node1, node2;
        tmp = lmods;
        while (tmp) {
		node1 = locate_node_according_path(root, tmp->path1);
		ASSERT (node1 != tmp->node1)
		tmp->node1 = node1;
		if (tmp->node2) {
			node2 = locate_node_according_path(root, tmp->path2);
			ASSERT (node2 != tmp->node2)
			tmp->node2 = node2;
		}
                tmp = tmp->next;
        }
}

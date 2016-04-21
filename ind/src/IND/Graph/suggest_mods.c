/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/*
#define DEBUG_JOINS
#define DEBUG_GAIN
#define DEBUG_LOOKAHEAD
*/

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

static Mod *Best_Mod;		/* set by *make_split_mods2();	*/
static Mod *ljoin_mods;

extern bt_ptr	*treeoptions;
extern int	*bestoptions;
extern int	depth;
extern int	depth_bound;
extern int	malloc_error;
extern int	min_set_size;       /* stop growing if set at node < */
extern int	Inference_flag;
extern int	Perform_Joins;

extern int	look_depth;
extern int	look_breadth;

extern int	max_opts;
extern float	THRESH_HOLD;

static float lookahead_ml();
egtesttype  merge_tests();

/***************************************************************************/
/*
 *      create_jnode() -- alloc memory for a new join node
 *                      return 0 if no memory
 */
static ot_ptr create_jnode(t1, t2)
ot_ptr  t1, t2;
{
ot_ptr  t;
int num_parents, j, count;

        t = (ot_ptr) salloc(sizeof(ot_rec));
        if ( !t ) return (ot_ptr)0;
        null_flags(t->tflags);
        t->tot_count = 0;
        t->eg_count = (float*)0;
        t->option.o = (bt_ptr)0;
        t->option.s.c = 0;

/* Parents */	
	num_parents = t1->num_parents + t2->num_parents;
	ASSERT( num_parents > 1)
        if (  !(t->parents = mems_alloc(bt_ptr, num_parents)) )
		return (ot_ptr)0;
        t->num_parents = num_parents;
        t->parent_c = num_parents;
	count = 0;
	for (j=0; j<t1->num_parents; j++) {
        	t->parents[count] = t1->parents[j];
		count ++;
	}
	for (j=0; j<t2->num_parents; j++) {
        	t->parents[count] = t2->parents[j];
		count ++;
	}
	ASSERT (count == num_parents)

        t->xtra.gr.gain = -FLOATMAX;
        t->xtra.gr.egs = (egset *)0;
        ASSERT( ! null_testing(t1))
        ASSERT( ! null_testing(t2))

	t->testing = merge_tests(t1->testing, t2->testing);
        return t;
}

/************************************************************************
 *
 *      join_two_onodes --- given two nodes (t1 and t2) it
 *		1) calls create_jnode to create the joined node
 *		2) calls merge_egs2 to create the merged list of egs
 *		3) calls add_counts to create the count distn at this node
 */


egset *merge_egs2();

ot_ptr join_two_onodes(t1, t2)
ot_ptr t1, t2;
{
ot_ptr jtree;
egset *eg;

	jtree = create_jnode(t1, t2);
	if (!jtree) {
		malloc_error = 1;
		printf("out of mem file %s line %d\n", __FILE__, __LINE__);
		return (ot_ptr)0;
	}

	eg = merge_egs2(t1->xtra.gr.egs, t2->xtra.gr.egs);
	if (! eg) {
		malloc_error = 1;
		printf("out of mem file %s line %d\n", __FILE__, __LINE__);
		return (ot_ptr)0;
	}
	ASSERT(jtree->xtra.gr.egs == 0)
	jtree->xtra.gr.egs = eg;
	ASSERT (jtree->xtra.gr.egs)

	add_counts(jtree);
	return (jtree);
}

/************************************************************************
 *
 *      find_best_gain:
 *              finds the best mod in lmods associated with node t
 */

static float find_best_gain(lmods, t)
Mod *lmods;
ot_ptr t;
{
Mod *md;

        md = lmods;
        while(md) {
                if ((md->type_mod == SPLIT_MOD) && (md->node1 == t) &&
                        (md->Diff_ML_saving == 0.0))
                        return (md->Abs_ML_saving);
                md = md->next;
        }
        return(0.0);
}

static binary_brothers(t1, t2)
ot_ptr t1, t2;
{
	if (t1->parents[0] != t2->parents[0])
		return(0);
	if (t1->num_parents > 1)
		return(0);
	if (t2->num_parents > 1)
		return(0);
	return (outcomes(t1->parents[0]->test) ==  2);
}

/************************************************************************
 *
 *      look_for_joins --- determines if there is a join between
 *		2 nodes (t current_site) that is worth exploring
 *
 *	the tlock flag avoids us looking at joins twice
 *	E.g., when we do a 3-way split we have 3 new leaves (l1 l2 l3)
 *	Without the tlock flag we would explore:
 *		look_for_joins(l1, l2)
 *		look_for_joins(l1, l3)
 *		look_for_joins(l2, l1)
 *		look_for_joins(l2, l3)
 *		look_for_joins(l3, l1)
 *		look_for_joins(l3, l2)
 *	but with it we explore
 *		look_for_joins(l1, l2)
 *		look_for_joins(l1, l3)
 *		look_for_joins(l2, l3)
 *
 *	this rountines algorithm
 *		1) discard joins we should consider
 *			- using tlock
 *			- must be a leaf
 *			- dont join nodes with only 1 class
 *			- no point join binary brothers
 *		2) call join_two_onodes to form the jnode
 *		3) call make_split_mods2 to check if there are
 *			any splits worth doing to jnode
 *			make_split_mods2 can use lookahead if the flags are set
 *		4) if any worthwhile splits
 *			4a) calculate the gain from the join
 *			4b) put the join mod into  ljoin_mods
 *		5) clean up (free the tree, etc)
 */

static ot_ptr current_site;
extern float Worst_jnode_cost;

static void look_for_joins(t)
ot_ptr t;
{
ot_ptr jtree;
float gain, loss, bg1, bg2;
Mod *md;

	if (tlock(t)) {
		/* No Point exploring this Join */
		return;
	}
	ASSERT( t != current_site)
	if (ttest(t)) {
		/* COMPLEX JOINS not implemented YET */
		return;
	}
	if (same_class(t->eg_count)) {
		return;
	}
	if (binary_brothers(t, current_site)) {
		/* No Point exploring this Join */
		return;
	}
#ifdef DEBUG_JOINS
	printf("\nlook_for_join: %7.2f egs with %7.2f egs\n",
		t->tot_count, current_site->tot_count);
#endif /* DEBUG_JOINS */

	jtree = join_two_onodes(current_site, t);
	md = (Mod *)0;
        ASSERT( ! null_testing(jtree))
	md = make_split_mods2(jtree, md, 0);

	if (md) {
		loss = leaf_length(jtree) -
			( leaf_length(t) + leaf_length(current_site) );
		gain = Best_Mod->Abs_ML_saving + 2.0 * join_weight()
			- loss - 2.0 * Worst_jnode_cost;

#ifdef DEBUG_JOINS
		printf("ll(t)		     = %8.2f\n", leaf_length(t));
		printf("ll(current_site)     = %8.2f\n",
			leaf_length(current_site));
		printf("ll(jtree)	     = %8.2f\n",
			leaf_length(jtree));
		printf("Best_Mod->Abs_ML_saving = %8.2f\n",
			 Best_Mod->Abs_ML_saving);
		printf("loss = %8.2f\n", loss);
		printf("Worst_jnode_cost = %8.2f\n", Worst_jnode_cost);
		printf("consider JOIN %7.2f egs with %7.2f egs gain = %7.2f\n",
			t->tot_count, current_site->tot_count, gain);
#endif /* DEBUG_JOINS */

		if (gain > 0.0) {
			bg1 = find_best_gain(ljoin_mods, current_site);
			bg2 = find_best_gain(ljoin_mods, t);

#ifdef DEBUG_JOINS
			printf("gain  = %8.2f\n", gain);
			printf("bg1   = %8.2f\n", bg1);
			printf("bg2   = %8.2f\n", bg2);
#endif /* DEBUG_JOINS */

		    if (gain - bg1 - bg2 > 0.0) {

#ifdef DEBUG_JOINS
			printf("and then perform\n");
			print_mod(Best_Mod);
#endif /* DEBUG_JOINS */

			ljoin_mods = make_mod(ljoin_mods, JOIN_MOD, (bt_rec *)0,
				t, current_site, 1);
			ljoin_mods->Abs_ML_saving = gain;
			ljoin_mods->Diff_ML_saving = gain - bg1 - bg2;
		    }
		}
	}

	delete_lmods(md);
	release_tree_egs(jtree);
	free_tree(jtree);
}

/************************************************************************
 *
 *      make_join_mods
 *	traverse the tree rooted at root, and call look_for_joins
 *	for each node
 */

Mod *make_join_mods(node, lmods, root)
ot_ptr node, root;
Mod *lmods;
{
	if (same_class(node->eg_count)) {
		return(lmods);
	}
	current_site = node;
	ljoin_mods = lmods;
	traverse_onodes(root, look_for_joins);
	return(ljoin_mods);
}

/************************************************************************
 *
 *      make_split_mods2
 *      given node find any worthwhile splits
 *      	- use lookahead if look_depth != 1
 *
 *	Algorithm
 *		1) dont split if
 *			- (depth > depth_bound)
 *			- (node->tot_count < min_set_size)
 *			- same_class(node->eg_count)
 *		2) call choose
 *		3) for loop through bestoptions turning them into mods
 *			and putting them in new_mods
 *		4) loop through new_mods calculating the gain
 *      		using lookahead if look_depth != 1
 *			prune out those mods if (gain < THRESH_HOLD)
 *		5) concatenate new_mods onto lmods
 *		6) return lmods
 */

bt_ptr make_btree();
Mod *concat();

Mod *make_split_mods2(node, lmods, pathflag)
ot_ptr node;
Mod *lmods;
int pathflag;
{
int choice, j, best;
float gain, leaf_ML, split_ML, best_gain;
bt_ptr option;
Mod *new_mods;
Mod *tmp, *ntmp;
egtesttype  testing;

	Best_Mod = (Mod *)0;
	depth = calc_depth_site(node);
	if ( (depth > depth_bound) ||  (node->tot_count < min_set_size) ||
			same_class(node->eg_count) ) {
		return( lmods );
	}

#ifdef DEBUG_SEARCH
printf("jono: entered make_split_mods\n");
printf("tot_count = %5.1f\n", node->tot_count);
#endif

        ASSERT( ! null_testing(node))
	testing = node->testing;
	choice = choose(node, max_opts, -10.0, testing);
	if (choice == 0) {
		return( lmods );
	}
	/********************************************************/
	/*	Save away  bestoptions from choose.c		*/
	/********************************************************/
	new_mods = (Mod *)0;
	j = 0;
	best=bestoptions[j];
	while (best != 0) {
		option = make_btree(treeoptions[best]);
		if (option == (bt_ptr)0) {
			malloc_error = 1;
			printf("out of mem file %s line %d\n",
				__FILE__, __LINE__);
		} else {
			option->parent = node;
			new_mods= make_mod(new_mods, SPLIT_MOD, option, node,
				(ot_rec *)0, pathflag);
			/* make mods might set malloc_error */
			if (malloc_error) {
				printf("out of mem file %s line %d\n",
					__FILE__, __LINE__);
			}
		}
		j ++;
		best=bestoptions[j];
	}


	/********************************************************/
	/*	Grow each treeoption to lookahead_height	*/
	/********************************************************/

	best_gain = - FLOATMAX;
	leaf_ML = -(node->xtra.gr.gain);
		/********************************************************/
		/* This assumes that the gain returned by choose	*/
		/*	is a message_length				*/
		/********************************************************/
	tmp = new_mods;
	while (tmp) {
		ntmp = tmp->next;
		if (look_depth == 1) {
			split_ML = -(tmp->option->gain);
		} else {
			split_ML = lookahead_ml(tmp);
#ifdef DEBUG_LOOKAHEAD
printf("LOOKAHEAD normal gain = %8.2f\n", leaf_ML + tmp->option->gain);
printf("LOOKAHEAD la     gain = %8.2f\n", leaf_ML - split_ML);
#endif
		}
		gain = leaf_ML - split_ML;

#ifdef DEBUG_GAIN
printf("leaf_ML (%5.0f)	= %8.2f\n", node->tot_count, leaf_ML);
printf("split_ML	= %8.2f\n", split_ML);
printf("nw		= %8.2f\n",  node_weight(node->testing, tmp->option));
printf("lw2		= %8.2f\n",  leaf_weight(node->tot_count, node));
printf("gain    	= %8.2f\n", gain);
printf("overgrown    	= %8d\n", tovergrown(node));
#endif

		if ((gain < THRESH_HOLD) &&
		    (Perform_Joins || tovergrown(node) || Inference_flag)) {
			new_mods = delete_mod(tmp, new_mods);
		} else {
			tmp->Abs_ML_saving = gain;
			if (gain > best_gain) {
				Best_Mod = tmp;
				best_gain = gain;
			}
		}
		tmp = ntmp;
	}

	lmods = concat(new_mods, lmods);
	tmp = lmods;
	while (tmp && (tmp->node1 == node)) {
		ASSERT( Best_Mod )
		tmp->Diff_ML_saving = tmp->Abs_ML_saving - best_gain;
		tmp = tmp->next;
	}
	return( lmods );
}

/************************************************************************
 *
 *      lookahead_ml
 *	loop through each child calling extendtree and lookaheadtree
 *
 */

extern int	max_dep;
bt_ptr copy_bstump();

static float lookahead_ml(md)
Mod *md;
{
int     i;
bt_ptr	option;
float ml;
ot_ptr t;
ot_ptr t2;
extern  bool free_egxtra;
bool  save_free_egxtra;
	max_dep = depth + look_depth - 2;
	depth = calc_depth_site(md->node1);

#ifdef DEBUG_LOOKAHEAD
	printf("max_dep = %d\n", max_dep);
	printf("look_depth = %d\n", look_depth);
	printf("depth = %d\n", depth);
#endif

        ASSERT( ! null_testing(md->node1))
	option = make_btree(md->option);
	ASSERT(md->type_mod == SPLIT_MOD)
	t = md->node1;
	perform_split(t, option, SPLIT_MOD, 0);

	foroutcomes(i, option->test) {
		t2 = option->branches[i];
		if ( depth <= max_dep ) {
			t2->lprob = - leaf_length(t2);
			if ( extendtree(t2,t2->testing) ) {
				malloc_error = 1;
				printf("out of mem file %s line %d\n",
					__FILE__, __LINE__);
				return FLOATMAX;
			}
			if ( lookaheadtree(t2,t2->testing) ) {
				malloc_error = 1;
				printf("out of mem file %s line %d\n",
					__FILE__, __LINE__);
				return FLOATMAX;
			}
		}
	}

#ifdef DEBUG_LOOKAHEAD
	printf("lookahead tree\n");
	printf("==============\n");
	ml = message_length(t, 0, 1);
#endif
	ml = message_length(t, 0, 0);
	jono_rem_option(t);
        save_free_egxtra = free_egxtra;
        /*
         *      WARNING: should really free the egs., but this gives memory  errors  
         *
        free_egxtra = TRUE;  */
        free_egxtra = save_free_egxtra;  
	free_btree(option);
	return (ml);
}

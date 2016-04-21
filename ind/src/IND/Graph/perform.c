/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

extern int verbose;
extern int Wander_flag;
extern float Store_thresh_hold;
int Perform_Joins;
static int execution_order;

Mod *choose_mod();
egtesttype  copy_tests();

/********************************************************
 *
 *	The routines make_complex_dueto_mod and del_complex_mods
 *	are doing basically the same job.
 *
 *		make_complex_dueto_mod
 *	When we are wandering through tree/dgraph space, we
 *	wish to save mods since we may wish to explore the
 *	possibility of doing these mods later
 *	This routine turns a
 *		SPLIT_MOD	=>	COMPLEX_SPLIT_MOD
 *		JOIN_MOD	=>	COMPLEX_JOIN_MOD
 *
 *		del_complex_mods
 *	When doing normal growth (no wandering) we just want
 *	to delete those mods that are no longer applicable
 */

static make_complex_dueto_mod(lmods, t)
Mod *lmods;
ot_ptr t;
{
Mod *md;
	md = lmods;
	while(md) {
		if ((md->type_mod == SPLIT_MOD) && (md->node1 == t)) {
			md->type_mod = COMPLEX_SPLIT_MOD;
			md->execution_order = execution_order;
		} else if ((md->type_mod == JOIN_MOD) &&
			((md->node1 == t) || (md->node2 == t)) ) {
				md->type_mod = COMPLEX_JOIN_MOD;
				md->execution_order = execution_order;
		}
		md = md->next;
	}
}

static Mod *del_complex_mods(lmods, t)
Mod *lmods;
ot_ptr t;
{
Mod *md, *mdnext;
	md = lmods;
	while(md) {
		mdnext = md->next;
		if (! md->active_flag) {
			if ((md->type_mod == SPLIT_MOD) && (md->node1 == t)) {
				lmods = delete_mod(md, lmods);
			} else if ((md->type_mod == JOIN_MOD) &&
				((md->node1 == t) || (md->node2 == t)) ) {
				lmods = delete_mod(md, lmods);
			}
		}
		md = mdnext;
	}
	return(lmods);
}

/********************************************************
 *	set_choose_mod_flag() routine
 *
 *	the set_choose_mod_flag routine sets the choose_mod_flag
 *	which means that a mod is suitable to be performed
 *
 *	the active_flag means that a mod has actually been performed
 *	already (and hence should not be performed again)
 *
 */

primitive(md)
Mod *md;
{
	return (PRIMITIVE_MOD(md) && (! md->active_flag));
}

complex(md)
Mod *md;
{
	return ((md->type_mod == COMPLEX_SPLIT_MOD) && (! md->active_flag));
}

set_choose_mod_flag(lmods, f)
Mod *lmods;
int (*f)();
{
int j;
	ASSERT( ! lmods->prev )
	j = 0;
	while(lmods) {
		if ( (*f)(lmods)) {
			lmods->choose_mod_flag = 1;
			j ++;
		} else {
			lmods->choose_mod_flag = 0;
		}
		lmods = lmods->next;
	}
	return(j);
}

/********************************************************/
/*	_makegraph(t) routines				*/
/********************************************************/

extern bool t_flg;

/********************************************************
 *      grow_probabilistically() routine
 *
 *	the grow_probabilistically() routine takes a node (root)
 *	and a list of mods and grows that tree (or dgraph) to
 *	completion.
 *
 *	ASSUMES that the lmods has some mods in it
 *		if given lmods == NULL will do nothing
 */

Mod *grow_probabilistically(root, lmods, start_execution_order)
ot_ptr root;
Mod *lmods;
int start_execution_order;
{
int npossible_mods;
Mod *md;
float ml2;

	if (!lmods) {
		return(lmods);
	}
	ASSERT( ! lmods->prev )
	execution_order = start_execution_order;
	npossible_mods = set_choose_mod_flag(lmods, primitive);

	while (npossible_mods > 0) {
		/* we now randomly choose a modification */
		execution_order++;
		set_abs_savings();
		md = choose_mod(lmods);
		md->execution_order = execution_order;
		perform_mod(md);

		if (verbose > 1) {
			printf("\t****** Selected ******\n");
			print_mod(md);
		}
		if (verbose > 3) {
			printf("DEPTH AFTER  = %d\n", tree_depth(root) );
		}

/* SANITY CHECKS */
		check_dgraph_pointers(root, root);
		ml2 = message_length(root, 1, 0);

		if (verbose > 3) {
			ml2 = message_length(root, 1, 1);
		}

		if (verbose > 2) {
			printf("\tmessage length = %8.2f\n", ml2);
		}

		ASSERT( ! lmods->prev )
		lmods = make_new_mods2(md, lmods, root);
		ASSERT( ! lmods->prev )
		if (Wander_flag) {
			if (md->type_mod == SPLIT_MOD) {
				make_complex_dueto_mod(lmods, md->node1);
			} else {
				ASSERT (md->type_mod == JOIN_MOD)
				make_complex_dueto_mod(lmods, md->node1);
				make_complex_dueto_mod(lmods, md->node2);
			}
		} else {
                        if (md->type_mod == SPLIT_MOD) {
                                lmods = del_complex_mods(lmods, md->node1);
                        } else {
                                ASSERT (md->type_mod == JOIN_MOD)
                                lmods = del_complex_mods(lmods, md->node1);
                                lmods = del_complex_mods(lmods, md->node2);
                        }
		}
		ASSERT( ! lmods->prev )
		npossible_mods = set_choose_mod_flag(lmods, primitive);
		if (verbose > 2) {
			print_list_mods(lmods);
		}
	}
	return(lmods);
}

/********************************************************
 *	continue_growing routines
 *
 *	when a dgraph is initally grown we dont allow hurdling operations
 *	(mods that increase the message length)
 *	but when we finish growing we want to smooth the
 *	graph, hence we continue_growing it for smoothing purposes.
 *
 */

static Mod *lm;

static void grow_leaf_node(t)
ot_ptr t;
{
	if (! ttest(t))
		lm = make_split_mods2(t, lm, 1);
}

Mod *continue_growing(t, lmods)
ot_ptr t;
Mod *lmods;
{
	lm = lmods;
	traverse_onodes(t, grow_leaf_node);
	return (lm);
}

/********************************************************
 *      stochastic_grow() routine
 *
 *      the stochastic_grow() routine takes a node (root)
 *      and a set of egs
 *
 *	dGraphs:
 *	makes the appropiate calls to makegraph
 *		(1) multiple calls to makegraph if we are searching for Pjoin
 *		(2) only 2 calls if the user set Pjoin
 *			1st call --- make graph with Pjoin
 *			2nd call --- make a tree (incase a tree is better)
 *
 *	dGraphs:
 *		one call to makegraph (with joins disabled)
 */

int max_opts;
extern int Max_Good_Tree;
extern int      look_depth;
extern int      look_breadth;
extern prior_flags     prior_flag;
double lookup_p_join();
extern bool free_egxtra;

stochastic_grow(t, egs)
ot_ptr t;
egset   *egs;
{
double p_join;
	if ( ! t_flg ) {
		uerror("_makegraph routines only usable with t_flg on\n", "");
	}
	if (Wander_flag) {
		max_opts = 3;
		Max_Good_Tree = MAX_GOOD_TREE;
	} else {
		Max_Good_Tree = 1;
		max_opts = 1;
	}
	if (look_depth > 1)
		max_opts = look_breadth;
	add_counts(t);
	free_egxtra = TRUE;
	if (decision_graph_flag && !Wander_flag) {
		if ( flag_set(prior_flag, search_p_join) ) {
			if (verbose)
				printf("Searching p_join\n");
			construct_dgraph(t, egs, 0.05);
			construct_dgraph(t, egs, 0.10);
			construct_dgraph(t, egs, 0.20);
			construct_dgraph(t, egs, 0.30);
			construct_dgraph(t, egs, 0.40);
			construct_dgraph(t, egs, 0.50);
		} else {
			p_join = lookup_p_join();
			construct_dgraph(t, egs, p_join);
		}
		decision_graph_flag = 0;
                set_wall_prior(0.0);
		makegraph(t);
		if (verbose)
			printf("TREE  cost = %8.2f\n", t->bestprob);
		decision_graph_flag = 1;
	} else {
		makegraph(t);
	}
}

construct_dgraph(t, egs, p_join)
ot_ptr t;
egset   *egs;
float p_join;
{
ot_ptr t2;
        set_wall_prior(p_join);
	t2 = copy_otree(t);
	ASSERT(t2->xtra.gr.egs == 0)
	t2->xtra.gr.egs = copy_set(egs);
	add_counts(t2);
	makegraph(t2);
	if (verbose)
		printf("With p_join = %8.2f GRAPH cost = %8.2f\n",
			p_join, t2->bestprob);
	free_tree(t2);
}

float THRESH_HOLD;
egtesttype init_test();
extern int Inference_flag;
extern float Worst_jnode_cost;

static makegraph(t)
ot_ptr t;
/********************************************************/
/*	equivalent of _maketree(t) in makeopttree.c	*/
/********************************************************/
{
float ml;
Mod *List_mods, *list2;
ot_ptr copy_root;

	if (Wander_flag) {
		if (decision_graph_flag) {
			THRESH_HOLD = 0.0;
			Perform_Joins = 1;
			copy_root = copy_otree(t);
		} else {
			THRESH_HOLD = -2.0;
			Perform_Joins = 0;
		}
	} else {
		if (decision_graph_flag) {
			THRESH_HOLD = 0.0;
			Perform_Joins = 1;
		} else {
			THRESH_HOLD = -0.5;
			Perform_Joins = 0;
		}
	}
	if (verbose)
		printf("Entered makegraph\n");
	ml = message_length(t, 1, 0);
	if (verbose)
		printf("Raw data message_length = %8.3f\n", ml);

	t->testing = init_test();
	Worst_jnode_cost = 0.0;

	Store_thresh_hold = ml;
	List_mods = (Mod *)0;
	List_mods = make_split_mods2(t, List_mods, 1);
	list2 = grow_probabilistically(t, List_mods, 0);
	ml = message_length(t, 1, 0);
	store_good_tree(t, ml);
	if (Wander_flag && list2) {
		if (verbose)
			printf("About to wander message_length = %8.3f\n", ml);
		if (decision_graph_flag) {
			t = wander_through_graph_space(copy_root, list2, ml, t);
			list2 = (Mod *)0;
		} else {
			list2 = wander_through_tree_space(t, list2);
#ifdef UNUSED
			if (Multiple_trees) {
				printf("SECOND WANDER\n");
				/* Now find trees of high posterior */
				list2 = wander_through_tree_space(t, list2);
			}
#endif
		}
		ml = message_length(t, 1, 0);
	}
	delete_lmods(list2);
	if (decision_graph_flag && !Inference_flag) {
		if (verbose) {
			printf("Overgrowing Graph (for smoothing)\n");
		}
		Perform_Joins = 0;
		THRESH_HOLD = -2.0;
		List_mods = (Mod *)0;
		List_mods = continue_growing(t, List_mods);
		list2 = grow_probabilistically(t, List_mods, 100);
		ml = message_length(t, 1, 0);
		store_good_tree(t, ml);
		delete_lmods(list2);
	}
	if (verbose > 2)
		printf("finished makegraph message_length = %8.3f\n", ml);
}

/********************************************************/
/*	jono_add_option routines 			*/
/* 		equiv of add_option in tree.c		*/
/* 		and rem_option in tree.c		*/
/********************************************************/

jono_add_option(t, bt)
ot_ptr t;
bt_ptr bt;
{
int j, overgrown;
	overgrown = tovergrown(t);
        null_flags(t->tflags);
        set_flag(t->tflags,test);
	if (overgrown)
	        set_flag(t->tflags, overgrown);

	t->option.o = bt;
	t->option.o->parent = t;
	foroutcomes(j, t->option.o->test) {
	      ASSERT(t->option.o->branches[j]->xtra.gr.egs==(egset*)0)
	      t->option.o->branches[j]->xtra.gr.egs=(egset*)0;
	}
}

jono_rem_option(t)
ot_ptr t;
{
int overgrown;
	overgrown = tovergrown(t);
        null_flags(t->tflags);
	if (overgrown)
	        set_flag(t->tflags, overgrown);
        t->option.o = (bt_ptr)0;
}

/************************************************************************
 *
 *      should be a macro
 */

null_testing(t)
ot_ptr t;
{
	return ( t->testing.unordp == (unordtype *)0 );
}

/************************************************************************
 *
 *      actually perform a split
 */

perform_split(t, new_option, type_mod, overgrown)
ot_ptr t;
bt_ptr new_option;
int type_mod, overgrown;
{
int i, j;
bt_ptr option;
egtesttype  testing;
	if (type_mod == SPLIT_MOD) {
		if (ttest(t)) {
			printf("jono: split node on attr %d\n",
				new_option->test->attr);
			printf("type_mod = %d\n",
				type_mod);
			ASSERT ( !ttest(t))
		}
	} else {
		ASSERT (type_mod == COMPLEX_SPLIT_MOD)
		ASSERT (  ttest(t))
	}
	testing = t->testing;
	ASSERT( ! null_testing(t))
	jono_add_option(t, new_option);
	foroptions (option,j,t) {
		add_egs(option);
		foroutcomes(i, option->test) {
			add_test(option->test,i,testing);
			ASSERT ( option->branches[i]->testing.unordp == 0 )
			option->branches[i]->testing = copy_tests(testing);
			add_counts(option->branches[i]);
                        rem_test(option->test,i,testing);
			option->branches[i]->tflags.b.overgrown = overgrown;
		}
	}
}

/************************************************************************
 *
 *      fix_parent_pointers routine
 *
 *		called by perform_join
 */

static fix_parent_pointers(jtree, t1, t2)
ot_ptr jtree, t1, t2;
{
int j, i;
bt_ptr bt;
	for (j=0; j<jtree->num_parents; j++) {
		bt = jtree->parents[j];
		i = find_outcome_number(t1, bt);
		if (i == -1) {
			i = find_outcome_number(t2, bt);
			ASSERT( i != -1)
		}
		bt->branches[i] = jtree;
	}

	release_tree_egs( t1 );
	free_tree( t1 );
	release_tree_egs( t2 );
	free_tree( t2 );
}

/************************************************************************
 *
 *      perform_join routine
 *		actually perform a join
 */

ot_ptr perform_join(t1, t2)
ot_ptr t1, t2;
{
ot_ptr jtree;
        jtree = join_two_onodes(t1, t2);
	fix_parent_pointers(jtree, t1, t2);
	return (jtree);
}

/************************************************************************
 *
 *      perform_mod routine
 */

bt_ptr make_btree();

perform_mod(md)
Mod *md;
{
bt_ptr option;
int overgrown;
	switch (md->type_mod) {
		case SPLIT_MOD: case COMPLEX_SPLIT_MOD:
			option = make_btree(md->option);
			if (md->Abs_ML_saving < 0.0)
				overgrown = 1;
			else
				overgrown = 0;
			perform_split(md->node1, option,
				(int)md->type_mod, overgrown);
			break;
		case JOIN_MOD:
			ASSERT( Perform_Joins )
			md->jtree = perform_join(md->node1, md->node2);
			break;
		default:
			ASSERT(0)
			break;
	}
	md ->active_flag = 1;
}

/************************************************************************
 *
 *      make_new_mods2 routine
 *
 *	make_new_mods2 takes a mod (md), and the root of the tree
 *	for each leaf made by mod (md) we attempt to split it
 *	or join it with other leaves
 */

Mod *make_new_mods2(md, lmods, root)
Mod *md, *lmods;
ot_ptr root;
{
int i, j;
bt_ptr option;
ot_ptr t;
	switch (md->type_mod) {
	case SPLIT_MOD: case COMPLEX_SPLIT_MOD:
		t = md->node1;
		foroptions(option,j,t) { 
/* look for splits */
			foroutcomes(i, option->test) {
				ASSERT( ! null_testing(option->branches[i]) )
				lmods = make_split_mods2(option->branches[i],
					lmods, 1);
			}
/* look for joins */
			if (Perform_Joins) {
				foroutcomes(i, option->test) {
					tlock(option->branches[i]) = 1;
				}
				foroutcomes(i, option->test) {
					lmods = make_join_mods(option->
						branches[i], lmods, root);
					tlock(option->branches[i]) = 0;
				}
			}
		}
		break;
	case JOIN_MOD:
		t = md->jtree;
		ASSERT( t )
		ASSERT( ! null_testing(t) )
/* look for splits */
		lmods = make_split_mods2(t, lmods, 1);
/* look for joins */
		t->tflags.b.lock = 1;
		lmods = make_join_mods(t, lmods, root);
		t->tflags.b.lock = 0;
		break;
	default:
		ASSERT(0)
		break;
	}
	return(lmods);
}

/********************************************************/
/*	inefficient --- should be stored in the tree	*/
/********************************************************/

find_outcome_number(ot, bt)
ot_ptr ot;
bt_ptr bt;
{
int i;
	foroutcomes(i, bt->test) {
		if (bt->branches[i] == ot) {
			return(i);
		}
	}
	return(-1);
}

/****************************************/
/*	testing primitives		*/
/****************************************/

print_tests(testing)
egtesttype  testing;
{
int  i;
	printf("print_testrs\n");
	fordattrs(i) {
		if (unord_val_s(testing,i) == DONTKNOW)
			printf("attr %2d - ?\n", i);
		else
			printf("attr %2d - %d\n", i, unord_val_s(testing,i));
	}
}

egtesttype  copy_tests(testing)
egtesttype  testing;
{
int  i;
egtesttype newtesting = init_test();
	for ( i=1; i<=ndattrs; i++)
                unord_val_s(newtesting,i)=unord_val_s(testing,i);
	return newtesting;
}

/********************************************************/
/*		calc_depth_site routines			*/
/********************************************************/

calc_depth_site(node)
ot_ptr node;
{
int d;
ot_ptr ot;
bt_ptr bt;
	ot = node;
	d = 0;
	while (ot != (ot_rec *)0) {
		ASSERT( ot->num_parents >= 1)
		bt = ot->parents[0];
		if (bt != (bt_rec *)0) {
			d ++;
			ot = bt->parent;
		} else {
			ot = (ot_rec *)0;
		}
	}
	return(d);
}

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to wander through tree space
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

static Mod *local_lmods;
static bt_ptr old_option;
static ot_ptr local_tree_site;
static Mod *traverse_prune();

extern int verbose, depth_bound;
extern bool  oflg;

extern  int  complex();

/************************************************************************
 *
 *      preform_complex_split
 *		A complex split is one where a node has already been
 *		split, and we wish to change the attribute that
 *		we split on
 *
 *	Algorithm
 *		1) call choose_mod to select a complex split
 *		2) call perform_mod to do it
 *		3) call make_new_mods2 to suggest modifications for
 *			the children of this new split
 *		4) call grow_probabilistically to grow this new
 *			mod to full height
 */

static Mod *perform_complex_split(lmods, root)
Mod *lmods;
ot_ptr root;
{
int npossible_mods;
Mod *md;

	ASSERT( ! lmods->prev )
	npossible_mods = set_choose_mod_flag(lmods, complex);
	if ( npossible_mods == 0 ) {
		return( (Mod *) 0);
	}
	set_diff_savings();
	md = choose_mod(lmods);

	if (verbose > 1) {
		printf("\t****** Complex Split ******\n");
		print_mod(md);
	}
	ASSERT (complex(md))
	local_tree_site = md->node1;

	old_option = local_tree_site->option.o;
	perform_mod(md);

	local_lmods = (Mod *)0;
	local_lmods = make_new_mods2(md, local_lmods, root);
	local_lmods = grow_probabilistically(local_tree_site, local_lmods, 0);
	return(md);
}

/************************************************************************
 *
 *	unactivate routines
 *		A mods active_flag is set when the tree
 *		currently been grown uses this mod
 *
 *		when we perform a complex split, we must turn off
 *		the old mod at the site where we install the new mod
 */

static Mod *locate_active_md(t, lmods, excludemd)
ot_ptr t;
Mod *lmods, *excludemd;
{
Mod *md;
	md = (Mod *)0;
	while(lmods) {
		if ((lmods->node1 == t) && (lmods->active_flag) &&
				(lmods != excludemd)) {
			ASSERT (! md)
			md = lmods;
		}
		lmods = lmods->next;
	}
	ASSERT (md)
	return(md);
}

static unactivate(t, lmods, newmd)
ot_ptr t;
Mod *lmods, *newmd;
{
Mod *md;
	md = locate_active_md(t, lmods, newmd);
	md->active_flag = 0;
	md->type_mod = COMPLEX_SPLIT_MOD;
}

/************************************************************************
 *
 *      wander_through_tree_space
 *
 *      Algorithm
 *	perform the following loop until timeout or ?????
 *              1) call perform_complex_split
 *			selects a complex mod and
 *			calls grow_probabilistically to grow it to full height
 *              2) ml_after = message_length(root)
 *              3) if (ml_after < ml_before)
 *			install new tree and continue wandering
 *                 else
 *			re-install old tree and continue wandering
 */


float Store_thresh_hold;
extern int Multiple_trees;
extern int Store_modifications_only;

Mod *wander_through_tree_space(root, lmods)
ot_ptr root;
Mod *lmods;
{
float ml_before, ml_after;
Mod *md, *active_md;
int cycle=1, depth;
	ml_before = message_length(root, 1, 0);
	if (verbose) {
		print_list_mods(lmods);
	}

again:
	cycle ++;
	if (verbose) {
		printf("CYCLE %d\n", cycle);
	}
	if ( oflg ) {
		interact(root, root->testing,7);
		/*    sets depth_bound=0 if want to exit   */
	}	
	if ( depth_bound ) 
		md = perform_complex_split(lmods, root);
	if (md && depth_bound) {
		ml_after = message_length(root, 1, 0);
		if (verbose > 1) {
			printf("****** After Attempting ******\n");
			print_mod(md);
			printf("MESSAGE LENGTH BEFORE = %8.3f\t", ml_before);
			printf("AFTER  = %8.3f\t", ml_after );
			printf("DEPTH AFTER  = %d\n", tree_depth(root) );
		}
		/*
		 *   NB.   this is point to add Markov chain like search
		 */
		if (ml_after < ml_before) {
			lmods = traverse_prune(old_option, lmods);
			unactivate(local_tree_site, lmods, md);
/* cleanup_option(old_option); */
			free_btree(old_option);
			lmods = concat(lmods, local_lmods);
			ml_before = ml_after;
			if (verbose) {
				print_list_mods(lmods);
				printf("MOD SUCCEEDED\n");
			}
			depth = calc_depth_site(local_tree_site);
			if (verbose) {
			printf("CYCLE %d MESSAGE LENGTH = %8.3f Depth = %d\n",
				cycle, ml_before, depth);
			}
			store_good_tree(root, ml_before);
			goto again;
		}
/* FAILED --- RESTORE OLD TREE */
		if (Multiple_trees && !Store_modifications_only &&
			(ml_after<Store_thresh_hold) && (ml_after>ml_before)) {
		/* If the tree is ok we still might want to store it */
			
			store_good_tree(root, ml_after);
		}

		active_md = locate_active_md(local_tree_site, lmods, md);
		unactivate(local_tree_site, lmods, active_md);
/* cleanup_option(local_tree_site->option.o); */
		free_btree(local_tree_site->option.o);

		local_tree_site->option.o = old_option;
		if (verbose > 1) {
			printf("MOD FAILED\n");
		}
		delete_lmods(local_lmods);
		goto again;
	}
	return( lmods );
}

cleanup_option(bt)
bt_ptr bt;
{
int j;
extern  bool free_egxtra;
bool  save_free_egxtra;
        save_free_egxtra = free_egxtra;
	/*
	 *	WARNING:  this should be TRUE but then get memory errors
 	 */
        free_egxtra = FALSE;
	foroutcomes(j, bt->test) {
		free_tree(bt->branches[j]);
		bt->branches[j] = new_node(bt);
	}
        free_egxtra = save_free_egxtra;
}

/****************************************/
/*	traverse_prune routines		*/
/****************************************/

static ot_ptr Current_Prune_Node;

static Mod *prune_list_mods(lmods)
Mod *lmods;
{
Mod *md, *nmd;
	md = lmods;
/*
printf("pruning node with %5.0f egs\n", Current_Prune_Node->tot_count);
*/
	while(md) {
		nmd = md->next;
		if (md->node1 == Current_Prune_Node) {
			lmods = delete_mod(md, lmods);
		}
		md = nmd;
	}
	return( lmods );
}

static Mod *Rec_traverse_prune(t, lmods)
ot_ptr t;
Mod *lmods;
{
bt_ptr option;
int i, j;
	Current_Prune_Node = t;
	lmods = prune_list_mods(lmods);
	if ( ttest(t) ) {
		foroptions(option, j, t) {
			foroutcomes(i, option->test) {
				lmods = Rec_traverse_prune(option->branches[i],
					lmods);
			}
		}
	}
	return(lmods);
}

static Mod *traverse_prune(option, lmods)
bt_ptr option;
Mod *lmods;
{
int i;
	foroutcomes(i, option->test) {
		lmods = Rec_traverse_prune(option->branches[i], lmods);
	}
	return(lmods);
}


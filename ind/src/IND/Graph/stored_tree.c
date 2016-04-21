/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to
 *              store the best MAX_GOOD_TREE trees
 *		has an array of trees		-- good_tree
 *		and their message lengths	-- good_tree_ml
 */

#include <stdio.h>
#include <math.h>
#include "mod.h"

/****************************************/
/*	store_good_tree() routines	*/
/****************************************/

static ot_ptr	good_tree[MAX_GOOD_TREE];
static float	good_tree_ml[MAX_GOOD_TREE];
static int	ngood=0;
int Max_Good_Tree;

extern float Store_thresh_hold;
extern int Multiple_trees;
extern int verbose;

#define MAX_FLAG	1
#define MIN_FLAG	-1

free_stored_trees()
{
int j;

	for (j=0; j<ngood; j++) {
		free_tree(good_tree[j]);
	}
}

egtesttype  copy_tests();

/************************************************************************
 *
 *      store_good_tree
 *              find the position (pos) in the "good_tree" array
 *		where tree t should go
 *		put it there
 */

store_good_tree(t, ml)
ot_ptr t;
float ml;
{
int pos, j;
ot_ptr ctree;
	ASSERT( Max_Good_Tree <= MAX_GOOD_TREE)
	ASSERT( ngood <= Max_Good_Tree )

	if (ngood < Max_Good_Tree) {
		pos = ngood;
	} else {
		pos = find_pos(MAX_FLAG);
		if (ml > good_tree_ml[pos])
			return;
	}
	ctree = copy_otree(t);
	ctree->testing = copy_tests(t->testing);
	ASSERT (message_length(ctree, 1, 0) == ml)
	force_leaves(ctree);
	ASSERT (message_length(ctree, 1, 0) == ml)

	for (j=0; j<ngood; j++) {
		if (ml == good_tree_ml[j]) {
			/* printf("jono: reject tree %8.3f\n", ml); */
			pos = j;
			free_tree(good_tree[pos]);
			goto Replace;
		}
	}
	if (ngood < Max_Good_Tree) {
		ngood ++;
	} else {
		ASSERT (ml < good_tree_ml[pos])
		free_tree(good_tree[pos]);
	}
Replace:
	good_tree   [pos] = ctree;
	good_tree_ml[pos] = ml;
	if (ngood == Max_Good_Tree) {
		pos = find_pos(MAX_FLAG);
		Store_thresh_hold = good_tree_ml[pos];
	}
}

/************************************************************************
 *
 *      find_pos
 *		finds the position of the best (or worst) tree
 *		best (smallest message length) when	sign == MIN_FLAG
 *		worst when				sign == MAX_FLAG
 */

static find_pos(sign)
int sign;
{
int pos, j;
float max, cost;
	pos = 0;
	max = (float)sign * good_tree_ml[pos];

	for (j=1; j<ngood; j++) {
		cost = (float)sign * good_tree_ml[j];
		if (cost > max) {
			pos = j;
			max = cost;
		}
	}
	return(pos);
}

#define MAX_BUF	200
static char buf[MAX_BUF];
extern int      Stochastic_Growth;

/************************************************************************
 *
 *      write_stored_trees
 *		if Multiple_trees write the good_tree array to file
 *		otherwise write the best tree
 *		
 */

write_stored_trees(treename, tree, head)
char *treename;
ot_ptr tree;
t_head *head;
{
int j;
	ASSERT( ngood <= Max_Good_Tree )
	if (verbose>1)
		printf("entered write_trees %s ngood = %d\n", treename, ngood);
	head->genprob = 1.0;
	if (Multiple_trees) {
		for (j=0; j<ngood; j++) {
			head->sprob = good_tree_ml[j];
			sprintf(buf, "%s.x%d", treename, j);
			if (verbose) {
				printf("writing %s: message_length = %8.3f\n",
					buf, head->sprob);
			}
			write_tree(buf, good_tree[j], head);
		}
		if (ngood < Max_Good_Tree) {
			int pos;
			if (verbose) {
				printf("warning not enough good trees\n");
			}
			pos = find_pos(MIN_FLAG);
			head->sprob = good_tree_ml[pos];
			for (j=ngood; j<Max_Good_Tree ; j++) {
				sprintf(buf, "%s.x%d", treename, j);
				if (verbose) {
				    printf("writing %s messagelength = %8.3f\n",
					buf, head->sprob);
				}
				write_tree(buf, good_tree[pos], head);
			}
		}
	} else if (Stochastic_Growth ) {
		j = find_pos(MIN_FLAG);
		head->sprob = good_tree_ml[j];
		if (verbose) {
			printf("writing %s messagelength= %8.3f alpha= %8.2f\n",
				treename, head->sprob, head->prior.alphaval);
		}
		write_tree(treename, good_tree[j], head);
	} else {
		write_tree(treename, tree, head);
	}
}

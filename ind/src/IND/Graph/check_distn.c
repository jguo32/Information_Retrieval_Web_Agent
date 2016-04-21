/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */
/*  written  Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to
 *		(1) help debugging
 *		(2) print out the class distn at a node and/or tree
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/************************************************************************
 *
 *	check_dist routines
 */

#define MAX_CL	100
static Check_distn[MAX_CL];

static void check_distn_node(t)
ot_ptr t;
{
int j;
	if (! ttest(t)) {
		ASSERT( t->eg_count )
		for (j=0; j<ndecs; j++) {
			Check_distn[j] += t->eg_count[j];
		}
	}
}

check_distn(t)
ot_ptr t;
{
int j;
	ASSERT( t->eg_count )
	if (ndecs >= MAX_CL)
		return;
	for (j=0; j<ndecs; j++)
		Check_distn[j] = 0;
        traverse_onodes2(t, check_distn_node);
	for (j=0; j<ndecs; j++) {
		if (Check_distn[j] != t->eg_count[j]) {
			printf("error in check_distn\nAt leaves : ");
			for (j=0; j<ndecs; j++) {
				printf("%d+", Check_distn[j]);
			}
			printf("\nAt root : ");
			for (j=0; j<ndecs; j++) {
				printf("%5.1f+", t->eg_count[j]);
			}
			printf("\n");
			print_distn(t);
			exit(0);
		}
	}
}

/************************************************************************
 *
 *	print_dist routines
 */

void print_distn_node(t)
ot_ptr t;
{
int j;
	if (! ttest(t)) {
		printf("leaf ");
	} else {
		printf("node ");
	}
	printf("%6.1f egs : ", t->tot_count);
	for (j=0; j<ndecs; j++) {
		printf("%5.1f+", t->eg_count[j]);
	}
	printf("\n");
}

print_distn(t)
ot_ptr t;
{
int j;
	ASSERT( t->eg_count )
	if (ndecs >= MAX_CL)
		return;
	for (j=0; j<ndecs; j++)
		Check_distn[j] = 0;
        traverse_onodes(t, print_distn_node);
}

/************************************************************************
 *
 *	 check_dgraph_pointers routines
 */

static ot_ptr Root;

static void check_dgraph_pointers_node(t)
ot_ptr t;
{
int j, i;
bt_ptr bt;
	if (t != Root) {
		for (j=0; j<t->num_parents; j++) {
			bt = t->parents[j];
			i = find_outcome_number(t, bt);
			ASSERT( i != -1)
			ASSERT (bt->branches[i] == t)
		}
	}
}

check_dgraph_pointers(t, r)
ot_ptr t, r;
{
	Root = r;
	traverse_onodes(t, check_dgraph_pointers_node);
}


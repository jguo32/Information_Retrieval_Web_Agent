/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 *
 *
 *   written 1992 Jonathan Oliver
 */

/************************************************************************
 *
 *      routines to
 *              calculate the message length of sending a dgraph
 *		from sender to receiver
 *
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/*
#define DEBUG_TRANSMISSION_COST
*/

extern int verbose;

static double calc_B(m, n)
int m, n;
{
double ntom, prod, sum, res;
int i;
	if (n == 1)
		return(1.0);
	ntom = pow((double)n, (double)m);

	sum = 0.0;
	prod = (double) n;
	for (i=1; i<=n-1; i++) {
		sum += calc_B(m, i) * prod;
		prod = prod * (double)(n-i);
	}
	res = (ntom - sum)/prod;
	return (res);
}

static int M, N, Line_no, Unsent_jnodes, Jnodes;
float Worst_jnode_cost;

static Rec_transmit_graph(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
#ifdef DEBUG_TRANSMISSION_COST
printf("trans node %7.1f egs num_parents = %d M=%d N=%d\n",
t->tot_count, t->num_parents, M, N);
#endif
	if ( t->num_parents > 1 ) {
		if (t->line_no == 0) {
			M ++;
			N ++;
			Line_no ++;
			t->line_no = Line_no;
#ifdef DEBUG_TRANSMISSION_COST
			printf("alloc line_no = %d\n", Line_no);
#endif
			t->parent_c = 1;
			return;
		} else if (t->parent_c < t->num_parents) {
			M ++;
			t->parent_c ++;
			if (t->parent_c == t->num_parents) {
				Unsent_jnodes ++;
				save_jline_no_node(t);
			}
			return;
		} else {
			Jnodes += t->num_parents;
			ASSERT (t->parent_c == t->num_parents)
			/* We are starting a new transmission at a join node */
		}
	}
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				Rec_transmit_graph(option->branches[j]);
			}
		}
	}
}

double transmission_cost_graph(t)
ot_ptr t;
{
int k, x;
ot_ptr ot;
double tot, c, lc, lm;

	Jnodes = Unsent_jnodes = Line_no = M = N = 0;
	zero_line_no_tree(t);
	alloc_jline_no_tree();
	Rec_transmit_graph(t);
	tot = 0.0;

	while ( Unsent_jnodes > 0 ) {
		ASSERT( M > 1)
		c =  calc_B(M,N);
		lc = log(c);
		lm = log( (double)(M-1));
		tot += lc + lm;

#ifdef DEBUG_TRANSMISSION_COST
		printf("calc_B(%3d,%3d)  = %7.2f\n", M, N, c);
		printf("log( ^ )         = %7.2f\n", lc);
		printf("log(%3d)         = %7.2f\n", M-1, lm);
#endif

		x=jline_stored();
		for (k=0; k<x; k++) {
			ot = find_jline_no(k);
			if(ot) {
#ifdef DEBUG_TRANSMISSION_COST
				printf("found = %d\n", k);
#endif
				Unsent_jnodes --;
				unsave_jline_no_node(ot);
				ASSERT(ot->line_no == k)
				N --;
				M -= ot->num_parents;
				ASSERT( N >= 0 );
				ASSERT( M >= 0 );
				Rec_transmit_graph(ot);
			}
		}
	}


	if (tot > 0.0) {
		float w;
		w = tot/(float)Jnodes;
		if (w > Worst_jnode_cost)
			Worst_jnode_cost = w;
		if (verbose > 2) {
			printf("transmission_cost_graph: tot = %7.2f\n", tot);
			printf("Jnodes = %d Worst_jnode_cost = %7.2f\n",
				Jnodes, Worst_jnode_cost);
		}
	}
	return(tot);
}

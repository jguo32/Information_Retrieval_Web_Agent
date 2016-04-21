/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

/************************************************************************
 *
 *      routines to
 *              (1) help debugging
 *              (2) print out the class distn at a node and/or tree
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/************************************************************************
 *
 *      external flags
 */

extern int Search_for_smooth;
extern int verbose;

/************************************************************************
 *
 *      local flags
 */

static int pflag;		/* print flag for message_length()	*/
static int Prune_Root_Flag;	/* set if we can "prune" the root of	*/
				/* the tree we are calculating the	*/
				/* message_length of. 			*/
static ot_ptr Root;		/* the root of the tree 		*/

/************************************************************************
 *
 *      smooth_over_messages routines
 *		option "-b" in dgraph_opts.c
 *		When this option is set we calculate the message length
 *		of a node by adding the probabilities of leaf and split:
 *			prob(node) = prob(leaf) + prob(split)
 *		rather than taking the minimum
 */

static double fmin(x1, x2)
double x1, x2;
{
	if (x1 < x2)
		return (x1);
	return (x2);
}

static double smooth_over_messages(leaf_cost, node_cost)
double leaf_cost, node_cost;
{
double p1, p2, p3, ml;
	if (fabs( leaf_cost - node_cost) > 4.0 )
		return( fmin ( leaf_cost, node_cost) );
	p1 = exp(-leaf_cost);
	p2 = exp(-node_cost);
	p3 = p1+p2;
	ml = - log(p3);
	return (ml);
}

/************************************************************************
 *
 *      leaf_length routine
 *		uses the lprobset flag.
 *		lprobset is set when this node(t) has a value for lprob
 */

float leaf_length(t)
ot_ptr t;
{
void    print_distn_node();
	if (! tlprobset(t)) {
		t->tflags.b.lprobset = 1;
		t->lprob = leaf_prob(t,t->testing);
		if (verbose > 3) {
			print_distn_node(t);
			printf("cost = %8.3f nits\n", t->lprob);
		}
	}
	ASSERT (t->lprob <= 0.0)
	return ( - t->lprob);
}

/************************************************************************
 *
 *      message_length_node routine
 *              calcs the message length of the node t
 *	ASSUMES that this procedure has been called for the children of t
 *		i.e. a postfix traversal
 *	stores the result in t->bestprob
 */

static void message_length_node(t, testing)
ot_ptr t;
egtesttype  testing;
{
float x, ml;
bt_ptr option;
int i, j;
ot_ptr ot;

	t->lprob = - leaf_length(t);

/* Are we in a tree or in a graph? */
	t->tflags.b.tree_section = 1;
	if (decision_graph_flag) {
        	if ( ttest(t) ) {
                	foroptions(option, j, t) {
                        	foroutcomes(i, option->test) {
                                	if ( ! option->branches[i]->
						tflags.b.tree_section)
	        				t->tflags.b.tree_section = 0;
                        	}
                	}
		}
		if (t->num_parents > 1) {
	        	t->tflags.b.tree_section = 0;
		}
	}

        if ( ttest(t) ) {
		t->bestprob = FLOATMAX;
		foroptions(option, j, t) {
			x = - node_weight(testing, option);
                        if (cut_test(option->test)) {
				if (option->nprob > 0.0) {
					printf("nprob = %8.2f\n",option->nprob);
					ASSERT (option->nprob <= 0.0)
				}
				x -= option->nprob;
                        }
                        foroutcomes(i, option->test) {
				ot = option->branches[i];
				if (ot->num_parents > 1)
					ml = ot->bestprob / (float)ot->num_parents;
				else
					ml = ot->bestprob;
				ASSERT( ml >= 0.0)
				x += ml;
                        }
			if (x < t->bestprob)
				t->bestprob = x;
		}
	        if (t->tflags.b.tree_section) {
                	if (Search_for_smooth) {
                        	t->bestprob = smooth_over_messages(- t->lprob,
					t->bestprob);
                	} else if (Prune_Root_Flag || (Root != t)) {
				float lprob;
				lprob = - t->lprob;
                        	if (lprob < t->bestprob)
                                	t->bestprob = lprob;
			}
                }
                if (pflag) {
                        printf("(%5.0f) node_cost       = %6.2f\n",
                                t->tot_count, t->bestprob);
                }
	} else {
		t->bestprob = - t->lprob;
                if (pflag) {
                        printf("(%5.0f) leaf_cost       = %6.2f\n",
                                t->tot_count, t->bestprob);
                }
	}
	if (t->num_parents > 1) {
		t->bestprob -= (float)t->num_parents * join_weight();
                if (pflag) {
	                printf("(%5.0f) join nodes      = %6.2f\n",
				t->tot_count, -(float)t->num_parents * join_weight());
		}
	}
}

double transmission_cost_graph();
extern int      look_depth;

/************************************************************************
 *
 *      message_length routine
 *              calcs the message length of the tree rooted at t
 *      performs a postfix traversal of message_length_node
 *
 *	The prune_root_flag (2nd param) tells message_length to
 *		prune down to t been just a leaf if this is the
 *		smallest message_length.
 *		When prune_root_flag == 0 message_length
 *		forces the root to be a split used by lookahead_ml()
 *
 *	The pflag (3rd param) tells message_length to print
 *		out an explanation of how it arrived at the message_length
 */

float message_length(t, prune_root_flag, print_flag)
ot_ptr t;
int prune_root_flag;
int print_flag;
{
	pflag = print_flag;
	Prune_Root_Flag = prune_root_flag;
	Root = t;
	if (pflag)
		printf("++++++ Calc Message Length ++++++\n");
	traverse_onodes_test(t, message_length_node);
	if (decision_graph_flag && is_dgraph(t))
		t->bestprob += transmission_cost_graph(t);
	if (pflag)
		printf("			__________\n");
	if ((verbose > 3) || pflag) {
		printf("	message_length    %6.2f\n", t->bestprob);
	}
	if (pflag)
		printf("++++ End Calc Message Length ++++\n");

	return(t->bestprob);
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   Altered to write decision graphs Jon Oliver August 1992
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

extern int verbose;
static  _write_tree();

write_tree(treename, t, head)
ot_ptr	t;
char	*treename;
t_head	*head;
{
	FILE	*treefile, *efopen();
	head->magicno = MAGICNO;
	head->treesize = tree_size(t);
	head->leafsize = leaf_size(t);
#ifdef GRAPH
        thgraph(head) = is_dgraph(t);
        if (verbose > 1)
                printf("is_dgraph = %d\n", thgraph(head));
#endif

	treefile = efopen(treename, "w");
	fwrite((char *) head, sizeof(t_head), 1, treefile);

        if (decision_graph_flag) {
		alloc_jline_no_tree();
		unset_transit_flag(t);
		/* locks the transit flag */
	}
	_write_tree(t, treefile);
        if (decision_graph_flag) {
		unlock_transit_flag();
	}

	fclose(treefile);
}

static _write_tree(t, treefile)
ot_ptr	t;
FILE	*treefile;
{
	int	i,j;
	bt_ptr	option;
	tree_flags	ot_tflags;
	static  test_rec  *tsp;

	fwrite((char *)&(t->tflags), sizeof(tree_flags) ,1, treefile);

        if (t->num_parents > 1) {
                if (previous_transit(t)) {
                        fwrite((char *)&(t->line_no), sizeof(int) ,1, treefile);
                        return;
                }
                assign_jline_no_node(t);
        } else {
                t->line_no = 0;
        }
	fwrite((char *)&(t->line_no), sizeof(int) ,1, treefile);
	fwrite((char *)&(t->num_parents), sizeof(int), 1,  treefile);
	fwrite((char *)&(t->tot_count), sizeof(float), 1,  treefile);
	fwrite((char *)&(t->lprob), sizeof(float), 1, treefile);
	fwrite((char *)t->eg_count, sizeof(float), ndecs, treefile);
	if ( ttest(t) ) {
	    if ( toptions(t) ) 
	        fwrite(&(t->option.s.c), sizeof(short), 1, treefile);    
	    foroptions(option,j,t) {
                fwrite(&(option->tflags), sizeof(tree_flags),1, treefile);
                fwrite(&(option->nprob), sizeof(float),1, treefile);
                fwrite(&(option->np.nprop), sizeof(float),1, treefile);
                fwrite(&(option->gain), sizeof(float),1, treefile);
		tsp = option->test;
                fwrite((char *)tsp, sizeof(test_rec),1, treefile);
                if ( bigsubset_test(tsp) )
                        put_bitarray(tsp->tval.valbigset, treefile);
	        foroutcomes(i,tsp) {
		    if (option->branches[i])
			_write_tree(option->branches[i], treefile);
		    else {
		        null_flags(ot_tflags);
		        set_flag(ot_tflags,emptt);
		        fwrite((char *)&ot_tflags, sizeof(tree_flags) ,1, treefile);
		    }
	        }	
	    }
	}
}


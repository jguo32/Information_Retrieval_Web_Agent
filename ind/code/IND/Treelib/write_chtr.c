/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/***********************************************************************/
/*
 *	write tree (in "char" format) to file with name "treename"
 */
static	int	ct_depth;

write_chtr(treename, tree, head)
char    *treename;
ot_ptr	tree;
t_head	*head;
{
        FILE    *fp, *efopen();
	static void    _write_chead(), _write_ctree();

        if (decision_graph_flag) 
               error("character format not implemented for dgraphs", "");
        fp = efopen(treename, "w");
	ct_depth = -1;
	head->treesize = tree_size(tree);
	head->leafsize = leaf_size(tree);
        _write_chead(fp,head);
        _write_ctree(fp,tree);
        fclose(fp);
}

static void
_write_chead(fp,head)
FILE    *fp;
t_head	*head;
{
	fprintf(fp, "SIZE: %d %d %o %lg %lg %lg\n",
		head->treesize, head->leafsize, int_flags(head->hflags),
				head->sprob, head->eprob, head->genprob);
	fprintf(fp, "PRIOR: %g %g %g %g %o %d %lg\n",
		head->prior.alphaval, 
		head->prior.node_weight,
		head->prior.leaf_weight, head->prior.p_join,
		int_flags(head->prior.prior_flag), 
		head->prior.depth_bound ,
		head->prior.prior_nml );
}

static void
_write_ctree(fp,t)
register FILE    *fp;
register ot_ptr	t;
{
        register int     i,j;
	bt_ptr	option;
	register test_rec	*tsp;

	for (i=0; i<ct_depth; i++)
		fputs("  ",fp);
	ASSERT( t ) 
	ct_depth++;
	if ( ferror(fp) )
		uerror("can't write chr-tree","");
        fprintf(fp, "%o", int_flags(t->tflags) );
        fprintf(fp, ",  %g",  t->eg_count[0] );
        for(i=1; i<ndecs; i++)
                fprintf(fp, "+%g", t->eg_count[i] );
	fprintf(fp," = %g (%g,%g)", t->tot_count, t->lprob, t->bestprob );
        if ( ttest(t) ) {
            if ( toptions(t) )
                fprintf(fp, " %d", &t->option.s.c );
	    foroptions(option,j,t) {
        	fprintf(fp, " %o", int_flags(option->tflags) );
		tsp=option->test; 
                fprintf(fp, " %o %d", int_flags(tsp->tsflags), tsp->attr );
	        if ( cut_test(tsp) ) 
                        fprintf(fp, " %g", tsp->tval.cut );
	        else if ( subset_test(tsp) ) 
                        fprintf(fp, " %o", tsp->tval.valset );
	        else if ( bigsubset_test(tsp) ) 
                       write_bitarray(tsp->tval.valbigset ,fp);
                fprintf(fp, " (%g %g %g)", 
			option->nprob, option->np.nprop, option->gain );
	        fprintf(fp,"\n");
                foroutcomes(i,option->test)
                         _write_ctree(fp,option->branches[i]);
	    }
        } else
	    fprintf(fp,"\n");
	if ( ferror(fp) )
		uerror("can't write chr-tree","");
	ct_depth--;
}

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
 *	read in single tree (in "char" format) from file with name "treename"
 */

read_chtr(treename, ret, head)
char    *treename;
t_head	*head;
ot_ptr	*ret;
{
        FILE    *fp, *efopen();
        ot_ptr   t;
        static ot_ptr   _read_ctree();
	static void    _read_chead();

        if (decision_graph_flag) 
                error("character format not implemented for dgraphs", "");
        fp = efopen(treename, "r");
        _read_chead(fp,head);
        t = _read_ctree(fp);
	t->parents = (bt_ptr *) salloc(sizeof(bt_ptr));
	t->parents[0] = 0;
#ifdef GRAPH
	t->parent_c = 1;
#endif
	t->num_parents = 1;
        fclose(fp);
        *ret = t;
}

static void _read_chead(fp,head)
FILE    *fp;
t_head	*head;
{
	unsigned int  oct;
        if ( fscanf(fp, "SIZE: %d", &head->treesize ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %d", &head->leafsize ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %o", &oct ) <= 0)
		uerror("can't read in char-format tree","");
	set_flags(head->hflags,oct);
        if ( fscanf(fp, " %lg", &head->sprob ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %lg", &head->eprob ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %lg", &head->genprob ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " PRIOR: %f", &head->prior.alphaval ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %f", &head->prior.node_weight ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %f", &head->prior.leaf_weight ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %f", &head->prior.p_join ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %d", &oct) <= 0)
		uerror("can't read in char-format tree","");
	set_flags(head->prior.prior_flag,oct);
        if ( fscanf(fp, " %d", &head->prior.depth_bound ) <= 0)
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " %lf", &head->prior.prior_nml ) <= 0)
		uerror("can't read in char-format tree","");
}

static ot_ptr
_read_ctree(fp)
register FILE    *fp;
{
        int     i,j, oct;
        register ot_ptr   t, t2;
        register bt_ptr   option;
	test_rec	*tsp;
       
        t = (ot_ptr) salloc( sizeof(ot_rec) );
        if ( fscanf(fp, " %o", &oct ) <= 0)
		uerror("can't read in char-format tree","");
	set_flags(t->tflags,oct);
	if ( tempty(t) ) {
		sfree(t);
		return 0;
	}
        t->eg_count = (float *) salloc( ndecs * sizeof(float) );
        if ( fscanf(fp, ", %f", &t->eg_count[0] ) <=0 )
		uerror("can't read in char-format tree","");
        for(i=1; i<ndecs; i++)
                if ( fscanf(fp, "+%f", &t->eg_count[i] ) <=0 )
		    uerror("can't read in char-format tree","");
        if ( fscanf(fp, " = %f", &t->tot_count ) <=0 )
		uerror("can't read in char-format tree","");
        if ( fscanf(fp, " (%f,%f)", &t->lprob, &t->bestprob ) <=1 )
		uerror("can't read in char-format tree","");
	if ( ttest(t) ) {
	    if ( toptions(t) ) {
                if ( fscanf(fp, " %d", &t->option.s.c ) <=0 )
			uerror("can't read in char-format tree","");
	        t->option.s.o = (bt_ptr *) salloc(t->option.s.c*sizeof(bt_ptr));
		for (i=0; i<t->option.s.c; i++)
		    t->option.s.o[i] = (bt_ptr) salloc(sizeof(bt_rec));
	    } else {
	        option = t->option.o = (bt_ptr) salloc(sizeof(bt_rec));
	    }
	    foroptions(option,j,t) {
		option->parent = t;
	        oct=0;
        	if ( fscanf(fp, " %o", &oct ) <= 0)
			uerror("can't read in char-format tree","");
		set_flags(option->tflags,oct);
	        option->test = tsp = (test_rec *) salloc(sizeof(test_rec));
	        for(;;) {
                    if ( fscanf(fp, " %o %d", &oct, &(tsp->attr)) <= 0)
		        uerror("can't read in char-format tree","");
	            set_flags(tsp->tsflags,oct);
	            tsp->no = 2;
	            if ( cut_test(tsp) ) {
        	        if ( fscanf(fp, " %f", &tsp->tval.cut) <= 0)
		            uerror("can't read in char-format tree","");
	            } else if ( subset_test(tsp) ) {
        	        if ( fscanf(fp, " %o", &tsp->tval.valset) <= 0)
		            uerror("can't read in char-format tree","");
	            } else if ( bigsubset_test(tsp) ) {
        	        if ( read_bitarray(tsp->tval.valbigset, fp) < 0)
		            uerror("can't read in char-format tree","");
	            } else {
		        tsp->tval.valset = 0;
		        tsp->no = unordvals(tsp->attr);
	            }
		    break;
	        }
                if ( fscanf(fp, " (%f %f %f)", 
		      &option->nprob, &option->np.nprop, &option->gain ) <=0 )
		    uerror("can't read in char-format tree","");
                option->branches =
			(ot_ptr *) salloc(outcomes(option->test)*sizeof(ot_ptr));
                foroutcomes(i,option->test) {
                    option->branches[i] = _read_ctree(fp);
		    if ( tempty(option->branches[i]) ) {
			sfree(option->branches[i]);
			option->branches[i] = 0;
		    } else {
                       t2 = option->branches[i];
                       t2->parents = (bt_ptr *) salloc(sizeof(bt_ptr));
                       t2->parents[0] = option;
#ifdef GRAPH
                       t2->parent_c = 1;
#endif
                       t2->num_parents = 1;
		    }
                }
	    }
	} else {
	    t->option.o = (bt_ptr)0;
	}
	return t;
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   Altered to copy decision graphs Jon Oliver August 1992
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

ot_ptr	copy_otree();
static ot_ptr	Rec_copy_otree();

ot_ptr find_jline_no();

/***************************************************************************/
/*
 *	copy_btree() -- copy tree complete except for "xtra.gr.egs"
 */
static bt_ptr Rec_copy_btree(bt)
bt_ptr	bt;
{
	void	*memcpy();
	int	i;
	bt_ptr	new_bt = (bt_ptr) salloc(sizeof(bt_rec));
	ot_ptr	ot;

	if ( !new_bt )
	    return (bt_ptr)0;
	memcpy(new_bt,bt,sizeof(bt_rec));
	if ( ! (new_bt->test = copy_test(bt->test)) )
	    return (bt_ptr)0;
	if ( bt->branches ) {
	    if ( !( new_bt->branches = (ot_ptr *) salloc(outcomes(new_bt->test) 
					* sizeof(ot_ptr)) ))
		    return (bt_ptr)0;
	    foroutcomes(i,new_bt->test) {
#ifdef GRAPH
		if ( bt->branches[i]->num_parents > 1 ) {
		    if ( previous_transit(bt->branches[i]) ) {
			/* Already copied */
			ot = find_jline_no(bt->branches[i]->line_no);
			ASSERT (ot);
			ASSERT (ot != bt->branches[i])
                        ASSERT (ot->line_no == bt->branches[i]->line_no)
                        ASSERT (ot->tot_count == bt->branches[i]->tot_count)

		    } else {
		    	if ( ! (ot = Rec_copy_otree(bt->branches[i])) )
				return (bt_ptr)0;
		  	assign_jline_no_node(ot);
		  	bt->branches[i]->line_no = ot->line_no;
			ASSERT (ot->parent_c == 0)
		    }
		} else 
#endif
		{
		    if ( ! (ot = Rec_copy_otree(bt->branches[i])) )
			return (bt_ptr)0;
		}
		new_bt->branches[i] = ot;
#ifdef GRAPH
		ot->parents[ot->parent_c] = new_bt;
		ot->parent_c ++;
	        ASSERT (ot->parent_c <= ot->num_parents)
#else
		ot->parents[0] =  new_bt;
#endif
	    } 
	} else {
	    new_bt->branches = (ot_ptr *)0;
	}
	return new_bt;
}

/***************************************************************************/
/*
 *	copy_otree() -- copy tree complete
 */
static ot_ptr Rec_copy_otree(ot)
ot_ptr	ot;
{
	void	*memcpy();
	int	i;
	bt_ptr	option;
	ot_ptr	new_ot;

	if ( ! (new_ot = mem_alloc(ot_rec)) )
		return (ot_ptr)0;
	memcpy(new_ot,ot,sizeof(ot_rec));

	if ( !(new_ot->parents = mems_alloc(bt_ptr,ot->num_parents) )) 
			return (ot_ptr)0;
	for (i=0; i< ot->num_parents; i++) 
		new_ot->parents[i] = (bt_ptr) 0;
#ifdef GRAPH
	new_ot->parent_c = 0;
#endif

	if ( toptions(ot) ) {
		if ( !(new_ot->option.s.o = 
			mems_alloc(bt_ptr,ot->option.s.c) ))
				return (ot_ptr)0;
		foroptions(option,i,ot) {
			if ( !(new_ot->option.s.o[i] = Rec_copy_btree(option) ))
				return (ot_ptr)0;
			new_ot->option.s.o[i]->parent = new_ot;
		}
	} else if ( ttest(ot) ) {
		if ( !(new_ot->option.o = Rec_copy_btree(ot->option.o) ))
			return (ot_ptr)0;
		new_ot->option.o->parent = new_ot;
	} else {
		new_ot->option.o = (bt_ptr)0;
	}
	if ( !(new_ot->eg_count = mems_alloc(float,ndecs) ))
		return (ot_ptr)0;
	if (ot->eg_count)
		memcpy(new_ot->eg_count,ot->eg_count,ndecs*sizeof(float));
	new_ot->testing.unordp = (unordtype *) 0;
	new_ot->xtra.gr.egs =  (egset *)0;
	return new_ot;
}

ot_ptr copy_otree(ot)
ot_ptr	ot;
{
        if (decision_graph_flag) {
		alloc_jline_no_tree();
		/*    will be using assign_jline_no_node() to fill
		 *    up table, so just empty it for now  */
		unset_transit_flag(ot);
		/* locks the transit flag */
	}
	ot = Rec_copy_otree(ot);
        if (decision_graph_flag) {
		unlock_transit_flag();
	}

#ifdef GRAPH
        ASSERT (ot->num_parents == 1)
        ASSERT (ot->parent_c == 0)
#endif
        ot->parents = (bt_ptr *) salloc(sizeof(bt_ptr)*ot->num_parents);
#ifdef GRAPH
        ot->parents[ot->parent_c] = 0;
        ot->parent_c ++;
	check_parent_graph(ot, __LINE__, __FILE__);
#else
	ot->parents[0] = 0;
#endif

	return(ot);
}

/***************************************************************************/
/*
 *      copy_bstump(bt) -- create bt_rec, using "bt" as a draft
 *               return 0 if out of memory;   assumes "bt" was created by
 *               choose();   ignores leaves and parents
 *
 */
bt_ptr
copy_bstump(bt)
bt_ptr  bt;             /*  draft bt_rec to copy   */
{
  bt_ptr   new_bt;
  if ( !(new_bt = mem_alloc(bt_rec) ) )
        return (bt_ptr)0;
  new_bt->tflags = bt->tflags;
  new_bt->nprob = bt->nprob;
  new_bt->np.nodew = bt->np.nodew;
  new_bt->gain = bt->gain;
  if ( ! (new_bt->test = copy_test(bt->test)) ) {
                sfree(new_bt);
                return (bt_ptr)0;
  }
  new_bt->branches = 0;
  return new_bt;
}


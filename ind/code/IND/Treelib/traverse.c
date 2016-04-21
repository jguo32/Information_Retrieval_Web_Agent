/*
 *   IND 1.0 released 9/15/91   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */
/*
 *   written 1992 Jonathan Oliver
 */
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/***********************************************************************/
/*
 *      traverse_onodes routine,  traverses all nodes of tree "t"
 *      and calls "f" at each node
 */

/*
 *      vanilla version
 */
static void     (*trav_o_vanilla_f)();

static Rec_traverse_onodes(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
	if (previous_transit(t))
		return;
	(*trav_o_vanilla_f)(t);
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				Rec_traverse_onodes(option->branches[j]);
			}
		}
	}
}

/*
 *        preorder traversal of all nodes
 */

#ifdef DEBUG_GRAPH
_traverse_onodes(t, f, fname, line)
ot_ptr  t;
void	(*f)();
char *fname;
int line;
#else
traverse_onodes(t, f)
ot_ptr  t;
void	(*f)();
#endif
{
        if (decision_graph_flag) {
#ifdef DEBUG_GRAPH
                _unset_transit_flag(t, fname, line);
#else
                unset_transit_flag(t);
#endif
		/* locks the transit flag */
	}
        trav_o_vanilla_f = f;
	Rec_traverse_onodes(t);
        if (decision_graph_flag) {
		unlock_transit_flag();
	}
}

/*
 *     keep depth counter as you go
 */
static   int   Depth;
static void     (*trav_o_depth_f)();

static Rec_traverse_onodes_depth(t)
ot_ptr  t;
{
int     i,j;
bt_ptr  option;
        if (t == NULL)
                return;
        if (previous_transit(t))
                return;
        (*trav_o_depth_f)(t,Depth);
        if ( ttest(t) ) {
                foroptions(option,i,t) {
                        Depth++;
                        foroutcomes(j,option->test) {
                                Rec_traverse_onodes_depth(option->branches[j]);
                        }
                        Depth--;
                }
        }
}


/*
 *        pre-order traversal, keep depth as you go
 */
traverse_onodes_depth(t, f)
ot_ptr  t;
void    (*f)();
{
        if (decision_graph_flag) {
                unset_transit_flag(t);
                /* locks the transit flag */
        }
        Depth = 0;
        trav_o_depth_f = f;
        Rec_traverse_onodes_depth(t);
        if (decision_graph_flag) {
                unlock_transit_flag();
        }
}

/***************************************************************
 * traversal using transit2
 *
 */

static void     (*trav_o_2nd_f)();

static Rec_traverse_onodes2(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
	if (previous_transit2(t))
		return;
        (*trav_o_2nd_f)(t);
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				Rec_traverse_onodes2(option->branches[j]);
			}
		}
	}
}

traverse_onodes2(t, f)
ot_ptr  t;
void	(*f)();
{
        if (decision_graph_flag) {
                unset_transit_flag2(t);
		/* locks the transit flag */
	}
        trav_o_2nd_f = f;
	Rec_traverse_onodes2(t);
        if (decision_graph_flag) {
		unlock_transit_flag2();
	}
}

/***************************************************************
 * traversal using transit3
 *
 */

static void     (*trav_o_3rd_f)();

static Rec_traverse_onodes3(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
	if (previous_transit3(t))
		return;
        (*trav_o_3rd_f)(t,Depth);
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				Rec_traverse_onodes3(option->branches[j]);
			}
		}
	}
}

traverse_onodes3(t, f)
ot_ptr  t;
void	(*f)();
{
        if (decision_graph_flag) {
                unset_transit_flag3(t);
		/* locks the transit flag */
	}
        trav_o_3rd_f = f;
	Rec_traverse_onodes3(t);
        if (decision_graph_flag) {
		unlock_transit_flag3();
	}
}

/***************************************************************
 * postorder traversal using transit2
 *
 */

static void     (*trav_o_post_f)();

static Rec_traverse_onodes_post(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
	if (previous_transit2(t))
		return;
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				Rec_traverse_onodes_post(option->branches[j]);
			}
		}
	}
        (*trav_o_post_f)(t,Depth);
}

traverse_onodes_post(t, f)
ot_ptr  t;
void	(*f)();
{
        if (decision_graph_flag) {
                unset_transit_flag2(t);
		/* locks the transit flag */
	}
        trav_o_post_f = f;
	Rec_traverse_onodes_post(t);
        if (decision_graph_flag) {
		unlock_transit_flag2();
	}
}

/***********************************************************************/
/*
 *	traverse_onodes_test	routine
 */

static void     (*trav_o_test_f)();

static Rec_traverse_onodes_test(t, testing)
ot_ptr  t;
egtesttype  testing;
{
int	i,j;
bt_ptr	option;

	if (t == NULL)
		return;
	if (previous_transit2(t))
		return;
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				add_test(option->test, i, testing);
				Rec_traverse_onodes_test(option->branches[j],
					testing);
				rem_test(option->test, i, testing);
			}
		}
	}

	(*trav_o_test_f )(t, testing);
}

traverse_onodes_test(t, f)
ot_ptr  t;
void	(*f)();
{
egtesttype  testing;
        if (decision_graph_flag) {
		unset_transit_flag2(t);
		/* locks the transit2 flag */
	}
	testing = t->testing;
        trav_o_test_f = f;
	Rec_traverse_onodes_test(t, testing);
        if (decision_graph_flag) {
		unlock_transit_flag2();
	}
}

/***********************************************************************/
/*
 *      traverse_parents routine,  traverses all parents of node "t"
 *      and call "f" at each node,  may visit parents multiple
 *      times so "f" must be OK to repeat
 */
static void     (*trav_p_f)();

static Rec_traverse_parents(t)
ot_ptr  t;
{
        int     i;

        ASSERT ( t );
        (*trav_p_f)(t);
        if ( t->num_parents ) {
                for (i=t->num_parents-1 ; i>=0; i-- )
                        if (t->parents[i])
			    Rec_traverse_parents(t->parents[i]->parent);
        }
}

traverse_parents(t, f)
ot_ptr  t;
void    (*f)();
{
        trav_p_f = f;
        Rec_traverse_parents(t);
}

/***********************************************************************/
/*
 *	transit	routine
 */

char unset_fname[100];
int  unset_line;
static char Lock, Lock2, Lock3;

unlock_transit_flag()
{
	ASSERT( Lock )
	Lock = 0;
	strcpy(unset_fname, "none");
	unset_line = 0;
}

unlock_transit_flag2()
{
        ASSERT( Lock2 )
        Lock2 = 0;
}

unlock_transit_flag3()
{
        ASSERT( Lock3 )
        Lock3 = 0;
}

#ifdef DEBUG_GRAPH
_unset_transit_flag(t, fname, line)
ot_ptr t;
char *fname;
int line;
{
	if( Lock ) {
                printf("error 2 transits file %s line %d\n", fname, line);
                printf("already doing    file %s line %d\n",
			unset_fname, unset_line);
                exit(0);
        }
	strcpy(unset_fname, fname);
	unset_line = line;
#else
unset_transit_flag(t)
ot_ptr t;
{
	if( Lock ) 
		error("transit flag locked","");
#endif
	Lock = 1;
	Rec_unset_transit_flag(t);
}

Rec_unset_transit_flag(t)
ot_ptr t;
{
int	i,j;
bt_ptr	option;

        if (t == NULL)
                return;
	t->tflags.b.transit = 0;
        if ( ttest(t) ) {
                foroptions(option,i,t) {
                        foroutcomes(j,option->test) {
                                Rec_unset_transit_flag(option->branches[j]);
                        }
                }
        }
}

/***********************************************************************/
/*
 *	unset_transit_flag2	routine
 */

unset_transit_flag2(t)
ot_ptr t;
{
        ASSERT ( !Lock2 )
        Lock2 = 1;
        Rec_unset_transit_flag2(t);
}

Rec_unset_transit_flag2(t)
ot_ptr t;
{
int	i,j;
bt_ptr	option;

        if (t == NULL)
                return;
	t->tflags.b.transit2 = 0;
        if ( ttest(t) ) {
                foroptions(option,i,t) {
                        foroutcomes(j,option->test) {
                                Rec_unset_transit_flag2(option->branches[j]);
                        }
                }
        }
}

/***********************************************************************/
/*
 *	unset_transit_flag3	routine
 */

unset_transit_flag3(t)
ot_ptr t;
{
        ASSERT ( !Lock3 )
        Lock3 = 1;
        Rec_unset_transit_flag3(t);
}

Rec_unset_transit_flag3(t)
ot_ptr t;
{
int	i,j;
bt_ptr	option;

        if (t == NULL)
                return;
	t->tflags.b.transit3 = 0;
        if ( ttest(t) ) {
                foroptions(option,i,t) {
                        foroutcomes(j,option->test) {
                                Rec_unset_transit_flag3(option->branches[j]);
                        }
                }
        }
}

/***********************************************************************/
/*
 *	previous_transit routine
 */

previous_transit(t)
ot_ptr t;
{
	if (! decision_graph_flag)
		return(0);
	if (t->tflags.b.transit) {
		return(1);
	}
	t->tflags.b.transit = 1;
	return(0);
}

previous_transit2(t)
ot_ptr t;
{
	if (! decision_graph_flag)
		return(0);
	if (t->tflags.b.transit2) {
		return(1);
	}
	t->tflags.b.transit2 = 1;
	return(0);
}

previous_transit3(t)
ot_ptr t;
{
	if (! decision_graph_flag)
		return(0);
	if (t->tflags.b.transit3) {
		return(1);
	}
	t->tflags.b.transit3 = 1;
	return(0);
}

/***********************************************************************/
/*
 *      find_onodes     routine to find the node in "t" which returns
 *                      !0 for the function "f"
 */
static int (*find_o_f)();

static ot_ptr Rec_find_onode(t)
ot_ptr  t;
{
int	i,j;
bt_ptr	option;
ot_ptr  t2;

	if (t == NULL)
		return (ot_ptr) 0;
	if (previous_transit2(t))
		return (ot_ptr) 0;
	if ( (*find_o_f)(t) )
		return (t);
	if ( ttest(t) ) {
		foroptions(option,i,t) {
	        	foroutcomes(j,option->test) {
				t2 = Rec_find_onode(option->branches[j]);
				if (t2) {
					return (t2);
				}
			}
		}
	}
	return ((ot_ptr) 0);
}

ot_ptr find_onode(t, f)
ot_ptr  t;
int	(*f)();
{
ot_ptr  t2;
        if (decision_graph_flag) {
		unset_transit_flag2(t);
		/* locks the transit flag */
	}
        find_o_f = f;
	t2 = Rec_find_onode(t);
        if (decision_graph_flag) {
		unlock_transit_flag2();
	}
	return (t2);
}

static void zero_line_no_node(t)
ot_ptr	t;
{
	t->line_no = 0;
}

zero_line_no_tree(t)
ot_ptr	t;
{
	traverse_onodes2(t, zero_line_no_node);
}

#ifdef DEBUG_GRAPH
/***********************************************************************/
/*
 *	first_parent	routine
 */


bt_ptr _first_parent(t)
ot_ptr	t;
{
	ASSERT ( t->num_parents == 1 );  
	return ( t->parents[0] );
}

_no_parents(t)
ot_ptr	t;
{
bt_ptr tp;
	tp = first_parent(t);
	return ( !tp || !tp->parent );
}

ot_ptr _oparent(t)
ot_ptr	t;
{
bt_ptr tp;
	tp = first_parent(t);
	return (tp->parent);
}
#endif  /*  DEBUG_GRAPH  */


/***********************************************************************/
/*
 *	find_jline_no    routine to find the join node in "t" which 
 *                       has line_no, much more efficient than above
 *      assign_jline_no_tree    routine assigns line numbers to 
 *                              join nodes only
 */

static ot_ptr *Line_ptr = 0;		/*    array indexed by Line_no   */
static int    Line_ptr_size = 0;	/*    size of the Line_ptr array  */
static int    Line_ptr_max = 0;		/*    amount of array actually used  */

/*
 *	will only find join nodes
 */
ot_ptr find_jline_no(line_no)
int	line_no;
{
	if (decision_graph_flag)  {
		ASSERT ( Line_ptr )
		if ( line_no >= Line_ptr_max ) {
			return (ot_ptr) 0;
		} else  {
			return Line_ptr[line_no];
		}
	} else {
		return (ot_ptr) 0;
	}
}

assign_jline_no_node(t)
ot_ptr	t;
{
	if ( t->num_parents > 1 ) {
	  /*
	   *     as long as "Line_ptr" array exists, use it
	   *     but when out of memory, "Line_ptr" will
	   *     will be set to 0 so revert to slow method
	   *     used above
	   */
	  ASSERT ( Line_ptr )
	  if ( Line_ptr_max >= Line_ptr_size ) {
	  	Line_ptr_size += 10;
	  	Line_ptr = mems_realloc(Line_ptr,ot_ptr,Line_ptr_size);
	  }
	  Line_ptr[Line_ptr_max] = t;
	  t->line_no = Line_ptr_max++;
	} else
	  t->line_no = 0;
}

save_jline_no_node(t)
ot_ptr	t;
{
int k;
	ASSERT ( Line_ptr )
	if ( t->line_no >= Line_ptr_size ) {
		Line_ptr_size += 10;
		Line_ptr = mems_realloc(Line_ptr,ot_ptr,Line_ptr_size);
		for (k=Line_ptr_size-10; k<Line_ptr_size; k++)
			Line_ptr[k] = (ot_ptr)0;
	}
	if ( t->line_no >= Line_ptr_max ) {
		Line_ptr_max = t->line_no+1;
	}
	Line_ptr[t->line_no] = t;
}

unsave_jline_no_node(t)
ot_ptr	t;
{
	ASSERT ( Line_ptr )
	ASSERT ( t->line_no < Line_ptr_max )
	ASSERT (Line_ptr[t->line_no] == t)
	Line_ptr[t->line_no] = (ot_ptr)0;
}

alloc_jline_no_tree()
{
int k;
	Line_ptr_max = 1;
	if ( !Line_ptr ) {
		if ( !(Line_ptr = mems_alloc(ot_ptr,10)) ) {
			for (k=0; k< 10; k++)
				Line_ptr[k] = (ot_ptr)0;
			return 0;
		}
	} else {
		for (k=0; k<Line_ptr_size; k++)
			Line_ptr[k] = (ot_ptr)0;
	}
        return 1;
}

jline_stored()
{
        return (Line_ptr_max);
}

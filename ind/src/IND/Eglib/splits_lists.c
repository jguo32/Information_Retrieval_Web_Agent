/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/* #define OLD_STUFF  */
/* #define DEBUG */
/*  #define CHECK */

static bool  copy_sets=TRUE;

/***************************************************************************/
/*
 *	release_egs(t) - release sets added by add_egs()
 */
release_egs(bt)
bt_ptr		bt;
{
	int	i;
	ot_ptr  ot;
	forbranches(ot,i,bt) {
          free_set(ot->xtra.gr.egs);    
	  ot->xtra.gr.egs = (egset*)0; 
	}
}

/***************************************************************************/
/*
 *	release_tree_egs(t) - recursively release sets added by add_egs()
 */
release_tree_egs(ot)
ot_ptr		ot;
{
        int	i,j;
	bt_ptr	option;
	ot_ptr  c_ot;
	if ( ot ) {
	  if ( ot->xtra.gr.egs ) {
	    free_set(ot->xtra.gr.egs);  
	    ot->xtra.gr.egs = (egset*)0;
	  }
	  if ( ttest(ot) ) {
	      foroptions(option,i,ot) {
		forbranches(c_ot,j,option) 
		  release_tree_egs(c_ot);
	      }
	  }
	}
}

#define Copy_Weighted_Set(eg)   \
        if ( weighted(eg) ) {                                           \
                foreachegw(eg,*(new_eg++),egp,*(new_wt++),wtp) ;        \
        } else {                                                        \
                foreacheg(eg,*(new_eg++),egp) ;                 \
                for (i=setsize(eg)-1; i>=0; i--)                        \
                        *(new_wt++) = 1.0;                              \
        }

/***************************************************************************/
/*
 *      merge_egs2()
 *                   return 1 if some memory error, 0 if OK
 */
egset *merge_egs2(eg1, eg2)
egset   *eg1, *eg2;
{
int i;
egset   *new;
egtype  *new_eg;
float   *new_wt, *wtp;
foregtype   egp;
int     newsize=0;
bool    newweighted=FALSE;

        newsize = setsize(eg1) + setsize(eg2);
        if ( ! (new = new_set(newsize) )  )
                return (egset *)0;
        if ( weighted(eg1) || weighted(eg2) )
                newweighted =  TRUE;
        new_eg = new->members;
        if ( newweighted )  {
                if ( ! (new->weights = mems_alloc(float,newsize) )  )
                        return (egset *)0;
                new_wt = new->weights;

                Copy_Weighted_Set(eg1);
                Copy_Weighted_Set(eg2);
        } else {
                foreacheg(eg1,*(new_eg++),egp) ;
                foreacheg(eg2,*(new_eg++),egp) ;
        }
        new->size = newsize;
        new->copied = FALSE;
        return new;
}

/***************************************************************************/
/*
 *	add_egs(t) - partition eg set from bt's parent down to all of bt's
 *			children
 *                NB.   this routine has been optimised for speed,
 *                      so quite a bit of redundant code in places
 *		     return 1 if some error, 0 if OK
 */
int
add_egs(bt)
bt_ptr		bt;
{
        extern  ot_ptr  tree;
	ot_ptr   ot;
	egset	*list;          /*  place list being split here  */
	egset	*tegs;
	egtype	eg, eg1;
	int	i,j,nv;
	int	mf;
	static egtype   **sets;    /* split list's egs, indexed by outcome  */
	static float   **wghts;    /* split list's weights, indexed by outcome  */
	static short *outcome_set=0;  /* outcome indexed by elements of "list" */
	static  int  outcome_set_size = 0;
	foregtype	egp;
	float   *props;
	float   wt,wt1, *wtp;
	bool    unknns = FALSE, new_weights;
	static int   *osets;       /*  cardinality of set indexed by outcome */
	static int     *stop;
	static int     *next;
#ifdef	CHECK
	int	v;
	egset	*copy, *set;
#endif   CHECK

	/*
	 *	check for silly cases
	 */
	if ( !bt ) return 1;
	list = bt->parent->xtra.gr.egs;
#ifdef	CHECK
	copy = copy_set(list);
	if ( !copy )  return 1;
#endif   CHECK
	if ( ! setsize(list) )
	  error("add_egs called on empty set","");
	if ( bt->branches[0]->xtra.gr.egs )
		return 0;

	/*
	 *	process missing values options to find parameters
	 */
	nv = outcomes(bt->test);
	if (  Uflg == 2 )
	    error("add_egs has unimplemented Uflg\n","");
	if ( Uflg == 3 && unkns(bt->test->attr) ) {
	    /*
	     *	find most frequent outcome
	     */
	    mf = 0;
	    forbranches(ot,i,bt)
		if ( i && ot->tot_count >=
				bt->branches[mf]->tot_count )
			mf = i;
	} else
		mf = nv;
	if ( Uflg == 4 || Uflg == 1 )
	  props = subtree_prop(bt);

	/*
	 *	initialize arrays
	 */
	if ( !outcome_set ) {
	    outcome_set = (short *) 
	      salloc(setsize(list)*sizeof(short));
	    outcome_set_size = setsize(list);
	    sets = (egtype **) salloc((nvalsattr+1)*sizeof(egtype *));
	    wghts = (float **) salloc((nvalsattr+1)*sizeof(egtype *));
	    osets = (int *) salloc((nvalsattr+1)*sizeof(int));
	    stop = (int *) salloc((nvalsattr+1)*sizeof(int));
	    next = mems_alloc(int,nvalsattr+1);
#ifdef DEBUG_dealloc
	    record_alloc(0,0,"add_egs()",0);
#endif
	    if ( !next )
		return 1;
	} else if ( setsize(list) > outcome_set_size ) {
	    outcome_set = mems_realloc(outcome_set,short,setsize(list));
	    outcome_set_size = setsize(list);
	    if ( !outcome_set )
		return 1;
	}

	for (i=0; i<=nv; i++)
	  osets[i]=0;
	/*
	 *	have to create whole new sets,
	 *  first, work out how many of each outcome, and store
	 *  outcomes temporarily in array
	 */
	 switch ( Uflg ) {
	      default:
		for(i=setsize(list)-1; i>=0; i--) {
		  outcome_set[i] = outcome(example(list,i), bt->test);
		  if ( outcome_set[i] >= 0 )
		    osets[outcome_set[i]]++;
		  else {
		    outcome_set[i] = nv;
		    unknns = TRUE;
		  }
		}
		break;
	      case 1: 
	      case 5:
		for(i=setsize(list)-1; i>=0; i--) {
		  outcome_set[i] = outcome(example(list,i), bt->test);
		  if ( outcome_set[i] < 0 ) {
		    unknns=TRUE;
		    forbranches(ot,j,bt) 
		      osets[j]++;
		  } else
		    osets[outcome_set[i]]++;
		}
		break;
	      case 3:
		for(i=setsize(list)-1; i>=0; i--) {
		  outcome_set[i] = outcome(example(list,i), bt->test);
		  if ( outcome_set[i] < 0 )
		    outcome_set[i] = mf;
		  osets[outcome_set[i]]++;
		}		  
		break;
	      case 4:
		for(i=setsize(list)-1; i>=0; i--) {
		  outcome_set[i] = outcome(example(list,i), bt->test);
		  if ( outcome_set[i] < 0 )
		    outcome_set[i] = vrandom(props,nv);
		  osets[outcome_set[i]]++;
		}		  
		break;
	}

        /*
         *	check if new weight arrays required
         */	
        new_weights =  (Uflg==1 || Uflg==5) && unknns;

        /*
         *	now assign each of the examples to the sets
         */
	if ( !copy_sets && !new_weights &&
	    (!toptions(bt->parent) || bt == bt->parent->option.s.o[0]) ) {
	  /*
	   *	not creating weights this time,
	   *    and this is first option in list of parent's options
	   *	so partition bt's parents egs. insitu
	   */
	  /*
	   *  assign sets to nodes
	   */
	  forbranches(ot,i,bt) {
	    tegs = ot->xtra.gr.egs = new_set(0);
	    if ( !tegs )  return 1;
	    tegs->size = osets[i];
	    if ( weighted(list) && osets[i] )
	      if ( !( tegs->weights =
			(float *) salloc(osets[i]*sizeof(int)) ))
		return 1;
            tegs->copied = TRUE;
	  }
	  /*
	   *  now rearrange parent set of examples to agree with outcome_set
	   *     NB.   the nv-th partition corresponds to unknowns if any
	   */
	  for ( next[0] = -1, i=1; i<=nv; i++)
	    stop[i-1] = next[i] = next[i-1]+osets[i-1];
	  stop[nv] = list->size-1;
	  for(j=0; j<=nv; j++) {
	    /*     search for last out-of-place eg in j-th partition */
	    while ( stop[j] > next[j] && outcome_set[stop[j]] == j )
	      stop[j]--;
	    if ( stop[j]<=next[j] )
	      continue;               /*   j-th partition now in place  */
	    i = outcome_set[stop[j]];
	    outcome_set[stop[j]] = -1;   /*  flag set so next for-loop stops */
	    eg = list->members[stop[j]];
	    if ( weighted(list) ) 
	      wt = list->weights[stop[j]];
	    /*
	     *   eg, wt, i   store details of currently out-of-place example
	     */
	    for (;;) {
	      /*
	       *   find a spot for out-of-place to go
	       */
	      next[i]++;
	      while ( outcome_set[next[i]] == i )
		next[i]++;
	      if ( outcome_set[next[i]]<0 ) {
		/*
		 *   no example subsequently displaced, so i-th (= j-th)
		 *   partition now complete
		 */
		if ( i!= j )
		  error("Shuffle muddled in add_egs","");
		list->members[next[i]] = eg;
		if ( weighted(list) ) 
		  list->weights[next[i]] = wt;
		break;
	      }
	      /*
	       *    example so displaced becomes next out-of-place example
	       */
	      eg1 = list->members[next[i]];
	      list->members[next[i]] = eg;
	      eg = eg1;
	      if ( weighted(list) ) {
		wt1 = list->weights[next[i]];
		list->weights[next[i]] = wt;
		wt = wt1;
	      }
	      i = outcome_set[next[i]];
	    }
	  }
	  /*
	   *  now point new sets into this sorted parent set
	   */
	  for ( bt->branches[0]->xtra.gr.egs->members=list->members, i=1;
	       i<nv; i++)
	    bt->branches[i]->xtra.gr.egs->members =
	      bt->branches[i-1]->xtra.gr.egs->members + osets[i-1];    
	  if ( weighted(list) )
	    for ( bt->branches[0]->xtra.gr.egs->weights=list->weights, i=1;
		 i<nv; i++)
	      bt->branches[i]->xtra.gr.egs->weights =
		bt->branches[i-1]->xtra.gr.egs->weights + osets[i-1];    
	} else {
	  /*
	   *	have to construct entire new sets
	   */
	  /*
	   *  assign new sets to nodes
	   */
	  forbranches(ot,i,bt) {
	    tegs = ot->xtra.gr.egs = new_set(osets[i]);
	    if ( !tegs ) return 1;
	    tegs->size = osets[i];
	    if ( osets[i] && ( weighted(list) || new_weights) )
	      if ( ! (tegs->weights = mems_alloc(float,osets[i]) ) )
		return 1;
	    sets[i] = tegs->members;
	    wghts[i] = tegs->weights;
	  }
	  /*
	   *     spread examples out across sets, according to "outcome_set"
	   */
	  if ( weighted(list) ) {
	    if ( new_weights ) {
	      /*
	       *    partition AND update existing weights by Uflg==1,5
	       */
	      if ( Uflg==1 ) {
		for (i=0, egp=list->members, wtp=list->weights; 
		                               i<setsize(list); i++) {
		  if ( outcome_set[i]>=0 ) {
		    *(sets[outcome_set[i]]++) = *(egp++);
		    *(wghts[outcome_set[i]]++) = *(wtp++);
		  } else {
		    forbranches(ot,j,bt) {
		      *(sets[j]++) = *egp;
		      *(wghts[j]++) = *wtp * props[j];
		    }
		    wtp++;
		    egp++;
		  }
	        }
	      } else {   /*  Uflg==5, identical to above code except "/ nv"  */
		for (i=0, egp=list->members, wtp=list->weights; 
		                               i<setsize(list); i++) {
		  if ( outcome_set[i]>=0 ) {
		    *(sets[outcome_set[i]]++) = *(egp++);
		    *(wghts[outcome_set[i]]++) = *(wtp++);
		  } else {
		    forbranches(ot,j,bt) {
		      *(sets[j]++) = *egp;
		      *(wghts[j]++) = *wtp / nv;
		    }
		    wtp++;
		    egp++;
		  }
	        }
	      }
	    } else {
	      /*
	       *    no modifications, simply partition examples
	       */
	      if ( unknns ) {
		for (i=0, egp=list->members, wtp=list->weights; 
		                               i<setsize(list); i++) {
		    if ( outcome_set[i]>=0 ) {
		      *(sets[outcome_set[i]]++) = *(egp++);
		      *(wghts[outcome_set[i]]++) = *(wtp++);
		    }
	        }
	      } else {
		for (i=0, egp=list->members, wtp=list->weights; 
		                               i<setsize(list); i++) {
		    *(sets[outcome_set[i]]++) = *(egp++);
		    *(wghts[outcome_set[i]]++) = *(wtp++);
	        }
	      }
	    }
	  } else {
	    if ( new_weights ) {
	      /*
	       *    partition AND create weights by Uflg==1,5
	       */
	      if ( Uflg==1 ) {
		for (i=0, egp=list->members; i<setsize(list); i++) {
		  if ( outcome_set[i]>=0 ) {
		    *(sets[outcome_set[i]]++) = *(egp++);
		    *(wghts[outcome_set[i]]++) = 1.0;
		  } else {
		    forbranches(ot,j,bt) {
		      *(sets[j]++) = *egp;
		      *(wghts[j]++) = props[j];
		    }
		    egp++;
		  }
	        }
	      } else {   /*  Uflg==5, identical to above code except "/ nv"  */
		for (i=0, egp=list->members; i<setsize(list); i++) {
		  if ( outcome_set[i]>=0 ) {
		    *(sets[outcome_set[i]]++) = *(egp++);
		    *(wghts[outcome_set[i]]++) = 1.0;
		  } else {
		    forbranches(ot,j,bt) {
		      *(sets[j]++) = *egp;
		      *(wghts[j]++) = 1.0 / nv;
		    }
		    egp++;
		  }
	        }
	      }
	    } else {
	      /*
	       *    no modifications, simply partition examples
	       */
	      if ( unknns ) {
		for (i=0, egp=list->members; i<setsize(list); i++) {
		    if ( outcome_set[i]>=0 ) {
		      *(sets[outcome_set[i]]++) = *(egp++);
		    }
	        }
	      } else {
		for (i=0, egp=list->members; i<setsize(list); i++) {
		    *(sets[outcome_set[i]]++) = *(egp++);
	        }
	      }
	    }
	  }
      }

#ifdef CHECK
	/*
	 *	check to see output sets add up to original
	 */
	forbranches(ot,i,bt) {
	    set = ot->xtra.gr.egs;
	    foreacheg(set,eg,egp) {
		for (j=copy->size-1; j>=0; j--)
		    if ( copy->members[j].unordp==eg.unordp ) {
			copy->members[j].unordp = 0;
			break;
		    }
	    }
	}
	foreacheg(copy,eg,egp)
	    if ( eg.unordp && ( Uflg!=0 || outcome(eg, bt->test)>=0 ) )
		error("partitioning faulty","");
	/*
	 *	check to see output sets have correct entries
	 */
	forbranches(ot,i,bt) {
	    set = ot->xtra.gr.egs;
	    foreacheg(set,eg,egp) {
		if ( (v=outcome(eg, bt->test))!=i &&
			!( Uflg==3 && mf==i && v<0 ) &&
		        !( Uflg==1 && v<0 && unknns ) &&
		        !( Uflg==5 && v<0 && unknns ) &&
		        !( Uflg==4 && v<0 ) )
			error("partitioning faulty","");
	    }
	}
  	sfree((char*)copy->members);  
  	sfree((char*)copy);  
#endif
	return 0;
}


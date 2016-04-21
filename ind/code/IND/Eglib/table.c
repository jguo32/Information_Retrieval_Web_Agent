/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"
#include "TABLE.h"

	ftbl	tbl;
extern  float  min_set_split;
extern  bool   t_flg;

/*******************************************************************/
/*
 *	tinit(tbl,tester) -- zero out "tbl" for test "tester"
 */
void
tinit(atbl,tester)
register ftbl	*atbl;
register test_rec	*tester;
{
	register int	i,j;
	atbl->tester = tester;
	if (tester)
		atbl->nv = outcomes(tester);
	else
		atbl->nv = ndecs;
	atbl->tot_known = 0;
	atbl->unknown.vals = 0;
	fordecs(j) {
	    atbl->unknown.d[j] = 0;
	}
	forless(i,atbl->nv) {
	    atbl->known[i].vals = 0;
	    fordecs(j) 
		atbl->known[i].d[j] = 0;
	}
	null_flags(atbl->flags);
	if ( tester ) {
		if ( subsets_test(tester) && remdr_test(tester) )
	  		set_flag(atbl->flags,masked);
	}
}

/*******************************************************************/
/*
 *	tmake(set, tester) -- initialise the counting table;
 *	for ordered attributes, just consider known values (no split)
 */
void
tmake(set, tester)
egset	*set;
register test_rec *tester;
{
	int	v;	/* Scratch variables */
	egtype	e;	/* An example */
		foregtype egp;
		 int	dec;	/* A decision. */
	float    wt, *wtp;
	int	 nv;
	int	some;
	bool	prepare_set;
	if ( ! tbl.known ) {
		/*
		 *	initialise  "tbl" first time through
		 */
		talloc(&tbl, Max( ndecs, nvalsattr+3 ));
#ifdef DEBUG_dealloc
		record_alloc(0,0,"t_make()",0);
#endif
	}
	tinit(&tbl, tester);
	/*
	 *	Count the frequency of each (decision, value) combination.
	 *      NB.   this splices in code from tester() with counting
	 */
	if ( !set )
	  ;
	else if ( !tester ) {
	  if ( weighted(set) ) {
	     foreachegw(set,e,egp,wt,wtp)  {
	        dec = class_val(e);
	    	tbl.known[dec].d[dec] += wt;
	     }
	  } else {
	     foreacheg(set,e,egp) {
	        dec = class_val(e);
	    	tbl.known[dec].d[dec] ++;
	     }
	  }
	} else if (cut_test(tester) ) {
	    nv = 2;
	    if ( tester->tval.cut <= -FLOATMAX*0.9 ) {
	       /*
	        *    put everything in GREATERTHAN for now
		*    and leave unknowns unassigned - it'll have to be
		*    done dynamically
	        */
	       unset_flag(tbl.flags,unknowns_assigned);
	       if ( weighted(set) ) {
	         foreachegw(set,e,egp,wt,wtp)  {
	    	    if (ord_unoutcome(e, tester))
	    		tbl.unknown.d[class_val(e)] += wt;
	    	    else
	    		tbl.known[GREATERTHAN].d[class_val(e)] += wt;
	         }
	       } else {
	         foreacheg(set,e,egp) {
	    	    if (ord_unoutcome(e, tester))
	    		tbl.unknown.d[class_val(e)] ++;
	    	    else
	    		tbl.known[GREATERTHAN].d[class_val(e)] ++;
	         }
	       }
	    } else {
	        /*
	         */
	       if ( weighted(set) ) {
	         foreachegw(set,e,egp,wt,wtp) {
	    	    if (ord_unoutcome(e, tester))
	    		tbl.unknown.d[class_val(e)] += wt;
		    else
	    		tbl.known[ord_outcome(e, tester)].d[class_val(e)] 
			  += wt;
	         }
	       } else {
	         foreacheg(set,e,egp) {
	    	    if (ord_unoutcome(e, tester))
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[ord_outcome(e, tester)].d[class_val(e)] ++;
	         }
	       }
	    }
	} else if ( set_test(tester) ) {
	;
	} else if ( subset_test(tester) ) {
	    nv = unordvals(tester->attr);
	    if ( !tester->tval.valset ) {
	       /*
		*    prepare for subset testing ...
	        *    split everything up + add empty set test
	        */
		prepare_set=TRUE;
		for(v=1+nv; v>1; v--) {
	    	    tbl.known[v].vals = 0;
	    	    fordecs(dec) 
			tbl.known[v].d[dec] = 0;
		}
	        if ( weighted(set) ) {
	          foreachegw(set,e,egp,wt,wtp)  {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[v+2].d[class_val(e)]++;
		  }
	        } else {
	          foreacheg(set,e,egp) {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[v+2].d[class_val(e)]++;
		  }
	        }
	      } else {
		prepare_set=FALSE;
	        /*
	         *    a valset exists so use it to make split
	         */
	        if ( weighted(set) ) {
	          foreachegw(set,e,egp,wt,wtp)  {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] += wt;
		    else
	    		tbl.known[bit_set(tester->tval.valset,v)?1:0]
					.d[class_val(e)]+= wt;
	          }
		} else {
	          foreacheg(set,e,egp) {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[bit_set(tester->tval.valset,v)?1:0]
					.d[class_val(e)]++;
	          }
		}
	      }
	} else if ( bigsubset_test(tester) ) {
	    /*
	     *  WARNING:  duplicates code for "subset_test(tester)"
	     */
	    nv = unordvals(tester->attr);
	    bitarray_empty(tester->tval.valbigset);
	    if ( bs_flag ) {
	        /*
		 *    prepare for subset testing ...
	         *    split everything up + add empty set test
	         */
		prepare_set=TRUE;
		for(v=1+nv; v>1; v--) {
	    	    tbl.known[v].vals = 0;
	    	    fordecs(dec) 
			tbl.known[v].d[dec] = 0;
		}
	        if ( weighted(set) ) {
	          foreachegw(set,e,egp,wt,wtp)  {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[v+2].d[class_val(e)]++;
		  }
	        } else {
	          foreacheg(set,e,egp) {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[v+2].d[class_val(e)]++;
		  }
	        }
	    } else {
	        /*
	         *    a valset exists so use it to make split
	         */
		prepare_set=FALSE;
	        if ( weighted(set) ) {
	          foreachegw(set,e,egp,wt,wtp)  {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] += wt;
		    else
	    		tbl.known[bitarray_set(tester->tval.valbigset,v)?1:0]
					.d[class_val(e)]+= wt;
	          }
		} else {
	          foreacheg(set,e,egp) {
	    	    if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		    else
	    		tbl.known[bitarray_set(tester->tval.valbigset,v)?1:0]
					.d[class_val(e)]++;
	          }
		}
	    }
	} else {
	  /*
	   *     assume standard splitting on attribute
	   */
	  nv = unordvals(tester->attr);
	  if ( weighted(set) ) {
	    foreachegw(set,e,egp,wt,wtp)  {
	    	if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] += wt;
		else
	    		tbl.known[v].d[class_val(e)] += wt;
	    }
	  } else {
	    foreacheg(set,e,egp) {
	    	if ((v=attr_outcome(e, tester))==DONTKNOW)
	    		tbl.unknown.d[class_val(e)] ++;
		else
	    		tbl.known[v].d[class_val(e)] ++;
	    }
	  }
	}
	/*
	 *		count totals, deal with unknowns and nullsplit, etc.
	 */
	fordecs(dec)
		tbl.unknown.vals += tbl.unknown.d[dec];
	if ( tester && subsets_test(tester) && prepare_set ) {
	    /*
	     *      am initializing table for subsets test
	     */
	    for(v=nv+1; v>1; v--) {
		fordecs(dec) {
			tbl.known[v].vals += tbl.known[v].d[dec];
			tbl.known[0].d[dec] += tbl.known[v].d[dec];
		}
		tbl.tot_known += tbl.known[v].vals;
	    }
	    tbl.known[0].vals = tbl.tot_known;
	    if ( !tbl.tot_known ) 
	      set_flag(tbl.flags,nullsplit);    
	    else if ( t_flg && tbl.unknown.vals ) {
	        /*
		 *    unknowns being assigned, so sprinkle
		 *    them proportionally amongst the right entries
		 */
	        set_flag(tbl.flags,unknowns_assigned);
		for(v=nv+1; v>1; v--) {
		  fordecs(dec) {
			tbl.known[v].d[dec] += 
			  tbl.unknown.d[dec] * tbl.known[v].vals / tbl.tot_known;
		  }
		  tbl.known[v].vals *=  
			  1.0 + tbl.unknown.vals / tbl.tot_known ;
		}
		fordecs(dec) {
			tbl.known[0].d[dec] += tbl.unknown.d[dec] ;
		}
		tbl.tot_known += tbl.unknown.vals;	
		tbl.known[0].vals = tbl.tot_known;
	    }
	} else {
	    some = 0;
	    forless(v,tbl.nv) {
		fordecs(dec) {
			tbl.known[v].vals += tbl.known[v].d[dec];
		}
		tbl.tot_known += tbl.known[v].vals;
		if ( tbl.known[v].vals ) some++;
	    }  
	    if ( !tbl.tot_known ) 
	      set_flag(tbl.flags,nullsplit);    
	    else if ( some<=1  )
		set_flag(tbl.flags,nullsplit);
	    else if ( t_flg && tbl.unknown.vals ) {
	        /*
		 *    unknowns being assigned, so sprinkle
		 *    them proportionally amongst the right entries
		 */
	        set_flag(tbl.flags,unknowns_assigned);
		for(v=nv-1; v>=0; v--) {
		  fordecs(dec) {
			tbl.known[v].d[dec] += 
			  tbl.unknown.d[dec] * tbl.known[v].vals / tbl.tot_known;
		  }
		  tbl.known[v].vals *=  
			  1.0 + tbl.unknown.vals / tbl.tot_known ;
		}
		tbl.tot_known += tbl.unknown.vals;	
	    }
	    some = 0;
            forless(v,tbl.nv) 
 		if ( tbl.known[v].vals >= min_set_split ) 
			some++;
	    if ( some <= 1 )
		set_flag(tbl.flags,nullsplit);
	}  
	if ( !tbl.unknown.vals ) 
	  set_flag(tbl.flags,unknowns_assigned);
}

ttotdec(decs)
float *decs;
{
	tbl.d = decs;
}


/*******************************************************************/
/*
 *	tmod_one(set,index) -- modify counting table by moving "index"
 *			element in set from partition 1 to 0
 */
void
tmod_one(set,index)
egset	*set;
int	index;
{
#define  v1  1
#define  v2  0
	int	dec;	/* A decision. */
	float    wt;
	fent 	*fr = tbl.known+v1,
		*to = tbl.known+v2;
        if ( weighted(set) ) {
		to->vals   += (wt=weight(set,index));
		fr->vals   -= wt;
		to->d[dec=class_val(example(set,index))] += wt;
		fr->d[dec] -= wt;
	} else {
		to->vals   ++;
		fr->vals   --;
		to->d[dec=class_val(example(set,index))] ++;
		fr->d[dec] --;
	}
	if ( fr->vals < min_set_split ||
		 to->vals < min_set_split )
		set_flag(tbl.flags,nullsplit);
	else
		unset_flag(tbl.flags,nullsplit);
}

/*******************************************************************/
/*
 *	tmod_count(set) -- modify counting table by moving counts in v
 *			   from partition f to t
 * 
 */
void
tmod_count(v,f,t)
int	v,f,t;
{
	int	i;	/* Scratch variables */
	float	vals;
	fent	*src = tbl.known+v+2,
		*fr = tbl.known+f,
		*to = tbl.known+t;
	fordecs(i) {
		fr->d[i] -= vals = src->d[i];
		to->d[i] += vals;
	}
	fr->vals -= src->vals;
	to->vals += src->vals;
	if ( fr->vals < min_set_split ||
		 to->vals < min_set_split )
		set_flag(tbl.flags,nullsplit);
	else
		unset_flag(tbl.flags,nullsplit);
}

/*******************************************************************/
/*
 *	tprint() -- print counting table.
 */
tprint()
{
	register int	i, j;	/* Scratch variables */

	fprintf(stdrep, "Value\ttotal ");
	fordecs(j)
		fprintf(stdrep, " %3d", j);

	if ( tbl.tester ) for(i=0; i<tbl.nv; i++) {
		fprintf(stdrep, "\n");
		display_result(tbl.tester,i,stdrep);
		fprintf(stdrep," %d\t%3g", i, (float) tbl.known[i].vals);
		fordecs(j)
			fprintf(stdrep, " %3g", (float) tbl.known[i].d[j]);
	} else {
		fprintf(stdrep, "\n");
		fprintf(stdrep,"class\t%3g", (float) tbl.tot_known);
		fordecs(j)
			fprintf(stdrep, " %3g", (float) tbl.known[j].d[j]);
	}
	if (  tbl.unknown.vals ) {
		fprintf(stdrep, "\n?\t%g", (float) tbl.unknown.vals);
		fordecs(j)
			fprintf(stdrep, " %3g", (float) tbl.unknown.d[j]);
		fprintf(stdrep, "\ntotal\t%3g", (float) tbl.tot_known);
	}
	fprintf(stdrep, "\n");
}


/*******************************************************************/
/*
 *	talloc(tbl) -- make space for "tbl"
 */
void
talloc(tbl, nv)
ftbl	*tbl;
int	nv;
{
	int	i;
	tbl->unknown.d = mems_alloc(float,ndecs);
	if ( tbl->known = mems_alloc(fent,nv) ) {
	    tbl->known[0].d = mems_alloc(float, nv * ndecs);
	    for (i = 1; i < nv; i++)
	    {
		tbl->known[i].d = tbl->known[0].d + ndecs * i;
	    }
	}
	if ( nv > BITSETMAX )
		tbl->maskarray = new_bitarray(nv);
	else
		tbl->maskarray = 0;
	null_flags(tbl->flags);
	tbl->maskset = EMPTY_SET;
}



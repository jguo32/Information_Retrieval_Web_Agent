/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* print -- tree printer.    
 *
 *	Original format by David Harper
 *
 *	Major reorganise and additions
 *		1989, 1991 - Wray Buntine
 *	Altered to print decision graphs Jon Oliver August 1992
 */
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


static  int	print_depth = 10000;
static  int	print_line_no;
static	ot_ptr	t_root;
static  tprint_flags    flags;
static	char	*pprec	= "%.4g";
static	float   *dd=0, *dd2=0;
static	_print_tree();
static	_print_test();

/************************************************************************/
/*
 *	print_tree(t, flags, depth)	--  print the decision tree
 */
print_tree(t, flgs, depth)
ot_ptr	t;
tprint_flags	flgs;
int	depth;
{
	int	tab;

	t_root = t;
	print_depth = depth;
	flags = flgs;
	if ( flag_set(flags,counts) ) unset_flag(flags,prob);
	if ( flag_set(flags,counts) ) unset_flag(flags,stddev);
	if ( flag_set(flags,stddev )  ) set_flag(flags,prob);
	if ( flag_set(flags,probt) && !dd ) {
	  dd = (float *) salloc(ndecs*sizeof(float));
	  dd2 = (float *) salloc(ndecs*sizeof(float));
	  if (!dd2)  error("insufficient memory in print_tree","");
	}
	tab = 0;
        if (decision_graph_flag) {
	        set_flag(flags,lineno);
                zero_line_no_tree(t);
                unset_transit_flag(t);
        }
	if ( flag_set(flags,lineno) ) {
          t->line_no = 1;
          print_line_no = 2;
	  fprintf(stdrep,"%3d: ",t->line_no);	
	}
	_print_tree(t, tab);
        if (decision_graph_flag) {
                unlock_transit_flag();
        }
	fprintf(stdrep,"\n");
	if ( dd ) {
	  sfree(dd);
	  if ( dd2 )
		sfree(dd2);
	}
}

static void
unset_transit(t)
ot_ptr  t;
{
	unset_flag(t->tflags,transit);
}
static void
set_transit(t)
ot_ptr  t;
{
	set_flag(t->tflags,transit);
}

/************************************************************************/
/*
 *	print_parents(t, flags)	--  print the parents of the node t
 */
print_parents(t, flgs)
ot_ptr	t;
tprint_flags	flgs;
{
	int	tab;

	flags = flgs;
	if ( flag_set(flags,counts) ) unset_flag(flags,prob);
	if ( flag_set(flags,counts) ) unset_flag(flags,stddev);
	if ( flag_set(flags,stddev )  ) set_flag(flags,prob);
	if ( !flag_set(flags,prob )  ) unset_flag(flags,probt);
	tab = 0;
        if (decision_graph_flag) {
	        set_flag(flags,lineno);
/*
                alloc_line_no_tree();
*/
        }
        traverse_parents(t,set_transit);
	if ( flag_set(flags,lineno) )
	  fprintf(stdrep,"%3d: ",t->line_no);	
	set_flag(flags,untransit);
	_print_tree(t, tab);
        traverse_parents(t,unset_transit);
	fprintf(stdrep,"\n");
}

static
_print_tree(t, tab)
ot_ptr	t;
int	tab;
{
	int	i,j;
	int	nv;
	bt_ptr	option;

        if (previous_transit(t) ) {
                fprintf(stdrep,"GOTO %d", t->line_no);
        } else if ( tleaf(t) ) {
		print_node(t);
	} else if ( !ttest(t)) {
		print_node(t);
		fprintf(stdrep,"(ungrown node)");
	} else if ( tab >= print_depth ) {
		print_node(t);
		fprintf(stdrep," (non_leaf)");
	} else if ( flag_set(flags,visit) && tvisited(t) ) {
		print_node(t);
		fprintf(stdrep," (visited)");
	} else if ( flag_set(flags,untransit) && !ttransit(t) ) {
		print_node(t);
		fprintf(stdrep," (non-transit)");
	} else {
		if (flag_set(flags,intnl))
			print_node(t);
		if ( toptions(t) ) for (option=t->option.s.o[i=0];
					option;   option =
					(++i<t->option.s.c)?t->option.s.o[i]:0 ) {
		    fprintf(stdrep,"\n");
		    for (j=tab; j > 0; j--)
			fprintf(stdrep,"|   ");
		    fprintf(stdrep,"Option:");	
		    if (flag_set(flags,gain)) 
			fprintf(stdrep," (SP%g N%g + dN%g)", 
				option->gain, option->np.nodew, option->nprob);
		    if (flag_set(flags,tprob) ) 
		        fprintf(stdrep," (N%g)", option->np.nprop);	
		    if (flag_set(flags,pprob)) 
		 	fprintf(stdrep," (SP%lg)", t->xtra.pn.sprob);	
		    nv = outcomes(option->test);
		    for (j = 0; j < nv; j++)
			_print_test(option, tab, j);
		} else {
		    option = t->option.o;
		    nv = outcomes(option->test);
		    if ( flag_set(flags,ccxtra) ) 
			fprintf(stdrep," (N%g S%g g%g G%g e%g->%d)",
				t->xtra.cc->N, t->xtra.cc->S, t->xtra.cc->g,
				t->xtra.cc->G, t->xtra.cc->err, 
				(int)t->xtra.cc->best );
		    if (flag_set(flags,gain)) 
			fprintf(stdrep," (SP%g N%g + dN%g)", 
				option->gain, option->np.nodew, option->nprob);
		    for (i = 0; i < nv; i++)
			_print_test(option, tab, i);
		}
	}
}

static
_print_test(t, tab, val)
register bt_ptr	t;
register int	tab;
register int	val;
{
	int	i;
	if ( flag_set(flags,lineno) ) {
          fprintf(stdrep,"\n%3d: ",  print_line_no);
          if ( t->branches[val]->line_no == 0 )
                t->branches[val]->line_no = print_line_no;
          print_line_no ++;
	} else {
	  fprintf(stdrep,"\n");
	}
	for (i=tab; i > 0; i--)
		fprintf(stdrep,"|   ");
	display_outcome(t->test,val,stdrep);

	if ( t->branches[val] && t->branches[val]->tot_count ) {
		_print_tree(t->branches[val], ++tab);
	} else if ( !t->branches[val] ) {
		fprintf(stdrep," (ungrown node)");
	} else {
	        print_node(t->branches[val]);
	}
}


static  void
node_class_par(t)
ot_ptr   t;
{
  /*
   *     code to calc. expected prob and prob^2
   *     taken from _tree_class_updt() and _tree_class2_updt()
   */
       int   i;
       if ( ! t->tot_count ) {
	   if ( smoothflag ) {
	        fordecs(i) {
                  dd[i] +=  t->lprob * palpha(0.0,i)/palphatot(0.0);
		  if ( flag_set(flags,stddev) )  
		     dd2[i] += t->lprob * palpha(0.0,i) / palphatot(0.0) *
                              (palpha(0.0,i) + (palphatot(0.0)-palpha(0.0,i))/
                                (palphatot(0.0)+1.0)) / palphatot(0.0);
	        }
	        return;
	   }
	   if ( Zflg || no_parents(t) )
                t = t_root;
           else
                t = oparent(t);      
       } 
       if ( t->lprob )  fordecs(i) {
         dd[i] += t->lprob*t->eg_count[i];
	 if ( flag_set(flags,stddev) ) {
                dd2[i] += t->lprob * t->eg_count[i] *
                  (t->eg_count[i] + (1.0-t->eg_count[i])/
                       (t->tot_count+palphatot(t->tot_count)+1.0));
	 }    
       }
}

/* 
 *     print details about the counts, class, etc.;
 *     strategy for getting node probabilities
 *     taken from _tree_class_updt*() and _tree_class2_updt*()
 */
print_node(t)
ot_ptr	t;  
{
	double	sqrt();
	int	i;
	ot_ptr	t1;    /*   to get probabilities, use t1->eg_count from here */
	if ( !t->tot_count )
	  if ( Zflg || no_parents(t) ) 
	      t1 = t_root;
	  else
	      t1 = oparent(t);
	else
	  t1 = t;
	if (flag_set(flags,recnt)) {
		fprintf(stdrep, "(test)" );
	    	for (i = 0; i < ndecs; i++)
			fprintf(stdrep,"+%g", t->xtra.re_count[i]);
		if (flag_set(flags,counts) || flag_set(flags,prob))
		    	fprintf(stdrep, " (tree)" );
	}
	if (flag_set(flags,reerr)) {
		fprintf(stdrep, "(test costs)" );
		fprintf(stdrep,"%lg ", t->xtra.errs);
	}
	if (flag_set(flags,counts)) {
	    if ( t->eg_count )
	      for (i = 0; i < ndecs; i++)
			fprintf(stdrep,"+%g", t->eg_count[i]);
	    else 
	      fprintf(stdrep," node empty");
	}
	if ( flag_set(flags,probt) && tleaf(t) ) {
	    /*
	     *	  assume eg_count has been set to probs already
	     */
	      fordecs(i) {
		dd[i] = 0.0;
	        dd2[i] = 0.0;
	      }
	      traverse_parents(t, node_class_par);
	      if ( flag_set(flags,prob)) {
		for (i = 0; i < ndecs; i++) {
		  putc(' ',stdrep);
		  fprintf(stdrep,pprec, dd[i] );	
		  if ( flag_set(flags,stddev) ) {
		    fprintf(stdrep,"~");
		    fprintf(stdrep,pprec, sqrt(dd2[i] - dd[i]*dd[i]) );
		  }
		}
	      }
	} else if (flag_set(flags,prob)) {
	    /*
	     *	  assume eg_count has been set to probs already
	     */
	  if ( !t->tot_count && smoothflag ) {
	    for (i = 0; i < ndecs; i++) {
	      putc(' ',stdrep);
	      fprintf(stdrep,pprec, palpha(0.0,i)/palphatot(0.0) );	
	      if ( flag_set(flags,stddev) ) {
		fprintf(stdrep,"~");
		fprintf(stdrep,pprec, palpha(0.0,i)/palphatot(0.0) /
			sqrt(palphatot(0.0)+1.0) );	
	      }
	    }  
	  } else {
	    if ( ! t1->eg_count )
	      fprintf(stdrep," ?");
	    for (i = 0; i < ndecs; i++) {
	      putc(' ',stdrep);
	      fprintf(stdrep,pprec, t1->eg_count[i] );	
	      if ( flag_set(flags,stddev) ) {
		fprintf(stdrep,"~");
		fprintf(stdrep,pprec, t1->eg_count[i]/
			sqrt(t1->tot_count+palphatot(t1->tot_count)+1.0) ); 
	      }
	    }
	  }
	}
	if (flag_set(flags,decis)) {
	  if ( flag_set(flags,probt) && tleaf(t) ) {
	    fprintf(stdrep," %s", find(decattr, best_dec(dd)));
	  } else {
	    if ( ! t1->eg_count )
	      fprintf(stdrep," ?");
	    else
	      fprintf(stdrep," %s", find(decattr, best_dec(t1->eg_count)));
	  }
	}
	if (flag_set(flags,gain) && !ttest(t)) 
		fprintf(stdrep," (L%g)", t->lprob );
	if (flag_set(flags,pprob) ) 
		fprintf(stdrep," (L%lg P%lg)",
			t->xtra.pn.lprob, t->xtra.pn.sprob);	
	if (flag_set(flags,tprob) ) 
		fprintf(stdrep," (L%g)", t->lprob );	
	if ( flag_set(flags,ccxtra) ) 
		fprintf(stdrep," (e%g)", t->xtra.cc->err );
	if ( !t->tot_count && ( flag_set(flags,decis) || flag_set(flags,prob)) ) {
	  if ( smoothflag ) {
	       fprintf(stdrep," (from prior)");
	  } else if ( Zflg) {
	       fprintf(stdrep," (from root)");
	  } else {
	       fprintf(stdrep," (from parent)");
	  }
	}
}

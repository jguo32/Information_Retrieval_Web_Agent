/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h> 
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"
#include "TABLE.h"

/* #define DEBUG_SUBSET  */

bool	t_flg = TRUE;		/* transduction criterion */
bool    Fflg = FALSE;	        /* make choice of test totally random */

static float rand_cut();
static unord_gain();
static subset_gain();

extern	ftbl	tbl;		/* counting table */
extern  int	depth;
/*
 *	a ghastly disarray of option parameters
 */
        char	purflg = 'i';		/* purity measure */
static  unsigned char  allsub;
static	double	pre_prob_prune = -FLOATMAX*0.9; 
                                        /* stopping criteria for gain */
extern  float	errstds;		/* stdvs for error pruning   */
extern  float   min_set_split;         /*  see usage in table.c  */
extern  int     min_set_size;         /* don`t split on sets <=  */
static  int     max_cuts=1;          /*  max. number of cuts saved for attr. */
                                     /*  currently can only be set to 1  */
static  float   gain_cutoff;
static  int     subsample_size = 1000;  /*  subsample if more than this egs  */

/*
 *	storage for gain calculations
 */
bt_ptr	*treeoptions=0;         /*   bt_rec assoc. with the test  */
float   *gainoptions=0;         /*   if purflg='r', stores info gain for test */
int	gmi;		     /*   current free array entry  */
int	max_gmi;	     /*   max free array entry  */
int     *bestoptions=0;     /*   best "max_opts" tests so far in "treeoptions" */
static  int  max_opts;
static  double  max_opt_fact;
ot_rec  dummyt;		     /*   place for "calc_log_prob" */

/*
 *	process options for "choose"
 */
choose_opts(c,optarg)
int	c;
char	*optarg;
{
        extern  char   *strchr();
	switch (c)
	{
        case 'S':
		if ( !strcmp(optarg,SB_ONE_STR) ) 
			allsub=SB_ONE;
		else if ( !strcmp(optarg,SB_FULL_STR) ) 
			allsub = SB_FULL;
		else if ( !strcmp(optarg,SB_REST_STR) ) 
			allsub = SB_REST;
		else {
			uerror("incorrect argument to S flag","");
			break;
		}
		break;
#ifdef MULTI_CUTS
        case 'Y':
		max_cuts = 4;
		if ( sscanf(optarg," %d", &max_cuts) <= 0 )
            	  uerror("incorrect argument for option 'Y'","");
		if ( !t_flg )
		  uerror("multiple cuts only allowed Bayesian splitting","");
		break;
#endif
	case 'i':		/*   splitting rule  */
		t_flg = 0;
		if ( sscanf(optarg," %c", &purflg) <= 0 ||
		  ! strchr("itrg",purflg) )
            	  uerror("incorrect purity flag for option 'i'","");
		if ( max_cuts > 1 )
		  uerror("multiple cuts only allowed Bayesian splitting","");
		break;
	case 't':		/*	bayes split  */
		t_flg=1;
		break;
	case 'n':
		if ( sscanf(optarg," %lf", &pre_prob_prune) <= 0 )
            	  uerror("incorrect argument for option 'n'","");
		break;
	case 'G':		 /* subsample if more than this many egs.  */
		if ( sscanf(optarg," %d", &subsample_size) <= 0 )
                  uerror("incorrect argument for option 'G'","");
		break;
	case 'F':
		Fflg++;
		rand_random();
		break;
	default:
 		error("incorrect option for choose","");
	}
}

/*
 *	display choose options
 */
display_choose()
{
	fprintf(stdrep,"SPLITTING RULE OPTIONS:\n");
        fprintf(stdrep,"splitting using ");
	if ( Fflg )
		fprintf(stdrep,"random assignment;\n");
	if ( t_flg )
		fprintf(stdrep,"Bayesian rule;\n");
	else switch ( purflg ) {
	    	case 'i' :
			fprintf(stdrep,"information gain;\n");
			break;
	    	case 't' :
			fprintf(stdrep,"twoing;\n");
			break;
	    	case 'g' :
			fprintf(stdrep,"Gini criterion;\n");
			break;
	    	case 'r' :
			fprintf(stdrep,"information gain ratio;\n");
			break;
	}
	fprintf(stdrep,"for nodes with more than %g counts,\n  subsample down",
			(float)subsample_size*1.2 );
	fprintf(stdrep," to approximately %d counts;\n",subsample_size);
	if ( t_flg ) {
	    fprintf(stdrep,"don't choose a test if (its probability)/");
	    fprintf(stdrep,"(leaf probability) < %g;\n", pre_prob_prune);
	} else 
	    fprintf(stdrep,"don't choose a test if its gain is < %g;\n",
			pre_prob_prune);
	if ( max_cuts > 1 ) {
		fprintf(stdrep,"allow up to %d choices on cut point for",
				max_cuts);
		fprintf(stdrep," each continuous attribute;\n");
	}
	switch ( allsub ) {
	case SB_OFF :
		break;
	case SB_ONE :
		fprintf(stdrep,"test all multi-discrete attrs by equality");
		fprintf(stdrep," with a single value;\n");
		break;
	case SB_FULL :
	 	fprintf(stdrep,"test all multi-discrete attrs by binary subsetting;\n");
		break;
	case SB_REST :
	 	fprintf(stdrep,"test all multi-discrete attrs by splitting");
		fprintf(stdrep," with a remainder branch;\n");
		break;
	}
	fprintf(stdrep,"\n");
}


/****************************************************************************/
/*
 *	init_gainstore() -- initialize the storage spots for gain
 */
void
init_gainstore()
{
  int  i, j;
  void   (*saved_salc_error)();
  extern void   (*salc_error)();
  test_rec   *tester;

  if ( t_flg && pre_prob_prune> 0.0 )
    pre_prob_prune = log((double)pre_prob_prune);

  /*
   *      set allocator to abort if runs out of memory
   */
  saved_salc_error = salc_error;
  salc_error = 0;
  max_gmi = 1;
  /*
   *	first, count out how many test storage spots are needed
   *    (storage spots are set up initially for every possible test)
   */
  if ( max_cuts > max_opts ) 
    max_cuts = max_opts;
  foraattrs(i) {
    if ( num_type(i) ) 
      max_gmi += max_cuts;
    else 
	max_gmi ++;
  }
  treeoptions = mems_alloc(bt_ptr, max_gmi+2);
  bestoptions = mems_alloc(int, max_gmi+2);
  if ( purflg=='r' )
    gainoptions = mems_alloc(float, max_gmi+2);
  dummyt.parents = mem_alloc(bt_ptr);
  dummyt.num_parents = 1;
  dummyt.eg_count = mems_alloc(float,ndecs);
  /*
   *	then initialise bt_rec storage spots
   */
  for ( gmi=1 ; gmi < max_gmi; gmi++)
    {
      treeoptions[gmi]  = (bt_ptr) salloc(sizeof(bt_rec));
      null_flags(treeoptions[gmi]->tflags);
      treeoptions[gmi]->parent = (ot_ptr)0;
      treeoptions[gmi]->nprob = 0;
      treeoptions[gmi]->np.nprop = 0.0;
      treeoptions[gmi]->branches = (ot_ptr *)0;
      treeoptions[gmi]->test =
	(test_rec *)salloc(sizeof(test_rec));
    }
#ifdef DEBUG_dealloc
  record_alloc(0,0,"init_gainstore()",0);
#endif
  /*
   *	then initialise test storage spots in the bt_recs
   */
  if ( allsub ) foraattrs(i) 
      if ( type(i)==DISCRETE && unordvals(i) > 2 )
	     unordsubsets(i) = allsub;
  gmi=1;	
  nvalstest = 2;
  foraattrs(i) {
    if ( num_type(i) ) {
      for (j=0; j<max_cuts; j++) {
	tester = treeoptions[gmi++]->test;
	tester->attr = i;
	null_flags(tester->tsflags);
	set_flag(tester->tsflags,cut);
	tester->no = 2;
	tester->tval.cut = -FLOATMAX;
      }
    } else {
      if ( unordsubsets(i) ) {
	if ( t_flg )
		init_int_log(unordvals(i));
	tester = treeoptions[gmi++]->test;
	tester->attr = i;
	tester->no = 2;
        null_flags(tester->tsflags);
        if ( unordvals(i) > SYMSETMAX ) {
        	tester->tval.valbigset = new_bitarray(unordvals(i));
		set_flag(tester->tsflags,bigsubset);
	} else {
		set_flag(tester->tsflags,subset);
        	tester->tval.valset = EMPTY_SET;
	}
	if ( unordsubsets(i)==SB_REST ) {
		set_flag(tester->tsflags,remdr);
		tester->no = unordvals(i);
	}
      } else {
	tester = treeoptions[gmi++]->test;
        tester->tval.valset = 0;
	tester->attr = i;
	tester->no = unordvals(i);
        null_flags(tester->tsflags);
        set_flag(tester->tsflags,attr);
      }
      if ( tester->no > nvalstest)
	nvalstest = tester->no;
    }	
  }
  salc_error = saved_salc_error;
}

/****************************************************************************/
/*
 *	free_gainstore() -- free the storage spots for gain
 */
free_gainstore()
{
  int   gmi;
  if ( treeoptions ) {
	for ( gmi=1 ; gmi < max_gmi; gmi++)
      		sfree(treeoptions[gmi]);
  	sfree(treeoptions);
  	sfree(bestoptions);
  	if ( purflg=='r')
	  sfree(gainoptions);
   }
}

/****************************************************************************/
/*
 *	install_options(t,try) -- test in "try" at node "t"
 *                         is installed in "bestoptions" array
 */
void
install_options(t, gmi)
ot_ptr  t;
int  gmi;           /*   gmi = index to treeoptions[]   */
{
    int j;
    int	swap, last;
    /*
     *   find gmi's position in current list of best
     */
    for (j=max_opts-1; 
	 j>=0 && (!bestoptions[j] ||
		  treeoptions[bestoptions[j]]->gain
		  < treeoptions[gmi]->gain) ; j--) ;
    /*  
     *    worse than current items in bestoptions[] so return
     */
    if ( ++j == max_opts )
      return;
    /*
     *   "j" is where to place the new gmi
     *   check the pre-pruning stopping conditions
     */
    if ( t_flg )  {
        if ( treeoptions[gmi]->gain
                < t->xtra.gr.gain + pre_prob_prune  ||
	    bestoptions[0] && treeoptions[gmi]->gain < 
	              treeoptions[bestoptions[0]]->gain + max_opt_fact )
          return;
    } else {
	if ( treeoptions[gmi]->gain < pre_prob_prune )
	  return;
    }
    if ( !j ) {
      /*
       *   this new guy is now the best (index 0) ...
       *   remove any options not with factor of current best
       */
      for (j=0 ;  j<max_opts && bestoptions[j];  j++) 
	  if ( treeoptions[gmi]->gain + max_opt_fact
	                >= treeoptions[bestoptions[j]]->gain )
	    break;
      for ( ;  bestoptions[j] && j<max_opts; j++)
	bestoptions[j] = 0;
      j = 0;
    }
    /*
     *    install gmi in its new place, and ripple the rest
     */
    last = gmi;
    for (; bestoptions[j] && j<max_opts; j++) {
      swap = bestoptions[j];
      bestoptions[j] = last;
      last = swap;
    }
    if ( j<max_opts ) 
      bestoptions[j] = last;
}


/****************************************************************************/
/*
 *	fix_gainratio()   -   adjust gains to agree with gain ratio
 *                    
 */
void
fix_gainratio(t)
ot_ptr   t;
{
    int  i, cnt=0;
    float  ave=0.0;
    
    if ( t->xtra.gr.gain == 0.0 ) return;

    /*     
     *    first find the average of the gains
     */
    for ( i=1 ; i < max_gmi; i++)
      if ( treeoptions[i]->gain > -FLOATMAX*0.5 ) {
	ave += gainoptions[i];
	cnt++;
      } 
    if ( cnt < 1 ) return;
    if ( cnt > 1 ) ave /= cnt;
    /*      
     *      now compute bestoptions[]
     */
    bestoptions[0] = 0;
    for ( i=1 ; i < max_gmi; i++)
      if (  bestoptions[0] ) {
        if ( treeoptions[i]->gain > treeoptions[bestoptions[0]]->gain
	    && gainoptions[i] >= ave-EPSILON )
	  bestoptions[0] = i;
      } else {
	if ( treeoptions[i]->gain >  -FLOATMAX*0.5
	    && gainoptions[i] >= ave-EPSILON )
	  bestoptions[0] = i;
      }
    ASSERT( treeoptions[bestoptions[0]]->gain > -FLOATMAX*0.5 )
}

/****************************************************************************/
/*
 *	evaluate(t,try) -- evaluate quality of placing test in "try" at node
 *                         "t",  answer essentially appears in t->xtra.gr.gain
 */
void
evaluate_test(t,itry,testing)
ot_ptr  t;		/*  node being checked */
int     itry;		/*  index to treeoptions  */
egtesttype  testing;	/*  context  */
{
  bt_ptr     try = treeoptions[itry];
  test_rec   *tester = try->test;
  
  try->gain = -FLOATMAX;
  dummyt.testing = testing;
  if (!cannot_test(tester,testing)) {
    if ( t_flg ) {
    	try->parent = t;
	try->np.nodew = node_weight(testing,try);
    }
    if (Fflg) {
      /*
       *       do random assignment of test and gain
       */
      try->gain = frandom();
      if (cut_test(tester)) {
	  tester->tval.cut = rand_cut(t->xtra.gr.egs, (int)tester->attr);
      } else if (set_test(tester)) {
	  tester->tval.valset = 
		set_inter(UNIV_SET(unordvals(tester->attr)),random());
      } else if (subset_test(tester)) {
	    if ( unordsubsets(tester->attr)==SB_ONE ) {
	      tester->tval.valset = SINGLETON(random()%unordvals(tester->attr));
	    } else
	      tester->tval.valset = 
		set_inter(UNIV_SET(unordvals(tester->attr)),random());
      } else if (bigsubset_test(tester)) {
	    if ( unordsubsets(tester->attr)==SB_ONE ) {
	      empty_bitarray(tester->tval.valbigset);
	      set_bitarray(tester->tval.valbigset,
				(random()%unordvals(tester->attr)));
	    } else
	      random_bitarray(tester->tval.valbigset);
      } 
    } else if (cut_test(tester)) {
      if (t_flg) 
	ord_log_tprob(t, itry);
      else
	ord_info(t, itry);
      gmi += max_cuts - 1;
    } else if (set_test(tester)) {
	;
    } else if (subsets_test(tester)) {
      if ( remdr_test(tester) ) 
        remdr_gain(t, itry);
      else
        subset_gain(t, itry);
    } else {
	unord_gain(t, itry);
    }
  }
}
 

/****************************************************************************/
/*
 *	choose(t) -- reviews all tests to extend node t and returns 
 *		potential subtrees
 *		assumes "t" has eg_count + tot_count + xtra.gr.gain set
 */
int
choose(t,m_opts,m_opt_fact, testing)
ot_ptr  t;
double	m_opt_fact;
int     m_opts;
egtesttype  testing;
{
	int	i,j;
	static	unsigned setup = 1;
	egset   *saveegs;
	float   *savecounts, savetot;

	max_opts = m_opts;
	max_opt_fact = m_opt_fact;
	if ( setup ) {
	  init_gainstore();
	  setup = 0;
	}
	for ( i=0; i<=max_opts; i++)
	  bestoptions[i] = 0;
	/*
	 *   arrange subsampling if needed
         */
	if ( t->tot_count > subsample_size*1.2 ) {
	  saveegs = t->xtra.gr.egs;
	  /*
	   *     take sufficiently large sample so that its "tot_count"
	   *	 will be approximately subsample_size
           *    NB.   with NO unknowns, this will be exact
	   */
	  if ( ! ( t->xtra.gr.egs = 
		  random_subset(t->xtra.gr.egs, 
		     (int)(subsample_size*setsize(t->xtra.gr.egs)/ t->tot_count),
		     t->tot_count)) ) {
	    bestoptions[0] = 0;
	    return 0;
	  }
	  savecounts = t->eg_count;
	  savetot = t->tot_count;
          if ( !(t->eg_count = cal_d_vec(t->xtra.gr.egs))) {
	    bestoptions[0] = 0;
	    return 0;
	  }
	  t->tot_count = 0.0;
	  for (i=0; i<ndecs; i++)
                t->tot_count += t->eg_count[i];	  
	} else {
	  scale_prob(0);
	  saveegs = 0;
	}
	/*
	 *	calc. leaf gain for t 
	 */
	if (t->xtra.gr.gain <= -FLOATMAX*0.99) 
	    if ( t_flg ) {
		t->lprob = t->xtra.gr.gain = leaf_prob(t,testing);
	    } else {
	    	calc_leaf_gain(t, purflg);
		ttotdec(t->eg_count);
	    }
	/*
	 *	For each test, compute the gain of the data
	 *	given the split resulting from that attribute,
         *      and then save the best few.
	 */
	gain_cutoff = t->xtra.gr.gain + pre_prob_prune*t->tot_count;
	for(i=1; i< max_gmi; ) {
	      evaluate_test(t, i, testing);
	      if (cut_test(treeoptions[i]->test))   {
		  for (j=0; j<max_cuts; j++) {
		    if ( treeoptions[i]->gain > -FLOATMAX*0.9 )
		      install_options(t, i+j);
		  }
		  i += max_cuts;
	      } else {
		if ( treeoptions[i]->gain > -FLOATMAX*0.9 )
		  install_options(t, i);
		i++;
	      }
	}
	if ( purflg=='r' && bestoptions[0] )
	  fix_gainratio(t);
        if ( saveegs ) {
	  /*
	   *       clean up after subsampling
	   */
	  /* 
	   *   don't free_set(t->xtra.gr.egs) so random_subset can recycle
	   */	  
	  t->xtra.gr.egs = saveegs;
	  sfree(t->eg_count);
	  t->eg_count = savecounts;
	  t->tot_count = savetot;
	  scale_prob(0);
	  if ( t_flg ) {
	    /*
	     *   set ->lprob to be correct, and adjust the other code lengths
	     *   to be correct relatively
	     */
	    t->xtra.gr.gain = leaf_prob(t,testing);
	    for (j=1 ;  j<max_gmi ;  j++)  
	      if ( treeoptions[j]->gain > -FLOATMAX*0.9 )
		treeoptions[j]->gain += t->xtra.gr.gain - t->lprob;
	    t->lprob = t->xtra.gr.gain;
	  }
	}
	return bestoptions[0];
}

/****************************************************************************/
/*
 *	unord_gain(t, bt) -- calc. the gain of the
 *		data given the split for node t specified in bt
 */
static unord_gain(t, itry)
ot_ptr	t;
int     itry;
{
        bt_ptr	bt = treeoptions[itry];
	tmake(t->xtra.gr.egs, bt->test);
	if ( tnullsplit() )
	    /*
	     *    discourage splits that don't separate the data
	     */
	    bt->gain = -FLOATMAX;
	else {
	    if ( t_flg ) {
	        calc_log_prob(itry);
	    } else {
	        calc_gain(t,itry,purflg);
	    }
	}
}

/****************************************************************************/
/*
 *	subset_gain(set, bt) -- calc. the gain of the
 *		data for taking subset
 */
static subset_gain(t, itry)
ot_ptr   t;
int      itry;
{
        bt_ptr	        bt = treeoptions[itry];
	static bitarray	abigset[2];	/*   set of values placed in set */
	bitset		aset[2];	/*   set of values to try in set */
	int		i, best_i, ith, nv;
	int             failed;
	int		outfield,infield;  /*  index aset and abigset */
	float		prob;
	float		best_gain, best_diff;
	bool		bigset = bigsubset_test(bt->test);
	
        /* 
	 *   Make the initial table, 
	 *   for subsetting, first two entries are subsets,
	 *   the rest are the usual counts so "i+2" not "i"
	 */
	if ( bigset ) {
        	abigset[1] = bt->test->tval.valbigset;
		empty_bitarray(abigset[1]);
	} else {
        	bt->test->tval.valset = EMPTY_SET;
		aset[1] = EMPTY_SET;
	}
	tmake(t->xtra.gr.egs, bt->test);
	bt->gain = -FLOATMAX;
	nv = unordvals(bt->test->attr);
	if ( bigset ) {
	    if ( ! abigset[0] ) {
		abigset[0] = new_bitarray(nvalsattr);
	    }
	    resize_bitarray( abigset[0], nv );
	    univ_bitarray( abigset[0], nv );
	} else {
	    aset[0] = UNIV_SET(nv);
	}
	prob = 0.0;
	best_gain = -FLOATMAX;
	/*
	 *	ignore outcomes with no counts
	 */
	for(i=nv-1; i>=0;  i--)
		if ( !nvalue(i+2) ) {
		  if ( bigset )
			unset_bitarray(abigset[0],i);
		  else
			unset_bit(aset[0],i);
		  nv--;
		}
	if ( nv<=1 )
	  return;
	/*
	 *	simple O(nv^2) greedy search to find local best
	 */
	nv = unordvals(bt->test->attr);
	outfield = 0;
	infield = 1;
	failed = 0;
	ith = 0;
	while ( failed<=1 ) {
	    if ( t_flg ) {
		if ( infield ) {
			ith++;
			prob += int_log[ith]-int_log[nv-ith+1];  
		} else {
			ith--;
			prob -= int_log[ith]-int_log[nv-ith+1];  
		}
	    } else 
		if ( infield ) {
			ith++;
		} else {
			ith--;
		}      
	    best_i = -1;
	    for ( i=nv-1; i>=0; i-- ) {
		if ( bigset ) {
			if ( !bitarray_set(abigset[outfield],i)  )
				continue;
		} else {
			if ( !bit_set(aset[outfield],i)  )
				continue;
		}
		tmod_count(i,outfield,infield);
		if ( tnullsplit() )
		  continue;
	        if ( t_flg ) {
	    	        calc_log_prob(itry);
			bt->gain += (bt->nprob = prob);
		        if ( best_gain < bt->gain ) {
		            best_gain = bt->gain;
		            best_diff = bt->nprob;
		            best_i = i;
		        }
	        } else {
	                calc_gain(t, itry,purflg);
		        if ( best_gain < bt->gain ) {
		            best_gain = bt->gain;
		            best_i = i;
		        }
		}
#ifdef DEBUG_SUBSET
		if ( !bigset ) 
			  bt->test->tval.valset = aset[1];
		fprintf(stdrep, "subsetting ");
		display_test(bt->test,stdrep);
		fprintf(stdrep, " with %s, %o, gain = %g, nprob = %g (%lg)\n",
			unordstrs(bt->test->attr)[i],
			aset[1]||(1<<i), 
			bt->gain, bt->nprob, exp((double)-bt->nprob) );
#endif
		tmod_count(i,infield,outfield);
	      }
	    if ( best_i >=0 ) {
		if ( bigset ) {
			set_bitarray(abigset[infield],best_i);
			unset_bitarray(abigset[outfield],best_i);
		} else {
			set_bit(aset[infield],best_i);
			unset_bit(aset[outfield],best_i);
		}
		tmod_count(best_i,outfield,infield);
		failed = 0;
	    } else {
		/*
		 *	failed in one direction so try other
		 */
		infield = outfield;
		if ( infield ) {
		  if ( t_flg )
		    prob += int_log[ith]-int_log[nv-ith+1];	
		  outfield = 0;
		} else {
		  if ( t_flg )
		    prob -= int_log[ith]-int_log[nv-ith+1];	
		  outfield = 1;
		}
		failed ++;
		/*
		 *         locally best set is size one, so quit here
		 */
		if ( ith <= 2 )  break;
	    }
	    if ( ith==1 && unordsubsets(bt->test->attr)==SB_ONE ) break;
	}
	bt->gain = best_gain;
	if ( t_flg ) {
		bt->nprob = best_diff;
	}
	if ( !bigset ) {
	  bt->test->tval.valset = aset[1];
	}
#ifdef DEBUG_SUBSET
		fprintf(stdrep, "subsetting chosen ");
		display_test(bt->test,stdrep);
		fprintf(stdrep, ", %o, gain = %g, nprob = %g\n",
			abigset[1], bt->gain, bt->nprob );
#endif
}

/****************************************************************************/
/*
 *	remdr_gain(set, bt) -- calc. the gain of the
 *		data for taking subset
 */
remdr_gain(t, itry)
ot_ptr   t;
int      itry;
{
     error("no remainders implemented!","");
}


/**********************************************************************
 *
 *           stuff for handling the cut point graph to gnuplot
 */

extern  bool    do_graphs;      /* flag that controls graph plotting */
static  char* cg_name;
static  FILE  *cgf;
static  int   cg_index;
init_gain_graph()
{
  cg_name = tempnam(".","INDtt");
  cgf = efopen(cg_name,"w");
  cg_index = 0;
}

#define  update_gain_graph(pt,y)      \
  for ( ; cg_index<=pt ; cg_index++)  fprintf(cgf, "%d %f\n", cg_index, y); 

fin_gain_graph(set,itry,attr)
egset   *set;
int     itry, attr;
{
  bt_ptr	*bt = treeoptions + itry;
  int   i;
  float   gscale = - FLOATMAX;
  for (i=0; i<max_cuts; i++, bt++) {
    if ( (*bt)->gain < -FLOATMAX*0.9 )
      break;
    if ( gscale < (*bt)->gain )
      gscale = (*bt)->gain;
  }
  fprintf(cgf, "%d %f\n", 1, gscale*1.1);
  fclose(cgf);
  if ( i ) {
    call_gnuplot(cg_name,name(attr));
  }
}
  

/****************************************************************************/
/*
 *	ord_log_tprob(set, bt) -- compute the point to split
 *	the set on and return the log probability of the data given the split
 *   NB.  should add MDL-style cuts, as in old cut_list.c
 */
ord_log_tprob(t, itry)
     ot_ptr      t;
     int         itry;
{
  bt_ptr	bt = treeoptions[itry];
  egset	*set = t->xtra.gr.egs;
  int	attr = bt->test->attr, size=setsize(set), index;
  float         this_val, last_val;
#ifdef MULTI_CUTS
  typedef struct  Cut_stuff Cut stuff;
  static struct  Cut_stuff      /*   details about current slice of cuts */
    {
      int   next, prev;        /*   index to next slice  */
      int   max_i;       /*   eg_index  for max. value in slice  */
      int   start_i, end_i;  /*   eg_index  for start, end value in slice  */
      double  max_logp;   /*   logp at max_i, scaled  */
      double  ave_logp;   /*   ave. logp for slice, scaled  */
      }  ;
  Cut_stuff   *cuts=0;
  int     next_c;
  if ( !cuts ) {
    cuts = mems_alloc(Cut_stuff, max_cuts+1);
  }
  if ( max_cuts > 1 ) {
    for ( next_c=0; next_c <= max_cuts; next_c++ ) {
      cuts[next_c].next = next_c + 1;
      cuts[next_c].prev = next_c - 1;
      cuts[next_c].ave_logp = -FLOATMAX;
    }
    cuts[max_cuts].next = -1;
  } else {
#else
      int   max_i;       /*   eg_index  for max. value  */
      int   cnt=0;
      double  max_logp;   /*   logp at max_i, scaled  */
      double  ave_logp;   /*   ave. logp for slice, scaled  */
#endif  
  
  if ( do_graphs) init_gain_graph();
  ave_logp = -FLOATMAX;
  /*
   *    initialize the set by sorting etc.
   */
  qsort_set(set,attr);
  bt->test->tval.cut = -FLOATMAX;
  tmake(set,bt->test);
  ASSERT( tnullsplit() )
  bt->gain = -FLOATMAX;
  if ( ! ntot() || size<=min_set_split ) {
    bt->test->tval.cut = -FLOATMAX;
    return;
  }
  max_logp = -FLOATMAX;
  /*
   *     when looping ignore the stuff at the ends
   */  
  for ( index = 0 ; index < size-min_set_split; 
        index++) {
      if ( (this_val=ord_val(example(set,index),attr))==FDKNOW ) {
	/*   
         *    unknowns kept up the top of array, so finished
	 */
	break;
      }
      if ( tnullsplit() ) {
	/*
	 *    ignore
	 */
	;
      } else if ( this_val==last_val) {
	/*
	 *    skip over repeat values
	 */
	if ( bt->gain > -FLOATMAX*0.9 ) {
	  ave_logp = logsum(ave_logp,(double)bt->gain);
	  cnt++;
	}
      } else {
	calc_log_prob(itry);
	if ( do_graphs) update_gain_graph(index,bt->gain);
	/*
	 *    otherwise compute max. on the fly
	 */
	if ( bt->gain > -FLOATMAX*0.9 ) {
	  ave_logp = logsum(ave_logp,(double)bt->gain);
	  cnt++;
	}
	if ( bt->gain > max_logp ) {
	  max_logp = bt->gain;
	  max_i = index;
	}
      }
      last_val = this_val;
      tmod_one(set,index);
  }
  /*
   *   install found cut point in treeoptions
   */
  if ( max_logp > -FLOATMAX*0.9 && cnt ) {
      bt->gain = ave_logp - log((double)cnt);
      bt->nprob = bt->gain - max_logp;
      if ( bt->nprob > 0.0 )
	bt->gain = -FLOATMAX;
      else
	bt->test->tval.cut = (ord_val(example(set,max_i),attr)
	                    + ord_val(example(set,max_i-1),attr)) / 2.0;
  } else {
    bt->gain = -FLOATMAX;
  }
  if ( do_graphs) fin_gain_graph(set, itry, attr);
}


/****************************************************************************/
/*
 *	ord_info(t, bt) -- compute the point to split the set on
 *	and return the amount of information still needed to classify the set
 *	after making the split.
 */
ord_info(t, itry)
ot_ptr	t;
int     itry;
{
  bt_ptr	bt = treeoptions[itry];
  egset *set = t->xtra.gr.egs;
  int attr = bt->test->attr;
  float this_val, last_val, best_gain = -FLOATMAX, best_val, ave_gain;
  int   i, index, size=setsize(set), last_index, cnt;
  int     this_min_set_split;
  static  float  *ratio_vals = 0;
  static  float  *gain_vals = 0;
  static  int    gain_size=0;
  
  ASSERT( max_cuts == 1 );

  if ( purflg=='r' ) {
    /*
     *     setup split-info storage  
     */
    cnt = 0;
    ave_gain = 0.0;
    if ( !gain_vals ) {
      gain_vals = mems_alloc(float,size);
      ratio_vals = mems_alloc(float,size);
      gain_size = size;
    } else if ( gain_size < size ) {
      ratio_vals = mems_realloc(ratio_vals,float,size);
      gain_vals = mems_realloc(gain_vals,float,size);
      gain_size = size;
    }
  }
  if ( do_graphs) init_gain_graph();

  /*
   *    initialize the set by sorting etc.
   */
  bt->test->tval.cut = -FLOATMAX;
  qsort_set(set,attr);
  tmake(set,bt->test);
  if ( ! ntot() || size<=min_set_split ) {
      bt->gain = -FLOATMAX;
      return;
  }
  /*
   *     Quinlan's rule for ignoring end cuts
   */
  this_min_set_split = ntot_kn()*0.1/(ndecs+1.0);
  if ( this_min_set_split < min_set_split )
    this_min_set_split = min_set_split;
  else
    if ( this_min_set_split > 25 )
      this_min_set_split = 25;
  bt->gain = -FLOATMAX;
  /*
   *     when looping ignore the stuff at the ends
   */  
  for ( index = 0 ; index < size-this_min_set_split; index++) {
      if ( (this_val=ord_val(example(set,index),attr))==FDKNOW ) {
	/*   
         *    unknowns kept up the top of array, so finished
	 */
	break;
      }
      if ( tnullsplit() ) {
	/*
	 *   ignore these
	 */
	if (  purflg=='r' ) 
	  ratio_vals[index] = -FLOATMAX;	
      } else if ( this_val==last_val) {
	/*
	 *    skip over repeat values
	 */
	if (  purflg=='r' ) {
	  ratio_vals[index] = -FLOATMAX;
	  if ( bt->gain > -FLOATMAX*0.9 ) {
	    ave_gain += gainoptions[itry];
	    cnt++;
	  }
	}
      } else {
	/*
	 *     valid split point to try
	 */
	calc_gain(t,itry,purflg);
	if ( do_graphs) update_gain_graph(index,bt->gain);
	if ( purflg=='r' ) {
	  /*
	   *    for 'r', just store values for now, will max. later
	   */
	  gain_vals[index] = gainoptions[itry];
	  if ( (ratio_vals[index] = bt->gain) >  -FLOATMAX*0.9 ) {
	    ave_gain += gain_vals[index];
	    cnt ++;
	  } 
	} else {
	  /*
	   *    otherwise compute max. on the fly
	   */
	  if ( bt->gain > best_gain ) {
	    /*
	     *      chosen cut-point is the mid-point of the two vals
	     */
	    best_val = (this_val+last_val)/2.0;
	    best_gain = bt->gain;
	  }
	}
      }
      last_val = this_val;
      tmod_one(set,index);
  }
  if ( purflg=='r' ) {
    if ( cnt ) {
      size = index;           /*   save last index checked */
      ave_gain /= cnt;
      best_gain = -FLOATMAX*0.9;
      last_index = 0;
      for ( index=this_min_set_split; index<size; index++ ) {
        if ( ratio_vals[index] > best_gain &&
	     gain_vals[index] >= ave_gain - EPSILON ) {
	  last_index = index;
	  best_gain = ratio_vals[index];
	}
      }
      if ( !last_index )
	bt->gain = -FLOATMAX;
      else {
	bt->test->tval.cut = (ord_val(example(set,last_index),attr)+
		  ord_val(example(set,last_index-1),attr)) / 2.0;
	bt->gain = best_gain;
	gainoptions[itry] = gain_vals[last_index];
      }
    } else
      bt->gain = -FLOATMAX;
  } else {
    if ( best_gain < -FLOATMAX*0.9 ) {
      bt->gain = -FLOATMAX;
    } else {
      bt->gain = best_gain;
      bt->test->tval.cut = best_val;
    }
  }
  if ( do_graphs ) fin_gain_graph(set, itry, attr);
}

/***************************************************************************/
/*
 *      rand_cut(set, attr) -- find random cut
 */
static float
rand_cut(set, attr)
register egset   *set;
int   attr;
{
        int  cut_index;
        int  cut_size = setsize(set);
        /*
         *      sort the set and hide the unknowns
         *      from view (up the top)
         *      by adjusting the indices
         */
        qsort_set(set, attr);
        for (cut_index = cut_size-1; cut_index >=0; cut_index--)
            if ( ord_val(set->members[cut_index], attr) != FDKNOW )
                        break;
        if ( cut_index ) {
                cut_size = (cut_index-0.00000001)*frandom();
                return (ord_val(set->members[cut_size],attr) + 
			ord_val(set->members[cut_size+1],attr))*0.5;
        } else {
                return ord_val(set->members[0],attr);
        }
}

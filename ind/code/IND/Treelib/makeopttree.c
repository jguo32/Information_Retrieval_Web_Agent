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

extern	bool	t_flg;			/* transduction splitting flag */
extern	int	depth;
extern	ot_ptr	tree;
extern  int	timeout;
extern  int	intint;
extern  int	verbose;
extern int	depth_bound; 
#ifdef GRAPH
extern int	Stochastic_Growth;
#endif

extern  void *rem_heap();

extern	bt_ptr	*treeoptions;
extern	int	max_gmi;
extern  int     *bestoptions;
static  int _extendtree();
static int _makeopttree();
static int _maketree();
static float _update_gain();
extern void makeopttree();
static int startlookaheadtree();
static int _lookaheadtree();
static egtesttype   testing;

/*   #define  RECORD_GAINS  */
#ifdef RECORD_GAINS
static FILE      *gainf;
#define  GAIN_FILE  "/cop/home/copernicus2/wray/Data/gains.out"
#endif 

/*  #define DEBUG_LOOK  */
/*  #define DEBUG_MAKE  */
/* #define DEBUG_MAKE2 */

#ifdef DEBUG_MAKE
static void
report_node(t)
ot_ptr  t;
{
 double        others, cw, lw, tw;
 bt_ptr        option;
  bt_ptr  best;
  float   best_sprob;
 tprint_flags  pflags;
 int   j;
    /*
     *      find best options
     */
    if ( verbose <1 )
      return;
    if ( tleaf(t) ) {
        printf("no options in this node\n");
        return;
    }
    best_sprob = -FLOATMAX;
    best = (bt_ptr)0;
    foroptions(option,j,t) {
      if ( option->gain >= best_sprob ) {
        best = option;
        best_sprob = option->gain;
      }
    }
    /*      show stats on rel. weights */
    others = 0.0;
    lw = FLOATMAX;
    cw = exp( (double) (t->lprob - best->gain) );
    foroptions(option,j,t) {
      tw = exp((double)(option->gain - best->gain));
      if ( tw < lw )
        lw = tw;
      if ( option != best )
        others += tw;
      printf("Test %d : recalc max log. prob. = %lg ; ",
                   j, btree_submaxw(option,testing,1) );
      printf("recalc sub log. prob. = %lg\n",
             btree_submaxw(option,testing,0) );
      printf("Relative probability (abs. log. prob.) of test #%d ",j);
      display_test(option->test,stdout);
      printf(" = %6lg (%g)\n", tw, option->gain );
    }
    printf("Relative probability of non-choice branches = %lg\n", others);
    printf("Relative probability of worst branch = %lg\n", lw);
    printf("Recalc subtree max. log. prob. = %lg , ", tree_submaxw(t,testing,1) );
    printf("recalc subtree sub log. prob. = %lg \n ", tree_submaxw(t,testing,0) );
    printf("Sum log probability of subtree = %lg \n", t->xtra.gr.gain );
    printf("Relative probability (abs. log. prob.) of leaf = %lg (%g) \n",
           cw, t->lprob);
    if ( verbose >1 ) {
      printf("Tree so far: ");
      null_flags(pflags);
      set_flag(pflags,intnl);
      set_flag(pflags,counts);
      set_flag(pflags,gain);
      set_flag(pflags,decis);
      print_tree(t,pflags,depth+2);
    }
}
#endif

/*
 *	A GHASTLY DISARRAY OF OPTION PARAMETERS
 *       NB.  all "factors" are stored as "- log" form, see makeopttree();
 *       Critics note that these parameters control the extent of search
 *       but not the objective/goal of search.
 */
bool	oflg = FALSE;		/* Override flag */
bool    jtree_flag=FALSE;       /* if set, use makeopttree()   */
bool    incg_flag=TRUE;       /* if set, do incremental growing from heap  */
int	min_set_size = 1;	/* stop growing if set at node < */
float	min_set_split = 0.5;	/* stop growing if sets at split < */
bool    max_not_ave = TRUE;    /* lookahead smoothes rather than maximises */
#define  combine(a,b)  (max_not_ave? Max(a,b) : logsum((double)a,(double)b))
/*
 *       these options effect how the tree is initially extended
 *       during the initial choice of options to place at a node
 */
static	double	choice_fact= 0.00001; /* include tests within factor of best */
static  int     choice_breadth = 1;       /* include the best few tests  */
/*
 *       these options effect how the tree is initially extended
 *       during the initial beam-lookahead (after choice)
 */
static	double	look_fact= 0.00001;   /* include tests within factor of best */
	int     look_breadth = 1;  /* include the best few tests  */
int     look_depth = 1;    /* go to this depth max. */
/*
 *       these options effect how the tree is grown
 *       after the beam-lookahead stage
 *       NB.  some default initialisation is in makeopt_opts(c)
 *       NB.  all factors are stored as "- log" form, see makeopt_opts(c)
 */
static  int     grow_breadth = 1;      
static	double	grow_fact = 0.005; 
/*
 *      used with "grow_fact", currently grow those options
 *      after the beam-lookahead stage which are within "incg_fact"
 *      of the best option, otherwise if within "grow_fact" of the 
 *      best option, put on heap and grow later
 */
static	double	incg_fact = 0.9;
/*
 *      these modulate a fancy version of early stopping
 */
static	double	grow_add_fact = 0.75;
static	double	grow_leaf_fact = 0.00001;
/*
 *      post prune an option if its posterior prob. (->xtra.gr.gain)
 *      is not within a factor of the best option and the leaf
 */
static	double	post_fact = 0.005;
static  int     post_breadth = 1;      

/*
 *	various storage
 */
int	max_dep;
static ot_ptr   root;
static  double  this_sprob; 
static  int    bestj;

/*
 *	for stopping rule,
 *	return true if all but one classes have < min
 */
bool
near_same_class(decs,min)
float    *decs, min;
{
	int	i, some=0;
	fordecs(i) 
		if ( decs[i] < min )
			some++;
	if ( some >= ndecs-1 )
		return TRUE;
	else
		return FALSE;
}

/*
 *	process options for "maketree"
 */
make_opts(c,optarg)
int	c;
char	*optarg;
{
	switch (c)
	{
	case 'O':
		max_not_ave = FALSE;
		break;
	case 'o':
		oflg++;
		break;
	case 's':
		sscanf( optarg,"%d,%f", &min_set_size, &min_set_split );
		if ( 2*min_set_split >= min_set_size )
			min_set_size = 2*min_set_split;
		break;
	case  'J':
		jtree_flag++;
		sscanf( optarg,"%d,%lf,%lf,%lf",
			&grow_breadth, 
		       &grow_fact, &grow_add_fact, &grow_leaf_fact );
		if ( choice_breadth < grow_breadth )
		  choice_breadth = grow_breadth;
		break;
	case  'K':
		jtree_flag++;
		sscanf( optarg,"%d,%lf",
			&post_breadth, &post_fact );
		if ( choice_breadth < post_breadth )
		    choice_breadth = post_breadth;
		break;
	case  'L':
		sscanf( optarg,"%lf", &incg_fact );
		if ( !incg_fact ) {
			incg_flag = FALSE;
			incg_fact = -FLOATMAX;
		}
		break;
	case 'B':
		jtree_flag++;		
		sscanf(optarg,"%d, %d, %lf, %d, %lf", &look_depth,
		       &look_breadth, &look_fact,
		       &choice_breadth, &choice_fact ) ;
		if ( choice_breadth < look_breadth )
		  choice_breadth = look_breadth;
		break;
	case 'R':	
		uerror("Randomization option not operational.\n","");
		break;
	default:
 		error("incorrect option for make","");
	}
}

display_make()
{
  fprintf(stdrep,"GROWING OPTIONS:\n");
  fprintf(stdrep,"don't split a node that is pure or greater than ");
  fprintf(stdrep,"depth %d;\n", depth_bound);
  fprintf(stdrep,"don't split a node with < %d counts;\n", 
	  min_set_size);
  fprintf(stdrep,"don't make a cut test with < %d counts;\n", 
	  min_set_size);
  if (jtree_flag) {
    fprintf(stdrep,"LOOKAHEAD OPTIONS:\n");
    fprintf(stdrep,"on original choice of tests at depth 0,\n  allow");
    fprintf(stdrep," upto %d options within factor %lg of best;\n",
	    choice_breadth, choice_fact );
    fprintf(stdrep,"lookahead (beam-search) all tests at node to depth %d,\n",
	    look_depth);
    fprintf(stdrep,"  and only explore upto %d tests with probability within %lg of the best;\n",
	    look_breadth, look_fact);
  }
  if ( jtree_flag && grow_breadth>1 ) {
    fprintf(stdrep,"POST-LOOKAHEAD GROWING OPTIONS:\n");
    fprintf(stdrep,"grow upto %d optional subtrees after lookahead;\n",
	    grow_breadth);
    fprintf(stdrep,"only grow those with lookahead probability within factor %lg of the best,\n",
	    grow_fact);
    fprintf(stdrep,"  and within factor %lg of the leaf;\n", 
	    grow_leaf_fact);
    if ( incg_flag ) {
      fprintf(stdrep,"but those with lookahead probability not within factor %lg of the best\n",
	    incg_fact);
      fprintf(stdrep,"  are placed on a heap and grown if space/time permits;\n");
    }
    fprintf(stdrep,"stop growing if the best subtree is within a factor");
    fprintf(stdrep," %lg of the total subtrees;\n", 
	    grow_add_fact);
  }
  if ( jtree_flag && post_breadth>1 ) {
    fprintf(stdrep,"POST-GROWING OPTIONS:\n");
    fprintf(stdrep,"keep around upto %d optional subtrees at each node;\n",
	    post_breadth);
    fprintf(stdrep,"and only those within factor %lg of the best;\n",
	    post_fact);
  }
  fprintf(stdrep,"\n");
}

/*
 *          called by memory allocator (salloc()) when
 *          runs out of memory (as set up in makeopttree());
 *          chops everything down so the system stops growing
 */
void
mktree_mem()
{
	min_set_size = 100000;
	choice_breadth = 0;
	look_breadth = 0;
	look_depth = 0;
	depth_bound = 0;
	max_dep = 0;
	grow_breadth = 0;
	if ( incg_flag )
	  while ( rem_heap() ) ;
	return;
}

/*
 * return set of indices for best "breadth" options within factor of the best  
 */
static bitset
best_fact_breadth(t,fact,breadth)
ot_ptr  t;
int     breadth;
double   fact;
{
  bitset  grow_set=EMPTY_SET;
  int j, grown=0, bestj;
  double best_sprob;
  bt_ptr  best=(bt_ptr)0, option;
  
  /*
   *    find the best option
   */
  foroptions(option,j,t) {
    if ( !best || option->gain >= best->gain ) {
      best = option;
      bestj = j;
    }
  }
  set_bit(grow_set,bestj);
  grown++;
  best_sprob =  best->gain;
  while ( grown < breadth ) {
    /*
     *	    find next best option within factor of best_sprob
     */
    this_sprob = best_sprob + fact;
    bestj = -1;
    foroptions(option,j,t) {
      if ( !bit_set(grow_set,j) && option->gain >= this_sprob ) {
	bestj = j;
	this_sprob = option->gain;
      }	
    }
    if ( bestj<0 )
      break;
    set_bit(grow_set,bestj);
    grown++;
  }
  return grow_set;
}

/****************************************************************************/
maketree(t, egs)
ot_ptr	t;
egset	*egs;
{
	t->xtra.gr.egs = egs;
#ifdef GRAPH
	if ( Stochastic_Growth ) {
	  stochastic_grow(t, egs);
	} else  {
#endif
       	  testing = init_test();
          if ( jtree_flag ) {
	  makeopttree(t);
	  } else {
	    add_counts(t);
	    _maketree(t);
	  }
#ifdef GRAPH
	}
#endif
}
 

/****************************************************************************
 *
 *           this routine is the standard greedy growing algorithm
 *           for classic ID3;  the fancy routine is in makeopttree.c
 *
 ****************************************************************************/
static
_maketree(t)
ot_ptr	t;
{
	static  int	choice;
	bool	override;	/* Store oflg for this subtree */
	int	i,j ;
	bt_ptr	option;

	if ( tleaf(t) || depth_bound <= depth  ||
	     t->tot_count < min_set_size ||
	     near_same_class(t->eg_count,(float)(min_set_size/2.0)) ) {
		force_leaf(t);
		return;
	}
	depth++;
	if ( ! (choice=choose(t,1,-0.001,testing)) ) {
		force_leaf(t);
	} else {
            override = oflg; 
    	    if ( oflg ) 
		    if (!interact(t,testing,0)) 
			override = FALSE;  
	    if ( !choice || tleaf(t) )
		    force_leaf(t);
	    else {
	            add_option(t, treeoptions[bestoptions[0]]);
		    /*
		     *	     grow further
		     */
		    if ( !tvisited(t) ) {
			foroptions(option,j,t)
			    if ( !tvisited(option) ) {
		                add_egs(option);
			        foroutcomes(i,option->test) {  
			            add_test(option->test,i,testing);
				    add_counts(option->branches[i]);
			            _maketree(option->branches[i]);
			            rem_test(option->test,i,testing);
			        }
		                release_egs(option);
		            }
		      }
		  }
	    if (oflg && ttest(t))
	      if ( !interact(t,testing,1) )
		override = FALSE;
	    oflg = override;
	}
	depth--;
}


/****************************************************************************
 *
 *     makeopttree(t)
 *         fancy growing routine which does lookahead search,
 *         multiple options etc.
 *         MUST maintain
 *                  t->xtra.gr.gain = log prob. of this subtree and all
 *                                   its pruned versions
 *                  option->gain = same above for bt_rec node
 *                  t->lprob      = log. prob. for this node being a leaf
 *         in order to control growing
 *
 ***************************************************************************/
void
  makeopttree(t)
ot_ptr	t;
{
  extern void	(*salc_error)();
  extern void	(*tmt_error)();
  int    i;
  bt_ptr   option;
  static   bool   factors_not_adj = TRUE;
#ifdef DEBUG_MAKE
  float  f ;
#endif
#ifdef RECORD_GAINS
  float   save_gain;
  gainf = fopen(GAIN_FILE,"a+");
#endif
  if ( factors_not_adj ) {
    /*
     *       adjust all factors because we work in log-prob space
     */
    if ( post_fact>0.0 ) post_fact = log((double)post_fact);
    if ( grow_add_fact>0.0 ) grow_add_fact  = log((double)grow_add_fact);
    if ( grow_leaf_fact>0.0 ) 
      grow_leaf_fact = log((double)grow_leaf_fact);
    if ( grow_fact>0.0 ) grow_fact = log((double)grow_fact);
    if ( incg_fact>0.0 ) incg_fact = log((double)incg_fact);
    if ( look_fact>0.0 ) look_fact = log((double)look_fact);
    if ( choice_fact>0.0 ) choice_fact = log((double)choice_fact);
    factors_not_adj = FALSE;
  }
  /*
   *       do the work
   */
  tmt_error = salc_error = mktree_mem;
  root = t;
  depth = 0;
  if ( !add_counts(t) ) {
    t->lprob = leaf_prob(t,testing);
    if ( !_extendtree(t) ) 
      if ( !_makeopttree(t) && depth_bound > 0 )
	while ( option = (bt_ptr)rem_heap() ) {
	  tree_test(option->parent,testing);
	  if ( oflg ) 
	    if (!interact(option->parent,testing,6)) 
	      oflg = FALSE;  
#ifdef RECORD_GAINS 
	  save_gain = option->gain;
#endif
	  option->gain = option->np.nodew + option->nprob;
	  foroutcomes(i,option->test) {
	    add_test(option->test,i,testing);
	    depth++;
	    if ( _makeopttree(option->branches[i]) ) {
	      /*
	       *      on abort, empty the heap and quit 
	       */
	      while ( rem_heap() ) ;
	      _update_gain(t); 
	    }
	    depth--;
	    rem_test(option->test,i,testing);
	    option->gain += 
	      option->branches[i]->xtra.gr.gain;
	  }
#ifdef RECORD_GAINS 
	    fprintf(gainf,"%d %f %f %f\n", negs, 
		option->parent->tot_count, save_gain, option->gain );
#endif
	  _update_parent_gain(option);
	  null_test(testing);
#ifdef DEBUG_MAKE
	  printf("just redid subtree (%g):  ",option->parent->tot_count);
	  printf("node tree prob = %g, recalc tree max. log. prob. = %lg \n", 
		 t->xtra.gr.gain, tree_submaxw(t,testing,max_not_ave) );
#endif
	}
    depth = 0;
#ifdef DEBUG_MAKE
    f = t->xtra.gr.gain;
#endif
    rec_post_prune(t);
#ifdef DEBUG_MAKE
    if (  f > t->xtra.gr.gain && max_not_ave ) {
      fprintf(stdrep, "WARNING:  t->xtra.gr.gain decreased by pruning\n");
      fprintf(stdrep, "  prev gain = %g, new gain = %g, egs = %g\n",
	      f, t->xtra.gr.gain, t->tot_count );
    }
#endif  
  } else
    force_leaf(t);
#ifdef RECORD_GAINS 
  fclose(gainf);
#endif
  tmt_error = salc_error = 0;
}


/****************************************************************************/
/*
 *          global (to makeopttree.c) variables used in this procedure are:
 *                 testing
 *                 depth
 */
static int
_makeopttree(t)
ot_ptr	t;
{
	int	i, j;
	bt_ptr	option, best_ungrown;
	bool	override;
	bitset  grow_set;
	float   best_gain;
#ifdef DEBUG_MAKE
	float   f;
#endif
#ifdef RECORD_GAINS 
	float   save_gain;
#endif

	if ( tleaf(t) )
	     return 0;
	/*
	 *	lookahead to find quality of options
	 */
 	if ( _extendtree(t) ) return 1;  
	max_dep = depth + look_depth - 1;
	if ( startlookaheadtree(t) ) return 1;
	/*
	 *	process interactive queries
	 */
        override = oflg; 
    	if ( oflg ) 
	    if (!interact(t,testing,2)) 
		override = FALSE;  
	if ( tleaf(t) )
	     return 0;
	depth++;
#ifdef DEBUG_MAKE2
	  printf("STARTING makeopttree (%g):\n",t->tot_count);
	  report_node(t);
#endif
	if ( ttest(t) && options(t)==1 ) {
	  /*
	   *	special case of growing if there is only 1 option
	   */
#ifdef DEBUG_MAKE2
	  printf("MAKING SINGLE NODE (%g):\n",t->tot_count);
	  report_node(t);
#endif
	  if ( t->xtra.gr.gain - t->lprob > grow_leaf_fact ) {
#ifdef RECORD_GAINS 
	    save_gain =  t->xtra.gr.gain;
#endif
	    option = t->option.o;
	    option->gain = option->np.nodew + option->nprob;
	    foroutcomes(i,option->test) {
	      add_test(option->test,i,testing);
	      if ( _makeopttree(option->branches[i]) ) return 1;
	      rem_test(option->test,i,testing);
	      option->gain += option->branches[i]->xtra.gr.gain;
	    }
	    t->xtra.gr.gain = combine(option->gain,t->xtra.gr.gain);
#ifdef RECORD_GAINS 
	    fprintf(gainf,"%d %f %f %f\n", negs, t->tot_count, save_gain,
				t->xtra.gr.gain );
#endif
	  }
#ifdef DEBUG_MAKE2
	  printf("MADE SINGLE NODE (%g):\n",t->tot_count);
	  report_node(t);
#endif
	} else {
	  /*
	   *	find best option
	   */
#ifdef DEBUG_MAKE2
	  printf("MAKING OPTIONS NODE (%g):\n",t->tot_count);
	  report_node(t);
#endif
	  best_ungrown = (bt_ptr)0;
#ifdef DEBUG_MAKE2
	  printf("OPTION GAINS :");
#endif
	  foroptions(option,j,t) {
#ifdef DEBUG_MAKE2
	    printf(" option %d gain %g,", j, option->gain );
#endif
	    if ( !best_ungrown || option->gain >= 
		best_ungrown->gain ) 
	      best_ungrown = option;
	  }
	  best_gain = best_ungrown->gain;
#ifdef DEBUG_MAKE2
	  printf("\n");
#endif
	  /*
	   *      for multiple options:
	   *	grow if one test is significantly better than
	   *	both the leaf, and the comb. of all other tests
	   */
	  if ( !timeout && ttest(t) && 
	      (t->xtra.gr.gain - best_ungrown->gain < grow_add_fact
	       || t->xtra.gr.gain - t->lprob > grow_leaf_fact )  ) {
	    grow_set = best_fact_breadth(t,grow_fact,grow_breadth);
	    /*
	     *      first apply post-pruning to any options that wont be grown
	     */
	    foroptions(option,j,t) 
	      if ( !bit_set(grow_set,j) ) {
	        option->gain = option->np.nodew + option->nprob;
		foroutcomes(i,option->test) {
#ifdef DEBUG_MAKE
		  f = option->branches[i]->xtra.gr.gain;
#endif
		  rec_post_prune(option->branches[i]);
#ifdef DEBUG_MAKE
		  if (  f > option->branches[i]->xtra.gr.gain && max_not_ave ) {
		    fprintf(stdrep, 
			 "WARNING:  branch->xtra.gr.gain decreased by pruning\n");
		    fprintf(stdrep, "  prev gain = %g, new gain = %g, egs = %g\n",
			      f, t->xtra.gr.gain, t->tot_count );
		  }
#endif  
		  option->gain += 
		    option->branches[i]->xtra.gr.gain;
		}
	      }
#ifdef DEBUG_MAKE2
	    printf("POST_PRUNED (%g) non-extended lookahead, about to recurse on %o:\n",
		   t->tot_count, grow_set );
	    report_node(t);
#endif
	    /*
	     *      grow most probable first
	     */
	    while (set_notempty(grow_set)) {
	      /*
	       *      find next best ungrown option
	       */
	      best_ungrown = (bt_ptr)0;
	      foroptions(option,j,t) {
		if ( bit_set(grow_set,j) &&
		    ( !best_ungrown || option->gain >= 
		                      best_ungrown->gain ) ) {
		  bestj = j;
		  best_ungrown = option;
		}
	      }
	      if ( !best_ungrown ) break;
	      unset_bit(grow_set,bestj);
	      if ( best_ungrown->gain - best_gain > incg_fact ) {
		/*
		 *      grow it now
		 */
#ifdef RECORD_GAINS 
	        save_gain =  best_ungrown->gain;
#endif
		best_ungrown->gain = best_ungrown->np.nodew
		  + best_ungrown->nprob;
		foroutcomes(i,best_ungrown->test) {
		  add_test(best_ungrown->test,i,testing);
		  if ( _makeopttree(best_ungrown->branches[i]) ) return 1;
		  rem_test(best_ungrown->test,i,testing);
		  best_ungrown->gain += 
		    best_ungrown->branches[i]->xtra.gr.gain;
		}
#ifdef RECORD_GAINS 
	    fprintf(gainf,"%d %f %f %f\n", negs, t->tot_count, save_gain,
				best_ungrown->gain );
#endif
	      } else {
		/*
		 *    add to heap and possibly grow later
		 */
		add_heap(best_ungrown->gain-best_gain,(void*)best_ungrown);
	      }	      
	    }
	    t->xtra.gr.gain = t->lprob;
	    foroptions(option,j,t) 
	      t->xtra.gr.gain = combine(t->xtra.gr.gain,option->gain);
	  }
	  if ( !incg_flag ) {
	    /*
	     *      post prune if no incremental growing
	     */
#ifdef DEBUG_MAKE2
	    printf("about to POST PRUNE (%g):  ",t->tot_count);
	    printf("node subtree prob = %g, recalc subtree max. log. prob. = %lg \n", 
		   t->xtra.gr.gain, tree_submaxw(t,testing,max_not_ave) );
#endif
#ifdef DEBUG_MAKE
	    f = t->xtra.gr.gain;
#endif
	    rec_post_prune(t);
#ifdef DEBUG_MAKE
	    if (  f > t->xtra.gr.gain  && max_not_ave ) {
	      fprintf(stdrep, 
		      "WARNING:  t->xtra.gr.gain decreased by pruning\n");
	      fprintf(stdrep, "  prev gain = %g, new gain = %g, egs = %g\n",
		      f, t->xtra.gr.gain, t->tot_count );
	    }
#endif  
#ifdef DEBUG_MAKE2
	    printf("just POST PRUNED (%g):  ",t->tot_count);
	    printf("node subtree prob = %g, recalc subtree max. log. prob. = %lg \n", 
		   t->xtra.gr.gain, tree_submaxw(t,testing,max_not_ave) );
#endif
	  }
	}
#ifdef DEBUG_MAKE2
	printf("GROWN and PRUNED, about to quit (%g):  ",t->tot_count);
	report_node(t);
	printf("node subtree nits = %g, recalc subtree max. nits = %lg\n", 
		 t->xtra.gr.gain, tree_submaxw(t,testing,max_not_ave) );
#endif
#ifdef DEBUG_MAKE
	if ( f=fabs((double)t->xtra.gr.gain,tree_submaxw(t,testing,max_not_ave))
		 < 0.001  ) 
	    fprintf(stdrep, "WARNING:  t->xtra.gr.gain, out by %g\n",f);
#endif
	depth--;
    	if ( oflg ) 
	    if (!interact(t,testing,1)) 
		override = FALSE;  
	oflg = override;
	return 0;
}

/*
 *           computation done to correct gain thoughout tree
 */
static float
_update_gain(t)
ot_ptr   t;
{
	int     i,j;
	bt_ptr	option;

	t->xtra.gr.gain = t->lprob;
	if ( ttest(t) ) foroptions(option,j,t) {
	    option->gain = option->np.nodew + option->nprob;
	    foroutcomes(i,option->test) {
	      option->gain += _update_gain(option->branches[i]);
	    }
	    t->xtra.gr.gain = combine(option->gain,t->xtra.gr.gain);
	}
	return t->xtra.gr.gain;
}

/*
 *           having updated node, now adjust parents' gain
 */
_update_parent_gain(option)
bt_ptr   option;
{
  int   i,j;
  ot_ptr   t = option->parent;

  for (;;) {
    t->xtra.gr.gain = t->lprob;
    foroptions(option,j,t) {
      t->xtra.gr.gain = combine(option->gain,t->xtra.gr.gain);
    }
    if ( !(option = first_parent(t)) )
        return;
    option->gain = option->np.nodew + option->nprob;
    foroutcomes(i,option->test) {
      option->gain += option->branches[i]->xtra.gr.gain;
    }
    t = option->parent;
    ASSERT ( t && !decision_graph_flag)
  }
}

/****************************************************************************
 *
 *	post-prune a parent node (but not recursively) 
 *
 */
static int
post_prune(t)
ot_ptr	t;
{
	int     j;
	bt_ptr	option;
	double	best_sprob;
	/*
	 *       post prune those options that are significantly
         *       less than the best or the leaf
	 *          first find best option/leaf, then check the rest
	 *       then delete options until under count
         */
	best_sprob = t->lprob;
	foroptions(option,j,t) {
	  if ( option->gain >= best_sprob ) 
	    best_sprob = option->gain;
	}
	foroptions(option,j,t) {
	  /*
	   *	prune all options not within factor of best
	   */
	  if ( option->gain - best_sprob < post_fact ) {
	    rem_option(t,j);
	    if ( !toptions(t) && tparent(t) ) {
	      if ( t->option.o->gain - best_sprob < post_fact ) 
		rem_option(t,0);
	      break;
	    }
	  }
	}
	while ( tparent(t) && options(t) > post_breadth ) {
	  /*
	   *    remove options until under count
	   */
	  this_sprob = best_sprob;
	  bestj = 0;
	  foroptions(option,j,t) {
	    if ( option->gain <= this_sprob ) {
	      bestj = j;
	      this_sprob = option->gain;
	    }
	  }
	  rem_option(t,bestj);
	}
	t->xtra.gr.gain = t->lprob;
	if ( tparent(t) ) foroptions(option,j,t) 
	      t->xtra.gr.gain = combine(t->xtra.gr.gain,option->gain);
}

/*
 *       recursive version of post_prune()
 */
rec_post_prune(t)
ot_ptr	t;
{
	int     i,j;
	bt_ptr  option;

	if ( tparent(t) ) {
	  foroptions(option,j,t) 
	    foroutcomes(i,option->test) 
	      if ( !tvisited(option->branches[i]) )
		rec_post_prune(option->branches[i]);
	  post_prune(t);
	} else {
	  ASSERT( !ttest(t) )
	  force_leaf(t);
	  free_set(t->xtra.gr.egs);
	  t->xtra.gr.egs = 0;
	  t->xtra.gr.gain = t->lprob;
	}
	set_flag(t->tflags,visited);
}


/****************************************************************************
 *
 *	construct tree during initial lookahead stage
 *	make sure t->xtra.gr.gain is set on exit
 *	when calling startlookaheadtree(), must have already called _extendtree()
 *      lookahead ALL options currently attached to node
 *        (i.e. as set by initial call to _extendtree with choice_breadth, etc.)
 *      but while doing so only do to breadth "look_breadth"
 */
static int
startlookaheadtree(t)
ot_ptr	t;
{
	int	i, j;
	bt_ptr	option;

	depth++;
	/*
	 *	grow to depth and calc. sub weight on way up
	 */
	t->xtra.gr.gain = t->lprob;
#ifdef DEBUG_LOOK
	  printf("LOOKING AHEAD (%g) at all tests, gain = %g, starting:\n",
		 t->tot_count, t->xtra.gr.gain);
	  report_node(t);
#endif
	if ( !timeout && tparent(t) ) {
	  foroptions(option,j,t) {
#ifdef DEBUG_LOOK
	        printf("LOOKING AHEAD (%g): option %d nodew = %g, nprob = %g \n",
		   t->tot_count, j, option->np.nodew , option->nprob );
#endif
	      option->gain = option->np.nodew + option->nprob; 
	      foroutcomes(i,option->test) {
		if ( depth <= max_dep ) {
		  add_test(option->test,i,testing);
		  if ( _extendtree(option->branches[i]) ) return 1;
		  if ( _lookaheadtree(option->branches[i]) ) return 1;
		  rem_test(option->test,i,testing);
		}
		option->gain += option->branches[i]->xtra.gr.gain;
#ifdef DEBUG_LOOK
		if ( verbose > 1 )
	printf("LOOKING AHEAD (%g):  cumm option %d gain = %g, branch %d = %g \n",
	              t->tot_count,  j, option->gain, i, 
	              option->branches[i]->xtra.gr.gain );
#endif
	      }
	      t->xtra.gr.gain = combine(t->xtra.gr.gain,option->gain);
#ifdef DEBUG_LOOK
		printf("LOOKING AHEAD:  cumm gain = %g, option = %g \n",
		   t->xtra.gr.gain, option->gain);
#endif
	      if ( timeout ) break;
	  }
#ifdef DEBUG_LOOK
	  printf("LOOKED AHEAD (%g) at all tests, gain = %g, about to quit:\n",
		 t->tot_count, t->xtra.gr.gain);
	  report_node(t);
#endif
	}
#ifdef DEBUG_LOOK
    	if ( oflg ) 
	       interact(t,testing,3);
#endif
	depth--;
	return 0;
}	

/*
 *	external driver for lookaheadtree
 */
extern int
lookaheadtree(t, ttesting)
ot_ptr  t;
egtesttype ttesting;
{
	testing = ttesting;
	return _lookaheadtree(t);
}

/*
 *         this routine is exactly the same as that above except
 *          that it selectively grows (see usage of look_set)
 */
static int
_lookaheadtree(t)
ot_ptr	t;
{
	int	i, j;
	bt_ptr	option;
	bitset  look_set;

	depth++;
	/*
	 *	grow to depth and calc. sub weight on way up
	 */
	t->xtra.gr.gain = t->lprob;
#ifdef DEBUG_LOOK
	  printf("_LOOKING AHEAD (%g) gain = %g, starting:\n",
		 t->tot_count, t->xtra.gr.gain);
	  report_node(t);
#endif
	if ( !timeout && tparent(t) ) {
	  look_set = best_fact_breadth(t,look_fact,look_breadth);
#ifdef DEBUG_LOOK
	  printf("_LOOKING AHEAD (%g): at tests %o:\n",
		 t->tot_count, look_set);
#endif
	  foroptions(option,j,t) {
#ifdef DEBUG_LOOK
	      printf("_LOOKING AHEAD (%g):  option %d nodew = %g, nprob = %g \n",
		   t->tot_count, j, option->np.nodew , option->nprob );
#endif
	    if ( bit_set(look_set,j) ) {
	      option->gain = option->np.nodew + option->nprob;
	      foroutcomes(i,option->test) {
		if ( depth <= max_dep ) {
		  add_test(option->test,i,testing);
		  if ( _extendtree(option->branches[i]) ) return 1;
		  if ( _lookaheadtree(option->branches[i]) ) return 1;
		  rem_test(option->test,i,testing);
		}
		option->gain += option->branches[i]->xtra.gr.gain;
#ifdef DEBUG_LOOK
	    if ( verbose > 1 )
	   printf("_LOOKING AHEAD (%g):  cumm option %d gain = %g, branch %d = %g \n",
		        t->tot_count, j, option->gain, i,
		        option->branches[i]->xtra.gr.gain );
#endif
	      }
	    }
	    t->xtra.gr.gain = combine(t->xtra.gr.gain,option->gain);
#ifdef DEBUG_LOOK
	      printf("_LOOKING AHEAD (%g):  cumm gain = %g, option %d = %g \n",
		    t->tot_count, t->xtra.gr.gain, i, option->gain);
#endif
	    if ( timeout ) break;
	  }
#ifdef DEBUG_LOOK
	  printf("_LOOKED AHEAD (%g) at tests %o, gain = %g, ",
		 t->tot_count, look_set, t->xtra.gr.gain);
	  printf("recalc. gain = %lg, about to quit:\n",
		 tree_submaxw(t,testing,max_not_ave) );
	  report_node(t);
#endif
#ifdef DEBUG_MAKE
	  if ( f=fabs((double)t->xtra.gr.gain,tree_submaxw(t,testing,max_not_ave))
		 < 0.001  ) 
	    fprintf(stdrep, "WARNING:  t->xtra.gr.gain, out by %g\n",f);
#endif
	}
#ifdef DEBUG_LOOK
    	if ( oflg ) 
	       interact(t,testing,4);
#endif
	depth--;
	return 0;
}	

/*
 *	external driver for _extendtree()
 */
extern int
extendtree(t, ttesting)
ot_ptr  t;
egtesttype ttesting;
{
	testing = ttesting;
	return _extendtree(t);
}

/****************************************************************************
 *
 *	extend tree from leaf to depth 1 option tree
 *	if not leaf, do nothing
 *	use choose to select which optional tests to add
 *      enter with t->lprob and t->eg_count set correctly
 *	exit with t->xtra.gr.gain set correctly
 */
static int
_extendtree(t)
ot_ptr	t;
{
	int	i,j ;
	bt_ptr	option;

	if ( tleaf(t) )
	    return 0;
	if ( tparent(t) )
	        /*
	         *	tree already grown so ignore
	         */
		;
	else if ( timeout || depth_bound <= depth  ||
	     t->tot_count < min_set_size ||
	     near_same_class(t->eg_count,(float)(min_set_size/2.0)) ||
	     ! (choose(t,choice_breadth,choice_fact,testing)) ) {
	        /*
	         *	tree must be forced to a leaf
	         */
		force_leaf(t);
		free_set(t->xtra.gr.egs);
		t->xtra.gr.egs = 0;
		t->xtra.gr.gain = t->lprob;
	} else {
	    depth++;
    	    if ( oflg ) 
		    if (!interact(t,testing,0)) 
			oflg = 0;  
	    /*
	     *    add the best choices
	     */
	    t->xtra.gr.gain = t->lprob;
	    for (j=0; j<choice_breadth && bestoptions[j]; j++) {
	      if ( add_option(t, treeoptions[bestoptions[j]]) ) goto cant_grow;
	    }
	    if ( tparent(t) ) {
	      foroptions(option,j,t) {
	        if ( add_egs(option) )   goto cant_grow;
		option->gain = option->np.nodew + option->nprob;
	        foroutcomes(i,option->test)  {
		  if ( add_counts(option->branches[i]) )  goto cant_grow;
		  add_test(option->test,i,testing);
		  option->gain +=
		    option->branches[i]->xtra.gr.gain =
		      option->branches[i]->lprob = 
			leaf_prob(option->branches[i],testing);
		  rem_test(option->test,i,testing);
		  ASSERT(option->branches[i]->xtra.gr.egs)
		}
	    	t->xtra.gr.gain = combine(t->xtra.gr.gain,option->gain);
	      }
	      free_set(t->xtra.gr.egs);
	      t->xtra.gr.egs = 0;
	      depth--;
	    } else 
	      goto cant_grow;
#ifdef DEBUG_LOOK
    	    if ( oflg ) 
	       interact(t,testing,5);
#endif
	}
	/*
	 *       OK return
	 */
	return 0;
cant_grow:
	/*
	 *       error return, out of memory, so clean up and quit
         *       NB.  ignore half grown tree by setting flags first;
	 *            otherwise force_leaf() tries to clean up and mem-faults
	 */
        unset_flag(t->tflags,test);
        unset_flag(t->tflags,optiont);
	force_leaf(t);
	t->lprob = t->xtra.gr.gain = leaf_prob(t,testing);
	mktree_mem();
	return 1;	
}

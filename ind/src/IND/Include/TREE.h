/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  TREE.h -- header file for all tree programs
 *    
 *	author - Wray Buntine, 1989,1991,1992
 */

#define GRAPH 

/*
 *		tree node flags
 */
typedef  union  tree_flags tree_flags;
union  tree_flags 
{
    struct  {
	unsigned test : 1;		/*  node is a test */
	unsigned leaf : 1;		/*  node is a leaf */
	unsigned emptt : 1;		/*  node has been forced empty */
	unsigned visited : 1;		/*  node has been visited */
	unsigned optiont : 1;		/*  node has optiont */
	unsigned transit : 1;		/*  ??? ask jono  */
	unsigned lock : 1;		/*  ??? ask jono  */
	unsigned transit2 : 1;          /*  ??? ask jono  */
	unsigned tree_section : 1;      /*  ??? ask jono  */
	unsigned transit3 : 1;          /*  ??? ask jono  */
	unsigned lprobset : 1;          /*  ??? ask jono  */
	unsigned overgrown : 1;         /*  ??? ask jono  */
    } b;
    unsigned short i;
};

#define	tleaf(T)		flag_set((T)->tflags,leaf)
#define	ttest(T)		flag_set((T)->tflags,test)
#define	tempty(T)		flag_set((T)->tflags,emptt)
#define	tvisited(T)		flag_set((T)->tflags,visited)
#define	toptions(T)		flag_set((T)->tflags,optiont)
#define	ttransit(T)		flag_set((T)->tflags,transit)
#define	tlock(T)		flag_set((T)->tflags,lock)
#define ttransit2(T)            flag_set((T)->tflags,transit2)
#define ttree_section(T)        flag_set((T)->tflags,tree_section)
#define ttransit3(T)            flag_set((T)->tflags,transit3)
#define tlprobset(T)            flag_set((T)->tflags,lprobset)
#define tovergrown(T)           flag_set((T)->tflags,overgrown)
#define	tparent(T)		((T)->option.o || toptions(T))

#define bad_tflags(T)	( tempty(T) || ( tleaf(T) && (ttest(T)||toptions(T)) ) )

/*
 *		tree header flags
 */
typedef union  thead_flags thead_flags;
union  thead_flags 
{
    struct {
	unsigned probs : 1;		/*  has probs set, not counts */
	unsigned bayes : 1;		/*  wants or has bayes pruning */
	unsigned optiont : 1;		/*  has options */
	unsigned random : 1;		/*  generation randomized */
	unsigned graph : 1;		/*  not a tree but a graph */
    } b;
    unsigned char i;
};

#define	thprobs(T)		flag_set((T)->hflags,probs)
#define	thbayes(T)		flag_set((T)->hflags,bayes)
#define	thoptions(T)		flag_set((T)->hflags,optiont)
#define	thrandom(T)		flag_set((T)->hflags,random)
#define	thgraph(T)		flag_set((T)->hflags,graph)

/* 
 *		print_tree flags  
 */
typedef union  tprint_flags tprint_flags;
union  tprint_flags 
{
    struct {
	unsigned counts : 1;		/*  display counts  */
	unsigned decis : 1;		/*  display decision  */
	unsigned prob : 1;		/*  display probabilities */
	unsigned gain : 1;		/*  display gain  */
	unsigned intnl : 1;		/*  print internal node goodies as well  */
	unsigned recnt : 1;		/* "xtra" is re_counts - print 'em too */
	unsigned tprob : 1;		/*  display smoothing prob vals in node */
	unsigned pprob : 1;		/*  display (b-prune) prob vals in node */
	unsigned reerr : 1;		/*  "xtra" is re_errora - print 'em too */
	unsigned visit : 1;		/*   treat visited nodes as leafs  */
	unsigned ccxtra : 1;		/*  print stuff in xtra.cc */	
	unsigned stddev : 1;		/*  display std. dev.s as well */
	unsigned probt : 1;		/*  display for averaged tree  */
	unsigned lineno : 1;		/*  display line no. for nodes  */
	unsigned untransit : 1;		/*  only print nodes w/ transit flag */
    } b;
    unsigned int i;
};

/*
 *              tree prior flags, used in "prior.c"
 */
typedef union  prior_flags prior_flags;
union  prior_flags
{
    struct {
        unsigned local_normal : 1;     /*  normalize weights at each node  */
        unsigned wallace_weight : 1;   /*  if not, then bushy prior */
        unsigned attr_choice : 1;      /*  test prob / #choices */
        unsigned node_prop : 1;        /*  multiply palphaval by node freq.  */
        unsigned alpha_nonsym : 1;     /*  non-symmetric palpha  */
        unsigned alpha_classes : 1;     /*  dont divide alpha by the number of classes  */
        unsigned search_p_join : 1;    /*  search for p_join for dgraphs  */
    } b;
    unsigned char i;
};

/* Special Constants */
#define EPSILON	1e-6

typedef	struct	ot_rec	ot_rec;	/* type for options record */
typedef		ot_rec	*ot_ptr;
typedef	struct	bt_rec	bt_rec;	/* type for branch record */
typedef		bt_rec	*bt_ptr;
typedef	struct	t_head	t_head;	/* type for tree header */
typedef	struct	t_prior	t_prior; /* type for tree prior info. */
typedef struct  CC_rec   CC_rec;  /* type for CC pruning storage */
typedef union  xtras   xtras;  /* extra junk at end of record */


struct CC_rec    /* type for storing details of CV pruning, see CART p294 */
{
        float  N;               /* subtree complexity */
	float  S;		/* subtree cost  */
	float  C;		/* subtree cost  */
	float	 G;		/*    best CC ratio  */
	unsigned char  best;	/*    index for finding best  */
	unsigned char  class;	/*    index to locally best decision  */
	float	g;		/*  CC ratio for this node  */
	float	err;		/*  errors at this node  */
};

union xtras {
      struct    	/* for storing details of tree growing */
      {
        float  gain;            /*  quality of split or log. prob of node */
	egset	*egs;		/*  for this node  */
      } gr;
      struct            /* type for storing details of tree pruning */
      {
        float  lprob;       /* probab. of leaf node */
        float  sprob;       /* log prob of subnode */
      } pn;
      CC_rec	*cc;
      float     *errs;
      float	*re_count;
} xtra;		/* spare space for use by grow, prune, etc */

struct ot_rec	/* type for tree */
{
	tree_flags	tflags;	/* set of binary flags as listed above */
	float	tot_count; 	/* total count of egs (to save recalculating) */
	float	lprob;	   	/* basic probab. this node will be a leaf */
	float   bestprob;       /* best message length for this node */
	union  bts {
		bt_ptr	o;	/* one test */
		struct  btss {
			bt_ptr	*o;	/* several optional tests */
			short	c;	/*  numb. of options  */
		} s;
	} option;
	float	*eg_count;	/* vector of counts (one for each class) */
	bt_ptr	*parents;	/* reverse link to parents  */
	int	num_parents, line_no;
#ifdef GRAPH
	int	parent_c;
#endif
	egtesttype  testing;
	xtras xtra;		/* spare space for use by grow, prune, etc */
};

struct bt_rec	/* type for tree */
{
	tree_flags	tflags;	/* set of binary flags as listed above */
	float	nprob;	   	/* basic node prior prob for this subtree,
				   i.e.  any log. probs. other than those
					 already in node_weight()  */
	union {
	  float	nprop; 	     /* basic proportion for option (see "reclass.c") */
	  float	nodew; 	     /* basic node weight for log. prob. */
	} np;
	ot_ptr	parent;		/* reverse link to parent  */
	test_rec	*test;	/* details of node branches */
	ot_ptr	*branches;	/* subtrees */
	float   gain;		/* gain (in choose) or log prob. for node  */
};

struct t_prior	/*  prior details about tree, see "prior.c" 
		    and "read/write_chtr.c"  */
{
        float   alphaval;
	prior_flags	prior_flag;
        float   node_weight, leaf_weight, p_join;
	int	depth_bound;
	double  prior_nml;
};

struct t_head	/*  header details about tree  */
{
        int     magicno;
        int     treesize;            /*    number of nodes  */
        int     leafsize;            /*    number of leaves  */
	thead_flags	hflags;      /*    see "tree header flags"     */
	double	sprob;               /*    post.log.prob for tree */
	double	eprob;               /*    post.log.prob for egs */
	double	genprob;             /*    prob. of generat. this random tree
				           (for importance sampling)    */
        t_prior	prior;
};

/*
 *	for "log gamma" calcul. use system routine
 */
#define ourgamma(x)     gamma(x)

/*
 *	define in Treelib
 */
float	leaf_weight();
float   leaf_weight2();
float	Dleaf_weight();
float	D2leaf_weight();
float   join_weight();
float	node_weight();
float	sub_gain();
float	leaf_gain();
float	leaf_util();
float	leaf_prob();
float	log_beta();
float	Dleaf_prob();
float	D2leaf_prob();
float	*subtree_prop();
float	*tree_class();
float	*tree_class_ave();
float	*tree_class2();
float	*tree_class2_ave();
ot_ptr	new_node();
ot_ptr	copy_otree();
bt_ptr	copy_bstump();
int	tree_size();
int	leaf_size();
int	recnt_tree();
float	recnt_val();
int	recost_tree();
float	tree_ave_depth();
float	CC_errprune();
float	mincost_prune();
int	CC_costprune();
float	alpha_min();
float   exp_nodes();
double  tree_avet();
double  tree_submaxw();
double  btree_submaxw();
double  tree_maxt();

extern	bool	Zflg;		/*	method of handling zero-count nodes */
extern	bool	smoothflag;	/*	tree is beging smoothed  */
extern	char	Uflg;		/*	method of handling unknowns */
#ifdef GRAPH
extern  bool    decision_graph_flag;
#else
#define decision_graph_flag 0
#endif


#define  foroptions(op,i,t)   for ( i=0, op = toptions(t)? \
				(t->option.s.o[i=t->option.s.c-1]) :\
				(t->option.o) ;\
			        i>=0; op = (--i>=0)?t->option.s.o[i]:\
				 (bt_ptr)0 )
#define  forparents(par,i,t)  for (i=t->num_parents ; (--i)>=0 && (par=t->parents[i]);  )

#define  forbranches(ot,i,bt)  for (i = outcomes(bt->test)-1 ; \
				i>=0 && (ot=bt->branches[i]); i-- )

#define	options(t)	( toptions(t)?t->option.s.c:1 )
/*
 *	cost of node in terms of errors or loss in scaled utility
 *	(utility scaled so always between 0 and 1)
 */
#define cost_val(t) (utilities ? \
			(( t->tot_count*util_max - \
			   prob_util(t->eg_count,best_dec(t->eg_count)) )\
                                         / (util_max-util_min)) \
                       : (t->tot_count - t->eg_count[best_dec(t->eg_count)]) )
#define cc_cost_val(t) (utilities ? \
			(( t->tot_count*util_max - \
			   prob_util(t->eg_count,(int)t->xtra.cc->class) )\
                                         / (util_max-util_min)) \
                       : (t->tot_count - t->eg_count[(int)t->xtra.cc->class]) )

#ifdef GRAPH
/* #define  DEBUG_GRAPH  */
#endif
#ifdef DEBUG_GRAPH

extern char unset_fname[];
extern int unset_line;

#define unset_transit_flag(t)   _unset_transit_flag(t, __FILE__, __LINE__)
#define traverse_onodes(t, f)   _traverse_onodes(t, f, __FILE__, __LINE__)

#define first_parent(t)  _first_parent(t)
#define no_parents(t)   _no_parents(t)
#define oparent(t)     _oparent(t)
bt_ptr _first_parent();
int  _no_parents();
ot_ptr  _oparent();
#else
#define first_parent(t)    ( t->parents[0] )
#define no_parents(t)       (  !t->parents[0] || ! t->parents[0]->parent )
#define oparent(t)        (t->parents[0]->parent)
#define aparent(t,i)        (t->parents[i]->parent)
#endif		 /*  DEBUG_GRAPH  */

/*
 *	defined in Graph
 */
ot_ptr find_jline_no();

/*
 *	from prior.c
 */
#define  palpha(cnt,dec) ( flag_set(prior_flag,node_prop) ? \
		palphadec[dec]* (cnt+1)/tree->tot_count : palphadec[dec] )
#define  palphatot(cnt) ( flag_set(prior_flag,node_prop) ? \
		palphaval *(cnt+1)/tree->tot_count : palphaval )
extern float	palphaval,*palphadec;
extern prior_flags	prior_flag;
extern  ot_ptr  tree;


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* -- ID3 tree pruner, turns counts tree into class probability tree
 *			with possible pruning
 *
 *	Authors - Wray Buntine,   1989-92
 *	          modified Jon Oliver 1992, added multiple trees and x option
 *
 */
#define MAIN
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


char	*progname;
static	char	*treename;
char	prunename[100];
char	*order;
struct  ot_rec  *tree;    
t_head	thead;

tprint_flags	print_flags;

char	pruner = '0';
char	converter = '0';
bool	Dflg = FALSE;
bool	util_flg = FALSE;	/* utility gain */
int	depth = 0;
extern int	verbose;

int Multiple_trees=0;


char	*usage = "Usage: %s [-vDnbrBd:efV:p:c:A:E:P:lq:o:Ms:] order tree\n";
#define c45_default  0.25

main(argc, argv)
int	argc;
char	*argv[];
{
	int depth_bound = -1;
	int	c;			/* option from argv */
	float stds();
	char	*testfile = 0;
	float   prune_factor=1.0;	/* pruning parameter */
	float   prune_factor1=0.999;	/* pruning parameter */
	float   prune_factor2=0.01;	/* pruning parameter */
	bool	noprun=FALSE;
	int	max_opts = 10;
	long	seed=0;

	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;


	progname = argv[0];
	if (argc < 3)
 		uerror(usage,"");
	while ((c = getopt(argc, argv, "lvSNDnbBd:efV:p:c:A:E:P:lq:o:Mx:rs:")) != EOF)
		switch (c)
		{
                case 's':
                        if ( sscanf(optarg," %ld", &seed) <= 0 )
                         uerror("incorrect option argument (can't read long)",
				"");
			srandom(seed);
                        break;
                case 'v':
                        verbose++;
                        break;
		case 'A':
		case 'P':
			prior_opts(c,optarg);
			break;
		case 'V':
			pruner = c;
			testfile = optarg;
			break;
		case 'l':
			noprun = TRUE;
			break;
		case 'n':
		case 'b':
		case 'r':
			converter = c;
			break;
		case 'B':
			converter = 'B';
			break;
		case 'D':
			Dflg++;
			break;
		case 'd':
		        if ( sscanf(optarg," %d", &depth_bound) <= 0 )
            		 uerror("incorrect option argument (can't read integer)","");
			break;
		case 'c':
			if ( sscanf(optarg," %f", &prune_factor) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
		case 'f':
		case 'e':
		case 'M':
			pruner = c;
			break;
		case 'p':
			if ( converter=='b'  ) {
			 if ( sscanf(optarg," %f", &prune_factor1) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			} else {
			 if ( sscanf(optarg," %f", &prune_factor) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			 if (  pruner = 'f' )
				prune_factor /= c45_default;
			}
			break;
		case 'q':
			if ( sscanf(optarg," %f", &prune_factor2) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			break;
		case 'o':
		        if ( sscanf(optarg," %d", &max_opts) <= 0 )
            		 uerror("incorrect option argument (can't read integer)","");
			break;
		case 'x':
		        if ( sscanf(optarg," %d", &Multiple_trees) <= 0 )
            		 uerror("incorrect option argument (can't read integer)","");
			if (Multiple_trees < 2)
				error ("%s: incorrect argument for flag 'x'\n",
					progname);
			break;
		default:
 			uerror(usage,"");
		}

	order = argv[optind];
	treename = argv[optind + 1];
	create(order);
	if ( converter=='r' && !seed )
		rand_random();
	if (! Multiple_trees) {
		prune_tree(treename, testfile, depth_bound,
			prune_factor, prune_factor1, prune_factor2,
			noprun, max_opts);
	} else {
		int j;
		char buf[256];
		for (j=0; j<Multiple_trees; j++) {
			sprintf(buf, "%s.x%d", treename, j);
			prune_tree(buf, testfile, depth_bound,
				prune_factor, prune_factor1, prune_factor2,
				noprun, max_opts);
		}
	}
	exit(0);
}

float message_length();

prune_tree(tname, testfile, depth_bound,
	prune_factor, prune_factor1, prune_factor2,
	noprun, max_opts)
char *tname, *testfile;
int depth_bound;
float   prune_factor;
float   prune_factor1;
float   prune_factor2;
bool    noprun;
int 	max_opts;
{
extern void tree_smooth();
char *p1, *p2;
egtesttype  testing;
double   tree_smpl();
	if ( verbose )
		  fprintf(stdrep, "\n");
	read_tree(tname, &tree, &thead);
        install_prior(&thead.prior,tree->eg_count);
	if ( depth_bound >= 0 )  {
		if ( verbose )
		    fprintf(stdrep, "depth pruning to depth %d;\n", 
				depth_bound);
		depth_prune(tree, depth_bound);
	}
	/*
         *	this program turns leaf counts into leaf probabilities;
	 *      if they are probabilities already, then just die quietly
	 */
	if ( thprobs(&thead) )  {
		if ( verbose )
		  fprintf(stdrep, 
		    "tree has probabilities already so no further pruning;\n");
		goto tprune_write;
	}
	/*
	 *	first do the pruning
	 */
	if ( (thoptions(&thead) || thgraph(&thead)) && strchr("fMecV",pruner) )
		uerror("is option tree or graph, can't prune!","");
	switch(pruner) {
	case 'f':	
		if ( verbose )
		    fprintf(stdrep, "C4.5 pruning with factor %g;\n", 
				prune_factor*c45_default);
		c45_prune(tree, prune_factor*c45_default);
		break;
	case 'M':	
		if ( verbose )
		    fprintf(stdrep, 
				"minimum cost pruning on counts in tree;\n");
		mincost_prune(tree);
		break;
	case 'e':	
		if ( verbose )
		    fprintf(stdrep, "pessimistic pruning with factor %g;\n", 
				prune_factor);
		pess_prune(tree, prune_factor);
		break;
	case 'c':	
		if ( verbose )
		    fprintf(stdrep, 
				"cost complexity prune to alpha level %g;\n",
				prune_factor);
		CC_costprune(tree, prune_factor);
		break;
	case 'V':
		if ( verbose )
		    fprintf(stdrep, 
		"cost complexity prune using test set and the %g-SE rule;\n",
				prune_factor);
		negs = file_recost(testfile, tree);
		CC_test_prune(tree, prune_factor, negs);
		break;
	default:
		;
	}

	/*
 	 * 	now convert to probs
	 */
	switch(converter) {
	case 'n':
		/*	
		 *	convert counts at nodes to probs using Laplacian
		 *	rule, with no fancy tricks
		 */
		if ( verbose )
		    fprintf(stdrep, 
		       "convert counts to probabilities by Laplace formula;\n");
		tree_reprob(tree);
		set_flag(thead.hflags,probs);
		break;
	case 'r':
	case 'b':
		/*	
		 *	convert counts at nodes to probs using Laplacian
		 *	and install node probabilities for
		 *	tree averaging later on (by tclass)
		 */
		if ( verbose )
		    fprintf(stdrep, 
		       "convert counts to probabilities by Bayesian smoothing;\n");
		set_flag(thead.hflags,bayes);
#ifdef GRAPH
                if (decision_graph_flag && is_dgraph(tree)) {
                        tree->testing = init_test();
			
                        thead.sprob =  thead.prior.prior_nml -
                                message_length(tree, 1, 0);
                        graph_avet(tree,prune_factor2, max_opts, noprun);
                } else {
#endif
			testing = init_test();
			thead.eprob = log_beta(tree->eg_count); 
                        thead.sprob =  thead.prior.prior_nml +
                            tree_avet(tree,prune_factor2, max_opts, noprun,testing);
			sfree(testing.unordp);
			if ( converter=='r' ) {
                            thead.sprob += log(tree_smpl(tree));
			}
#ifdef GRAPH
                }
#endif
		set_flag(thead.hflags,probs);
		break;
	case 'B':
		/*	
		 *	convert counts at nodes to probs using Laplacian
		 *	and determine max. posterior subtree
		 */
		if ( verbose ) {
		    fprintf(stdrep, 
		       "find the maximum a-posterior (miminum message length) tree then\n");
		    fprintf(stdrep, 
		       "convert counts to probabilities by Laplace formula;\n");
		}
		testing = init_test();
		thead.eprob = log_beta(tree->eg_count); 
		thead.sprob =  thead.prior.prior_nml +
		  tree_maxt(tree,prune_factor1,prune_factor2,testing);
		set_flag(thead.hflags,probs);
		sfree(testing.unordp);
		break;
	default:
		break;
	}

tprune_write:

	if ( Dflg ) {
		if ( thbayes(&thead) ) {
#ifdef GRAPH
		  if ( thgraph(&thead) )
			graph_smooth(tree,Dflg);
		  else
#endif
			tree_smooth(tree,Dflg);
		  unset_flag(thead.hflags,bayes);
		} else
			dec_prune(tree);	
	}
	p1 = strstr(tname, ".treec");
	if ( p1 != NULL) {
		strcpy(prunename, tname);
		p1 = strchr (p1, 'c');
		p1 ++;
		p2 = strstr(prunename, ".treec");
		p2 = strchr (p2, 'c');
		strcpy(p2, p1);
	} else {
		sprintf(prunename, "%s.p", tname);
	}
	write_tree(prunename, tree, &thead );
	if ( verbose )
		  fprintf(stdrep, "\n");
}

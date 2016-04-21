/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* tgen.c -- driver for the tree generation program
 *
 *	Original Author - David Harper, 1985 + Chris Carter, 1988)
 *
 *	Modifications:
 *		- Major structural changes throughout everything
 *			(Wray Buntine,  1989, 1991)
 */
#define	MAIN
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

extern  void sigint_handler();
extern  void sigxcpu_handler();
extern  int timeout;
extern  bool read_prior;

int	depth = 0;	/* Depth of current node */
t_head	thead;
int Multiple_trees=0;

extern int    verbose;
char	*progname;	/* Name of program -- for error messages */
char	*usage = "Usage: %s [-vC:c:d:ei:N:P:n:op:tT:U:A:B:MOQs:X:L:] order encoded tree\n";

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	egset	*egs;		/* Set of examples. */
	double	log();
	float   CC_CV_grow();
	long	CC_seed = 0;
	bool	CC_test_flg = FALSE;
	bool	CC_CV_flg = FALSE;
	bool	enc_not_done = FALSE;	/*  set to true if encoded file */
	int	CV_fold;
	float	CC_test_prop;
	float	prune_factor =1.0;
	int	Wtree_cycles = 0;    /*  maximum number of cycles allowed
					 for Wallace style growing  */
	float   W_mina = 0.25;      /*  minimum alpha for above */
        double  fabs();
        /*
         *         stuff for wallace style growing cycle
	 */

	extern	int	optind;	/* Argument processing variables. */
	extern	char	*optarg;
	char	*egname;	/* Name of file containing examples */
	char	*order;		/* Name of the order file */
	char	*treename;	/* The decision tree */

	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 3)
 		uerror(usage,"");
        while ((c = getopt(argc, argv,
          "OevNS:s:aC:c:d:i:P:n:op:tU:A:B:Mp:r:FfJ:L:X:Y:xIwbmK:G:D"))
                        != EOF) {
		switch (c)
		{
		case 'v':
			verbose++;
#ifdef DEBUG_dealloc
			debugging_alloc ++;
#endif
			break;
		case 'U':
			tree_opts(c,optarg);
			break;
/*
		case 'W':
			Wtree_cycles = 2;
			sscanf(optarg,"%d,%f", &Wtree_cycles, &W_mina);
			break;  
		case 'R':
			set_flag( thead.hflags,random);
			thead.genprob = 1.0;
			choose_opts(c,optarg);
			break;   */
		case 's':
		case 'o':
		case 'O':
		case 'B':
		case 'K':
		case 'J':
		case 'L':
			make_opts(c,optarg);
			break;
#ifdef GRAPH
                case 'I':
                case 'w':
                case 'x':
                case 'b':
                case 'm':
                case 'D':
			dgraph_opts(c,optarg);
			break;
#endif
                case 'X':
                case 'Y':
		case 'G':
		case 'i':
		case 'n':
		case 't':
		case 'M':
		case 'F':
		case 'S':
			choose_opts(c,optarg);
			break;
		case 'e':
			enc_not_done++;
			break;
		case 'A':
		case 'P':
		case 'N':
		case 'd':
			prior_opts(c,optarg);
			break;
		case 'p':
	        	if ( sscanf(optarg," %f", &prune_factor) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			break;
		case 'c':
			CC_test_flg++;
                        if ( sscanf(optarg,"%f,%ld", 
					&CC_test_prop, &CC_seed) < 1)
			    uerror("can't scan arguments to -c option","");
			break;
		case 'C':
			CC_CV_flg++;
                        if ( sscanf(optarg,"%d,%ld", &CV_fold, &CC_seed) < 1 )
			    uerror("can't scan arguments to -C option","");
			break;
		default:
 			uerror(usage,"");
		}
	}
	order	 = argv[optind];
	egname = argv[optind + 1];
	treename = argv[optind + 2];

	/*
	 *	set up the symbol table
	 */

	read_prior = TRUE;
	create(order);

	/* do internal coding unless specifically told file is encoded */
	if (enc_not_done)
	  egs = make_set(read_eg_file(egname));
	else
	  egs = make_set(read_enc_egs(egname));

        tree = new_node((bt_ptr)0);
	tree->xtra.gr.egs = egs;
	add_counts(tree);
	install_prior((t_prior *)0,tree->eg_count);
	if ( verbose ) {
		display_prior();
		display_choose();
		display_make();
	}
	signal(SIGINT,sigint_handler);
	signal(SIGXCPU,sigxcpu_handler);
	if ( CC_test_flg ) {
	  CC_test_grow(tree, egs, CC_test_prop, prune_factor, CC_seed);
	} else if ( CC_CV_flg ) {
	  thead.sprob = CC_CV_grow(tree, egs, CV_fold, prune_factor, CC_seed);
	} else if ( Wtree_cycles ) {
	  wall_grow(tree,egs,Wtree_cycles, W_mina);
	} else 
	  maketree(tree, egs);
	load_prior(&thead.prior);
#ifdef GRAPH
	write_stored_trees(treename, tree, &thead);
        free_stored_trees();
#else
        write_tree(treename, tree, &thead);
#endif
	if ( timeout )
	  fprintf(stdrep,"time limit exceeded\n");
	free_tree(tree);
	free_gainstore();
#ifdef DEBUG_dealloc
	if ( debugging_alloc ) {
		report_alloc();
		display_alloc();
	}
#endif
	myexit(0);
}
	

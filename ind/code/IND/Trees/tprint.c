/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

 /* print -- tree printer. Prints a tree to stdrep.
 *
 *	Original Author - David Harper.
 *
 *	Major reorganise and additions
 *		"re-counts', internal node printing, "flags", ...
 *		1989, 1991	Wray Buntine
 */
#define MAIN
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


int	depth = 0;

char	*progname;
char	*order;
char	*treename;
char	*usage = "Usage: [bcdD:gilpqrt:u:U:ZO] attr_file tree_file\n";
tprint_flags      print_flags;
t_head 	thead;
extern int	verbose;

extern  float   cut_prob;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	char	*test_file;
	extern  double tree_smpl();

	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;
	int	print_depth=1000000;
	bool	rand_extract = FALSE;
	double  genprob;
	tprint_flags      flags;
	bool    bflg = FALSE;

	/*
	 *	Process the arguments.
	 */

	null_flags(flags);
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 2)
		uerror(usage,"");

	while ((c = getopt(argc, argv, "vcD:dpgqu:iZ:t:U:lbO")) != EOF)
	switch (c)
	{
	case 'O':
		rand_extract++;
		break;
	case 'v':
		verbose++;
		break;
	case 'l':
		set_flag(flags,lineno);
		break;
	case 'c':
		set_flag(flags,counts);
		break;
	case 'b':
		bflg++;
		break;
	case 'i':
		set_flag(flags,intnl);
		break;
	case 'p':
		/*   hit twice and get std. as well as probabilities */
		if ( flag_set(flags,prob) )
		    set_flag(flags,stddev);
		else
		    set_flag(flags,prob);
		break;
	case 'q':
		set_flag(flags,tprob);
		break;
	case 'r':
		set_flag(flags,pprob);
		break;
	case 'g':
		set_flag(flags,gain);
		break;
	case 'd':
		set_flag(flags,decis);
		break;
	case 'u':
		if ( sscanf(optarg," %f", &cut_prob) <= 0 )
            	  uerror("incorrect option argument (can't read float)","");
		break;
        case 'U':
	case 'Z':
		tree_opts(c,optarg);
		break;
	case 'D':
        	if ( sscanf(optarg," %d", &print_depth) <= 0 )
                   uerror("incorrect option argument (can't read integer)","");
		break;
	case 't':
		test_file = optarg;
		set_flag(flags,recnt);
		break;
	default:
		uerror(usage,"");
	}

	order    = (char*)salloc(strlen(argv[optind])+6);
	strcpy (order, argv[optind]);

	if (optind < (argc - 1)) {
	  treename = (char *)salloc(strlen(argv[optind+1])+1);
	  strcpy (treename, argv[optind+1]);
	} else {
	  strcat (order, ".attr");
	  treename = (char *)salloc(strlen(argv[optind])+6);
	  strcpy (treename, argv[optind]);
	  strcat (treename, ".tree");
        }

	create(order);
	sfree (order);
	read_tree(treename, &tree, &thead);
	sfree (treename);
	install_prior(&thead.prior,tree->eg_count);
	if ( flag_set(flags,recnt) ) {
		recnt_tree_f(tree, test_file);
	}
	if ( thbayes(&thead) ) {
		if  ( bflg ) {
			unset_flag(flags,probt);
		} else 
			set_flag(flags,probt);
	} else if ( flag_set(flags,probt) ) {
	  uerror(
	     "option -b must be used with tree from tprune -b\n",
	     "");
	}
	if ( !thprobs(&thead) && ( flag_set(flags,prob) || rand_extract ||
				  flag_set(flags,stddev) ) ) {
	  uerror("options -pO must be used with probability tree, not counts tree\n",
		 "");
	}
	if ( thprobs(&thead) && ( flag_set(flags,gain) || flag_set(flags,counts) ) ) {
	  uerror("options -cg must be used with counts tree, not probability tree\n",
		 "");
	}
	if ( verbose )
	  fprintf(stdrep, "Printing with tree flags 0%o.\n",int_flags(flags));
	if ( thbayes(&thead) ) 
		smoothflag = TRUE;
	if ( rand_extract ) {
		rand_random();
		thead.sprob += log(genprob=tree_smpl(tree));
	   fprintf(stdrep, "Generated tree has probability %lg\n",
				genprob);
	}
	print_tree(tree, flags, print_depth);
	exit(0);
}


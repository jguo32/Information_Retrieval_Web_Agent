/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* tquest.c -- program to use trees to classify examples
 *
 *	Author - Wray Buntine, 1992.
 *
 */
#define  MAIN
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

#define DEBUG_CHECK    /* set if certain checks to be made during runtime */

char    *progname;	/* Name of program -- for error messages */
char    *order;		/* Name of the order file */
char    *treename;	/* The decision tree */
char	*usage = "Usage: [-pvul] order tree\n";

int	depth = 0;
extern int	verbose;
tprint_flags	print_flags;

bool    stddev_flg=0,  prob_flg=0, util_flg=0, loop_flg=0;


main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	ot_ptr	t;
	t_head	thead;
	int     default_names;      /* = 1 if using default file names */

	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;
	extern  bool   tree_reclass_query;

	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 2)
		uerror(usage,progname);

	while ((c = getopt(argc, argv, "pvul")) != EOF)
		switch (c)
		{
		case 'v':
			stddev_flg++;
		case 'p':
			prob_flg++;
			break;
		case 'u':
			util_flg++;
			break;
		case 'l':
			loop_flg++;
			break;
		default:
			uerror(usage,progname);
		}

	if (optind == (argc - 1)) {
	    default_names = 1;
	    order =     (char *)salloc(100 * sizeof(char));
	    treename = (char *)salloc(100 * sizeof(char));

	    strcpy (order, argv[optind]);
	    strcat (order, ".attr");
	    strcpy (treename, argv[optind]);
	    strcat (treename, ".tree");
	} else {
	    default_names = 0;
	    order = argv[optind];
	    optind++;
	    treename = argv[optind++];
	}

	create(order);

	read_tree(treename, &t, &thead);
	if ( !thprobs(&thead) )
	  uerror("Trees haven't been converted to probabilities, use tprune","");
	 tree_reclass_query = TRUE;
	interp_egs(t, &thead);
	if (default_names) {
	    sfree (order);
	    sfree (treename);
	}
	exit(0);
}


/************************************************************************/
/*
 *	interp_egs(t)	--  interpret each example
 */

interp_egs(t, thead)
ot_ptr	t;
t_head	*thead;
{
	float	sum;
	float	*dd;
                /*   = array of probabilities for tree for class */
	float	*dd2;
                /*   = array of variances for tree for class */
	int	dec;
	int	j;
	egtype	eg;

	eg.unordp = (unordtype *) salloc(egsize);
	for (;;) {
	  assign_unknown( eg); 
	  fprintf(stdrep,"\nQuerying example:\n");
	  if ( thbayes(thead) ) {
	    dd = tree_class_ave(t, eg);
	    if ( stddev_flg )
	      dd2 = tree_class2_ave(t, eg);
	  } else {
	    dd = tree_class(t, eg);
	    if ( stddev_flg )
	      dd2 = tree_class2(t, eg);
	  }
	  fprintf(stdrep,"\nQuerying finished.\n");
#ifdef DEBUG_CHECK
	  sum = 0.0;
	  for (j = 0; j < ndecs; j++)
	    sum += dd[j];
	  if ( sum < 0.9 || sum > 1.1 )
	    fprintf(stderr,
		    "probability for example unnormalised (%lg)\n", sum);
#endif
	  if ( stddev_flg ) {
	    /*
	     *   subtract mean squared to get variance
	     */
	    for (j = 0; j < ndecs; j++) 
	      dd2[j] -= dd[j]*dd[j];
	  }
	  dec = best_dec(dd);
	  fprintf(stdrep, "Best decision: %s.\n", find(0, dec));
	  if ( prob_flg ) {
	    fprintf(stdrep,"Probability estimates: %lg",dd[0]);
	    for (j = 1; j < ndecs; j++) 
	      fprintf(stdrep,"+%lg",dd[j]);
	    fprintf(stdrep,".\n");
	    if ( stddev_flg ) {
	      fprintf(stdrep," and their std. dev.: ");
	      for (j = 0; j < ndecs; j++) 
		fprintf(stdrep,"%lg,",sqrt(dd2[j]));
	      fprintf(stdrep,"\n");
	    }
	  }
	  if ( util_flg ) {
	    fprintf(stdrep,"Expected utility: %g.\n", prob_util(dd,dec) );
	  }
	  sfree(dd);
	  if ( stddev_flg )
	    sfree(dd2);
	  if ( !loop_flg )
	  	break;
	}
}


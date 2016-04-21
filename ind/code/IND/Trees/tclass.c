/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* tclass.c -- program to use trees to classify examples
 *
 *	Original Author - David Harper, 1985.
 *
 *	Modifications: 
 *		extensive rewriting by Wray Buntine, 1989, 1991
 *		added -x flag, Jon Oliver, 1992
 */
#define  MAIN
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

#define DEBUG_CHECK    /* set if certain checks to be made during runtime */

#define cv_error(i)  (100.0-100.0 * thead[i].sprob)
#define cv_stddev(i) (thead[i].sprob<=0.0||thead[i].sprob>=1.0 ? 0.0 : \
		      100.0*sqrt(thead[i].sprob*(1.0-thead[i].sprob) \
				      /t[i]->tot_count) )

char    *progname;	/* Name of program -- for error messages */
char    *egfile;	/* Name of file containing examples */
bool	enc_not_done = FALSE;   /*  set to true if test file encoded  */
char    *order;		/* Name of the order file */
char    *treename1;
char    **treename;	/* The decision tree */
char	*usage = "Usage: [-cDdqPpsSU:m:bW:toeZqv] order tree(s) [examples]\n";

int	depth = 0;
extern int	verbose;
tprint_flags	print_flags;

int	**ncorrect;	/* count of correct by tree by actual dec.	*/
double	*corr_prop;	/* correct prop by tree by actual dec.	*/
float	*util;		/* utility by tree		*/
int	*tsize;		/* leafcount by tree		*/
int	ntrees = 1;
int	*cnegs;		/*  no. of egs. read in by class */
double	***freq;	/* table for mis classification matrix for trees */
double  *restrat; 	/* restratifying factors */
double  *weights; 	/* (unnormalised) weight for tree */
double  **sse;		/* sum of squares of the error */
double  *sse_prop;	/* sum of squares of the error */
double  **expc;		/* expected error */
double  *expc_prop;	/* expected error */
double  *expc_var;	/* expected variance */
double  **treediff;  	/* diff. between trees */

bool	treediff_flg = FALSE;	/* do matrix of tree differences.  */
bool    Gflg = FALSE;   /* assume head->sprob is errors count from CC_CV_grow
			   and print out error estimation for it */
bool	tflg = FALSE;	/* do matrix of (mis)classification probs.  */
bool    bflg = FALSE;   /* be brief in output */
bool    cflg = FALSE;   /* Print out the given class of each example */
bool    eflg = FALSE;   /* Print out whether right (1) or wrong (0) for each tree */
int     Eflg = 0;       /* estimate error from tree using randomly gen. 
			 * examples with Uflg=6 in tree_reclass
			 */
bool    dflg = FALSE;   /* Print out the decision for each example */
bool    gflg = FALSE;   /* Print out the posterior (in header) each tree 
			 * and if tree was randomly generated, print log. prob
			 * of generation
			 */
bool    Dflg = FALSE;   /*
			 *	Print out the decision for each tree and
			 *	each example
			 */
bool    mcflg = FALSE;   /* use most common when making decision */
bool    pflg = FALSE;   /* Print out the probability vector for each example */
bool    Pflg = FALSE;   /*
			 *	Print out the probability vector for
			 *	each tree and each example
			 */
bool    sflg = FALSE;   /*
			 *	summarise the performance of the classifier
			 *	both the percentage correct and the mean-square
			 *	error are printed
			 */
bool    Sflg = FALSE;   /*
			 *	and for each component tree (if ntrees > 1)
			 */
bool	mflg = FALSE;   /*   indicates interest in "multiple tree"  */
bool	wflg = FALSE;   /*   set weights from "thead.sprob" and ".genprob" using
			 *   importance sampling formula
			 */
bool	lflg = FALSE;   /*   print leafcount for each tree      */
bool	stdflg = FALSE;   /*   print std. dev. of probs      */
bool	disp_prior_flg = FALSE;   /*   display prior for tree     */
 
int Multiple_trees=0;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	int	i,j,k;
	ot_ptr	*t;
	t_head	*thead;
	double	dd, exp();
	int     default_names;      /* = 1 if using default file names */

	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;

	weights = make_dvec(1);
	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 2)
		uerror(usage,progname);

	while ((c = getopt(argc, argv, "QwcDdqPpsSU:m:bW:toeZlXvgGfx:")) 
				!= EOF)
		switch (c)
		{
/*		case 'r':
			rev_strat_flg++;
			break;    */
		case 'q':
			treediff_flg++;
			break;
		case 'g':
			gflg++;
			break;
		case 'l':
			lflg++;
			break;
		case 'w':
			wflg++;
			break;
		case 'o':
			mcflg++;
			break;
		case 'e':
			enc_not_done++;
			break;
		case 'f':
			eflg++;
			break;
/*
 *       the "Uflg=6" routines have been disabled,
         might install later, so this wont work.
		case 'E':
			if ( sscanf(optarg," %d", &Eflg) <= 0 )
            		 uerror("incorrect option argument (can't read integer)","");
			 */
		case 'v':
			stdflg++;
			break;
		case 'Z':
		case 'U':
			tree_opts(c,optarg);
			break;
		case 'Q':
			disp_prior_flg++;
			break;
		case 'c':
			cflg++;
			break;
		case 't':
			tflg++; 
			break;
		case 'd':
			dflg++;
			break;
/*
		case 'u':
			if ( (sscanf(optarg," %f",&cut_prob)<=0) ||
				(cut_prob <= 0.0) || cut_prob >= 1.0)
			error (    "%s: incorrect argument for flag 'u'\n",
				progname );
			break;     */
		case 'D':
			Dflg++;
			break;
		case 'm':
			if ( (sscanf(optarg," %d",&ntrees)<=0) ||
				(ntrees < 2))
			error (    "%s: incorrect argument for flag 'm'\n",
				progname);
			sfree(weights);
			weights = make_dvec(ntrees);
			for (i=0; i<ntrees; i++) 
				weights[i] = 1.0/ntrees;
			break;
		case 'W':
			if (ntrees <2)
			    error (    
				"%s: 'W' flag must have 'm' flag already set \n",
				progname );
			/*   read weights from "optarg" into "weights[]"  */
			dd = 0.0;
			for (i=0; i<ntrees; i++) {
			    if ( sscanf(optarg, " %lf",&weights[i]) <= 0 )
			        error (    
				    "%s: argument to 'W' flag must be weights list\n",
					progname );
			     while ( *optarg == ' ' || *optarg == '\t'
						|| *optarg == '\n' ) optarg++;
			     while ( *optarg && *optarg != ' ' && *optarg != '\t'
						&& *optarg != '\n' ) optarg++;
			     dd += weights[i];
			}
			/*   normalise  "weights[]"   */
			if (dd)
			    for (i=0; i<ntrees; i++) 
				weights[i] /= dd;
			else
			    for (i=0; i<ntrees; i++) 
				weights[i] = 1.0/ntrees;
			break;
		case 'p':
			pflg++;
			break;
		case 'P':
			Pflg++;
			break;
		case 'b':
			bflg++;  tflg=0;
			break;
		case 'S':
			Sflg++;
			break;
		case 'G':
			Gflg++;
			break;
		case 's':
			sflg++;
			break;
                case 'x':
                        if (ntrees != 1)
                                error ("%s: incompatible flags with flag 'x'\n",
                                        progname);
                        Multiple_trees = 1;
                        if ((sscanf(optarg," %d",&ntrees)<=0) || (ntrees < 2))
                                error ("%s: incorrect argument for flag 'x'\n",
                                        progname);
                        break;
		default:
			uerror(usage,progname);
		}

	/*
	 *	adjust flags to account for the fact that
	 *	flags all change their meaning for "ntrees"==1
	 */
	if ( ntrees==1 ) {
		if (dflg) 
			Dflg++;
		if (sflg)  {
			Sflg++;
			sflg = FALSE;
		}
		if (pflg)
			Pflg++;
	} else {
		if (dflg || pflg || sflg || eflg || treediff_flg)
			mflg++;
	}


	treename = (char **)salloc(ntrees* sizeof(char *));

	if (optind == (argc - 1))
	  {
	    default_names = 1;
	    order =     (char *)salloc(100 * sizeof(char));
	    treename1 = (char *)salloc(100 * sizeof(char));
	    egfile =    (char *)salloc(100 * sizeof(char));

	    strcpy (order, argv[optind]);
	    strcat (order, ".attr");
	    strcpy (treename1, argv[optind]);
	    strcat (treename1, ".tree");
	    treename[0] = &treename1[0];
	    strcpy (egfile, argv[optind]);
	    strcat (egfile, ".tst");
	  }
	else
	  {
	    default_names = 0;
	    order = argv[optind];
	    optind++;
            if (Multiple_trees) {       /* -x option */
                char buf[256];
                for (i = 0; i < ntrees; i++) {
                        sprintf(buf, "%s.x%d", argv[optind], i);
                        treename[i] = (char *) salloc(strlen(buf)+1);
                        strcpy(treename[i], buf);
                }
                optind++;
            } else {
                for (i = 0; i < ntrees; i++)
                        treename[i] = argv[optind++];
            }
	    if (optind < argc)
	      egfile = argv[optind];
	    else
	      egfile = "";
	  }

	create(order);

	cnegs = make_ivec(ndecs);
	if (sflg || Sflg) {
	    if (tflg || utilities || rev_strat_flg) {
	        restrat = make_dvec(ndecs);
	        freq = (double ***) salloc((ntrees+1)*sizeof(double **));
	        for (i = 0; i <= ntrees; i++)
		    freq[i] = make_dmtx(ndecs,ndecs);
	    }  
	    ncorrect = make_imtx(ntrees+1,ndecs);
	    corr_prop = make_dvec(ntrees+1);
	    sse = make_dmtx(ntrees+1,ndecs);
	    sse_prop = make_dvec(ntrees+1);
	    expc = make_dmtx(ntrees+1,ndecs);
	    expc_prop = make_dvec(ntrees+1);
	    if ( stdflg)
		expc_var = make_dvec(ntrees+1);
	    if ( utilities ) {
		util = make_fvec(ntrees+1);
	    }
	}
	tsize = make_ivec(ntrees+1);
	if ( treediff_flg )
		treediff = make_dmtx(ntrees+1,ntrees+1);

	t = (ot_ptr *)salloc(ntrees* sizeof(ot_ptr));
	thead = (t_head *)salloc(ntrees* sizeof(t_head));
	for (i = 0; i < ntrees; i++) {
		read_tree(treename[i], t+i, thead+i);
		if ( !thprobs(thead+i) )
		  uerror("Trees haven't been converted to probabilities, use tprune","");
		tsize[i] = thead[i].leafsize;
		/*    NB.   if priors different then trouble! */
		if ( !i ) {
		 	tree = t[0];
			install_prior(&(thead[i].prior),t[i]->eg_count);
		}
	}
	if ( wflg && ntrees > 1 ) {
	  /*
	   *        calculate weights using importance sampling formula
	   */
	  double  maxw, totw;
	  maxw = thead[0].sprob - thead[0].genprob;
	  for (i = 1; i < ntrees; i++) {
	    if ( maxw < thead[i].sprob - thead[i].genprob ) 
	      maxw = thead[i].sprob - thead[i].genprob;
	  }
	  totw = 0.0;
	  for (i = 0; i < ntrees; i++) 
	    totw += weights[i] = exp(thead[i].sprob - thead[i].genprob - maxw);

          for (i = 0; i < ntrees; i++)  
          	weights[i] /= totw;
	}
	tsize[ntrees] = 0;
	interp_egs(t, thead, egfile);
	if (default_names)
	  {
	    sfree (order);
	    sfree (treename1);
	    sfree (egfile);
	  }
	/*
	 *	all statistics currently spread over actual classes
	 *	so collate
	 */
    if ( sflg || Sflg || Eflg ) {
	if ( rev_strat_flg ) {
	    double	sum;
	    /*
	     *	test set stratified so reverse it to report "normalised" results
	     */
	    sum = 0.0;
	    for ( i=0; i<ndecs; i++) 
		    sum += cnegs[i]/strat_true[i];
	    for ( i=0; i<ndecs; i++) {
		restrat[i] = 1.0/sum/strat_true[i]; 
	    }
	    for ( i = (Sflg ? 0 : ntrees) ;
		  sflg ? i<=ntrees : i<ntrees ; i++) {
	        for ( j=0; j<ndecs; j++) {
		    expc_prop[i] += expc[i][j]*restrat[j];
		    sse_prop[i] += sse[i][j]*restrat[j];
		    corr_prop[i] += ncorrect[i][j]*restrat[j];
	            for ( k=0; k<ndecs; k++)
			freq[i][j][k] *= restrat[k]*negs;
		}
	    }
	} else {
	    for ( i = (Sflg ? 0 : ntrees) ;
		  (sflg||Eflg) ? i<=ntrees : i<ntrees ; i++) {
	        for ( j=0; j<ndecs; j++) {
		    expc_prop[i] += expc[i][j];
		    sse_prop[i] += sse[i][j];
		    corr_prop[i] += ncorrect[i][j];
	        }
	        expc_prop[i] /= negs;
		/*   errors for individual examples are correlated, so usual
		     variance trick, var of sum = sum of vars/sqrt(negs),
		     cannot be applied.   Calc. below returns an improper
		     value, but the average of the variances per example is a
		     useful indication of uncertainty  */
		if ( stdflg )
		  expc_var[i] /= negs;
	        sse_prop[i] /= negs;
	        corr_prop[i] /= negs;
	    }
	}
	if ( utilities ) {
	    for ( i = (Sflg ? 0 : ntrees) ;
		  sflg ? i<=ntrees : i<ntrees ; i++) 
		    util[i] = mtx_utility(freq[i]);
	}
    }

	if (Sflg && negs)
	    for (i = 0; i < ntrees; i++)
	    {
	    /*
	     *	print details for individual trees
	     */
		if ( bflg && !Eflg ) {
		    fprintf ( stdrep, "%lg %lg %lg ",
				100.0 * corr_prop[i], sse_prop[i],
				100.0 * expc_prop[i] );
		    if (stdflg && ntrees==1 )
		      fprintf ( stdrep, " %lg ", 100.0*sqrt(expc_var[i]) );
		    if ( gflg ) 
		      fprintf ( stdrep,	"%lg %lg ", 
			-thead[i].sprob, -thead[i].eprob );
		    if ( Gflg && ntrees==1 ) 
		      if ( thbayes(thead) )
			fprintf ( stdrep, "0 0 ");
		      else
			fprintf ( stdrep, "%lg %lg ",
				 cv_error(i), cv_stddev(i) );
		    if ( lflg ) 
		      fprintf ( stdrep,	"%d %g ", tsize[i], exp_nodes(t[i]) );
		    if ( utilities ) 
			fprintf ( stdrep,"%g ", util[i] / negs );
		} else {
		    if ( i == 0 && ntrees > 1 ) {
		        fprintf ( stdrep, "Tree weights =" );
			for (j = 0; j < ntrees; j++)
		    	    fprintf ( stdrep, " %lg", weights[j] );
			fputc('\n', stdrep);
		    }
		    if ( !Eflg ) {
		      fprintf ( stdrep,
			       "Percentage accuracy for tree %d = %lg +/- %lg\n",
			       i+1, 100*corr_prop[i],
			       100*sqrt(corr_prop[i]*(1.0-corr_prop[i])/negs) );
		    }
		    if ( !Eflg || ntrees==1 ) {
		      fprintf ( stdrep,
			       "Mean square error for tree %d = %lg\n",
			       i+1, sse_prop[i] );
		    }
		    if ( stdflg ) {
		      fprintf ( stdrep,
		       "Expected accuracy for tree %d = %lg\n",
		       i+1, 100.0 * expc_prop[i] );
		      fprintf ( stdrep,
		       "Typical std. dev. of expected accuracy for an example = %lg\n",
			       100.0 * sqrt(expc_var[i]) );
		    }  else
		      fprintf ( stdrep,
			"Expected accuracy for tree %d = %lg\n",
			i+1, 100.0*expc_prop[i] );
		    if ( utilities && (!Eflg || ntrees==1) ) {
			fprintf ( stdrep,
			"Average utility for tree %d = %g\n",
			i+1, util[i] / negs );
		    }
		    if ( gflg ) {
		      fprintf ( stdrep,
			       "Neg. Log Posterior for tree %d = %lg (nits)\n",
			       i+1,  -thead[i].sprob );
		      fprintf ( stdrep,
			       "Neg. Log Posterior for examples = %lg (nits)\n",
			       -thead[i].eprob );
		      if ( thrandom(thead+i) )
			fprintf ( stdrep,
			       "Log Prob. of generating random tree %d = %lg\n",
			       i+1,  thead[i].genprob );
		    }
		    if ( Gflg )
		      if ( thbayes(thead) )
			fprintf ( stdrep, 
				 "-G flag incompatible with Bayes pruning\n");
		      else
			fprintf ( stdrep, 
				 "Estimated accuracy from X-val = %lg+/-%lg\n",
				 cv_error(i), cv_stddev(i) );
		    if ( lflg ) {
			fprintf ( stdrep,
			"Leaf count for tree %d = %d, expected = %f\n",
			i+1, tsize[i], exp_nodes(t[i]) );
		    }
		    if (tflg && (!Eflg || ntrees==1) ) {
		        fprintf ( stdrep,
			    "Misclassification matrix for tree %d:\n", i+1);
		        fprintf ( stdrep, 
			     " (row = predicted, col = actual, in .attr file order)\n");
		        mmprint(freq[i],ndecs);
		    }
		}
	    }
	if (sflg && negs)
	  /*
	   *	print details for multiple tree
	   */
	  if ( bflg && !Eflg) {
	    fprintf ( stdrep, "%lg %lg %lg",
		     100.0 * corr_prop[ntrees], sse_prop[ntrees],
		     100.0*expc_prop[ntrees] );
	    if (stdflg)
	      fprintf ( stdrep, " %lg ",
		       100.0*sqrt(expc_var[ntrees]) );
	    if ( utilities ) 
	      fprintf ( stdrep,"%g ", util[ntrees] / negs );
	    if ( Gflg )
	      if ( thbayes(thead) )
		fprintf ( stdrep, "0 0 ");
	      else
		fprintf ( stdrep, "%lg %lg ",
				 cv_error(i), cv_stddev(i) );
	  } else {
	    if ( !Eflg ) {
	      fprintf ( stdrep,
		       "Percentage accuracy for multiple tree = %lg +/- %lg\n",
		       100.0 * corr_prop[ntrees],
		       100*sqrt(corr_prop[ntrees]*(1.0-corr_prop[i])/negs) );
	    }
	    fprintf ( stdrep,
		       "Mean square error for multiple tree = %lg\n",
		       sse_prop[ntrees] );
	    if ( stdflg ) {
	      fprintf ( stdrep,
		       "Expected accuracy for multiple tree = %lg\n",
		       100.0 * expc_prop[ntrees] );
	      fprintf ( stdrep,
		 "Typical variance of expected accuracy for an example = %lg\n",
		       100.0 * sqrt(expc_var[ntrees]) );
	    }
	    else
	      fprintf ( stdrep,
		       "Expected accuracy for multiple tree = %lg\n",
		       100.0 * expc_prop[ntrees] );
	    if ( utilities ) {
	      fprintf ( stdrep,
		       "Average utility for multiple tree = %g\n",
		       util[ntrees] / negs );
	    }
	    if ( Gflg )
	      if ( thbayes(thead) )
		fprintf ( stdrep, 
			 "-G flag incompatible with Bayes pruning\n");
	      else
		fprintf ( stdrep, "Estimated accuracy from X-val = %lg+/-%lg\n",
				 cv_error(i), cv_stddev(i) );
	    if (tflg) {
	      fprintf ( stdrep,
		       "Misclassification matrix for multiple tree");
	      fprintf ( stdrep, " (row = predicted, col = actual) :\n");
	      mmprint(freq[ntrees],ndecs);
	    }
	    if ( encodeerrs() ) {
	      fprintf ( stdrep,
		       "Examples unable to be encoded (ignored) = %d\n",
		       encodeerrs() );
	    }
	  }
	if ( bflg && !Eflg )
	    fprintf ( stdrep, "\n" );
        if (treediff_flg) {
                fprintf(stdrep, "Tree differences:\n (up tri. = disagree on class,");
		fprintf(stdrep, "  low tri. = manhattan dist. of prob.)\n");
                for (i=0; i<=ntrees; i++) {
                    fprintf(stdrep, "  ");
                    for (j=0; j<=ntrees; j++)
                        fprintf(stdrep, "%lf   ", treediff[i][j] / negs);
                    fprintf(stdrep, "\n");
                }
        }
	exit(0);
}


/************************************************************************/
/*
 *	interp_egs(t, egname)	--  interpret each example
 */

interp_egs(t, thead, egname)
ot_ptr	*t;
t_head	*thead;
char	*egname;
{
	float	*d = mems_alloc(float,ndecs);
	float	*d2 = mems_alloc(float,ndecs);
	float	sum;
	float	dvec_diff();
	float	**dd = mems_alloc(float*, ntrees);
                /*   = array of probabilities for tree for class */
	float	**dd2 = mems_alloc(float*, ntrees);
                /*   = array of variances for tree for class */
	int	this_dec, *dec = make_ivec(ntrees), eg_class;
	int	i, j;
	egtype	eg;

	if ( Eflg ) {
	  eg.unordp = mems_alloc(unordtype,egsize);
	  Uflg = 6;
	}
	if ( !enc_not_done )
		init_read_enc_eg(egname,0);
	for (;;) {
	        if ( Eflg ) {
		  if ( negs >= Eflg )
			break;
		  eg_class = 0;
		  assign_unknown(eg);
		} else {
		  if ( enc_not_done && !(eg = read_eg(egname)).unordp )
			break;
		  if ( !enc_not_done && !(eg = read_enc_eg()).unordp )
			break;
		  eg_class = class_val(eg);
		}
		cnegs[eg_class]++;
		if (cflg)
			fprintf(stdrep, "%s\t", find(0, eg_class));

		for (j = 0; j < ndecs; j++) {
			d[j] = 0.0;
			d2[j] = 0.0;
			}
		for (i = 0; i < ntrees; i++)
		{
			if ( thbayes(thead+i) ) {
			    dd[i] = tree_class_ave(t[i], eg);
			    if ( stdflg )
			        dd2[i] = tree_class2_ave(t[i], eg);
			} else {
			    dd[i] = tree_class(t[i], eg);
			    if ( stdflg )
			        dd2[i] = tree_class2(t[i], eg);
			}
#ifdef DEBUG_CHECK
			sum = 0.0;
			for (j = 0; j < ndecs; j++)
				sum += dd[i][j];
			if ( sum < 0.9 || sum > 1.1 )
			  fprintf(stderr,
			     "probability for example %d unnormalised (%lg)\n",
				  negs,  sum);
#endif
			if (mflg) for (j = 0; j < ndecs; j++) {
			    d[j] += dd[i][j] * weights[i];
			    if ( stdflg )
			        d2[j] += dd2[i][j] * weights[i];
			} else
			    if ( stdflg )
				/*
				 *   subtract mean squared to get variance
				 */
			       for (j = 0; j < ndecs; j++) 
					dd2[i][j] -= dd[i][j]*dd[i][j];
			if (mcflg)
				this_dec = dec[i] = most_common(dd[i]);
			else
				this_dec = dec[i] = best_dec(dd[i]);
			if ( Eflg )
			  eg_class = this_dec;
			if (Dflg)
				fprintf(stdrep, "%s\t", find(0, this_dec));
			if (eflg)
				fprintf(stdrep, "%c\t",
					(this_dec==eg_class)?'1':'0');
			if (Sflg) {
			    if ( Eflg ) {
			      if (tflg || utilities)
			        for (j = 0; j < ndecs; j++) 
				  freq[i][this_dec][j] += dd[i][j];
			    } else {
			      if (tflg || utilities)
				freq[i][this_dec][eg_class]++;
			      if (this_dec == eg_class)
				ncorrect[i][eg_class]++;
			    }
			    sse[i][eg_class] += calc_mse(eg_class, dd[i]);
			    expc[i][this_dec] += dd[i][this_dec];
			    if (stdflg )
				expc_var[i] += dd2[i][this_dec];
			}
			if ( Pflg  ) {
			        fprintf(stdrep,"%lg",dd[i][0]);
			    	for (j = 1; j < ndecs; j++) 
				  fprintf(stdrep,"+%lg",dd[i][j]);
				if ( stdflg ) {
				    fprintf(stdrep,"+-");
			    	    for (j = 0; j < ndecs; j++) 
					fprintf(stdrep,"%lg,",sqrt(dd2[i][j]));
				  }
				fprintf(stdrep,"\n");
			}
		}
                if (treediff_flg) {
                    for (i=0; i<ntrees; i++)
                        for (j=0; j<i; j++) {
                                treediff[i][j] += dvec_diff(dd[i],dd[j]);
                                treediff[j][i] += (dec[i]==dec[j])? 0 : 1;
                        }
                }
		if (mflg ) {
			if ( stdflg )
			/*
			 *   subtract mean squared to get variance
			 */
			    for (j = 0; j < ndecs; j++) 
				d2[j] -= d[j]*d[j];
		 	this_dec = best_dec(d);
			if (sflg) {
			  if ( Eflg ) {
		    	    if (tflg || utilities) {
			      for (j = 0; j < ndecs; j++) 
				freq[ntrees][this_dec][j] += d[j];
			    }
		    	    sse[ntrees][this_dec] += calc_mse(this_dec, d);
			  } else {
		    	    if (tflg || utilities)
				freq[ntrees][this_dec][eg_class]++;
		    	    if (this_dec == eg_class)
				ncorrect[ntrees][eg_class]++;
		    	    sse[ntrees][eg_class] += calc_mse(eg_class, d);
			  }
			  expc[ntrees][this_dec] += d[this_dec];
			  if (stdflg )
			        expc_var[ntrees] += d2[this_dec];
			}
			if (dflg)
				fprintf(stdrep, "%s\t", find(0, this_dec));
			if (eflg)
				fprintf(stdrep, "%c\t",
					(this_dec==eg_class)?'1':'0');
			if (pflg) {
			        fprintf(stdrep,"%lg",sqrt(d[0]));
			    	for (j = 1; j < ndecs; j++) 
				  fprintf(stdrep,"+%lg",sqrt(d[j]));
				if ( stdflg ) {
				    fprintf(stdrep,"+/-");
			    	    for (j = 0; j < ndecs; j++) 
					fprintf(stdrep,"%lg,", sqrt(d2[j]) );
				}
				fprintf(stdrep,"\t");
			}
                        if (treediff_flg)  for (j=0; j<ntrees; j++) {
                            treediff[ntrees][j] += dvec_diff(d,dd[j]);
                            treediff[j][ntrees] += (this_dec==dec[j])? 0 : 1;
                        }
		}
		for (j=0; j<ntrees; j++) {
		  sfree(dd[j]);
		  if ( stdflg )
		    sfree(dd2[j]);
		}
		if (cflg || dflg || Dflg || eflg )
			putc('\n', stdrep);
	}
	if ( Eflg )
		sfree(eg.unordp);
	sfree(d);
	sfree(d2);
	sfree(dd);
	sfree(dd2);
}

/****************************************************************************/
/*
 *      dvec_diff(d1,d2) - compute manhattan distance of 2 prob. vecs.
 */
float
dvec_diff(d1,d2)
float	*d1,*d2;
{
        int     i;
        float   tot=0.0, xx;
 
        for (i = 0; i < ndecs; i++) {
                xx = (d1[i] - d2[i]);
                tot += (xx>0.0 ? xx : -xx);
        }
        return tot/ndecs;
 
}

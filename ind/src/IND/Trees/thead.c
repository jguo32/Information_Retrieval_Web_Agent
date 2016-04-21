/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* size.c -- program to #report details from tree header
 *
 *	Original Author - David Harper, 1985.
 */
#define MAIN
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"


char	*progname, *strrchr();
char	*usage = "Usage: [-nlpt -m #trees -A alpha -P prioropts -E alpha,beta ] tree\n";
int	depth=0;
extern int	verbose;

main(argc, argv)
int	argc;
char	*argv[];
{
	char	option = 's';
	int	c;
	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;
	int	i;
	char	**treename;
	t_head	head;
	int	no_bayes=0;
	int	ntrees = 1;
	FILE	*fp;

	verbose = 1;
	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];

	while ((c = getopt(argc, argv, "shltpnm:A:P:E:")) != EOF)
		switch (c) {
		case 'p':
			option='p';
			break;
		case 'l':
			option='l';
			break;
		case 'm':
        	        if ( sscanf(optarg," %d", &ntrees) <= 0 )
            		 uerror("incorrect option argument (can't read integer)","");
			if ( option=='h')
				uerror(usage,"");
			break;
		case 't':
			option='t';
			break;
		case 'n':
			option='h';
			no_bayes++;
			break;
		case 'P':
		case 'A':
		case 'E':
			prior_opts(c,optarg);
			option='h';
			if ( ntrees > 1)
				uerror(usage,"");
			break;
		case 's':
			option='s';
			break;
		case 'h':
			option ='h';
			break;
		default:
			uerror(usage,"");
		}

#ifdef GRAPH
	decision_graph_flag = decision_graph_flag;
#endif
	if (argc - optind < 1)
		uerror(usage,"");
	treename = (char **)salloc(ntrees* sizeof(char *));
	for (i = 0; i < ntrees; i++)
		treename[i] = argv[optind++];
	if (argc !=  optind)
		uerror(usage,"");

	for (i=0; i<ntrees; i++) {
		switch (option) {
		case 'l':
			read_header(treename[i], &head);
			printf("%d  ", head.leafsize);
			break;
		case 't':
			read_header(treename[i], &head);
			printf("%d  ", head.treesize);
			break;
		case 's':
			read_header(treename[i], &head);
			printf("Tree %d has %d leaves and %d nodes (weight %g)\n",
				i+1, head.leafsize, head.treesize, head.sprob);
			break;
		case 'h':
        		fp = efopen(treename[i], "r+");
        		if (fread((char *) &head, sizeof(t_head), 1, fp) != 1)
               			 uerror("Could not read in decision tree\n", "");
        		if (head.magicno != MAGICNO)
               			 uerror("Incorrect tree file type -- bad magic number", "");
			if ( no_bayes ) {
				unset_flag(head.hflags,bayes);
			} else {
				load_prior(&head.prior);
				set_flag(head.hflags,bayes);
			}
			rewind(fp);
			fwrite((char *) &head, sizeof(t_head), 1, fp);
			fclose(fp);
			break;
		case 'p':
			read_header(treename[i], &head);
		        install_prior(&head.prior,(float*)0);
			break;
		default:
			error("unknown option\n","");
		}
	}
	if (option != 's' && option != 'p'  )
		printf("\n");
        exit(0);

}

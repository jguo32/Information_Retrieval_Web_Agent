/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* weight.c -- program to print weight of a tree.
 *
 *	Original Author - David Harper, 1985.
 */
#define	MAIN
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

char	*progname, *strrchr();
char	*usage = "Usage: [-l C -e C] treecount treename ... \n";

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i, ntrees;
	char	option = 'e';
	int	c;
	double	lfactor=0.1;
	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;
	double	exp();
	char	**treename;
	t_head	head;

	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];

	while ((c = getopt(argc, argv, "e:nl:")) != EOF)
		switch (c) {
		case 'e':
			option='e';
			if ( sscanf(optarg," %lf", &lfactor) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			break;
		case 'l':
			option='l';
			if ( sscanf(optarg," %lf", &lfactor) <= 0 )
            		  uerror("incorrect option argument (can't read float)","");
			break;
		case 'n':
			option='n';
			break;
		default:
			uerror(usage,"");
		}

	if ( argc - optind < 2 ) 
		uerror(usage,"");
	ntrees    = atoi(argv[optind++]);
	treename = (char **)salloc(ntrees* sizeof(char *));
	for (i = 0; i < ntrees; i++)
		treename[i] = argv[optind++];
	if (argc !=  optind)
		uerror(usage,"");

	for (i=0; i<ntrees; i++) {
		read_header(treename[i], &head);
		switch (option) {
		case 'e':
			printf("%lg ", exp( - lfactor * head.leafsize ) );
			break;
		case 'l':
			printf("%lg ", lfactor * head.leafsize );
			break;
		default:
			error(usage,"");
		}
	}
	printf("\n");
	exit(0);
}


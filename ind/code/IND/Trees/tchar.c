/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

 /* pchar -- tree format converter
 *
 *	Author - Wray Buntine
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
char	*order;
char	*treein, *treeout;
char	*usage = "Usage: -a order treein treeout\n";
bool    aflg = FALSE;  	/* from char to tree */
int	verbose;

main(argc, argv)
int	argc;
char	*argv[];
{
	ot_ptr	t;
	t_head  head;
	int	c;
	bool    cflg = FALSE;
	extern  int     optind; /* Argument processing variables. */
	extern  char    *optarg;

	/*
	 *	Process the arguments.
	 */

	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 4)
		uerror(usage,progname);

	while ((c = getopt(argc, argv, "ac")) != EOF)
	switch (c)
	{
	case 'a':
		aflg++;
		break;
	case 'c':
		cflg++;
		break;
	default:
		uerror(usage,progname);
	}


	order= argv[optind];
	treein = argv[optind + 1];
	treeout = argv[optind + 2];

	create(order);
	if (aflg) {
		read_chtr(treein, &t, &head);
		if ( cflg && !thprobs(&head) )
		  chkcnt_tree(t);
		write_tree(treeout, t, &head);
	} else {
		read_tree(treein, &t, &head);
		write_chtr(treeout, t, &head);
	}
	exit(0);
	return 0;
}


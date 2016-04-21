/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* enc.c -- encodes data file, checks attribute file, etc.
 *
 *	Author - Wray Buntine, 1992.
 *
 */
#define	MAIN
#include <stdio.h>
#include <string.h>
#include "SYM.h"
#include "sets.h"
#include "DEFN.h"

char	*progname;	/* Name of program -- for error messages */
char	*usage = "Usage: %s [-nfcpaq] order data-in encoded-out \n";
int	verbose = 0;

prior_opts(c,opts)
char  c;
char  *opts;
{
	error("prior_opts should never be called","");
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	int	piping=0;	/* set non-zero if input file is stdin */
	extern  int sym_checking;/* checking level for attr. & data file */
	int	fill_all=0;	/* correct all atributes */
	int	c4_attr=0;	/* generate a c4 attr. file */
	int	no_enc=0;	/* don't encode, just parse egs */
	int	show_attr=0;	/* print atr updated ".attr" at the end */
	extern	int	optind;	/* Argument processing variables. */
	extern	char	*optarg;
	char	*egname;	/* Name of file containing examples */
	char	*encname;	/* Name of file to output encoded */
	char	*order;		/* Name of the order file */
	FILE	*outfile;	/*   file to write too  */
	egtype  eg;

	/*
	 *	Process the arguments.
	 */
	if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
		progname = argv[0];
	if (argc < 1)
 		uerror(usage,"");
        while ((c = getopt(argc, argv, "cpfnaq")) != EOF) {
		switch (c)
		{
		case 'n':
			no_enc++;
			break;
		case 'q':
			c4_attr++;
			break;
		case 'a':
			show_attr++;
			break;
		case 'f':
			fill_all++;
			break;
		case 'c':
			sym_checking++;
			break;
		case 'p':
			piping++;
			break;
		default:
 			uerror(usage,"");
		}
	}
	
	/*
	 *	construct file names
	 *	1.   if 1 command line arg., then that is the stem
	 *	     and use usual .attr, .dta, .enc recipe
	 *	2.   if "piping" true, then use stdin for egfile
	 *	3.   otherwise take file names of command line
	 */
        order    = (char*)salloc(strlen(argv[optind])+7);
        strcpy (order, argv[optind]);

        if ( piping ) {
   	  egname = "";
	  if (optind < (argc - 1)) {
            encname = (char *)salloc(strlen(argv[optind+1])+2);
            strcpy (encname, argv[optind+1]);
          } else {
            strcat (order, ".attr");
            encname = (char *)salloc(strlen(argv[optind])+7);
            strcpy (encname, argv[optind]);
            strcat (encname, ".enc");
          }
	} else {
	  if (optind < (argc - 2)) {
            egname = (char *)salloc(strlen(argv[optind+1])+2);
            strcpy (egname, argv[optind+1]);
            encname = (char *)salloc(strlen(argv[optind+2])+2);
            strcpy (encname, argv[optind+2]);
          } else {
            strcat (order, ".attr");
            egname = (char *)salloc(strlen(argv[optind])+7);
            strcpy (egname, argv[optind]);
            strcat (egname, ".dta");
            encname = (char *)salloc(strlen(argv[optind])+7);
            strcpy (encname, argv[optind]);
            strcat (encname, ".enc");
          }
	}

        create(order);
	if ( fill_all)
	    forattrs(c)
		sym_fill(c) = TRUE;

	if ( !no_enc ) {
		outfile = fopen(encname, "w");
		sfree(encname);
        	if ( fwrite((char *)&ld,sizeof(Header),1,outfile) != 1)
                	uerror("Could not write to encoded file", "");
        	while ( (eg = read_eg(egname)).unordp ) 
                	write_enc_eg(eg, outfile);
		/*
	 	 *	write the header once more to get. "negs" right
 	 	 */
        	rewind(outfile);
        	if (fwrite((char *)&ld,sizeof(Header),1,outfile) != 1)
                	uerror("Could not write to encoded file", "");
        	fclose(outfile);
	} else
        	while ( (eg = read_eg(egname)).unordp )  ;

	if ( show_attr )
		print_sym("");

	if ( c4_attr ) {
		strcpy(order+strlen(order)-4,"names");
		print_c4sym(order);
	}

	exit(0);
}

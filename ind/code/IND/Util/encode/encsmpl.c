/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* encsmpl.c -- add sampling info. to ".enc" header
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
char	*usage = "Usage: %s [-gcsvq] sample_args encoded \n";
int	verbose=0;

/* #define DEBUG */
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
	int     c;
        extern  int     optind; /* Argument processing variables. */
        extern  char    *optarg;
	unsigned char   train=0,test=0,printseed=0 ;
	char	*encname;	/* Name of file encoded */
	char	*order;	/* Name of file encoded */
	FILE	*encfile;	/*   file to write too  */
	Sampler *sampler;
	egtype   eg;
	int	c4_form = 0;  /* output text examples in quinlan's c4 form */
#ifdef DEBUG
	int  i;
	double f;
#endif

        /*
         *      Process the arguments.
         */
        if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
                progname = argv[0];
        if (argc < 1)
                uerror(usage,"");
        while ((c = getopt(argc, argv, "gcsvq")) != EOF) {
                switch (c)
                {
                case 'q':
                        c4_form++;
                        break;
                case 'g':
                        train++;
                        break;
                case 'c':
                        test++;
                        break;
                case 's':
                        printseed++;
                        break;
                case 'v':
                        verbose++;
                        break;
                default:
                        uerror(usage,"");
                }
        }
#ifdef DEBUG
	/*  setup randomization to check it resets OK during sample */
	rand_random();
	for (c=0; c<50; c++) {
		i = random();
		f = frandom();
	}
#endif
	if ( optind+2 != argc )
	    uerror(usage,progname);
	sampler = sample_opts(argv[optind++]);
	encname = (char *)salloc(strlen(argv[optind])+9);
	if ( sampler ) {
           /*
	    *	if no seed given then generate a near random one
	    */
	   if ( sampler->s_seed <= 0 ) {
		sampler->s_seed = time_random();
	   }
	   if ( printseed )
		   fprintf(stdrep,"seed = %ld\n",sampler->s_seed);
	   strcpy (encname, argv[optind]);
	   strcat (encname, ".enc");
	   encfile = ezopen(encname, "r");
           if ( fread((char *)&ld,sizeof(Header),1,encfile) != 1 )
               	uerror("Could not read encoded file", "");
           if (  ld.h_magicno != MAGICNO )
               	uerror("Encoded file wrong format", "");
           if ( ezopened )
                 pclose(encfile);
           else {
	         rewind(encfile);
                 fclose(encfile);
	   }
	   strcpy (encname, argv[optind]);
	   strcat (encname, ".encspl");
	   encfile = efopen(encname, "w");
           if ( fwrite((char *)sampler,sizeof(Sampler),1,encfile) != 1)
               	uerror("Could not write to sampling file", "");
	   sfree(sampler);
       	   fclose(encfile);
	}
	if ( train || test || verbose ) {
		order = (char *)salloc(strlen(argv[optind])+6);
		strcpy (order, argv[optind]);
        	strcat (order, ".attr");
		create(order);
		strcpy (encname, argv[optind]);
		strcat (encname, ".enc");
		init_read_enc_eg(encname,train);
		if ( train || test )
		    while (  (eg = read_enc_eg()).unordp )
			if ( c4_form )
			    write_c4eg(eg, stdrep);
			else
			    write_eg(eg, stdrep);
	} 
	exit(0);
}

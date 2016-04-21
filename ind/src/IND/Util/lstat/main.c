/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


/* main.c -- driver for stats analyser
 *
 *	Author  -  Wray Buntine
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <Lib.h>

#include "lstat.h"
#define	WHITE_SPACE	" \t\n"
#define NUM_MAX 50

static	char *usage = 
              "%s [-[avsAVSC] 'r1, ...,rn' -Fe -[lt] sep ] [ files ... ]\n";
/*
 *	stores   field separators
 */
char	field_sep[20] = " \t";
char	print_prec[10] = "%.4lf";
char	*tex_lead;
char	*tex_tail = " \\\\\n";
char	tex_flg = 0;
char	two_flg = 0;
char	debug = 0;
int	record_count=0;
int	field_per=7;	    /* this many lines are printedper output line */
char	*fgets();

int	perc[NUM_MAX];	    /* HACK - these fields are to be * by 100. */

char	*progname;	    /* Name of program (as invoked)	*/

main(argc, argv)
int	argc;
char	*argv[];
{
	lstat	stats[MAX_STATS];	/*   details for statistics  */
	int	last_stats=0;		/*   next free field   */
	int	numbers[NUM_MAX];		/*   temp storage for input */
	int	nos, i;
	char	*filename;
	char    brief = 0;
	char	file_diff = 0;		/*   take second field from second file */

        extern  int     optind; /* Argument processing variables. */
        extern  char    *optarg;
	int	c;

	perc[0] = -1;
		/* Process command line arguments */
        if ((progname = strrchr(argv[0], '/')) == NULL || *++progname == '\0')
                progname = argv[0];
	if (argc < 3)
		uerror(usage, progname);
        while ((c = getopt(argc, argv, "u:da:v:s:A:V:S:C:t:l:eFp:f:T:2q:b")) != EOF) {
	    if ( last_stats >= MAX_STATS )
			uerror("too many statistics required","");
            switch (c)
            {
            case 'f':		/*	setup precision for printing floats  */
		if ( sscanf(optarg,"%d", &i) < 1 )
			uerror("couldn't read in integer","");
		sprintf(print_prec,"%%.%1dlf", i) ;
		break;
            case 'p':		/*	print these fields as percents */
		perc[str_pintarr(optarg, perc, NUM_MAX, ",")] = -1;
		for ( i=0; perc[i]>=0; i++ )
			perc[i]--;
		break;
            case 'U':		/*  mean & means std of diff. between 1st & subsequ */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=1; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				D_MVAR, numbers[0], numbers[i]);
                break;
            case 'V':		/*   print mean & std of diff. between 1st & subsequ */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=1; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				D_VAR, numbers[0], numbers[i]);
                break;
            case 'S':		/*   print mean & t of diff. between 1st & subsequ */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=1; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				D_SVAR, numbers[0], numbers[i]);
                break;
             case 'C':		/*   print mean & std & t & F between 1st & subsequ */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=1; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				D_CVAR, numbers[0], numbers[i]);
                break;
             case 'A':		/*   print mean of diff. between 1st & subsequ */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=1; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				D_AVER, numbers[0], numbers[i]);
                break;
            case 'a':		/*   print average for fields listed   */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=0; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				S_AVER, numbers[i], numbers[i]);
                break;
            case 'u':		/*    print mean & means std for these */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=0; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				S_MVAR, numbers[i], numbers[i]);
                break;
            case 'v':		/*    print mean & std for these fields   */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=0; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				S_VAR, numbers[i], numbers[i]);
                break;
            case 'q':		/*    set items per line of output to print */
		if ( sscanf(optarg,"%d", &field_per) < 1 )
			uerror("couldn't read in integer","");
               break;
            case 's':		/*    print mean & t for these fields   */
		nos = str_pintarr(optarg, numbers, NUM_MAX, ",");
		for (i=0; i< nos; i++) 
		     init_lstat(stats+last_stats++,
				S_SVAR, numbers[i], numbers[i]);
               break;
	    case 'F':		/*   differentiate first file vs. others */
		file_diff++;
		break;
	    case 't':		/*  specify field sep.  */
		strcpy(field_sep,optarg);
		break;
	    case 'n':		/*  specify printing tag   */
		if ( last_stats )
			stats[last_stats-1].tag = optarg;
		break;
            case 'T':		/*    TeX compatable output   */
		if ( tex_flg )
			tex_tail = optarg;
		else
			tex_lead = optarg;
		tex_flg++;
            case '2':		/*    use two tailed t-test   */
		two_flg++;
                break;
            case 'b':	
		brief++;
                break;
            case 'd':		/*    verbose   */
		debug++;
                break;
            default:
                uerror(usage, progname);
            }
	}
	stats[last_stats].type = S_NONE;
	if ( tex_flg)
		printf("%s",tex_lead);
	if ( file_diff ) {
	    filename = argv[optind++];
	    while ( argc > optind ) {
		make_stats_diff(stats, filename, argv[optind]);
		if ( !tex_flg )
			printf("Files:   %s    %s (%d):  ",
				filename, argv[optind], record_count);
		report_stats(stats);
		if ( !tex_flg ) putchar('\n');
		optind++;
	    }
	} else 
	    while ( argc > optind ) {
		make_stats(stats,  argv[optind]);
		if ( !tex_flg && !brief )
			printf("%s (%d):  ", argv[optind], record_count);
		report_stats(stats);
		if ( !tex_flg ) putchar('\n');
		optind++;
	    }
	if ( tex_flg )
		printf(tex_tail);
 	exit(0);
}

make_stats(stats, filename)
lstat	*stats;
char	*filename;
{
	FILE	*infp;
	int	i;
	int	ln=0;			/*   current number of lines */
	int	fn;			/*   current number of fields */
	double	field[MAX_FIELDS];	/* contents of one line   */
	char	line[2000];		/* will be odd behaviour if overflow */
	int	f1, f2;
	double	temp;
	int	bad_length=0;
	int	line_len = -1;

	record_count = 0;
	zero_stats(stats);
	/*
	 *	"-"  is stdin
	 */
	if ( strcmp(filename,"-") )
		infp = efopen(filename,"r");
	else
		infp = stdin;
	for (i=0; stats[i].type != S_NONE; i++) {
	  stats[i].ignore = FALSE;	
	}
	/*
	 *	collect statistics from file
	 */
	while ( fgets(line, 2000, infp)  ) {
		/*
		 *	read line into "field" array
		 */
		ln++;
		if ( !( fn=linetodbls(line,field,ln) ) )
			continue;
		if ( line_len < 0 ) {
			line_len = fn;
			for (i=0; stats[i].type != S_NONE; i++) {
		    		if ( stats[i].f1 >= fn ||
		    			stats[i].f2 >= fn )
				stats[i].ignore = TRUE;	
			}
		}
		if ( fn != line_len ) {
			bad_length++;
			continue;
		}
		record_count++;
		for (i=0; perc[i]>=0; i++)
			field[perc[i]] *= 100.0;
		/*
		 *	now update statistics
 		 */
		for (i=0; stats[i].type != S_NONE; i++) {
		    if ( stats[i].ignore )
			continue;
		    f1 = stats[i].f1;
		    f2 = stats[i].f2;
		    switch( stats[i].type ) {
		    case S_MVAR:
		    case S_VAR:
		    case S_SVAR:
			stats[i].val2 += field[f1]*field[f1];
		    case S_AVER:
			stats[i].val1 += field[f1];
			break;
		    case D_VAR:
		    case D_MVAR:
		    case D_SVAR:
			temp = field[f1]-field[f2];
			stats[i].val1 += temp;
			stats[i].val2 += temp*temp;
			break;
		    case D_AVER:
			stats[i].val1 += field[f1]-field[f2];
			break;
		    case D_CVAR:
			stats[i].val1 += temp = field[f1];
			stats[i].val2 += temp*temp;
			stats[i].val3 += temp = field[f2];
			stats[i].val4 += temp*temp;
			break;
		    default:
			error("internal error, unknown stats type","");
		    }			
		}
	}
	if ( infp != stdin ) 
		fclose(infp);
	if ( bad_length )
	    fprintf(stderr, "some lines had wrong length so were skipped\n");
}

make_stats_diff(stats, filename1, filename2)
lstat	*stats;
char	*filename1, *filename2;
{
	FILE	*infp1, *infp2;
	int	i;
	int	ln=0;			/*   current number of lines */
	int	fn1;			/*   current number of fields */
	int	fn2;			/*   current number of fields */
	double	field1[MAX_FIELDS];	/* contents of one line   */
	double	field2[MAX_FIELDS];	/* contents of one line   */
	char	line1[2000];		/* will be odd behaviour if overflow */
	char	line2[2000];		/* will be odd behaviour if overflow */
	int	f1, f2;
	double	temp;
	int	read1, read2;
	int	bad_length=0;
	int	line_len1 = -1;
	int	line_len2 = -1;

	record_count=0;
	zero_stats(stats);
	/*
	 *	"-"  is stdin
	 */
	if ( strcmp(filename1,"-") )
		infp1 = efopen(filename1,"r");
	else
		infp1 = stdin;
	infp2 = efopen(filename2,"r");
	for (i=0; stats[i].type != S_NONE; i++) {
	  stats[i].ignore = FALSE;	
	}
	/*
	 *	collect statistics from file
	 */
	for ( ; ; ) {
		read1 = (!fgets(line1, 2000, infp1));
		read2 = (!fgets(line2, 2000, infp2));
		if ( read1 || read2 )
			break;
		/*
		 *	read lines into "field" arrays
		 */
		ln++;
		if ( !( fn1=linetodbls(line1,field1,ln) ) )
			continue;
		if ( !( fn2=linetodbls(line2,field2,ln) ) )
			continue;
		if ( line_len2 < 0 ) {
			line_len1 = fn1;
			line_len2 = fn2;
			for (i=0; stats[i].type != S_NONE; i++) {
		    		if ( stats[i].f1 >= fn1 ||
		    			stats[i].f2 >= fn2 )
				stats[i].ignore = TRUE;
			}
		}
		if ( fn1 != line_len1 || fn2 != line_len2 ) {
			bad_length++;
			continue;
		}
		record_count++;
		for (i=0; perc[i]>=0; i++) {
			field1[perc[i]] *= 100.0;
			field2[perc[i]] *= 100.0;
		}
		/*
		 *	now update statistics
 		 */
		for (i=0; stats[i].type != S_NONE; i++) {
		    if ( stats[i].ignore )
			continue;
		    f1 = stats[i].f1;
		    f2 = stats[i].f2;
		    switch( stats[i].type ) {
		    case S_VAR:
		    case S_MVAR:
		    case S_SVAR:
		    case D_VAR:
		    case D_MVAR:
		    case D_SVAR:
			temp = field1[f1]-field2[f2];
			if ( debug )
			    printf("%lf,",temp);
			stats[i].val1 += temp;
			stats[i].val2 += temp*temp;
			break;
		    case S_AVER:
		    case D_AVER:
			stats[i].val1 += field1[f1]-field2[f2];
			break;
		    case D_CVAR:
			stats[i].val1 += temp = field1[f1];
			stats[i].val2 += temp*temp;
			stats[i].val3 += temp = field2[f2];
			stats[i].val4 += temp*temp;
			break;
		    default:
			error("internal error, unknown stats type","");
		    }			
		}
	        if ( debug ) printf("\n");
	}
	if ( infp1 != stdin ) fclose(infp1);
	fclose(infp2);
	if ( !read1 || !read2 )
		fprintf(stderr, "files of unequal length\n");
	if ( bad_length )
	    fprintf(stderr, "some lines had wrong length so were skipped\n");
}

report_stats(stats)
lstat	*stats;
{
	int	i;
	double	std_dev, dev, dev2, sqrt(), st;
	double  stud_t(), F_dist();
	if ( record_count <= 1 ) {
		printf("fewer than two records processed\n");
		return;
	}
	for (i=0; stats[i].type != S_NONE; i++) {
	    if ( stats[i].ignore ) 
		continue;
	    if ( i )
		if ( !(i%field_per) )
		    printf("\n\t\t");
	        else
	    	    printf("   ");
	    if ( tex_flg )
			printf(" & ");
	    printf("%s", stats[i].tag);
	    switch( stats[i].type ) {
	    case S_AVER:
	    case D_AVER:
		printf(print_prec, (float)stats[i].val1/record_count );
		break;
	    case D_VAR:
	    case D_MVAR:
	    case S_VAR:
	    case S_MVAR:
		std_dev = record_count*stats[i].val2 - stats[i].val1*stats[i].val1;
		std_dev = sqrt( std_dev/record_count/(record_count-1) );
		printf(print_prec, (float)stats[i].val1/record_count );
		if ( tex_flg )
			printf(" $\\pm$ ");
		else
			printf("~");
		if ( stats[i].type==S_VAR || stats[i].type==D_VAR )
			printf(print_prec, (float)std_dev );
		else
		    printf(print_prec, 
				(float)(std_dev/sqrt((double)record_count)) );
		break;
	    case D_SVAR:
	    case S_SVAR:
		std_dev = record_count*stats[i].val2
				- stats[i].val1*stats[i].val1;
		std_dev = sqrt( std_dev/record_count/(record_count-1) );
		printf(print_prec, (float)stats[i].val1/record_count );
		if ( tex_flg )
		    printf(" (");
		else
		    printf(", t_{%d} = ", record_count-1 );
		if ( std_dev == 0.0 ) {
		    if ( two_flg )
		        printf(print_prec,  (float) 0.0 );
		    else
		        printf(print_prec,  (float) 0.5 );
		} else {
		    st = stats[i].val1/std_dev/sqrt((double) record_count);
		    if ( debug ) printf("(%lf,%d)", st, record_count-1);
		    st = stud_t( st, record_count-1);
		    if ( !two_flg )
			st /= 2.0;
		    st = 1.0 - st;
		    if ( st > 0.005 )
			st -= 0.005;
		    printf(print_prec,  (float) st );
		}
		if ( tex_flg )
		    printf(")");
		break;
	    case D_CVAR:
		dev = record_count*stats[i].val2 - stats[i].val1*stats[i].val1;
		dev2 = record_count*stats[i].val4 - stats[i].val3*stats[i].val3;
		dev /= record_count*(record_count-1);
		dev2 /= record_count*(record_count-1);
		printf(print_prec, (float)stats[i].val1/record_count );
		printf(", " );
		printf(print_prec, (float)stats[i].val3/record_count );
		if ( tex_flg )
			printf(" $\\pm$ ");
		else
			printf("~");
		printf(print_prec, (float)sqrt( (dev + dev2) / record_count) );
		if ( tex_flg )
		    printf(", $t_{%d}$ = ", 2*record_count-2 );
		else
		    printf(", t_{%d} = ", 2*record_count-2 );
		printf(print_prec, (float)
			stud_t( (stats[i].val1-stats[i].val3) /
				sqrt((dev + dev2) * record_count),
				2*record_count-2 ) );
		if ( tex_flg )
		    printf(", $F_{%d,%d}$ = ", record_count-1, record_count-1);
		else
		    printf(", F_{%d,%d} = ", record_count-1, record_count-1);
		printf(print_prec, (float)
			F_dist(dev/dev2, record_count-1, record_count-1) );
		break;
	    default:
			error("internal error, unknown stats type","");
	    }	
	}
}

zero_stats(stats)
lstat	*stats;
{
	int	i;
	for (i=0; stats[i].type != S_NONE; i++) {
		stats[i].val1 = 0.0;
		stats[i].val2 = 0.0;
		stats[i].val3 = 0.0;
		stats[i].val4 = 0.0;
	}
}


/*
 *	line should be list of floats separated by "field_sep"
 *	convert to array "field", and return no. read, terminating with 0.0
 */
int
linetodbls(line,field,ln)
char	*line;
double	*field;
int	ln;
{
	int	fn = 0;
	char	*citem, *strtok();
	for (	citem = strtok(line, field_sep);
		citem;
		citem = strtok(0, field_sep) ) {
	    if ( strcmp(citem, "\n")==0 )
		break;
	    if ( sscanf(citem,"%lf", &field[fn++]) <= 0 ) {
		fprintf(stderr, "Line %d has problems\n", ln);
		return 0;
	    }
	}
	field[fn] = 0.0;
	return fn;
}

/*
 *	initialise and check a "stat" entry
 */
init_lstat(ls, type,f1,f2)
lstat	*ls;
int	f1, f2;
int	type;
{
	ls->tag = "";
	ls->type = type;
	ls->ignore = FALSE;
	ls->f1 = f1-1;
	ls->f2 = f2-1;
	if (	ls->f1 < 0 || ls->f2 < 0 ||
		ls->f1 >= MAX_FIELDS || ls->f2 >= MAX_FIELDS )
	    uerror("field numbers in option argument out of range\n","");
}

/*
 *      read integer array as "sep" separated list from string "str", return siz
e
 */
int str_pintarr(str, arr, max, sep)
char    *str;
int     *arr, max;
char    *sep;
{
        int     count=0, this, last;
        char    *field, *strtok();

        for (   field = strtok(str,sep);
                field;
                field = strtok(0,sep)   ) {
            if ( count >= max )
                error("input list too large","");
	    if ( strchr(field,'-') ) {
		/* 
		 *	this is a range
		 */
		if ( sscanf(field,"%d-%d", &arr[count++], &last) <= 1 )
                	error("can't convert string to integer list","");
		for (this = arr[count-1]+1 ; this<=last; count++,this++ )
			arr[count] = this;
	    } else 
		if ( sscanf(field,"%d", &arr[count++]) <= 0 )
                	error("can't convert string to integer list","");
	    else
		;
	}
        return count;
}


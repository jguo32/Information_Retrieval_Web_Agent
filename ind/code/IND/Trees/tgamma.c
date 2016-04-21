#include <stdio.h>
#include <string.h>
#define NOOPT

#ifdef NOOPT
/*
 *      read double array as "sep"-separated list from "optarg", return size
 */
int opt_dblarr(arr, max, sep)
double  *arr;
int     max;
char    *sep;
{
        extern  char    *optarg;
        int     count=0;
        char    *field, *strtok();

        for (   field = strtok(optarg,sep);
                field;
                field = strtok(0,sep)   )
            if ( count >= max || sscanf(field,"%lf", &arr[count++]) <= 0 )
                exit(1);
        return count;
}
#endif

main(argc, argv)
int     argc;
char    *argv[];
{
        int     c;
        int  n=0;
        double  a=1.0;
        double  A[20];
        double  N[20];
        int     i;
        double  gamma(), atof();
	double  lg, totA, totAN;

        extern  int     optind; /* Argument processing variables. */
        extern  char    *optarg;

        while ((c = getopt(argc, argv, "n:a:A:")) != EOF)
                switch (c)
                {
                case 'n':
			sscanf(optarg,"%d", &n );
			for (i=0; i<n; i++ )
				A[i] = 0.5;
			break;
		case 'a':
			if (n<=0 ) exit(1);
			sscanf(optarg,"%lf", &a );
			for (i=0; i<n; i++ )
				A[i] = a/n;
			break;
		case 'A':
			opt_dblarr(A,20,",");
		default:
 			exit(1);
		}
	for ( i=0; i<n ; i++) 
		N[i] = atof(argv[optind+i]);
	totA = totAN = lg = 0.0;
	for ( i=0; i<n; i++)  {
		printf("(%lg,%lg) ",A[i],N[i]);
		lg += gamma(A[i]+N[i]);
		lg -= gamma(A[i]);
		totA += A[i];
		totAN += A[i]+N[i];
	}
	printf("= (%lg,%lg) ",totA, totAN-totA );
	lg -=  gamma(totAN) - gamma(totA);
	printf( " gives %lg\n", lg);

}
	

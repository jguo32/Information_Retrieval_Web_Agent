/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include	<stdio.h>
#include	<math.h>

#define expm_SIZE 1000
static double expm_tbl[expm_SIZE];
static double expmi_tbl[100];

/*
 *	linear interpolation of prestored table
 */
double
expm(x)
register double x;
{
	register int	xindex, xint;
	double	exp();

	if ( x <= 0.0 )
	    return 1.0;
	if ( x > 99.9 )
	    return 0.0;
	if ( !expm_tbl[1] ) {
	    for( xindex=expm_SIZE-1; xindex>=0; xindex--) {
		expm_tbl[xindex] = exp( -((double)(xindex)) / expm_SIZE);
	    }
	    for( xindex=99; xindex>=0; xindex--) {
		expmi_tbl[xindex] = exp(-(double)xindex);
	    }
	}
	xint = x;   x -= xint;
	x *= expm_SIZE;
	xindex = x;    x -= xindex;
	return expmi_tbl[xint] *
		( x*expm_tbl[xindex+1] + (1.0-x)*expm_tbl[xindex] );
}    


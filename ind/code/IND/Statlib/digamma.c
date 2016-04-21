/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include  <math.h>
#include  <stdio.h>
#include  "Lib.h"

/*
 *	from Abramowitz and Stegun
 */
double   digamma(x)
register double	x;
{
	static	double	a[] = {-12.0,120.0,-252.0,240.0,-122.0};
			/*  =  $(2(i+1))!/B_{2*(i+1)}$ , Bernoulli numbers  */
	double	val=0.0,xpow;
	int	i;

	if ( x<=0.0 ) error("Negative argument in digamma","");
	/*
	 *    use  6.3.6 to prepare value for next summation
	 */
	while(x<=6.0) 
		val -= 1.0/x++;
	/*
	 *    use  6.3.18, good for 10 decimal places when x > 6.0
	  */	
	val += log(x)-0.5/x;
	for(x*=x,xpow=x,i=0; i<4; i++,xpow*=x)
		val += 1.0/xpow/a[i];
	return val;
}

/*
 *	derivative of above, same source
 */
double   di2gamma(x)
register double	x;
{
	static	double	a[] = {1.0,6.0,-30.0,42.0,-30.0,13.2};
			/*  =  something about Bernoulli numbers  */
	double	val=0.0,xpow,xsq;
	int	i;

	if ( x<=0.0 ) error("Negative argument in di2gamma","");
	/*
	 *    use  6.4.6 to prepare value for next summation
	 */
	while(x<=7.0) {
		val += 1.0/(x*x);
		x++;
	}
	/*
	 *    use  6.4.12, good for ? decimal places when x > 7.0
	  */	
	xsq = x*x;
	val += 0.5/xsq;
	for(xpow=x,i=0; i<5; i++,xpow*=xsq)
		val += 1.0/xpow/a[i];
	return val;
}


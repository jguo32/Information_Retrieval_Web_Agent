#include <stdio.h>
#include <math.h>
#include "Lib.h"
#define	ACCUR	3.0e-7


/*
 *	Calculates Incomplete Beta values from 
 *	 Abramowitz and Stegun, 26.5.7, 26.5.8
 */

double betai(a,b,x)
double a,b,x;
{
	double	fact;
	double   _betai();

	/*
	 *    check arguments
	 */
	if ( a<=0.0 || b <= 0.0 || x<0.0 || x>1.0 ) 
		error("betai:  domain error","");
	if ( x==0.0 )
	  return 0.0;
	else if ( x==1.0 )
	  return 1.0;
	/*
	 *    calculate factor
	 */
	fact = exp(gamma(a+b)-gamma(a)-gamma(b)+a*log(x)
			+b*log(1.0-x));
	/*
	 *    use 26.5.7 to split into symmetric cases for continued fraction
	 */
	if ( x < (a+1.0)/(a+b+2.0) )
		return  fact * _betai(a,b,x)/a;
	else
		return 1.0 - fact * _betai(b,a,1.0-x)/b;
}


extern double
_betai(a,b,x)
double	a,b,x;
/*
 *	continued fraction for incomplete beta function, Abram&Stegun 26.5.8
 */
{
	double	qap,qam,qab,em,tem,d;
	double	bz, bm=1.0,bp,bpp;
	double	az=1.0,am=1.0,ap,app,aold;
	int	i;

	qab = a+b;
	qap = a+1.0;
	qam = a-1.0;
	bz = 1.0-qab*x/qap;
	for ( i=1; i<=100; i++) {
		em = (double)i;
		tem = em+em;
		d = em*(b-em)*x/((qam+tem)*(a+tem));
		ap = az+d*am;
		bp = bz+d*bm;
		d = -(a+em)*(qab+em)*x/((qap+tem)*(a+tem));
		app = ap+d*az;
		bpp = bp+d*bz;
		aold = az;
		am = ap/bpp;
		bm = bp/bpp;
		az = app/bpp;
		bz = 1.0;
		if ( fabs((az-aold)/az) < ACCUR )
			return az;
	}
	error("loop overran in _betai()","");
	return 0.0;
}


#ifdef OLDBETA

/*
 *	Calculates Beta values using Abramowitz & Stegun 26.5.21
 *	Calculates N_{0,1}(Z) using Abrabowitz & Stegun 26.2.18
 */
#define c1 0.196854
#define c2 0.115194
#define c3 0.000344
#define c4 0.019527

double betai1(a,b,x)
double a,b,x;
{
	double exp(), sqrt(), pow(), log();
	double y,w1,w2;
	if ( (a + b - 1.0) * (1.0 - x)  >= 0.8 ) {
	    /*	   
	     *           use 26.5.21   
	     */
	    if (a<1.0) a = 1.0;
	    w1 = pow(b*x,1.0/3.0);
	    w2 = pow(a*(1-x),1.0/3.0);
	    y = 3.0 * ( w1*(1.0-1.0/9.0/b) - w2*(1.0-1.0/9.0/a) )
			/ sqrt(w1*w1/b + w2*w2/a) ;
	    if (y>= 0.0)
	        return 1.0 -
		  0.5*pow((1.0 + y*(c1 + y*(c2 + y*(c3 + c4*y)))),-4.0);
	    else
	        return 
		   0.5*pow((1.0 - y*(c1 - y*(c2 - y*(c3 - y*c4)))),-4.0);
	} else {
		fprintf(stdrep,"Beta: domain error, a %lg, b %lg, x %lg\n",
			a,b,x);
		return 1.0;
	}
}
#endif

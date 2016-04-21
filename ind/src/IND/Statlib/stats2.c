/*
 *           various routines copied from somewhere
 */
#include <math.h>
/*
 *	Student's t
 *	returns significance at which "means equal" rejected
 *	(smaller is better rejection)
 */

double    stud_t(t,v)
double	t;
int	v;
{
	double betai();
	if ( t < 0.001 && t > -0.001 )
	    return 1.0;
	else
	    return  betai( v*0.5, 0.5, v/(v+t*t) );
}

/*
 *	F-distribution
 *	returns significance at which "1 has smaller variance than 2" rejected
 *	(smaller is better rejection)
 */
double    F_dist(F,v1,v2)
double	F;
int	v1, v2;
{
	double betai();
	return  betai( v1*0.5, v2*0.5, v2/((double)v2+v1*F) );
}
	
/*
 *	binomial distribution
 *	probability of success in k or more trials out of n
 */
double    binop(n,k,p)
double	p;
int	n, k;
{
	double betai();
	return  betai( (double)k, (double)(n-k+1), p );
}



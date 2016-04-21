/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include	<stdio.h>
#include	<math.h>
#include	"Lib.h"

/*
 *	fancy tabular routines for loggamma, logbeta, etc.
 */

#define GAMMA_SIZE 400
#define STEP 2.0
#define gammaptr(x)  (loggamma_tbl+(int)((x)*STEP+.00001))
#define gammaval(x) (x> GAMMA_SIZE/STEP)? gamma((double)x): \
		     (*(ptr = gammaptr(x)) ) ? *ptr : \
			(*ptr = gamma((double)(((int)((x)*STEP+0.00001))/STEP)))
static double loggamma_tbl[GAMMA_SIZE];
double	lbalpha=0.5;
double	gamma();

/***********************************************************************/
/*
 *	loggamma() -- To save time, values of gamma n/STEP! are
 *		stored for n = 0, 1, ..., size - 1.
 */
double
loggamma(n,m)
float n;
int	m;
{
	register double	*ptr;
	n+=m*lbalpha;
	return gammaval(n);
}    

/***********************************************************************/
/*
 *	logbeta() -- uses gamma table
 *		
 */
double
ggamma(n)
double n;
{
	return gamma(n);
}    
/***********************************************************************/
/*
 *	logbeta() -- uses gamma table
 *		
 */
double
logbeta(n,m)
float n,m;
{
	register double	*ptr, val=0.0;
	n+=lbalpha;
	m+=lbalpha;
	if (n<0 || m<0)
		error("negative argument to logbeta","");
	val += gammaval(n);
	val += gammaval(m);
	val -= gammaval(m+n);
	return val;
}    

/***********************************************************************/
/*
 *	logbetal() -- first argument is list of numbers
 *
 */
double
logbetal(nlist,size)
float	*nlist;
int	size;
{
	register double	*ptr, val=0.0, ntot=0.0;
	while ( size-->0 ) {
	    *nlist += lbalpha;
	    if (*nlist<0)
		error("negative argument to logbeta","");
	    val += gammaval(*nlist);
	    ntot += *nlist++;
	}
	val -= gammaval(ntot);
	return val;
}    

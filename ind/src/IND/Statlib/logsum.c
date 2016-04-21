/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *	return  log( exp(x) + exp(y) )
 */
double
logsum(x,y)
register double	x,y;
{
	double	exp(), log();
	/*
	 *	should really tabulate log(1+z) to speed
  	 */
	if ( x > y )
	    if ( x-y > 20.0 )
		return x;
	    else
		return x + log(1.0+exp(y-x));
	else
	    if ( y-x > 20.0 )
		return y;
	    else
		return y + log(1.0+exp(x-y));
}

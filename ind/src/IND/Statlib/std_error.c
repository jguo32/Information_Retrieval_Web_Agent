/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

float std_error(c,n)
float c,n;
{
        /* 
         *  returns standard error of binomial  ( n = total )
         */ 
        extern double sqrt();
        if ( c > n || n == 0.0 || c<0 )
                return 0.0;
        else
                return ((float) sqrt((double)c*(n-c)/n));
}

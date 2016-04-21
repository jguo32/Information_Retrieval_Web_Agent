/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <math.h>
#include "DEFN.h"

/*
 *      usual procedure for generating random normal deviates
 */
  
double normrandom()
{
  double X1;
  static double X2;
  double V1,V2,S,temp;
  static char flag = 0;
  double	log(), sqrt();
  
    if (flag == 1)
      {
        flag =0;
        X1 = X2;
      } else {
        do
	  { 
	    V1 = (2.0 * frandom()) -1.0;
	    V2 = (2.0 * frandom()) -1.0;
	    S = V1*V1 + V2*V2;
	  }
        while ( S >= 1.0 );
	
        X1 = V1 * (temp=sqrt(-2.0 * log(S)/S)) ;
        X2 = V2 * temp;
        flag = 1;
      }
  return(X1);
}

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "Lib.h"


/****************************************************************************/
/*
 *	shuffle(l,s)	-  shuffle vector of data
 */
shuffle(ptr,length,size)
char 	*ptr;  			/*  data to shuffle  */
int	length;		        /*  no. of data items  */
int	size;			/*  size of each data item  */
{
#define BUFSIZE	100
	register  int     i,j,index;
	register  char	  *p1, temp;
	for (i = 0; i < length; i++, ptr+=size ) {
		index = random()%(length-i);
		for (j=size-1, p1 = ptr+size*index; j>=0; j--) {
			temp = p1[j];
			p1[j] = ptr[j];
			ptr[j] = temp;
		}
	}	
}



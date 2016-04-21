/*
 *   IND 1.0 released 9/15/91   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 *
 */

#include <stdio.h>
#include <Lib.h>

/*
 *	read integer array as "sep" separated list from string "str", return size
 */
int str_intarr(str, arr, max, sep)
char    *str;
int	*arr, max;
char	*sep;
{
	int	count=0;
	char	*field, *strtok();

	for (	field = strtok(str,sep);
	     	field;
		field = strtok((char*)0,sep)   ) 
	    if ( count >= max || sscanf(field,"%d", &arr[count++]) <= 0 )
	        error("can't convert string to integer list","");
	return count;
}


/*
 *	read double array as "sep"-separated list from "str", return size
 */
int str_dblarr(str, arr, max, sep)
char    *str;
double	*arr;
int	max;
char	*sep;
{
	int	count=0;
	char	*field, *strtok();

	for (	field = strtok(str,sep);
	     	field;
		field = strtok((char*)0,sep)   ) 
	    if ( count >= max || sscanf(field,"%lf", &arr[count++]) <= 0 )
	        error("can't convert string to double list","");
	return count;
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"

/*
 *      read file and check for magic number, if its
 *	encoded then return true, else return false
 */
int
encoded_file(str)
char    *str;
{
	static 	FILE    *egfile;
	FILE    *efopen();
	int     magic_no;

	if ( !*str )
		return FALSE;
	egfile = efopen(str, "r");
        if (fread((char *)&magic_no,sizeof(int),1,egfile) != 1)
                uerror("Could not read header from example file", "");
	rewind(egfile);
	fclose(egfile);
	return magic_no == MAGICNO ;
}


/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *	efopen(file, mode) -- try to open file, otherwise print error
 *	message and die.
 */
#include <stdio.h>
#include "Lib.h"

FILE *efopen(file, mode)
char *file, *mode;
{
	FILE *fp, *fopen();
	char errmsg[100];

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	sprintf(errmsg, "Can't open file %s with mode %s", file, mode);
	uerror(errmsg, "efopen");
	return 0;
}

int  ezopened = 0;	/*   set to 1 if last open was with popen()  */

/*
 *	like efopen(), but attempts to zcat file if can't open
 */
FILE *ezopen(file, mode)
char *file, *mode;
{
	FILE *fp, *fopen(), *popen();
	char buf[100];

        ezopened = 0;
	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	if ( !strcmp(mode,"r") ) {
            ezopened = 1;
            sprintf(buf,"zcat %s",file);
            if ( (fp = popen(buf,mode)) != NULL)
		return fp;
	}
	sprintf(buf, "Can't zcat or open file %s with mode %s",
			file, mode);
	uerror(buf, "efopen");
	return 0;
}

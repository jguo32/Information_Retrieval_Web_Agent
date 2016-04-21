/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* 
 *   error_LF() -- print error message and die 
 *                 passing 0,0 for line file means these aren't
 *		   printed
 */

#include <stdio.h>
void error_LF(s1, s2,line,file)
char	*s1, *s2;
int    line;
char    *file;
{
	extern	int	errno, sys_nerr;
	extern	char	*sys_errlist[], *progname;

	fprintf(stderr, "\n");
	if (progname)
		fprintf(stderr, "%s: ", progname);
	if ( line && *file )
		fprintf(stderr, "line %d in file %s: ", line, file);
	fprintf(stderr, s1, s2);
	if (errno > 0 && errno < sys_nerr)
		fprintf(stderr, " (%s)", sys_errlist[errno]);
	fprintf(stderr, "\n");
	myexit(1);
}

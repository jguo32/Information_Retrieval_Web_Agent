/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#define TREE_c
#include <stdio.h>
#include "SYM.h"

char	Uflg = 1;
bool	Zflg = FALSE;
bool    smoothflag = FALSE;
int verbose = 0;

tree_opts(c,optarg)
int	c;
char * optarg;
{
	int			 i;
	switch (c)
	{
	case 'Z':
		Zflg++;
		break;
	case 'U':
		if ( sscanf(optarg,"%d",&i) <=0 ||
		     i < 1 || i > 6 )
 			uerror ("incorrect argument for option 'U'\n","");
		Uflg = i;
		break;
	default:
 		error("incorrect option for tree_opts","");
	}
}

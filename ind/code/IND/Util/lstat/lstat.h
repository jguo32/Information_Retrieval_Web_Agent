/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */


/*  lstat.h -- header file for line by line stats generation
 *
 *	Author - Wray Buntine, 1989
 *
 */

/* Array bounds */

#define  MAX_STATS  100
#define  MAX_FIELDS  100

/*  
 *        Attribute descriptors 
 *        to find out what these stand for, see how they are
 *        set in options processing in main.c
 */
#define S_AVER	0
#define S_VAR	1
#define S_SVAR	2
#define D_AVER	3
#define D_VAR	4
#define D_SVAR  5
#define D_CVAR  6
#define S_NONE	7
#define S_MVAR	8
#define D_MVAR	9
#define L_NULL	10

typedef	struct	lstat	lstat;	/* type for single stats entry */

struct lstat	/* type for single stat entries */
{
	char	*tag;		/*  pretty printing      */
	char	type;		/*  in defines above      */
	char    ignore;         /*  set if field not in this file */
	int	f1, f2;		/*  field indexes to use  */
	double	val1;		/*  accumulated value   */
	double	val2;		/*  accumulated value   */
	double	val3;		/*  accumulated value   */
	double	val4;		/*  accumulated value   */
};

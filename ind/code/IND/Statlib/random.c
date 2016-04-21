/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include "DEFN.h"

/*
 *      ?random()  stuff is all #defined to ?rand48() in "DEFN.h"
 *      which is perhaps a bit simple but adequate 
 *      our purposes and uniform from computer to computer
 */

#include <sys/types.h>
#ifdef AIX		/*  AIX don't have ftime()  */
#include <sys/times.h>
#else
#include <sys/timeb.h>
#endif
/*
 *	find a seed thats as near random as possible
 */
extern long time_random()
{
	long   seed;
#ifdef AIX
        struct tms timebuf;

        times(&timebuf);
        seed = (long) (timebuf.tms_utime + timebuf.tms_stime%1000000L);
#else
        struct  timeb timebuf;

        ftime(&timebuf);  
        seed = timebuf.millitm + timebuf.time%1000000L;
#endif
	return seed;
}

/*
 *	call "srandom" with a seed thats as near random as possible
 */
extern long rand_random()
{
	long seed = time_random();
	srandom(seed);
	return seed;
}


/*
 *	assumes "vec" is a probability vector of dim. "dim"
 *	randomly selects an index to the vector
 */
vrandom(vec, dim)
int	dim;
float	*vec;
{
	register	int	i;
	register	double	tot = 0.0;
	/*
	 *   normalise first to be sure
	 */
	for (i=0; i<dim; i++)
		tot += vec[i];
	tot *= frandom();
	for (i=0; i<dim; i++)
		if ( (tot-=vec[i]) <= 0.0 )
			break;
	return i%dim;
}

#ifdef DEBUG_RAND

long
random() 
{
  long  i;
  i = lrand48();
  printf("random() = %ld\n", i);
  return i;
}

double
frandom() 
{
  double  d;
  d = drand48();
  printf("frandom() = %lg\n", d);
  return d;
}

void
srandom(seed) 
long   seed;
{
  srand48(seed);
  printf("srandom(%ld)\n", seed);
}

#endif

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *	defined in  Lib
 */
#include <stdio.h>
double	**make_dmtx();
double	*make_dvec();
float	**make_fmtx();
float	*make_fvec();
float	*copy_fvec();
int	**make_imtx();
int	*make_ivec();
float	dot_fmtx();
float	dot_fvec();
double	dot_dmtx();
double	dot_dvec();
double	*dot_dmv();
double	rtsafe();
FILE	*efopen();
FILE	*ezopen();
int	ezopened;
void	error_LF();
int	opt_int();
double	opt_dbl();
void    add_childproc(), sigterm_handler(), myexit(), kill_childproc();
char  *new_hash(), *fill_hash();
/*
 *	defined in unix libraries
 */
char	*strcat(); 
char	*strcpy(); 
char	*strchr(); 
double	sqrt(), log(), exp(), gamma();
double  ceil(), floor();
void    *memcpy();
void    myexit();
extern  float	*int_log;
void	init_int_log();

/*
 *
 *	useful defns.
 *
 */

#define	bool	unsigned char
#define	byte	unsigned char
#define	word	unsigned int
#define	string	char *

#ifndef	FLOATMAX
#define FLOATMAX  1.7e38
#endif
#define LOG2	.693147180559
#define LOG3	1.09861228866
#define EMPTY	0
#define TRUE	1
#define FALSE	0
#define Max(a, b)	((a) > (b)? (a) : (b))
#define Min(a, b)	((a) < (b)? (a) : (b))
#define Abs(a)	        ((a<0)? -a : a )
#define forless(i,val)	for (i= (val)-1; i>=0; i--)
#define ASSERT(c) 
/*
#define ASSERT(c) { if (!(c)) error(" assertion error for c ",""); }
 */

#define  stdrep stdout
/*
 *	error() is for system like errors
 *	uerror() largely for user misuse errors
 */
#define  error(s1,s2)   error_LF(s1,s2,__LINE__,__FILE__)
#define  uerror(s1,s2)   error_LF(s1,s2,0,"")

/*
 *	these handle all the appropriate casteing and sizing
 */
#define mem_alloc(type) ((type *) salloc(sizeof(type)))
#define mems_alloc(type,size) ( (type *) salloc(sizeof(type)*(size)) )
#define mem_realloc(mem,type) ((type *) resalloc(mem,sizeof(type)))
#define mems_realloc(mem,type,size) ((type *)resalloc(mem,sizeof(type)*(size)))
#define sfree(mem)  safree((void *)(mem))

/*
 *	DEBUG_dealloc   =   on exit gives full report of memory
 *			    still allocated
 *	DEBUG_alloc     =   when runs out of memory, quits telling
 *			    where the request for memory was made
 */
/* #define DEBUG_dealloc  */
#ifdef DEBUG_dealloc
extern int debugging_alloc;
#define DEBUG_alloc  
#endif
#ifdef  DEBUG_alloc
#define salloc(s)	salloc_LF((int)s,__LINE__,__FILE__)
#define resalloc(m,s)       resalloc_LF((void*)m,(int)s,__LINE__,__FILE__)
void    *salloc_LF();
void    *resalloc_LF();
#else
#define salloc(s)	salloc_LF((int)s)
#define resalloc(m,s)       resalloc_LF((void*)m,(int)s)
void    *salloc_LF();
void    *resalloc_LF();
#endif

/*
 *	stuff for doing sampling,   see sample.c
 */
typedef struct Sampler Sampler;
struct Sampler
{
	unsigned char	s_type;		/*  codes, see sample.c */
        int     s_sample_size;  /*  sample size  */
        int     s_partitions;   /*  partitions for cross valid.  */
        int     s_partition;    /*  size of sample  */
        long    s_seed;         /*  seed for random sampling */
};
Sampler  *sample_opts();


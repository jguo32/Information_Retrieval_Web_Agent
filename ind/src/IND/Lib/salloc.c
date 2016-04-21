/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 * salloc() -- allocate memory; if not able to, print error message and die
 */
#include <stdio.h>
#include <Lib.h>

#ifdef __GNUC__
void     *realloc(), *malloc();
#else
char     *realloc(), *malloc();
#endif  /* __GNUC__ */


/*
 *	when out of memory
 *	if this is set, then call it and return 0
 *      else call usuall error() to abort
 */
void	(*salc_error)() = 0;

/*
 *	with DEBUG_alloc defined, this prints out line no. and file
 *	where the original call was made
 *
 *	with DEBUG_dealloc defined, this records details of memory
 *	alloc. and freeing, to enable printing of unfreed items
 *	at the end
 *
 *	for error handling -  only use in debugging!
 */
#ifdef DEBUG_dealloc
static struct as {
	int   size, line;
	char *file;
	void *mem;
}    *alloc_store=0;
static int	as_size=0;  as_max = 0;
char  *alloc_check_ptr=0;

record_alloc(size, line, file, ptr)
int  size, line;
char  *file;
void  *ptr;
{
	if ( !alloc_store ) {
		as_max = 100;
		alloc_store = (struct as *)malloc(as_max*sizeof(*alloc_store));
	} else if ( as_size >= as_max ) {
		as_max += 100;
		alloc_store = (struct as *)realloc(alloc_store,
					as_max*sizeof(*alloc_store));
	}
	alloc_store[as_size].size = size;
	alloc_store[as_size].line = line;
	alloc_store[as_size].file = file;
	alloc_store[as_size].mem = ptr;
	if ( alloc_check_ptr && alloc_check_ptr >=  ptr && (alloc_check_ptr-size) <= ptr )
		 error("found check_ptr","");
	as_size++;
}

rerecord_alloc(size, line, file, optr, ptr)
int  size, line;
char  *file;
void  *optr, *ptr;
{
	int i;
	int   found=0;
	for  (i=0; i<as_size; i++)
		if ( alloc_store[i].mem == optr ) {
			alloc_store[i].size = size;
			alloc_store[i].line = line;
			alloc_store[i].file = file;
			alloc_store[i].mem = ptr;
			found=1;
			break;
		}
	if ( ! found ) 
		error("freeing unallocated memory","");
}

unrecord_alloc(ptr)
void  *ptr;
{
	int i;
	int   found=0;
	for  (i=0; i<as_size; i++)
		if ( alloc_store[i].mem == ptr ) {
			as_size--;
			alloc_store[i].size = alloc_store[as_size].size;
			alloc_store[i].line = alloc_store[as_size].line;
			alloc_store[i].file = alloc_store[as_size].file;
			alloc_store[i].mem = alloc_store[as_size].mem;
			found=1;
			break;
		}
	if ( ! found ) 
		error("freeing unallocated memory","");
}

report_alloc()
{
  /*
   *   should sort into some time stamped order, and print out
   *   stuff differently
   */
	int i;
	printf("size\tline\tfile\t\tmem\n");
	for  (i=0; i<as_size; i++) {
		if ( alloc_store[i].mem )
		   printf("%d\t%d\t%s\t%d\n",
			alloc_store[i].size,
			alloc_store[i].line,
			alloc_store[i].file,
			alloc_store[i].mem );
		else
		   printf("--------- %s --------\n",
			alloc_store[i].file );
	}
}

int   debugging_alloc = 0;
#endif

#ifdef DEBUG_alloc
void *salloc_LF(size,line,file)
int size;
int line;
char  *file;
{
#else
void *salloc_LF(size)
int size;
{
#endif
#ifdef DEBUG_alloc
	char  buf[300];
#endif
	void *ptr;
	extern char *progname;
	static int s_out=0;
	if ( (ptr = (void *) malloc((unsigned int) size)) != 0)
#ifdef DEBUG_dealloc
	{
		if ( debugging_alloc )
			record_alloc(size,line,file,ptr);
		return ptr;
	}
#else
		return ptr;
#endif
	if ( salc_error ) {
	  if ( !s_out ) 
#ifdef DEBUG_alloc
	    fprintf(stderr,"%s: memory limit exceeded at line %d in file %s\n",
		progname,line,file);
#else
	    fprintf(stderr,"%s: memory limit exceeded\n",progname);
#endif
	  s_out = 1;
	  (*salc_error)();
	} else
#ifdef DEBUG_alloc
	{
		sprintf(buf,"out of memory at line %d in file %s",line,file);
		error(&buf[0],"");
	}
#else
		uerror("out of memory","");
#endif
        return (void *)0;

}

#ifdef DEBUG_alloc
void *resalloc_LF(mem,size,line,file)
void  *mem;
int size;
int line;
char  *file;
{
#else
void *resalloc_LF(mem,size)
void  *mem;
int size;
{
#endif
#ifdef DEBUG_alloc
	char  buf[300];
#endif
	void *ptr;
	extern char *progname;
	static int s_out=0;
	if ( !mem )
		error("attempting to realloc 0 pointer","");
	if ( (ptr = (void *) realloc(mem,(unsigned int) size)) != 0)
#ifdef DEBUG_dealloc
	{
		if ( debugging_alloc )
			rerecord_alloc(size, line, file, mem, ptr);
		return ptr;
	}
#else
		return ptr;
#endif
	if ( salc_error ) {
	  if ( !s_out )
#ifdef DEBUG_alloc
            fprintf(stderr,"%s: memory limit exceeded at line %d in file %s\n",
                progname,line,file);
#else
            fprintf(stderr,"%s: memory limit exceeded\n",progname);
#endif
	  s_out = 1;
	  (*salc_error)();
	} else
#ifdef DEBUG_alloc
        {
                sprintf(buf,"out of memory at line %d in file %s",line,file);
                error(&buf[0],"");
        }
#else
		uerror("out of memory","");
#endif
	return (void *)0;
}

safree(tt)
void *tt;
{
#ifdef DEBUG_dealloc
	if ( debugging_alloc )
        	unrecord_alloc(tt);
#endif
	free(tt);  
}

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <malloc.h>
#include <Lib.h>


display_alloc()
{
	struct  mallinfo  ma;

	ma = mallinfo();
	printf("total space in arena:        %d\n", ma.arena);	
	printf("number of ordinary blocks:   %d\n", ma.ordblks);	
	printf("number of small blocks:     %d\n", ma.smblks);	
	printf("number of holding blocks:     %d\n", ma.hblks);	
	printf("space in holding block headers:     %d\n", ma.hblkhd);	
	printf("space in small blocks in use:     %d\n", ma.usmblks);	
	printf("space in free small blocks:     %d\n", ma.fsmblks);	
	printf("space in ordinary blocks in use:     %d\n", ma.uordblks);	
	printf("space in free ordinary blocks:     %d\n", ma. fordblks);	
	printf("cost of enabling keep option:     %d\n", ma.keepcost);	
/* COMMENTS DUE TO COMPILER ERROR
"check_alloc.c", line 27: undefined struct/union member: mxfast
"check_alloc.c", line 28: undefined struct/union member: nlblks
"check_alloc.c", line 29: undefined struct/union member: grain
"check_alloc.c", line 31: undefined struct/union member: uordbytes
"check_alloc.c", line 32: undefined struct/union member: allocated
"check_alloc.c", line 34: undefined struct/union member: treeoverhead
cc: acomp failed for check_alloc.c
*/
/*	printf("max size of small blocks:     %d\n", ma.mxfast);	
	printf("number of small blocks in a holding block:     %d\n", ma.nlblks);
	printf("small block rounding factor:     %d\n", ma.grain);	
	printf("space (including overhead) allocated in ord. blks:     %d\n", 
				ma.uordbytes);	
	printf("number of ordinary blocks allocated:     %d\n", ma.allocated);
	printf("bytes used in maintaining the free tree:     %d\n", 
			ma.treeoverhead);	
*/
}



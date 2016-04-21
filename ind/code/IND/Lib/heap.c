/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* 
 *   heap rountines - maintain a balanced tree with ordering info where
 *			insertion = add one more
 *			extraction = remove the largest
 */

#include <stdio.h>
#include "Lib.h"

/*  #define TEST_HEAP    /* switch on for self contained debugging */

/*
 *	heap nodes
 */
typedef  struct heap heap;
struct heap {
	float 	best_subval;	/*  best value below */
	union {
		struct {
			struct  heap    *left, *right;
		} n;
		void    *valp;		/*  ptr. to item stored in heap */
	}  v;
	unsigned char     depth;
};

#define  heap_leaf(h)    (!(h)->depth )
#define  heap_noinsert(h)    ((h)->depth)

static   heap  *the_heap=0;
static  int	stacksize=0; 
static  heap	**stack=0;

/*
 *	add item to heap,
 *	return 1 if error
 */
add_heap(val,valp)
float	val;
void	*valp;
{
	heap 	*hv = the_heap;
	int	stackptr=0;
	if ( !the_heap ) {
		/*
		 *        new heap
		 */
		the_heap = (heap *)salloc(sizeof(heap));
		the_heap->depth = 0;
		the_heap->v.valp = valp;
		the_heap->best_subval = val;
		stacksize=10;
		if ( !(stack = (heap** )salloc(sizeof(heap*)*stacksize) ) )
		  return 1;
		return 0;
	} 
	/*
	 *        ripple down tree to find new insertion point
	 */
	while ( heap_noinsert(hv) ) {
		/*
		 *        at node with 2 childs so insert in shallowest subtree
		 */
		stack[stackptr++] = hv;
		if ( hv->v.n.left->depth > hv->v.n.right->depth ) 
			hv = hv->v.n.right;
		else
			hv = hv->v.n.left;
	} 
	/*
	 *        at leaf so make new node here
	 */
	stack[stackptr] = hv;
        if ( !(hv = (heap*)salloc(sizeof(heap))) )
	  return 1;
        hv->depth = 1;
        hv->v.n.left = stack[stackptr];
	hv->best_subval = Max(val,stack[stackptr]->best_subval);
	if ( !(hv->v.n.right = (heap*)salloc(sizeof(heap))) )
	  return 1;
	/*
         *        adjust parent node to reflect node replacing leaf
         */
	if ( !stackptr ) {
		the_heap = hv;
	} else if ( stack[stackptr-1]->v.n.left == stack[stackptr] ) {
		stack[stackptr-1]->v.n.left = hv;
	} else {
		stack[stackptr-1]->v.n.right = hv;
	}
	/*
	 *        now add new leaf to the new node
	 */
	hv = hv->v.n.right;
        hv->v.valp = valp;
	hv->depth = 0;
	hv->best_subval = val;
	/*
         *        now ripple back up and update nodes
         */
	while ( stackptr ) {
		hv = stack[--stackptr];
		hv->depth = 1+Min( hv->v.n.left->depth, hv->v.n.right->depth );
		hv->best_subval = Max( hv->v.n.left->best_subval, 	
				       hv->v.n.right->best_subval );
	}
	/*
	 *        check stack is OK
	 */
	if ( the_heap->depth > stacksize-2 ) {
		stacksize+=10;
		sfree(stack);
		if ( !(stack = (heap**)salloc(sizeof(heap*)*stacksize)) )
		  return 1;
	}
	return 0;
}

/*
 *	take best item from heap,
 *	return 0 if empty,  
 *	stack will be OK size due to add_heap()
 */
void *
rem_heap()
{
	heap 	*hv = the_heap;
	int	stackptr=0;
	void 	*found ;
	if ( !the_heap ) {
		return (void *)0;
	} else if ( !the_heap->depth ) {
		found = the_heap->v.valp;
		sfree(the_heap);
		the_heap = 0;
		sfree(stack);
		stacksize = 0;
		return found;
	} 
	/*
	 *        ripple down tree to find best val to remove
	 */
	while ( heap_noinsert(hv) ) {
		/*
		 *        at node with 2 childs so go to best
		 */
		stack[stackptr++] = hv;
		if ( hv->v.n.left->best_subval > hv->v.n.right->best_subval ) 
			hv = hv->v.n.left;
		else
			hv = hv->v.n.right;
	} 
	/*
	 *        at leaf so remove this guy
	 */
	found = hv->v.valp;
	stack[stackptr] = hv;
	/*
         *        set  hv  =  sibling of to-be-removed-leaf,
	 *	  this will be promoted to its parents position
         */
	ASSERT( stackptr );
	if ( stack[stackptr-1]->v.n.left == stack[stackptr] ) {
		hv = stack[stackptr-1]->v.n.right;
	} else {
		hv = stack[stackptr-1]->v.n.left;
	}
	sfree(stack[stackptr--]);
	/*
         *        adjust parent's parent to reflect promotion
         */
	if ( !stackptr ) {
	  the_heap = hv;
	} else if ( stack[stackptr-1]->v.n.left == stack[stackptr] ) {
	  stack[stackptr-1]->v.n.left = hv;
	} else {
	  stack[stackptr-1]->v.n.right = hv;
	}
	sfree(stack[stackptr]);
	/*
         *        now ripple back up and update nodes
         */
	while ( stackptr ) {
		hv = stack[--stackptr];
		hv->depth = 1+Min( hv->v.n.left->depth, hv->v.n.right->depth );
		hv->best_subval = Max( hv->v.n.left->best_subval, 	
				       hv->v.n.right->best_subval );
	}
	return found;
}


#ifdef TEST_HEAP
char   *progname = "tests";

prt_heap()
{
        heap    *hv=the_heap;
	int	stackptr=0;

	for ( ;; ) {
		if ( heap_noinsert(hv) ) {
		    printf("node:  best=%g, depth=%d\n",
				hv->best_subval, hv->depth );
		    stack[stackptr++] = hv->v.n.right;
		    hv = hv->v.n.left;
		} else {
		    printf("leaf:  val=%d, depth=%d\n",
				*((int*)hv->v.valp), hv->depth );
		    if ( !stackptr )
		      break;
		    hv = stack[--stackptr];
		}
	}
	printf("\n");
}

main()
{
	int	i, *ip;
	char	type;

	for(;;) {
		printf("command: ");
		scanf(" %c",&type);
		switch ( type ) {
		case 'a' :
			printf("adding: ");
			scanf(" %d",&i);
			ip = (int*) salloc(sizeof(int));
			*ip = i;
			add_heap((float)i, ip);
			break;
		case 'r' :
		        ip = (int*)rem_heap();
			if ( ip ) {
			  printf("rem: %d\n",*ip);
			  sfree(ip);
			} else
			  printf("rem:  heap empty\n");
                        break;
		case 'p' :
                        prt_heap();
                        break;
		}
	}
}

#endif

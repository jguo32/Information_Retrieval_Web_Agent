/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "DEFN.h"

/*  #define DEBUG  */
/*   #define FORCE_WEIGHTS  */
#       ifdef FORCE_WEIGHTS
extern   int  GGGGGG;
#	endif

/*
 *	Routines for managing the example sets.
 */

/***************************************************************************/
/*
 *	make_set(dblock) -- construct a list from data block
 *
 */
egset *make_set(dblock)
egtype *dblock;
{
	egset	*list;	/* The list of examples */
	int	i,j;

	list = (egset *) salloc(sizeof(egset));
	list->size = negs;
	list->members = dblock;
	list->weights =  0;
	list->copied = FALSE;
	list->original = TRUE;

	/*	set "unkn"    */
	for (i = 0; i < list->size; i++) {
		forattrs(j)
		    if ( num_type(j) ) {
			if( ord_val(list->members[i],j) == FDKNOW )
				unkns(j) = TRUE;
		    } else if ( set_type(j) ) {
			if( set_eq(set_val(list->members[i],j), SDKNOW) )
				unkns(j) = TRUE;
                    } else if( unord_val(list->members[i],j) == DONTKNOW )
                                unkns(j) = TRUE; 
	}
#	ifdef DEBUG
		fprintf(stdrep, "Created list: size %d\n", list->size);
#	endif
#       ifdef FORCE_WEIGHTS
	if ( GGGGGG ) {
	  list->weights =  (float *) salloc(list->size * sizeof(float));
	  for (i = 0; i < list->size; i++) 
	    list->weights[i] = 1.0;
	}
#	endif
	return list;
}

/***************************************************************************/
/*
 *	new_set(maxsize) -- create a new list capable of holding maxsize
 *	examples, but do not add any examples to it.
 */
egset *new_set(maxsize)
int maxsize;
{
	egset *list;	/* The list being created */
	list = (egset *) salloc(sizeof(egset));
	if ( !list ) return (egset *)0;
	list->size = 0;
	list->weights = 0;
	list->copied = FALSE;
	list->original = FALSE;
	if (maxsize ) {
	  if ( ! (list->members = mems_alloc(egtype, maxsize) ))
		return (egset *)0;
	} else 
		list->members = 0;
#	ifdef DEBUG
		fprintf(stdrep, "Made a list for %d examples\n", maxsize);
#	endif
	return list;
}

/***************************************************************************/
/*
 *	sameclass_set(list) -- if all members of list have the same class, 
 *	return that class, else return -1
 */
sameclass_set(list)
egset	*list;
{
	int	class;		/* Class of the first example */
	int	i;

	if (list->size) {
		class = class_val(list->members[0]);
		for (i = 1; i < list->size; i++)
			if (class != class_val(list->members[i]))
				return -1;
		return class;
	} else /* There were no examples */
		return -1;

}


/***************************************************************************/
/*
 *	cal_d_vec(list) -- count the weighted sum of examples for each class
 */
float*
cal_d_vec(list)
egset	*list;
{
	float*	 tbl;		/* Holds the counts */
	egtype   eg;
	foregtype  egp;
	float     wt, *wtp;

	if ( !(tbl = make_fvec(ndecs) ))
		return (float*)0;
	if ( weighted(list) ) {
	  foreachegw(list,eg,egp,wt,wtp)  {
	    tbl[class_val(eg)] += wt;
	  }
	} else {
	  foreacheg(list,eg,egp)
	    tbl[class_val(eg)]++;
	}
	return tbl;
}


/***************************************************************************/
/*
 *	cal_d_sum(list) -- count the weighted sum of examples
 */
float
cal_d_sum(list)
egset	*list;
{
        float  wsum;
	float     wt, *wtp;

	if ( weighted(list) ) {
	  wsum = 0.0;
	  foreachegww(list,wt,wtp) 
	    wsum += wt;
	  return wsum;
	} else 
	  return list->size;
}

/***************************************************************************/
/*
 *	print_set(list) -- print the contents of list on standard error.
 */
print_set(list)
egset *list;
{
	int   i;

	for (i = 0; i < list->size; i++) {
		printeg(list->members[i]);
		if ( weighted(list) )
		  fprintf(stdrep," (%g)\n",list->weights[i]);
		else
		  fprintf(stdrep,"\n");
	}
	fprintf(stdrep, "------------------------------------\n");
}

/***************************************************************************/
/*
 *	free_allset(list)  --  frees all memory plus for egs !! (make_set)
 */
free_allset(list)
register egset	*list;
{
	register  int	i;
	egtype	lptr;
	/*
	 *	list may have been scrambled so find least pointer 
	 */
	lptr = list->members[0];
	for (i=list->size-1; i>=0; i--)
		if (lptr.unordp > list->members[i].unordp )
			lptr = list->members[i];
	sfree(lptr.unordp);
	sfree(list->members);
	if ( list->weights) sfree(list->weights);
	sfree(list);
} 

/***************************************************************************/
/*
 *	check_set(list)  --  frees all memory plus for egs !! (make_set)
 */
check_set(list)
register egset	*list;
{
	register  int	i;
	if ( !list )
	  return;
	/*
	 *	read each item
	 */
	for (i=list->size-1; i>=0; i--)
		  if ( ! list->members[i].unordp )
		    fprintf(stdrep,"Blank example in set");
} 

/***************************************************************************/
/*
 *	free_set(list)  --  frees all memory allocated to list, and its list.
 */
free_set(list)
egset	*list;
{
#	ifdef DEBUG
		fprintf(stdrep,"\tfreeing list\n");
#	endif
	if ( !list || list->original )
	  return;
	if ( !list->copied  && setsize(list) ) {
		if ( list->members ) sfree(list->members);
		if ( list->weights ) sfree(list->weights);
	}
	sfree(list);
} 

/***************************************************************************/
/*
 *	copy_set(set) -- copy a set into new location
 */
egset *
copy_set(old_set)
egset	*old_set;
{
	int	i;
	egset	*new_set;	/* The new set of examples */

	if ( ! ( new_set = (egset *) salloc(sizeof(egset)) ))
		return (egset*)0;
	new_set->size = setsize(old_set);
	new_set->copied = FALSE;
	new_set->original = FALSE;
	/*
	 *	Copy the examples.
	 */
	if ( !(new_set->members = 
		(egtype *) salloc(new_set->size * sizeof(egtype)) ))
		return (egset*)0;
	for (i = new_set->size-1; i>=0 ; i--)
		new_set->members[i] = old_set->members[i];
	if ( old_set->weights ) {
		if ( !( new_set->weights =
			(float *) salloc(new_set->size * sizeof(float)) ))
			return (egset*)0;
		for (i = new_set->size-1; i>=0 ; i--)
			new_set->weights[i] = old_set->weights[i];
	} else
		new_set->weights = 0;
	return new_set;
}

/***************************************************************************/
/*
 *	sub_set(set) -- put subset in temporary location and return
 */
egset *
sub_set(old_set,start_i,end_i)
egset	*old_set;
int     start_i,end_i;
{
	static	egset	temp;	/* The new set of examples */

	if ( end_i==start_i )
	  return (egset *)0;
	temp.members = old_set->members + start_i;
	if ( weighted(old_set) )
	  temp.weights = old_set->weights + start_i;
	else
	  temp.weights = 0;
	temp.size = end_i-start_i;
	temp.copied = TRUE;
	temp.original = FALSE;
	return &temp;
}

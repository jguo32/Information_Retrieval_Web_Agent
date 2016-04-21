/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"

/*
 *	Routines for partitioning the example lists.
 */

/***************************************************************************/
/*
 *	partition(list, prop, list1,  list2) - split list into 2 bits of
 *				proportion as given in prop.
 */
partition(list, prop, list1,  list2)
egset	*list, **list1, **list2;
float	prop;			/*  proport. of egs. to put in list1 (round off) */
{
	register	egset *l1, *l2;

	*list1 = l1 = (egset *) salloc(sizeof(egset));
	*list2 = l2 = (egset *) salloc(sizeof(egset));
	if ( !l1 || !l2 )
		error("insufficient memory to partition","");

	l1->members = list->members;
	l1->size = list->size*prop + 0.5;
	l1->copied = TRUE;
	l2->members = list->members + l1->size;
	l2->size = list->size - l1->size;
	l2->copied = TRUE;
	if ( list->weights ) {
		l1->weights = list->weights;
		l2->weights = list->weights + l1->size;
	} else {
		l1->weights = 0;
		l2->weights = 0;
	}
}        


/***************************************************************************/
/*
 *	ithpartition(list, ith, folds, list1,  list2) - split list into "folds"
 *			bits, return "ith" as separate.
 */
ithpartition(list, ith, folds, list1,  list2)
egset	*list, **list1, **list2;
int	folds;			/*  list split into this many pieces */
int	ith;			/*  specifies which piece is taken, 0,..,folds-1 */
{
	register int i, start2;
	register egset	*l1, *l2;

	*list1 = l1 = (egset *) salloc(sizeof(egset));
	*list2 = l2 = (egset *) salloc(sizeof(egset));

	l2->size = list->size/folds + ( ith < (list->size%folds) ? 1 : 0 );
	l1->size = list->size - l2->size;
	l1->copied = FALSE;
	l2->copied = FALSE;
	start2 = (list->size/folds)*ith + Min( ith, list->size%folds );
	l1->members = mems_alloc(egtype, l1->size);
	l2->members = mems_alloc(egtype, l2->size);
	for (i=0; i<start2; i++) 
		l1->members[i] = list->members[i];
	for (i=start2; i<l2->size+start2; i++) 
		l2->members[i-start2] = list->members[i];
	for (i=l2->size+start2; i<list->size; i++) 
		l1->members[i-l2->size] = list->members[i];
	if ( list->weights ) {
		l1->weights =  mems_alloc(float, l1->size);
		l2->weights = mems_alloc(float, l2->size);
		for (i=0; i<start2; i++) 
			l1->weights[i] = list->weights[i];
		for (i=start2; i<l2->size+start2; i++) 
			l2->weights[i-start2] = list->weights[i];
		for (i=l2->size+start2; i<list->size; i++) 
			l1->weights[i-l2->size] = list->weights[i];
	} else {
		l1->weights = 0;
		l2->weights = 0;
	}
}        


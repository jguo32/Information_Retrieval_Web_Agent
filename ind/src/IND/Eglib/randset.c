/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "DEFN.h"
#include "TREE.h"

/*  #define DEBUG  */
/*
 *           defines in terms with the prior added,
 *	     by what factor the set was changed
 *	     used in log_prob, etc.
 */

/***************************************************************************/
/*
 *	take random sample of old_set of size, making a copy 
 *        don't use "random" because only roughly random numbers
 *        will be OK for this purpose ...
 *        so just generate numbers by adding a large prime;
 *        keeps set storage around for later calls with less or equal size;
 *           NB.   if some jerk tries to free the set, then kaboom!!
 */
egset *
random_subset(old_set,size,old_weight)
egset	*old_set;
int     size;
float   old_weight;
{
	unsigned int	i,j;
	float	tot_weight, scale_up;
	static  egset	*nset=0;	        /* The new set of examples */
	static  int     nset_size=0, nset_wsize=0;
	static  float   *real_weights=0;
	int     nexti = time_random();
	static  unsigned int   this_prime;
	static  unsigned int primes[10] = {80147, 65587, 90917, 56003, 84401,
				     98519, 72431, 66851, 92173, 99709 };  

#ifdef DEBUG
	printf("entry random_subset old size = %d, size = %d, old_weight = %f\n",
	       setsize(old_set), size, old_weight);
#endif
	if ( !nset ) {
	  /*
	   *   create a new set the right size
	   */
	  if (  !( nset = mem_alloc(egset) ))
	    return (egset*)0;
	  if ( !(nset->members = mems_alloc(egtype,size) ))
	    return (egset*)0;
#ifdef DEBUG
	  printf(".... random_subset, new members size = %d\n", size );
#endif
	  nset_size = size;
	} else if ( nset_size < size ) {
	  /*
	   *   increase the size of members
	   */
	   sfree(nset->members);
#ifdef DEBUG
	   printf(".... random_subset, free old, new members size = %d\n", size );
#endif
	   if ( !(nset->members = mems_alloc(egtype,size) ))
            return (egset*)0;
	   nset_size = size;
	} else
	  /*
	   *   leave things alone
	   */
	  ; 
	nset->size = size;
	nset->copied = TRUE;   

	if ( old_set->weights )  {
	   if ( nset_wsize >= size ) {
		nset->weights = real_weights;
	   } else {
	        /*
	         *   allocate the right size weights array
	         */
	        if ( nset_wsize  )
		    sfree(real_weights);
	   	if ( !( nset->weights = real_weights =
		    mems_alloc(float,size) ))
	      		return (egset*)0;    
	        nset_wsize = size;
#ifdef DEBUG
	        printf(".... random_subset, free old, new weights size = %d\n",
		       size );
#endif
	   }
	   tot_weight = 0.0;
	} else {
	   nset->weights = 0;
	   tot_weight = size;
	}

	/*
	 *	Copy the examples roughly randomly
	 */
	this_prime = primes[nexti%10];
	i=(nexti+this_prime)%old_set->size;
	for (j=0; j<size ; j++, i=(i+this_prime)%old_set->size ) {
	  nset->members[j] = old_set->members[i];
	  if ( old_set->weights ) 
	    tot_weight += nset->weights[j] = old_set->weights[i];
	}
	nexti = i;
	scale_up = (old_weight + palphatot(old_weight)) / 
				( tot_weight + palphatot(tot_weight) ) ;
	set_scale_prob(scale_up);
#ifdef DEBUG
	printf("exit random_subset, old_weight = %f, tot_weight = %f, scale-up = %f\n",
	       old_weight, tot_weight, scale_up );
#endif
	return nset;
}

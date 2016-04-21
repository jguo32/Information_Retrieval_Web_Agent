/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  BITSET.h -- header file for all programs using bitsets
 *
 *	Author - Wray Buntine, 1990
 *
 */

/*      BitSets  - for handling bit sets 
 *		    a "set" is the bits in an "unsigned int", 
 *		    assumed BITSETMAX bits
 *		    which is hard coded into set_card()
 *	NB.   high part of set is rubbish, so must mask out during
 *	      operations like cardinality, equality or loops, or
 *	      clean up set after set complements, etc, see "set_clean()"
 *	  i.e.   high bits are maintained as 0 throughout
 *
 *	Author - Wray Buntine , 1989
 *
 */

/*
 *	datatype = list of bits
 */
typedef	unsigned int bitset;

/*
 *	constants
 */
#define BITSETMAX 32
#define EMPTY_SET 0
#define UNIV_SET(i)  	(bit_univ[i])
#define SINGLETON(i)	(1<<i)

#if BITSETMAX % 8
  ??   /*   make it divisible by 8 */
#endif
/*  
 * 	flags implemented as fields in union
 *      e.g.   see tree_flags, tprint_flags, etc., in TREE.h
 */
#define null_flags(set)        set.i = 0
#define set_flags(set,val)     set.i = val
#define int_flags(set)         ((unsigned int)set.i)
#define set_flag(set,flag)     set.b.flag = TRUE
#define unset_flag(set,flag)     set.b.flag = FALSE
#define flag_set(set,flag)     set.b.flag

/*
 *	operations
 */
#define clean_set(set,size) 	(set&bit_univ[size])
#define set_clean(set,size) 	set &= bit_univ[size]
#define set_bit(set,i)		set |= 1<<i
#define bit_set(set,i)		(set & (1<<i))
#define unset_bit(set,i)	set &= ~(1<<i)
#define set_union(set1,set2)	(set1 | set2)
#define set_inter(set1,set2)	(set1 & set2)
#define set_minus(set1,set2)	(set1 & ~set2)
#define set_sub(set1,set2,size)	(set1 & ~set2 & bit_univ[size] == 0)
#define set_empty(set1)		(!(set1))
#define set_notempty(set1)	(set1)
#define set_comp(set,size)	(~set & bit_univ[size])
#define set_eq(set1,set2)	((set1==set2))
#define forinset(i,set,temp)  for (i=0,temp=set; temp; i++,temp>>=1 ) \
                                        if ( !(1&temp) ) continue; else
#define foroutset(i,set,temp,size) for (i=0,temp=set_comp(set,size);  \
					temp; i++,temp>>=1 ) \
                                        if ( !(1&temp) ) continue; else
#define setto_union(newset,set)	newset |= set
#define setto_inter(newset,set)	newset &= set
#define setto_minus(newset,set)	newset &= ~set
#define set_copy(newset,set)	newset = set
#if BITSETMAX == 16
#define set_card(set)       (cxic.i=set, bit_cv[cxic.c[0]]+bit_cv[cxic.c[1]])
#else
#if BITSETMAX == 32
#define set_card(set)       (cxic.i=set, bit_cv[cxic.c[0]]+bit_cv[cxic.c[1]]\
					+bit_cv[cxic.c[2]]+bit_cv[cxic.c[3]])
#else
#define set_card(set)        ?????
#endif
#endif

#define set_cardless(set,i) 	(set_card(set&bit_univ[i]))

#define singleton(set)		(set_card(set)==1)

union cxitype {
	 unsigned char	c[sizeof(bitset)];
	 bitset    i;
};

#ifndef	 BITS
extern	bitset 	bit_univ[];
extern	union cxitype cxic;
extern  unsigned char  bit_cv[];
extern  unsigned char  bit_af[];
extern  unsigned char  bit_nx[];
#endif



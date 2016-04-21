/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  BITARRAY.h -- header file for all programs using bit arrays
 *
 *	Author - Wray Buntine, 1991
 *
 */

/*      BitArrays  - for handling arrays of bits;
 *		    a bitarray is hard-coded as an
 *                  array of (BITSETMAX)-bit words
 *		    for processing large sets
 *	 	    (see BIT*.h and bitset.c)
 */

/*
 *	datatype =  bitarray is array of bits accessed as
 *                  a block of bitsets, or a block of unsigned chars
 *        set[-1]   =  (the size words of the set) - 1
 *        set[-2]   =  mask for used bits in last word of array
 *        bits start in word set[0] and end in word set[set[-1]]
 *	  unused bits in last word are set to 0
 *        bits in a set are numbered 0,1,...,(size-1)
 *	  0 unused bits in last word by anding with set[-2]
 */

typedef bitset  *bitarray;

#ifndef  BITS
extern  bitset    bit_univ[];
extern  unsigned char  bit_cv[];
extern  unsigned char  bit_nx[];
extern  unsigned char  bit_af[];
extern  unsigned int    bs_flag;
extern  int    card_i;
extern  bitarray new_bitarray();
extern  bitarray copy_bitarray();
extern int write_bitarray();
extern void free_bitarray();
extern int  read_bitarray();
extern void random_bitarray();
extern  bitarray get_bitarray();
extern int  put_bitarray();
#endif

/*
 *	operations for bit in set
 */
/*    set the i-th bit in "set"   */
#define set_bitarray(set,i)	(set)[i/BITSETMAX] |= 1<<(i%BITSETMAX)

/*    unset the i-th bit in "set"   */
#define unset_bitarray(set,i)	(set)[i/BITSETMAX] &= ~(1<<(i%BITSETMAX))

/*    is the i-th bit in "set" set?   */
#define bitarray_set(set,i)	((set)[i/BITSETMAX] & (1<<(i%BITSETMAX)))

/*
 *	miscellaneous
 */
/*    zero unused bits at end of set  */
#define bitarray_clean(set)        set[set[-1]] &= set[-2]

/*    set "card" to cardinality of set "set"  */
#define bitarray_card(card,set)		card_i=set[-1]; \
					card = set_card(set[card_i]&set[-2]); \
					for (; --card_i>=0; \
					     card+=set_card(set[card_i]))
/*    set "card" to no. of elements in set "set" below element i */
#define bitarray_cardless(card,set,i)  card_i=((i-1)/BITSETMAX); \
			       card=set_cardless(set[card_i],(i-1)%BITSETMAX);\
					for (; --card_i>=0; \
					     card+=set_card(set[card_i]) )

/*
 *	boolean tests on set, answer returned in "bs_flag"
 *	always follow with semicolon
 */
/*    is the set empty?   */
#define bitarray_empty(set1)        for (card_i=set1[-1]; \
                                         card_i>=0 && !set1[card_i];\
                                         card_i--);  bs_flag = (card_i<0)

/*    are the sets equal?   */
#define bitarray_equal(set1,set2)   for (card_i=set1[-1]; \
					 card_i>=0 && \
                                           (set1[card_i]==set2[card_i]);\
					 card_i--);   bs_flag=(card_i<0)

/*    is set1 a subset of set2?   */
#define bitarray_subset(set1,set2)   for (card_i=set1[-1]; \
					 card_i>=0 && \
                                           ((set1[card_i]|set2[card_i]) \
                                             ==set2[card_i]);\
					 card_i--);   bs_flag=(card_i<0)


/*
 *    All set operations such as union, intersection, etc.
 *    modify the first argument.
 *    Also, sets are assumed to be the same size.
 *	always follow with semicolon
 */

/*    copy "set2" into "set1"   */
#define bitarray_copy(set1,set2)  for (card_i=set1[-1]; card_i>=0; card_i--) \
					 set1[card_i] = set2[card_i]

/*    union "set2" into "set1"   */
#define bitarray_union(set1,set2) for (card_i=set1[-1]; card_i>=0; card_i--)\
					 set1[card_i] |= set2[card_i]

/*     intersect set2 into set1  */
#define bitarray_inter(set1,set2)   for (card_i=set1[-1]; card_i>=0; card_i--)\
					 set1[card_i] &= set2[card_i]

/*      subtract set2 from set1  */
#define bitarray_minus(set1,set2)   card_i=set1[-1]; \
                                    set1[card_i] &= ~set2[card_i] & \
                                          set1[-2]; \
                                    for (; --card_i>=0; \
					 set1[card_i] &= ~set2[card_i])

/*      set set1 to set2 xor set1  */
#define bitarray_xor(set1,set2)   card_i=set1[-1]; \
                                  set1[card_i] ^= set2[card_i]; \
                                  set1[card_i] &= set1[-2]; \
                                  for ( ; --card_i>=0; \
					 set1[card_i] ^= set2[card_i])

/*      set "set1" to its complement  */
#define bitarray_comp(set1)    card_i=set1[-1]; \
                               set1[card_i] = ~set1[card_i] & \
                                          set1[-2]; \
                               for (; --card_i>=0; \
					 set1[card_i] = ~set1[card_i])

/*      set "set1" to be the empty set  */
#define empty_bitarray(set1)        for (card_i=set1[-1]; card_i>=0; \
					   set1[card_i--] = 0 )

/*      set "set1" to be the universal set of size "size"   */
#define univ_bitarray(set1,size)    set1[card_i=set1[-1]]=set1[-2]; \
                                    for( ;  --card_i>=0; \
					   set1[card_i] = ~0 )

/*      change size of bitarray, assuming memory is OK */
#define resize_bitarray(set,size)   set[-1] = (size/BITSETMAX); \
				    set[-2] = bit_univ[(size%BITSETMAX)+1];

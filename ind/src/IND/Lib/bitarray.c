/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *              Various bits and pieces used by BITARRAY.h
 */
#define BITS
#include <stdio.h>
#include "BITSET.h"
#include "BITARRAY.h"
#include "Lib.h"

extern  bitset    bit_univ[];
extern  unsigned char  bit_cv[];
extern  union cxitype cxic;

/* 
 *      return flag for boolean operations on bitarrays 
 */
unsigned int     bs_flag;
/* 
 *      counter for bitarray operations
 */
int     card_i;


/*
 *	create new bitarray
 */
bitarray
new_bitarray(size)
int   size;
{
        register  bitarray	ba;
	size--;
	ba = ( (bitset *)salloc(((size/BITSETMAX)+3)*sizeof(bitset)) ) + 2;
	ba[-1] = size/BITSETMAX;
	ba[-2] = bit_univ[(size%BITSETMAX)+1];
	for  (size=ba[-1]; size>=0; size--) ba[size]=0;
       	return ba;
}

/*
 *      copy bitarray
 */
bitarray
copy_bitarray(set)
bitarray   set;
{
        bitarray      ba;
	int	i;
	ba = ( (bitset *)salloc((set[-1]+3)*sizeof(bitset)) ) + 2;
        for  (i=set[-1]; i>= -2; i--) ba[i]=set[i];
        return ba;
}
	
/*
 *	free bitarray
 */
void free_bitarray(ba)
bitarray	ba;
{
	sfree(ba-2);
}


/*
 *	write bit array as sequence of 0s and 1s,
 *	with "," every char and ";" every word
 */
int
write_bitarray(set,fp)
bitarray    set;
register FILE    *fp;
{
	register  int  i;
	int	size;
	size = set_card(set[-2]) + set[-1]*BITSETMAX;
	for (i=0; i<size; i++) {
	    if ( bitarray_set(set,i) )
		putc('1',fp);
	    else 
		putc('0',fp);
	    if ( i%32 == 31 )
		putc(';',fp);
	    else if ( i%8 == 7 )
		putc(',',fp);
	    if ( ferror(fp) )
		return -1;
	}
	return size;
}

/*
 *   complements  write_bitarray()
 */
int read_bitarray(set,fp)
bitarray    set;
register FILE    *fp;
{
	register  int  i;
	int 	c;
	int	size;
	size = set_card(set[-2]) + set[-1]*BITSETMAX;
	empty_bitarray(set);
	for (i=0; i<size; i++) {
	    if ( (c=getc(fp))=='1' )
		set_bitarray(set,i) ;
	    else if ( c<0 )
		break;
	    else if ( c==',' || c==';' )
		i--;
	    else if ( c!='0' )
		return -1;
	}
	if ( i<size )
		return -1;
	return size;
}

/*
 *   binary file get and put,  return 0 on error
 */
int
put_bitarray(set, file)
bitarray   set;
FILE	  *file;
{
	return fwrite((char*)(set-2), sizeof(bitset), (int)(set[-1]+3), file);
}

bitarray
get_bitarray(size, file)
int   size;
FILE	  *file;
{
	bitarray	set;
	set = new_bitarray(size);
	if ( fread((char*)(set-2), sizeof(bitset), (int)(set[-1]+3), file) 
		== set[-1]+3 )
		return set;
	else
		return 0;
}

void
random_bitarray(set)
bitarray    set;
{
  card_i=set[-1]; 
  set[card_i] = random();
  set[card_i] &= set[-2]; 
  for ( ; --card_i>=0; set[card_i] = random() );
}

/*
 *      return index of nth element in set
 */
int
bitarray_nth(set,n)
int     n;
bitarray  set;
{
        int     index=0;
	int	temp;
        if ( n<=0 ) return 0;
        for ( card_i=0; card_i<=set[-1]; card_i++) {
            if ( n <= (temp=set_card(set[card_i])) ) {
		return index + getset_nth(set[card_i],n);
            } else {
                index += 32;
                n -= temp;
            }
        }
        return index;
}


#ifdef TESTING_BITS
/*
 *       testing routine for BITARRAY.h stuff
 */
char    *progname="try";

main()
{
  int   i,j,k,l;
  bitarray   s1,s2,s3,s4,s5;
  
  s1 = new_bitarray(200);
  s2 = new_bitarray(200);
  s3 = new_bitarray(200);
  s4 = new_bitarray(200);
  s5 = new_bitarray(200);
  write_bitarray(s4,stdout);  printf("\n");
  for (i=0; i<200; i++) {
    if ( i%2==0 )
      set_bitarray(s1,i);
    if ( i%3==0 )
      set_bitarray(s2,i);
    if ( i%5==0 )
      set_bitarray(s3,i);
  }
  /*   test 1 */
  bitarray_copy(s4,s2);
  write_bitarray(s4,stdout);  printf("\n");
  bitarray_minus(s4,s3);
  write_bitarray(s4,stdout);  printf("\n");
  for (i=0; i<200; i++) {
    if ( bitarray_set(s4,i) && ( (i%3) || (i%5==0) ) )
      printf("test 1 failed for i = %d\n",i);
    if ( ! bitarray_set(s4,i) && (i%3==0) && (i%5) )
      printf("test 1 failed for i = %d\n",i);
    if ( (i%3==0) && i ) {
      bitarray_cardless(j, s4, i);
      printf("test 2:  #s < %d that div. 3 but not 5 = %d\n",
	     i, j );
    }
  }
  bitarray_card(j, s4);
  printf("test 2:  #s < 200 that div. 3 but not 5 = %d\n", j );

  /*   test 3 */
  bitarray_copy(s5,s2);
  bitarray_minus(s5,s3);
  bitarray_equal(s4,s5);
  printf("test 3:  equal sets are equal = %u\n", bs_flag);
  set_bitarray(s5,20);
  bitarray_equal(s4,s5);
  printf("test 3:  unequal sets are equal = %u\n", bs_flag);

  /*   test 4 */
  empty_bitarray(s5);
  bitarray_empty(s5);
  printf("test 4:  empty set is empty = %u\n", bs_flag);
  bitarray_card(j, s5);
  printf("test 4:  cardinality of empty set = %d\n", j);

  /*   test 5 */
  bitarray_copy(s5,s2);
  bitarray_union(s5,s3);
  bitarray_inter(s5,s1);
  printf("test 5:  #s div. by 2 and (3 or 5) :");
  for (i=0; i<200; i++) {
     if ( bitarray_set(s5,i) )
       printf("%d,", i);
  }
  printf("\n");

  /*   test 6 */
  bitarray_copy(s5,s2);
  bitarray_xor(s5,s3);
  printf("test 6:  #s div. by 3 xor 5 :");
  for (i=0; i<200; i++) {
     if ( bitarray_set(s5,i) )
       printf("%d,", i);
  }
  printf("\n");

  /*   test 6 */
  bitarray_copy(s5,s2);
  bitarray_xor(s5,s3);
  printf("test 6:  #s div. by 3 xor 5 :");
  for (i=0; i<200; i++) {
     if ( bitarray_set(s5,i) )
       printf("%d,", i);
  }
  printf("\n");
  bitarray_comp(s5);
  for (i=0; i<200; i++) {
     if ( bitarray_set(s5,i) )
       printf("%d,", i);
  }
  printf("\n");
  
  }

#endif

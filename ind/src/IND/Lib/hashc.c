/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* 
 *   hash routines -- maintain a hash table indexed by strings
 */

#include <stdio.h>
#include "SYM.h"

/* #define TEST_HASH    switch on for self contained debugging */

/*
 *	hash table
 */
typedef  struct hasht hasht;
struct hasht {
	int	*store;
	char	***base;	/*  points to array of (char*) index by
				 *  entries in "store"   */
	int	size;		/*  size of tables above */
	int 	used;		/*  entries in use */
	int 	accs;		/*  no. of probes */
	float 	ave_probes;	/*  ave. probes */
};
/*
 *	these magic numbers determine
 *	when the hash table will be rebuilt from scratch, etc
 */
#define  INIT_USAGE	0.7	/* when creating a new table, use this much */
#define  MAX_USAGE      0.85    /* rebuild table if usage is over this */
#define  MAX_PROBES     1.5     /* rebuild table if average probes over this */

/*
 *	stores pointer to the storage spot where the
 *	"store" for the last failed search should be
 *	added;
 */
static  int   last_i = 0;
/*
 *	hash table sizes must be prime for double hashing
 */
static  hash_sizes[] = {5,7,11,13,23,37,59,89,113,131,157,
			181,211,233,263,293,389,1001};
#define  hashed_str(tbl,ind)  (*tbl->base)[tbl->store[ind]]

/*
 *	if size given, alloc. new hash table of this size
 *	else "base" is actually ptr. to hash table,
 *		rebuild the hash table the next size up
 */
char *
new_hash(size,base)
int	size;
char	***base;
{
	hasht  *newh;
	int  i;
	if ( size ) {
		if ( !(newh = mem_alloc(hasht)) )
			error("no space for hash table","");
		newh->base = base;
	} else {
		newh = (hasht *)base;
		size = newh->size+1;
	}
	for (i=0; hash_sizes[i]<size; i++) ;
	if (  hash_sizes[i] >= 1000 )
		error("hash table too large","");
	size = hash_sizes[i];
	if ( !(newh->store = mems_alloc(int,size)) )
		error("no space for hash table","");
	for (i=0; i<size; i++)
		newh->store[i] = -1;
	newh->size = size;
	newh->used = 0;
	newh->accs = 4;		/*  offset to discount initial estimate */
	newh->ave_probes = 1.0;
	return (char *)newh;
}
#define renew_hash(th)  new_hash(0,(char ***)th)

/*
 *	create new hash table for the list of strings pointed
 *	to by base
 */
char *
fill_hash(base,size)
int	size;
char	***base;
{
	hasht  *newh = (hasht *)new_hash((int)(size/INIT_USAGE),base);
	int  i, found;
	for (i=size-1; i>=0 ; i--) {
	    if ( (*base)[i] ) {
		found=find_hash((char *)newh, (*base)[i] );
		if ( found == DONTKNOW )
			add_hash((char *)newh, i);
	    }
	}
	return (char *)newh;
}

int
used_hash(hash)
char   *hash;
{
	return ((hasht *)hash)->used;
}

/*
 *	find store associated with hash table "the_hasht"
 *	for string index "str"
 *	if found return store, >= 0
 *	else return DONTKNOW ;
 *	uses double hashing for open addressing
 */
int
find_hash(the_hasht, str)
char    *the_hasht;
char	*str;
{
	hasht   *thet = (hasht *)the_hasht;
	char	*cmp;
	int	hashv=0;
	int	snd_hashv=0;
	int	probes = 1;
	int	i;
	static  bool do_check=1;
	int	found;
	char	**base;
	int	*store;

	if ( !the_hasht ) {
		return DONTKNOW;
	}
#ifdef TEST_HASH
	printf("ave_probes %f, used %d\n",
		thet->ave_probes, thet->used);
#endif
	if ( do_check && ( (thet->ave_probes > MAX_PROBES) ||
			      (thet->used > thet->size * MAX_USAGE) )  ) {
		/*
		 *	if the table is full up,
		 *	then create a whole new one
		 */
		do_check = 0;
		base = *thet->base;
		store = thet->store;
		i = thet->size-1;
		renew_hash(thet);
		for (; i>=0 ; i--) {
		    if ( store[i]>=0 ) {
			found=find_hash((char *)thet, base[store[i]] );
			ASSERT( found == DONTKNOW );
			add_hash((char *)thet, store[i]);
		    }
		}
		do_check = 1;
		sfree(store);
		thet->accs = 4;
		thet->ave_probes = 1.0;
	}
	/*
	 *	compute hash values
	 */
	for (cmp=str; *cmp; cmp++)  hashv += *cmp;
	snd_hashv = 1 + (hashv%(thet->size-1));
#ifdef TEST_HASH
	printf("key %d, hash %d, 2nd %d\n",
		hashv, hashv % thet->size, snd_hashv);
#endif
	hashv %=  thet->size;
	/*
	 *	cycle through table using double hashing
	 */
	found = 0;
	while ( thet->store[hashv] >= 0 ) {
		if ( strcmp(str,hashed_str(thet, hashv)) ) {
			hashv = (hashv+snd_hashv)%thet->size;
			probes++;
		} else {
			/*    found the string  */
			found = 1;
			break;
		}
	}
	/*
	 *	update stats
	 */
	thet->ave_probes = (thet->ave_probes*thet->accs + probes)/
				(thet->accs+1.0);
	thet->accs++; 
	last_i = hashv;	
	if  ( found )
		return  thet->store[hashv];
	else
		return DONTKNOW;
}


/*
 *	previous search failed, so add new entry at "last_i"
 */
add_hash(the_hasht, store)
char *  the_hasht;
int	store;
{
        hasht   *thet = (hasht *)the_hasht;
	if ( thet->store[last_i] < 0 ) {
		thet->store[last_i] = store;
		thet->used++;
		ASSERT ( thet->used <=  thet->size );
	} else
		thet->store[last_i] = store;
}


#ifdef TEST_HASH
char   *progname = "tests";

prt_hash(the_hasht)
char *  the_hasht;
{
        hasht   *thet = (hasht *)the_hasht;
	int     i;
	printf("size: %d\n", thet->size );
	printf("used: %d\n", thet->used );
	printf("accs: %d\n", thet->accs );
	printf("ave.probes: %f\n", thet->ave_probes );
	for (i=0; i< thet->size; i++ ) {
		if ( thet->store[i]>=0 ) 
		    printf("%d: %s (%d) ",
			i, (*thet->base)[thet->store[i]],  thet->store[i] );
	}
	printf("\n");
}

main()
{
	int	i;
	char	type;
	int	store=0;
	char	index[100];
	char	**saved;		/*  place to keep strings for testing */
	char	*news;
	char *	the_hasht;
	saved = mems_alloc(char *,100);
	the_hasht=new_hash(4,&saved);
	for(;;) {
		printf("command: ");
		scanf(" %c",&type);
		switch ( type ) {
		case 'f' :
			printf("arg: ");
			scanf(" %s",&index[0]);
			news = salloc(strlen(&index[0])+1);
			strcpy(news,&index[0]);
			printf("found: %d\n",
				find_hash(the_hasht, news ) );
			break;
		case 's' :
                        for ( i=0; i<store; i++ )
				printf("%s ",saved[i]);
			printf("\n");
                        break;
		case 'p' :
                        prt_hash(the_hasht);
                        break;
		case 'a' :
			saved[store] = news;
                        add_hash(the_hasht, store);
			store++;
                        break;
		}
	}
}

#endif

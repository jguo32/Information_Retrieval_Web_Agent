/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  SYM.h -- header file for all programs using the Lib/symbol stuff
 *
 *	Author - Wray Buntine, 1989
 *	scrounged from  the old ID3.h with new "context" type added,
 *	which was written by David Harper and modified by Murray Dean
 *
 */
#include "Lib.h"
#include "BITSET.h"
#include "BITARRAY.h"

/* Special characters */
#define MAGICNO  133
#define	DONTKNOW 254		/* unknown for unordtype */
#define FDKNOW	 999999.0	/* unknown for float */
			/* unknown for bitset, do "grep SDKNOW sym.y" */
#define SDKNOW	 SINGLETON(SYMSETMAX)	
#define DKNsymb	 '?'

/* Attribute descriptors */
#define CTS	'C'
#define NORM	'N'
#define EXP	'E'
#define STP	'J'
#define POISS	'P'
#define DISCRETE 'U'
#define DECISION 'D'
#define SETTYPE  'S'

/* Limits */
#define MAXATTRS  254	    /* sets size of symbol table, change if
				   you want more attributes  */
#define MAXCLSS	  128	    /* array sizes throughout non-recursive calls */
#define SYMSETMAX  BITSETMAX-1       /*  max. set size, must be < BITSETMAX  */

/* Special type for value of discrete attributes */
typedef	unsigned char unordtype;

/* header block for file of encoded examples */
typedef struct  Header  Header; 
struct Header
{
	int	h_magicno;	/*  used in encoded_file(), read_enc_eg(),... */
	int	h_nsattrs;	/*  no. of set attrs  */
	int	h_ncattrs;	/*  no. of continuous attrs  */
	int	h_ndattrs;	/*  no. of discrete attrs  */
	int     h_decattr;      /*  index for decision attribute  */
	int	h_negs;		/*  no. of examples in file  */
	int	h_egsize;	/*  size in chars of eg stuff */
				/*	set to 0 if variable  */
	Sampler h_sampler;      /*  details of sampling information */
};

/*
 * defined functions - #defined for efficiency or portability reasons
 */
#define nattrs          (ncattrs + ndattrs + nsattrs)
#define ncattrs         ld.h_ncattrs
#define nsattrs         ld.h_nsattrs
#define ndattrs         ld.h_ndattrs
#define negs            ld.h_negs
#define decattr         ld.h_decattr
#define egsize		ld.h_egsize
#define	ndecs		st_ndecs             /* = num values of dec. attr*/
#define type(A)		st[(int)A].sym_type	/* = type of attr A*/
#define set_type(A)	(type(A) == SETTYPE)
#define num_type(A)	(type(A) == CTS ||\
			type(A) == NORM ||\
			type(A) == EXP ||\
			type(A) == STP ||\
			type(A) == POISS) /* = true if attr A is numeric */
#define	name(A)		st[(int)A]._name     /* = name of attr A (as a string)*/
#define	unkns(A)	flag_set(st[(int)A].sflags,unkn)
					    /* = true if attr A has unkns  */
#define	sym_fill(A)	flag_set(st[(int)A].sflags,fill)
					    /* = true if attr A  incomplete  */
#define	getmin(A)	st[(int)A].u.ord->min  /* = minimum value for CTS attr*/
#define	getmax(A)	st[(int)A].u.ord->max  /* = maximum value for CTS attr*/
#define	unordvals(A)	st[(int)A].u.unord->vals /* = num values of unord attr*/
#define	unordstrs(A)	st[(int)A].u.unord->ptr /* = attr. vals as strings */
#define	unordhash(A)	st[(int)A].u.unord->hasht /* = hash table */
#define	unordsubsets(A)	st[(int)A].u.unord->subsetting /* =   SB_* flags */
#define	unordvalname(A,v)  st[(int)A].u.unord->ptr[v] /* attr. val. name */
#define onlyif(A)       st[(int)A]._onlyif   /* = cont list for atr */
#define sindex(A)       st[(int)A].sym_index  /* index to typed data array */
					/* index for attr. val. */
#define lookupval(str,atr) find_hash(unordhash(atr),str) 

/*   loop over values of decision attribute  */
#define fordecs(i)	for (i=0 ; i< ndecs; i++)
/*   loop over values of unordered attribute  */
#define forunordvals(i,a)	for (i=0;  i<unordvals(a); i++)
/*   loop over attributes including decision attribute  */
#define forattrs(i)	for (i=0; i<=nattrs; i++)
/*   loop over attributes excluding decision attribute  */
#define foraattrs(i)	for (decattr?(i=0):(i=1); i<=nattrs; ((++i)==decattr)?(++i):i )
/*   loop over discrete attributes */
#define fordattrs(i)	forattrs(i) if (type(i)!=DISCRETE) continue; else

char	*find();

/************************************************************************
 *
 *      Tests  - for handling attribute tests as found at tree nodes
 *
 *	Author - Wray Buntine , 1989
 *
 */

union test_flags 
{
	struct {
		unsigned  subset : 1;
		unsigned  cut : 1;
		unsigned  attr : 1;
		unsigned  remdr : 1;
		unsigned  bigsubset : 1;
	} b;
	unsigned char i;
};

typedef union test_flags test_flags;
typedef	struct test_rec	test_rec;
struct test_rec	/* type for test details */
{
	unsigned int	attr;	/* attribute tested */
	unsigned char	no;	/* no. of test values  */
	test_flags	tsflags; /* indicates type of test */
	union	{
		bitset		valset;
		bitarray	valbigset;
		float		cut;
		int		oval;
	}	tval;
	union  	{
		float		prop_true;   /* prop. of successes
					     * for this test, if binary */
		float		*prop_vec;   /* for non-binary tests */
	}	tprop;
};

/* subsetting types, subsetting implemented using bitsets or bitarrays */
/* values placed in "unord_sym.subsetting"  */
#define SB_OFF    00		/* no subsetting */
#define SB_FULL   01		/* allow binary split with equal probability */
#define SB_ONE    02		/* test for one value, i.e. binary encoding */
#define SB_REST   03		/* only one "remainder" group plus full
				   splitting on other attributes */
#define SB_FULL_STR  "full"
#define SB_ONE_STR   "one"
#define SB_REST_STR  "rest"

/* cut-test indices  */
#define LESSTHAN          0
#define GREATERTHAN       1

#define set_test(tester)   (type(tester->attr) == SETTYPE)
#define discrete_test(tester)   (type(tester->attr) == DISCRETE)
#define attr_test(tester)   flag_set(tester->tsflags,attr)
#define subset_test(tester)   flag_set(tester->tsflags,subset)
#define subsets_test(tester)   (flag_set(tester->tsflags,subset) || \
					flag_set(tester->tsflags,bigsubset) )
#define bigsubset_test(tester)  flag_set(tester->tsflags,bigsubset)
#define remdr_test(tester)   flag_set(tester->tsflags,remdr)
#define cut_test(tester)      flag_set(tester->tsflags,cut)

#define	outcomes(tester)	( tester->no )

#define foroutcomes(i,tester)	for (i=outcomes(tester)-1; i>=0; i--)

#define ord_outcome(eg,tester)	(ord_val(eg,tester->attr)<tester->tval.cut) ? \
					LESSTHAN  :  GREATERTHAN

#define set_outcome(eg,tester)  bit_set(tester->tval.valset,unord_val(eg,tester->attr)?1:0
#define attr_outcome(eg,tester)  unord_val(eg,tester->attr)
#define bigset_outcome(eg,tester)  bitarray_set(tester->tval.valbigset, \
					(unord_val(eg,tester->attr))?1:0

#define ord_unoutcome(eg,tester) (ord_val(eg,tester->attr)==FDKNOW)
#define unord_unoutcome(eg,tester) (unord_val(eg,tester->attr)==DONTKNOW)

/***********************************************************************
 *
 *      Context  - for handling contexts
 *	 	 - a context is an array of tests, null terminated,
 *		 - the context is on if all tests return 0;
 *
 *	Author - Wray Buntine , 1992
 *
 */

typedef	struct contexts Context;
struct contexts	/* type for frequency table entries */
{
	test_rec	ctest;
	unsigned char	coutcome;
	struct  contexts *cnext;
};

#define Cnever(cont)  (cont->ctest.attr==decattr)    /* always out of context */
#define Cnull(cont)   (!cont)                     /* always in context */
#define	Cnil	(Context *) 0	/* null pointer for null context */


/*********************************************************************
 * 
 *      Symbol Table.
 *	used to store all info on attributes, values and classes
 */
union sym_flags 
{
	struct {
		unsigned  unkn : 1;	/*  true if attr. has missing vals */
		unsigned  fill : 1;	/*  true if attr. details incomplete */
	} b;
	unsigned char i;
};
struct unord_sym {
	int vals;	/* Number of possible values */
	char **ptr;	/* Pointer to the value strings */
	char *hasht;    /* hash table for looking up vals */
	unsigned subsetting : 2; /* filled with SB_* values */
};
struct ord_sym {
	float min;	/* Min possible value for cts attr */
	float max;	/* Max possible value ... */
};

typedef	union	sym_flags   sym_flags;
typedef	struct	unord_sym	unord_sym;
typedef	struct	ord_sym		ord_sym;
typedef	struct	Symbol		Symbol;
struct Symbol
{
	char sym_type;	/* DISCRETE, CTS, DECISION, SETTYPE */
	sym_flags sflags;	
	int  sym_index;
	union {	/*	by storing these elsewhere, attributes
		 *	can share details, e.g. with "asfor", etc.
		 *	such as attribute values, etc.   */
		unord_sym  *unord;
		ord_sym  *ord;
	} u;
	char	*_name;			/*  name of attribute  */
	Context *_onlyif;		/*  context  */
};


#ifndef SYMBOL
/*********************************************************************
 *
 *	external variables defined in "symbol.c"
 */
extern  Symbol  st[];
extern  Header  ld;
extern  int     nvalsattr; 	/* max. number of values for attribute */
extern  int     nvalstest; 	/* max. number of outcomes for test */
extern  int     st_ndecs;
extern  float   cut_prob;
extern  double  *strat_prob;
extern 	double	*true_prob;
extern  float   **utilities;
extern  float   util_max, util_min;
extern  double  *strat_true;
extern	bool	rev_strat_flg;
#endif

#ifndef TESTED
/**********************************************************************
 *
 *	external variables defined in "tested.c"
 */
extern  Context  *context[];    /* see tested.c */
#endif

	void	create();
	char	*find();
	bitset  strchkset();
	bitset  lookupset();
	int	getattribute();
	bool	tested(), is_tested(), cantesteg();
     /*  egtesttype   init_test();  , in sets.h  */
	int	testchcs();
	bool	same_class();
	bitset	getvals();
	void    print_sym();

	test_rec	*copy_test();
	bool		cannot_test(), in_context();
	test_flags	set_test_flags();

#ifndef TREE_c
	extern	char	Uflg;	/*  method of handling unknowns  */
#endif

#define forless(i,val)	for (i= (val)-1; i>=0; i--)

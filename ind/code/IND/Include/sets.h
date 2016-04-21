/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*
 *	Author  Wray Buntine 1989
 *	some bits scrounged from old Id3.h which was written
 *	by David Harper and modified by Murray Dean and Chris Carter
 */

typedef struct egset egset;
typedef union egtype egtype;
typedef union egtype egtesttype;
typedef egtype  *foregtype;

/*
 *	sets of examples are stored in the structure below,
 *	a set is a list of pointers, plus an optional list
 *	of example weights,
 */
struct egset
{
	egtype	*members;
	float	*weights;
	short	size;
	unsigned  copied  : 1;		/*  members & weights are copies */
	unsigned  original  : 1;	/*  this set is THE full data set */
};
/*
 *	each example is stored as an
 *      array of unsigned chars followed by array of floats followed
 *	by array of bitsets;
 *	for implementation dependent stuff see:   
 *		set_val(), ord_val(), unord_val(), eg_storage, eg_?offset
 *		Trees/tgendta.c: fill_eg(), Eglib:tested.c: init_test()
 *		class_val is stored at 0 for efficiency reasons
 */
union  egtype
{  
        float   *ordp;  
        unordtype *unordp; 
        bitset   *setp;  
};

/*
 *	utilities to process the examples in a set
 */
#define Snil		        (egset *) 0
        /*   attr. vals. given cont. or discrete index for attr.  */
#define	ord_val_s(E, A)	        ((E).ordp[(int)A])
#define	set_val_s(E, A)	        ((E).setp[(int)A])
#define	unord_val_s(E, A)       ((E).unordp[(int)A])
        /*   attr. vals. given raw attr. index */
#define	ord_val(E, A)	        ord_val_s(E, sindex(A))
#define	set_val(E, A)	        set_val_s(E, sindex(A))
#define	unord_val(E, A)         unord_val_s(E, sindex(A))
#define	class_val(E)	        (*(E).unordp)
#define empty(List) 		(!List || !List->size)
#define setsize(List)  		List->size
#define example(List, I) 	(List->members[I])
#define examples(List) 		(List->members)
#define value(List, I, attr) 	(List->members[I]+attr)
#define weight(List, I) 	(List->weights[I])
#define weighted(List)   	(List->weights)
#define eg_coffset              ( (ndattrs*sizeof(unordtype))/sizeof(float)+1 ) 
#define eg_soffset		( ((ncattrs+eg_coffset) * sizeof(float)) \
					/ sizeof(bitset)  )
#define eg_storage		( (nsattrs+eg_soffset) * sizeof(bitset) ) 


/*
 *      Types for variables in for loops:
  	egset	*set;		 set to loop on
	egtype  eg;		 next eg
	foregtype  egp;		 temporary storage for loop
	float   wt, *wtp;	 likewise for weights
 */
	 
#define foreacheg(set,eg,egp)  if ( set && set->size ) \
			       for ( egp = ((set)->members+set->size-1); \
			        (egp>=set->members)?(eg=*egp,1):0 ; --egp)
#define foreachegw(set,eg,egp,wt,wtp)	if ( set ) \
			for (egp = ((set)->members+set->size-1), \
			     wtp = ((set)->weights+set->size-1); \
			     (egp>=set->members)?(eg=*egp,wt=*wtp,1):0 ; \
			     --egp, --wtp ) 
#define foreachegww(set,wt,wtp)	if ( set ) \
			for (wtp = ((set)->weights+set->size-1); \
				     (wtp>=set->weights)?(wt=*wtp,1):0 ; \
				     --wtp ) 

/*
 *	functions defined in Eglib
 */
#define next_cutn(s,a)	        next_cut(s,a,(double *)0)
egset  *make_set();
egset  *sub_set();
egset  *new_set();
egset	*get_comp();
float *cal_d_vec();
float  cal_d_sum();
egset	*copy_set();
float	start_cut();
float	next_cut();
float	rand_cut();
float	cut_diff();
egset	*splits();
egset	*random_subset();
egtype  read_eg();
egtype  *read_eg_file();
egtype  read_enc_eg();
void   init_read_enc_eg();
egtype  *read_enc_egs();
extern	int	cattr_offset;
egtesttype   init_test(), null_test(), merge_test();
void   add_test(), rem_test();

/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/*  TABLE.h -- header file for tabular counting
 *
 *	Author - Wray Buntine, 1992
 *	scrounged from  the old ID3.h with new "context" type added,
 *	which was written by David Harper and modified by Murray Dean
 *
 */

/**********************************************************************
 *     
 *      Frequency Table.
 *	following structures used to store outcome by class counting table
 */

typedef	union	tbl_flags tbl_flags;	/* type for frequency tables */
typedef	struct	fent	fent;	/* type for frequency tables */
typedef	struct	ftbl	ftbl;	/* type for frequency tables */

union tbl_flags 
{
	struct {
		unsigned  unknowns_assigned : 1;
		unsigned  masked : 1;
		unsigned  nullsplit : 1;
	} b;
	unsigned char i;
};

struct fent	/* type for frequency table entries */
{
	float	*d;
	float	vals;
};

struct ftbl	/* type for frequency tables */
{
	unsigned char	nv;
	tbl_flags  flags;
	bitarray  maskarray;	/*  these entries to be ignored in calcs. */
	bitset    maskset;	/*  these entries to be ignored in calcs. */
	test_rec *tester;
	float	tot_known;
	fent	*known;
	fent	unknown;
	float   *d;
};

/* = total number of egs */
#define total()         (tbl.tot_known + tbl.unknown.vals)
#define ntot()          ( tunk_ass() ?  tbl.tot_known :  \
                                    tbl.tot_known + tbl.unknown.vals )
#define ntot_kn()       (tbl.tot_known )
#define ntot_unkn()       (tbl.unknown.vals )
#define tmasked()	(flag_set(tbl.flags,masked))
#define tnullsplit()	(flag_set(tbl.flags,nullsplit))
#define tunk_ass()	(flag_set(tbl.flags,unknowns_assigned))
#define tunknowns_exist()	(flag_set(tbl.flags,unknowns_assigned) && tbl.unknown.vals )
#define nval_dec(val, dec)  ( tunk_ass() ? tbl.known[val].d[dec] :  \
				 tbl.known[val].d[dec] +  \
			( tbl.unknown.d[dec] * tbl.known[val].vals / tbl.tot_known ) )
#define nval_dec_kn(val, dec)  ( tbl.known[val].d[dec] )
#define nvalue(val)      ( tunk_ass() ? tbl.known[val].vals :  \
				 tbl.known[val].vals *  \
			( 1.0 + tbl.unknown.vals / tbl.tot_known ) )
#define nvalue_kn(val)      ( tbl.known[val].vals )
#define ndec_kn(dec)  ( tbl.d[dec] - tbl.unknown.d[dec] )

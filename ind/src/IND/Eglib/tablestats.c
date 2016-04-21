/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <math.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"
#include "TABLE.h"

/*******************************************************************
 *
 *	Routines for  calculating statistics on table "tbl"
 *
 */

extern	ftbl	tbl;		/* counting table */
extern	int	depth;
extern  bt_ptr  *treeoptions;
extern  float   *gainoptions;
float   errstds;

/****************************************************************************/
/*
 *	calc_gain() -- calculate the expected purity for the set after
 *	the current split is made
 */
calc_gain(ot,itry,flg)
ot_ptr	ot;
int	itry;
char	flg;	/*	type of measure   */
{
	register int	v,d;		/* Loop counter. */
	double	info = 0;	        /* information */
	double	splitinfo;	       	/* for gain ratio */
	double	nv,nv1;		        /* No. of egs that have val v.*/
	double	tot;	                /* Total no. of egs.*/
	register double	t;		/* tempory storage. */
	register double	x;		/* tempory storage. */
	
	switch ( flg ) {
	case 't':		/*  twoing  */
	    if ( nvalstest > 2 )
	      uerror("Twoing splitting only available with 2-way splits","");
	    tot = ntot();
	    nv = nvalue(0);   nv1 = nvalue(1);
	    if ( !nv || !nv1 ) {
	      treeoptions[itry]->gain = 0.0;
	      return;
	    }
	    fordecs(d) {
	      t = nval_dec(0,d)/nv - nval_dec(1,d)/nv1;
	      info += Abs(t);
	    }
	    treeoptions[itry]->gain = info * info * nv * nv1 / tot / tot / 4.0;
	    return;
	case 'g':		/*  gini index of diversity  */
	    tot = ntot();
	    ASSERT ( tot )
	    forless(v,tbl.nv) {
			if ( nv = nvalue(v) ) {
				t = 0;
				fordecs(d) {
					x = nval_dec(v,d) / nv;
					if (x > EPSILON)
						t += x * (1-x);
				}
				info += t * nv / tot;
			}
	    }
	    treeoptions[itry]->gain = ot->xtra.gr.gain - info;
	    return;
	case 'r':		/*  information gain ratio */
	    x = tbl.unknown.vals/total();
	    if (x > EPSILON)
	      splitinfo = -x * log(x)/LOG2;
	    else
	      splitinfo = 0;
	case 'i':		/*  information gain  */
	    tot = ntot_kn();
	    ASSERT ( tot )
	    forless(v,tbl.nv) {
			if ( nv = nvalue_kn(v) ) {
				t = 0;
				fordecs(d) {
					x = nval_dec_kn(v,d) / nv;
					if (x > EPSILON)
						t -= x * log(x) / LOG2;
				}
				info += t * nv / tot;
				if ( flg == 'r' ) {
					x = nv/total();
					if (x > EPSILON)
                                                splitinfo -= x * log(x)/LOG2;
				}
			}
	    }
	    t = 0;
	    fordecs(d) {
		x = ndec_kn(d) / tot;
		if (x > EPSILON)
			t -= x * log(x) / LOG2;
	    }
	    treeoptions[itry]->gain = (t - info)*tot/total();
	    if ( flg=='r' ) {
		  gainoptions[itry] = treeoptions[itry]->gain;
		  if ( splitinfo > EPSILON )
			treeoptions[itry]->gain /= splitinfo;
		  else 
			treeoptions[itry]->gain = -FLOATMAX;
	    }
	    return;
	}
}

/****************************************************************************/
/*
 *	Calculate the log probability of split in tbl and store in .gain.
 */
void
calc_log_prob(itry)
int	itry;
{

	register  int	d,v;		/* Loop counter.		*/
	bt_ptr  bt = treeoptions[itry];
	double	lp;	    /* Log of probability of classification rule*/
	extern  ot_rec  dummyt;

	depth++;
	lp = bt->np.nodew;
	dummyt.parents[0] = bt;
	forless(v,tbl.nv)  {
	        add_test(bt->test, v, dummyt.testing);
		fordecs(d) {
		    dummyt.eg_count[d] = nval_dec(v,d);
		}
		lp += leaf_prob(&dummyt,dummyt.testing);
	        rem_test(bt->test, v, dummyt.testing);
	}
	if ( ntot() ) 
		bt->gain = lp;
	else
		bt->gain = -FLOATMAX;
	depth--;
}


/****************************************************************************/
/*
 *	calc_leaf_gain() -- calculate the expected purity of leaf
 */
void
calc_leaf_gain(ot,flg)
ot_ptr	ot;
char	flg;	/*	type of gain   */
{
	register int	v;		/* Loop counter. */
	register double	t = 0.0;		/* tempory storage. */
	register double	x;		/* tempory storage. */

        switch ( flg ) {
        case 't':
	    ot->xtra.gr.gain = 0.0;
	    return;
        case 'g':
	    fordecs(v) {
		x = ot->eg_count[v] / ot->tot_count;
	        t += x * (1.0-x);
	    }
	    ot->xtra.gr.gain = t;
	    return;
	case 'r':		/*  information gain ratio */
        case 'm':
        case 'i':
	    fordecs(v) {
		x = ot->eg_count[v] / ot->tot_count;
		if (x > EPSILON)
			t -= x * log(x)/LOG2;
	    }
	    ot->xtra.gr.gain = t;
	    return;
	}
}

/****************************************************************************/
/*
 *	Calculate the estimated error & variance for split in tbl.
 */
void
calc_error(aerr, avar)
float	*aerr, *avar;
{
	register  int	d,v;		/* Loop counter.		*/
	float	eerr = 0.0;		/*  expect. err. of split 	*/
	float	evar = 0.0;		/*  variance of err. of split 	*/
	static float	*count;		/* temp. store of counts at leaf  */
	static float	*prop;		/* expect. prop. of egs. in leaf  */
	static float	*propstd;	/* variance of above  		*/
	static float	*err;		/* expect. error at leaf	*/
	static float	*errstd;	/* variance of above		*/
	float	tot;

	if ( !count ) {
		count = mems_alloc(float,ndecs);
		prop = mems_alloc(float,ndecs);
		propstd = mems_alloc(float,ndecs);
		err = mems_alloc(float,ndecs);
		errstd = mems_alloc(float,ndecs);
	}
	depth++;
	forless(v,tbl.nv)  {
		fordecs(d) {
		    count[d] = nval_dec(v,d);
		}
		leaf_errstd(count,err+v,errstd+v);
	    	prop[v] = nvalue(v);
	}
	vec_propstd(prop,prop,propstd,tbl.nv);
	tot = ntot()+1.0;
	forless(v,tbl.nv)  {
		eerr += prop[v]*err[v];
		evar += (propstd[v]+prop[v]*prop[v])*errstd[v]
			 + prop[v]*(errstd[v]+err[v]*err[v])/tot;
	}
	evar -= eerr*eerr / tot;
	*aerr = eerr;
	*avar = evar;
	depth--;
}

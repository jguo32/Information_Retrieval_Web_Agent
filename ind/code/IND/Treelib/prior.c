/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   Altered to include decision graph priors Jon Oliver September 1992
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "DEFN.h"
#include "TREE.h"

/*
 *              Module for handling tree priors
 *		For a good explanation, see display_prior();
 */

#define PRIOR_NML

#define MAX_TREE_DEPTH 200

float	*palphadec=0;	       /*   array of alphas */
float	palphaval = 1.0;	/*   1st parameter for all beta priors etc. */
int	depth_bound = MAX_TREE_DEPTH;  /*   max. depth of trees */
int     subflg = 0;    /*  allow subsetting, used in prior_nmlzr() 
			   and flags are SB_* in SYM.h  */
#ifdef GRAPH
bool    decision_graph_flag=0;
#endif

extern	int	depth;
extern	double	pow();
ot_ptr  tree = NULL;

/*   prior weight for a node  */
static	float	n_weight = 0.0;		
/*   prior weight for a leaf  */
static	float	l_weight = 0.0;
/*   normalization for Dirichlet prior, is added to leaf weight  */
static  float    b_weight, Db_weight, D2b_weight;
prior_flags	 prior_flag;
static  bool     nmlz=FALSE;
static  bool     prior_flag_set=FALSE;
static  double   prior_nml = 0.0;
#ifdef PRIOR_NML
static  double   prior_nmlzr();
#endif

#define  normalized_prior  ( flag_set(prior_flag,wallace_weight) && \
			     flag_set(prior_flag,attr_choice)  )

/* 
 *       WALLACE-OLIVER PRIOR for decision graphs and trees
 */
static double  p_join = 0.0;
static double  *l_code_length;
static double  *n_code_length;
static float   j_code_length;
/* End DECISION GRAPH PRIOR */


/*
 *	resets alpha value used for all dirichlets
 */
set_palpha(val,cnts)
float    val;
float    *cnts;
{
        float  tot = 0.0, pa;
	int   i;
	  
	if ( !ndecs )  return;
	if ( palphadec ) sfree(palphadec);
	palphadec = (float * ) salloc(sizeof(float)*ndecs);
	if ( cnts && flag_set(prior_flag,alpha_nonsym) ) {
	  fordecs(i)
	    tot += cnts[i]+1.0;
	  fordecs(i)
	    palphadec[i] = palphaval*(cnts[i]+1.0)/tot;
	} else
	  fordecs(i)
	    palphadec[i] = palphaval/(flag_set(prior_flag,alpha_classes)?1.0:ndecs) ;
	palphaval = val;
	/*
	 *      each of these three formula occur in various places below;
         *      they are constant in tree so calculate once here
	 *      NB.   code comes from  leaf_weight(), Dleaf_weight(), etc.
	 */
	if ( !(flag_set(prior_flag,node_prop)) ) {
	  if ( flag_set(prior_flag,alpha_nonsym) ) {
	    b_weight = gamma((double)palphatot(tot)) ;
	    fordecs(i)
	      b_weight -= gamma((double)palpha(tot,i));
	  } else
	    b_weight = gamma((double)palphatot(tot)) 
	      - ndecs*gamma((double)palpha(tot,1));
	  if ( flag_set(prior_flag,alpha_nonsym) ) {
	    pa = palphatot(tot);
	    Db_weight = pa / palphaval * digamma((double)pa);
	    fordecs(i) {
	      pa = palpha(tot,i);
	      Db_weight -= pa / palphaval * digamma((double)pa);
	    }
	  } else {
	    pa = palphatot(tot);
	    Db_weight = pa / palphaval *
	      (digamma((double)pa) - digamma((double)pa/ndecs));
	  }
	  if ( flag_set(prior_flag,alpha_nonsym) ) {
	    pa = palphatot(tot);
	    D2b_weight = pa / palphaval * pa / palphaval 
	      * di2gamma((double)pa);
	    fordecs(i) {
	      pa = palpha(tot,i);
	      D2b_weight -= pa / palphaval * pa / palphaval 
		* di2gamma((double)pa);
	    }
	  } else {
	    pa = palphatot(tot);
	    D2b_weight = pa / palphaval *
	      (di2gamma((double)pa) - di2gamma((double)pa/ndecs));
	  }
	}
}

#ifdef GRAPH

double lookup_p_join()
{
	return (p_join);
}

float join_weight()
{
        return( j_code_length );
}
#endif

parents_arity(t)
ot_ptr t;
{
int parent_arity, j;
bt_ptr bt2;
float pa;
        if (t && t->parents) {
                bt2 = t->parents[0];
                if (bt2) {
			if (t->num_parents == 1)
				return ( outcomes(bt2->test) );
                        pa = 0.01;
                        for (j=0; j<t->num_parents; j++) {
                                bt2 = t->parents[j];
                                pa += outcomes(bt2->test);
                        }
                        parent_arity = pa / (float)t->num_parents;
        		return (parent_arity);
                }
        }
        return 1;
}

/*
 *	calculates prior weight of internal node
 *        NB.   same stuff also hard coded into the normalizer
 */
float node_weight(testing, bt)
egtesttype  testing;
bt_ptr bt;
{
float  nw;
        if ( flag_set(prior_flag,wallace_weight) ) {
                nw = n_code_length[parents_arity(bt->parent)];
	} else {
		nw = n_weight;
        }
	if  ( flag_set(prior_flag,attr_choice) )
		     return nw - int_log[testchcs(testing)];
		else
		     return nw;
}

/*
 *	calculates prior weight of leaf node
 *        NB.   same stuff also hard coded into the normalizer
 */
float leaf_weight(cnt, t)
float cnt;     /*   see palpha()  */
ot_ptr t;
{
int i;
        if ( flag_set(prior_flag,node_prop) )
          if ( flag_set(prior_flag,alpha_nonsym) ) {
            b_weight =  gamma((double)palphatot(cnt)) ;
            fordecs(i)
              b_weight -= gamma((double)palpha(cnt,i));
          } else
            b_weight = gamma((double)palphatot(cnt)) -
              ndecs*gamma((double)palpha(cnt,1));

	ASSERT(t)
        if ( flag_set(prior_flag,wallace_weight) ) {
                return b_weight + l_code_length[parents_arity(t)];
        } else
                return b_weight + l_weight;
}

/*
 *     NB.   no longer consistent with above 
 */
float leaf_weight2(parent_arity)
int parent_arity;
{
        if ( flag_set(prior_flag,wallace_weight) ) {
                return l_code_length[parent_arity];
	} else {
                return  l_weight;
        }
}

/*
 *	calculates derivative of prior weight of leaf node w.r.t. either
 *        palphaval
 */

float Dleaf_weight(cnt)
     float cnt;   /*   see palpha()  */
{
        float  pa;
	int i;
	if ( flag_set(prior_flag,node_prop) ) 
	   if ( flag_set(prior_flag,alpha_nonsym) ) {
	     pa = palphatot(cnt);
	     Db_weight = pa / palphaval * digamma((double)pa);
	     fordecs(i) {
	       pa = palpha(cnt,i);
	       Db_weight -= pa / palphaval * digamma((double)pa);
	     }
	   } else {
	     pa = palphatot(cnt);
	     Db_weight = pa / palphaval *
	       (digamma((double)pa) - digamma((double)pa/ndecs));
	   }
	return Db_weight;
}
 
float D2leaf_weight(cnt)
      float cnt;   /*   see palpha()  */
{
	 float   pa;
	 int    i;
	 if ( flag_set(prior_flag,node_prop) ) 
	   if ( flag_set(prior_flag,alpha_nonsym) ) {
	     pa = palphatot(cnt);
	     D2b_weight = pa / palphaval * pa / palphaval 
	       * di2gamma((double)pa);
	     fordecs(i) {
	       pa = palpha(cnt,i);
	       D2b_weight -= pa / palphaval * pa / palphaval 
		 * di2gamma((double)pa);
	     }
	   } else {
	     pa = palphatot(cnt);
	     D2b_weight = pa / palphaval * pa / palphaval *
	       (di2gamma((double)pa) - di2gamma((double)pa/ndecs)/ndecs);
	   }
	 return D2b_weight;
}

/*
 *	initialise prior from command line argument
 *      must behave OK if called multiple times with same/different
 *      options
 */

prior_opts(c,optarg)
int	c;
char	*optarg;
{
	 if ( ! prior_flag_set ) {
	   prior_flag_set = TRUE;
	   null_flags(prior_flag);
           if (decision_graph_flag) 
		 set_flag(prior_flag,wallace_weight);
	 }
	 switch (c)
	 {
	 case 'N':
		 nmlz++;
		 break;
	 case 'd':
	         if ( sscanf(optarg," %d", &depth_bound) <= 0 )
            	   uerror("incorrect argument for option 'd'","");
		 break;
	 case 'A':
		 if ( sscanf(optarg,"nonsym,%f", &palphaval)==1  ) {
		   set_flag(prior_flag, alpha_nonsym);
		 } else if ( !strcmp(optarg,"null") ) {
		   palphaval = 0.0000001;
		   unset_flag(prior_flag, alpha_classes);
		   unset_flag(prior_flag, alpha_nonsym);
		 } else if ( !strcmp(optarg,"classes") ) {
		   palphaval = 1.0;
		   set_flag(prior_flag, alpha_classes);
		 } else if ( sscanf(optarg,"classes,%f", &palphaval)==1  ) {
		   set_flag(prior_flag, alpha_classes);
		 } else if ( sscanf(optarg,"%f", &palphaval) ==1  ) {
		   unset_flag(prior_flag, alpha_nonsym);
		   unset_flag(prior_flag, alpha_classes);
		 } else
		   error ("incorrect argument for option 'A'\n","");
		 if ( palphaval<=0 )
		       error ("non-positive alpha value for option 'A'\n","");
		 break;
	 case 'P':
#ifdef GRAPH
                if (decision_graph_flag) {
		        l_weight = 0;
			set_flag(prior_flag,wallace_weight);
		        if ( !strcmp(optarg,"null") ) {
			  unset_flag(prior_flag, alpha_classes);
			  unset_flag(prior_flag, alpha_nonsym);
			  unset_flag(prior_flag,attr_choice);
			  unset_flag(prior_flag,wallace_weight);
			  palphaval = 0.0000001;
			  l_weight = 0.0;
			  p_join = 0.0;
		        } else if ( !strcmp(optarg,"mml") ) {
			  set_flag(prior_flag,search_p_join);
			} else if ( !strcmp(optarg,"nochoices") ) {
			  unset_flag(prior_flag,attr_choice);
			} else if ( !strcmp(optarg,"choices") ) {
			  set_flag(prior_flag,attr_choice);
			} else if (sscanf(optarg,"mml,%lf", &p_join) <=0 )
                                error("option 'P' with decision graphs wrong\n","");
			  else 
	                        unset_flag(prior_flag,search_p_join);
 
                } else {
#endif
		  if ( !strcmp(optarg,"null") ) {
		    unset_flag(prior_flag, alpha_classes);
		    unset_flag(prior_flag, alpha_nonsym);
		    unset_flag(prior_flag,attr_choice);
		    unset_flag(prior_flag,wallace_weight);
		    palphaval = 0.0000001;
		    l_weight = 0.0;
		    p_join = 0.0;
		  } else if ( !strcmp(optarg,"nochoices") ) {
		    unset_flag(prior_flag,attr_choice);
		  } else if ( !strcmp(optarg,"choices") ) {
		    set_flag(prior_flag,attr_choice);
		  } else if ( !strcmp(optarg,"mml") ) {
		    set_flag(prior_flag,wallace_weight);
		    l_weight = 0;
		  } else if ( sscanf(optarg,"mml,%f", &l_weight )==1 ) {
		    set_flag(prior_flag,wallace_weight);
		  } else if ( sscanf(optarg,"weight,%f,%f", 
				     &n_weight, &l_weight) >= 1 ) {
		    unset_flag(prior_flag,wallace_weight);
		  } else
		    error("incorrect argument for option 'P'\n","");
#ifdef GRAPH
                }
#endif
	        break;
	 default:
		 error("incorrect option for prior","");
	 }
}

/*
 *	initialise current values from prior structure
 *       (argument of 0 means they are already set by prior_opts()
 *        so just clean things up)
 */

install_prior(tpr,cnts)
t_prior	*tpr;
float   *cnts;
{
	 if ( ! prior_flag_set ) {
	   prior_flag_set = TRUE;
	   null_flags(prior_flag);
	 }
	 if ( tpr ) {
		 palphaval = tpr->alphaval;
		 n_weight = tpr->node_weight;
		 l_weight = tpr->leaf_weight;
                 p_join = tpr->p_join;
		 prior_flag = tpr->prior_flag;
		 depth_bound = tpr->depth_bound;
		 prior_nml = tpr->prior_nml;
	}
	/*
	 *   this just initialises the *b_weight's, if required
	 */
        if ( flag_set(prior_flag,wallace_weight) )
                set_wall_prior(p_join);
	set_palpha(palphaval,cnts);
#ifdef PRIOR_NML
        if ( nmlz && ! normalized_prior ) {
	  if ( decision_graph_flag ) {
	    fprintf(stdrep,
		    "WARNING:  can't normalize graphs.\n");
	    prior_nml = 0.0;
	  } else {
	    if ( depth_bound == MAX_TREE_DEPTH  ) {
	      uerror(stdrep,
		    "WARNING:  prior being normalized with unbounded depth, use -d.\n");
	    } 
	    prior_nml = - prior_nmlzr(depth_bound);
	  }
	} else
	  /*
	   *    variable_weight priors are guaranteed normalized
	   */
#endif
	  prior_nml = 0.0;
}

/*
 *	load current values into prior structure tpr
 */

load_prior(tpr)
t_prior	*tpr;
{
	tpr->depth_bound = depth_bound;
	tpr->alphaval = palphaval;
	tpr->node_weight = n_weight;
	tpr->leaf_weight = l_weight;
        tpr->p_join = p_join;
	tpr->prior_flag = prior_flag;
	tpr->prior_nml = prior_nml;
}

display_prior()
{
    int   i, j;
    fprintf(stdrep, "PRIOR OPTIONS: \n");
    if ( flag_set(prior_flag,alpha_nonsym) ) {
       fprintf(stdrep, "alpha (prior weights for Dirichlet): ");
       for (i=0; i<ndecs; i++)
	 fprintf(stdrep, "%g,", palphadec[i]);
       fprintf(stdrep, "\n");
       if ( flag_set(prior_flag,node_prop) ) 
	 fprintf(stdrep, "    (multiply each by node-proportion)\n" );
    } else if ( flag_set(prior_flag,node_prop) )
       fprintf(stdrep, 
    "alpha (prior weight for symmetric Dirichlet): %g * (node-proportion).\n",
	  palphaval/ndecs);
    else
       fprintf(stdrep, 
	  "alpha (prior weight for symmetric Dirichlet): %g.\n",
	  palphaval/ndecs);
    fprintf(stdrep, "Maximum tree depth = %d.\n", depth_bound);
#ifdef GRAPH
    if (decision_graph_flag) {
        fprintf(stdrep,"Oliver-Wallace prior for graphs:\n");
        if ( flag_set(prior_flag,attr_choice) )	
            fprintf(stdrep, 
		"  Node probability divided by no. of test choices.\n" );
        fprintf(stdrep,"  Join probability = %8.2f,\n",      p_join );
        fprintf(stdrep,"  Root code leaf = %8.2f,\t",      l_code_length[1]);
        fprintf(stdrep,"  root split = %8.2f.\n",          n_code_length[1]);
        for (i=2; i<=nvalsattr; i++) {
           forattrs(j)  
		if ( type(j)==DISCRETE && unordvals(j)==i )
			break;
	   if ( i!=2 && j>nattrs )
		continue;
           fprintf(stdrep,"     Code leaf (%d) = %g,  ", i,l_code_length[i]);
           fprintf(stdrep,"split (%d) = %g,  ", i,n_code_length[i]);
           if ( p_join )
		fprintf(stdrep,"join (%d) = %g.\n", i,j_code_length);
	   else 
		fprintf(stdrep,"join (%d) = -MAXFLOAT.\n", i);
        }
    } else {
#endif
      if ( flag_set(prior_flag,wallace_weight) ) {
	  fprintf(stdrep, "Wallace prior for trees.\n" );
          if ( l_weight )
              fprintf(stdrep, "\tTree expected to shrink after %g bits tested.\n", 
                          l_weight );
          if ( flag_set(prior_flag,attr_choice) )	
            fprintf(stdrep, 
		"Node probability divided by no. of test choices.\n" );
      } else {
	fprintf(stdrep, 
		"Leaf and node weights (neg log probability in nits): %g %g.\n",
		-l_weight, -n_weight);
        if ( flag_set(prior_flag,attr_choice) )	
            fprintf(stdrep, 
		"Node probability divided by no. of test choices.\n" );
      }
#ifdef PRIOR_NML
      if ( nmlz ) {
	  fprintf(stdrep, 
		"Neg log. of tree prior normalizing constant (nits) = %lg.\n",
		 prior_nml);
          for ( i = 0; i < depth_bound; i++) {
	      fprintf(stdrep, "   log aprior of max. depth <= %d = %lg\n", 	
		          i, prior_nmlzr(i)+prior_nml );
	  }
      } else
#endif
	if ( ! normalized_prior )
	  fprintf(stdrep, "Warning:  tree prior unnormalized.\n" );
#ifdef GRAPH
    }
#endif
    fprintf(stdrep, "\n");
}

set_wall_prior(pj)
double pj;
{
        double p_split, p_leaf;
        int max_arity, i;
	double  pow();

        max_arity = 2;
        if ( nvalsattr > 2 )
	  max_arity = nvalsattr;
        if (!l_code_length) {
                l_code_length = make_dvec(max_arity+3);
                n_code_length = make_dvec(max_arity+3);
        }
        for (i=2; i<=max_arity; i++) {
                p_split = (1.0 - pj * 0.5) / (float) i;
                p_leaf  =  1.0 - pj - p_split;
                l_code_length[i] = log(p_leaf);
                n_code_length[i] = log(p_split);
                if (pj == 0.0)
                        j_code_length = - FLOATMAX;
                else
                        j_code_length = log(pj);
        }
        n_code_length[1] = log(1.0 - (1.0/nattrs));
        l_code_length[1] = log(1.0/nattrs);
	init_int_log(nattrs+2);
}

#ifdef PRIOR_NML
/***************************************************************/
/*
 *	for current prior parameters, find log of the normalising
 *      constant;
 *      ignores possibility of subsetting + other;
 *      should take similar time to building 1 tree to "depth_bound"
 *
 */
static int     *pn_valence;
double     el_weight,  en_weight;
       /*  stores counts for test valencies for current position
	   e.g.   #cut-point tests = pn_valence[1];
	          #binary tests = pn_valence[2]; 
        */
static int     pn_val_cnt;
       /*   max. allowed depth for this call  */
static  int    pn_dep;
       /*   all results are scaled by pn_scale*100.0 on return  */
static  int  pn_scale;
static  double MAX_scale;
static  double LOG_scale = 50.0;

static double prior_nmlzr(dep)
int   dep;
{
    int    i;
    double  pnd_store;
    double  exp(), log();
    static double _prior_nml_dep();
    pn_dep = dep;
    pn_valence = make_ivec(nvalsattr+3);
    MAX_scale = exp(LOG_scale);
    pn_val_cnt = 0;
    el_weight = exp(l_weight);
    en_weight = exp(n_weight);
    forattrs(i) {
      if ( i==decattr ) continue;
      pn_val_cnt++;
      if ( num_type(i) || set_type(i) ) {
	/*
	 *     binary tests that can occur further down the tree 
	 */
	pn_valence[1]++;
      } else if ( Cnull(onlyif(i)) || ! Cnever(onlyif(i)) ) {
	/*
	 *     once these tests occur, they can be repeated
	 */
	if ( unordsubsets(i) && unordsubsets(i)!=SB_REST  )
	  pn_valence[1]++;
	else
	  /*
	   *     once these tests occur, they can't be repeated
	   */
	  pn_valence[unordvals(i)]++;
      }
    }
    pnd_store = _prior_nml_dep(0,1);
    sfree(pn_valence);
    return  log(pnd_store) + pn_scale*LOG_scale;
  }

/*
 *      calc.  prior sum to depth "dep"
 *      where allowable tests are given by "pn_valence"
 *      and answer is returned to the "times" power;
 *      basic calculations cause overflow so calculation is done 
 *      using "this_sum" and "this_scale" which are combined thus:
 *                this_sum * MAX_scale^this_scale
 *
 *      real hack for subsetting so watch out
 */
static double _prior_nml_dep(dep,times)
int  dep;
int  times;
{
    int    i;
    double this_sum;
    double  pn_sum;
    /*   scaling for the current calculation is stored here */
    int   this_scale=0;
      /*
       *    macro to do  "this_sum += rp_sum",  but watch out for scaling
       */
#define  add_pn_to_this()          \
      if ( pn_scale == this_scale )\
	this_sum += pn_sum;\
      else if ( pn_scale == this_scale+1 ) {\
	this_scale++;\
	this_sum = this_sum/MAX_scale + pn_sum;\
      } else if ( pn_scale == this_scale-1 ) {\
	this_sum = this_sum + pn_sum/MAX_scale;\
      } else if ( pn_scale > this_scale+1 ) {\
	this_sum = pn_sum;\
	this_scale = pn_scale;\
      } 

    /*
     *     stop when reach depth_bound or out of attributes
     */
    if ( ! pn_val_cnt || dep>=depth_bound  ) {
      pn_scale = 0;
      return 1.0;
    } 

    /*
     *     calc prob. for subtree in (this_sum,this_scale)
     */
    if ( dep>=pn_dep ) {
      /*
       *       force leaves at this level
       */
      /*
       *   initialize "this_sum" to the leaf weight
       */
      if ( flag_set(prior_flag,wallace_weight) ) {
        if ( times > 1 )
	  this_sum = 1 - p_join - (1.0 - p_join * 0.5) / (float) times;
	else
	  /*
	   *   its the root node
	   */
	  this_sum  = 1.0/nattrs;
      } else
	this_sum = el_weight;
    } else {
      /*
       *   initialize "this_sum" to the node weight
       */
      if ( flag_set(prior_flag,wallace_weight) ) {
        if ( times > 1 )
	  this_sum  = (1.0 - p_join * 0.5) / (float) times;
	else
	  /*
	   *   its the root node
	   */
	  this_sum  = 1.0 - (1.0/nattrs);
      } else
	this_sum = en_weight;
      for (i=1; i<= nvalsattr; i++) {
	if ( i==1 && pn_valence[1] ) {
	  /*
	   *     these tests can be repeated
	   */
	  pn_sum = pn_valence[1] * en_weight
	    /  ((flag_set(prior_flag,attr_choice))?pn_val_cnt:1.0)
	      * _prior_nml_dep(dep+1,2);
	  add_pn_to_this();
	} else if ( i>1 && pn_valence[i] ) {
	  pn_valence[i]--;
	  pn_val_cnt--;
	  pn_sum = _prior_nml_dep(dep+1,i);
	  pn_valence[i]++;
	  pn_val_cnt++;
	  pn_sum *=  pn_valence[i] *  en_weight 
	    / ((flag_set(prior_flag,attr_choice))?pn_val_cnt:1.0);	
	  add_pn_to_this();
	} else 
	  continue;
      }
    }

    /*
     *    raise  (this_sum,this_scale)  to power integer "times"
     *    and store result in  (pn_sum,pn_scale)
     */
    pn_scale = this_scale * times;
    pn_sum = 1.0;
    this_scale = 0;
    for ( i=1; i<=times; i<<=1, this_sum *= this_sum) {
      /*
       *    correct scale of (this_sum,this_scale)
       */
      while ( this_sum > MAX_scale ) {
             this_sum /= MAX_scale;
	     this_scale++;
	     } 
      while ( this_sum < 1.0/MAX_scale ) {
	     this_sum *= MAX_scale;
	     this_scale--;
      } 
      if ( i & times ) {
	     /*
	      *    multiply (pn_sum,pn_scale) by (this_sum,this_scale)
	      */
             pn_sum  *= this_sum;
	     while ( pn_sum > MAX_scale ) {
	       pn_sum /= MAX_scale;
	       pn_scale++;
	     } 
	     while ( pn_sum < 1.0/MAX_scale ) {
	       pn_sum *= MAX_scale;
	       pn_scale--;
	     } 
	     pn_scale += this_scale;
      }
    }
    return pn_sum;
}

#endif

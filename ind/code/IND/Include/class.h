/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* defns. for Bayes classifiers  */

#define PI(j)  _PI[j]
#define ATTR(k)  _ATTR[k]
#define dens_typ(k)  _dens_typ[k]
#define ATTR_VAL_PROB(j,k,l)  THETA[k]._ATTR_VAL_PROB[j][l]
#define MEAN(j,k)  THETA[k].CT._MEAN[j]
#define STDV(j,k)  THETA[k].CT._STDV[j]
#define LAMBDA(j,k)  THETA[k]._LAMBDA[j]
#define STEP(j,k,l)  THETA[k].ST._STEP[j][l]
#define CUT_POINT(j,k)  THETA[k].ST._CUT_POINT[j]

typedef struct classifier classifier;
typedef union theta theta;

struct classifier
{
	int	class_max;		/* number of classes */
	int	node_max;		/* number of nodes */
	double   *_PI;
	char     *_dens_typ;            /* density function to use */
	int	 *_ATTR;	
	union theta
	{
		double	**_ATTR_VAL_PROB;
		struct Step
		{
			double	**_STEP;
			double	*_CUT_POINT;
		}
			ST;
		struct Cont
		{
			double 	*_MEAN;
			double 	*_STDV;
		}
			CT;
		double	*_LAMBDA;	
	}	*THETA;
};


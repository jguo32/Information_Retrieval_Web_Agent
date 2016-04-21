/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/***********************************************
 *
 *	defined in  Statlib
 */
float	*strat_fprob();
int	best_dec(), most_common();
int	best_cnt_dec();
float	best_util();
float	lbest_util();
float	sub_util();
float	leaf_util();
double	calc_mse();
float	prob_util();
float	utility_diff();
float	mtx_utility();
double	normrandom();
long	rand_random();
long	time_random();
double	*cumm_dvec();
double	gammarandom();
double	logsum();
double	loggamma();
double	digamma();
double	di2gamma();
double	logbeta();
double	logbetal();
double	*betarandom();
double	beta2random();
double	*ubetarandom();
double	factln(), bico(), beta(), stud_t(), stud_t_inv(),
        F_dist(), binop(), bicoln(), chisqp(), chisqq();
double	betai(), betai_inv();


/**********************************************
 *
 *	defined in  Eglib
 */
/*
 *	tests
 */
float	getmin_cur(), getmax_cur();
/*
 *	statistics table processing
 */
void	tzero(), tmake(), tmod(), tmod_count(),tfree(), talloc();
int	tprint(), most_freq();
double	calc_marsh();
void	calc_log_prob();
void	calc_leaf_log_prob();
void	calc_leaf_gain();
void	trestore(), tsave(), table_tree(), calc_error();
void	restrat_fprob();

/* 
 *	defined elsewhere
 */
char	*strcat(); 
char	*strrchr(); 
double	sqrt(), log(), exp(), gamma();
double  ceil(), floor();

long	lrand48();
double	drand48();
void	srand48();
/* #define DEBUG_RAND  */
#ifdef DEBUG_RAND
long  lrandom();
double frandom();
void srandom();
#else
#define lrandom()     lrand48()
#define frandom()     drand48()
#define srandom(seed)     srand48((long)seed)
#endif


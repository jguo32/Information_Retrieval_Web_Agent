/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include   <stdio.h>
#include   "Lib.h"
#include   "DEFN.h"

/*
 *       Routines to help with random sampling:
 *   
 *       sample_opts(optarg) = read in character string giving options
 *
 *       init_sample(total,train) =  get length of total file to sample
 *                             and do initialization
 *                               train = 0 for testing, 1 for training
 *
 *       next_in_sample() = return index to next item in file to sample
 *                               return -1 on completion
 *
 *	 size_sample() = size of sample taken, only valid after
 *			 call to init_sample() and before first call
 *			 to next_in_sample()
 */


/* #define TEST_SMPL  */
#ifdef TEST_SMPL 
int  verbose = 0;
#else
extern int verbose;
#endif

static  long    seed = 1234567;
static	int	total_size;
static	int	total_size_left;
static  int	train;		/*    1 for training, 0 for testing */
static union  {
    int	smple_size_left;
    struct {
    	int     sl_left;      /* Number of records selected so far, to left */
	int     sl_right;  
	int     sl_part;      /* ..., in the selected partition  */
	int     sl_rem;       /* stuff not in either train or test */
	int     parts, part;
	int	smple_size;
    } cv;
    struct {
	int     *sc_wr_buf, *tp_wr_buf;
	int	sc_wr_buf_size, smple_size;
    } wr;
} sp;
#define  sample_size_left  sp.smple_size_left
#define  sel_left  sp.cv.sl_left
#define  sel_right  sp.cv.sl_right
#define  sel_part  sp.cv.sl_part
#define  sel_rem  sp.cv.sl_rem
#define  sec_wr_buf  sp.wr.sc_wr_buf
#define  top_wr_buf  sp.wr.tp_wr_buf
#define  sec_wr_buf_size  sp.wr.sc_wr_buf_size
#define  sample_size    sp.wr.smple_size
#define  cv_sample_size    sp.cv.smple_size
#define  partitions     sp.cv.parts
#define  partition     sp.cv.part

/*
 *	training = sample a subset with out replacement, test = leftovers
 */
#define	WOUT_REPLACE  1
/*
 *	training = sample a subset with replacement, test = leftovers
 */
#define	WITH_REPLACE  2
/*
 *	training = sample a subset with replacement, test = full set
 */
#define LOGICAL_SMPL  3
/*
 *	training = all but the i-th partition, test = i-th partition
 */
#define CV_SMPL	      4
/*
 *	training = test = whole
 */
#define NO_SAMPLE     5

unsigned char	sample_style;

Sampler *
sample_opts(optarg)
char *optarg;
{
     char  *strtok();
     char  type;
     char  buf[100];
     int   ret;
     Sampler  *sampler;
     if ( sscanf(optarg,"%c,", &type) < 1 )
       uerror("sampling options null","");
     sampler = mem_alloc(Sampler);
     sampler->s_seed = 0;
     switch( type ) {
     case 'P' :
	sfree(sampler);
	return 0;
     case 'N' :
     	sampler->s_type = NO_SAMPLE;
     	sampler->s_seed = 1;
        break;
     case 'S' :
     	sampler->s_type = WOUT_REPLACE;
	if ( (ret=sscanf(optarg,"S,%d,%ld%s",
			 &sampler->s_sample_size,&sampler->s_seed,&buf[0]))<1
	     || ret > 2 )
	  uerror("sampling options for S incorrect","");
	if ( sampler->s_sample_size<=0 )
	  uerror("sample size not positive","");
	
	break;
     case 'R' :
     	sampler->s_type = WITH_REPLACE;
	if ( (ret=sscanf(optarg,"R,%d,%ld%s",
			 &sampler->s_sample_size,&sampler->s_seed,&buf[0]))<1
	     || ret > 2 )
	  uerror("sampling options for R incorrect","");
	if ( sampler->s_sample_size<=0 )
	  uerror("sample size not positive","");
	break;
     case 'L' :
    	sampler->s_type = LOGICAL_SMPL;
	if ( (ret=sscanf(optarg,"L,%d,%ld%s",
			 &sampler->s_sample_size,&sampler->s_seed,&buf[0]))<1
	    || ret > 2 )
	  uerror("sampling options for L incorrect","");
	if ( sampler->s_sample_size<=0 )
	  uerror("sample size not positive","");
	break;
     case 'D' :
     	sampler->s_type = CV_SMPL;
	if ( (ret=sscanf(optarg,"D,%d,%d,%d,%ld%s",
		 &sampler->s_sample_size,
		&sampler->s_partitions,&sampler->s_partition,
		&sampler->s_seed,&buf[0]))<3
	     || ret > 4 )
	  uerror("sampling options for C incorrect","");
	if ( sampler->s_partition>= sampler->s_partitions 
		|| sampler->s_partition<0 || sampler->s_partitions <=0 )
	  uerror("partitioning parameters incorrect","");
	break;
     case 'C' :
     	sampler->s_type = CV_SMPL;
	sampler->s_sample_size = 0;
	if ( (ret=sscanf(optarg,"C,%d,%d,%ld%s",
			 &sampler->s_partitions,
		&sampler->s_partition,&sampler->s_seed,&buf[0]))<2
	     || ret > 3 )
	  uerror("sampling options for C incorrect","");
	if ( sampler->s_partition>= sampler->s_partitions || 
		sampler->s_partition<0 || sampler->s_partitions <=0 )
	  uerror("partitioning parameters incorrect","");
	break;
     }
     return sampler;
}

static
new_sec_wr_buf()
{
  int  i,j;
  /*    block number  */
  j = (total_size-total_size_left)/sec_wr_buf_size ;
  /*
   *    if on the last block only look at remainders
   */
  if ( (j+1)*sec_wr_buf_size > total_size )
    sec_wr_buf_size = total_size - j*sec_wr_buf_size;
  /*
   *   next block of "sec_wr_buf_size" items in total/full sample
   *   occur "top_wr_buf[j]" times in the sample chosen;
   *   so construct the individual sample from this block
   */
  for ( i=sec_wr_buf_size-1; i>=0; i-- )
    sec_wr_buf[i]=0;
  for ( i=top_wr_buf[j]; i>0; i-- ) {
    sec_wr_buf[ (int) (frandom() * sec_wr_buf_size) ] ++;
  }
}

int
size_sample()
{
	switch ( sample_style ) {
	case WOUT_REPLACE :
	  if ( train )
	  	return sample_size_left;
	  else
	  	return total_size - sample_size_left;
	case LOGICAL_SMPL :
	  if ( train )
	  	return sample_size;
	  else
	  	return total_size;
	case WITH_REPLACE :
	  if ( train )
	  	return sample_size;
	  else
		/*	this answer is rubbish  */
	  	return total_size;
	case CV_SMPL :
	  if ( train )
	  	return cv_sample_size - sel_part;
	  else
	  	return sel_part;
	case NO_SAMPLE :
	  return total_size;
	}
	return -1;
}

int
next_in_sample()
{
	double     fr;
	int        i, this;
	switch ( sample_style ) {
	case WOUT_REPLACE :
	  if ( total_size_left && sample_size_left >= total_size_left ) {
	    sample_size_left--;
	    total_size_left--;
	    return total_size - total_size_left - 1;
	  }
	  for ( ; total_size_left>0; ) {
	        if ( frandom() < 
		    ((double)sample_size_left)/total_size_left-- ) {
	          sample_size_left--;
		  if ( train )
		    return total_size - total_size_left - 1;
		} else if ( !train )
		  return total_size - total_size_left - 1;
	  }
	  return -1;
	case LOGICAL_SMPL :
	  if (total_size_left<=0) {
	    if ( sec_wr_buf ) {
	      sfree(sec_wr_buf);
	      sfree(top_wr_buf);
	    }
	    return -1;
	  }
	  if ( !train )
		return total_size - total_size_left--;
	case WITH_REPLACE :
	  if (total_size_left<=0) {
	    if ( sec_wr_buf ) {
	      sfree(sec_wr_buf);
	      sfree(top_wr_buf);
	    }
	    return -1;
	  }
	  for (;;) {
	    i = (total_size-total_size_left)%sec_wr_buf_size ;
	    if ( train && sec_wr_buf[i] > 1 ) {
	      sec_wr_buf[i]--;
	      return total_size - total_size_left;
	    } else {
	      --total_size_left;
	      this = (sec_wr_buf[i])? train : !train;
	      /*
	       *    compute the new "sec_wr_buf"  if needed
	       */
	      if ( total_size_left && i+1==sec_wr_buf_size ) 
		  new_sec_wr_buf();
	      if ( this )
	        return total_size - total_size_left - 1;
	      else if ( total_size_left <= 0 ) {
	        if ( sec_wr_buf ) {
	          sfree(sec_wr_buf);
	          sfree(top_wr_buf);
	        }
		return -1;
	      }
	    }
	  }
	case CV_SMPL :
	  for ( ; total_size_left>0 ; total_size_left-- ) {
                fr = frandom() * total_size_left;
		if ( fr < sel_left ) {
                        sel_left--;
                        if ( train ) 
	      		    return total_size - total_size_left--;
                } else if ( fr < sel_left+sel_part ) {
			sel_part--;
                        if ( !train ) 
	      		    return total_size - total_size_left--;
                } else if ( fr < sel_left+sel_part+sel_right ) {
			sel_right--;
                        if ( train ) 
	      		    return total_size - total_size_left--;
		} else {
			sel_rem--;
		}
          }
	  return -1;
	case NO_SAMPLE :
	  if ( total_size_left > 0 )
          	return total_size - total_size_left--;
	  else
		return -1;
	  break;
	}
	return -1;
}

static
with_repl_sample()
{
	int	i;
	double   sqrt();
	if ( total_size > 2000 ) {
		sec_wr_buf_size = sqrt((double)total_size) + 0.5;
		top_wr_buf = mems_alloc(int,1+total_size/sec_wr_buf_size);
		/*
	 	 *   initialize the top_wr_buf[] to be count of
 	 	 *   sample in each block of size "sec_wr_buf_size"
	 	 */
		for ( i=total_size/sec_wr_buf_size; i>=0; i-- )
		    top_wr_buf[i]=0;
		for ( i=0; i< sample_size; i++) {
		    top_wr_buf[(int)((frandom()*total_size)/sec_wr_buf_size) ] ++;
		}
	} else {
		sec_wr_buf_size = total_size;
		top_wr_buf = mem_alloc(int);
		top_wr_buf[0] = sample_size;
	}
	sec_wr_buf = mems_alloc(int,sec_wr_buf_size);
	new_sec_wr_buf();
}

static
cross_val_sample()
{
	if ( !cv_sample_size )
		cv_sample_size = total_size;
	/*
	 *	work out how many in each group
	 */
        sel_left = (cv_sample_size/partitions) * partition;
        sel_part = (cv_sample_size/partitions);
        sel_right = (cv_sample_size/partitions) * (partitions-partition-1);
	/*    
	 *	assign leftovers  
	 */
	if ( cv_sample_size%partitions <= partition )
		sel_left += cv_sample_size%partitions;
	else {
		sel_left += partition;
		sel_part++;
		if ( cv_sample_size%partitions > partition+1 ) 
			sel_right += cv_sample_size%partitions - partition - 1;
	}
	/*
	 *	assign the sample remainder left out of the partition
	 */
	sel_rem = total_size - cv_sample_size;
}

no_sample(sampler)
Sampler  *sampler;
{
	sampler->s_type = NO_SAMPLE;
	sampler->s_seed = 1;
}

init_sample(total,trn,sampler)
int    total;
int    trn;
Sampler  *sampler;
{
  if ( sampler->s_type != NO_SAMPLE ) {
  	srandom((long)sampler->s_seed);
  }
  train = trn;
  total_size_left = total;
  total_size = total;
  seed = sampler->s_seed;
  sample_style = sampler->s_type;
  switch ( sample_style ) {
  case WOUT_REPLACE :
	sample_size_left = sampler->s_sample_size;
        if ( verbose ) 
	  fprintf(stdrep,
		  "SAMPLING:  %d without replacement from %d using seed %ld.\n",
		  sample_size_left, total, seed);
	break;
  case WITH_REPLACE :
	sample_size = sampler->s_sample_size;
        if ( verbose ) 
	  fprintf(stdrep,
		  "SAMPLING:  %d with replacement from %d using seed %ld.\n",
		  sample_size, total, seed);
        with_repl_sample();
	break;
  case LOGICAL_SMPL :
	sample_size = sampler->s_sample_size;
        if ( verbose ) {
	  fprintf(stdrep,
		  "SAMPLING:  %d with replacement from %d using seed %ld,\n",
		  sample_size, total, seed);
	  fprintf(stdrep, "  testing on full set.\n");
	}
        with_repl_sample();
	break;
  case CV_SMPL :
	cv_sample_size = sampler->s_sample_size;
        partitions = sampler->s_partitions;	
	partition = sampler->s_partition;
        if ( verbose ) {
	  if ( partitions==total )
	     fprintf(stdrep, "SAMPLING:  part %d of leave-one-out X-valid.",
			 partition);
	  else
	     fprintf(stdrep, "SAMPLING:  part %d of %d-fold X-valid.", 
			1+partition, partitions);
	  if ( cv_sample_size < total && cv_sample_size )
	    fprintf(stdrep, " from subsample of %d", cv_sample_size);
	  fprintf(stdrep, " using seed %ld.\n", seed);
	}
        cross_val_sample();
	break;
  case NO_SAMPLE :
        if ( verbose ) 
	    fprintf(stdrep, "SAMPLING:  take full set.\n");
	break;
  }
}

#ifdef TEST_SMPL
char  *progname = "sample";

main()
{
  char   buf[100];
  int    total;
  int    check[1000], checkt[1000];
  int    next, i, cnt;

  printf("enter sampling command: ");
  scanf("%s",&buf[0]);
  sample_opts(&buf[0]);
  printf("enter total size: ") ;
  scanf("%d",&total);

  init_sample(total);
  printf("sample sizes train: %d,  test: %d\n",
		size_sample(1), size_sample(0) ) ;
	

  for(i=0; i<total; i++ ) check[i]=0;
  for( i=0, next = next_in_sample(1); next>=0; ) {
    check[next]++;
    for ( ; i<next; i++ )  printf(".");
    for ( cnt=1; (next=next_in_sample(1))>=0 && next<=i; cnt++ ) 
      check[next]++;
    i++;
    printf("%d",cnt);
  }
  for ( ; i<total; i++ )  printf(".");  
  printf("\n");

  for(i=0, cnt=0; i<total; i++ ) cnt += check[i];
  printf("total training = %d\n", cnt);

  sample_opts(&buf[0]);
  init_sample(total);
  printf("sample sizes train: %d,  test: %d\n",
		size_sample(1), size_sample(0) ) ;

  for(i=0; i<total; i++ ) checkt[i]=0;
  for( i=0, next = next_in_sample(0); next>=0; ) {
    checkt[next]++;
    for ( ; i<next; i++ )  printf(".");
    for ( cnt=1; (next=next_in_sample(0))>=0 && next<=i; cnt++ ) 
      checkt[next]++;
    i++;
    printf("%d",cnt);
  }
  for ( ; i<total; i++ )  printf(".");  
  printf("\n");

  for(i=0, cnt=0; i<total; i++ ) cnt += checkt[i];
  printf("total test = %d,  ", cnt);

  for(i=0, cnt=0; i<total; i++ ) cnt += (checkt[i] && check[i]);
  printf("overlapping = %d\n", cnt);
  
}

#endif
  
  
    

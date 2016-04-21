/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/*  #define DEBUG_LOGPROB */

bool    do_graphs=FALSE;         /* flag to control choose.c's plotting */
extern bool jtree_flag;

static  bool    do_x_graphs=FALSE;
extern  void    init_graph_panes();
extern	bool	t_flg;		    /* transduction splitting flag */
extern	bool    Fflg;	            /* make choice of test totally random */

extern	int	depth;
extern	ot_ptr	tree;
extern  int     depth_bound;
extern  char    purflg;

extern	bt_ptr	*treeoptions;
extern  int     *bestoptions;
extern  float   *gainoptions;
extern	int	max_gmi;
extern	bool	oflg;		/* Override flag */
extern  int     intint;
extern  int     verbose;

static bt_rec	save_bt;

/*
 *    interaction is always in a given context, that changes
 *    its behaviour slightly
 */
#define CONTEXTS 8
static  char  *(context_desc[CONTEXTS]) = {"GROWING node from choices", 
				    "RETURNING node",
				    "GROWING option tree after lookahead" ,
				    "RETURNING from lookahead" ,
				    "RETURNING from _lookahead" ,
				    "RETURNING from extendtree" ,
				    "REGROWING node from heap" ,
				    "WANDERING" };
static  bool  ignore_interact[CONTEXTS] = {FALSE,FALSE,FALSE,FALSE,
					  FALSE,FALSE,FALSE,FALSE};
static  bool  creep = TRUE;


int
interact(t,testing,icontext)
     ot_ptr  t;
egtesttype  testing;
int	icontext ;   	/*  index to context_desc[]  */
{
  bt_ptr  best;
  float   best_sprob;
  char	  *measure;
  bt_ptr	option;
  tprint_flags  pflags;
  char    s[500];
  extern  int  opterr;
  extern  int  optind;
  int	ccc, override = 1;
  int   i,j,k;
  double	others, cw, lw, tw;
  float   eerr, estd;

  if ( !creep && ignore_interact[icontext] ) return override;
  creep = FALSE;
again:		
  printf("%s, interact? (type 'h' for help): ",context_desc[icontext]);
  if ( scanf(" %1s", s) <= 0 )
    s[0] = 'w';
  switch (s[0]) {
  case 'h':
  case 'H':
    if ( icontext < 7 ) {
     printf("Interaction:\n");
     printf(" 'n' = no, continue growing, 'm' = goto next interaction,\n");
     printf(" 'b' = toggle on/off interact during current mode,\n");
     printf(" 's' = no interaction for subtree, 'w' = none for parent tree.\n");
     printf("Modify growing:\n");
     printf(" 'a' = abort growing and save tree so far,\n");
     printf(" 'z' = quit and abandon tree,\n");
     printf(" 't' = change options for growing the tree,\n");  
     printf(" 'c' = change choice at this node, 'f' = force leaf. \n");
     printf(" 'd' = delete choice/option at this node. \n");
     printf("Reports on this node:\n");
     printf(" 'l' = list tests at node, 'o' = give report on options,\n");
     printf(" 'g' = print gains, 'e' = print error est. for choices\n");
     printf(" 'r' = give full report on current stored tests,\n");
     if ( ncattrs > 0 )
        printf(" 'x' = toggle on/off plotting of attribute gains,\n");
     printf(" 'k' = kill attribute gain graphs.\n");
     printf("Reports on tree:\n");
     printf(" 'q' = print subtree so far, 'p' = print tree so far,\n");
     printf(" 'v' = print parents of current node,\n");
     printf(" 'u' = print statistics on tree so far,\n");
    } else {
     printf("Interaction:\n");
     printf(" 'a' = abort growing and save tree so far,\n");
     printf(" 'z' = quit and abandon tree,\n");
     printf(" 'w' = no more interaction, 'n' = continue wandering,\n");
     printf(" 't' = change options for growing the tree,\n");  
     printf("Reports on tree:\n");
     printf(" 'p' = print tree so far,\n");
     printf(" 'u' = print statistics on tree so far,\n");
    } 
    goto again;

  case 't':
  case 'T':
    printf("Enter single 'tgen' command line option from sdiutGn: ");
    scanf( " -%c%s", &s[0], &s[1]);
    switch (s[0])
	{
	case 'v':
	  verbose++;
	  break;
	case 'd':
	  prior_opts((int)s[0],&s[1]);
	  break;
	case 's':
	  make_opts((int)s[0],&s[1]);
	  break;
	case 'i':
	case 'u':
	case 't':
	case 'G':
	case 'n':
	  choose_opts((int)s[0],&s[1]);
	  break;
	default:
 	  printf("Illegal option passed (e.g. try '-d 5' or '-n 0')\n");
	  break;
	}  
    goto again;

  case 'b':
  case 'B':
    if ( icontext >= 7 )
	goto again;
    if ( ignore_interact[icontext] ) {
	  ignore_interact[icontext] = FALSE;
          printf("Interaction when %s.\n",context_desc[icontext]);
    } else {
	  ignore_interact[icontext] = TRUE;
          printf("No interaction when %s.\n",context_desc[icontext]);
    }
    goto again;

  case 'r':              /*     report details of each potential test  */
  case 'R':
    if ( icontext >= 7 )
	goto again;
    if ( icontext==2 || icontext == 1 ) {
      printf("'r' ignored with option trees, lookahead, or on return; use 'o'\n");
      goto again;
    }
    if (t_flg) {
      /* 	show stats on rel. weights */
      others = 0.0;
      lw = FLOATMAX;
      cw = t->xtra.gr.gain - treeoptions[bestoptions[0]]->gain;
      for(i=1; i<max_gmi; i++) if (treeoptions[i]->gain > 
			-FLOATMAX*0.99999) {
	tw = treeoptions[i]->gain - treeoptions[bestoptions[0]]->gain ;
	if ( i!= bestoptions[0] )
	  others = logsum(others,tw);
	if ( (tw < lw) && (treeoptions[i]->gain > -FLOATMAX*0.9) )
	  lw = tw;
	printf("Relative nits of test (%d) ",i);
	display_test(treeoptions[i]->test,stdout);
	for ( j=0; bestoptions[j] && i!=bestoptions[j]; j++) ;
	if ( bestoptions[j] )
		printf(" = %6lg  <- choice %d\n", tw, j+1);
	else
		printf(" = %6lg\n", tw);
	if ( intint ) {
		intint = 0;
		goto again;
	}
      }
      printf("Relative nits of non-choice branches = %lg\n", others);
      printf("Relative nits of worst branch = %lg\n", lw);
      printf("Relative nits of leaf = %g - %g = %lg\n", 
	     t->xtra.gr.gain, treeoptions[bestoptions[0]]->gain, cw);
      printf("Choice = %d\n", bestoptions[0]);
    } else {	    /* 	show stats on gains */
      if ( purflg=='i' )
	measure = "Info-gain";
      else if ( purflg=='t' )
	measure = "Twoing";
      else if ( purflg=='r' )
	measure = "Gain-ratio";
      else if ( purflg=='g' )
	measure = "Gini";
      else
        measure = "measure";
      others = 0.0;
      lw = FLOATMAX;
      ccc = 0;
      for(i=1; i<max_gmi; i++)  {
	if (treeoptions[i]->gain > -FLOATMAX*0.9) {
	  tw = treeoptions[i]->gain;
	  if ( t==tree) {
	    printf("%s of test (#%d) ",measure,i);
	    display_test(treeoptions[i]->test,stdout);
	    printf(" = %6lg", tw);
	  } else {
	    printf("%s (relative) of test (#%d) ",measure,i);
	    display_test(treeoptions[i]->test,stdout);
	    printf(" = %6lg (%6lg)", tw, tw * t->tot_count/tree->tot_count);
	  }
	  if ( purflg=='r' )
	        printf(", gain = %6g", gainoptions[i]);
	  if ( i==bestoptions[0] )
		printf(" <- choice\n");
	  else
		printf("\n");
	  if ( (tw < lw) && (tw > -FLOATMAX*0.9) )
	    lw = tw;
	  if ( tw > -FLOATMAX*0.9 ) {
	    ccc++;
	    others += tw;
	  }
	}
	if ( intint ) {
		intint = 0;
		goto again;
	}
      }
      printf("Ave. gain of non-choice branches = %lg\n", others/ccc);
      printf("Gain of worst branch = %lg\n", lw);
      printf("Gain of leaf = %g\n", t->xtra.gr.gain);
      printf("Choice = %d\n", bestoptions[0]);
    }
    goto again;

  case 'o':              /*     report details of options  */
  case 'O':  
    /*
     *      find best options
     */
    if ( icontext >= 7 )
	goto again;
    if ( tleaf(t) ) {
	printf("no options in this node\n");
	goto again;
    }
    best_sprob = -FLOATMAX;
    best = (bt_ptr)0;
    foroptions(option,j,t) {
      if ( option->gain >= best_sprob ) {
	best = option;
	best_sprob = option->gain;
      }
    }
    /*      show stats on rel. weights */
    others = 0.0;
    lw = FLOATMAX;
    cw = exp( (double) (t->lprob - best->gain) );
    foroptions(option,j,t) {
      tw = exp((double)(option->gain - best->gain));
      if ( tw < lw )
	lw = tw;
      if ( option != best )
	others += tw;
#ifdef DEBUG_LOGPROB
      printf("Test %d : recalc max log. prob. = %lg ; ", 
	           j, btree_submaxw(option,testing,1) );
      printf("recalc sub log. prob. = %lg\n", 
	     btree_submaxw(option,testing,0) );
#endif
      printf("Relative probability (abs. log. prob.) of test #%d ",j);
      display_test(option->test,stdout);
      printf(" = %6lg (%g)\n", tw, option->gain );
      if ( intint ) {
		intint = 0;
		goto again;
      }
    }
    printf("Relative probability of non-choice branches = %lg\n", others);
    printf("Relative probability of worst branch = %lg\n", lw);
#ifdef DEBUG_LOGPROB
    printf("Recalc subtree max. log. prob. = %lg , ", tree_submaxw(t,testing,1) );
    printf("recalc subtree sub log. prob. = %lg \n ", tree_submaxw(t,testing,0) );  
#endif
    printf("Sum log probability of subtree = %g \n", t->xtra.gr.gain);
    printf("Relative probability (abs. log. prob.) of leaf = %lg (%g) \n",
	   cw, t->lprob);
    goto again;
   
  case 'u':        
  case 'U':
    printf("Nodes: %d,  Depth: %d,  Ave. Depth: %g, Current depth: %d\n",
	   tree_size(tree),  tree_depth(tree), (double) tree_ave_depth(tree), 
		depth);
    goto again;
    
  case 'l':        
  case 'L':
    if ( icontext >= 7 )
	goto again;
    printf("Number\tname\n");
    for ( j=max_gmi-1; j>0; j--)
      if (treeoptions[j]->gain > -FLOATMAX) {
	printf("%d\t",j);
	display_test(treeoptions[j]->test,stdout);
	putc('\n',stdout);
      }
    goto again;
    
  case 'g':
  case 'G':
    if ( icontext >= 7 )
	goto again;
    if ( icontext==2 ) {
      printf("'r' ignored with option trees or lookahead, use 'o'\n");
      goto again;
    }
    printf("(WARNING: statistics recalculated from full sample at node)\n");
    i = 0;
    if (t_flg) 
	measure = "nits";
    else if ( purflg=='i' )
	measure = "info-gain";
    else if ( purflg=='t' )
	measure = "twoing";
    else if ( purflg=='r' )
	measure = "gain-ratio";
    else if ( purflg=='g' )
	measure = "gini";
    else
        measure = "measure";
    printf("all (0) or number: ");
    scanf("%d", &i);
    if (!i) {
      for(i=1; i<max_gmi; i++) {
	if (treeoptions[i]->gain>-FLOATMAX*0.9999) {
	  printf("**** partition for test ");
	  tmake(t->xtra.gr.egs,treeoptions[i]->test);
	  display_test(treeoptions[i]->test,stdout);
	  printf(" (%d),  ",i);
	  if ( purflg=='r' )
	    printf("gain = %g, ", gainoptions[i]);
	  printf("%s = %g", measure, treeoptions[i]->gain);
	  putchar('\n');
	  tprint();
	  /*
	   *   this if hack skips over multiple cuts of same attr.
           */
	  if ( i>1 && (treeoptions[i]->test->attr ==
	 		treeoptions[i-1]->test->attr) && 
			num_type(treeoptions[i]->test->attr)  )
	    continue;
	  if (do_x_graphs)
	    if (cut_test(treeoptions[i]->test))
	      if (! Fflg) {
		do_graphs = TRUE;
		if (t_flg)
		  {
		    /*
		     *	memcpy hack prevents treeoptions from 
		     *	being overwritten
		     */
		    memcpy(&save_bt,treeoptions[i],sizeof(bt_rec));
		    ord_log_tprob(t,i);
		    memcpy(treeoptions[i],&save_bt,sizeof(bt_rec));
		  }
		else
		  {
		    memcpy(&save_bt,treeoptions[i],sizeof(bt_rec));
		    ord_info(t,i);
		    memcpy(treeoptions[i],&save_bt,sizeof(bt_rec));
		  }
		do_graphs = FALSE;
	      }
	} 
        if ( intint ) {
		intint = 0;
		goto again;
        }
      }   
      printf("**** partition for class %s (%d),", name(0), 0);
      printf("*  %s = %g\n", measure, t->xtra.gr.gain);
      tmake(t->xtra.gr.egs,(test_rec *)0);
      tprint();
    } else {
      if ( i<1 || i >= max_gmi ) {
	printf("illegal test number, ignored\n");
        goto again;
      }
      if (treeoptions[i]->gain>-FLOATMAX) {
	printf("**** partition for test ");
	display_test(treeoptions[i]->test,stdout);
	printf(",  %s = %g\n", measure,
	       treeoptions[i]->gain);
	if (do_x_graphs)
	  if (cut_test(treeoptions[i]->test))
	    if (! Fflg) {
	      do_graphs = TRUE;
	      if (t_flg)
		{
		  /*
		   *	memcpy hack prevents treeoptions from 
		   *	being overwritten
		   */
		  memcpy(&save_bt,treeoptions[i],sizeof(bt_rec));
		  ord_log_tprob(t,i);
		  memcpy(treeoptions[i],&save_bt,sizeof(bt_rec));
		}
	      else
		{
		  memcpy(&save_bt,treeoptions[i],sizeof(bt_rec));
		  ord_info(t,i);
		  memcpy(treeoptions[i],&save_bt,sizeof(bt_rec));
		}
	      do_graphs = FALSE;
	    }
	tmake(t->xtra.gr.egs,treeoptions[i]->test);
	tprint();
      } else {
	printf("**** test ");
	display_test(treeoptions[i]->test,stdout);
	printf(" invalid\n");
      }
      if ( intint ) {
		intint = 0;
		goto again;
      }
    }
    goto again;
    
  case 'k':
  case 'K':
    if ( icontext >= 7 )
	goto again;
    init_graph_panes(); /* resets mulitple graph positioner */
    (void)kill_childproc();
    
    goto again;      

  case 'x':
  case 'X':
    if ( icontext >= 7 )
	goto again;
    if ( ! ncattrs )
	goto again;
    if (do_x_graphs) {
      do_x_graphs = FALSE;
      printf("plotting of continuous attribute gains off.\n");
    }
    else {
      do_x_graphs = TRUE;
      printf("plotting of continuous attribute gains on - now use 'g'.\n");
    }
    goto again;
    
  case 'e':
  case 'E':
    if ( icontext >= 7 )
	goto again;
    if ( icontext==2 ) {
      printf("'e' ignored with option trees or lookahead\n");
      goto again;
    }
    i=0;
    printf("all (0) or number: ");
    scanf("%d", &i);
    if (!i) {
      for(i=1; i<max_gmi; i++)  {
	if (treeoptions[i]->gain>-FLOATMAX) {
	  printf("**** estimated error for test ");
	  display_test(treeoptions[i]->test,stdout);
	  tmake(t->xtra.gr.egs,treeoptions[i]->test);
	  calc_error(&eerr,&estd);
	  printf(" (%d) = %g~%g\n", i, eerr, sqrt((double)estd));
	} 
        if ( intint ) {
		intint = 0;
		goto again;
        }
      }   
      printf("**** estimated error for class %s (%d)", name(0), 0);
      leaf_errstd(t->eg_count,&eerr,&estd);
      printf(" = %g~%g\n", eerr,sqrt((double)estd));
    } else {
      if (treeoptions[i]->gain>-FLOATMAX) {
	printf("**** estimated error for test ");
	display_test(treeoptions[i]->test,stdout);
	tmake(t->xtra.gr.egs,treeoptions[i]->test);
	calc_error(&eerr,&estd);
	printf(" = %g~%g\n", eerr, sqrt((double)estd));
      } else {
	printf("**** test ");
	display_test(treeoptions[i]->test,stdout);
	printf(" invalid\n");
      }
    }
    goto again;
    
  case 'v':           /*  really want to read in flags here  */
  case 'V':
    if ( icontext >= 7 )
	goto again;
    printf("Parents of node: ");
    null_flags(pflags);
    set_flag(pflags,intnl);
    set_flag(pflags,counts);
    set_flag(pflags,gain);
    set_flag(pflags,decis);
    print_parents(t,pflags);
    goto again;

  case 'p':           /*  really want to read in flags here  */
  case 'P':
    printf("Depth to print: ");
    scanf("%d", &i);
    printf("Tree so far: ");
    null_flags(pflags);
    set_flag(pflags,intnl);
    set_flag(pflags,counts);
    set_flag(pflags,gain);
    set_flag(pflags,decis);
    print_tree(tree,pflags,depth+i);
    printf("Test is: ");
    goto again;

  case 'q':
  case 'Q':
    if ( icontext >= 7 )
	goto again;
    printf("Depth to print: ");
    scanf("%d", &i);
    printf("Subtree so far: ");
    null_flags(pflags);
    set_flag(pflags,intnl);
    set_flag(pflags,gain);
    set_flag(pflags,counts);
    print_tree(t,pflags,depth+i);
    goto again;
     
  case 'n':
  case 'N':
    break;

  case 'd':
  case 'D':
    if ( icontext >= 7 )
	goto again;
    if ( icontext == 0 ) {
      if ( bestoptions[1] != 0 ) {
	printf("Enter choice number (use 'r' to see these) to delete: ");
        scanf("%d", j);
	for (  ; bestoptions[j]; j++)
	  bestoptions[j] = bestoptions[j+1];
      } else
	printf("Only 1 choice, use 'f' to force leaf: ");
    } else if ( icontext == 2 ) {
      if ( ! toptions(t) ) {
	printf("no options in this node\n");
	goto again;
      }
      printf("Enter option number (use 'o' to see these) to delete: ");
      scanf("%d", j);
      rem_option(t,j);
    }
    break;

  case 'c':
  case 'C':
    if ( icontext >= 7 )
	goto again;
    if ( icontext==2 ) {
      printf("'c' ignored with option trees or lookahead\n");
      goto again;
    }
    if ( bestoptions[1] != 0 )
      printf("Enter test number to be first choice: ");
    else
      printf("Enter test number to be new choice: ");
    i = bestoptions[0];
    scanf("%d", bestoptions);
    if ( bestoptions[0]<=0 || bestoptions[0]>= max_gmi )
      /*
       *      no best option so quit
       */
      break;
    /*
     *     read parameters for the new best test
     */
    if (cut_test(treeoptions[bestoptions[0]]->test)) {
      printf("Enter cut point: ");
      scanf("%f", &treeoptions[bestoptions[0]]->test->tval.cut);
    } else if (set_test(treeoptions[bestoptions[0]]->test)) {
	error("unimple.\n","");
    } else if (subset_test(treeoptions[bestoptions[0]]->test)) {
      printf("Enter octal set specification: ");
      scanf("%o", &treeoptions[bestoptions[0]]->test->tval.valset);
    } else if (subset_test(treeoptions[bestoptions[0]]->test)) {
	error("unimple.\n","");
    }
    /*
     *     reshuffle bestoptions
     */
    if ( !i ) {
      bestoptions[1] = 0;
      break;
    } else {
      for (j=1; bestoptions[j] && bestoptions[0]!=bestoptions[j]; j++) {
	k = i;
	i = bestoptions[j];
	bestoptions[j] = k;
      }
      if ( bestoptions[j] )
	bestoptions[j] = i;
      else {
	bestoptions[j] = i;
	bestoptions[j+1] = 0;
      }
    }
    break;

  case 'm':
  case 'M':
    creep = TRUE;
    break;

  case 'f':
  case 'F':
    if ( icontext >= 7 )
	goto again;
    force_leaf(t);
    break;

  case 'z':
  case 'Z':
    myexit(1);

  case 'a':
  case 'A':
    depth_bound = 0;     /*  this will stop any further growth  */
    for (i=0; i<CONTEXTS; i++)
    	ignore_interact[i] = TRUE;
  case 'w':
  case 'W':
    override = 0;
  case 's':
  case 'S':
    if ( icontext || !jtree_flag )
      oflg = 0;
    break;

  default:
    goto again;
  }
  return override;
}


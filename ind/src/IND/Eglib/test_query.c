/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#define  TESTED
#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

/* 
 *	queries to find value of attribute relevant to test then
 *	returns outcome of test on attribute (see also #defines in SYM.h)
 *		-1 for unknown, 0-nvals otherwise
 */
int query_outcome(eg,tester)
test_rec  *tester;
egtype 	eg;
{
  char	vname[2000];
  float	f;
  int     bit;
  bitset   set;
  
  if ( !tester ) 
    return class_val(eg);
  if ( cut_test(tester) )  {
    if ( ord_val(eg,tester->attr) == FDKNOW ) {
repeat_cont:
      fprintf(stdrep,"Value for continuous attribute %s: ",
	      name(tester->attr) );
      scanf("%s",&vname[0]);
      if ( *vname != DKNsymb || *(vname+1) != 0 ) {
	/*  can't be "?"  */
	if ( sscanf(vname,"%f%s",&f,vname) != 1 ) {
	  fprintf(stdrep, 	
		  "Value not a float, retry or '?'.\n");
	  goto repeat_cont;
	}
	ord_val(eg,tester->attr) = f;
      }
    }
  } else if ( set_test(tester) ) {
    if ( set_eq(set_val(eg,tester->attr),SDKNOW) ) {
      fprintf(stdrep,"Value for set attribute %s: ",
	      name(tester->attr) );
      scanf("%s",&vname[0]);
      if ( *vname != DKNsymb || *(vname+1) != 0 ) {
	/*  can't be "?"  */
	if ( (set = lookupset(vname, (int)tester->attr)) == SDKNOW) {
	  fprintf(stdrep,"\nUnrecognized set value \"%s\",\n", vname);
	  fprintf(stdrep," should be in: { ");
	  print_anames(stdrep, (int)tester->attr, 14 ); 
	  fprintf(stdrep,"}\n or \"?\".\n");
	  goto repeat_disc;
	} else
	  set_val(eg,tester->attr) = set;
      }
    }
  } else {
    /*
     *    DISCRETE test
     */
    if ( unord_val(eg,tester->attr)==DONTKNOW ) {
repeat_disc:
      fprintf(stdrep,"Value for discrete attribute %s: ",
	      name(tester->attr) );
      scanf("%s",&vname[0]);
      if ( *vname != DKNsymb || *(vname+1) != 0 ) {
	/*  can't be "?"  */
	if ( (bit = lookupval(vname, tester->attr)) == DONTKNOW) {
	  fprintf(stdrep,"\nUnrecognized value \"%s\",\n", vname);
	  fprintf(stdrep," should be: ");
	  print_anames(stdrep, (int)tester->attr, 12 ); 
	  fprintf(stdrep," or \"?\".\n");
	  goto repeat_disc;
	} else
	  unord_val(eg,tester->attr) = bit;
      }
    }
  }
  return outcome(eg,tester);
}


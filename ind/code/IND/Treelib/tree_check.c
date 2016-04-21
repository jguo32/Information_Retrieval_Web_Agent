/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

/*****************************************
 *
 *	check parent pointers in tree
 * 
 */
static  void  node_pcheck(t)
ot_ptr  t;
{
	int   i,j,k;
	bt_ptr  par, option;
        if ( ttest(t) ) 
                foroptions(option,j,t)  {
                    if ( option->parent != t )
                        error("parent pointer out for option","");
                    foroutcomes(i,option->test) {
			forparents(par,k, option->branches[i])
				if ( par==option) {
					par = 0;
					break;
				}
			if ( par )
			    error("parent pointers out for node","");
                    }
		}
	if ( t->num_parents > 0 ) {
	    forparents(par,k,t)
                    foroutcomes(i,par->test) 
			if ( par->branches[i] == t ) {
				par = 0;
				break;
		        }
		    if ( par )
			    error("parent pointers out for node","");
    	}
}

int
tree_pcheck(t)
ot_ptr	t;
{
	traverse_onodes(t,node_pcheck);
}

/*****************************************
 *
 *	check flags in tree
 * 
 */
static  void  node_fcheck(t)
ot_ptr  t;
{
	int  j;
	bt_ptr  option;
	 if ( bad_tflags(t) )
                error("Bad tflags\n","");
	if (  ttest(t) )  foroptions(option,j,t) 
                if ( bad_tflags(option) )
                        error("Bad tflags\n","");
}

tree_fcheck(t)
ot_ptr	t;
{
	traverse_onodes(t, node_fcheck);
}

/*****************************************
 *
 *	check sets in tree
 * 
 */
static  void  node_scheck(t)
ot_ptr  t;
{
	if ( t->xtra.gr.egs ) check_set(t->xtra.gr.egs);
}

tree_scheck(t)
ot_ptr	t;
{
	traverse_onodes(t, node_scheck);
}


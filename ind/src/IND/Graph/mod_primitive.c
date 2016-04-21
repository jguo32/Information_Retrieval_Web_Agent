/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "mod.h"

/********************************************************/
/*	delete_mod routines				*/
/********************************************************/

Mod *delete_mod(md, lmods)
Mod *md, *lmods;
{
Mod *curr;
	if (md == lmods) {
		ASSERT (md->prev == (Mod *)0)
		curr = md->next;
		if (curr) {
			curr->prev = (Mod *)0;
		}
		lmods = md->next;
	} else {
		curr = md->prev;
		ASSERT (curr != (Mod *)0)
		if ( md->next == (Mod *)0 ) {
			curr->next = (Mod *)0;
		} else {
			curr->next = md->next;
			md->next->prev = curr;
		}
	}
	free_mod(md);
	return(lmods);
}

delete_lmods(lmods)
Mod *lmods;
{
Mod *tmp, *tmpnext;
	tmp = lmods;
	while (tmp) {
		tmpnext = tmp->next;
		lmods = delete_mod(tmp, lmods);
		tmp = tmpnext;
	}
	ASSERT(lmods == (Mod *)0)
}

/********************************************************/
/*	concat routines					*/
/*	concatenate 2 lists of modifications (l1, l2)	*/
/********************************************************/

Mod *concat(l1, l2)
Mod *l1, *l2;
{
Mod *tmp;
        if (! l1)
                return(l2);
        if (! l2)
                return(l1);
        tmp = l1;
        while (tmp->next)
                tmp = tmp->next;
        ASSERT (tmp->next == 0)
        tmp->next = l2;
        ASSERT (l2->prev == 0)
        l2->prev = tmp;
        return(l1);
}

/********************************************************/
/*	length_ routines				*/
/********************************************************/

static length_list_mods(lmods)
Mod *lmods;
{
Mod *next;
int j;
        j = 0;
        ASSERT( ! lmods->prev )
        while(lmods) {
                next  = lmods->next;
                if (next) {
                        ASSERT( next->prev == lmods )
                }
                lmods = next;
                j ++;
        }
        return(j);
}

/********************************************************/
/*	print_mod routines				*/
/********************************************************/

print_mod(md)
Mod *md;
{
	if (Join_Mod(md)) {
		printf("Join (%6.0f) -  (%6.0f) Save %8.2f nits [%6.1f]  ",
			md->tot_count1, md->tot_count2, 
			md->Abs_ML_saving, md->Diff_ML_saving);
	} else {
		printf("Node: %6.0f egs  Attr %2d Save %8.2f nits [%6.1f]  ",
			md->node1->tot_count, md->option->test->attr,
			md->Abs_ML_saving, md->Diff_ML_saving);
	}

	if (md->type_mod == COMPLEX_SPLIT_MOD) {
		printf("C.Spl ");
	} else if (md->type_mod == SPLIT_MOD) {
		printf("Split ");
	} else if (md->type_mod == JOIN_MOD) {
		printf("Join  ");
	} else if (md->type_mod == COMPLEX_JOIN_MOD) {
		printf("CJoin ");
	} else {
		printf("Error ");
	}

	printf("%3d ", md->execution_order);

	if (md->active_flag)
		printf("Act ");
	else
		printf("    ");

	if (md->choose_mod_flag)
		printf("Ch\n");
	else
		printf("  \n");
}

print_list_mods(lmods)
Mod *lmods;
{
	printf("===== List Mods =====\n");
	while(lmods) {
		print_mod(lmods);
		lmods = lmods->next;
	}
	printf("=== End List Mods ===\n");
}

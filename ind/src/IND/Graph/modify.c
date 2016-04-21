/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

#include <stdio.h>
#include <math.h>
#include "mod.h"

static float saving_thresh, normalise_total;
extern int Wander_flag;

/************************************************************************
 *
 *      set_traverse_flag routine
 *		the traverse flag should be set if this mod
 *		is suitable for selection by choose_mod
 */

static set_traverse_flag(mp)
Mod *mp;
{
	if (mp->choose_mod_flag)
		mp->traverse_flag = ( Saving(mp) > saving_thresh );
	else
		mp->traverse_flag = 0;
}

init_traverse_flag(mp)
Mod *mp;
{
	while (mp) {
		mp->traverse_flag = 1;
		mp = mp->next;
	}
}

/************************************************************************
 *
 *      add_up_normalise routine
 *              sum the probabilities of the list of modifications
 */

static add_up_normalise(mp)
Mod *mp;
{
	mp->Rel_saving = pow(2.0, Saving(mp) - saving_thresh );
	normalise_total += mp->Rel_saving;
	ASSERT(normalise_total > 0)
}

/************************************************************************
 *
 *      choose_mod routine
 *		given a list of mods (lmods) choose the next one to
 *		be performed.
 *	If Wander_flag then
 *		choose mp stochastically
 *	else
 *		choose mp to be the best (greatest savings in message_length)
 */

Mod *choose_mod(lmods)
Mod *lmods;
{
float roll;
Mod *mp;
Mod *mp2;
float diff;
	ASSERT(lmods != NULL)
	mp = max_mod(lmods);
	if ( ! Wander_flag )
		return(mp);

	if (Saving(mp) > Mod_Include_Limit) {
		saving_thresh = Saving(mp) - Mod_Include_Limit;
	} else {
		mp2 = min_mod(lmods);
		if (Saving(mp2) > 0) {
			saving_thresh = 0.0;
		} else {
			saving_thresh = Saving(mp2) - 1;
			diff =  Saving(mp) - Mod_Include_Limit;
			if (saving_thresh < diff)
				saving_thresh = diff;
		}
	}

	init_traverse_flag(lmods);
	traverse_mod(lmods, set_traverse_flag);

	normalise_total = 0.0;
	traverse_mod(lmods, add_up_normalise);

	roll = frandom() * normalise_total;
	mp = lmods;
	for ( mp = lmods; mp; mp = mp->next ) {
		if (mp->traverse_flag) {
			roll -= mp->Rel_saving;
			mp2 = mp;
			if (roll <= 0.0)
				break;
		}
	}
	ASSERT(roll <= normalise_total * EPSILON)
	return(mp2);
}

/************************************************/
/* 	primitive routines for lists of Mods	*/
/*		traverse_mod(mp, f)		*/
/*		inti_traverse_flag(mp)		*/
/*		Mod *max_mod(mp)		*/
/*		Mod *min_mod(mp)		*/
/************************************************/

int malloc_error = 0;

Mod *make_mod(lmods, type_mod, option, node1, node2, pathflag)
Mod *lmods;
int type_mod;
bt_ptr option;
ot_ptr node1, node2;
int pathflag;
{
Mod *tmp;
	tmp = mem_alloc(Mod);
	if (tmp == (Mod *)0) {
		malloc_error = 1;
		printf("out of memory file %s line %d\n", __FILE__, __LINE__);
		return( lmods );
	}
	/*	Abs_ML_saving	set in procedures	*/
	/*	Rel_saving	set in procedures	*/
	/*	traverse_flag	set in procedures	*/
	/*	choose_mod_flag	set in procedures	*/
	tmp->active_flag = 0;
	tmp->type_mod = type_mod;
	tmp->node1 = node1;
	if (type_mod == SPLIT_MOD) {
		ASSERT (node2 == (ot_rec *)0)
		tmp->node2 = (ot_rec *)0;
		tmp->option = option;
                tmp->tot_count1 = tmp->tot_count2 = 0.0;
	} else {
		ASSERT (type_mod == JOIN_MOD)
		ASSERT (option == (bt_rec *)0)
		tmp->option = (bt_rec *)0;
		tmp->node2 = node2;
                tmp->tot_count1 = node1->tot_count;
                tmp->tot_count2 = node2->tot_count;
	}
	tmp->jtree = (ot_rec *)0;
	tmp->next = lmods;
	tmp->prev = (Mod *)0;
	if (lmods != (Mod *)0)
		lmods->prev = tmp;
	tmp->execution_order = 0;
	tmp->path1 = (char *)0;
	tmp->path2 = (char *)0;
	if (pathflag && Wander_flag) {
		set_path(tmp);
	}
	return(tmp);
}

/************************************************************************
 *
 *      free_mod routine
 */

free_mod(md)
Mod *md;
{
	if (md->path1)
		sfree(md->path1);
	if (md->path2)
		sfree(md->path2);
	free_btree(md->option);
	sfree(md);
}

/************************************************************************
 *
 *      traverse the list of mods calling f
 */

traverse_mod(mp, f)
Mod *mp;
int (*f)();
{
	while (mp) {
		if (mp->traverse_flag) {
			(*f)(mp);
		}
		mp = mp->next;
	}
}

/************************************************************************
 *
 *      max_mod routine
 *		find the mod with the greatest saving
 */

Mod *max_mod(mp)
Mod *mp;
{
float ML_saving;
Mod *max_mp;
	ASSERT( mp != NULL)
	while ( ! mp->choose_mod_flag )
		mp = mp->next;
	max_mp = mp;
	ML_saving = Saving(mp);
	mp = mp->next;
	while (mp) {
		if (( mp->choose_mod_flag ) && (Saving(mp) > ML_saving)) {
			max_mp = mp;
			ML_saving = Saving(mp);
		}
		mp = mp->next;
	}
	return(max_mp);
}

/************************************************************************
 *
 *      min_mod routine
 *              find the mod with the smallest saving
 */

Mod *min_mod(mp)
Mod *mp;
{
float ML_saving;
Mod *min_mp;
	ASSERT( mp != NULL)
	while ( ! mp->choose_mod_flag )
		mp = mp->next;
	min_mp = mp;
	ML_saving = Saving(mp);
	mp = mp->next;
	while (mp) {
		if (( mp->choose_mod_flag ) && (Saving(mp) < ML_saving)) {
			min_mp = mp;
			ML_saving = Saving(mp);
		}
		mp = mp->next;
	}
	return(min_mp);
}

/********************************************************/
/*	Saving() routines 				*/
/*		determine the type of Saving to be	*/
/*		used to select a mod by choose_mod	*/
/********************************************************/

#define ABS	1
#define DIFF	2
static int Saving_type;

set_abs_savings()
{
	Saving_type = ABS;
}

set_diff_savings()
{
	Saving_type = DIFF;
}

static float Saving(mp)
Mod *mp;
{
	if (Saving_type == ABS)
		return( mp->Abs_ML_saving );
	if (Saving_type == DIFF)
		return( mp->Diff_ML_saving );
	ASSERT(0);
	return(0.0);
}


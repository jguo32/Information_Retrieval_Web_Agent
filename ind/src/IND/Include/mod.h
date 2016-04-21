/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

/* written Jonathan Oliver 1992 */

#include "SYM.h"
#include "sets.h"
#include "TREE.h"
#include "DEFN.h"

struct MOD {
	float Abs_ML_saving;  /* Savings in nits that this mod can do */
	float Diff_ML_saving;
		/* Saving Best mod (at this node) - Savings this mod */
	float Rel_saving;
	unsigned traverse_flag : 1;
	unsigned active_flag : 1;
	unsigned choose_mod_flag : 1;
		/* choose_mod_flag is used choose_mod() in jono_modify.c */
	unsigned wander_flag : 1;
		/*   have I wandered through this  */
	unsigned char type_mod;
	ot_ptr node1, node2, jtree;
	bt_ptr option;
	struct MOD *next, *prev;
	short execution_order;
	char *path1, *path2;
	float tot_count1, tot_count2;
};

#define Mod_Include_Limit	20.0

typedef struct MOD Mod;

Mod *delete_mod ();
Mod *choose_mod ();
Mod *make_mod();
Mod *copy_mod();
Mod *make_split_mods2();
Mod *make_join_mods();
Mod *make_new_mods2();
Mod *grow_probabilistically();
Mod *concat();

Mod *wander_through_tree_space();
ot_ptr wander_through_graph_space();

ot_ptr locate_node_according_path();
ot_ptr perform_join();
ot_ptr join_two_onodes();

int traverse_mod();
int init_traverse_flag();
Mod *max_mod();
Mod *min_mod();
float message_length(), Saving();
float leaf_length();

#define SPLIT_MOD		0
#define JOIN_MOD		1
#define COMPLEX_SPLIT_MOD	2
#define COMPLEX_JOIN_MOD	3

#define PRIMITIVE_MOD(md)	( (md->type_mod == SPLIT_MOD) || \
				  (md->type_mod == JOIN_MOD) )
#define Join_Mod(md)		( (md->type_mod == JOIN_MOD) || \
				  (md->type_mod == COMPLEX_JOIN_MOD) )

#define MAX_GOOD_TREE   10

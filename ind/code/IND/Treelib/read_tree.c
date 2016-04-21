/*IND-version2.0
 *   IND 2.0 released 9/15/92   contact: ind@kronos.arc.nasa.gov
 *   by Wray Buntine (and others, see IND/README)
 *   NASA Ames Research Center, MS 269-2, Moffett Field, CA 94035
 */

#include <stdio.h>
#include "SYM.h"
#include "sets.h"
#include "TREE.h"

 
/***********************************************************************/
/*
 *	read in single tree from file with name "str"
 */

read_tree(str, ret, head)
t_head	*head;
ot_ptr	*ret;
char	*str;
{
	FILE	*fp, *efopen();
	ot_ptr   t;
	static ot_ptr   _read_tree();
 
	fp = efopen(str, "r");
 
	if (fread((char *) head, sizeof(t_head), 1, fp) != 1)
		uerror("Could not read in decision tree\n", "");
	if (head->magicno != MAGICNO)
		uerror("Incorrect tree file type -- bad magic number", "");
#ifdef GRAPH
        if (thgraph(head)) {
                decision_graph_flag = 1;
        }
        alloc_jline_no_tree();
#endif

	t = _read_tree(fp);

#ifdef GRAPH
	ASSERT (t->num_parents == 1)
	ASSERT (t->parent_c == 0)
	t->parents = (bt_ptr *) salloc(sizeof(bt_ptr)*t->num_parents);
	t->parents[t->parent_c] = 0;
	t->parent_c ++;
	check_parent_graph(t, __LINE__, __FILE__);
#else
	t->parents = (bt_ptr *) salloc(sizeof(bt_ptr));
	t->parents[0] = 0;
#endif
	fclose(fp);
	*ret = t;
}
 
#ifdef GRAPH
/***********************************************************************/
/*
 *	check_parent	routines
 */

static char Fname[100];
static int Lno;

static void check_parent_node(t)
ot_ptr  t;
{
	if (t->num_parents != t->parent_c) {
		printf("error %d %s while checking parent_c in dgraph\n",
			Lno, Fname);
		exit(0);
	}
}

check_parent_graph(t, lineno, fname)
ot_ptr  t;
int lineno;
char *fname;
{
	strcpy(Fname, fname);
	Lno = lineno;
	traverse_onodes(t, check_parent_node);
}
#endif

/***********************************************************************/
/*
 *	read header
 */
read_header(str, head)
t_head	*head;
char	*str;
{
	FILE	*fp, *efopen();
 
	fp = efopen(str, "r");
	if (fread((char *) head, sizeof(t_head), 1, fp) != 1)
		uerror("Could not read in decision tree\n", "");
	if (head->magicno != MAGICNO)
		uerror("Incorrect tree file type -- bad magic number", "");
	fclose(fp);
}

/************************************************************************/
/*
 *	_read_tree(fp)	--  allocate pointers to tree nodes
 */

ot_ptr find_jline_no();

static ot_ptr _read_tree(fp)
FILE	*fp;
{
	int	i,j;
	bt_ptr	option;
	ot_ptr   t, t2;
 
	t = (ot_ptr)salloc(sizeof(ot_rec));
	if (! fread(&(t->tflags), sizeof(tree_flags), 1, fp)  )
		uerror("Could not read in decision tree\n", "");
	if (tempty(t)) {
		sfree(t);
		return (ot_ptr)0;
	}


        if ( !fread(&(t->line_no), sizeof(int), 1, fp)  )
                uerror("Could not read in decision tree\n", "");
#ifdef GRAPH
        if (t->line_no) {
                t2 = find_jline_no(t->line_no);
                if (t2) {
                        ASSERT(t2->line_no == t->line_no)
                        /* read before */
                        sfree(t);
                        return(t2);
                } else {
                        save_jline_no_node(t);
                }
        }
        t->parent_c = 0;
#endif
        if ( !fread(&(t->num_parents), sizeof(int), 1, fp)  )
                uerror("Could not read in decision tree\n", "");
        t->parents = (bt_ptr *) salloc(sizeof(bt_ptr)*t->num_parents);
	t->testing.unordp = 0;
	if ( !fread(&(t->tot_count), sizeof(float), 1, fp)  )
		uerror("Could not read in decision tree\n", "");
	if (! fread(&(t->lprob), 1, sizeof(float), fp)  )
		uerror("Could not read in decision tree\n", "");
	t->eg_count = (float *) salloc(sizeof(float)*ndecs);
	if ( fread((char *)t->eg_count, sizeof(float), ndecs, fp) != ndecs)
		uerror("Could not read in decision tree\n", "");
	if ( ttest(t) ) {
		if ( toptions(t) ) {
		if (! fread(&(t->option.s.c), sizeof(short), 1, fp) )
			uerror("Could not read in decision tree\n", "");
		t->option.s.o = (bt_ptr *) salloc(t->option.s.c*sizeof(bt_ptr));
		for (i=0; i<t->option.s.c; i++)
			t->option.s.o[i] = (bt_ptr) salloc(sizeof(bt_rec));
		} else {
			option = t->option.o = (bt_ptr) salloc(sizeof(bt_rec));
		}
		foroptions(option,j,t) {
			if (! fread(&(option->tflags), sizeof(int), 1,  fp) )
				uerror("Could not read in decision tree\n", "");
			if (! fread(&(option->nprob), sizeof(float),1, fp) )
				uerror("Could not read in decision tree\n", "");
			if ( !fread(&(option->np.nprop), sizeof(float), 1, fp) )
				uerror("Could not read in decision tree\n", "");
			if ( !fread(&(option->gain), sizeof(float), 1, fp) )
				uerror("Could not read in decision tree\n", "");
			option->parent = t;
			option->test = (test_rec *) salloc(sizeof(test_rec));
			if ( !fread((char *)option->test, sizeof(test_rec), 1,  fp) )
				uerror("Could not read in decision tree\n", "");
			if ( bigsubset_test(option->test) ) {
				if ( ! (option->test->tval.valbigset =
					get_bitarray(unordvals(option->test->attr),fp) ))
				uerror("Could not read in decision tree\n", "");
			}
			option->branches =
				(ot_ptr *) salloc(outcomes(option->test)*sizeof(ot_ptr));
			foroutcomes(i,option->test) {
				if ( option->branches[i] = _read_tree(fp) ) {
					t2 = option->branches[i];
#ifdef GRAPH
					t2->parents[t2->parent_c] = option;
					t2->parent_c ++;
					ASSERT (t2->parent_c <= t2->num_parents)
#else
					t2->parents[0] = option;
#endif
				}
			}
		}
	} else {
		t->option.o = (bt_ptr)0;
	}
	return t;
}

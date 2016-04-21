# Makefile for the Tree Library

PROGS = write_tree.c write_chtr.c read_tree.c read_chtr.c \
	treesize.c subtree_prop.c free_tree.c \
	leaf_gain.c print_tree.c \
	pess_prune.c  tree_class.c prior.c \
	depth_prune.c tree_reclass.c \
	tree_reprob.c recnt_tree.c TREE.c dec_prune.c \
	tree_add.c alpha_list.c reerr_tree.c alpha_err.c \
	CC_tst_prune.c CC_test_grow.c CC_CV_grow.c \
	alpha_min.c copy_tree.c tree_check.c \
	minerr_prune.c tree.c rem_tree.c \
	exp_nodes.c wall_grow.c chkcnt_tree.c \
	choose.c makeopttree.c interact.c  tree_subw.c \
	tree_smooth.c tree_test.c traverse.c tree_smpl.c
FILES	=  Makefile $(PROGS)
OBJS = 	write_tree.o write_chtr.o read_tree.o read_chtr.o \
	treesize.o subtree_prop.o free_tree.o \
	leaf_gain.o print_tree.o \
	pess_prune.o  tree_class.o prior.o \
	depth_prune.o tree_reclass.o \
	tree_reprob.o recnt_tree.o TREE.o dec_prune.o \
	tree_add.o alpha_list.o reerr_tree.o alpha_err.o \
	CC_tst_prune.o CC_test_grow.o CC_CV_grow.o \
	alpha_min.o copy_tree.o tree_check.o \
	minerr_prune.o tree.o rem_tree.o \
	exp_nodes.o wall_grow.o chkcnt_tree.o \
	choose.o makeopttree.o interact.o  tree_subw.o \
	tree_smooth.o tree_test.o traverse.o tree_smpl.o

INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
	   class.h sets.h
CFLAGOPTS = 
CFLAGS	= -I../Include $(CFLAGOPTS)
CC	= gcc

install:	libtree.a

libtree.a:	$(OBJS)
	rm -f libtree.a
	ar qv libtree.a $(OBJS)
	ranlib libtree.a

.c.o:
	$(CC) -c $(CFLAGS) $< 

lint:
	lint -I../Include $(PROGS) 

clean:
	rm -f $(OBJS) *.O $(INCLUDES) a.out

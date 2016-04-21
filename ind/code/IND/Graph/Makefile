# Makefile for the Graph growing and Modify Library

PROGS = modify.c perform.c mess_length.c stored_tree.c \
	suggest_mods.c check_distn.c is_dgraph.c  trans_graph.c \
	graph_reprob.c mod_primitive.c wander_tree.c path.c \
	wander_graph.c dgraph_opts.c
FILES	=  Makefile $(PROGS)
OBJS = 	modify.o perform.o mess_length.o stored_tree.o \
	suggest_mods.o check_distn.o is_dgraph.o  trans_graph.o \
	graph_reprob.o mod_primitive.o wander_tree.o path.o \
	wander_graph.o dgraph_opts.o

INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
CFLAGOPTS = 
CFLAGS	= -I../Include $(CFLAGOPTS)
CC	= gcc

install:	libjgph.a

libjgph.a:	$(OBJS)
	rm -f libjgph.a
	ar qv libjgph.a $(OBJS)
	ranlib libjgph.a

.c.o:
	$(CC) -c $(CFLAGS) $< 

lint:
	lint -I../Include $(PROGS) 

clean:
	rm -f $(OBJS) *.O  a.out $(INCLUDES)

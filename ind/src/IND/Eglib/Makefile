# Makefile for the Lib

SYM	=  ../Include/SYM.h
SET	=  ../Include/sets.h
BIN	= /home/copernicus2/wray/bin

LIST_PROGS = lists.c splits_lists.c
PROGS	=  sort_set.c  enc_file.c print_sym.c \
	   symbol.c symlex.c symyacc.c tested.c read_eg.c  \
	   read_enc_egs.c write_enc_eg.c read_enc_eg.c $(LIST_PROGS) \
	   partition.c assign_unk.c \
	   table.c tablestats.c write_eg.c shuffle_set.c \
	   test_display.c test_mem.c test_random.c \
	   test_outcome.c randset.c write_enc_egs.c test_query.c 
FILES	=  Makefile $(PROGS) sym.l sym.y sym.h lists.c splits_lists.c
LIST_OBJS = lists.o splits_lists.o
OBJS	=  sort_set.o  enc_file.o print_sym.o \
	   symbol.o symlex.o symyacc.o tested.o read_eg.o  \
	   read_enc_egs.o write_enc_eg.o read_enc_eg.o $(LIST_OBJS) \
	   partition.o assign_unk.o \
	   table.o tablestats.o write_eg.o shuffle_set.o  \
	   test_display.o test_mem.o test_random.o \
	   test_outcome.o randset.o write_enc_egs.o test_query.o 

INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
CFLAGOPTS = -O2
CFLAGS	= -I../Include $(CFLAGOPTS)
CC	= cc

install:	libeg.a

libeg.a:	$(OBJS)
	rm -f libeg.a
	ar qv libeg.a $(OBJS)
	ranlib libeg.a

#     this shouldn't need to be run since symlex.c supplied with release
symlex.c:  sym.l  
	flex -t sym.l > symlex.c

symlex.o:  symyacc.o $(SYM) symlex.c
	$(CC) -c $(CFLAGS) symlex.c

#     this shouldn't need to be run since symyacc.c supplied with release
sym.h symyacc.c:   sym.y 
	yacc -d sym.y
	mv y.tab.h sym.h
	mv y.tab.c symyacc.c

symyacc.o:   symyacc.c $(SYM)
	$(CC) -c $(CFLAGS) symyacc.c

lists.o:	lists.c $(SET)
	$(CC) -c $(CFLAGS) lists.c 
splits_lists.o:	splits_lists.c $(SET)
	$(CC) -c $(CFLAGS) splits_lists.c 

.c.o:
	$(CC) -c $(CFLAGS) $< 

lint:
	lint -I../Include $(PROGS) ../Trees/table.c ../Trees/choose.c

clean:
	rm -f $(OBJS) *.O  a.out  $(INCLUDES)

# Makefile for the Stats utilities lib

SET	=  ../Include/SET.h
DEFN	=  ../Include/Lib.h
CC	= gcc

PROGS = bitset.c copy_fvec.c count_dvec.c ddot.c fdot.c efopen.c \
	error.c make_dmtx.c make_fmtx.c make_imtx.c \
	pr_mtx.c pr_vec.c salloc.c make_pmtx.c  \
	interrupt.c bitarray.c myexit.c matherr.c check_alloc.c \
	scanin.c hashc.c  int_log.c heap.c gnuplot.c
FILES	=  Makefile $(PROGS) 
OBJS = bitset.o copy_fvec.o count_dvec.o ddot.o fdot.o efopen.o \
	error.o make_dmtx.o make_fmtx.o make_imtx.o \
	pr_mtx.o pr_vec.o salloc.o make_pmtx.o  \
	interrupt.o bitarray.o myexit.o matherr.o check_alloc.o \
	scanin.o hashc.o  int_log.o heap.o gnuplot.o
INCLUDES = BITARRAY.h DEFN.h SYM.h TREE.h mod.h BITSET.h Lib.h TABLE.h \
           class.h sets.h
CFLAGOPTS =  
CFLAGS	= -I../Include $(CFLAGOPTS) 
LIBES   = -L../Eglib -L../Treelib -L../Statlib -L../Lib  -L../Graph \
                -ljgph -ltree -ljgph -leg -lstat -leg -lutil -lm

install:	libutil.a

libutil.a:	$(OBJS)
	rm -f libutil.a
	ar qv libutil.a $(OBJS)
	ranlib libutil.a

.c.o:    $(SET)  $(DEFN)
	$(CC) -c $(CFLAGS) $< 

clean:
	rm -f *.O libutil.a a.out *.o  $(INCLUDES)
